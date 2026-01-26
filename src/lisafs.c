//  lisafs.c
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#include "lisafs.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "image_dc42.h"
#include "io_utils.h"


struct lisafs_image {
    image_dc42 *image;
    lisafs_mf_loader_loader_header header;
    lisafs_mddf mddf;

    // Cached from header
    char volname[33];
    char password[33];
    lisafs_baddr block0;

    // Cached from MDDF
    lisafs_paddr slist_addr;
    lisafs_paddr root_page;
};


time_t lisafs_timestamp_to_time_t(lisafs_timestamp timestamp)
{
    // The UNIX epoch begins 2177452800 after the Lisa epoch.
    const time_t offset = 2177452800;
    return ((time_t)timestamp) - offset;
}


int lisafs_image_read_header(lisafs_image * _Nonnull image)
{
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

    return 0;

error:
    return -1;
}


const char * _Nonnull lisafs_fsversion_string(lisafs_fsversion version)
{
    static char buf[32];
    switch (version) {
        case release1: return "1.0";
        case release2: return "2.0";
        case release3: return "3.0";
        default:
            snprintf(buf, 32, "Unknown (%hd)", version);
            return buf;
    }
}


int lisafs_image_read_mddf(lisafs_image * _Nonnull image)
{
    lisafs_block mddf_block;
    lisafs_tag mddf_tag;
    int read_err = image_dc42_read_block(image->image, image->block0, mddf_block, mddf_tag);
    if (read_err == -1) goto error;

    lisafs_mddf *mddf = &image->mddf;
    lisafs_mddf *raw_mddf = (lisafs_mddf *)mddf_block;

    mddf->fsversion         = swap16(raw_mddf->fsversion);
    mddf->volid.a           = swap32(raw_mddf->volid.a);
    mddf->volid.b           = swap32(raw_mddf->volid.b);
    mddf->volnum            = swap16(raw_mddf->volnum);
    memcpy(mddf->volname, raw_mddf->volname, 33);
    mddf->padc1             = raw_mddf->padc1;
    memcpy(mddf->password, raw_mddf->password, 33);
    mddf->padc2             = raw_mddf->padc2;
    mddf->init_machine_id   = swap32(raw_mddf->init_machine_id);
    mddf->init_machine_id	= swap32(raw_mddf->init_machine_id);
    mddf->master_machine_id	= swap32(raw_mddf->master_machine_id);
    mddf->DT_created	    = swap32(raw_mddf->DT_created);
    mddf->DT_copy_created	= swap32(raw_mddf->DT_copy_created);
    mddf->DT_copied	        = swap32(raw_mddf->DT_copied);
    mddf->DT_scavenged	    = swap32(raw_mddf->DT_scavenged);
    mddf->copy_thread	    = swap32(raw_mddf->copy_thread);
    mddf->firstblock	    = swap32(raw_mddf->firstblock);
    mddf->lastblock 	    = swap32(raw_mddf->lastblock);
    mddf->lastfspage	    = swap32(raw_mddf->lastfspage);
    mddf->blockcount	    = swap32(raw_mddf->blockcount);
    mddf->blocksize	        = swap16(raw_mddf->blocksize);
    mddf->datasize	        = swap16(raw_mddf->datasize);
    mddf->cluster_size	    = swap16(raw_mddf->cluster_size);
    mddf->MDDFaddr	        = swap32(raw_mddf->MDDFaddr);
    mddf->MDDFsize	        = swap16(raw_mddf->MDDFsize);
    mddf->bitmap_addr	    = swap32(raw_mddf->bitmap_addr);
    mddf->bitmap_size	    = swap32(raw_mddf->bitmap_size);
    mddf->bitmap_bytes	    = swap16(raw_mddf->bitmap_bytes);
    mddf->bitmap_pages	    = swap16(raw_mddf->bitmap_pages);
    mddf->slist_addr	    = swap32(raw_mddf->slist_addr);
    mddf->slist_packing	    = swap16(raw_mddf->slist_packing);
    mddf->slist_block_count	= swap16(raw_mddf->slist_block_count);
    mddf->first_file	    = swap16(raw_mddf->first_file);
    mddf->empty_file	    = swap16(raw_mddf->empty_file);
    mddf->maxfiles	        = swap16(raw_mddf->maxfiles);
    mddf->hintsize	        = swap16(raw_mddf->hintsize);
    mddf->leader_offset	    = swap16(raw_mddf->leader_offset);
    mddf->leader_pages	    = swap16(raw_mddf->leader_pages);
    mddf->flabel_offset	    = swap16(raw_mddf->flabel_offset);
    mddf->unusedi1	        = swap16(raw_mddf->unusedi1);
    mddf->map_offset	    = swap16(raw_mddf->map_offset);
    mddf->map_size	        = swap16(raw_mddf->map_size);
    mddf->filecount	        = swap16(raw_mddf->filecount);
    mddf->freestart	        = swap32(raw_mddf->freestart);
    mddf->unusedl1	        = swap32(raw_mddf->unusedl1);
    mddf->freecount         = swap32(raw_mddf->freecount);
    mddf->rootsnum	        = swap16(raw_mddf->rootsnum);
    mddf->rootmaxentries	= swap16(raw_mddf->rootmaxentries);
    mddf->mountinfo         = swap16(raw_mddf->mountinfo);
    mddf->overmount_stamp.a = swap32(raw_mddf->overmount_stamp.a);
    mddf->overmount_stamp.b = swap32(raw_mddf->overmount_stamp.b);
    mddf->pmem_id	        = swap32(raw_mddf->pmem_id);
    for (int i = 0; i < 32; i++) {
        mddf->pmem[i]       = swap16(raw_mddf->pmem[i]);
    }
    mddf->vol_scavenged     = swap16(raw_mddf->vol_scavenged);
    mddf->tbt_copied        = swap16(raw_mddf->tbt_copied);
    mddf->smallmap_offset	= swap16(raw_mddf->smallmap_offset);
    mddf->hentry_offset	    = swap16(raw_mddf->hentry_offset);
    mddf->backup_volid.a    = swap32(raw_mddf->backup_volid.a);
    mddf->backup_volid.b    = swap32(raw_mddf->backup_volid.b);
    mddf->flabel_size	    = swap16(raw_mddf->flabel_size);
    mddf->fs_overhead	    = swap16(raw_mddf->fs_overhead);
    mddf->result_scavenge	= swap16(raw_mddf->result_scavenge);
    mddf->boot_code	        = swap16(raw_mddf->boot_code);
    mddf->boot_environ  	= swap16(raw_mddf->boot_environ);
    mddf->oem_id	        = swap32(raw_mddf->oem_id);
    mddf->root_page	        = swap32(raw_mddf->root_page);
    mddf->tree_depth	    = swap16(raw_mddf->tree_depth);
    mddf->node_id	        = swap16(raw_mddf->node_id);
    mddf->vol_seq_no	    = swap16(raw_mddf->vol_seq_no);
    mddf->vol_mounted       = swap16(raw_mddf->vol_mounted);

    size_t volname_len = mddf->volname[0] <= 32 ? mddf->volname[0] : 32;
    memset(image->volname, 0, 33);
    memcpy(image->volname, &mddf->volname[1], volname_len);

    size_t password_len = mddf->password[0] <= 32 ? mddf->password[0] : 32;
    memset(image->password, 0, 33);
    memcpy(image->password, &mddf->password[1], password_len);

    image->slist_addr = mddf->slist_addr;
    image->root_page = mddf->root_page;

    return 0;

error:
    return -1;
}


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

    int header_err = lisafs_image_read_header(image);
    if (header_err) goto error;

    // Read and validate the MDDF.

    int mddf_err = lisafs_image_read_mddf(image);
    if (mddf_err) goto error;

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

void * _Nonnull lisafs_image_get_raw_image(lisafs_image * _Nonnull image)
{
    assert(image->image != NULL);

    return image->image;
}

void * _Nonnull lisafs_image_get_loader_header(lisafs_image * _Nonnull image)
{
    return &image->header;
}

lisafs_mddf * _Nonnull lisafs_image_get_mddf(lisafs_image * _Nonnull image)
{
    return &image->mddf;
}

const char * _Nonnull lisafs_image_get_volname(lisafs_image * _Nonnull image)
{
    return image->volname;
}

const char * _Nonnull lisafs_image_get_password(lisafs_image * _Nonnull image)
{
    return image->password;
}

int lisafs_image_read_block(lisafs_image * _Nonnull image,
                            lisafs_baddr n,
                            lisafs_block _Nonnull block,
                            lisafs_tag _Nonnull tag)
{
    assert(image->image != NULL);

    return image_dc42_read_block(image->image, n, block, tag);
}

int lisafs_image_read_page(lisafs_image * _Nonnull image,
                           lisafs_paddr n,
                           lisafs_page _Nonnull page,
                           lisafs_pagelabel * _Nonnull label)
{
    assert(image->image != NULL);

    lisafs_baddr real_n = image->block0 + n;

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
