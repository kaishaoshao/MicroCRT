/*
 * Default vfprintf shell for the MicroCRT printf framework.
 *
 * The shared formatter logic lives in ../vfprintf_impl.inc. This file only
 * selects the default variant symbol and includes the shared implementation
 * template so that future integer/full/wide shells can reuse the same body.
 */

#ifndef PRINTF_VARIANT
#define PRINTF_VARIANT __IO_VARIANT_DOUBLE
#endif

#ifndef PRINTF_NAME
#define PRINTF_NAME __d_vfprintf
#endif

#include "../vfprintf_impl.inc"
