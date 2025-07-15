
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

#include <stdio.h>
#include <stdint.h>
#include "ssdv.h"

/* Printing */
void ssdv_print_header(uint8_t *pkt, FILE* fd);
void ssdv_print_header_stdout(uint8_t *pkt);
void ssdv_print_header_stderr(uint8_t *pkt);

/* Decoding */
char ssdv_dec_init_default(ssdv_t *ssdv);
uint8_t* ssdv_dec_buf_opts(ssdv_t *ssdv, uint8_t *src, size_t len_in, int verbose, int droptest, size_t *len_out);
uint8_t* ssdv_dec_buf(ssdv_t *ssdv, uint8_t *src, size_t len_in, size_t *len_out);
int ssdv_dec_file_opts(ssdv_t *ssdv, FILE *fin, FILE *fout, int droptest, int verbose);
int ssdv_dec_file(ssdv_t *ssdv, FILE *fin, FILE *fout);

/* Encoding */
char ssdv_enc_init_default(ssdv_t *ssdv);
uint8_t* ssdv_enc_buf(ssdv_t *ssdv, uint8_t *src, size_t len_in, size_t *len_out);
int ssdv_enc_file(ssdv_t *ssdv, FILE *fin, FILE *fout);

