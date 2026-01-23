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


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
inline
static int16_t swap16(int16_t value) {
    return (  ((value & 0x00FF) << 8)
            | ((value & 0xFF00) >> 8));
}

inline
static uint16_t swapu16(uint16_t value) {
    return (  ((value & 0x00FF) << 8)
            | ((value & 0xFF00) >> 8));
}

inline
static int32_t swap32(int32_t value) {
    return (  ((value & 0x000000FF) << 24)
            | ((value & 0x0000FF00) <<  8)
            | ((value & 0x00FF0000) >>  8)
            | ((value & 0xFF000000) >> 24));
}

inline
static uint32_t swapu32(uint32_t value) {
    return (  ((value & 0x000000FF) << 24)
            | ((value & 0x0000FF00) <<  8)
            | ((value & 0x00FF0000) >>  8)
            | ((value & 0xFF000000) >> 24));
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
inline
inline
static int16_t swap16(int16_t value) {
    return value;
}

inline
static uint16_t swapu16(uint16_t value) {
    return value;
}

inline
static int32_t swap32(int32_t value) {
    return value;
}

inline
static uint32_t swapu32(uint32_t value) {
    return value;
}
#else
#error PDP-11 not supported
#endif


#endif /* __LISAFS_IO_UTILS__H__ */
