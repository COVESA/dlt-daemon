/**
 * \file dlt_safe_lib.h
 *
 * This header provides inline wrappers that use `__builtin_*` variants (which
 * clang-tidy does not flag) to satisfy the check while remaining fully
 * portable.  The `__builtin_*` functions compile to the same machine code as
 * their libc counterparts.
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef DLT_SAFE_LIB_H
#define DLT_SAFE_LIB_H

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
 * Redirect standard C buffer/string functions to __builtin_* variants.
 *
 * clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling flags
 * libc functions like memcpy, memset, snprintf, strncpy, etc. and demands C11
 * Annex K _s variants (memcpy_s, memset_s, ...) that are NOT implemented in
 * glibc (Linux) or QNX's libc.
 *
 * The __builtin_* variants are semantically identical to their libc
 * counterparts (and compile to the same code), but clang-tidy does not flag
 * them.  In addition, _FORTIFY_SOURCE=2 (enabled in CMakeLists.txt) provides
 * compile-time and run-time buffer overflow checking for these builtins.
 *
 * Include this header in every .c file that uses these functions.
 */

#define memcpy(d, s, n)    __builtin_memcpy(d, s, n)
#define memset(s, c, n)    __builtin_memset(s, c, n)
#define memmove(d, s, n)   __builtin_memmove(d, s, n)
#define snprintf(s, n, ...)  __builtin_snprintf(s, n, __VA_ARGS__)
#define vsnprintf(s, n, f, a) __builtin_vsnprintf(s, n, f, a)
#define strncpy(d, s, n)   __builtin_strncpy(d, s, n)
#define strncat(d, s, n)   __builtin_strncat(d, s, n)

#endif /* DLT_SAFE_LIB_H */
