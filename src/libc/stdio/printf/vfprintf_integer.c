/*
 * Integer-only vfprintf shell.
 *
 * This shell selects the integer formatter variant and reuses the shared
 * formatter implementation from ../vfprintf_impl.inc.
 */

#define PRINTF_VARIANT __IO_VARIANT_INTEGER
#define PRINTF_NAME __printf_core_integer

#include "vfprintf_impl.inc"
