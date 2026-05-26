/*
 * Default printf core shell for the MicroCRT printf framework.
 *
 * The shared formatter logic lives in ../printf_core_impl.inc. This file only
 * selects the default variant symbol and includes the shared implementation
 * template so that future integer/full/wide shells can reuse the same body.
 */

#ifndef PRINTF_CORE_PROFILE
#define PRINTF_CORE_PROFILE __IO_VARIANT_DOUBLE
#endif

#ifndef PRINTF_CORE_SYMBOL
#define PRINTF_CORE_SYMBOL __printf_core_default
#endif

#include "printf_core_impl.inc"
