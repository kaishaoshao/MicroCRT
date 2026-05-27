#ifndef _MICROCRT_PRINTF_STRING_API_H_
#define _MICROCRT_PRINTF_STRING_API_H_

#include "printf_out.h"

/* String-backed route only: convert char buffer into __printf_out, then call core. */
static inline int
__printf_vformat_cstr(char *s, const char *fmt, va_list ap,
                      int (*core)(struct __printf_out *out, const char *fmt, va_list ap))
{
    struct __printf_out out;
    struct __printf_cstr_out buf = { .pos = s, .end = NULL, .cap = (size_t) -1 };

    __printf_out_init_cstr(&out, &buf);
    return core(&out, fmt, ap);
}

static inline int
__printf_vformat_cstrn(char *s, size_t n, const char *fmt, va_list ap,
                       int (*core)(struct __printf_out *out, const char *fmt, va_list ap))
{
    struct __printf_out out;
    struct __printf_cstr_out buf;

    buf.pos = s;
    buf.end = n ? (s + n - 1) : s;
    buf.cap = n;

    __printf_out_init_cstr(&out, &buf);
    return core(&out, fmt, ap);
}

#endif
