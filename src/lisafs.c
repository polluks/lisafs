//  lisafs.c
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#include "lisafs.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "image_dc42.h"
#include "io_utils.h"


struct lisafs_image {
    image_dc42 *image;
    lisafs_mf_loader_loader_header header;
    size_t block0;
};

/*
    lisafs_longint  jmp;            //< JMP instruction to skip header
    lisafs_integer  boot_id;        //< should be 0xAAAA
    lisafs_integer  ldr_version;    //< should be 0x0850
    lisafs_integer  globalsize;     //< amount of global data for loader
    lisafs_integer  codesize;       //< size of the loader
    lisafs_integer  pc_offset;      //< offset from loader base to main entry
    lisafs_integer  fs_block0;      //< block address of MDDF
*/

lisafs_image * _Nullable lisafs_image_open(const char * _Nonnull const path)
{
    lisafs_image *image = calloc(sizeof(lisafs_image), 1);
    if (image == NULL) {
        errno = ENOMEM;
        goto error;
    }

    // Open the underlying disk image.

    image->image = image_dc42_open(path);
    if (image == NULL) {
        errno = ENOMEM;
        goto error;
    }

    // Read and validate the microfloppy loader header.

    lisafs_block block0;
    lisafs_tag tag0;
    int read_err = image_dc42_read_block(image->image, 0, block0, tag0);
    if (read_err == -1) goto error;

    lisafs_mf_loader_loader_header *header = &image->header;
    lisafs_mf_loader_loader_header *raw_header = (lisafs_mf_loader_loader_header *)block0;

    header->jmp         = swap32(raw_header->jmp);
    header->boot_id     = swap16(raw_header->boot_id);
    header->ldr_version = swap16(raw_header->ldr_version);
    header->globalsize  = swap16(raw_header->globalsize);
    header->codesize    = swap16(raw_header->codesize);
    header->pc_offset   = swap16(raw_header->pc_offset);
    header->fs_block0   = swap16(raw_header->fs_block0);

    image->block0 = header->fs_block0;

    return image;

error:
    lisafs_image_close(image);
    return NULL;
}

int lisafs_image_close(lisafs_image * _Nullable image)
{
    if (image == NULL) return 0;

    int savederrno = errno; // don't let the act of saving destroy errno

    image_dc42_close(image->image);
    image->image = NULL;

    free(image);

    errno = savederrno;

    return 0;
}

/*!
    Read both the raw data and tag bytes of physical block n from the
    given open Lisa disk image.
 */
int lisafs_image_read_block(lisafs_image * _Nonnull image,
                            size_t n,
                            lisafs_block _Nonnull block,
                            lisafs_tag _Nonnull tag)
{
    assert(image->image != NULL);

    return image_dc42_read_block(image->image, n, block, tag);
}

/*!
    Read both the data and label of page n from the given Lisa disk
    image. This takes into account things like the disk (not image)
    header, since page 0 almost certainly isn't physical block 0.
 */
int lisafs_image_read_page(lisafs_image * _Nonnull image,
                           size_t n,
                           lisafs_page _Nonnull page,
                           lisafs_pagelabel * _Nonnull label)
{
    assert(image->image != NULL);

    size_t real_n = image->block0 + n;

    lisafs_tag tag;
    int read_err = image_dc42_read_block(image->image, real_n, page, tag);
    if (read_err == -1) return -1;

    // Expand tag into label.

    // TODO: Expand tag into label.

    // The tag doens't contain the absolute page, instead it expects the
    // driver to set that based on the block being read.
    label->abspage = real_n;

    return 0;
}
