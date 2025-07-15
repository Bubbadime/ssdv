
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

char ssdv_enc_init_default(ssdv_t *ssdv);
char ssdv_dec_init_default(ssdv_t *ssdv);

void ssdv_print_header(uint8_t *pkt, int fd);
void ssdv_print_header_stdout(uint8_t *pkt);
void ssdv_print_header_stderr(uint8_t *pkt);

uint8_t* ssdv_read_file(FILE* f, size_t* lenOut);

uint8_t* ssdv_dec_buf(uint8_t *src, size_t lenIn, ssdv_t *ssdv, size_t *lenOut);
uint8_t* ssdv_enc_buf(uint8_t *src, size_t lenIn, ssdv_t *ssdv, size_t *lenOut);

int ssdv_dec_file(FILE *fin, FILE *fout, ssdv_t *ssdv);
int ssdv_enc_file(FILE *fin, FILE *fout, ssdv_t *ssdv);

