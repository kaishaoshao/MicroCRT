/*
 * Entry: %s / %ls
 */

static int
__printf_dispatch_string(struct __printf_out *out, int *stream_len, uint16_t flags, int *prec,
                         int *width, va_list ap, const char **msg_out,
                         struct __printf_text_runtime *text)
{
    if (__printf_use_wchar(flags)) {
        const wchar_t *wstr = va_arg(ap, wchar_t *);

        if (!wstr) {
#if PRINTF_CAP_SECURE
            *msg_out = "arg corresponding to '%s' is null";
            return 1;
#else
            static const wchar_t null_wstr[] = L"(null)";
            wstr = null_wstr;
#endif
        }
        return __printf_format_string(out, stream_len, flags, prec, width, NULL, wstr, text);
    }
    {
        const char *pnt = va_arg(ap, char *);

        if (!pnt) {
#if PRINTF_CAP_SECURE
            *msg_out = "arg corresponding to '%s' is null";
            return 1;
#else
            pnt = "(null)";
#endif
        }
        return __printf_format_string(out, stream_len, flags, prec, width, pnt, NULL, text);
    }
}
