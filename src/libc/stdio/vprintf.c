/*
 * Default vprintf-family wrapper.
 *
 * This file is the real home of the public vprintf() wrapper.
 */

#include "printf/__printf_stream_api.h"
#include <stdio.h>

int
vprintf(const char *fmt, va_list ap)
{
    return __printf_vformat_stream(stdout, fmt, ap);
}
