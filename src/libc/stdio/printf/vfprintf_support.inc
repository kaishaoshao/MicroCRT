/*
 * Compatibility wrapper.
 *
 * The real shared support layer now lives under core/.
 * Keep this top-level include name stable while shells and impl glue
 * are migrated incrementally.
 */

#include "core/vfprintf_support.inc"
