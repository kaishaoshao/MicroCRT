#include "stdio_private.h"
#include <stdio.h>

int
printf(const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = vfprintf(stdout, fmt, ap);
    va_end(ap);

    return i;
}
