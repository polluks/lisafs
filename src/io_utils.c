//  io_utils.c
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#include "io_utils.h"

#include <errno.h>


int read_int8(FILE * LISAFS_NONNULL f, int8_t * LISAFS_NONNULL value)
{
    size_t items_read = fread(value, sizeof(int8_t), 1, f);
    if (items_read != 1) {
        if (feof(f)) {
            errno = ENODATA;
            return -1;
        } else {
            errno = ferror(f);
            return -1;
        }
    } else {
        return 0;
    }
}

int read_int16(FILE * f, int16_t * LISAFS_NONNULL value)
{
    int16_t temp_value;
    size_t items_read = fread(&temp_value, sizeof(int16_t), 1, f);
    if (items_read != 1) {
        if (feof(f)) {
            errno = ENODATA;
            return -1;
        } else {
            errno = ferror(f);
            return -1;
        }
    } else {
        *value = swap16(temp_value);
        return 0;
    }
}

int read_int32(FILE * f, int32_t * LISAFS_NONNULL value)
{
    int32_t temp_value;
    size_t items_read = fread(&temp_value, sizeof(int32_t), 1, f);
    if (items_read != 1) {
        if (feof(f)) {
            errno = ENODATA;
            return -1;
        } else {
            errno = ferror(f);
            return -1;
        }
    } else {
        *value = swap32(temp_value);
        return 0;
    }
}


int read_uint8(FILE *f, uint8_t * LISAFS_NONNULL value)
{
    size_t items_read = fread(value, sizeof(uint8_t), 1, f);
    if (items_read != 1) {
        if (feof(f)) {
            errno = ENODATA;
            return -1;
        } else {
            errno = ferror(f);
            return -1;
        }
    } else {
        return 0;
    }
}

int read_uint16(FILE *f, uint16_t * LISAFS_NONNULL value)
{
    uint16_t temp_value;
    size_t items_read = fread(&temp_value, sizeof(uint16_t), 1, f);
    if (items_read != 1) {
        if (feof(f)) {
            errno = ENODATA;
            return -1;
        } else {
            errno = ferror(f);
            return -1;
        }
    } else {
        *value = swapu16(temp_value);
        return 0;
    }
}

int read_uint32(FILE *f, uint32_t * LISAFS_NONNULL value)
{
    uint32_t temp_value;
    size_t items_read = fread(&temp_value, sizeof(uint32_t), 1, f);
    if (items_read != 1) {
        if (feof(f)) {
            errno = ENODATA;
            return -1;
        } else {
            errno = ferror(f);
            return -1;
        }
    } else {
        *value = swapu32(temp_value);
        return 0;
    }
}


int read_pstring(FILE * LISAFS_NONNULL f, char * LISAFS_NONNULL buf, size_t len)
{
    size_t items_read = fread(buf, sizeof(char), len, f);
    if (items_read != len) {
        if (feof(f)) {
            errno = ENODATA;
            return -1;
        } else {
            errno = ferror(f);
            return -1;
        }
    } else {
        return 0;
    }
}
