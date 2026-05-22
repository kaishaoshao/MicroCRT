/*
 * FILE-backed vfprintf adapter.
 *
 * Public stdio entrypoints convert FILE into the printf output abstraction
 * and then call the selected printf_core variant.
 */

#include "printf/__printf_stream_api.h"

int
vfprintf(FILE *stream, const char *fmt, va_list ap)
{
    return __printf_vformat_stream(stream, fmt, ap);
}
