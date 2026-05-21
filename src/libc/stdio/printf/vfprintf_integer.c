/*
 * Compatibility wrapper.
 *
 * The real integer vfprintf shell now lives under shells/.
 * Keep this top-level filename stable while build files and includes are
 * migrated incrementally.
 */

#include "shells/vfprintf_integer.c"
