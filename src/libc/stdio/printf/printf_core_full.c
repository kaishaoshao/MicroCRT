/*
 * Full-feature printf core shell.
 *
 * This shell preserves the current "all supported features enabled" variant
 * behind a dedicated symbol for the public printf_full-family wrappers.
 */

#define PRINTF_CORE_PROFILE __IO_VARIANT_DOUBLE
#define PRINTF_CORE_SYMBOL __printf_core_full

#include "printf_core_impl.inc"
