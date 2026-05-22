/*
 * Bounded string-backed printf-family wrapper.
 *
 * This file is the real home of snprintf()/vsnprintf().
 */

#include "printf/__printf_stream_api.h"
#include "printf/__printf_string_api.h"

int
snprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int i = __printf_vformat_cstrn(s, n, fmt, ap, __printf_core_default);
    va_end(ap);
    return i;
}

int
vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
    return __printf_vformat_cstrn(s, n, fmt, ap, __printf_core_default);
}
