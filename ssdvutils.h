
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

typedef struct ssdv_mem_arena_t ssdv_mem_arena_t;
struct ssdv_mem_arena_t {
    uint8_t *buf;
    size_t length;
    size_t used;
};

ssdv_mem_arena_t ssdv_read_file(FILE* f);

size_t ssdv_memcpy_packet(ssdv_mem_arena_t *src, ssdv_mem_arena_t *dest, size_t read_offset, size_t pkt_length);

ssdv_mem_arena_t ssdv_dec_buf(ssdv_mem_arena_t *src, ssdv_t *ssdv);
ssdv_mem_arena_t ssdv_enc_buf(ssdv_mem_arena_t *src, ssdv_t *ssdv);

int ssdv_dec_file(FILE *fin, FILE *fout, ssdv_t *ssdv);
int ssdv_enc_file(FILE *fin, FILE *fout, ssdv_t *ssdv);

