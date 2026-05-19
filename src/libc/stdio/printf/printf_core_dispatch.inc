        /* Only a format character is valid. */

#define TOCASE(c) ((c) - case_convert)

#ifndef _NEED_IO_SHRINK
        if ((TOLOWER(c) >= 'e' && TOLOWER(c) <= 'g')
#ifdef _NEED_IO_C99_FORMATS
            || TOLOWER(c) == 'a'
#endif
        ) {
#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
#include "_printf_float.inc"
#else
            SKIP_FLOAT_ARG(flags, ap);
            pnt = "*float*";
            size = sizeof("*float*") - 1;
            goto str_lpad;
#endif
        } else
#endif /* ifndef _NEED_IO_SHRINK */
        {
            int buf_len;
#ifdef _NEED_IO_WCHAR
            wchar_t *wstr = NULL;
#endif
#ifdef _NEED_IO_WCHAR
            pnt = NULL;
#endif

            if (c == 'c') {
 #include "_printf_char.inc"
            } else if (c == 's') {
 #include "_printf_str.inc"
            }

#if defined(__IO_PERCENT_N) || defined(VFPRINTF_S)
            else if (c == 'n') {
 #include "_printf_n.inc"
            }
#endif

            else {
 #include "_printf_int.inc"
            }
        }

#ifndef _NEED_IO_SHRINK
        /* Tail is possible. */
        while (width-- > 0) {
            my_putc(' ', stream);
        }
#endif
