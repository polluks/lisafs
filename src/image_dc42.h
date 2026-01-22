//  image_dc42.h
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#ifndef __IMAGE_DC42__H__
#define __IMAGE_DC42__H__

#include "lisafs_defines.h"

#include <stdio.h>
#include <stdlib.h>


/*!
    Known encodings for a DiskCopy 4.2 microfloppy image.
 */
enum image_dc42_encoding: uint8_t {
    gcr_clv_ssdd = 0x00,    //< 400KB
    gcr_clv_dsdd = 0x01,    //< 800KB
    mfm_cav_dsdd = 0x02,    //< 720KB
    mfm_cav_dshd = 0x03,    //< 1440KB
};
typedef enum image_dc42_encoding image_dc42_encoding;

/*!
    Known formats for a DiskCopy 4.2 microfloppy image.
 */
union image_dc42_format {
    uint8_t gcr;
    uint8_t mfm;
};
typedef union image_dc42_format image_dc42_format;

/*!
    A DiskCopy 4.2 image starts with an 84-byte header.
 */
struct image_dc42_header {
    char volume_name[64];           //< Length-prefixed volume name
    uint32_t data_size;
    uint32_t tag_size;
    uint32_t data_checksum;
    uint32_t tag_checksum;
    image_dc42_encoding encoding;
    image_dc42_format format;
    uint16_t magic_number;          //< Should be 0x0100
};
typedef struct image_dc42_header image_dc42_header;


/*!
    A DiskCopy 4.2 image. While the image is open, its file is valid.
 */
struct image_dc42 {
    image_dc42_header header;
    FILE * LISAFS_NULLABLE file;
    char * LISAFS_NULLABLE name;
};
typedef struct image_dc42 image_dc42;


/*! Open a DiskCopy 4.2 image for use. */
int image_dc42_open(const char * LISAFS_NONNULL path,
                    image_dc42 * LISAFS_NONNULL image);

/*! Close an open DiskCopy 4.2 image. */
int image_dc42_close(image_dc42 * LISAFS_NONNULL image);

/*!
    Read the block at the given index from an open DiskCopy 4.2 image
    into the given buffer.
 */
int image_dc42_read_block(image_dc42 * LISAFS_NONNULL image,
                          size_t block,
                          uint8_t * LISAFS_NONNULL buf);

/*!
    Read the tags for the block at the given index from an open
    DiskCopy 4.2 image into the given buffer.
 */
int image_dc42_read_tag(image_dc42 * LISAFS_NONNULL image,
                        size_t block,
                        uint8_t * LISAFS_NONNULL buf);

#endif /* __IMAGE_DC42__H__ */
