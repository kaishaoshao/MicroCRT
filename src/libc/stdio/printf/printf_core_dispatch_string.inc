/*
 * Dispatch branch: %s
 */

            else if (c == 's') {
#ifdef _NEED_IO_WCHAR
                if (flags & FL_LONG) {
                    const wchar_t *wstr = va_arg(ap, wchar_t *);

                    if (!wstr) {
#ifdef VFPRINTF_S
                        msg = "arg corresponding to '%s' is null";
                        goto handle_error;
#else
                        static const wchar_t null_wstr[] = L"(null)";
                        wstr = null_wstr;
#endif
                    }
                    if (__printf_entry_s(out, &stream_len, flags, &prec, &width, NULL, wstr, u.mb) < 0)
                        goto fail;
                } else
#endif
                {
                    const char *pnt = va_arg(ap, char *);

                    if (!pnt) {
#ifdef VFPRINTF_S
                        msg = "arg corresponding to '%s' is null";
                        goto handle_error;
#else
                        pnt = "(null)";
#endif
                    }
                    if (__printf_entry_s(out, &stream_len, flags, &prec, &width, pnt
#ifdef _NEED_IO_WCHAR
                                         ,
                                         NULL, u.mb
#endif
                            )
                        < 0)
                        goto fail;
                }
            }
