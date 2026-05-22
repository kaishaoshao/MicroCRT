/*
 * Dispatch branch: %n
 */

#if defined(__IO_PERCENT_N) || defined(VFPRINTF_S)
            else if (c == 'n') {
#ifdef VFPRINTF_S
                msg = "format string contains percent-n";
                goto handle_error;
#else
                if (__printf_entry_n(flags, ap, stream_len) < 0)
                    goto fail;
#endif
            }
#endif
