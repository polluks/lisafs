//  image_dc42.c
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#include "image_dc42.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io_utils.h"


const size_t image_dc42_header_size = 84;
const size_t image_dc42_block_size = 512;
const size_t image_dc42_tag_size = 12;


struct image_dc42 {
    image_dc42_header header;
    FILE * LISAFS_NULLABLE file;
    char * LISAFS_NULLABLE name;
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

/*
    NOTE: We're not currently considering interleave under the
          assumption that interleave information is recorded in a disk
          image so it can be accurately written, but that the blocks in
          the image are stored in logical rather than physical order.
 */

inline
static off_t image_dc42_offset_for_block(image_dc42 * _Nonnull image,
                                         size_t block)
{
    return image_dc42_header_size
         + (block * image_dc42_block_size);
}

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
    assert(image->file != NULL);

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
