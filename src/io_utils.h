//  io_utils.h
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#ifndef __LISAFS_IO_UTILS__H__
#define __LISAFS_IO_UTILS__H__

#include "lisafs_defines.h"

#include <stdio.h>


LISAFS_EXTERN int read_int8(FILE * LISAFS_NONNULL f, int8_t * LISAFS_NONNULL value);
LISAFS_EXTERN int read_int16(FILE * LISAFS_NONNULL f, int16_t * LISAFS_NONNULL value);
LISAFS_EXTERN int read_int32(FILE * LISAFS_NONNULL f, int32_t * LISAFS_NONNULL value);

LISAFS_EXTERN int read_uint8(FILE * LISAFS_NONNULL f, uint8_t * LISAFS_NONNULL value);
LISAFS_EXTERN int read_uint16(FILE * LISAFS_NONNULL f, uint16_t * LISAFS_NONNULL value);
LISAFS_EXTERN int read_uint32(FILE * LISAFS_NONNULL f, uint32_t * LISAFS_NONNULL value);

LISAFS_EXTERN int read_pstring(FILE * LISAFS_NONNULL f, char * LISAFS_NONNULL buf, size_t len);


#endif /* __LISAFS_IO_UTILS__H__ */
