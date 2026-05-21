/*
 * Compatibility wrapper.
 *
 * The real shared core body now lives under core/.
 * Keep this top-level include name stable while vfprintf_impl.inc and
 * related code are migrated incrementally.
 */

#include "core/printf_core_body.inc"
