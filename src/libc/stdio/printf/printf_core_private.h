#ifndef _MICROCRT_PRINTF_CORE_PRIVATE_H_
#define _MICROCRT_PRINTF_CORE_PRIVATE_H_

/*
 * Transitional core-private header for the MicroCRT printf framework.
 *
 * This header intentionally keeps the existing migrated private
 * definitions together while wrapper/bridge users are peeled away to
 * printf_bridge.h. The next steps can shrink this file further without
 * disturbing call sites.
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "printf_bridge.h"
#include "printf_out.h"

#ifndef PRINTF_CAP_SHRINK
#define PRINTF_CAP_SHRINK 0
#endif

#ifndef PRINTF_CAP_SECURE
#define PRINTF_CAP_SECURE 0
#endif

#ifndef IO_VARIANT_IS_FLOAT
#define IO_VARIANT_IS_FLOAT(v) ((v) == __IO_VARIANT_FLOAT || (v) == __IO_VARIANT_DOUBLE)
#endif

#define TOLOWER(c) ((c) | ('a' - 'A'))

struct __printf_spec {
    uint16_t flags;
#if !PRINTF_CAP_SHRINK
    int width;
    int prec;
#endif
    int argno;
};

static inline void
__printf_spec_reset(struct __printf_spec *spec)
{
    spec->flags = 0;
#if !PRINTF_CAP_SHRINK
    spec->width = 0;
    spec->prec = 0;
#endif
    spec->argno = 0;
}

static inline int
__printf_emit(struct __printf_out *out, int *stream_len, unsigned ch)
{
    (*stream_len)++;
    return out->put((int) ch, out->cookie);
}

struct __printf_text_runtime {
    char *buf;
    wchar_t *wbuf;
#if PRINTF_CAP_WIDETOMB
    char *mb_buf;
#endif
};

static inline bool
__printf_cap_wchar_enabled(void)
{
    return PRINTF_CAP_WCHAR != 0;
}

static inline bool
__printf_cap_shrink_enabled(void)
{
    return PRINTF_CAP_SHRINK != 0;
}

static inline bool
__printf_cap_positional_enabled(void)
{
    return PRINTF_CAP_POSITIONAL != 0;
}

static inline bool
__printf_cap_c99_formats_enabled(void)
{
    return PRINTF_CAP_C99_FORMATS != 0;
}

static inline bool
__printf_cap_binary_enabled(void)
{
    return PRINTF_CAP_BINARY != 0;
}

static inline bool
__printf_cap_secure_enabled(void)
{
    return PRINTF_CAP_SECURE != 0;
}

static inline bool
__printf_cap_percent_n_enabled(void)
{
    return PRINTF_CAP_PERCENT_N != 0;
}

static inline bool
__printf_core_has_float(void)
{
    return IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE);
}

static inline bool
__printf_use_wchar(uint16_t flags)
{
    return __printf_cap_wchar_enabled() && ((flags & PRINTF_FLAG_LONG) != 0);
}

typedef struct {
    va_list ap;
} my_va_list;

static inline void
__printf_spec_reset_fields(struct __printf_spec *spec)
{
#if !PRINTF_CAP_SHRINK
    spec->width = 0;
    spec->prec = 0;
#else
    (void) spec;
#endif
}

static inline void
__printf_spec_reset_for_argno(struct __printf_spec *spec)
{
    spec->flags = 0;
    __printf_spec_reset_fields(spec);
}

static inline void
__printf_spec_accumulate_digit(struct __printf_spec *spec, unsigned c)
{
#if !PRINTF_CAP_SHRINK
    c -= '0';
    if (spec->flags & PRINTF_FLAG_PRECISION)
        spec->prec = 10 * spec->prec + (int) c;
    else {
        spec->width = 10 * spec->width + (int) c;
        spec->flags |= PRINTF_FLAG_WIDTH;
    }
#else
    (void) spec;
    (void) c;
#endif
}

static inline void
__printf_spec_read_dynamic_field(struct __printf_spec *spec, va_list ap)
{
#if PRINTF_CAP_SHRINK
    (void) spec;
    (void) va_arg(ap, int);
#else
    if (spec->flags & PRINTF_FLAG_PRECISION)
        spec->prec = va_arg(ap, int);
    else
        spec->width = va_arg(ap, int);
#endif
}

static inline void
__printf_spec_normalize_fields(struct __printf_spec *spec)
{
#if !PRINTF_CAP_SHRINK
    if (spec->prec < 0) {
        spec->prec = 0;
        spec->flags &= ~PRINTF_FLAG_PRECISION;
    }

    if (spec->width < 0) {
        spec->width = -spec->width;
        spec->flags |= PRINTF_FLAG_LEFT_ADJ;
    }
#else
    (void) spec;
#endif
}

#ifdef PRINTF_FLAG_LONG
static inline uint16_t
__printf_apply_size_modifier(uint16_t flags, unsigned c)
{
    if (c == 'l') {
        if (flags & PRINTF_FLAG_LONG)
            flags |= PRINTF_FLAG_REPEAT_TYPE;
        return flags | PRINTF_FLAG_LONG;
    }

    if (c == 'h') {
        if (flags & PRINTF_FLAG_SHORT)
            flags |= PRINTF_FLAG_REPEAT_TYPE;
        return flags | PRINTF_FLAG_SHORT;
    }

    if (c == 'L')
        return flags | PRINTF_FLAG_REPEAT_TYPE | PRINTF_FLAG_LONG;

#if PRINTF_CAP_C99_FORMATS
    if (c == 'j') {
        if (sizeof(intmax_t) == sizeof(long))
            return flags | PRINTF_FLAG_LONG;
        if (sizeof(intmax_t) == sizeof(long long))
            return flags | PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE;
        if (sizeof(intmax_t) == sizeof(short))
            return flags | PRINTF_FLAG_SHORT;
        return flags;
    }

    if (c == 'z') {
        if (sizeof(size_t) == sizeof(long))
            return flags | PRINTF_FLAG_LONG;
        if (sizeof(size_t) == sizeof(long long))
            return flags | PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE;
        if (sizeof(size_t) == sizeof(short))
            return flags | PRINTF_FLAG_SHORT;
        return flags;
    }

    if (c == 't') {
        if (sizeof(ptrdiff_t) == sizeof(long))
            return flags | PRINTF_FLAG_LONG;
        if (sizeof(ptrdiff_t) == sizeof(long long))
            return flags | PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE;
        if (sizeof(ptrdiff_t) == sizeof(short))
            return flags | PRINTF_FLAG_SHORT;
        return flags;
    }
#endif

    return flags;
}
#endif

#if PRINTF_CAP_SECURE
static inline const char *
__printf_validate_core_inputs(struct __printf_out *out, const char *fmt)
{
    if (out == NULL)
        return "output sink is null";
    if (fmt == NULL)
        return "null format string";
    return NULL;
}

static inline int
__printf_handle_core_error(struct __printf_out *out, const char *msg)
{
    if (__cur_handler != NULL)
        __cur_handler(msg, NULL, -1);
    if (out != NULL)
        __printf_out_fail(out);
    return -1;
}
#endif

#endif
