//  lisafs_defines.h
//  Part of lisafs.
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


#define LISAFS_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define LISAFS_ASSUME_NONNULL_END   _Pragma("clang assume_nonnull end")

#define LISAFS_HEADER_BEGIN    LISAFS_ASSUME_NONNULL_BEGIN
#define LISAFS_HEADER_END      LISAFS_ASSUME_NONNULL_END

#define LISAFS_SOURCE_BEGIN    LISAFS_ASSUME_NONNULL_BEGIN
#define LISAFS_SOURCE_END      LISAFS_ASSUME_NONNULL_END


#endif /* __LISAFS__DEFINES__H__ */
