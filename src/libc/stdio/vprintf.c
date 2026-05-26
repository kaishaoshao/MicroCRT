/*
 * Default vprintf-family wrapper.
 *
 * This file is the real home of the public vprintf() wrapper.
 */

#include <stdio.h>

int
vprintf(const char *fmt, va_list ap)
{
    return vfprintf(stdout, fmt, ap);
}
