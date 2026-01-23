//  lisafs.h
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#ifndef __LISAFS__H__
#define __LISAFS__H__

#include "lisafs_defines.h"

#include <stdlib.h>


// MARK: - Types and Structures

typedef int16_t lisafs_boolean;
typedef int16_t lisafs_integer;
typedef int32_t lisafs_longint;

typedef uint8_t lisafs_tag[24];
typedef uint8_t lisafs_block[512];


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
    A microfloppy has a header in its first block with some information
    about the loader it contains. It starts with an actual JMP
    instruction so the boot ROM can load that first block and just jump
    to its first word to boot from it.
 */
struct lisafs_mf_loader_loader_header {
    lisafs_longint  jmp;            //< JMP instruction to skip header
    lisafs_integer  boot_id;        //< should be 0xAAAA
    lisafs_integer  ldr_version;    //< should be 0x0850
    lisafs_integer  globalsize;     //< amount of global data for loader
    lisafs_integer  codesize;       //< size of the loader
    lisafs_integer  pc_offset;      //< offset from loader base to main entry
    lisafs_integer  fs_block0;      //< block address of MDDF
} LISFS_PACKED;
typedef struct lisafs_mf_loader_loader_header lisafs_mf_loader_loader_header;


/*!
    A Lisa filesystem image. The contents are private.
 */
struct lisafs_image;
typedef struct lisafs_image lisafs_image;


// MARK: - Functions

/*!
    Open and return the Lisa filesystem image at the given path.
 */
lisafs_image * _Nullable lisafs_image_open(const char * _Nonnull const path);

/*!
    Close the given Lisa filesystem image.
 */
int lisafs_image_close(lisafs_image * _Nullable image);

/*!
    Read both the raw data and tag bytes of physical block n from the
    given open Lisa disk image.
 */
int lisafs_image_read_block(lisafs_image * _Nonnull image,
                            size_t n,
                            lisafs_block _Nonnull block,
                            lisafs_tag _Nonnull tag);

/*!
    Read both the data and label of page n from the given Lisa disk
    image. This takes into account things like the disk (not image)
    header, since page 0 almost certainly isn't physical block 0.
 */
int lisafs_image_read_page(lisafs_image * _Nonnull image,
                           size_t n,
                           lisafs_page _Nonnull page,
                           lisafs_pagelabel * _Nonnull label);

#endif /* __LISAFS__H__ */
