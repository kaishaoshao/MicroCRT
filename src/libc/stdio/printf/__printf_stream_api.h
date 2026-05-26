#ifndef _MICROCRT_PRINTF_STREAM_API_H_
#define _MICROCRT_PRINTF_STREAM_API_H_

#include "printf_out.h"

int __printf_core_default(struct __printf_out *out, const char *fmt, va_list ap);
int __printf_core_integer(struct __printf_out *out, const char *fmt, va_list ap);
int __printf_core_full(struct __printf_out *out, const char *fmt, va_list ap);

/* FILE-backed route only: convert FILE into __printf_out, then call core. */
static inline int
__printf_vformat_stream_core(FILE *stream, const char *fmt, va_list ap,
                             int (*core)(struct __printf_out *out, const char *fmt, va_list ap))
{
    struct __printf_out out;

    __printf_out_init_file(&out, stream);
    return core(&out, fmt, ap);
}

static inline int
__printf_vformat_stream(FILE *stream, const char *fmt, va_list ap)
{
    return __printf_vformat_stream_core(stream, fmt, ap, __printf_core_default);
}

static inline int
__printf_vformat_stream_int(FILE *stream, const char *fmt, va_list ap)
{
    return __printf_vformat_stream_core(stream, fmt, ap, __printf_core_integer);
}

static inline int
__printf_vformat_stream_full(FILE *stream, const char *fmt, va_list ap)
{
    return __printf_vformat_stream_core(stream, fmt, ap, __printf_core_full);
}

#endif
