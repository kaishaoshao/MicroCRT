/*
 * Shared formatter core for %c / %lc.
 */

static int
__printf_format_char(struct __printf_out *out, int *stream_len, uint16_t flags, int *width, int arg,
                     struct __printf_text_runtime *text)
{
#if PRINTF_CAP_SHRINK
    return __printf_emit(out, stream_len, (unsigned char) arg);
#else
    if (__printf_use_wchar(flags)) {
        text->wbuf[0] = (wchar_t) arg;
        text->wbuf[1] = L'\0';
        return __printf_emit_string_common(out, stream_len, flags, width, 1, NULL, text->wbuf,
                                           text);
    }
    text->buf[0] = (char) arg;
    return __printf_emit_string_common(out, stream_len, flags, width, 1, text->buf, NULL, text);
#endif
}
