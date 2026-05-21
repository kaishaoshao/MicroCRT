/*
 * String-backed printf-family wrapper.
 *
 * This file is the real home of sprintf()/vsprintf().
 */

#include "../printf_bridge.h"

int
sprintf(char *s, const char *fmt, ...)
{
    va_list ap;
    int i;
    struct __file_str f = FDEV_SETUP_STRING_WRITE(s, NULL);

    va_start(ap, fmt);
    i = vfprintf(&f.file, fmt, ap);
    va_end(ap);

    *f.pos = '\0';
    return i;
}

int
vsprintf(char *s, const char *fmt, va_list ap)
{
    int i;
    struct __file_str f = FDEV_SETUP_STRING_WRITE(s, NULL);

    i = vfprintf(&f.file, fmt, ap);
    *f.pos = '\0';
    return i;
}
