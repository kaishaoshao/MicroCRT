/*
 * Integer-family printf wrappers.
 *
 * This file is the real home of the iprintf-family public wrappers.
 */

#include "printf/__printf_stream_api.h"
#include "printf/__printf_string_api.h"

int
iprintf(const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_stream_int(stdout, fmt, ap);
    va_end(ap);
    return i;
}

int
viprintf(const char *fmt, va_list ap)
{
    return __printf_vformat_stream_int(stdout, fmt, ap);
}

int
fiprintf(FILE *stream, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_stream_int(stream, fmt, ap);
    va_end(ap);
    return i;
}

int
vfiprintf(FILE *stream, const char *fmt, va_list ap)
{
    return __printf_vformat_stream_int(stream, fmt, ap);
}

int
siprintf(char *s, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_cstr(s, fmt, ap, __printf_core_integer);
    va_end(ap);
    return i;
}

int
vsiprintf(char *s, const char *fmt, va_list ap)
{
    return __printf_vformat_cstr(s, fmt, ap, __printf_core_integer);
}
