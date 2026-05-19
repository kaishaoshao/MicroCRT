/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_LIBC_CONF_DEFAULTS_H
#define __SEGGER_RTL_LIBC_CONF_DEFAULTS_H

/*********************************************************************
*
*       Things needing sorting early in compilation
*
**********************************************************************
*/

#if defined(__cplusplus)
  #define __SEGGER_RTL_BOOL                     bool
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
  #define __SEGGER_RTL_BOOL                     _Bool
#else
  #define __SEGGER_RTL_BOOL                     unsigned
#endif

//
// Configuration of language standard
// Should be:
//   C89:   198900L (gcc defines __STDC__ only)
//   C90:   199000L (gcc defines __STDC__ only)
//   C90:   199409L
//   C99:   199901L
//   C11:   201112L
//   C18:   201710L
//   C++98: 199711L
//   C++11: 201103L
//   C++14: 201402L
//   C++17: 201703L
//
#define __SEGGER_RTL_STDC_VERSION_C90           199000L
#define __SEGGER_RTL_STDC_VERSION_C94           199409L
#define __SEGGER_RTL_STDC_VERSION_C99           199901L
#define __SEGGER_RTL_STDC_VERSION_C11           201112L
#define __SEGGER_RTL_STDC_VERSION_C18           201710L
#define __SEGGER_RTL_STDC_VERSION_CPP98         199711L
#define __SEGGER_RTL_STDC_VERSION_CPP11         201103L
#define __SEGGER_RTL_STDC_VERSION_CPP14         201402L
#define __SEGGER_RTL_STDC_VERSION_CPP17         201703L

#if !defined(__SEGGER_RTL_STDC_VERSION)
  #if defined(__STDC_VERSION__)
    #define __SEGGER_RTL_STDC_VERSION           __STDC_VERSION__
  #elif defined (__STDC__)
    #define __SEGGER_RTL_STDC_VERSION           __SEGGER_RTL_STDC_VERSION_C90
  #endif
#endif

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Conf.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define __SEGGER_RTL_NAN_FORMAT_IEEE            0    // NaN is fully conformant IEEE NaN
#define __SEGGER_RTL_NAN_FORMAT_FAST            1    // NaN is AEABI-conformant IEEE NaN; 64-bit NaN can be distingushed from Inf using only the upper 32 bits
#define __SEGGER_RTL_NAN_FORMAT_COMPACT         2    // NaN is defined only by the top 16 bits of the floating value

#define __WIDTH_INT                             0
#define __WIDTH_LONG                            1
#define __WIDTH_LONG_LONG                       2

#define __WIDTH_NONE                            0
#define __WIDTH_FLOAT                           1
#define __WIDTH_DOUBLE                          2

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// Configuration of internal implementation.
//
#ifndef   __SEGGER_RTL_SIDE_BY_SIDE
  #define __SEGGER_RTL_SIDE_BY_SIDE             0
#endif

#ifndef   __SEGGER_RTL_FORCE_SOFT_FLOAT
  #define __SEGGER_RTL_FORCE_SOFT_FLOAT         0
#endif

#ifndef   __SEGGER_RTL_INCLUDE_AEABI_API
  #define __SEGGER_RTL_INCLUDE_AEABI_API        0
#endif

#ifndef   __SEGGER_RTL_INCLUDE_GNU_API
  #define __SEGGER_RTL_INCLUDE_GNU_API          0
#endif

#ifndef   __SEGGER_RTL_INCLUDE_GNU_FP16_API
  #define __SEGGER_RTL_INCLUDE_GNU_FP16_API     0
#endif

#ifndef   __SEGGER_RTL_INCLUDE_SEGGER_API
  #define __SEGGER_RTL_INCLUDE_SEGGER_API       0
#endif

#ifndef   __SEGGER_RTL_INCLUDE_C_API
  #define __SEGGER_RTL_INCLUDE_C_API            1
#endif

#ifndef   __SEGGER_RTL_INCLUDE_BENCHMARKING
  #define __SEGGER_RTL_INCLUDE_BENCHMARKING     0
#endif
//
// Selection based on typeset.
//
#if __SEGGER_RTL_TYPESET == 16
  #define __SEGGER_RTL_SELECT_TYPESET(T16, T32, T64)        T16
#elif __SEGGER_RTL_TYPESET == 32
  #define __SEGGER_RTL_SELECT_TYPESET(T16, T32, T64)        T32
#elif __SEGGER_RTL_TYPESET == 64
  #define __SEGGER_RTL_SELECT_TYPESET(T16, T32, T64)        T64
#else
  #define __SEGGER_RTL_SELECT_TYPESET(T16, T32, T64)        999
#endif

//
// Basic configuration for compilers that expose information using
// internal macros, such as GCC and clang
// and fallback to "good" defaults if available.
//
// Type sizes
//
#if !defined(__SEGGER_RTL_SIZEOF_INT) && defined(__SIZEOF_INT__)
  #define __SEGGER_RTL_SIZEOF_INT               __SIZEOF_INT__
#endif
#if !defined(__SEGGER_RTL_SIZEOF_INT)
  #define __SEGGER_RTL_SIZEOF_INT               __SEGGER_RTL_SELECT_TYPESET(2, 4, 4)
#endif

#if !defined(__SEGGER_RTL_SIZEOF_LONG) && defined(__SIZEOF_LONG__)
  #define __SEGGER_RTL_SIZEOF_LONG              __SIZEOF_LONG__
#endif
#if !defined(__SEGGER_RTL_SIZEOF_LONG)
  #define __SEGGER_RTL_SIZEOF_LONG              __SEGGER_RTL_SELECT_TYPESET(4, 4, 8)
#endif

#if !defined(__SEGGER_RTL_SIZEOF_PTR) && defined(__SIZEOF_POINTER__)
  #define __SEGGER_RTL_SIZEOF_PTR               __SIZEOF_POINTER__
#endif
#if !defined(__SEGGER_RTL_SIZEOF_PTR)
  #define __SEGGER_RTL_SIZEOF_PTR               __SEGGER_RTL_SELECT_TYPESET(2, 4, 8)
#endif

//
// Configuration of long double size; default to binary64.
//
#if !defined(__SEGGER_RTL_SIZEOF_LDOUBLE)
  #define __SEGGER_RTL_SIZEOF_LDOUBLE           8
#endif

//
// Types (regular)
//
#if !defined(__SEGGER_RTL_INT8_T) && defined(__INT8_TYPE__)
  #define __SEGGER_RTL_INT8_T                   __INT8_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT8_T)
  #define __SEGGER_RTL_INT8_T                   signed char
#endif
#define __SEGGER_RTL_I8                         __SEGGER_RTL_INT8_T

#if !defined(__SEGGER_RTL_UINT8_T) && defined(__UINT8_TYPE__)
  #define __SEGGER_RTL_UINT8_T                  __UINT8_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT8_T)
  #define __SEGGER_RTL_UINT8_T                  unsigned char
#endif
#define __SEGGER_RTL_U8                         __SEGGER_RTL_UINT8_T

#if !defined(__SEGGER_RTL_INT16_T) && defined(__INT16_TYPE__)
  #define __SEGGER_RTL_INT16_T                  __INT16_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT16_T)
  #define __SEGGER_RTL_INT16_T                  short
#endif
#define __SEGGER_RTL_I16                        __SEGGER_RTL_INT16_T

#if !defined(__SEGGER_RTL_UINT16_T) && defined(__UINT16_TYPE__)
  #define __SEGGER_RTL_UINT16_T                 __UINT16_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT16_T)
  #define __SEGGER_RTL_UINT16_T                 unsigned short
#endif
#define __SEGGER_RTL_U16                        __SEGGER_RTL_UINT16_T

#if !defined(__SEGGER_RTL_INT32_T) && defined(__INT32_TYPE__)
  #define __SEGGER_RTL_INT32_T                  __INT32_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT32_T)
  #define __SEGGER_RTL_INT32_T                  __SEGGER_RTL_SELECT_TYPESET(long, int, int)
#endif
#define __SEGGER_RTL_I32                        __SEGGER_RTL_INT32_T
#define __SEGGER_RTL_I32_C(X)                   __SEGGER_RTL_INT32_C(X)

#if !defined(__SEGGER_RTL_UINT32_T) && defined(__UINT32_TYPE__)
  #define __SEGGER_RTL_UINT32_T                 __UINT32_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT32_T)
  #define __SEGGER_RTL_UINT32_T                 __SEGGER_RTL_SELECT_TYPESET(unsigned long, unsigned, unsigned)
#endif
#define __SEGGER_RTL_U32                        __SEGGER_RTL_UINT32_T
#define __SEGGER_RTL_U32_C(X)                   __SEGGER_RTL_UINT32_C(X)

#if !defined(__SEGGER_RTL_INT64_T) && defined(__INT64_TYPE__)
  #define __SEGGER_RTL_INT64_T                  __INT64_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT64_T)
  #define __SEGGER_RTL_INT64_T                  __SEGGER_RTL_SELECT_TYPESET(long long, long long, long long)
#endif
#define __SEGGER_RTL_I64                        __SEGGER_RTL_INT64_T
#define __SEGGER_RTL_I64_C(X)                   __SEGGER_RTL_INT64_C(X)

#if !defined(__SEGGER_RTL_UINT64_T) && defined(__UINT64_TYPE__)
  #define __SEGGER_RTL_UINT64_T                 __UINT64_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT64_T)
  #define __SEGGER_RTL_UINT64_T                 __SEGGER_RTL_SELECT_TYPESET(unsigned long long, unsigned long long, unsigned long)
#endif
#define __SEGGER_RTL_U64                        __SEGGER_RTL_UINT64_T
#define __SEGGER_RTL_U64_C(X)                   __SEGGER_RTL_UINT64_C(X)

//
// Types (size at least)
//
#if !defined(__SEGGER_RTL_INT_LEAST8_T) && defined(__INT_LEAST8_TYPE__)
  #define __SEGGER_RTL_INT_LEAST8_T             __INT_LEAST8_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT_LEAST8_T)
  #define __SEGGER_RTL_INT_LEAST8_T             __SEGGER_RTL_I8
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST8_T) && defined(__UINT_LEAST8_TYPE__)
  #define __SEGGER_RTL_UINT_LEAST8_T            __UINT_LEAST8_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST8_T)
  #define __SEGGER_RTL_UINT_LEAST8_T            __SEGGER_RTL_U8
#endif

#if !defined(__SEGGER_RTL_INT_LEAST16_T) && defined(__INT_LEAST16_TYPE__)
  #define __SEGGER_RTL_INT_LEAST16_T              __INT_LEAST16_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT_LEAST16_T)
  #define __SEGGER_RTL_INT_LEAST16_T            __SEGGER_RTL_INT16_T
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST16_T) && defined(__UINT_LEAST16_TYPE__)
  #define __SEGGER_RTL_UINT_LEAST16_T           __UINT_LEAST16_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST16_T)
  #define __SEGGER_RTL_UINT_LEAST16_T           __SEGGER_RTL_U16
#endif

#if !defined(__SEGGER_RTL_INT_LEAST32_T) && defined(__INT_LEAST32_TYPE__)
  #define __SEGGER_RTL_INT_LEAST32_T              __INT_LEAST32_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT_LEAST32_T)
  #define __SEGGER_RTL_INT_LEAST32_T              __SEGGER_RTL_INT32_T
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST32_T) && defined(__UINT_LEAST32_TYPE__)
  #define __SEGGER_RTL_UINT_LEAST32_T           __UINT_LEAST32_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST32_T)
  #define __SEGGER_RTL_UINT_LEAST32_T           __SEGGER_RTL_UINT32_T
#endif

#if !defined(__SEGGER_RTL_INT_LEAST64_T) && defined(__INT_LEAST64_TYPE__)
  #define __SEGGER_RTL_INT_LEAST64_T            __INT_LEAST64_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT_LEAST64_T)
  #define __SEGGER_RTL_INT_LEAST64_T            __SEGGER_RTL_INT64_T
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST64_T) && defined(__UINT_LEAST64_TYPE__)
  #define __SEGGER_RTL_UINT_LEAST64_T           __UINT_LEAST64_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST64_T)
  #define __SEGGER_RTL_UINT_LEAST64_T           __SEGGER_RTL_UINT64_T
#endif
//
// Types (fast with size at least)
//
#if !defined(__SEGGER_RTL_INT_FAST8_T) && defined(__INT_FAST8_TYPE__)
  #define __SEGGER_RTL_INT_FAST8_T              __INT_FAST8_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT_FAST8_T)
  #define __SEGGER_RTL_INT_FAST8_T              __SEGGER_RTL_INT8_T
#endif

#if !defined(__SEGGER_RTL_UINT_FAST8_T) && defined(__UINT_FAST8_TYPE__)
  #define __SEGGER_RTL_UINT_FAST8_T             __UINT_FAST8_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT_FAST8_T)
  #define __SEGGER_RTL_UINT_FAST8_T             __SEGGER_RTL_UINT8_T
#endif

#if !defined(__SEGGER_RTL_INT_FAST16_T) && defined(__INT_FAST16_TYPE__)
  #define __SEGGER_RTL_INT_FAST16_T             __INT_FAST16_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT_FAST16_T)
  #define __SEGGER_RTL_INT_FAST16_T             __SEGGER_RTL_INT16_T
#endif

#if !defined(__SEGGER_RTL_UINT_FAST16_T) && defined(__UINT_FAST16_TYPE__)
  #define __SEGGER_RTL_UINT_FAST16_T            __UINT_FAST16_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT_FAST16_T)
  #define __SEGGER_RTL_UINT_FAST16_T            __SEGGER_RTL_UINT16_T
#endif

#if !defined(__SEGGER_RTL_INT_FAST32_T) && defined(__INT_FAST32_TYPE__)
  #define __SEGGER_RTL_INT_FAST32_T             __INT_FAST32_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT_FAST32_T)
  #define __SEGGER_RTL_INT_FAST32_T             __SEGGER_RTL_INT32_T
#endif

#if !defined(__SEGGER_RTL_UINT_FAST32_T) && defined(__UINT_FAST32_TYPE__)
  #define __SEGGER_RTL_UINT_FAST32_T            __UINT_FAST32_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT_FAST32_T)
  #define __SEGGER_RTL_UINT_FAST32_T            __SEGGER_RTL_UINT32_T
#endif

#if !defined(__SEGGER_RTL_INT_FAST64_T) && defined(__INT_FAST64_TYPE__)
  #define __SEGGER_RTL_INT_FAST64_T             __INT_FAST64_TYPE__
#endif
#if !defined(__SEGGER_RTL_INT_FAST64_T)
  #define __SEGGER_RTL_INT_FAST64_T             __SEGGER_RTL_INT64_T
#endif

#if !defined(__SEGGER_RTL_UINT_FAST64_T) && defined(__UINT_FAST64_TYPE__)
  #define __SEGGER_RTL_UINT_FAST64_T            __UINT_FAST64_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINT_FAST64_T)
  #define __SEGGER_RTL_UINT_FAST64_T            __SEGGER_RTL_UINT64_T
#endif
//
// Types (maximum integer)
//
#if !defined(__SEGGER_RTL_INTMAX_T) && defined(__INTMAX_TYPE__)
  #define __SEGGER_RTL_INTMAX_T                 __INTMAX_TYPE__
#endif
#if !defined(__SEGGER_RTL_INTMAX_T)
  #define __SEGGER_RTL_INTMAX_T                 __SEGGER_RTL_I64
#endif
#if !defined(__SEGGER_RTL_UINTMAX_T) && defined(__UINTMAX_TYPE__)
  #define __SEGGER_RTL_UINTMAX_T                __UINTMAX_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINTMAX_T)
  #define __SEGGER_RTL_UINTMAX_T                __SEGGER_RTL_U64
#endif
//
// Types (architecture types)
//
#if !defined(__SEGGER_RTL_SIZE_T) && defined(__SIZE_TYPE__)
  #define __SEGGER_RTL_SIZE_T                   __SIZE_TYPE__
#endif
#if !defined(__SEGGER_RTL_SIZE_T)
  #define __SEGGER_RTL_SIZE_T                   __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_UINT16, __SEGGER_RTL_UINT32, __SEGGER_RTL_UINT64)
#endif

#if !defined(__SEGGER_RTL_WINT_T) && defined(__WINT_TYPE__)
  #define __SEGGER_RTL_WINT_T                   __WINT_TYPE__
#endif

#if !defined(__SEGGER_RTL_WCHAR_T) && defined(__WCHAR_TYPE__)
  #define __SEGGER_RTL_WCHAR_T                  __WCHAR_TYPE__
#endif

#if !defined(__SEGGER_RTL_SIZEOF_WCHAR_T) && defined(__SIZEOF_WCHAR_T__)
  #define __SEGGER_RTL_SIZEOF_WCHAR_T           __SIZEOF_WCHAR_T__
#endif

#if !defined(__SEGGER_RTL_PTRDIFF_T) && defined(__PTRDIFF_TYPE__)
  #define __SEGGER_RTL_PTRDIFF_T                __PTRDIFF_TYPE__
#endif
#if !defined(__SEGGER_RTL_PTRDIFF_T)
  #define __SEGGER_RTL_PTRDIFF_T                __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_INT16_T, __SEGGER_RTL_INT32_T, __SEGGER_RTL_INT64_T)
#endif

#if !defined(__SEGGER_RTL_INTPTR_T) && defined(__INTPTR_TYPE__)
  #define __SEGGER_RTL_INTPTR_T                 __INTPTR_TYPE__
#endif
#if !defined(__SEGGER_RTL_INTPTR_T)
  #define __SEGGER_RTL_INTPTR_T                 __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_INT16_T, __SEGGER_RTL_INT32_*, __SEGGER_RTL_INT64_T)
#endif

#if !defined(__SEGGER_RTL_UINTPTR_T) && defined(__UINTPTR_TYPE__)
  #define __SEGGER_RTL_UINTPTR_T                __UINTPTR_TYPE__
#endif
#if !defined(__SEGGER_RTL_UINTPTR_T)
  #define __SEGGER_RTL_UINTPTR_T                __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_UINT16_T, __SEGGER_RTL_UINT32_T, __SEGGER_RTL_UINT64_T)
#endif

#if !defined(__SEGGER_RTL_SIG_ATOMIC_T) && defined(__SIG_ATOMIC_TYPE__)
  #define __SEGGER_RTL_SIG_ATOMIC_T             __SIG_ATOMIC_TYPE__
#endif
#if !defined(__SEGGER_RTL_SIG_ATOMIC_T)
  #define __SEGGER_RTL_SIG_ATOMIC_T             __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_INT16_T, __SEGGER_RTL_INT32_T, __SEGGER_RTL_INT64_T)
#endif
//
// Types (128-bit integer, if supported)
//
#if defined(__SEGGER_RTL_HAS_INT128)
  #if !defined(__SEGGER_RTL_UINT128_T)
    #define __SEGGER_RTL_UINT128_T              __uint128_t
  #endif
  #if !defined(__SEGGER_RTL_INT_128_T)
    #define __SEGGER_RTL_INT128_T               __int128_t
  #endif
  #if !defined(__SEGGER_RTL_UINT_LEAST128_T)
    #define __SEGGER_RTL_UINT_LEAST128_T        __uint128_t
  #endif
  #if !defined(__SEGGER_RTL_INT_LEAST128_T)
    #define __SEGGER_RTL_INT_LEAST128_T         __int128_t
  #endif
  #if !defined(__SEGGER_RTL_UINT_FAST128_T)
    #define __SEGGER_RTL_UINT_FAST128_T         __uint128_t
  #endif
  #if !defined(__SEGGER_RTL_INT_FAST128_T)
    #define __SEGGER_RTL_INT_FAST128_T          __int128_t
  #endif
#endif

//
// Minima, maxima
//
#if !defined(__SEGGER_RTL_INT8_MAX) && defined(__INT8_MAX__)
  #define __SEGGER_RTL_INT8_MAX                 __INT8_MAX__ 
#endif
#if !defined(__SEGGER_RTL_INT8_MIN) && defined(__SEGGER_RTL_INT8_MAX)
  #define __SEGGER_RTL_INT8_MIN                 (-__SEGGER_RTL_INT8_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT8_MAX) && defined(__UINT8_MAX__)
  #define __SEGGER_RTL_UINT8_MAX                __UINT8_MAX__
#endif

#if !defined(__SEGGER_RTL_INT16_MAX) && defined(__INT16_MAX__)
  #define __SEGGER_RTL_INT16_MAX                __INT16_MAX__
#endif
#if !defined(__SEGGER_RTL_INT16_MIN) && defined(__SEGGER_RTL_INT16_MAX)
  #define __SEGGER_RTL_INT16_MIN                (-__SEGGER_RTL_INT16_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT16_MAX) && defined(__UINT16_MAX__)
  #define __SEGGER_RTL_UINT16_MAX               __UINT16_MAX__
#endif

#if !defined(__SEGGER_RTL_INT32_MAX) && defined(__INT32_MAX__)
  #define __SEGGER_RTL_INT32_MAX                __INT32_MAX__
#endif
#if !defined(__SEGGER_RTL_INT32_MIN) && defined(__SEGGER_RTL_INT32_MAX)
  #define __SEGGER_RTL_INT32_MIN                (-__SEGGER_RTL_INT32_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT32_MAX) && defined(__UINT32_MAX__)
  #define __SEGGER_RTL_UINT32_MAX               __UINT32_MAX__
#endif

#if !defined(__SEGGER_RTL_INT64_MAX) && defined(__INT64_MAX__)
  #define __SEGGER_RTL_INT64_MAX                __INT64_MAX__
#endif
#if !defined(__SEGGER_RTL_INT64_MIN) && defined(__SEGGER_RTL_INT64_MAX)
  #define __SEGGER_RTL_INT64_MIN                (-__SEGGER_RTL_INT64_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT64_MAX) && defined(__UINT64_MAX__)
  #define __SEGGER_RTL_UINT64_MAX               __UINT64_MAX__
#endif

#if defined(__SEGGER_RTL_HAS_INT128)
  #if !defined(__SEGGER_RTL_INT128_MAX) && defined(__INT128_MAX__)
    #define __SEGGER_RTL_INT128_MAX             __INT128_MAX__
  #endif
  #if !defined(__SEGGER_RTL_INT128_MIN) && defined(__SEGGER_RTL_INT128_MAX)
    #define __SEGGER_RTL_INT128_MIN             (-__SEGGER_RTL_INT128_MAX - 1)
  #endif
  #if !defined(__SEGGER_RTL_UINT128_MAX) && defined(__INT128_MAX__)
    #define __SEGGER_RTL_UINT128_MAX            __INT128_MAX__
  #endif
#endif

//
// Minima, maxima (size at least)
//
#if !defined(__SEGGER_RTL_INT_LEAST8_MAX) && defined(__INT_LEAST8_MAX__)
  #define __SEGGER_RTL_INT_LEAST8_MAX           __INT_LEAST8_MAX__
#endif
#if !defined(__SEGGER_RTL_INT_LEAST8_MAX)
  #define __SEGGER_RTL_INT_LEAST8_MAX           __SEGGER_RTL_INT8_MAX
#endif
#if !defined(__SEGGER_RTL_INT_LEAST8_MIN) && defined(__SEGGER_RTL_INT_LEAST8_MAX)
  #define __SEGGER_RTL_INT_LEAST8_MIN           (-__SEGGER_RTL_INT_LEAST8_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST8_MAX) && defined(__UINT_LEAST8_MAX__)
  #define __SEGGER_RTL_UINT_LEAST8_MAX          __UINT_LEAST8_MAX__
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST8_MAX)
  #define __SEGGER_RTL_UINT_LEAST8_MAX          __SEGGER_RTL_UINT8_MAX
#endif

#if !defined(__SEGGER_RTL_INT_LEAST16_MAX) && defined(__INT_LEAST16_MAX__)
  #define __SEGGER_RTL_INT_LEAST16_MAX          __INT_LEAST16_MAX__
#endif
#if !defined(__SEGGER_RTL_INT_LEAST16_MAX)
  #define __SEGGER_RTL_INT_LEAST16_MAX          __SEGGER_RTL_INT16_MAX
#endif
#if !defined(__SEGGER_RTL_INT_LEAST16_MIN) && defined(__SEGGER_RTL_INT_LEAST16_MAX)
  #define __SEGGER_RTL_INT_LEAST16_MIN          (-__SEGGER_RTL_INT_LEAST16_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST16_MAX) && defined(__UINT_LEAST16_MAX__)
  #define __SEGGER_RTL_UINT_LEAST16_MAX         __UINT_LEAST16_MAX__
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST16_MAX)
  #define __SEGGER_RTL_UINT_LEAST16_MAX         __SEGGER_RTL_UINT16_MAX
#endif

#if !defined(__SEGGER_RTL_INT_LEAST32_MAX) && defined(__INT_LEAST32_MAX__)
  #define __SEGGER_RTL_INT_LEAST32_MAX          __INT_LEAST32_MAX__
#endif
#if !defined(__SEGGER_RTL_INT_LEAST32_MAX)
  #define __SEGGER_RTL_INT_LEAST32_MAX          __SEGGER_RTL_INT32_MAX
#endif
#if !defined(__SEGGER_RTL_INT_LEAST32_MIN) && defined(__SEGGER_RTL_INT_LEAST32_MAX)
  #define __SEGGER_RTL_INT_LEAST32_MIN          (-__SEGGER_RTL_INT_LEAST32_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST32_MAX) && defined(__UINT_LEAST32_MAX__)
  #define __SEGGER_RTL_UINT_LEAST32_MAX         __UINT_LEAST32_MAX__
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST32_MAX)
  #define __SEGGER_RTL_UINT_LEAST32_MAX         __SEGGER_RTL_UINT32_MAX
#endif

#if !defined(__SEGGER_RTL_INT_LEAST64_MAX) && defined(__INT_LEAST64_MAX__)
  #define __SEGGER_RTL_INT_LEAST64_MAX          __INT_LEAST64_MAX__
#endif
#if !defined(__SEGGER_RTL_INT_LEAST64_MAX)
  #define __SEGGER_RTL_INT_LEAST64_MAX          __SEGGER_RTL_INT64_MAX
#endif
#if !defined(__SEGGER_RTL_INT_LEAST64_MIN) && defined(__SEGGER_RTL_INT_LEAST64_MAX)
  #define __SEGGER_RTL_INT_LEAST64_MIN          (-__SEGGER_RTL_INT_LEAST64_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST64_MAX) && defined(__UINT_LEAST64_MAX__)
  #define __SEGGER_RTL_UINT_LEAST64_MAX         __UINT_LEAST64_MAX__
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST64_MAX)
  #define __SEGGER_RTL_UINT_LEAST64_MAX         __SEGGER_RTL_UINT64_MAX
#endif

#if defined(__SEGGER_RTL_HAS_INT128)
  #if !defined(__SEGGER_RTL_INT_LEAST128_MAX) && defined(__INT_LEAST128_MAX__)
    #define __SEGGER_RTL_INT_LEAST128_MAX       __INT_LEAST128_MAX__
  #endif
  #if !defined(__SEGGER_RTL_INT_LEAST128_MAX)
    #define __SEGGER_RTL_INT_LEAST128_MAX       __SEGGER_RTL_INT128_MAX
  #endif
  #if !defined(__SEGGER_RTL_INT_LEAST128_MIN) && defined(__SEGGER_RTL_INT_LEAST128_MAX)
    #define __SEGGER_RTL_INT_LEAST128_MIN       (-__SEGGER_RTL_INT_LEAST128_MAX - 1)
  #endif
  #if !defined(__SEGGER_RTL_UINT_LEAST128_MAX) && defined(__UINT_LEAST128_MAX__)
    #define __SEGGER_RTL_UINT_LEAST128_MAX      __UINT_LEAST128_MAX__
  #endif
  #if !defined(__SEGGER_RTL_UINT_LEAST128_MAX)
    #define __SEGGER_RTL_UINT_LEAST128_MAX      __SEGGER_RTL_UINT128_MAX
  #endif
#endif

//
// Minima, maxima (fast with size at least)
//
#if !defined(__SEGGER_RTL_INT_FAST8_MAX) && defined(__INT_FAST8_MAX__)
  #define __SEGGER_RTL_INT_FAST8_MAX            __INT_FAST8_MAX__
#endif
#if !defined(__SEGGER_RTL_INT_FAST8_MAX)
  #define __SEGGER_RTL_INT_FAST8_MAX            __SEGGER_RTL_INT8_MAX
#endif
#if !defined(__SEGGER_RTL_INT_FAST8_MIN) && defined(__SEGGER_RTL_INT_FAST8_MAX)
  #define __SEGGER_RTL_INT_FAST8_MIN              (-__SEGGER_RTL_INT_FAST8_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT_FAST8_MAX) && defined(__UINT_FAST8_MAX__)
  #define __SEGGER_RTL_UINT_FAST8_MAX           __UINT_FAST8_MAX__
#endif
#if !defined(__SEGGER_RTL_UINT_FAST8_MAX)
  #define __SEGGER_RTL_UINT_FAST8_MAX           __SEGGER_RTL_UINT8_MAX
#endif

#if !defined(__SEGGER_RTL_INT_FAST16_MAX) && defined(__INT_FAST16_MAX__)
  #define __SEGGER_RTL_INT_FAST16_MAX           __INT_FAST16_MAX__
#endif
#if !defined(__SEGGER_RTL_INT_FAST16_MAX)
  #define __SEGGER_RTL_INT_FAST16_MAX           __SEGGER_RTL_INT16_MAX
#endif
#if !defined(__SEGGER_RTL_INT_FAST16_MIN) && defined(__SEGGER_RTL_INT_FAST16_MAX)
  #define __SEGGER_RTL_INT_FAST16_MIN             (-__SEGGER_RTL_INT_FAST16_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT_FAST16_MAX) && defined(__UINT_FAST16_MAX__)
  #define __SEGGER_RTL_UINT_FAST16_MAX          __UINT_FAST16_MAX__
#endif
#if !defined(__SEGGER_RTL_UINT_FAST16_MAX)
  #define __SEGGER_RTL_UINT_FAST16_MAX          __SEGGER_RTL_UINT16_MAX
#endif

#if !defined(__SEGGER_RTL_INT_FAST32_MAX) && defined(__INT_FAST32_MAX__)
  #define __SEGGER_RTL_INT_FAST32_MAX           __INT_FAST32_MAX__
#endif
#if !defined(__SEGGER_RTL_INT_FAST32_MAX)
  #define __SEGGER_RTL_INT_FAST32_MAX           __SEGGER_RTL_INT32_MAX
#endif
#if !defined(__SEGGER_RTL_INT_FAST32_MIN) && defined(__SEGGER_RTL_INT_FAST32_MAX)
  #define __SEGGER_RTL_INT_FAST32_MIN           (-__SEGGER_RTL_INT_FAST32_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT_FAST32_MAX) && defined(__UINT_FAST32_MAX__)
  #define __SEGGER_RTL_UINT_FAST32_MAX            __UINT_FAST32_MAX__
#endif
#if !defined(__SEGGER_RTL_UINT_FAST32_MAX)
  #define __SEGGER_RTL_UINT_FAST32_MAX          __SEGGER_RTL_UINT32_MAX
#endif

#if !defined(__SEGGER_RTL_INT_FAST64_MAX) && defined(__INT_FAST64_MAX__)
  #define __SEGGER_RTL_INT_FAST64_MAX           __INT_FAST64_MAX__
#endif
#if !defined(__SEGGER_RTL_INT_FAST64_MAX)
  #define __SEGGER_RTL_INT_FAST64_MAX           __SEGGER_RTL_INT64_MAX
#endif
#if !defined(__SEGGER_RTL_INT_FAST64_MIN) && defined(__SEGGER_RTL_INT_FAST64_MAX)
  #define __SEGGER_RTL_INT_FAST64_MIN           (-__SEGGER_RTL_INT_FAST64_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINT_FAST64_MAX) && defined(__UINT_FAST64_MAX__)
  #define __SEGGER_RTL_UINT_FAST64_MAX          __UINT_FAST64_MAX__
#endif
#if !defined(__SEGGER_RTL_UINT_FAST64_MAX)
  #define __SEGGER_RTL_UINT_FAST64_MAX          __SEGGER_RTL_UINT64_MAX
#endif

#if defined(__SEGGER_RTL_HAS_INT128)
  #if !defined(__SEGGER_RTL_INT_FAST128_MAX) && defined(__INT_FAST128_MAX__)
    #define __SEGGER_RTL_INT_FAST128_MAX        __INT_FAST128_MAX__
  #endif
  #if !defined(__SEGGER_RTL_INT_FAST128_MAX)
    #define __SEGGER_RTL_INT_FAST128_MAX        __SEGGER_RTL_INT128_MAX
  #endif
  #if !defined(__SEGGER_RTL_INT_FAST128_MIN) && defined(__SEGGER_RTL_INT_FAST128_MAX)
    #define __SEGGER_RTL_INT_FAST128_MIN        (-__SEGGER_RTL_INT_FAST128_MAX - 1)
  #endif
  #if !defined(__SEGGER_RTL_UINT_FAST128_MAX) && defined(__UINT_FAST128_MAX__)
    #define __SEGGER_RTL_UINT_FAST128_MAX       __UINT_FAST128_MAX__
  #endif
  #if !defined(__SEGGER_RTL_UINT_FAST128_MAX)
    #define __SEGGER_RTL_UINT_FAST128_MAX       __SEGGER_RTL_UINT128_MAX
  #endif
#endif

//
// Minima, maxima (maximum integer)
//
#if !defined(__SEGGER_RTL_INTMAX_MAX) && defined(__INTMAX_MAX__)
  #define __SEGGER_RTL_INTMAX_MAX               __INTMAX_MAX__ 
#endif
#if !defined(__SEGGER_RTL_INTMAX_MAX)
  #define __SEGGER_RTL_INTMAX_MAX               __SEGGER_RTL_INT64_MAX
#endif
#if !defined(__SEGGER_RTL_INTMAX_MIN) && defined(__SEGGER_RTL_INTMAX_MAX)
  #define __SEGGER_RTL_INTMAX_MIN               (-__SEGGER_RTL_INTMAX_MAX - 1)
#endif
#if !defined(__SEGGER_RTL_UINTMAX_MAX) && defined(__UINTMAX_MAX__)
  #define __SEGGER_RTL_UINTMAX_MAX              __UINTMAX_MAX__
#endif
#if !defined(__SEGGER_RTL_UINTMAX_MAX)
  #define __SEGGER_RTL_UINTMAX_MAX              __SEGGER_RTL_UINT64_MAX
#endif

//
// Minima, maxima (architecture types)
//
#if !defined(__SEGGER_RTL_SIZE_MAX) && defined(__SIZE_MAX__)
  #define __SEGGER_RTL_SIZE_MAX                 __SIZE_MAX__
#endif
#if !defined(__SEGGER_RTL_SIZE_MAX)
  #define __SEGGER_RTL_SIZE_MAX                 __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_UINT16_MAX, __SEGGER_RTL_UINT32_MAX, __SEGGER_RTL_UINT64_MAX)
#endif

#if !defined(__SEGGER_RTL_WINT_MAX) && defined(__WINT_MAX__)
  #define __SEGGER_RTL_WINT_MAX                 __WINT_MAX__
#endif
#if !defined(__SEGGER_RTL_WINT_MIN) && defined(__WINT_MIN__)
  #define __SEGGER_RTL_WINT_MIN                 __WINT_MIN__
#endif
#if !defined(__SEGGER_RTL_WINT_MIN) && defined(__SEGGER_RTL_WINT_MAX)
  #define __SEGGER_RTL_WINT_MIN                 (-__SEGGER_RTL_WINT_MAX - 1)
#endif

#if !defined(__SEGGER_RTL_WCHAR_MAX) && defined(__WCHAR_MAX__)
  #define __SEGGER_RTL_WCHAR_MAX                __WCHAR_MAX__
#endif
#if !defined(__SEGGER_RTL_WCHAR_MAX) && defined (__SEGGER_RTL_SIZEOF_WCHAR_T) && (__SEGGER_RTL_SIZEOF_WCHAR_T == 2)
  #define __SEGGER_RTL_WCHAR_MAX                65535u
#endif
#if !defined(__SEGGER_RTL_WCHAR_MAX) && defined (__SEGGER_RTL_SIZEOF_WCHAR_T) && (__SEGGER_RTL_SIZEOF_WCHAR_T == 4)
  #define __SEGGER_RTL_WCHAR_MAX                2147483647L
#endif
#if !defined(__SEGGER_RTL_WCHAR_MIN) && defined(__WCHAR_MIN__)
  #define __SEGGER_RTL_WCHAR_MIN                __WCHAR_MIN__
#endif
#if !defined(__SEGGER_RTL_WCHAR_MIN) && defined (__SEGGER_RTL_SIZEOF_WCHAR_T) && (__SEGGER_RTL_SIZEOF_WCHAR_T == 2)
  #define __SEGGER_RTL_WCHAR_MIN                0u
#endif
#if !defined(__SEGGER_RTL_WCHAR_MIN) && defined (__SEGGER_RTL_SIZEOF_WCHAR_T) && (__SEGGER_RTL_SIZEOF_WCHAR_T == 4)
  #define __SEGGER_RTL_WCHAR_MIN                  (-2147483647L - 1)
#endif

#if !defined(__SEGGER_RTL_PTRDIFF_MAX) && defined(__PTRDIFF_MAX__)
  #define __SEGGER_RTL_PTRDIFF_MAX              __PTRDIFF_MAX__
#endif
#if !defined(__SEGGER_RTL_PTRDIFF_MAX)
  #define __SEGGER_RTL_PTRDIFF_MAX              __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_INT16_MAX, __SEGGER_RTL_INT32_MAX, __SEGGER_RTL_INT64_MAX)
#endif
#if !defined(__SEGGER_RTL_PTRDIFF_MIN) && defined(__SEGGER_RTL_PTRDIFF_MAX)
  #define __SEGGER_RTL_PTRDIFF_MIN              (-__SEGGER_RTL_PTRDIFF_MAX - 1)
#endif

#if !defined(__SEGGER_RTL_INTPTR_MAX) && defined(__INTPTR_MAX__)
  #define __SEGGER_RTL_INTPTR_MAX               __INTPTR_MAX__
#endif
#if !defined(__SEGGER_RTL_INTPTR_MAX)
  #define __SEGGER_RTL_INTPTR_MAX               __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_INT16_MAX, __SEGGER_RTL_INT32_MAX, __SEGGER_RTL_INT64_MAX)
#endif
#if !defined(__SEGGER_RTL_INTPTR_MIN) && defined(__SEGGER_RTL_INTPTR_MAX)
  #define __SEGGER_RTL_INTPTR_MIN               (-__SEGGER_RTL_INTPTR_MAX - 1)
#endif
//
#if !defined(__SEGGER_RTL_UINTPTR_MAX) && defined(__UINTPTR_MAX__)
  #define __SEGGER_RTL_UINTPTR_MAX              __UINTPTR_MAX__
#endif
#if !defined(__SEGGER_RTL_UINTPTR_MAX)
  #define __SEGGER_RTL_UINTPTR_MAX              __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_UINT16_MAX, __SEGGER_RTL_UINT32_MAX, __SEGGER_RTL_UINT64_MAX)
#endif

#if !defined(__SEGGER_RTL_SIG_ATOMIC_MAX) && defined(__SIG_ATOMIC_MAX__)
  #define __SEGGER_RTL_SIG_ATOMIC_MAX           __SIG_ATOMIC_MAX__
#endif
#if !defined(__SEGGER_RTL_SIG_ATOMIC_MAX)
  #define __SEGG__SEGGER_RTL_SIG_ATOMIC_MAX     __SEGGER_RTL_SELECT_TYPESET(__SEGGER_RTL_INT16_MAX, __SEGGER_RTL_INT32_MAX, __SEGGER_RTL_INT64_MAX)
#endif
#if !defined(__SEGGER_RTL_SIG_ATOMIC_MIN) && defined(__SEGGER_RTL_SIG_ATOMIC_MAX)
  #define __SEGGER_RTL_SIG_ATOMIC_MIN           0
#endif
//
// Minima, maxima (floats)
//
#if !defined(__SEGGER_RTL_FLT_MAX) && defined(__FLT_MAX__)
  #define __SEGGER_RTL_FLT_MAX                  __FLT_MAX__
#elif !defined(__SEGGER_RTL_FLT_MAX)
  #define __SEGGER_RTL_FLT_MAX                  3.40282347E+38f
#endif
#if !defined(__SEGGER_RTL_FLT_MIN) && defined(__FLT_MIN__)
  #define __SEGGER_RTL_FLT_MIN                  __FLT_MIN__
#elif !defined(__SEGGER_RTL_FLT_MIN)
  #define __SEGGER_RTL_FLT_MIN                  1.17549435e-38f
#endif

#if !defined(__SEGGER_RTL_DBL_MAX) && defined(__DBL_MAX__)
  #define __SEGGER_RTL_DBL_MAX                  __DBL_MAX__
#elif !defined(__SEGGER_RTL_DBL_MAX)
  #define __SEGGER_RTL_DBL_MAX                  1.7976931348623157E+308
#endif

#if !defined(__SEGGER_RTL_DBL_MIN) && defined(__DBL_MIN__)
  #define __SEGGER_RTL_DBL_MIN                  __DBL_MIN__
#elif !defined(__SEGGER_RTL_DBL_MIN)
  #define __SEGGER_RTL_DBL_MIN                  2.2250738585072014e-308
#endif

#if !defined(__SEGGER_RTL_LDBL_MAX) && defined(__LDBL_MAX__)
  #define __SEGGER_RTL_LDBL_MAX                   __LDBL_MAX__
#elif !defined(__SEGGER_RTL_LDBL_MAX)
  #if __SEGGER_RTL_SIZEOF_LDOUBLE == 8
    #define __SEGGER_RTL_LDBL_MAX               1.7976931348623157E+308L
  #elif __SEGGER_RTL_SIZEOF_LDOUBLE == 16
    #define __SEGGER_RTL_LDBL_MAX               1.18973149535723176508575932662800702e+4932L
  #endif
#endif
#if !defined(__SEGGER_RTL_LDBL_MIN) && defined(__LDBL_MIN__)
  #define __SEGGER_RTL_LDBL_MIN                 __LDBL_MIN__
#elif !defined(__SEGGER_RTL_LDBL_MIN)
  #define __SEGGER_RTL_LDBL_MIN                 3.36210314311209350626267781732175260e-4932L
#endif

//
// Width
//
#if !defined(__SEGGER_RTL_INT8_WIDTH) && defined(__INT8_WIDTH__)
  #define __SEGGER_RTL_INT8_WIDTH               __INT8_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT8_WIDTH)
  #define __SEGGER_RTL_INT8_WIDTH               8 
#endif

#if !defined(__SEGGER_RTL_INT16_WIDTH) && defined(__INT16_WIDTH__)
  #define __SEGGER_RTL_INT16_WIDTH              __INT16_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT16_WIDTH)
  #define __SEGGER_RTL_INT16_WIDTH              16 
#endif

#if !defined(__SEGGER_RTL_INT32_WIDTH) && defined(__INT32_WIDTH__)
  #define __SEGGER_RTL_INT32_WIDTH                __INT32_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT32_WIDTH)
  #define __SEGGER_RTL_INT32_WIDTH              32 
#endif

#if !defined(__SEGGER_RTL_INT64_WIDTH) && defined(__INT64_WIDTH__)
  #define __SEGGER_RTL_INT64_WIDTH                __INT64_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT64_WIDTH)
  #define __SEGGER_RTL_INT64_WIDTH              64 
#endif

#if !defined(__SEGGER_RTL_INT128_WIDTH) && defined(__INT128_WIDTH__)
  #define __SEGGER_RTL_INT128_WIDTH               __INT128_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT128_WIDTH)
  #define __SEGGER_RTL_INT128_WIDTH             128 
#endif

#if !defined(__SEGGER_RTL_UINT8_WIDTH) && defined(__INT8_WIDTH__)
  #define __SEGGER_RTL_UINT8_WIDTH                __INT8_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT8_WIDTH)
  #define __SEGGER_RTL_UINT8_WIDTH              8 
#endif

#if !defined(__SEGGER_RTL_UINT16_WIDTH) && defined(__INT16_WIDTH__)
  #define __SEGGER_RTL_UINT16_WIDTH               __INT16_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT16_WIDTH)
  #define __SEGGER_RTL_UINT16_WIDTH             16 
#endif

#if !defined(__SEGGER_RTL_UINT32_WIDTH) && defined(__INT32_WIDTH__)
  #define __SEGGER_RTL_UINT32_WIDTH               __INT32_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT32_WIDTH)
  #define __SEGGER_RTL_UINT32_WIDTH             32 
#endif

#if !defined(__SEGGER_RTL_UINT64_WIDTH) && defined(__INT64_WIDTH__)
  #define __SEGGER_RTL_UINT64_WIDTH             __INT64_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT64_WIDTH)
  #define __SEGGER_RTL_UINT64_WIDTH             64 
#endif

#if !defined(__SEGGER_RTL_UINT128_WIDTH) && defined(__INT128_WIDTH__)
  #define __SEGGER_RTL_UINT128_WIDTH              __INT128_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT128_WIDTH)
  #define __SEGGER_RTL_UINT128_WIDTH            128 
#endif

#if !defined(__SEGGER_RTL_INT_LEAST8_WIDTH) && defined(__INT_LEAST8_WIDTH__)
  #define __SEGGER_RTL_INT_LEAST8_WIDTH         __INT_LEAST8_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_LEAST8_WIDTH)
  #define __SEGGER_RTL_INT_LEAST8_WIDTH         __SEGGER_RTL_INT8_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_LEAST16_WIDTH) && defined(__INT_LEAST16_WIDTH__)
  #define __SEGGER_RTL_INT_LEAST16_WIDTH        __INT_LEAST16_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_LEAST16_WIDTH)
  #define __SEGGER_RTL_INT_LEAST16_WIDTH        __SEGGER_RTL_INT16_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_LEAST32_WIDTH) && defined(__INT_LEAST32_WIDTH__)
  #define __SEGGER_RTL_INT_LEAST32_WIDTH        __INT_LEAST32_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_LEAST32_WIDTH)
  #define __SEGGER_RTL_INT_LEAST32_WIDTH        __SEGGER_RTL_INT32_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_LEAST64_WIDTH) && defined(__INT_LEAST64_WIDTH__)
  #define __SEGGER_RTL_INT_LEAST64_WIDTH        __INT_LEAST64_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_LEAST64_WIDTH)
  #define __SEGGER_RTL_INT_LEAST64_WIDTH        __SEGGER_RTL_INT64_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_LEAST128_WIDTH) && defined(__INT_LEAST128_WIDTH__)
  #define __SEGGER_RTL_INT_LEAST128_WIDTH       __INT_LEAST128_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_LEAST128_WIDTH)
  #define __SEGGER_RTL_INT_LEAST128_WIDTH       __SEGGER_RTL_INT128_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST8_WIDTH) && defined(__INT_LEAST8_WIDTH__)
  #define __SEGGER_RTL_UINT_LEAST8_WIDTH        __INT_LEAST8_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST8_WIDTH)
  #define __SEGGER_RTL_UINT_LEAST8_WIDTH        __SEGGER_RTL_UINT8_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST16_WIDTH) && defined(__INT_LEAST16_WIDTH__)
  #define __SEGGER_RTL_UINT_LEAST16_WIDTH       __INT_LEAST16_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST16_WIDTH)
  #define __SEGGER_RTL_UINT_LEAST16_WIDTH       __SEGGER_RTL_UINT16_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST32_WIDTH) && defined(__INT_LEAST32_WIDTH__)
  #define __SEGGER_RTL_UINT_LEAST32_WIDTH       __INT_LEAST32_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST32_WIDTH)
  #define __SEGGER_RTL_UINT_LEAST32_WIDTH       __SEGGER_RTL_UINT32_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST64_WIDTH) && defined(__INT_LEAST64_WIDTH__)
  #define __SEGGER_RTL_UINT_LEAST64_WIDTH       __INT_LEAST64_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST64_WIDTH)
  #define __SEGGER_RTL_UINT_LEAST64_WIDTH       __SEGGER_RTL_UINT64_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_LEAST128_WIDTH) && defined(__INT_LEAST128_WIDTH__)
  #define __SEGGER_RTL_UINT_LEAST128_WIDTH        __INT_LEAST128_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_LEAST128_WIDTH)
  #define __SEGGER_RTL_UINT_LEAST128_WIDTH        __SEGGER_RTL_UINT128_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_FAST8_WIDTH) && defined(__INT_FAST8_WIDTH__)
  #define __SEGGER_RTL_INT_FAST8_WIDTH            __INT_FAST8_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_FAST8_WIDTH)
  #define __SEGGER_RTL_INT_FAST8_WIDTH            __SEGGER_RTL_INT8_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_FAST16_WIDTH) && defined(__INT_FAST16_WIDTH__)
  #define __SEGGER_RTL_INT_FAST16_WIDTH           __INT_FAST16_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_FAST16_WIDTH)
  #define __SEGGER_RTL_INT_FAST16_WIDTH           __SEGGER_RTL_INT16_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_FAST32_WIDTH) && defined(__INT_FAST32_WIDTH__)
  #define __SEGGER_RTL_INT_FAST32_WIDTH           __INT_FAST32_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_FAST32_WIDTH)
  #define __SEGGER_RTL_INT_FAST32_WIDTH           __SEGGER_RTL_INT32_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_FAST64_WIDTH) && defined(__INT_FAST64_WIDTH__)
  #define __SEGGER_RTL_INT_FAST64_WIDTH           __INT_FAST64_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_FAST64_WIDTH)
  #define __SEGGER_RTL_INT_FAST64_WIDTH           __SEGGER_RTL_INT64_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INT_FAST128_WIDTH) && defined(__INT_FAST128_WIDTH__)
  #define __SEGGER_RTL_INT_FAST128_WIDTH          __INT_FAST128_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_INT_FAST128_WIDTH)
  #define __SEGGER_RTL_INT_FAST128_WIDTH          __SEGGER_RTL_INT128_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_FAST8_WIDTH) && defined(__INT_FAST8_WIDTH__)
  #define __SEGGER_RTL_UINT_FAST8_WIDTH           __INT_FAST8_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_FAST8_WIDTH)
  #define __SEGGER_RTL_UINT_FAST8_WIDTH           __SEGGER_RTL_UINT8_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_FAST16_WIDTH) && defined(__INT_FAST16_WIDTH__)
  #define __SEGGER_RTL_UINT_FAST16_WIDTH          __INT_FAST16_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_FAST16_WIDTH)
  #define __SEGGER_RTL_UINT_FAST16_WIDTH          __SEGGER_RTL_UINT16_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_FAST32_WIDTH) && defined(__INT_FAST32_WIDTH__)
  #define __SEGGER_RTL_UINT_FAST32_WIDTH          __INT_FAST32_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_FAST32_WIDTH)
  #define __SEGGER_RTL_UINT_FAST32_WIDTH          __SEGGER_RTL_UINT32_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_FAST64_WIDTH) && defined(__INT_FAST64_WIDTH__)
  #define __SEGGER_RTL_UINT_FAST64_WIDTH          __INT_FAST64_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_FAST64_WIDTH)
  #define __SEGGER_RTL_UINT_FAST64_WIDTH          __SEGGER_RTL_UINT64_WIDTH 
#endif

#if !defined(__SEGGER_RTL_UINT_FAST128_WIDTH) && defined(__INT_FAST128_WIDTH__)
  #define __SEGGER_RTL_UINT_FAST128_WIDTH         __INT_FAST128_WIDTH__ 
#endif
#if !defined(__SEGGER_RTL_UINT_FAST128_WIDTH)
  #define __SEGGER_RTL_UINT_FAST128_WIDTH         __SEGGER_RTL_UINT128_WIDTH 
#endif

#if !defined(__SEGGER_RTL_INTPTR_WIDTH) && defined(__INTPTR_WIDTH__)
  #define __SEGGER_RTL_INTPTR_WIDTH               __INTPTR_WIDTH__ 
#endif

#if !defined(__SEGGER_RTL_UINTPTR_WIDTH) && defined(__INTPTR_WIDTH__)
  #define __SEGGER_RTL_UINTPTR_WIDTH              __INTPTR_WIDTH__ 
#endif

#if !defined(__SEGGER_RTL_INTMAX_WIDTH) && defined(__INTMAX_WIDTH__)
  #define __SEGGER_RTL_INTMAX_WIDTH               __INTMAX_WIDTH__ 
#endif

#if !defined(__SEGGER_RTL_UINTMAX_WIDTH) && defined(__INTMAX_WIDTH__)
  #define __SEGGER_RTL_UINTMAX_WIDTH              __INTMAX_WIDTH__ 
#endif

#if !defined(__SEGGER_RTL_PTRDIFF_WIDTH) && defined(__PTRDIFF_WIDTH__)
  #define __SEGGER_RTL_PTRDIFF_WIDTH              __PTRDIFF_WIDTH__ 
#endif

#if !defined(__SEGGER_RTL_SIZE_WIDTH) && defined(__SIZE_WIDTH__)
  #define __SEGGER_RTL_SIZE_WIDTH                 __SIZE_WIDTH__ 
#endif

#if !defined(__SEGGER_RTL_WCHAR_WIDTH) && defined(__WCHAR_WIDTH__)
  #define __SEGGER_RTL_WCHAR_WIDTH                __WCHAR_WIDTH__ 
#endif

#if !defined(__SEGGER_RTL_WINT_WIDTH) && defined(__WINT_WIDTH__)
  #define __SEGGER_RTL_WINT_WIDTH                 __WINT_WIDTH__ 
#endif
  //
// Constant creators
// Note: Clang defines __UINT<x>_SUFFIX and GCC defined __UINT<x>_C(x)
  //
#define __CONCAT(X, S)        X ## S
#define __CONCAT1(X, S)       __CONCAT(X, S)
#define __XINTX_C(X, S)       __CONCAT1(X, S)

#if !defined(__SEGGER_RTL_INT8_C) && defined(__INT8_C_SUFFIX__)
  #define __SEGGER_RTL_INT8_C(X)                  __XINTX_C(X,__INT8_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_INT8_C) && defined(__INT8_C)
  #define __SEGGER_RTL_INT8_C(X)                  __INT8_C(X)
#endif

#if !defined(__SEGGER_RTL_UINT8_C) && defined(__UINT8_C)
  #define __SEGGER_RTL_UINT8_C(X)                 __UINT8_C(X)
#elif !defined(__SEGGER_RTL_UINT8_C) && defined(__UINT8_C_SUFFIX__)
  #define __SEGGER_RTL_UINT8_C(X)                 __XINTX_C(X,__UINT8_C_SUFFIX__)
#endif

#if !defined(__SEGGER_RTL_INT16_C) && defined(__INT16_C_SUFFIX__)
  #define __SEGGER_RTL_INT16_C(X)                 __XINTX_C(X,__INT16_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_INT16_C) && defined(__INT16_C)
  #define __SEGGER_RTL_INT16_C(X)                 __INT16_C(X)
#endif

#if !defined(__SEGGER_RTL_UINT16_C) && defined(__UINT16_C_SUFFIX__)
  #define __SEGGER_RTL_UINT16_C(X)                __XINTX_C(X,__UINT16_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_UINT16_C) && defined(__UINT16_C)
  #define __SEGGER_RTL_UINT16_C(X)                __UINT16_C(X)
#endif

#if !defined(__SEGGER_RTL_INT32_C) && defined(__INT32_C_SUFFIX__)
  #define __SEGGER_RTL_INT32_C(X)                 __XINTX_C(X,__INT32_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_INT32_C) && defined(__INT32_C)
  #define __SEGGER_RTL_INT32_C(X)                 __INT32_C(X)
#endif

#if !defined(__SEGGER_RTL_UINT32_C) && defined(__UINT32_C_SUFFIX__)
  #define __SEGGER_RTL_UINT32_C(X)                __XINTX_C(X, __UINT32_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_UINT32_C) && defined(__UINT32_C)
  #define __SEGGER_RTL_UINT32_C(X)                __UINT32_C(X)
#endif

#if !defined(__SEGGER_RTL_INT64_C) && defined(__INT64_C_SUFFIX__)
  #define __SEGGER_RTL_INT64_C(X)                 __XINTX_C(X,__INT64_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_INT64_C) && defined(__INT64_C)
  #define __SEGGER_RTL_INT64_C(X)                 __INT64_C(X)
#endif

#if !defined(__SEGGER_RTL_UINT64_C) && defined(__UINT64_C_SUFFIX__)
  #define __SEGGER_RTL_UINT64_C(X)                __XINTX_C(X,__UINT64_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_UINT64_C) && defined(__UINT64_C)
  #define __SEGGER_RTL_UINT64_C(X)                __UINT64_C(X)
#endif

#if defined(__SEGGER_RTL_HAS_INT128)
  #if !defined(__SEGGER_RTL_INT128_C) && defined(__INT128_C_SUFFIX__)
    #define __SEGGER_RTL_INT128_C(X)              __XINTX_C(X,__INT128_C_SUFFIX__)
  #elif !defined(__SEGGER_RTL_INT128_C) && defined(__INT128_C)
    #define __SEGGER_RTL_INT128_C(X)            __INT128_C(X)
  #endif

  #if !defined(__SEGGER_RTL_UINT128_C) && defined(__UINT128_C_SUFFIX__)
    #define __SEGGER_RTL_UINT128_C(X)           __XINTX_C(X,__UINT128_C_SUFFIX__)
  #elif !defined(__SEGGER_RTL_UINT128_C) && defined(__UINT128_C)
    #define __SEGGER_RTL_UINT128_C(X)           __UINT128_C(X)
  #endif
#endif

#if !defined(__SEGGER_RTL_INTMAX_C) && defined(__INTMAX_C_SUFFIX__)
  #define __SEGGER_RTL_INTMAX_C(X)              __XINTX_C(X,__INTMAX_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_INTMAX_C) && defined(__INTMAX_C)
  #define __SEGGER_RTL_INTMAX_C(X)              __INTMAX_C(X)
#endif

#if !defined(__SEGGER_RTL_UINTMAX_C) && defined(__UINTMAX_C_SUFFIX__)
  #define __SEGGER_RTL_UINTMAX_C(X)             __XINTX_C(X,__UINTMAX_C_SUFFIX__)
#elif !defined(__SEGGER_RTL_UINTMAX_C) && defined(__UINTMAX_C)
  #define __SEGGER_RTL_UINTMAX_C(X)             __UINTMAX_C(X)
#endif
//
// Default types for atomic operation parameters and returns.
//
#if !defined(__SEGGER_RTL_ATOMIC_U8)
  #define __SEGGER_RTL_ATOMIC_U8                __SEGGER_RTL_U8
#endif
#if !defined(__SEGGER_RTL_ATOMIC_U16)
  #define __SEGGER_RTL_ATOMIC_U16               __SEGGER_RTL_U16
#endif
#if !defined(__SEGGER_RTL_ATOMIC_U32)
  #define __SEGGER_RTL_ATOMIC_U32               __SEGGER_RTL_U32
#endif
#if !defined(__SEGGER_RTL_ATOMIC_U64)
  #define __SEGGER_RTL_ATOMIC_U64               __SEGGER_RTL_U64
#endif

#if defined(__SEGGER_RTL_HAS_INT128)
#if !defined(__SEGGER_RTL_ATOMIC_U128) && defined(__SEGGER_RTL_U128)
  #define __SEGGER_RTL_ATOMIC_U128              __SEGGER_RTL_U128
#endif
#endif

// -2 - Favor size at the expense of speed
// -1 - Favor size over speed
//  0 - Balanced
// +1 - Favor speed over size
// +2 - Favor speed at the expense of size
//
#ifndef   __SEGGER_RTL_OPTIMIZE
  #define __SEGGER_RTL_OPTIMIZE                 0
#endif

#ifndef   __SEGGER_RTL_FORMAT_INT_WIDTH
  #define __SEGGER_RTL_FORMAT_INT_WIDTH         __WIDTH_LONG_LONG
#endif

#ifndef   __SEGGER_RTL_FORMAT_FLOAT_WIDTH
  #define __SEGGER_RTL_FORMAT_FLOAT_WIDTH       __WIDTH_DOUBLE
#endif

#ifndef   __SEGGER_RTL_FORMAT_WCHAR
  #define __SEGGER_RTL_FORMAT_WCHAR             1
#endif

#ifndef   __SEGGER_RTL_FORMAT_WIDTH_PRECISION
  #define __SEGGER_RTL_FORMAT_WIDTH_PRECISION   1
#endif

#ifndef   __SEGGER_RTL_FORMAT_CHAR_CLASS
  #define __SEGGER_RTL_FORMAT_CHAR_CLASS        1
#endif

#if __SEGGER_RTL_FORMAT_FLOAT_WIDTH
  #undef  __SEGGER_RTL_FORMAT_INT_WIDTH
  #define __SEGGER_RTL_FORMAT_INT_WIDTH         __WIDTH_LONG_LONG
  #undef  __SEGGER_RTL_FORMAT_WIDTH_PRECISION
  #define __SEGGER_RTL_FORMAT_WIDTH_PRECISION   1
#endif

#ifndef   __SEGGER_RTL_MINIMAL_LOCALE
  #define __SEGGER_RTL_MINIMAL_LOCALE           0
#endif

#ifndef   __SEGGER_RTL_MAX_HEAP_SIZE
  #define __SEGGER_RTL_MAX_HEAP_SIZE            (8*1024*1024u)
#endif

#ifndef   __SEGGER_RTL_ATEXIT_COUNT
  #define __SEGGER_RTL_ATEXIT_COUNT             1
#endif

#ifndef   __SEGGER_RTL_STDOUT_BUFFER_LEN
  #define __SEGGER_RTL_STDOUT_BUFFER_LEN        64
#endif

#ifndef   __SEGGER_RTL_THREAD
  #define __SEGGER_RTL_THREAD __thread
#endif

#ifndef   __SEGGER_RTL_STATE_THREAD
  #define __SEGGER_RTL_STATE_THREAD             __SEGGER_RTL_THREAD   // emRun state variables are thread-local if thread-local storage supported
#endif

#ifndef   __SEGGER_RTL_LOCALE_THREAD
  #define __SEGGER_RTL_LOCALE_THREAD                                  // Locales are not thread-local by default
#endif

#ifndef   __SEGGER_RTL_FP_HW
  #define __SEGGER_RTL_FP_HW                      0
#endif

#ifndef   __SEGGER_RTL_NAN_FORMAT
  #define __SEGGER_RTL_NAN_FORMAT                 __SEGGER_RTL_NAN_FORMAT_IEEE
#endif

#ifndef   __SEGGER_RTL_ATOMIC_LOCK
  #define __SEGGER_RTL_ATOMIC_LOCK()              __SEGGER_RTL_X_atomic_lock()
#endif

#ifndef   __SEGGER_RTL_ATOMIC_UNLOCK
  #define __SEGGER_RTL_ATOMIC_UNLOCK(X)           __SEGGER_RTL_X_atomic_unlock(X)
#endif

#ifndef   __SEGGER_RTL_ATOMIC_SYNCHRONIZE
  #define __SEGGER_RTL_ATOMIC_SYNCHRONIZE()       __SEGGER_RTL_X_atomic_synchronize()
#endif

#ifndef   __SEGGER_RTL_ATOMIC_IS_LOCK_FREE
  #define __SEGGER_RTL_ATOMIC_IS_LOCK_FREE(S,P)   0
#endif

#if !defined(__SEGGER_RTL_OFFSETOF)
  #if defined(__has_builtin) 
    #if __has_builtin(__builtin_offsetof)
      #define __SEGGER_RTL_OFFSETOF(s,m)          __builtin_offsetof(s, m)
    #endif
  #endif
#endif
#if !defined(__SEGGER_RTL_OFFSETOF)
  #define __SEGGER_RTL_OFFSETOF(s,                m)           ((size_t)&(((s *)0)->m))
#endif

//
// Check configuration
//
#ifndef  __SEGGER_RTL_BYTE_ORDER
  #error __SEGGER_RTL_BYTE_ORDER is not configured
#endif

#ifndef  __SEGGER_RTL_MAX_ALIGN
  #error __SEGGER_RTL_MAX_ALIGN is not configured
#endif

#ifndef  __SEGGER_RTL_FP_ABI
  #error __SEGGER_RTL_FP_ABI is not configured
#endif

#ifndef  __SEGGER_RTL_SIZE_T
  #error __SEGGER_RTL_SIZE_T is not configured
#endif

#ifndef  __SEGGER_RTL_PTRDIFF_T
  #error __SEGGER_RTL_PTRDIFF_T is not configured
#endif

#ifndef  __SEGGER_RTL_INT8_T
  #error __SEGGER_RTL_INT8_T is not configured
#endif

#ifndef  __SEGGER_RTL_INT8_MAX
  #error __SEGGER_RTL_INT8_MAX is not configured
#endif

#ifndef  __SEGGER_RTL_INT8_MIN
  #error __SEGGER_RTL_INT8_MIN is not configured
#endif

#ifndef  __SEGGER_RTL_UINT8_T
  #error __SEGGER_RTL_UINT8_T is not configured
#endif

#ifndef  __SEGGER_RTL_UINT8_MAX
  #error __SEGGER_RTL_UINT8_MAX is not configured
#endif

#ifndef  __SEGGER_RTL_INT16_T
  #error __SEGGER_RTL_INT16_T is not configured
#endif

#ifndef  __SEGGER_RTL_INT16_MAX
  #error __SEGGER_RTL_INT16_MAX is not configured
#endif

#ifndef  __SEGGER_RTL_INT16_MIN
  #error __SEGGER_RTL_INT16_MIN is not configured
#endif

#ifndef  __SEGGER_RTL_INT16_T
  #error __SEGGER_RTL_INT16_T is not configured
#endif

#ifndef  __SEGGER_RTL_UINT16_MAX
  #error __SEGGER_RTL_UINT16_MAX is not configured
#endif

#ifndef  __SEGGER_RTL_INT32_T
  #error __SEGGER_RTL_INT32_T is not configured
#endif

#ifndef  __SEGGER_RTL_INT32_MAX
  #error __SEGGER_RTL_INT32_MAX is not configured
#endif

#ifndef  __SEGGER_RTL_INT32_MIN
  #error __SEGGER_RTL_INT32_MIN is not configured
#endif

#ifndef  __SEGGER_RTL_UINT32_T
  #error __SEGGER_RTL_UINT32_T is not configured
#endif

#ifndef  __SEGGER_RTL_UINT32_MAX
  #error __SEGGER_RTL_UINT32_MAX is not configured
#endif

#ifndef  __SEGGER_RTL_INT64_T
  #error __SEGGER_RTL_INT64_T is not configured
#endif

#ifndef  __SEGGER_RTL_INT64_MAX
  #error __SEGGER_RTL_INT64_MAX is not configured
#endif

#ifndef  __SEGGER_RTL_INT64_MIN
  #error __SEGGER_RTL_INT64_MIN is not configured
#endif

#ifndef  __SEGGER_RTL_UINT64_T
  #error __SEGGER_RTL_UINT64_T is not configured
#endif

#ifndef  __SEGGER_RTL_UINT64_MAX
  #error __SEGGER_RTL_UINT64_MAX is not configured
#endif

#if defined(__SEGGER_RTL_HAS_INT128) && !defined(__SEGGER_RTL_INT128_MAX)
  #error __SEGGER_RTL_INT128_MAX is not configured
#endif

#if defined(__SEGGER_RTL_HAS_INT128) && !defined(__SEGGER_RTL_INT128_MIN)
  #error __SEGGER_RTL_INT128_MIN is not configured
#endif

#if defined(__SEGGER_RTL_HAS_INT128) && !defined(__SEGGER_RTL_UINT128_MAX )
  #error __SEGGER_RTL_UINT128_MAX is not configured
#endif

#if defined(__SEGGER_RTL_HAS_INT128) && !defined(__SEGGER_RTL_UINT128_T)
  #error __SEGGER_RTL_UINT128_T is not configured
#endif

#ifndef  __SEGGER_RTL_WINT_T
  #error __SEGGER_RTL_WINT_T is not configured
#endif

#ifndef  __SEGGER_RTL_WINT_MAX
  #error __SEGGER_RTL_WINT_MAX is not configured
#endif

#ifndef  __SEGGER_RTL_WINT_MIN
  #error __SEGGER_RTL_WINT_MIN is not configured
#endif

#ifndef  __SEGGER_RTL_WCHAR_T
  #error __SEGGER_RTL_WCHAR_T is not configured
    #endif

#ifndef  __SEGGER_RTL_WCHAR_MAX
  #error __SEGGER_RTL_WCHAR_MAX is not configured
  #endif

#ifndef  __SEGGER_RTL_WCHAR_MIN
  #error __SEGGER_RTL_WCHAR_MIN is not configured
#endif

#ifndef  __SEGGER_RTL_SIZEOF_WCHAR_T
  #error __SEGGER_RTL_SIZEOF_WCHAR_T is not configured
#endif

#ifndef  __SEGGER_RTL_INT8_C
  #error __SEGGER_RTL_INT8_C is not configured
#endif

#ifndef  __SEGGER_RTL_INT16_C
  #error __SEGGER_RTL_INT16_C is not configured
#endif

#ifndef  __SEGGER_RTL_INT32_C
  #error __SEGGER_RTL_INT32_C is not configured
#endif

#ifndef  __SEGGER_RTL_INT64_C
  #error __SEGGER_RTL_INT64_C is not configured
#endif

#if defined(__SEGGER_RTL_HAS_INT128) && !defined(__SEGGER_RTL_UINT128_C)
  #error __SEGGER_RTL_UINT128_C is not configured
#endif

#ifndef  __SEGGER_RTL_UINT8_C
  #error __SEGGER_RTL_UINT8_C is not configured
#endif

#ifndef  __SEGGER_RTL_UINT16_C
  #error __SEGGER_RTL_UINT16_C is not configured
#endif

#ifndef  __SEGGER_RTL_UINT32_C
  #error __SEGGER_RTL_UINT32_C is not configured
#endif

#ifndef  __SEGGER_RTL_UINT64_C
  #error __SEGGER_RTL_UINT64_C is not configured
#endif

#if defined(__SEGGER_RTL_HAS_INT128) && !defined(__SEGGER_RTL_U128_C)
  #error __SEGGER_RTL_U128_C is not configured
#endif

//
// Check type sizes
//
#if !defined(__SEGGER_RTL_SIZEOF_LONG) || (__SEGGER_RTL_SIZEOF_LONG != 4 && __SEGGER_RTL_SIZEOF_LONG != 8)
  #error __SEGGER_RTL_SIZEOF_LONG is not configured or not configured correctly
#endif

#if !defined(__SEGGER_RTL_SIZEOF_PTR) || (__SEGGER_RTL_SIZEOF_PTR != 2 && __SEGGER_RTL_SIZEOF_PTR != 4 && __SEGGER_RTL_SIZEOF_PTR != 8)
  #error __SEGGER_RTL_SIZEOF_PTR is not configured or not configured correctly
#endif

#if !defined(__SEGGER_RTL_SIZEOF_WCHAR_T) || (__SEGGER_RTL_SIZEOF_WCHAR_T != 2 && __SEGGER_RTL_SIZEOF_WCHAR_T != 4)
  #error __SEGGER_RTL_SIZEOF_WCHAR_T is not configured or not configured correctly
#endif

#if !defined(__SEGGER_RTL_SIZEOF_INT) || (__SEGGER_RTL_SIZEOF_LONG != 2 && __SEGGER_RTL_SIZEOF_LONG != 4 && __SEGGER_RTL_SIZEOF_LONG != 8)
  #error __SEGGER_RTL_SIZEOF_INT is not configured or not configured correctly
#endif

#if __SEGGER_RTL_FORMAT_FLOAT_WIDTH != __WIDTH_NONE && __SEGGER_RTL_FORMAT_FLOAT_WIDTH != __WIDTH_DOUBLE && __SEGGER_RTL_FORMAT_FLOAT_WIDTH != __WIDTH_FLOAT
  #error __SEGGER_RTL_FORMAT_FLOAT_WIDTH is not configured or not configured correctly
#endif

#if __SEGGER_RTL_FORMAT_INT_WIDTH  < __WIDTH_INT || __WIDTH_LONG_LONG < __SEGGER_RTL_FORMAT_INT_WIDTH
  #error __SEGGER_RTL_FORMAT_INT_WIDTH is not configured or not configured correctly
#endif

#ifndef   __SEGGER_RTL_BitcastToU32
  #if defined(__clang) || defined(__SEGGER_CC)
    #define __SEGGER_RTL_BitcastToU32(X)        __builtin_bit_cast(__SEGGER_RTL_U32, (float)(X))
  #else
    #define __SEGGER_RTL_BitcastToU32(X)        __SEGGER_RTL_BitcastToU32_inline(X)
  #endif
#endif

#ifndef   __SEGGER_RTL_BitcastToF32
  #if defined(__clang) || defined(__SEGGER_CC)
    #define __SEGGER_RTL_BitcastToF32(X)        __builtin_bit_cast(float, (__SEGGER_RTL_U32)(X))
  #else
    #define __SEGGER_RTL_BitcastToF32(X)        __SEGGER_RTL_BitcastToF32_inline(X)
  #endif
#endif

#ifndef   __SEGGER_RTL_BitcastToU64
  #if defined(__clang) || defined(__SEGGER_CC)
    #define __SEGGER_RTL_BitcastToU64(X)        __builtin_bit_cast(__SEGGER_RTL_U64, (double)(X))
  #else
    #define __SEGGER_RTL_BitcastToU64(X)        __SEGGER_RTL_BitcastToU64_inline(X)
  #endif
#endif

#ifndef   __SEGGER_RTL_BitcastToF64
  #if defined(__clang) || defined(__SEGGER_CC)
    #define __SEGGER_RTL_BitcastToF64(X)        __builtin_bit_cast(double, (__SEGGER_RTL_U64)(X))
  #else
    #define __SEGGER_RTL_BitcastToF64(X)        __SEGGER_RTL_BitcastToF64_inline(X)
  #endif
#endif

#ifndef   __SEGGER_RTL_SMULL_X
  #define __SEGGER_RTL_SMULL_X(X, Y)            __SEGGER_RTL_SMULL_X_func((X), (Y))
  #define __SEGGER_RTL_SMULL_X_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_UMULL_X
  #define __SEGGER_RTL_UMULL_X(X, Y)            __SEGGER_RTL_UMULL_X_func((X), (Y))
  #define __SEGGER_RTL_UMULL_X_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_SMULL_HI
  #define __SEGGER_RTL_SMULL_HI(X, Y)           __SEGGER_RTL_SMULL_HI_func((X), (Y))
  #define __SEGGER_RTL_SMULL_HI_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_UMULL_HI
  #define __SEGGER_RTL_UMULL_HI(X, Y)           __SEGGER_RTL_UMULL_HI_func((X), (Y))
  #define __SEGGER_RTL_UMULL_HI_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_SMULL
  #define __SEGGER_RTL_SMULL(L,H,X,Y)           __SEGGER_RTL_SMULL_func(&(L), &(H), (X), (Y))
  #define __SEGGER_RTL_SMULL_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_UMULL
  #define __SEGGER_RTL_UMULL(L,H,X,Y)           __SEGGER_RTL_UMULL_func(&(L), &(H), (X), (Y))
  #define __SEGGER_RTL_UMULL_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_SMLAL
  #define __SEGGER_RTL_SMLAL(L,H,X,Y)           __SEGGER_RTL_SMLAL_func(&(L), &(H), (X), (Y))
  #define __SEGGER_RTL_SMLAL_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_UMLAL
  #define __SEGGER_RTL_UMLAL(L,H,X,Y)           __SEGGER_RTL_UMLAL_func(&(L), &(H), (X), (Y))
  #define __SEGGER_RTL_UMLAL_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_FLT_SELECT
  #define __SEGGER_RTL_FLT_SELECT(HEX, DEC)     DEC
#endif

#ifndef   __SEGGER_RTL_DIVMOD_U32
  #define __SEGGER_RTL_DIVMOD_U32(Q, R, N, D)   do { Q = (N) / (D); R = (N) - (Q)*(D); } while (0)
#endif

#ifndef   __SEGGER_RTL_DIVMOD_U64
  #define __SEGGER_RTL_DIVMOD_U64(Q, R, N, D)   do { Q = (N) / (D); R = (N) - (Q)*(D); } while (0)
#endif

#ifndef   __SEGGER_RTL_NEVER_INLINE
  #define __SEGGER_RTL_NEVER_INLINE
#endif

#ifndef   __SEGGER_RTL_ALWAYS_INLINE
  #define __SEGGER_RTL_ALWAYS_INLINE
#endif

#ifndef   __SEGGER_RTL_REQUEST_INLINE
  #define __SEGGER_RTL_REQUEST_INLINE
#endif

#ifndef   __SEGGER_RTL_NO_RETURN
  #if __STDC_VERSION__ >= __SEGGER_RTL_STDC_VERSION_C11
    #define __SEGGER_RTL_NO_RETURN              _Noreturn
  #else
    #define __SEGGER_RTL_NO_RETURN
  #endif
#endif

#ifndef   __SEGGER_RTL_UNLIKELY
  #define __SEGGER_RTL_UNLIKELY(X)              (X)
#endif

#ifndef   __SEGGER_RTL_LIKELY
  #define __SEGGER_RTL_LIKELY(X)                (X)
#endif

#ifndef   __SEGGER_RTL_UNREACHABLE
  #define __SEGGER_RTL_UNREACHABLE()
#endif


#ifndef   __SEGGER_RTL_RODATA_IS_RW
  #define __SEGGER_RTL_RODATA_IS_RW             0
#endif

#ifndef   __SEGGER_RTL_USE_PARA                             // Some compiler complain about unused parameters.
  #define __SEGGER_RTL_USE_PARA(Para)           (void)Para  // This works for most compilers.
#endif

#ifndef   __SEGGER_RTL_SIDE_BY_SIZE
  #define __SEGGER_RTL_SIDE_BY_SIZE             0
#endif

#ifndef   __SEGGER_RTL_SPECIALIZE_COMPARES
  #define __SEGGER_RTL_SPECIALIZE_COMPARES      1
#endif

#ifndef   __SEGGER_RTL_PUBLIC_API
  #define __SEGGER_RTL_PUBLIC_API
#endif

#ifndef   __SEGGER_RTL_CLZ_U32
  #define __SEGGER_RTL_CLZ_U32(X)               __SEGGER_RTL_CLZ_U32_inline(X)
  #define __SEGGER_RTL_CLZ_U32_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_CLZ_U64
  #define __SEGGER_RTL_CLZ_U64(X)               __SEGGER_RTL_CLZ_U64_inline(X)
  #define __SEGGER_RTL_CLZ_U64_SYNTHESIZED
#endif

#ifndef   __SEGGER_RTL_FAST_CODE_SECTION
  #define __SEGGER_RTL_FAST_CODE_SECTION(X)
#endif

#ifndef   __SEGGER_RTL_SCALED_INTEGER
  #define __SEGGER_RTL_SCALED_INTEGER           0
#endif

#ifndef   __SEGGER_RTL_FILE_IMPL
  #define __SEGGER_RTL_FILE_IMPL                __SEGGER_RTL_FILE_impl
#endif

#ifndef   __SEGGER_RTL_U64_H
  #define __SEGGER_RTL_U64_H(X)                 ((__SEGGER_RTL_U32)((__SEGGER_RTL_U64)(X) >> 32))
#endif
#ifndef   __SEGGER_RTL_U64_L
  #define __SEGGER_RTL_U64_L(X)                 ((__SEGGER_RTL_U32)(__SEGGER_RTL_U64)(X))
#endif
#ifndef   __SEGGER_RTL_U64_MK
  #define __SEGGER_RTL_U64_MK(H, L)             (((__SEGGER_RTL_U64)(__SEGGER_RTL_U32)(H) << 32) + (__SEGGER_RTL_U32)(L))
#endif
#ifndef   __SEGGER_RTL_I64_H
  #define __SEGGER_RTL_I64_H(X)                 ((__SEGGER_RTL_I32)((__SEGGER_RTL_I64)(X) >> 32))
#endif
#ifndef   __SEGGER_RTL_I64_L
  #define __SEGGER_RTL_I64_L(X)                 ((__SEGGER_RTL_U32)(__SEGGER_RTL_U64)(X))
#endif

#ifndef   __SEGGER_RTL_UNALIGNED_ATTR
  #define __SEGGER_RTL_UNALIGNED_ATTR
#endif

//
// If complex types are not supported by the compiler, synthesize them
// with a pseudo-complex represented as a structure.
//
#ifndef   __SEGGER_RTL_FLOAT32_C_COMPLEX
  #define __SEGGER_RTL_FLOAT32_C_COMPLEX        __SEGGER_RTL_FLOAT32_PSEUDO_COMPLEX
#endif

#ifndef   __SEGGER_RTL_FLOAT64_C_COMPLEX
  #define __SEGGER_RTL_FLOAT64_C_COMPLEX        __SEGGER_RTL_FLOAT64_PSEUDO_COMPLEX
#endif

//
// GCC and clang provide a built-in support for some math constants.
//
#if defined(__GNUC__) || defined(__clang__)
  #ifndef   __SEGGER_RTL_INFINITY
    #define __SEGGER_RTL_INFINITY               __builtin_inff()
  #endif
  
  #ifndef   __SEGGER_RTL_NAN 
    #define __SEGGER_RTL_NAN                    __builtin_nanf("0x7fc00000")
  #endif
  
  #ifndef   __SEGGER_RTL_HUGE_VAL
    #define __SEGGER_RTL_HUGE_VAL               __builtin_huge_val()
  #endif
  
  #ifndef   __SEGGER_RTL_HUGE_VALF
    #define __SEGGER_RTL_HUGE_VALF              __builtin_huge_valf()
  #endif

  #ifndef   __SEGGER_RTL_HUGE_VALL
    #define __SEGGER_RTL_HUGE_VALL              __builtin_huge_vall()
  #endif
#endif

//
// Sanity checks
//
#if (__SEGGER_RTL_INCLUDE_GNU_API == 2) && (defined(__ARM_ARCH_ISA_ARM) || defined(__ARM_ARCH_ISA_THUMB))
  #error The GNU API is not supported for the Arm architecture with assembly speedups
#endif

#if (__SEGGER_RTL_SIDE_BY_SIDE > 0) && (__SEGGER_RTL_INCLUDE_SEGGER_API == 0)
  #error __SEGGER_RTL_SIDE_BY_SIDE selected requires __SEGGER_RTL_INCLUDE_SEGGER_API selected
#endif

// Private macros.
#define __SEGGER_RTL_HIDE(X)                    __c_##X

#if __SEGGER_RTL_RODATA_IS_RW
  #define __SEGGER_RTL_RODATA
#else
  #define __SEGGER_RTL_RODATA                   const
#endif

//
// Private configuration.  This is work in progress and cannot be
// otherwise configured.
//
#define __SEGGER_RTL_SUBNORMALS_READ_AS_ZERO    1  // Incoming subnormals are read as a signed zero.
#define __SEGGER_RTL_SUBNORMALS_FLUSH_TO_ZERO   1  // Outgoing subnormals are generated as a signed zero.

#if __SEGGER_RTL_STDOUT_BUFFER_LEN <= 0
  #error Bad configuration of __SEGGER_RTL_STDOUT_BUFFER_LEN
#endif

#if __SEGGER_RTL_ATEXIT_COUNT <= 0
  #error Bad configuration of __SEGGER_RTL_ATEXIT_COUNT
#endif

#ifndef   __SEGGER_RTL_CORE_HAS_UDIV_X
  #define __SEGGER_RTL_CORE_HAS_UDIV_X          0
#endif

#ifndef   __SEGGER_RTL_CORE_HAS_IDIV_X
  #define __SEGGER_RTL_CORE_HAS_IDIV_X          0
#endif

#ifndef   __SEGGER_RTL_CORE_HAS_UDIVM_X
  #define __SEGGER_RTL_CORE_HAS_UDIVM_X         0
#endif

#ifndef   __SEGGER_RTL_CORE_HAS_IDIVM_X
  #define __SEGGER_RTL_CORE_HAS_IDIVM_X         0
#endif

#ifndef   __SEGGER_RTL_NO_BUILTIN
  #error  __SEGGER_RTL_NO_BUILTIN must be defined for your target compiler!
#endif

//
// Configuration of signal numbers and handler functions.
//
#ifndef __SEGGER_RTL_SIGNAL_MAX                 // If this is not defined, then default implementation.
  #define __SEGGER_RTL_SIGNAL_SIGABRT           0
  #define __SEGGER_RTL_SIGNAL_SIGFPE            1
  #define __SEGGER_RTL_SIGNAL_SIGILL            2
  #define __SEGGER_RTL_SIGNAL_SIGINT            3
  #define __SEGGER_RTL_SIGNAL_SIGSEGV           4
  #define __SEGGER_RTL_SIGNAL_SIGTERM           5
  #define __SEGGER_RTL_SIGNAL_MAX               6  // Maximum number of signals
  //
  // These are functions.
  //
  #ifndef __SEGGER_RTL_EXCLUDE_STATIC_FUNCTIONS
    void __SEGGER_RTL_SIGNAL_SIG_DFL(int);
    void __SEGGER_RTL_SIGNAL_SIG_ERR(int);
    void __SEGGER_RTL_SIGNAL_SIG_IGN(int);
  #endif
#endif

#endif

/*************************** End of file ****************************/
