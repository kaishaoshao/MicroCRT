#ifndef _MICROCRT_PRINTF_CONFIG_H_
#define _MICROCRT_PRINTF_CONFIG_H_

#include <stdint.h>

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
#define PRINTF_CORE_SYMBOL __printf_core_default
#endif

#ifndef PRINTF_CORE_PROFILE
#define PRINTF_CORE_PROFILE __IO_VARIANT_DOUBLE
#endif

#ifndef IO_VARIANT_IS_FLOAT
#define IO_VARIANT_IS_FLOAT(v) ((v) == __IO_VARIANT_FLOAT || (v) == __IO_VARIANT_DOUBLE)
#endif

#ifdef WIDE_CHARS
#define CHAR wchar_t
#else
#define CHAR char
#endif

/*
 * Capability resolution rule:
 *
 * 1. Built-in defaults
 * 2. Profile defaults from PRINTF_CORE_PROFILE
 * 3. Explicit PRINTF_CAP_* definitions in the core shell
 *
 * In other words:
 * explicit capability > profile default > built-in default
 */

#define PRINTF_PROFILE_CAP_SHRINK      0
#define PRINTF_PROFILE_CAP_LONG_LONG   0
#define PRINTF_PROFILE_CAP_POSITIONAL  0
#define PRINTF_PROFILE_CAP_C99_FORMATS 0
#define PRINTF_PROFILE_CAP_BINARY      0
#define PRINTF_PROFILE_CAP_FLOAT       0
#define PRINTF_PROFILE_CAP_DOUBLE      0
#define PRINTF_PROFILE_CAP_LONG_DOUBLE 0
#define PRINTF_PROFILE_CAP_WCHAR       0

#if PRINTF_CORE_PROFILE == __IO_VARIANT_MINIMAL
#undef PRINTF_PROFILE_CAP_SHRINK
#define PRINTF_PROFILE_CAP_SHRINK 1
#endif

#if PRINTF_CORE_PROFILE == __IO_VARIANT_INTEGER
#undef PRINTF_PROFILE_CAP_LONG_LONG
#define PRINTF_PROFILE_CAP_LONG_LONG 1
#undef PRINTF_PROFILE_CAP_POSITIONAL
#define PRINTF_PROFILE_CAP_POSITIONAL 1
#undef PRINTF_PROFILE_CAP_C99_FORMATS
#define PRINTF_PROFILE_CAP_C99_FORMATS 1
#undef PRINTF_PROFILE_CAP_BINARY
#define PRINTF_PROFILE_CAP_BINARY 1
#undef PRINTF_PROFILE_CAP_WCHAR
#define PRINTF_PROFILE_CAP_WCHAR 1
#endif

#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
#undef PRINTF_PROFILE_CAP_LONG_LONG
#define PRINTF_PROFILE_CAP_LONG_LONG 1
#undef PRINTF_PROFILE_CAP_POSITIONAL
#define PRINTF_PROFILE_CAP_POSITIONAL 1
#undef PRINTF_PROFILE_CAP_C99_FORMATS
#define PRINTF_PROFILE_CAP_C99_FORMATS 1
#undef PRINTF_PROFILE_CAP_BINARY
#define PRINTF_PROFILE_CAP_BINARY 1
#undef PRINTF_PROFILE_CAP_DOUBLE
#define PRINTF_PROFILE_CAP_DOUBLE 1
#undef PRINTF_PROFILE_CAP_LONG_DOUBLE
#define PRINTF_PROFILE_CAP_LONG_DOUBLE 1
#undef PRINTF_PROFILE_CAP_WCHAR
#define PRINTF_PROFILE_CAP_WCHAR 1
#endif

#ifndef PRINTF_CAP_SHRINK
#define PRINTF_CAP_SHRINK PRINTF_PROFILE_CAP_SHRINK
#endif
#ifndef PRINTF_CAP_LONG_LONG
#define PRINTF_CAP_LONG_LONG PRINTF_PROFILE_CAP_LONG_LONG
#endif
#ifndef PRINTF_CAP_POSITIONAL
#define PRINTF_CAP_POSITIONAL PRINTF_PROFILE_CAP_POSITIONAL
#endif
#ifndef PRINTF_CAP_C99_FORMATS
#define PRINTF_CAP_C99_FORMATS PRINTF_PROFILE_CAP_C99_FORMATS
#endif
#ifndef PRINTF_CAP_BINARY
#define PRINTF_CAP_BINARY PRINTF_PROFILE_CAP_BINARY
#endif
#ifndef PRINTF_CAP_FLOAT
#define PRINTF_CAP_FLOAT PRINTF_PROFILE_CAP_FLOAT
#endif
#ifndef PRINTF_CAP_DOUBLE
#define PRINTF_CAP_DOUBLE PRINTF_PROFILE_CAP_DOUBLE
#endif
#ifndef PRINTF_CAP_LONG_DOUBLE
#define PRINTF_CAP_LONG_DOUBLE PRINTF_PROFILE_CAP_LONG_DOUBLE
#endif
#ifndef PRINTF_CAP_WCHAR
#define PRINTF_CAP_WCHAR PRINTF_PROFILE_CAP_WCHAR
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

/* Multi-byte support is derived from wchar capability unless explicitly overridden. */
#ifndef PRINTF_CAP_MBTOWIDE
#if PRINTF_CAP_WCHAR && defined(__MB_CAPABLE) && defined(WIDE_CHARS)
#define PRINTF_CAP_MBTOWIDE 1
#else
#define PRINTF_CAP_MBTOWIDE 0
#endif
#endif

#ifndef PRINTF_CAP_WIDETOMB
#if PRINTF_CAP_WCHAR && defined(__MB_CAPABLE) && !defined(WIDE_CHARS)
#define PRINTF_CAP_WIDETOMB 1
#else
#define PRINTF_CAP_WIDETOMB 0
#endif
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

#if PRINTF_CAP_FLOAT
#define FLOAT_UINT uint32_t
#else
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
#else
typedef unsigned long ultoa_unsigned_t;
typedef long          ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG__
#endif

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

#include "printf_core_private.h"
#include "../../../libm/common/math_config.h"

#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
#include "dtoa.h"
#endif

static inline void
__printf_skip_float_arg(uint16_t flags, va_list ap)
{
#if PRINTF_CAP_LONG_DOUBLE && PRINTF_FLOAT_CAP_LARGE
    if ((flags & (PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE))
        == (PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE)) {
        (void) va_arg(ap, long double);
        return;
    }
    (void) va_arg(ap, double);
#elif PRINTF_CAP_FLOAT
    (void) flags;
    (void) va_arg(ap, uint32_t);
#else
    (void) flags;
    (void) va_arg(ap, double);
#endif
}

static inline ultoa_unsigned_t
__printf_read_unsigned_arg(va_list ap, uint16_t flags)
{
    ultoa_unsigned_t value;

    if (flags & PRINTF_FLAG_LONG) {
        if (flags & PRINTF_FLAG_REPEAT_TYPE)
#if PRINTF_CAP_LONG_LONG
            value = va_arg(ap, unsigned long long);
#else
            value = (unsigned long) va_arg(ap, unsigned long long);
#endif
        else
            value = va_arg(ap, unsigned long);
        return value;
    }

    value = va_arg(ap, unsigned int);
    if (flags & PRINTF_FLAG_SHORT) {
        if (flags & PRINTF_FLAG_REPEAT_TYPE)
            value = (unsigned char) value;
        else
            value = (unsigned short) value;
    }
    return value;
}

static inline ultoa_signed_t
__printf_read_signed_arg(va_list ap, uint16_t flags)
{
    ultoa_signed_t value;

    if (flags & PRINTF_FLAG_LONG) {
        if (flags & PRINTF_FLAG_REPEAT_TYPE)
#if PRINTF_CAP_LONG_LONG
            value = va_arg(ap, signed long long);
#else
            value = (signed long) va_arg(ap, signed long long);
#endif
        else
            value = va_arg(ap, signed long);
        return value;
    }

    value = va_arg(ap, signed int);
    if (flags & PRINTF_FLAG_SHORT) {
        if (flags & PRINTF_FLAG_REPEAT_TYPE)
            value = (signed char) value;
        else
            value = (signed short) value;
    }
    return value;
}

#endif
