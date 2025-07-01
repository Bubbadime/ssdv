
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
#include <unistd.h>
#include <string.h>
#include "ssdv.h"
#include "ssdvutils.h"

ssdv_mem_arena_t ssdv_read_file(FILE* f) {
    ssdv_mem_arena_t result = {0};
    size_t fcur = ftell(f);
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    result.buf = malloc(fsize);
    result.length = fsize;
    fseek(f, 0, SEEK_SET);
    fread(result.buf, result.length, 1, f);
    result.used = fsize;
    fseek(f, fcur, SEEK_SET);
    return result;
}

size_t ssdv_memcpy_packet(ssdv_mem_arena_t *src, ssdv_mem_arena_t *dest, size_t read_offset, size_t pkt_length) {
    size_t src_avail = src->length - read_offset;
    size_t dest_avail = dest->length - dest->used;
    size_t copy_length = (src_avail < dest_avail)? src_avail : dest_avail;
    copy_length = (copy_length < pkt_length)? copy_length : pkt_length;
    memcpy(dest->buf + dest->used, src->buf + read_offset, copy_length);
    dest->used += copy_length;
    return copy_length;

}

ssdv_mem_arena_t ssdv_dec_buf(ssdv_mem_arena_t *src, ssdv_t *ssdv) {

    ssdv_mem_arena_t result = {0};
    int src_good = src->buf != 0 && src->length > 0;

    int i, c;
	int droptest = 0;
	int verbose = 1;
	int errors;
	int pkt_length = SSDV_PKT_SIZE;
	int skipped;

	uint8_t pkt[SSDV_PKT_SIZE], *jpeg;
	size_t jpeg_length;

    ssdv_mem_arena_t pkt_arena = {0};
    pkt_arena.buf = pkt;
    pkt_arena.length = SSDV_PKT_SIZE;

    if (!src_good) {
        fprintf(stderr, "Buffer error. Src: %p %u\n", src->buf, src->length);
        return result;
    }

    jpeg_length = 1024 * 1024 * 4;
    jpeg = malloc(jpeg_length);
    ssdv_dec_set_buffer(ssdv, jpeg, jpeg_length);
    result.length = jpeg_length;

    i = 0;

    size_t read_offset = 0;
    size_t bytes_read = 0;
    while((bytes_read = ssdv_memcpy_packet(src, &pkt_arena, read_offset, pkt_length)) > 0)
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
            if ((bytes_read = ssdv_memcpy_packet(src, &pkt_arena, read_offset, 1)) <= 0)
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
            ssdv_packet_info_t p;

            if(skipped > 0)
            {
                fprintf(stderr, "Skipped %d bytes.\n", skipped);
            }

            ssdv_dec_header(&p, pkt);
            fprintf(stderr, "Decoded image packet. Callsign: \"%s\", Image ID: %d, Resolution: %dx%d, Packet ID: %d (%d errors corrected)\n"
                    ">> Type: %d, Quality: %d, EOI: %d, MCU Mode: %d, MCU Offset: %d, MCU ID: %d/%d\n",
                    p.callsign_s,
                    p.image_id,
                    p.width,
                    p.height,
                    p.packet_id,
                    errors,
                    p.type,
                    p.quality,
                    p.eoi,
                    p.mcu_mode,
                    p.mcu_offset,
                    p.mcu_id,
                    p.mcu_count
                   );
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
    return result;
}

ssdv_mem_arena_t ssdv_enc_buf(ssdv_mem_arena_t *src, ssdv_t *ssdv) {

    ssdv_mem_arena_t result = {0};
    ssdv_mem_arena_t pkt_arena = {0};
    ssdv_mem_arena_t b_arena = {0};

    ssdv_mem_arena_t tmp = {0};

    size_t page_count = 1;
    size_t page_size = getpagesize();

    tmp.buf = malloc(page_size);
    tmp.length = page_size;

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
            size_t r = ssdv_memcpy_packet(src, &b_arena, read_offset, 128);;
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
            return result;
        }
        // Extend the buffer if we run out of room
        if (tmp.length - tmp.used < pkt_length) {
            page_count *= 2;
            tmp.buf = realloc(tmp.buf, page_count * page_size);
            tmp.length = page_count * page_size;
            printf("Had to realloc: %u pages used\n", page_count);
        }

        //fwrite(pkt, 1, pkt_length, fout);
        ssdv_memcpy_packet(&pkt_arena, &tmp, 0, pkt_length);
        i++;
    }

    fprintf(stderr, "Wrote %i packets\n", i);
    result = tmp;
    return result;
}

int ssdv_dec_file(FILE *fin, FILE *fout, ssdv_t *ssdv) {

    int i, c;
	int droptest = 0;
	int verbose = 1;
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
            ssdv_packet_info_t p;

            if(skipped > 0)
            {
                fprintf(stderr, "Skipped %d bytes.\n", skipped);
            }

            ssdv_dec_header(&p, pkt);
            fprintf(stderr, "Decoded image packet. Callsign: \"%s\", Image ID: %d, Resolution: %dx%d, Packet ID: %d (%d errors corrected)\n"
                    ">> Type: %d, Quality: %d, EOI: %d, MCU Mode: %d, MCU Offset: %d, MCU ID: %d/%d\n",
                    p.callsign_s,
                    p.image_id,
                    p.width,
                    p.height,
                    p.packet_id,
                    errors,
                    p.type,
                    p.quality,
                    p.eoi,
                    p.mcu_mode,
                    p.mcu_offset,
                    p.mcu_id,
                    p.mcu_count
                   );
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

int ssdv_enc_file(FILE *fin, FILE *fout, ssdv_t *ssdv) {

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

