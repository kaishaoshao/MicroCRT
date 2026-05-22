#ifndef _MICROCRT_PRINTF_STREAM_API_H_
#define _MICROCRT_PRINTF_STREAM_API_H_

#include "printf_out.h"

int __printf_core_default(struct __printf_out *out, const char *fmt, va_list ap);
int __printf_core_integer(struct __printf_out *out, const char *fmt, va_list ap);

static inline int
__printf_vformat_stream(FILE *stream, const char *fmt, va_list ap)
{
    struct __printf_out out;

    __printf_out_init_file(&out, stream);
    return __printf_core_default(&out, fmt, ap);
}

static inline int
__printf_vformat_stream_int(FILE *stream, const char *fmt, va_list ap)
{
    struct __printf_out out;

    __printf_out_init_file(&out, stream);
    return __printf_core_integer(&out, fmt, ap);
}

#endif
