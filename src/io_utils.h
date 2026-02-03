//  io_utils.h
//	Part of lisafs.
//
//	Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#ifndef __IO_UTILS__H__
#define __IO_UTILS__H__

#include <stdint.h>
#include <stdio.h>


int read_int8(FILE * _Nonnull f, int8_t * _Nonnull value);
int read_int16(FILE * _Nonnull f, int16_t * _Nonnull value);
int read_int32(FILE * _Nonnull f, int32_t * _Nonnull value);

int read_uint8(FILE * _Nonnull f, uint8_t * _Nonnull value);
int read_uint16(FILE * _Nonnull f, uint16_t * _Nonnull value);
int read_uint32(FILE * _Nonnull f, uint32_t * _Nonnull value);

int read_pstring(FILE * _Nonnull f, char * _Nonnull buf, size_t len);


#endif /* __LISAFS_IO_UTILS__H__ */
