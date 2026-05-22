/*
 * Dispatch layer.
 *
 * Entered after printf_core_parser.inc has finished parsing one
 * conversion descriptor. Responsibilities:
 * - classify the final conversion character
 * - route to float / char / string / %n / integer handling
 * - keep width-tail handling centralized after conversion output
 */

        /* Only a format character is valid. */

#define TOCASE(c) ((c) - case_convert)

#include "printf_core_dispatch_float.inc"
        {
#ifdef _NEED_IO_WCHAR
            wchar_t *wstr = NULL;
#endif
#ifdef _NEED_IO_WCHAR
            pnt = NULL;
#endif

 #include "printf_core_dispatch_char.inc"
 #include "printf_core_dispatch_string.inc"
 #include "printf_core_dispatch_percent_n.inc"
 #include "printf_core_dispatch_integer.inc"
        }

#ifndef _NEED_IO_SHRINK
        /* Tail is possible. */
        if (__printf_emit_width_tail(out, &stream_len, &width) < 0)
            goto fail;
#endif
