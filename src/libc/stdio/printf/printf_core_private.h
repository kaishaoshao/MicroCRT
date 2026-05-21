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

#define IO_VARIANT_IS_FLOAT(v) ((v) == __IO_VARIANT_FLOAT || (v) == __IO_VARIANT_DOUBLE)

#define TOLOWER(c) ((c) | ('a' - 'A'))

#endif
