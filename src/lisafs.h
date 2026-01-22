//  lisafs.h
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#ifndef __LISAFS__H__
#define __LISAFS__H__

#include <stdio.h>

#include "lisafs_defines.h"


// MARK: - Types and Structures

typedef int16_t lisafs_boolean;
typedef int16_t lisafs_integer;
typedef int32_t lisafs_longint;


/*!
    Lisa uses 64-bit unique IDs. Someone must have worked with PR1ME or
    Apollo before working on Lisa…
 */
struct lisafs_uid {
    lisafs_longint a, b;
} LISAFS_PACKED;
typedef struct lisafs_uid lisafs_uid;


/*!
    Every 536-byte block on a Lisa volume is divided between a 24 bytes
    header (called a page label) and 512 bytes of data (called a page).
    This helps both I/O performance (since the current block's header
    says where the next block is on the device) and recoverability
    (since a file may be intact even if its catalog entry is corrupt).

    A microfloppy doesn't actually store the full 24-byte header, it's
    instead stored packed in the tag and expanded by the driver on read.
 */
struct lisafs_pagelabel {
    lisafs_integer  version;
    lisafs_integer  flags;
    lisafs_integer  fileid;
    lisafs_integer  dataused;
    lisafs_longint  abspage;
    lisafs_longint  relpage;
    lisafs_longint  fwdlink;
    lisafs_longint  bkwdlink;
} LISAFS_PACKED;
typedef struct lisafs_pagelabel lisafs_pagelabel;

/*! The data portion of a page on a Lisa device. */
typedef uint8_t lisafs_page[512];

/*!
 A microfloppy has an 8-word header at its start with some information
 about the loader at the front of the disk. It starts with an actual JMP
 instruction so the boot ROM can load side 0 track 0 sector 0 and just
 jump to its first word to boot from a microfloppy. The "true" block 0
 of the disk follows any such loader.
 */
struct lisafs_mf_loader_loader_header {
    lisafs_integer  jmp;            //< JMP instruction to skip header
    lisafs_integer  boot_id;
    lisafs_integer  ldr_version;
    lisafs_integer  self_descr[4];
} LISFS_PACKED;
struct lisafs_mf_loader_loader_header lisafs_mf_loader_loader_header;


/*!
    A Lisa filesystem image.
 */
struct lisafs_image {
    FILE    * LISAFS_NONNULL file;  //!< The underying stdio FILE.
    size_t  size;                   //!< The size of the image in bytes.
};
typedef struct lisafs_image lisafs_image;


// MARK: - Functions


/*!
    Read both the content and optionally the data of page n from the
    given Lisa disk image. A hard disk image will have full pages, a
    microfloppy image will have only page data.
 */
int lisafs_read_page(FILE * LISAFS_NONNULL image,
                     size_t n,
                     lisafs_pagelabel * LISAFS_NULLABLE label,
                     lisafs_page LISAFS_NONNULL page);



#endif /* __LISAFS__H__ */
