/*
 * Default printf-family wrapper.
 *
 * This file is the real home of the public printf() wrapper.
 */

#include "printf/__printf_stream_api.h"
#include <stdio.h>

int
printf(const char *fmt, ...)
{
    va_list ap;
    int i;

    va_start(ap, fmt);
    i = __printf_vformat_stream(stdout, fmt, ap);
    va_end(ap);

    return i;
}
