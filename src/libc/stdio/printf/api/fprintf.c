/*
 * Default fprintf-family wrapper.
 *
 * This file is the real home of the public fprintf() wrapper.
 */

#include "../printf_bridge.h"

int
fprintf(FILE *stream, const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = vfprintf(stream, fmt, ap);
    va_end(ap);

    return i;
}
