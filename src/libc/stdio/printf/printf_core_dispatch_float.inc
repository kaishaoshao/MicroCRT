/*
 * Dispatch branch: float family.
 */

#ifndef _NEED_IO_SHRINK
        if ((TOLOWER(c) >= 'e' && TOLOWER(c) <= 'g')
#ifdef _NEED_IO_C99_FORMATS
            || TOLOWER(c) == 'a'
#endif
        ) {
#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
#include "_printf_float.c"
#else
            SKIP_FLOAT_ARG(flags, ap);
            if (__printf_emit_string_common(out, &stream_len, flags, &width, sizeof("*float*") - 1,
                                            "*float*"
#ifdef _NEED_IO_WCHAR
                                            ,
                                            NULL, u.mb
#endif
                    )
                < 0)
                goto fail;
#endif
        } else
#endif
