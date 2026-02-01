//  lisafs_defines.h
//  Part of LisaFilesystem.
//
//  Copyright © 2026 Christopher M. Hanson. All rights reserved.
//  See file COPYING for details.

#ifndef __LISAFS__DEFINES__H__
#define __LISAFS__DEFINES__H__

#include <stdbool.h>
#include <stdint.h>


#if defined(__cplusplus) || defined(cplusplus)
#define LISAFS_EXTERN extern "C"
#else
#define LISAFS_EXTERN extern
#endif


#define LISAFS_PACKED __attribute__((packed))
#define LISAFS_NONNULL _Nonnull
#define LISAFS_NULLABLE _Nullable

#endif /* __LISAFS__DEFINES__H__ */
