/*
 * Dispatch branch: %c
 */

            if (c == 'c') {
                if (__printf_entry_c(out, &stream_len, flags, &width, va_arg(ap, int), u.buf
#ifdef _NEED_IO_WCHAR
                                     ,
                                     u.wbuf, u.mb
#endif
                        )
                    < 0)
                    goto fail;
            }
