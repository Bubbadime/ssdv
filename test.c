
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

/* ssdvutils.h added by GLRobotics 2025 */

// TODO(BP): REMOVE THIS FILE!!!

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ssdv.h"
#include "ssdvutils.h"

uint8_t* ssdv_read_file(FILE* f, size_t *len_out) {
    uint8_t* result = 0;
    size_t fcur = ftell(f);
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    result = malloc(fsize);
    *len_out = fsize;
    fseek(f, 0, SEEK_SET);
    fread(result, *len_out, 1, f);
    fseek(f, fcur, SEEK_SET);
    return result;
}

int main(int argc, char* argv[]) {
    FILE *encIn, *encOut;
    FILE *decIn, *decOut, *decOutOpts;
    FILE *printOut;
	uint8_t image_id = 0;
	int8_t quality = 4;
	int pkt_length = SSDV_PKT_SIZE;
	ssdv_t ssdv;

    encIn = fopen("in.jpeg", "rb");
    encOut = fopen("out.ssdv", "wb");

    decIn = fopen("in.ssdv", "rb");
    decOut = fopen("out.jpeg", "wb");
    decOutOpts = fopen("out_opts.jpeg", "wb");

    printOut = fopen("print.txt", "w");

    uint8_t *encBufIn, *encBufOut;
    uint8_t *decBufIn, *decBufOut, *decBufOutOpts;

    size_t encLenIn, encLenOut, encLenOutOpts;
    size_t decLenIn, decLenOut, decLenOutOpts;

    encBufIn = ssdv_read_file(encIn, &encLenIn);
    decBufIn = ssdv_read_file(decIn, &decLenIn);

    /* Decoding */
    ssdv_dec_init_default(&ssdv);
    ssdv_dec_file(&ssdv, decIn, decOut);
    fclose(decOut);

    ssdv_dec_init_default(&ssdv);
    ssdv_dec_file_opts(&ssdv, decIn, decOutOpts, 1, 0);
    fclose(decOutOpts);

    ssdv_dec_init_default(&ssdv);
    decBufOut = ssdv_dec_buf(&ssdv, decBufIn, decLenIn, &decLenOut);

    ssdv_dec_init_default(&ssdv);
    decBufOutOpts = ssdv_dec_buf_opts(&ssdv, decBufIn, decLenIn, 1, 0, &decLenOutOpts);

/* Encoding */
    ssdv_enc_init_default(&ssdv);
    ssdv_enc_file(&ssdv, encIn, encOut);
    fclose(encOut);

    ssdv_enc_init_default(&ssdv);
    encBufOut = ssdv_enc_buf(&ssdv, encBufIn, encLenIn, &encLenOut);

/* Printing */
    ssdv_fprint_header(decBufOut, printOut);
    ssdv_print_header(decBufOut);
    ssdv_perror_header(decBufOut);

    return 0;
}

