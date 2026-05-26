#ifndef _MICROCRT_PRINTF_CONFIG_H_
#define _MICROCRT_PRINTF_CONFIG_H_

/*
 * Shared printf configuration for one compiled core variant.
 *
 * This file is the only place that should define:
 * - exported core symbol selection
 * - profile selection
 * - compile-time capability table
 * - runtime parser/formatter flag values
 * - integer/float argument extraction rules derived from the capability table
 */

#ifndef PRINTF_CORE_SYMBOL
#ifndef PRINTF_NAME
#define PRINTF_CORE_SYMBOL __printf_core_default
#else
#define PRINTF_CORE_SYMBOL PRINTF_NAME
#endif
#endif

#ifndef PRINTF_CORE_PROFILE
#ifndef PRINTF_VARIANT
#define PRINTF_CORE_PROFILE __IO_VARIANT_DOUBLE
#else
#define PRINTF_CORE_PROFILE PRINTF_VARIANT
#endif
#endif

/*
 * Compatibility aliases kept for legacy include paths.
 * New code should use PRINTF_CORE_* directly.
 */
#ifndef PRINTF_NAME
#define PRINTF_NAME PRINTF_CORE_SYMBOL
#endif

#ifndef PRINTF_VARIANT
#define PRINTF_VARIANT PRINTF_CORE_PROFILE
#endif

#ifndef IO_VARIANT_IS_FLOAT
#define IO_VARIANT_IS_FLOAT(v) ((v) == __IO_VARIANT_FLOAT || (v) == __IO_VARIANT_DOUBLE)
#endif

#ifdef WIDE_CHARS
#define CHAR wchar_t
#else
#define CHAR char
#endif

#ifndef PRINTF_CAP_SHRINK
#define PRINTF_CAP_SHRINK 0
#endif
#ifndef PRINTF_CAP_LONG_LONG
#define PRINTF_CAP_LONG_LONG 0
#endif
#ifndef PRINTF_CAP_POSITIONAL
#define PRINTF_CAP_POSITIONAL 0
#endif
#ifndef PRINTF_CAP_C99_FORMATS
#define PRINTF_CAP_C99_FORMATS 0
#endif
#ifndef PRINTF_CAP_BINARY
#define PRINTF_CAP_BINARY 0
#endif
#ifndef PRINTF_CAP_FLOAT
#define PRINTF_CAP_FLOAT 0
#endif
#ifndef PRINTF_CAP_DOUBLE
#define PRINTF_CAP_DOUBLE 0
#endif
#ifndef PRINTF_CAP_LONG_DOUBLE
#define PRINTF_CAP_LONG_DOUBLE 0
#endif
#ifndef PRINTF_CAP_WCHAR
#define PRINTF_CAP_WCHAR 0
#endif
#ifndef PRINTF_CAP_MBTOWIDE
#define PRINTF_CAP_MBTOWIDE 0
#endif
#ifndef PRINTF_CAP_WIDETOMB
#define PRINTF_CAP_WIDETOMB 0
#endif
#ifndef PRINTF_CAP_SECURE
#define PRINTF_CAP_SECURE 0
#endif
#ifndef PRINTF_CAP_PERCENT_N
#ifdef __IO_PERCENT_N
#define PRINTF_CAP_PERCENT_N 1
#else
#define PRINTF_CAP_PERCENT_N 0
#endif
#endif

#if PRINTF_CORE_PROFILE == __IO_VARIANT_MINIMAL
#undef PRINTF_CAP_SHRINK
#define PRINTF_CAP_SHRINK 1
#undef PRINTF_CAP_LONG_LONG
#define PRINTF_CAP_LONG_LONG 0
#undef PRINTF_CAP_POSITIONAL
#define PRINTF_CAP_POSITIONAL 0
#undef PRINTF_CAP_C99_FORMATS
#define PRINTF_CAP_C99_FORMATS 0
#undef PRINTF_CAP_BINARY
#define PRINTF_CAP_BINARY 0
#undef PRINTF_CAP_FLOAT
#define PRINTF_CAP_FLOAT 0
#undef PRINTF_CAP_DOUBLE
#define PRINTF_CAP_DOUBLE 0
#undef PRINTF_CAP_LONG_DOUBLE
#define PRINTF_CAP_LONG_DOUBLE 0
#undef PRINTF_CAP_WCHAR
#define PRINTF_CAP_WCHAR 0
#endif

#if PRINTF_CORE_PROFILE == __IO_VARIANT_INTEGER
#undef PRINTF_CAP_SHRINK
#define PRINTF_CAP_SHRINK 0
#undef PRINTF_CAP_LONG_LONG
#define PRINTF_CAP_LONG_LONG 1
#undef PRINTF_CAP_POSITIONAL
#define PRINTF_CAP_POSITIONAL 1
#undef PRINTF_CAP_C99_FORMATS
#define PRINTF_CAP_C99_FORMATS 1
#undef PRINTF_CAP_BINARY
#define PRINTF_CAP_BINARY 1
#undef PRINTF_CAP_FLOAT
#define PRINTF_CAP_FLOAT 0
#undef PRINTF_CAP_DOUBLE
#define PRINTF_CAP_DOUBLE 0
#undef PRINTF_CAP_LONG_DOUBLE
#define PRINTF_CAP_LONG_DOUBLE 0
#undef PRINTF_CAP_WCHAR
#define PRINTF_CAP_WCHAR 1
#endif

#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
#undef PRINTF_CAP_SHRINK
#define PRINTF_CAP_SHRINK 0
#undef PRINTF_CAP_LONG_LONG
#define PRINTF_CAP_LONG_LONG 1
#undef PRINTF_CAP_POSITIONAL
#define PRINTF_CAP_POSITIONAL 1
#undef PRINTF_CAP_C99_FORMATS
#define PRINTF_CAP_C99_FORMATS 1
#undef PRINTF_CAP_BINARY
#define PRINTF_CAP_BINARY 1
#undef PRINTF_CAP_FLOAT
#define PRINTF_CAP_FLOAT 0
#undef PRINTF_CAP_DOUBLE
#define PRINTF_CAP_DOUBLE 1
#undef PRINTF_CAP_LONG_DOUBLE
#define PRINTF_CAP_LONG_DOUBLE 1
#undef PRINTF_CAP_WCHAR
#define PRINTF_CAP_WCHAR 1
#endif

/* Multi-byte support is derived from wchar capability and host support. */
#if PRINTF_CAP_WCHAR && defined(__MB_CAPABLE)
#ifdef WIDE_CHARS
#undef PRINTF_CAP_MBTOWIDE
#define PRINTF_CAP_MBTOWIDE 1
#else
#undef PRINTF_CAP_WIDETOMB
#define PRINTF_CAP_WIDETOMB 1
#endif
#endif

/*
 * Compatibility aliases kept for legacy helpers and upstream fragments.
 * New code should use PRINTF_CAP_* directly.
 */
#if PRINTF_CAP_SHRINK
#define _NEED_IO_SHRINK
#endif
#if PRINTF_CAP_LONG_LONG
#define _NEED_IO_LONG_LONG
#endif
#if PRINTF_CAP_POSITIONAL
#define _NEED_IO_POS_ARGS
#endif
#if PRINTF_CAP_C99_FORMATS
#define _NEED_IO_C99_FORMATS
#endif
#if PRINTF_CAP_BINARY
#define _NEED_IO_PERCENT_B
#endif
#if PRINTF_CAP_FLOAT
#define _NEED_IO_FLOAT
#endif
#if PRINTF_CAP_DOUBLE
#define _NEED_IO_DOUBLE
#endif
#if PRINTF_CAP_LONG_DOUBLE
#define _NEED_IO_LONG_DOUBLE
#endif
#if PRINTF_CAP_WCHAR
#define _NEED_IO_WCHAR
#endif
#if PRINTF_CAP_MBTOWIDE
#define _NEED_IO_MBTOWIDE
#endif
#if PRINTF_CAP_WIDETOMB
#define _NEED_IO_WIDETOMB
#endif

/* Order matches the format-string parser state machine. */
#if PRINTF_CAP_SHRINK
#define PRINTF_FLAG_ZERO_FILL 0x0000
#define PRINTF_FLAG_PLUS      0x0000
#define PRINTF_FLAG_SPACE     0x0000
#define PRINTF_FLAG_LEFT_ADJ  0x0000
#else
#define PRINTF_FLAG_ZERO_FILL 0x0001
#define PRINTF_FLAG_PLUS      0x0002
#define PRINTF_FLAG_SPACE     0x0004
#define PRINTF_FLAG_LEFT_ADJ  0x0008
#endif

#define PRINTF_FLAG_ALT_FORM    0x0010
#define PRINTF_FLAG_WIDTH       0x0020
#define PRINTF_FLAG_PRECISION   0x0040
#define PRINTF_FLAG_LONG        0x0080
#define PRINTF_FLAG_SHORT       0x0100
#define PRINTF_FLAG_REPEAT_TYPE 0x0200
#define PRINTF_FLAG_NEGATIVE    0x0400

#if PRINTF_CAP_C99_FORMATS
#define PRINTF_FLAG_FLOAT_HEX 0x0800
#endif

#define PRINTF_FLAG_FLOAT_EXP 0x1000
#define PRINTF_FLAG_FLOAT_FIX 0x2000

/* Compatibility aliases kept for legacy include paths only. */
#define FL_ZFILL     PRINTF_FLAG_ZERO_FILL
#define FL_PLUS      PRINTF_FLAG_PLUS
#define FL_SPACE     PRINTF_FLAG_SPACE
#define FL_LPAD      PRINTF_FLAG_LEFT_ADJ
#define FL_ALT       PRINTF_FLAG_ALT_FORM
#define FL_WIDTH     PRINTF_FLAG_WIDTH
#define FL_PREC      PRINTF_FLAG_PRECISION
#define FL_LONG      PRINTF_FLAG_LONG
#define FL_SHORT     PRINTF_FLAG_SHORT
#define FL_REPD_TYPE PRINTF_FLAG_REPEAT_TYPE
#define FL_NEGATIVE  PRINTF_FLAG_NEGATIVE
#if PRINTF_CAP_C99_FORMATS
#define FL_FLTHEX PRINTF_FLAG_FLOAT_HEX
#endif
#define FL_FLTEXP PRINTF_FLAG_FLOAT_EXP
#define FL_FLTFIX PRINTF_FLAG_FLOAT_FIX

#include "printf_core_private.h"
#include "../../../libm/common/math_config.h"

#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
#include "dtoa.h"
#endif

#if PRINTF_CAP_LONG_DOUBLE && PRINTF_FLOAT_CAP_LARGE
#define SKIP_FLOAT_ARG(flags, ap)                                                           \
    do {                                                                                    \
        if (((flags) & (PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE))                        \
            == (PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE))                                \
            (void)va_arg(ap, long double);                                                  \
        else                                                                                \
            (void)va_arg(ap, double);                                                       \
    } while (0)
#define FLOAT double
#if __SIZEOF_DOUBLE__ == 8
#define FLOAT_UINT uint64_t
#elif __SIZEOF_DOUBLE__ == 4
#define FLOAT_UINT uint32_t
#endif
#elif PRINTF_CAP_FLOAT
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

#if PRINTF_CAP_LONG_LONG
typedef unsigned long long ultoa_unsigned_t;
typedef long long          ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG_LONG__
#define arg_to_t(ap, flags, _s_, _result_)                  \
    if ((flags) & PRINTF_FLAG_LONG) {                       \
        if ((flags) & PRINTF_FLAG_REPEAT_TYPE)              \
            (_result_) = va_arg(ap, _s_ long long);         \
        else                                                \
            (_result_) = va_arg(ap, _s_ long);              \
    } else {                                                \
        (_result_) = va_arg(ap, _s_ int);                   \
        if ((flags) & PRINTF_FLAG_SHORT) {                  \
            if ((flags) & PRINTF_FLAG_REPEAT_TYPE)          \
                (_result_) = (_s_ char)(_result_);          \
            else                                            \
                (_result_) = (_s_ short)(_result_);         \
        }                                                   \
    }
#else
typedef unsigned long ultoa_unsigned_t;
typedef long          ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG__
#define arg_to_t(ap, flags, _s_, _result_)                            \
    if ((flags) & PRINTF_FLAG_LONG) {                                 \
        if ((flags) & PRINTF_FLAG_REPEAT_TYPE)                        \
            (_result_) = (_s_ long)va_arg(ap, _s_ long long);         \
        else                                                          \
            (_result_) = va_arg(ap, _s_ long);                        \
    } else {                                                          \
        (_result_) = va_arg(ap, _s_ int);                             \
        if ((flags) & PRINTF_FLAG_SHORT) {                            \
            if ((flags) & PRINTF_FLAG_REPEAT_TYPE)                    \
                (_result_) = (_s_ char)(_result_);                    \
            else                                                      \
                (_result_) = (_s_ short)(_result_);                   \
        }                                                             \
    }
#endif

#define arg_to_unsigned(ap, flags, _result_) arg_to_t(ap, flags, unsigned, _result_)
#define arg_to_signed(ap, flags, _result_)   arg_to_t(ap, flags, signed, _result_)

#if SIZEOF_ULTOA <= 4
#if PRINTF_CAP_BINARY
#define PRINTF_BUF_SIZE 32
#else
#define PRINTF_BUF_SIZE 11
#endif
#else
#if PRINTF_CAP_BINARY
#define PRINTF_BUF_SIZE 64
#else
#define PRINTF_BUF_SIZE 22
#endif
#endif

#endif
