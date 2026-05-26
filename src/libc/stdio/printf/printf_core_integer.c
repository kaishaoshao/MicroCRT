/*
 * Integer-only printf core shell.
 *
 * This shell selects the integer formatter variant and reuses the shared
 * formatter implementation from ../printf_core_impl.inc.
 */

#define PRINTF_CORE_PROFILE __IO_VARIANT_INTEGER
#define PRINTF_CORE_SYMBOL __printf_core_integer

#include "printf_core_impl.inc"
