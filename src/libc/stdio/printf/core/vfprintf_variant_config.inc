/*
 * Variant configuration layer.
 *
 * This file answers "which printf_core variant are we compiling?".
 * It defines:
 * - exported symbol name
 * - character model (narrow/wide)
 * - feature macros (_NEED_IO_*)
 * - float argument extraction rules
 * - integer argument width rules
 * - base scratch-buffer sizing
 */

#ifndef PRINTF_NAME
#define PRINTF_VARIANT __IO_VARIANT_DOUBLE
#define PRINTF_NAME    __d_vfprintf
#endif

#include "../printf_core_private.h"
#include "../../../libm/common/math_config.h"
#include <stdio.h>

/*
 * This file can be compiled into more than one flavour by setting
 * PRINTF_VARIANT to:
 *
 *  __IO_VARIANT_MINIMAL: limited integer-only support with option for long long
 *
 *  __IO_VARIANT_INTEGER: integer support except for long long with
 *              options for positional params
 *
 *  __IO_VARIANT_LLONG: full integer support including long long with
 *                options for positional params
 *
 *  __IO_VARIANT_FLOAT: full integer support along with float, but not double
 *
 *  __IO_VARIANT_DOUBLE: full support
 */

#if __IO_DEFAULT != PRINTF_VARIANT || defined(WIDE_CHARS)
#define vfprintf PRINTF_NAME
#endif

#ifdef WIDE_CHARS
#define CHAR wchar_t
#if __SIZEOF_WCHAR_T__ == 2
#define UCHAR uint16_t
#elif __SIZEOF_WCHAR_T__ == 4
#define UCHAR uint32_t
#endif
#else
#define CHAR  char
#define UCHAR unsigned char
#endif

/*
 * Compute which features are required. This should match the _HAS
 * values computed in stdio.h
 */

#if PRINTF_VARIANT == __IO_VARIANT_MINIMAL

#define _NEED_IO_SHRINK
#if defined(__IO_MINIMAL_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#ifdef __IO_C99_FORMATS
#define _NEED_IO_C99_FORMATS
#endif

#elif PRINTF_VARIANT == __IO_VARIANT_INTEGER

#if defined(__IO_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#ifdef __IO_POS_ARGS
#define _NEED_IO_POS_ARGS
#endif
#ifdef __IO_C99_FORMATS
#define _NEED_IO_C99_FORMATS
#endif
#ifdef __IO_PERCENT_B
#define _NEED_IO_PERCENT_B
#endif

#elif PRINTF_VARIANT == __IO_VARIANT_LLONG

#if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#ifdef __IO_POS_ARGS
#define _NEED_IO_POS_ARGS
#endif
#ifdef __IO_C99_FORMATS
#define _NEED_IO_C99_FORMATS
#endif
#ifdef __IO_PERCENT_B
#define _NEED_IO_PERCENT_B
#endif

#elif PRINTF_VARIANT == __IO_VARIANT_FLOAT

#if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#define _NEED_IO_POS_ARGS
#define _NEED_IO_C99_FORMATS
#ifdef __IO_PERCENT_B
#define _NEED_IO_PERCENT_B
#endif
#define _NEED_IO_FLOAT

#elif PRINTF_VARIANT == __IO_VARIANT_DOUBLE

#if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#if defined(_HAS_IO_WCHAR) || defined(WIDE_CHARS)
#define _NEED_IO_WCHAR
#endif
#define _NEED_IO_POS_ARGS
#define _NEED_IO_C99_FORMATS
#ifdef __IO_PERCENT_B
#define _NEED_IO_PERCENT_B
#endif
#define _NEED_IO_DOUBLE
#if defined(__IO_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#define _NEED_IO_LONG_DOUBLE
#endif

#else

#error invalid PRINTF_VARIANT

#endif

/* Figure out which multi-byte char support we need */
#if defined(_NEED_IO_WCHAR) && defined(__MB_CAPABLE)
#ifdef WIDE_CHARS
#define _NEED_IO_MBTOWIDE
#else
#define _NEED_IO_WIDETOMB
#endif
#endif

#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
#include "../dtoa.h"
#endif

#if defined(_NEED_IO_FLOAT_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#define SKIP_FLOAT_ARG(flags, ap)                                           \
    do {                                                                    \
        if ((flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE)) \
            (void)va_arg(ap, long double);                                  \
        else                                                                \
            (void)va_arg(ap, double);                                       \
    } while (0)
#define FLOAT double
#elif defined(_NEED_IO_FLOAT)
#define SKIP_FLOAT_ARG(flags, ap) (void)va_arg(ap, uint32_t)
#define FLOAT_UINT                uint32_t
#else
#define SKIP_FLOAT_ARG(flags, ap) (void)va_arg(ap, double)
#if __SIZEOF_DOUBLE__ == 8
#define FLOAT_UINT uint64_t
#elif __SIZEOF_DOUBLE__ == 4
#define FLOAT_UINT uint32_t
#endif
#endif

#ifdef _NEED_IO_LONG_LONG
typedef unsigned long long ultoa_unsigned_t;
typedef long long          ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG_LONG__
#define arg_to_t(ap, flags, _s_, _result_)          \
    if ((flags) & FL_LONG) {                        \
        if ((flags) & FL_REPD_TYPE)                 \
            (_result_) = va_arg(ap, _s_ long long); \
        else                                        \
            (_result_) = va_arg(ap, _s_ long);      \
    } else {                                        \
        (_result_) = va_arg(ap, _s_ int);           \
        if ((flags) & FL_SHORT) {                   \
            if ((flags) & FL_REPD_TYPE)             \
                (_result_) = (_s_ char)(_result_);  \
            else                                    \
                (_result_) = (_s_ short)(_result_); \
        }                                           \
    }
#else
typedef unsigned long ultoa_unsigned_t;
typedef long          ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG__
#define arg_to_t(ap, flags, _s_, _result_)                    \
    if ((flags) & FL_LONG) {                                  \
        if ((flags) & FL_REPD_TYPE)                           \
            (_result_) = (_s_ long)va_arg(ap, _s_ long long); \
        else                                                  \
            (_result_) = va_arg(ap, _s_ long);                \
    } else {                                                  \
        (_result_) = va_arg(ap, _s_ int);                     \
        if ((flags) & FL_SHORT) {                             \
            if ((flags) & FL_REPD_TYPE)                       \
                (_result_) = (_s_ char)(_result_);            \
            else                                              \
                (_result_) = (_s_ short)(_result_);           \
        }                                                     \
    }
#endif

#if SIZEOF_ULTOA <= 4
#ifdef _NEED_IO_PERCENT_B
#define PRINTF_BUF_SIZE 32
#else
#define PRINTF_BUF_SIZE 11
#endif
#else
#ifdef _NEED_IO_PERCENT_B
#define PRINTF_BUF_SIZE 64
#else
#define PRINTF_BUF_SIZE 22
#endif
#endif

#define arg_to_unsigned(ap, flags, result_var) arg_to_t(ap, flags, unsigned, result_var)
#define arg_to_signed(ap, flags, result_var)   arg_to_t(ap, flags, signed, result_var)
