/*
 * Integer-only vfprintf shell.
 *
 * This shell selects the integer formatter variant and reuses the shared
 * formatter implementation from ../vfprintf_impl.inc.
 */

#define PRINTF_VARIANT __IO_VARIANT_INTEGER
#define PRINTF_NAME __i_vfprintf

#include "../vfprintf_impl.inc"
