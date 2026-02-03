//  image_dc42.c
//	Part of lisafs.
//
//	Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#include "image_dc42.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "endian_utils.h"
#include "io_utils.h"


const size_t image_dc42_header_size = 84;
const size_t image_dc42_block_size = 512;
const size_t image_dc42_tag_size = 12;


struct image_dc42 {
    image_dc42_header header;
    FILE * _Nullable file;
    char * _Nullable name;
};


image_dc42 * _Nullable image_dc42_open(const char * _Nonnull path)
{
    image_dc42 *image = calloc(sizeof(image_dc42), 1);
    if (image == NULL) {
        errno = ENOMEM;
        goto error;
    }

    // Open the file.

    image->file = fopen(path, "r");
    if (image->file == NULL) {
        goto error;
    }

    // Read and validate the image header.

    image_dc42_header *header = &image->header;

    if (read_pstring(image->file, header->volume_name, 64) == -1) goto error;
    if (read_uint32(image->file, &header->data_size) == -1) goto error;
    if (read_uint32(image->file, &header->tag_size) == -1) goto error;
    if (read_uint32(image->file, &header->data_checksum) == -1) goto error;
    if (read_uint32(image->file, &header->tag_checksum) == -1) goto error;
    if (read_uint8(image->file, (uint8_t *)&header->encoding) == -1) goto error;
    if (read_uint8(image->file, (uint8_t *)&header->format) == -1) goto error;
    if (read_uint16(image->file, &header->magic_number) == -1) goto error;

    if (header->magic_number != 0x0100) {
        errno = EFTYPE;
        goto error;
    }

    // Grab the image name in a more convenient format.

    image->name = calloc(sizeof(char), 64);
    if (image->name == NULL) goto error;
    strncpy(image->name, &header->volume_name[1], header->volume_name[0]);

    return image;

error:
    image_dc42_close(image);
    return NULL;
}

int image_dc42_close(image_dc42 * _Nullable image)
{
    if (image == NULL) return 0;

    int savederrno = errno; // don't let the act of saving destroy errno

    fclose(image->file);
    image->file = NULL;

    free(image->name);
    image->name = NULL;

    free(image);

    errno = savederrno;

    return 0;
}

image_dc42_header * _Nonnull image_dc42_get_header(image_dc42 * _Nonnull image)
{
    return &image->header;
}

const char * _Nonnull const image_dc42_get_name(image_dc42 * _Nonnull image)
{
    return image->name;
}

const char * _Nonnull image_dc42_get_encoding_name(image_dc42_encoding encoding)
{
    static char buf[32] = {0};
    switch (encoding) {
        case gcr_clv_ssdd: return "3.5in 400KB GCR";
        case gcr_clv_dsdd: return "3.5in 800KB GCR";
        case mfm_cav_dsdd: return "3.5in 720KB MFM";
        case mfm_cav_dshd: return "3.5in 1440KB MFM";
        default:
            snprintf(buf, 32, "Unknown (0x%02x)", encoding);
            break;
    }
    return buf;
}

/*!
    Get the byte offset of the start of the given block in the disk
    image.
 */
inline
static off_t image_dc42_offset_for_block(image_dc42 * _Nonnull image,
                                         size_t block)
{
    return image_dc42_header_size
         + (block * image_dc42_block_size);
}

/*!
    Get the byte offset of the start of the given block's tag in the
    disk image.
 */
inline
static off_t image_dc42_offset_for_tag(image_dc42 * _Nonnull image,
                                       size_t block)
{
    // An image can have no tags, so just treat those cases as all 0.

    if (image->header.tag_size == 0) return 0;

    return image_dc42_header_size
         + image->header.data_size
         + (block * image_dc42_tag_size);
}

int image_dc42_read_block(image_dc42 * _Nonnull image,
                          size_t n,
                          uint8_t * _Nonnull block,
                          uint8_t * _Nonnull tag)
{
    if (image->file == NULL) {
        errno = EBADF;
        return -1;
    }

    off_t block_off = image_dc42_offset_for_block(image, n);

    int seek_err = fseeko(image->file, block_off, SEEK_SET);
    if (seek_err == -1) {
        return -1;
    }

    size_t read_items = fread(block, image_dc42_block_size, 1, image->file);
    if (read_items != 1) {
        errno = ferror(image->file);
        return -1;
    }

    off_t tag_off = image_dc42_offset_for_tag(image, n);
    if (tag_off != 0) {
        int seek_err = fseeko(image->file, tag_off, SEEK_SET);
        if (seek_err == -1) {
            return -1;
        }

        size_t read_items = fread(tag, image_dc42_tag_size, 1, image->file);
        if (read_items != 1) {
            errno = ferror(image->file);
            return -1;
        }
    } else {
        memset(tag, 0, image_dc42_tag_size);
    }

    return 0;
}
