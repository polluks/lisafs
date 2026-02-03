//  lisafs_main.h
//  Part of lisafs.
//
//  Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#ifndef __LISAFS_MAIN__H__
#define __LISAFS_MAIN__H__

#include "lisafs_defines.h"

#include "lisafs.h"


LISAFS_HEADER_BEGIN


LISAFS_EXTERN const char *program_name;
LISAFS_EXTERN const char *image_file_path;
LISAFS_EXTERN const char *command_name;

LISAFS_EXTERN lisafs_image *image;

LISAFS_EXTERN void print_usage(void);


LISAFS_HEADER_END


#endif /* __LISAFS_MAIN__H__ */
