//  lisafs.c
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

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

    // Cached from the image.
    lisafs_s_entry * _Nullable s_files;

    // Cached from the image.
    lisafs_directory directory;
};


time_t lisafs_timestamp_to_time_t(lisafs_timestamp timestamp)
{
    // The UNIX epoch begins 2177452800 after the Lisa epoch.
    const time_t offset = 2177452800;
    return ((time_t)timestamp) - offset;
}

const char * _Nullable lisafs_filetype_string(lisafs_filetype t)
{
    switch (t) {
        case undefined:     return "undefined";
        case MDDFfile:      return "mddf";
        case rootcat:       return "rootcat";
        case freelist:      return "freelist";
        case badblocks:     return "badblocks";
        case sysdata:       return "sysdata";
        case spool:         return "spool";
        case exec:          return "exec";
        case userdir:       return "userdir";
        case pipe:          return "pipe";
        case bootfile:      return "bootfile";
        case swapdata:      return "swapdata";
        case swapcode:      return "swapcode";
        case ramap:         return "ramap";
        case userfile:      return "userfile";
        case killedobject:  return "killedobject";
        case tempfile:      return "tempfile";
        default: {
            static char buf[32];
            snprintf(buf, 32, "unknown (%d)", (int)t);
            return buf;
        } break;
    }
}


int lisafs_read_header(lisafs_image * _Nonnull image)
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


int lisafs_read_mddf(lisafs_image * _Nonnull image)
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


int lisafs_cache_s_files(lisafs_image * _Nonnull image)
{
    const lisafs_paddr s_files_start = image->mddf.slist_addr;
    const lisafs_integer slist_block_count = image->mddf.slist_block_count;

    // The s-list itself is cached rather than the blocks containing it.

    const lisafs_integer slist_packing = image->mddf.slist_packing;
    const lisafs_integer slist_max_entries = slist_packing * slist_block_count;
    image->s_files = calloc(sizeof(lisafs_s_entry), slist_max_entries);
    if (image->s_files == NULL) {
        errno = ENOMEM;
        goto error;
    }

    // Go through each block, copying and converting all the s-list
    // entries that it contains.

    for (int16_t b = 0; b < slist_block_count; b++) {
        lisafs_page page = {0};

        int read_err = lisafs_read_page(image, s_files_start + b, &page);
        if (read_err == -1) goto error;

        lisafs_s_entry *slist_page_entries = (lisafs_s_entry *)page.data;
        for (int i = 0; i < slist_packing; i++) {
            int entry_idx = slist_packing * b + i;
            lisafs_s_entry *entry = &image->s_files[entry_idx];
            lisafs_s_entry *raw_entry = &slist_page_entries[i];

            entry->hintaddr = swap32(raw_entry->hintaddr);
            entry->fileaddr = swap32(raw_entry->fileaddr);
            entry->filesize = swap32(raw_entry->filesize);
            entry->version  = swap16(raw_entry->version);
        }
    }

    // Now set up entries for the always-present special files.

    image->s_files[LISAFS_MDDF_SNUM].hintaddr = 0;
    image->s_files[LISAFS_MDDF_SNUM].fileaddr = image->mddf.MDDFaddr;
    image->s_files[LISAFS_MDDF_SNUM].filesize = image->mddf.MDDFsize;
    image->s_files[LISAFS_MDDF_SNUM].version = 0;

    image->s_files[LISAFS_BITMAP_SNUM].hintaddr = 0;
    image->s_files[LISAFS_BITMAP_SNUM].fileaddr = image->mddf.bitmap_addr;
    image->s_files[LISAFS_BITMAP_SNUM].filesize = image->mddf.bitmap_pages * image->mddf.datasize;
    image->s_files[LISAFS_BITMAP_SNUM].version = 0;

    image->s_files[LISAFS_SLIST_SNUM].hintaddr = 0;
    image->s_files[LISAFS_SLIST_SNUM].fileaddr = image->mddf.slist_addr;
    image->s_files[LISAFS_SLIST_SNUM].filesize = image->mddf.slist_block_count * image->mddf.datasize;
    image->s_files[LISAFS_SLIST_SNUM].version = 0;

    image->s_files[LISAFS_ROOTDIR_SNUM].hintaddr = 0;
    image->s_files[LISAFS_ROOTDIR_SNUM].fileaddr = image->mddf.root_page;
    image->s_files[LISAFS_ROOTDIR_SNUM].filesize = image->mddf.tree_depth * image->mddf.datasize * 4;
    image->s_files[LISAFS_ROOTDIR_SNUM].version = 0;

    return 0;

error:
    return -1;
}


struct lisafs_btree_entry {
    lisafs_directory_entry entry;
    struct lisafs_btree_entry * _Nullable left;
    struct lisafs_btree_entry * _Nullable right;
};
typedef struct lisafs_btree_entry lisafs_btree_entry;


int lisafs_cache_directory(lisafs_image * _Nonnull image)
{
    // Get the first two B-tree pages of the catalog file.

    uint8_t btpage[2048];

    lisafs_paddr paddr = image->mddf.root_page;
    for (int i = 0; i < 4; i++) {
        lisafs_page page;
        int read_err = lisafs_read_page(image, paddr, &page);
        if (read_err == -1) goto error;

        memcpy(&btpage[i * 512], page.data, 512);

        paddr = page.label.fwdlink;
    }

    // Get the node description from it.

    lisafs_nodedesc root_node;
    lisafs_nodedesc *raw_root_node = (lisafs_nodedesc *) &btpage[2048 - 12];
    root_node.nkeys = swap16(raw_root_node->nkeys);
    root_node.prior = swap32(raw_root_node->prior);
    root_node.next  = swap32(raw_root_node->next);
    root_node.kind  = raw_root_node->kind;
    root_node.cksum = raw_root_node->cksum;

    fprintf(stdout, "nkeys:\t" "%hd" "\n", root_node.nkeys);
    fprintf(stdout, "prior:\t" "%d"  "\n", root_node.prior);
    fprintf(stdout, "next:\t"  "%d"  "\n", root_node.next);
    fprintf(stdout, "kind:\t"  "%s"  "\n", ((root_node.kind == leaf) ? "leaf" : "nonleaf"));

    // Now go through its entries and compose the real tree.

    lisafs_paddr pg;
    int offset;
    if (root_node.kind == nonleaf) {
        lisafs_paddr *raw_pg = (lisafs_paddr *)&btpage[0];
        pg = swap32(*raw_pg);
        offset = 4;
        fprintf(stdout, "pg:\t" "%d" "\n", pg);
    } else {
        pg = -1;
        offset = 0;
    }
    lisafs_directory_entry *raw_entry = (lisafs_directory_entry *)&btpage[offset];
    lisafs_directory_entry entry;

    memcpy(&entry.header_only.key, &raw_entry->header_only.key, 36);
    entry.header_only.etype      =  raw_entry->header_only.etype;
    entry.header_only.etype_pad  =  raw_entry->header_only.etype_pad;

    fprintf(stdout, "entry 0 type:\t");
    switch (entry.header_only.etype) {
        case emptyentry:  fprintf(stdout, "empty"); break;
        case direntry:    fprintf(stdout, "directory"); break;
        case linkentry:   fprintf(stdout, "link"); break;
        case fileentry:   fprintf(stdout, "file"); break;
        case pipeentry:   fprintf(stdout, "pipe"); break;
        case ecentry:     fprintf(stdout, "ec"); break;
        case killedentry: fprintf(stdout, "killed"); break;
        case removed:     fprintf(stdout, "removed)"); break;
        case threadentry: fprintf(stdout, "thread"); break;
    }
    fprintf(stdout, "\n");

    return 0;

error:
    return -1;
}


lisafs_image * _Nullable lisafs_open(const char * _Nonnull const path)
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

    int header_err = lisafs_read_header(image);
    if (header_err) goto error;

    // Read and validate the MDDF.

    int mddf_err = lisafs_read_mddf(image);
    if (mddf_err) goto error;

    // Read and validate the S-files.

    int s_files_err = lisafs_cache_s_files(image);
    if (s_files_err) goto error;

    // Read and validate the directory (catalog B-Tree).

    int directory_err = lisafs_cache_directory(image);
    if (directory_err) goto error;

    return image;

error:
    lisafs_close(image);
    return NULL;
}

int lisafs_close(lisafs_image * _Nullable image)
{
    if (image == NULL) return 0;

    int savederrno = errno; // don't let the act of saving destroy errno

    free(image->s_files);
    image->s_files = NULL;

    image_dc42_close(image->image);
    image->image = NULL;

    free(image);

    errno = savederrno;

    return 0;
}

void * _Nonnull lisafs_get_raw_image(lisafs_image * _Nonnull image)
{
    assert(image->image != NULL);

    return image->image;
}

void * _Nonnull lisafs_get_loader_header(lisafs_image * _Nonnull image)
{
    return &image->header;
}

lisafs_mddf * _Nonnull lisafs_get_mddf(lisafs_image * _Nonnull image)
{
    return &image->mddf;
}

const char * _Nonnull lisafs_get_volname(lisafs_image * _Nonnull image)
{
    return image->volname;
}

const char * _Nonnull lisafs_get_password(lisafs_image * _Nonnull image)
{
    return image->password;
}

int lisafs_read_block(lisafs_image * _Nonnull image,
                      lisafs_baddr n,
                      lisafs_block _Nonnull block,
                      lisafs_tag _Nonnull tag)
{
    assert(image->image != NULL);

    return image_dc42_read_block(image->image, n, block, tag);
}

int lisafs_read_page(lisafs_image * _Nonnull image,
                     lisafs_paddr n,
                     lisafs_page * _Nonnull page)
{
    assert(image->image != NULL);

    const lisafs_baddr real_n = image->block0 + n;

    uint8_t tag[12];
    int read_err = image_dc42_read_block(image->image, real_n, page->data, tag);
    if (read_err == -1) return -1;

    // Expand tag into label.

    // Since the Sony microfloppy doesn't have enough space for the full
    // 24-byte page label, that information is compressed cleverly in a
    // way that takes advantage of the fact that it will never need
    // 32-bit relative page addresses and that the absolute page address
    // doesn't need to be stored. The exact algorithm is derivable from
    // the FINISH_READ function in the -OS-SOURCE/SONYASM.TEXT source
    // file.

    // The first few values are encoded directly.

    int16_t tag_version = *((int16_t *) &tag[0]);
    int16_t tag_flags   = *((int16_t *) &tag[2]);
    int16_t tag_fileid  = *((int16_t *) &tag[4]);
    int16_t tag_relpage = *((int16_t *) &tag[6]);
    int16_t tag_fwdldu  = *((int16_t *) &tag[8]);
    int16_t tag_bkwdldu = *((int16_t *) &tag[10]);

    // The data used and backwards/forwards link fields are encoded
    // cleverly, since a microfloppy is of limited size; this means a
    // file can only be so large, and the relative links can only reach
    // so far. Thus the data used value is kept in the high bits of the
    // link fields, while the link values are kept in the low bits of
    // the links, and if the link value is 0x07ff it's expanded to
    // 0xffffffff.

#define LISAFS_EXTLINK(l) (((l & 0x07ff) != 0x7ff) ? (l & 0x07ff) : 0xffffffff)
    int16_t fwdldu      = swap16(tag_fwdldu);
    int16_t bkwdldu     = swap16(tag_bkwdldu);
    int16_t dataused    = (((bkwdldu & 0xf800) >> 5) + (fwdldu & 0xf800)) >> 6;

    page->label.version  = swap16(tag_version);
    page->label.flags    = swap16(tag_flags);
    page->label.fileid   = swap16(tag_fileid);
    page->label.abspage  = real_n;
    page->label.relpage  = swap16(tag_relpage); // 16-to-32 expansion here

    page->label.dataused = dataused;
    page->label.fwdlink  = LISAFS_EXTLINK(fwdldu);
    page->label.bkwdlink = LISAFS_EXTLINK(bkwdldu);
#undef LISAFS_EXTLINK

    return 0;
}

int lisafs_get_sfile_info(lisafs_image * _Nonnull image,
                          lisafs_fileid file,
                          lisafs_s_entry * _Nonnull entry)
{
    assert(image->image != NULL);
    assert(image->s_files != NULL);

    if ((file < 0) || (file >= image->mddf.empty_file)) {
        errno = EINVAL;
        goto error;
    }

    entry->hintaddr = image->s_files[file].hintaddr;
    entry->fileaddr = image->s_files[file].fileaddr;
    entry->filesize = image->s_files[file].filesize;
    entry->version  = image->s_files[file].version;

    return 0;

error:
    return -1;
}

lisafs_longint lisafs_get_sfile_size(lisafs_image * _Nonnull image,
                                     lisafs_fileid file)
{
    assert(image->image != NULL);

    // Find the info for the sfile.

    lisafs_s_entry entry;
    int entry_err = lisafs_get_sfile_info(image, file, &entry);
    if (entry_err == -1) goto error;

    return entry.filesize;

error:
    return -1;
}

int lisafs_read_sfile_hints(lisafs_image * _Nonnull image,
                            lisafs_fileid file,
                            lisafs_hentry * _Nonnull hints)
{
    assert(image->image != NULL);

    // Don't support special S-files.

    if (file < LISAFS_FIRSTUSER_SF) {
        errno = EINVAL;
        goto error;
    }

    // Find the info for the sfile.

    lisafs_s_entry entry;
    int entry_err = lisafs_get_sfile_info(image, file, &entry);
    if (entry_err == -1) goto error;

    // Every non-special sfile will have hints.

    if (entry.hintaddr) {
        lisafs_page file_leader;

        int hints_err = lisafs_read_page(image, entry.hintaddr, &file_leader);
        if (hints_err == -1) goto error;

        lisafs_hentry *raw_hints = (lisafs_hentry *)&file_leader.data[image->mddf.hentry_offset];
        memcpy(hints->name,      raw_hints->name, 33);
        hints->name_pad        = raw_hints->name_pad;
        hints->UID.a           = swap32(raw_hints->UID.a);
        hints->UID.b           = swap32(raw_hints->UID.b);
        hints->version         = swap16(raw_hints->version);
        hints->ftype           = raw_hints->ftype;
        hints->ftype_pad       = raw_hints->ftype_pad;
        hints->date_created    = swap32(raw_hints->date_created);
        hints->date_accessed   = swap32(raw_hints->date_accessed);
        hints->date_modified   = swap32(raw_hints->date_modified);
        hints->date_backup     = swap32(raw_hints->date_backup);
        hints->date_scavenged  = swap32(raw_hints->date_scavenged);
        hints->machine_id      = swap32(raw_hints->machine_id);
        hints->killed          = raw_hints->killed;
        hints->safety_on       = raw_hints->safety_on;
        hints->protected       = raw_hints->protected;
        hints->master          = raw_hints->master;
        hints->close_by_OS     = raw_hints->close_by_OS;
        hints->file_open       = raw_hints->file_open;
        hints->result_scavenge = swap16(raw_hints->result_scavenge);
        hints->unusedi1        = swap16(raw_hints->unusedi1);
        hints->system_type     = swap16(raw_hints->system_type);
        hints->user_type       = swap16(raw_hints->user_type);
        hints->user_subtype    = swap16(raw_hints->user_subtype);
        hints->build_info.release_number      = swap16(raw_hints->build_info.release_number);
        hints->build_info.build_number        = swap16(raw_hints->build_info.build_number);
        hints->build_info.compatibility_level = swap16(raw_hints->build_info.compatibility_level);
        hints->build_info.revision_level      = swap16(raw_hints->build_info.revision_level);
        hints->file_portion    = swap16(raw_hints->file_portion);
        memcpy(hints->password,  raw_hints->password, 9);
        hints->password_pad[0] = raw_hints->password_pad[0];
        hints->password_pad[1] = raw_hints->password_pad[1];
        hints->password_pad[2] = raw_hints->password_pad[2];
        hints->parentID        = swap16(raw_hints->parentID);
        hints->fsOverhead      = swap16(raw_hints->fsOverhead);
    } else {
        errno = ENOENT;
        goto error;
    }

    return 0;

error:
    return -1;
}

/*! Get the (complete) file map and its size for the given sfile. */
lisafs_mapentry * _Nullable lisafs_copy_sfile_map(
                                lisafs_image * _Nonnull image,
                                lisafs_fileid file,
                                lisafs_integer * _Nonnull count)
{
    lisafs_mapentry *map = NULL;
    lisafs_integer map_count = 0;

    // Don't support special S-files.

    if (file < LISAFS_FIRSTUSER_SF) {
        errno = EINVAL;
        goto error;
    }

    // Find the info for the S-file.

    lisafs_s_entry entry;
    int entry_err = lisafs_get_sfile_info(image, file, &entry);
    if (entry_err == -1) goto error;

    // If this file has no hint address, it has no leader and therefore
    // no file map, which means it's almost certainly a special file.

    if (entry.hintaddr <= 0) {
        errno = EINVAL;
        goto error;
    }

    // If it's an old (1.0 or earlier) filesystem, it uses a large
    // filemap in the second page of the "file leader." If it's a
    // new(er) filesystem, it uses a small filemap at an offset within
    // the file leader, with the hints at the front. (The first page of
    // the file leader is found at the "hint address" in its S-file
    // entry, and its subsequent pages are only locatable via the label
    // forward chain.)

    bool oldfs = (image->mddf.fsversion <= release1);

    // Get the raw filemap and convert it to the one that gets returned.

    lisafs_filemap filemap;
    lisafs_smallmap smallmap;

    // Fill in the real filemap, if one is used.

    if (oldfs) {
        lisafs_page file_leader;
        int leader0_err = lisafs_read_page(image, entry.hintaddr, &file_leader);
        if (leader0_err == -1) goto error;

        // Read the next page of file leader, if there is one.

        if (file_leader.label.fwdlink != -1) {
            int leader1_err = lisafs_read_page(image, file_leader.label.fwdlink, &file_leader);
            if (leader1_err == -1) goto error;
        } else {
            // This file is broken, in that it has a file leader but no
            // second page on which to keep the filemap.

            errno = ENODATA;
            goto error;
        }

        lisafs_filemap *raw_filemap = (lisafs_filemap *)&file_leader.data[image->mddf.map_offset];

        filemap.size        = swap32(raw_filemap->size);
        filemap.max_entries = swap16(raw_filemap->max_entries);
        filemap.ecount      = swap16(raw_filemap->ecount);

        // TODO: Support multiple pages of filemap.

        if (filemap.ecount > 84) {
            errno = ENOTSUP;
            goto error;
        }

        map_count = filemap.ecount;
        map = calloc(sizeof(lisafs_mapentry), map_count);
        if (map == NULL) {
            errno = ENOMEM;
            goto error;
        }

        for (int i = 0; i < smallmap.ecount; i++) {
            map[i].address = swap32(raw_filemap->map[i].address);
            map[i].cpages  = swap16(raw_filemap->map[i].cpages);
        }
    } else {
        lisafs_page file_leader;
        int leader_err = lisafs_read_page(image, entry.hintaddr, &file_leader);
        if (leader_err == -1) goto error;

        lisafs_smallmap *raw_smallmap = (lisafs_smallmap *)&file_leader.data[image->mddf.smallmap_offset];

        smallmap.size        = swap32(raw_smallmap->size);
        smallmap.max_entries = swap16(raw_smallmap->max_entries);
        smallmap.ecount      = swap16(raw_smallmap->ecount);

        // TODO: Support multiple pages of smallmap.

        if (smallmap.ecount > 10) {
            errno = ENOTSUP;
            goto error;
        }

        map_count = smallmap.ecount;
        map = calloc(sizeof(lisafs_mapentry), map_count);
        if (map == NULL) {
            errno = ENOMEM;
            goto error;
        }

        for (int i = 0; i < smallmap.ecount; i++) {
            map[i].address = swap32(raw_smallmap->map[i].address);
            map[i].cpages  = swap16(raw_smallmap->map[i].cpages);
        }
    }

    *count = map_count;
    return map;

error:
    free(map);
    map = NULL;

    return NULL;
}


int lisafs_read_sfile(lisafs_image * _Nonnull image,
                      lisafs_fileid file,
                      void * _Nonnull buf,
                      size_t buf_size)
{
    assert(image->image != NULL);

    lisafs_mapentry *map = NULL;    // need to clean up at exit

    // Don't support special S-files.

    if (file < LISAFS_FIRSTUSER_SF) {
        errno = EINVAL;
        goto error;
    }

    lisafs_integer map_count;
    map = lisafs_copy_sfile_map(image, file, &map_count);
    if (map == NULL) goto error;

    // Traverse all of the entries in the file map, reading the file
    // contents into the buffer given to us by the user.

    void *current = buf;
    size_t remaining = buf_size;

    for (int i = 0; i < map_count; i++) {
        lisafs_mapentry *entry = &map[i];
        lisafs_page page;

        for (int j = 0; j < entry->cpages; j++) {
            int read_err = lisafs_read_page(image, entry->address, &page);
            if (read_err == -1) goto error;

            size_t to_copy = remaining > 512 ? 512 : remaining;
            memcpy(current, page.data, to_copy);
            remaining -= to_copy;
        }
    }

    free(map);
    map = NULL;

    return 0;

error:
    free(map);
    map = NULL;

    return -1;
}
