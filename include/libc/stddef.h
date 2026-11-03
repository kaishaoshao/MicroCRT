//===-- include/libc/stddef.h - MicroCRT Standard Type Definitions -*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines standard basic types and constants: ptrdiff_t, size_t,
/// wchar_t, NULL, offsetof, and max_align_t.
///
//===----------------------------------------------------------------------===//

#ifndef __STDDEF_H__
#define __STDDEF_H__

//===----------------------------------------------------------------------===//
// Standard Pointer and Size Types
//===----------------------------------------------------------------------===//

#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED
/// Signed integer type resulting from pointer subtraction.
typedef long ptrdiff_t;
#endif

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
/// Unsigned integer type resulting from the sizeof operator.
typedef unsigned long size_t;
#endif

#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
/// Wide character type capable of representing distinct system codes.
typedef int wchar_t;
#endif

/// Null pointer constant.
#undef NULL
#define NULL ((void *)0)

/// Offset of member from the beginning of its parent structure type in bytes.
#define offsetof(type, member) ((size_t)&(((type *)0)->member))

/// Type whose alignment requirement is at least as strict as all scalar types.
typedef struct {
  long long __max_align_ll;
  long double __max_align_ld;
} max_align_t;

#endif // __STDDEF_H__
