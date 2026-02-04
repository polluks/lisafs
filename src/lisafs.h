//  lisafs.h
//	Part of lisafs.
//
//	Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#ifndef __LISAFS__H__
#define __LISAFS__H__

#include "lisafs_defines.h"

#include <stdlib.h>
#include <time.h>


LISAFS_HEADER_BEGIN


// MARK: - Types and Structures

typedef int8_t lisafs_byte;
typedef int8_t lisafs_boolean1;
typedef int16_t lisafs_boolean;
typedef int16_t lisafs_integer;
typedef int32_t lisafs_longint;


/*! A Lisa timestamp in seconds since midnight on 1 January 1901. */
typedef uint32_t lisafs_timestamp;

/*! Convert a Lisa timestamp to a UNIX time_t. */
LISAFS_EXTERN time_t lisafs_timestamp_to_time_t(lisafs_timestamp timestamp);

/*! Convenience to convert a Lisa timestamp directly to a string. */
LISAFS_EXTERN const char * _Nonnull lisafs_timestamp_string(lisafs_timestamp timestamp);


/*! A Lisa filesystem block address is an absolute block number. */
typedef int32_t lisafs_baddr;

/*! A Lisa filesystem page address is a block number relative to the MDDF. */
typedef int32_t lisafs_paddr;

/*! A Lisa filesystem file ID is the index of an S-file. */
typedef int16_t lisafs_fileid;

/*! A B-Tree node ID. */
typedef int16_t lisafs_nodeid;

#define LISAFS_LABEL_SIZE	24

/*! A raw Lisa filesystem tag. */
typedef uint8_t lisafs_label[LISAFS_LABEL_SIZE];

#define LISAFS_BLOCK_SIZE    512

/*! A raw Lisa fileystem block. */
typedef uint8_t lisafs_block[LISAFS_BLOCK_SIZE];


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
    lisafs_fileid   fileid;
    lisafs_integer  dataused;
    lisafs_baddr    abspage;
    lisafs_paddr    relpage;
    lisafs_longint  fwdlink;
    lisafs_longint  bkwdlink;
} LISAFS_PACKED;
typedef struct lisafs_pagelabel lisafs_pagelabel;

#define LISAFS_REDLIGHT     -1      //!< link terminator in page label

/*! The data portion of a page on a Lisa device. */
typedef uint8_t lisafs_pagedata[512];


/*!
    A complete Lisa page.
 */
struct lisafs_page {
    lisafs_pagedata     data;
    lisafs_pagelabel    label;
};
typedef struct lisafs_page lisafs_page;


/*!
    A microfloppy has a header in its first block with some information
    about the loader it contains. It starts with an actual JMP
    instruction so the boot ROM can load that first block and just jump
    to its first word to boot from it.
 */
struct lisafs_mf_loader_loader_header {
    lisafs_longint  jmp;            //!< JMP instruction to skip header
    lisafs_integer  boot_id;        //!< should be 0xAAAA
    lisafs_integer  ldr_version;    //!< should be 0x0850
    lisafs_integer  globalsize;     //!< amount of global data for loader
    lisafs_integer  codesize;       //!< size of the loader
    lisafs_integer  pc_offset;      //!< offset from loader base to main entry
    lisafs_integer  fs_block0;      //!< block address of MDDF
} LISAFS_PACKED;
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

/*!
    Get the string equivalent of the given version. This does not need
    to be freed by the caller.

    - WARNING: Not reentrant.
 */
LISAFS_EXTERN const char * _Nonnull lisafs_fsversion_string(lisafs_fsversion version);


/*!
    The different mount states that a volume can have.
 */
enum lisafs_mountstate : int8_t {
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
    lisafs_baddr            firstblock;
    lisafs_baddr            lastblock;
    lisafs_paddr            lastfspage;
    lisafs_longint          blockcount;
    lisafs_integer          blocksize;
    lisafs_integer          datasize;
    lisafs_integer          cluster_size;
    lisafs_paddr            MDDFaddr;
    lisafs_integer          MDDFsize;
    lisafs_paddr            bitmap_addr;
    lisafs_longint          bitmap_size;
    lisafs_integer          bitmap_bytes;
    lisafs_integer          bitmap_pages;
    lisafs_paddr            slist_addr;         //!< Page where S-list starts
    lisafs_integer          slist_packing;      //!< S-list entries per block
    lisafs_integer          slist_block_count;  //!< Total S-list blocks
    lisafs_fileid           first_file;
    lisafs_fileid           empty_file;
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
    lisafs_fileid           rootsnum;
    lisafs_integer          rootmaxentries;
    lisafs_mountstate       mountinfo;
    lisafs_byte             mountinfo_pad;
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
    lisafs_paddr            root_page;          //!< Root node of B-tree directory
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
    A "small file" entry. It's interesting that they were called this
    when the filesize limit was effectively 2GB, as long as enough
    contiguous space was available that the file's extents didn't
    overflow the filemap.
 */
struct lisafs_s_entry {
    lisafs_paddr    hintaddr;
    lisafs_paddr    fileaddr;
    lisafs_longint  filesize;
    lisafs_integer  version;
} LISAFS_PACKED;
typedef struct lisafs_s_entry lisafs_s_entry;


/*!
    Lisa file types.
 */
enum lisafs_filetype: int8_t {
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

const char * _Nullable lisafs_filetype_string(lisafs_filetype t);


/*!
    Lisa "build control" record.
 */
struct lisafs_build_control {
    lisafs_integer  release_number;         //!< public release number
    lisafs_integer  build_number;           //!< internal build membership
    lisafs_integer  compatibility_level;    //!< local compatibility level
    lisafs_integer  revision_level;         //!< iteration of file
} LISAFS_PACKED;
typedef struct lisafs_build_control lisafs_build_control;

/*!
    Lisa file hint entry.
 */
struct lisafs_hentry {
    char                name[33];
    char                name_pad;
    lisafs_uid          UID;
    lisafs_integer      version;
    lisafs_filetype     ftype;
    int8_t              ftype_pad;
    lisafs_timestamp    date_created;
    lisafs_timestamp    date_accessed;
    lisafs_timestamp    date_modified;
    lisafs_timestamp    date_backup;
    lisafs_timestamp    date_scavenged;
    lisafs_longint      machine_id;
    lisafs_boolean1     killed;
    lisafs_boolean1     safety_on;
    lisafs_boolean1     protected;
    lisafs_boolean1     master;
    lisafs_boolean1     close_by_OS;
    lisafs_boolean1     file_open;
    lisafs_integer      result_scavenge;
    lisafs_integer      unusedi1;
    lisafs_integer      system_type;
    lisafs_integer      user_type;
    lisafs_integer      user_subtype;
    lisafs_build_control    build_info;
    lisafs_integer      file_portion;       //!< portion of large file split across media
    char                password[9];
    char                password_pad[3];
    lisafs_nodeid       parentID;
    lisafs_integer      fsOverhead;
} LISAFS_PACKED;
typedef struct lisafs_hentry lisafs_hentry;

/*!
    Lisa file map entry.
 */
struct lisafs_mapentry {
    lisafs_baddr    address;        //!< absolute page of contiguous chunk
    lisafs_integer  cpages;         //!< number of pages in chunk
} LISAFS_PACKED;
typedef struct lisafs_mapentry lisafs_mapentry;


/*!
    Lisa file map.
 */
struct lisafs_filemap {
    lisafs_longint  size;           //!< number of blocks in this s-file
    lisafs_integer  max_entries;    //!< max count of mapentry in this map
    lisafs_integer  ecount;         //!< count of mapentry in this map
    lisafs_mapentry map[84];        //!< the map itself
} LISAFS_PACKED;
typedef struct lisafs_filemap lisafs_filemap;


/*!
    Lisa small file map.
 */
struct lisafs_smallmap {
    lisafs_longint  size;           //!< number of blocks in this s-file
    lisafs_integer  max_entries;    //!< max count of mapentry in this map
    lisafs_integer  ecount;         //!< count of mapentry in this map
    lisafs_mapentry map[10];        //!< the small map itself
} LISAFS_PACKED;
typedef struct lisafs_smallmap lisafs_smallmap;


/*!
    A B-Tree key.

    - WARNING: This must be packed to be stored in 36 bytes.
 */
struct lisafs_key {
	lisafs_byte key_length;
	lisafs_integer parent_id;
	char name[33];
} LISAFS_PACKED;
typedef struct lisafs_key lisafs_key;

/*! Type of a B-Tree entry. */
enum lisafs_entrytype: int8_t {
    emptyentry  = 0,    //!< empty
    direntry    = 1,    //!< directory
    linkentry   = 2,    //!< link
    fileentry   = 3,    //!< file
    pipeentry   = 4,    //!< pipe, no longer supported
    ecentry     = 5,    //!< event channel, no longer supported
    killedentry = 6,    //!< killed object entry
    removed     = 7,    //!< removed
    threadentry = 8,    //!< directory thread entry
};
typedef enum lisafs_entrytype lisafs_entrytype;

/*! A Lisa B-Tree entry header. */
struct lisafs_btentryheader {
    lisafs_key key;
    lisafs_entrytype etype;
    lisafs_byte etype_pad;
} LISAFS_PACKED;
typedef struct lisafs_btentryheader lisafs_btentryheader;

/*! A Lisa B-Tree entry representing a filesystem object. */
struct lisafs_objectrec {
    lisafs_btentryheader header;
    lisafs_fileid sfile;
    lisafs_timestamp fileDTC;
    lisafs_timestamp fileDTM;
    lisafs_longint size;
    lisafs_longint physSize;
    lisafs_integer fsOvrhd;
    lisafs_integer flags;
    lisafs_longint unused;
} LISAFS_PACKED;
typedef struct lisafs_objectrec lisafs_objectrec;

/*! A Lisa B-Tree entry representing a filesystem directory. */
struct lisafs_directrec {
    lisafs_btentryheader header;
    lisafs_nodeid nodeID;
    lisafs_timestamp DtCreat;
    lisafs_longint unused;
} LISAFS_PACKED;
typedef struct lisafs_directrec lisafs_directrec;

/*! A Lisa B-Tree thread entry. */
struct lisafs_threadrec {
    lisafs_btentryheader header;
    lisafs_nodeid parID;
    char myName[33];
    char myName_pad;
    lisafs_longint unused;
} LISAFS_PACKED;
typedef struct lisafs_threadrec lisafs_threadrec;

/*! A runtime representation of a Lisa directory entry. */
union lisafs_directory_entry {
    lisafs_btentryheader header_only;
    lisafs_objectrec object;
    lisafs_directrec directory;
    lisafs_threadrec thread;
};
typedef union lisafs_directory_entry lisafs_directory_entry;

/*! A catalog entry used on pre-3.0 filesystems. */
struct lisafs_centry {
    char name[33];
    char name_pad;
    lisafs_entrytype cetype;
    lisafs_byte cetype_pad;
    lisafs_fileid sfile;
    lisafs_longint attributes;
    lisafs_paddr readpage;
    lisafs_integer readoffset;
    lisafs_paddr writepage;
    lisafs_integer writeoffset;
} LISAFS_PACKED;
typedef struct lisafs_centry lisafs_centry;


/*!
    A Lisa filesystem image. The contents are private.
 */
typedef struct lisafs_image lisafs_image;


// MARK: - Functions

/*!
    Open and return the Lisa filesystem image at the given path.
 */
lisafs_image * _Nullable lisafs_open(const char * _Nonnull const path);

/*!
    Close the given Lisa filesystem image.
 */
int lisafs_close(lisafs_image * _Nullable image);

/*!
    Get the raw disk image underlying this filesystem image.
 */
void * _Nonnull lisafs_get_raw_image(lisafs_image * _Nonnull image);

/*!
    Get the loader header for the given filesystem image. The type of
    structure returned will depend on the specific type of disk, e.g.
    microfloppy versus ProFile/Widget.
 */
void * _Nonnull lisafs_get_loader_header(lisafs_image * _Nonnull image);

/*!
    Get the media data description file for the given filesystem image.
 */
lisafs_mddf * _Nonnull lisafs_get_mddf(lisafs_image * _Nonnull image);


/*! Get the version of the filesystem in the given filesystem image. */
lisafs_fsversion lisafs_get_fsversion(lisafs_image * _Nonnull image);

/*! Get the volume name represented by the given filesystem image. */
const char * _Nonnull lisafs_get_volname(lisafs_image * _Nonnull image);

/*! Get the password for the given filesystem image. */
const char * _Nonnull lisafs_get_password(lisafs_image * _Nonnull image);

/*!
    Read both the raw data and tag bytes of physical block n from the
    given open Lisa disk image.
 */
int lisafs_read_block(lisafs_image * _Nonnull image,
                      lisafs_baddr n,
                      lisafs_block _Nonnull block,
                      lisafs_label _Nonnull label);

/*!
    Read both the data and label of page n from the given Lisa disk
    image. This takes into account things like the disk (not image)
    header, since page 0 almost certainly isn't physical block 0.
 */
int lisafs_read_page(lisafs_image * _Nonnull image,
                     lisafs_paddr n,
                     lisafs_page * _Nonnull page);

/*!
    Get the size of the given S-file.
 */
lisafs_longint lisafs_get_sfile_size(lisafs_image * _Nonnull image,
                                     lisafs_fileid file);

/*!
    Read the file hints for the given S-file.

    - WARNING: This cannot read hints for special S-files (those with a
               file ID less than `LISAFS_FIRSTUSER_SF`), since they have
               no hints.
 */
int lisafs_read_sfile_hints(lisafs_image * _Nonnull image,
                            lisafs_fileid file,
                            lisafs_hentry * _Nonnull hints);

/*!
    Read a specified quantity of data from the S-file with the given
    file ID into the given buffer.

    - WARNING: This cannot read special S-files (those with a file ID
               less than `LISAFS_FIRSTUSER_SF`); those should be read
               page-by-page if necessary.
 */
int lisafs_read_sfile(lisafs_image * _Nonnull image,
                      lisafs_fileid file,
                      void * _Nonnull buf,
                      size_t buf_size);


/*!
    The iterator function passed to ``lisafs_iterate_btree_entries``.
 */
typedef int (*lisafs_btree_entry_iterator)(lisafs_directory_entry * _Nonnull entry,
                                           void * _Nullable context);

/*!
    Iterate over the directory entries in the b-tree, calling the
    iterator function until it either returns a failure (-1), a
    stop value (1), or there are no more entries. The iterator
    function is passed the given context.

    - NOTE: Release 3.0 only.
 */
int lisafs_iterate_btree_entries(lisafs_image * _Nonnull image,
                                 lisafs_btree_entry_iterator _Nonnull iterator,
                                 void * _Nullable context);


/*!
    The iterator function passed to ``lisafs_iterate_directory_entries``.
 */
typedef int (*lisafs_directory_entry_iterator)(lisafs_centry * _Nonnull entry,
                                               void * _Nullable context);

/*!
     Iterate over the non-empty directory entries, calling the iterator
     function until it either returns a failure (-1), a stop value (1),
     or there are no more entries. The iterator function is passed the
     given context.

     - NOTE: Release 2.0 and earlier only.
 */
int lisafs_iterate_directory_entries(lisafs_image * _Nonnull image,
                                     lisafs_directory_entry_iterator _Nonnull iterator,
                                     void * _Nullable context);


/*!
    A decomposed Lisa filesystem path. The components of the decomposed
    path are in left-to-right (containment) order.
 */
struct lisafs_path {
    char * _Nonnull * _Nonnull components;
    lisafs_integer component_count;
};
typedef struct lisafs_path lisafs_path;

/*!
    Create a new ``lisa_path`` instance containing the components of
    the given string in canonical Lisa path format. Returns `NULL` and
    sets `errno` on failure.

    - NOTE: This function does not support wildcards, does not support
            prepending a device name component, and does not support
            paths relative to anything but the volume root. (That is,
            one can omit a leading `-` character.)
 */
LISAFS_EXTERN
lisafs_path * _Nullable
lisafs_path_from_string(const char *string);

/*!
    Release the storage used by the given ``lisa_path`` instance.

    - NOTE: Preserves `errno`.
 */
LISAFS_EXTERN
void
lisafs_path_free(lisafs_path * _Nullable path);

/*!
    Look up and return the sfile for the filesystem object at the given
    path, returning -1 if not found.
 */
LISAFS_EXTERN
lisafs_fileid
lisafs_lookup_sfile(lisafs_image *image, lisafs_path *path);


LISAFS_HEADER_END


#endif /* __LISAFS__H__ */
