/*
 * Integer-family printf wrappers.
 *
 * This file is the real home of the iprintf-family public wrappers.
 */

#include "../printf_bridge.h"

int __i_vfprintf(FILE *stream, const char *fmt, va_list ap);

int
iprintf(const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __i_vfprintf(stdout, fmt, ap);
    va_end(ap);
    return i;
}

int
viprintf(const char *fmt, va_list ap)
{
    return __i_vfprintf(stdout, fmt, ap);
}

int
fiprintf(FILE *stream, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __i_vfprintf(stream, fmt, ap);
    va_end(ap);
    return i;
}

int
vfiprintf(FILE *stream, const char *fmt, va_list ap)
{
    return __i_vfprintf(stream, fmt, ap);
}

int
siprintf(char *s, const char *fmt, ...)
{
    va_list ap;
    int i;
    struct __file_str f = FDEV_SETUP_STRING_WRITE(s, NULL);

    va_start(ap, fmt);
    i = __i_vfprintf(&f.file, fmt, ap);
    va_end(ap);

    *f.pos = '\0';
    return i;
}

int
vsiprintf(char *s, const char *fmt, va_list ap)
{
    int i;
    struct __file_str f = FDEV_SETUP_STRING_WRITE(s, NULL);

    i = __i_vfprintf(&f.file, fmt, ap);
    *f.pos = '\0';
    return i;
}
