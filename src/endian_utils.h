//  endian_utils.h
//  Part of lisafs.
//
//  Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#ifndef __ENDIAN_UTILS__H__
#define __ENDIAN_UTILS__H__

#include <stdint.h>


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


#endif /* __ENDIAN_UTILS__H__ */
