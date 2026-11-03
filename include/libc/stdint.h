//===-- include/libc/stdint.h - MicroCRT Integer Types ----------*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares exact-width, least-width, and pointer-holding integer
/// types along with their corresponding limit macros according to ISO C99.
///
//===----------------------------------------------------------------------===//

#ifndef __STDINT_H__
#define __STDINT_H__

//===----------------------------------------------------------------------===//
// Exact-Width Integer Types
//===----------------------------------------------------------------------===//

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

//===----------------------------------------------------------------------===//
// Minimum-Width Integer Types
//===----------------------------------------------------------------------===//

typedef int8_t             int_least8_t;
typedef int16_t            int_least16_t;
typedef int32_t            int_least32_t;
typedef int64_t            int_least64_t;

typedef uint8_t            uint_least8_t;
typedef uint16_t           uint_least16_t;
typedef uint32_t           uint_least32_t;
typedef uint64_t           uint_least64_t;

//===----------------------------------------------------------------------===//
// Fastest Minimum-Width Integer Types
//===----------------------------------------------------------------------===//

typedef int8_t             int_fast8_t;
typedef int                int_fast16_t;
typedef int                int_fast32_t;
typedef int64_t            int_fast64_t;

typedef uint8_t            uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned int       uint_fast32_t;
typedef uint64_t           uint_fast64_t;

//===----------------------------------------------------------------------===//
// Pointer-Holding Integer Types
//===----------------------------------------------------------------------===//

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
typedef int32_t            intptr_t;
typedef uint32_t           uintptr_t;
#else
typedef long               intptr_t;
typedef unsigned long      uintptr_t;
#endif

//===----------------------------------------------------------------------===//
// Greatest-Width Integer Types
//===----------------------------------------------------------------------===//

typedef int64_t            intmax_t;
typedef uint64_t           uintmax_t;

//===----------------------------------------------------------------------===//
// Limits of Exact-Width Integer Types
//===----------------------------------------------------------------------===//

#define INT8_MIN         (-128)
#define INT8_MAX         (127)
#define UINT8_MAX        (255)

#define INT16_MIN        (-32768)
#define INT16_MAX        (32767)
#define UINT16_MAX       (65535)

#define INT32_MIN        (-2147483647 - 1)
#define INT32_MAX        (2147483647)
#define UINT32_MAX       (4294967295U)

#define INT64_MIN        (-9223372036854775807LL - 1LL)
#define INT64_MAX        (9223372036854775807LL)
#define UINT64_MAX       (18446744073709551615ULL)

//===----------------------------------------------------------------------===//
// Limits of Other Integer Types
//===----------------------------------------------------------------------===//

#define PTRDIFF_MIN      INT64_MIN
#define PTRDIFF_MAX      INT64_MAX
#define SIZE_MAX         UINT64_MAX

#endif // __STDINT_H__
