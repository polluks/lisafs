//  lisafs_dev_commands.h
//  Part of lisafs.
//
//  Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#ifndef __LISAFS_DEV_COMMANDS__H__
#define __LISAFS_DEV_COMMANDS__H__

#include "lisafs_defines.h"


LISAFS_HEADER_BEGIN


#if LISAFS_ENABLE_DEV_COMMANDS
LISAFS_EXTERN int lisafs_dumpblock(int argc, char * _Nullable * _Nonnull argv);
LISAFS_EXTERN int lisafs_dumppage(int argc, char * _Nullable * _Nonnull argv);
LISAFS_EXTERN int lisafs_fsinfo(int argc, char * _Nullable * _Nonnull argv);
LISAFS_EXTERN int lisafs_imageinfo(int argc, char * _Nullable * _Nonnull argv);
LISAFS_EXTERN int lisafs_sfextract(int argc, char * _Nullable * _Nonnull argv);
LISAFS_EXTERN int lisafs_sflist(int argc, char * _Nullable * _Nonnull argv);
#endif


LISAFS_HEADER_END


#endif /* __LISAFS_DEV_COMMANDS__H__ */
