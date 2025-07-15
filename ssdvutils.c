
/* SSDV - Slow Scan Digital Video                                        */
/*=======================================================================*/
/* Copyright 2011-2016 Philip Heron <phil@sanslogic.co.uk>               */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/*
 * ssdvutils.h added by GLRobotics 2025
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ssdv.h"
#include "ssdvutils.h"

// Memory struct for internal use
typedef struct ssdv_mem_arena_t ssdv_mem_arena_t;
struct ssdv_mem_arena_t {
    uint8_t *buf;
    size_t length;
    size_t used;
};

// Mimics the behavior of fread for code consistency between file and buffer API
static size_t ssdv_memcpy_packet(ssdv_mem_arena_t *src, ssdv_mem_arena_t *dest, size_t read_offset, size_t pkt_length) {
    size_t src_avail = src->length - read_offset;
    size_t dest_avail = dest->length - dest->used;
    size_t copy_length = (src_avail < dest_avail)? src_avail : dest_avail;
    copy_length = (copy_length < pkt_length)? copy_length : pkt_length;
    memcpy(dest->buf + dest->used, src->buf + read_offset, copy_length);
    dest->used += copy_length;
    return copy_length;
}

// Init the ssdv struct for encoding with default parameters
char ssdv_enc_init_default(ssdv_t *ssdv) {
    char result = ssdv_enc_init(ssdv, SSDV_TYPE_NORMAL, "", 0, 4, 256);
    return result;
}

// Init the ssdv struct for decoding with default parameters
char ssdv_dec_init_default(ssdv_t *ssdv) {
    char result = ssdv_dec_init(ssdv, 256);
    return result;
}

// Print the header decoded from pkt into fd
void ssdv_print_header(uint8_t *pkt, FILE* fd) {

    ssdv_packet_info_t p;
    ssdv_dec_header(&p, pkt);
    fprintf(fd, "decoded image packet. callsign: \"%s\", image id: %d, resolution: %dx%d, packet id: %d \n"
            ">> type: %d, quality: %d, eoi: %d, mcu mode: %d, mcu offset: %d, mcu id: %d/%d\n",
            p.callsign_s,
            p.image_id,
            p.width,
            p.height,
            p.packet_id,
            p.type,
            p.quality,
            p.eoi,
            p.mcu_mode,
            p.mcu_offset,
            p.mcu_id,
            p.mcu_count
           );
}

// Print the header decoded from pkt to stdout
void ssdv_print_header_stdout(uint8_t *pkt) {
    ssdv_print_header(pkt, stdout);
    return;
}

// Print the header decoded from pkt to stderr
void ssdv_print_header_stderr(uint8_t *pkt) {
    ssdv_print_header(pkt, stderr);
    return;
}

/* Decode a buffer of ssdv packets
 * Fills a buffer with the resulting jpeg binary data,
 * and puts the length of the buffer in len_out
 */
uint8_t* ssdv_dec_buf_opts(ssdv_t *ssdv, uint8_t *src, size_t len_in, int verbose, int droptest, size_t *len_out) {

    ssdv_mem_arena_t result = {0};
    ssdv_mem_arena_t src_arena = {0};
    src_arena.buf = src;
    src_arena.length = len_in;
    src_arena.used = len_in;

    int src_good = src != 0 && len_in > 0;

    int i, c;
	int errors;
	int pkt_length = SSDV_PKT_SIZE;
	int skipped;

	uint8_t pkt[SSDV_PKT_SIZE], *jpeg;
	size_t jpeg_length;

    ssdv_mem_arena_t pkt_arena = {0};
    pkt_arena.buf = pkt;
    pkt_arena.length = SSDV_PKT_SIZE;

    if (!src_good) {
        fprintf(stderr, "Buffer error. Src: %p %u\n", src_arena.buf, src_arena.length);
        return 0;
    }

    jpeg_length = 1024 * 1024 * 4;
    jpeg = malloc(jpeg_length);
    ssdv_dec_set_buffer(ssdv, jpeg, jpeg_length);
    result.length = jpeg_length;

    i = 0;

    size_t read_offset = 0;
    size_t bytes_read = 0;
    while((bytes_read = ssdv_memcpy_packet(&src_arena, &pkt_arena, read_offset, pkt_length)) > 0)
    {
        read_offset += bytes_read;
        /* Drop % of packets */
        if(droptest && (rand() / (RAND_MAX / 100) < droptest)) continue;

        /* Test the packet is valid */
        skipped = 0;
        while((c = ssdv_dec_is_packet(pkt, pkt_length, &errors)) != 0)
        {
            /* Read 1 byte at a time until a new packet is found */
            memmove(&pkt[0], &pkt[1], pkt_length - 1);
            pkt_arena.used -= 1;
            if ((bytes_read = ssdv_memcpy_packet(&src_arena, &pkt_arena, read_offset, 1)) <= 0)
            {
                break;
            }
            read_offset += 1;

            skipped++;
        }

        /* No valid packet was found before EOF */
        if(c != 0) break;

        if(verbose)
        {

            if(skipped > 0)
            {
                fprintf(stderr, "Skipped %d bytes.\n", skipped);
            }
            ssdv_print_header_stderr(pkt);
            pkt_arena.used = 0;
        }

        /* Feed it to the decoder */
        ssdv_dec_feed(ssdv, pkt);
        i++;
    }

    ssdv_dec_get_jpeg(ssdv, &jpeg, &jpeg_length);
    result.buf = jpeg;
    result.used = jpeg_length;
    fprintf(stderr, "Read %i packets\n", i);
    *len_out = result.used;
    return result.buf;
}

// calls ssdv_dec_buf_opts with default parameters
uint8_t* ssdv_dec_buf(ssdv_t *ssdv, uint8_t *src, size_t len_in, size_t *len_out) {
    uint8_t* result;
	int droptest = 0;
	int verbose = 0;
    result = ssdv_dec_buf_opts(ssdv, src, len_in, verbose, droptest, len_out);
    return result;
}

/* Encode a buffer of jpeg data
 * Fills a buffer with the resulting ssdv packets,
 * and puts the length of the buffer in len_out
 */
uint8_t* ssdv_enc_buf(ssdv_t *ssdv, uint8_t *src, size_t len_in, size_t *len_out) {

    ssdv_mem_arena_t result = {0};
    ssdv_mem_arena_t pkt_arena = {0};
    ssdv_mem_arena_t b_arena = {0};

    ssdv_mem_arena_t src_arena = {0};
    src_arena.buf = src;
    src_arena.length = len_in;
    src_arena.used = len_in;

    ssdv_mem_arena_t tmp = {0};

    size_t alloc_count = 1;
    size_t alloc_size = 4096;

    tmp.buf = malloc(alloc_size);
    tmp.length = alloc_size;

    int i, c;
	int pkt_length = SSDV_PKT_SIZE;
	uint8_t pkt[SSDV_PKT_SIZE], b[128];
    pkt_arena.buf = pkt;
    pkt_arena.length = pkt_length;
    b_arena.buf = b;
    b_arena.length = 128;


    ssdv_enc_set_buffer(ssdv, pkt);

    i = 0;
    size_t read_offset = 0;

    while(1)
    {
        while((c = ssdv_enc_get_packet(ssdv)) == SSDV_FEED_ME)
        {
            b_arena.used = 0;
            //size_t r = fread(b, 1, 128, fin);
            size_t r = ssdv_memcpy_packet(&src_arena, &b_arena, read_offset, 128);;
            read_offset += r;

            if(r <= 0)
            {
                fprintf(stderr, "Premature end of buffer: %u\n", read_offset);
                break;
            }
            ssdv_enc_feed(ssdv, b, r);
        }

        if(c == SSDV_EOI)
        {
            fprintf(stderr, "ssdv_enc_get_packet said EOI\n");
            break;
        }
        else if(c != SSDV_OK)
        {
            fprintf(stderr, "ssdv_enc_get_packet failed: %i\n", c);
            fprintf(stderr, "Total read: %u\n", read_offset);
            return 0;
        }
        // Extend the buffer if we run out of room
        if (tmp.length - tmp.used < pkt_length) {
            alloc_count *= 2;
            tmp.buf = realloc(tmp.buf, alloc_count * alloc_size);
            tmp.length = alloc_count * alloc_size;
            fprintf(stderr, "Had to realloc: %zu bytes used\n", alloc_count * alloc_size);
        }

        ssdv_memcpy_packet(&pkt_arena, &tmp, 0, pkt_length);
        i++;
    }
    if (tmp.length != tmp.used) {
        tmp.buf = realloc(tmp.buf, tmp.used);
        tmp.length = tmp.used;
    }

    fprintf(stderr, "Wrote %i packets\n", i);
    result = tmp;
    *len_out = result.used;
    return result.buf;
}

/* Decode a file of ssdv packets
 * Reads data from fin, and write to fout
 */
int ssdv_dec_file_opts(ssdv_t *ssdv, FILE *fin, FILE *fout, int droptest, int verbose) {

    int i, c;
	int errors;
	int pkt_length = SSDV_PKT_SIZE;
	int skipped;

	uint8_t pkt[SSDV_PKT_SIZE], *jpeg;
	size_t jpeg_length;

    jpeg_length = 1024 * 1024 * 4;
    jpeg = malloc(jpeg_length);
    ssdv_dec_set_buffer(ssdv, jpeg, jpeg_length);

    i = 0;
    while(fread(pkt, pkt_length, 1, fin) > 0)
    {
        /* Drop % of packets */
        if(droptest && (rand() / (RAND_MAX / 100) < droptest)) continue;

        /* Test the packet is valid */
        skipped = 0;
        while((c = ssdv_dec_is_packet(pkt, pkt_length, &errors)) != 0)
        {
            /* Read 1 byte at a time until a new packet is found */
            memmove(&pkt[0], &pkt[1], pkt_length - 1);

            if(fread(&pkt[pkt_length - 1], 1, 1, fin) <= 0)
            {
                break;
            }

            skipped++;
        }

        /* No valid packet was found before EOF */
        if(c != 0) break;

        if(verbose)
        {
            if(skipped > 0)
            {
                fprintf(stderr, "Skipped %d bytes.\n", skipped);
            }

            ssdv_print_header_stderr(pkt);
        }

        /* Feed it to the decoder */
        ssdv_dec_feed(ssdv, pkt);
        i++;
    }

    ssdv_dec_get_jpeg(ssdv, &jpeg, &jpeg_length);
    fwrite(jpeg, 1, jpeg_length, fout);
    free(jpeg);

    fprintf(stderr, "Read %i packets\n", i);
    return 0;
}

// calls ssdv_dec_file_opts with default parameters
int ssdv_dec_file(ssdv_t *ssdv, FILE *fin, FILE *fout) {
    int droptest = 0;
    int verbose = 0;
    int result = ssdv_dec_file_opts(ssdv, fin, fout, droptest, verbose);
    return result;
}

/* Encode a jpeg file to ssdv packets
 * Reads data from fin, and write to fout
 */
int ssdv_enc_file(ssdv_t *ssdv, FILE *fin, FILE *fout) {

    int i, c;
	int pkt_length = SSDV_PKT_SIZE;
	uint8_t pkt[SSDV_PKT_SIZE], b[128];

    ssdv_enc_set_buffer(ssdv, pkt);

    i = 0;

    while(1)
    {
        while((c = ssdv_enc_get_packet(ssdv)) == SSDV_FEED_ME)
        {
            size_t r = fread(b, 1, 128, fin);

            if(r <= 0)
            {
                fprintf(stderr, "Premature end of file\n");
                break;
            }
            ssdv_enc_feed(ssdv, b, r);
        }

        if(c == SSDV_EOI)
        {
            fprintf(stderr, "ssdv_enc_get_packet said EOI\n");
            break;
        }
        else if(c != SSDV_OK)
        {
            fprintf(stderr, "ssdv_enc_get_packet failed: %i\n", c);
            return(-1);
        }

        fwrite(pkt, 1, pkt_length, fout);
        i++;
    }

    fprintf(stderr, "Wrote %i packets\n", i);
    return 0;

}

