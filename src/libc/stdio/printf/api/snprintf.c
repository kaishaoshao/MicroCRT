/*
 * Bounded string-backed printf-family wrapper.
 *
 * This file is the real home of snprintf()/vsnprintf().
 */

#include "../printf_bridge.h"

int
snprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list ap;
    int i;
    struct __file_str f = FDEV_SETUP_STRING_WRITE(s, FDEV_STRING_WRITE_END(s, n));

    va_start(ap, fmt);
    i = vfprintf(&f.file, fmt, ap);
    va_end(ap);

    if (n)
        *f.pos = '\0';

    return i;
}

int
vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
    int i;
    struct __file_str f = FDEV_SETUP_STRING_WRITE(s, FDEV_STRING_WRITE_END(s, n));

    i = vfprintf(&f.file, fmt, ap);

    if (n)
        *f.pos = '\0';

    return i;
}
