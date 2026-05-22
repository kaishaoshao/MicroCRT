/*
 * String-backed printf-family wrapper.
 *
 * This file is the real home of sprintf()/vsprintf().
 */

#include "printf/__printf_stream_api.h"
#include "printf/__printf_string_api.h"

int
sprintf(char *s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int i = __printf_vformat_cstr(s, fmt, ap, __printf_core_default);
    va_end(ap);
    return i;
}

int
vsprintf(char *s, const char *fmt, va_list ap)
{
    return __printf_vformat_cstr(s, fmt, ap, __printf_core_default);
}
