//  lisafs_commands.h
//  Part of lisafs.
//
//  Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#ifndef __LISAFS_COMMANDS__H__
#define __LISAFS_COMMANDS__H__

#include "lisafs_defines.h"

#include "lisafs_main.h"


LISAFS_HEADER_BEGIN


int lisafs_extract(int argc, char * _Nullable * _Nonnull argv);
int lisafs_list(int argc, char * _Nullable * _Nonnull argv);


LISAFS_HEADER_END


#endif /* __LISAFS_COMMANDS__H__ */
