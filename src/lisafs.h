//  lisafs.h
//	Part of LisaFilesystem.
//
//	Copyright © 2026 Base Hit Ventures, LLC. All rights reserved.

#ifndef __LISAFS__H__
#define __LISAFS__H__

#include "lisafs_defines.h"

#include <stdlib.h>
#include <time.h>


// MARK: - Types and Structures

typedef int16_t lisafs_boolean;
typedef int16_t lisafs_integer;
typedef int32_t lisafs_longint;

/*!
    A Lisa timestamp in seconds since midnight on 1 January 1901.
 */
typedef uint32_t lisafs_timestamp;

/*! Convert a Lisa timestamp to a UNIX time_t. */
LISAFS_EXTERN time_t lisafs_timestamp_to_time_t(lisafs_timestamp timestamp);


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

#define LISAFS_REDLIGHT     -1      //!< link terminator in page label

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
    The Lisa filesystem versions.
 */
enum lisafs_fsversion : int16_t {
    release1 = 14,          //!< 1.0
    release2 = 15,          //!< 2.0
    release3 = 17,          //!< 3.0
};
typedef enum lisafs_fsversion lisafs_fsversion;

const char * _Nonnull lisafs_fsversion_string(lisafs_fsversion version);


/*!
    The different mount states that a volume can have.
 */
enum lisafs_mountstate : int16_t {
    unmounted,
    temp_unmounted,
    defmounted,
    realmounted,
    fsmounted,
};
typedef enum lisafs_mountstate lisafs_mountstate;


/*!
    A Lisa filesystem starts with a structure called the "media
    description data file," abbreviated MDDF, that contains information
    about the volume it represents and pointers to the additional
    structures. This is always in logical block 0 on the volume, which
    may or may not be physical block 0.
 */
struct lisafs_mddf {
    lisafs_fsversion        fsversion;
    lisafs_uid              volid;
    lisafs_integer          volnum;
    char                    volname[33];
    char                    padc1;
    char                    password[33];
    char                    padc2;
    lisafs_longint          init_machine_id;
    lisafs_longint          master_machine_id;
    lisafs_timestamp        DT_created;
    lisafs_timestamp        DT_copy_created;
    lisafs_timestamp        DT_copied;
    lisafs_timestamp        DT_scavenged;
    lisafs_longint          copy_thread;
    lisafs_longint          firstblock;
    lisafs_longint          lastblock;
    lisafs_longint          lastfspage;
    lisafs_longint          blockcount;
    lisafs_integer          blocksize;
    lisafs_integer          datasize;
    lisafs_integer          cluster_size;
    lisafs_longint          MDDFaddr;
    lisafs_integer          MDDFsize;
    lisafs_longint          bitmap_addr;
    lisafs_longint          bitmap_size;
    lisafs_integer          bitmap_bytes;
    lisafs_integer          bitmap_pages;
    lisafs_longint          slist_addr;         //!< Page where S-list starts
    lisafs_integer          slist_packing;      //!< S-list entries per block
    lisafs_integer          slist_block_count;  //!< Total S-list blocks
    lisafs_integer          first_file;
    lisafs_integer          empty_file;
    lisafs_integer          maxfiles;
    lisafs_integer          hintsize;
    lisafs_integer          leader_offset;
    lisafs_integer          leader_pages;
    lisafs_integer          flabel_offset;
    lisafs_integer          unusedi1;
    lisafs_integer          map_offset;
    lisafs_integer          map_size;
    lisafs_integer          filecount;
    lisafs_longint          freestart;
    lisafs_longint          unusedl1;
    lisafs_longint          freecount;
    lisafs_integer          rootsnum;
    lisafs_integer          rootmaxentries;
    lisafs_mountstate       mountinfo;
    lisafs_uid              overmount_stamp;
    lisafs_longint          pmem_id;
    lisafs_integer          pmem[32];
    lisafs_boolean          vol_scavenged;
    lisafs_boolean          tbt_copied;
    lisafs_integer          smallmap_offset;
    lisafs_integer          hentry_offset;
    lisafs_uid              backup_volid;
    lisafs_integer          flabel_size;
    lisafs_integer          fs_overhead;
    lisafs_integer          result_scavenge;
    lisafs_integer          boot_code;
    lisafs_integer          boot_environ;
    lisafs_longint          oem_id;
    lisafs_longint          root_page;          //<! Root node of B-tree directory
    lisafs_integer          tree_depth;
    lisafs_integer          node_id;
    lisafs_integer          vol_seq_no;
    lisafs_boolean          vol_mounted;
} LISAFS_PACKED;
typedef struct lisafs_mddf lisafs_mddf;


#define LISAFS_FREE_SNUM        0       //!< SF num on a free page
#define LISAFS_MDDF_SNUM        1       //!< SF num of the MDDF
#define LISAFS_BITMAP_SNUM      2       //!< SF num of the allocation bitmap
#define LISAFS_SLIST_SNUM       3       //!< SF num of SF list file
#define LISAFS_ROOTDIR_SNUM     4       //!< SF num of root directory
#define LISAFS_FIRSTUSER_SF     5       //!< SF of the first user file


/*!
    A small file entry.
 */
struct lisafs_s_entry {
    lisafs_longint  hintaddr;
    lisafs_longint  fileaddr;
    lisafs_longint  filesize;
    lisafs_integer  version;
} LISAFS_PACKED;
typedef struct lisafs_s_entry lisafs_s_entry;


/*!
    Lisa file types.
 */
enum lisafs_filetype {
    undefined,
    MDDFfile,
    rootcat,
    freelist,
    badblocks,
    sysdata,
    spool,
    exec,
    userdir,
    pipe,
    bootfile,
    swapdata,
    swapcode,
    ramap,
    userfile,
    killedobject,
    tempfile,
};
typedef enum lisafs_filetype lisafs_filetype;


/*!
    Lisa entry types.
 */
enum lisafs_entrytype {
    emptyentry,
    direntry,
    linkentry,
    fileentry,
    pipeentry,
    ecentry,
    killedentry,
    removed,
    threadentry,
};
typedef enum lisafs_entrytype lisafs_entrytype;


/*!
    Lisa file map entry.
 */
struct lisafs_mapentry {
    lisafs_longint  address;    //< absolute page of contiguous chunk
    lisafs_integer  cpages;     //< number of pages in chunk
};
typedef struct lisafs_mapentry lisafs_mapentry;


/*!
    Lisa file map.
 */
struct lisafs_filemap {
    lisafs_longint  size;           //< number of blocks in this s-file
    lisafs_integer  max_entries;    //< max count of mapentry in this map
    lisafs_integer  ecount;         //< count of mapentry in this map
    lisafs_mapentry map[84];        //< the map itself

};
typedef struct lisafs_filemap lisafs_filemap;


/*!
    Lisa small file map.
 */
struct lisafs_smallmap {
    lisafs_longint  size;           //< number of blocks in this s-file
    lisafs_integer  max_entries;    //< max count of mapentry in this map
    lisafs_integer  ecount;         //< count of mapentry in this map
    lisafs_mapentry map[10];        //< the small map itself
};
typedef struct lisafs_smallmap lisafs_smallmap;

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
    Get the raw disk image underlying this filesystem image.
 */
void * _Nonnull lisafs_image_get_raw_image(lisafs_image * _Nonnull image);

/*!
    Get the loader header for the given filesystem image. The type of
    structure returned will depend on the specific type of disk, e.g.
    microfloppy versus ProFile/Widget.
 */
void * _Nonnull lisafs_image_get_loader_header(lisafs_image * _Nonnull image);

/*!
    Get the media data description file for the given filesystem image.
 */
lisafs_mddf * _Nonnull lisafs_image_get_mddf(lisafs_image * _Nonnull image);

const char * _Nonnull lisafs_image_get_volname(lisafs_image * _Nonnull image);
const char * _Nonnull lisafs_image_get_password(lisafs_image * _Nonnull image);

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
