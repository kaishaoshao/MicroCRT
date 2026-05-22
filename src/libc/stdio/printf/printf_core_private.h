#ifndef _MICROCRT_PRINTF_CORE_PRIVATE_H_
#define _MICROCRT_PRINTF_CORE_PRIVATE_H_

/*
 * Transitional core-private header for the MicroCRT printf framework.
 *
 * This header intentionally keeps the existing migrated private
 * definitions together while wrapper/bridge users are peeled away to
 * printf_bridge.h. The next steps can shrink this file further without
 * disturbing call sites.
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "printf_bridge.h"
#include "printf_out.h"

#define IO_VARIANT_IS_FLOAT(v) ((v) == __IO_VARIANT_FLOAT || (v) == __IO_VARIANT_DOUBLE)

#define TOLOWER(c) ((c) | ('a' - 'A'))

struct __printf_spec {
    uint16_t flags;
#ifndef _NEED_IO_SHRINK
    int width;
    int prec;
#endif
    int argno;
};

static inline void
__printf_spec_reset(struct __printf_spec *spec)
{
    spec->flags = 0;
#ifndef _NEED_IO_SHRINK
    spec->width = 0;
    spec->prec = 0;
#endif
    spec->argno = 0;
}

static inline int
__printf_emit(struct __printf_out *out, int *stream_len, unsigned ch)
{
    (*stream_len)++;
    return out->put((int) ch, out->cookie);
}

#endif
