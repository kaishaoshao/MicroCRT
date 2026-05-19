#include "stdio_private.h"

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
