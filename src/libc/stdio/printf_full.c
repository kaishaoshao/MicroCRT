/*
 * Full-profile printf wrappers.
 *
 * These public entrypoints intentionally bind to the dedicated full core
 * symbol so the default family can diverge later without changing callers.
 */

#include "printf/__printf_stream_api.h"
#include "printf/__printf_string_api.h"

int
printf_full(const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_stream_full(stdout, fmt, ap);
    va_end(ap);
    return i;
}

int
vprintf_full(const char *fmt, va_list ap)
{
    return __printf_vformat_stream_full(stdout, fmt, ap);
}

int
fprintf_full(FILE *stream, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_stream_full(stream, fmt, ap);
    va_end(ap);
    return i;
}

int
vfprintf_full(FILE *stream, const char *fmt, va_list ap)
{
    return __printf_vformat_stream_full(stream, fmt, ap);
}

int
sprintf_full(char *s, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_cstr(s, fmt, ap, __printf_core_full);
    va_end(ap);
    return i;
}

int
vsprintf_full(char *s, const char *fmt, va_list ap)
{
    return __printf_vformat_cstr(s, fmt, ap, __printf_core_full);
}

int
snprintf_full(char *s, size_t n, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_cstrn(s, n, fmt, ap, __printf_core_full);
    va_end(ap);
    return i;
}

int
vsnprintf_full(char *s, size_t n, const char *fmt, va_list ap)
{
    return __printf_vformat_cstrn(s, n, fmt, ap, __printf_core_full);
}
