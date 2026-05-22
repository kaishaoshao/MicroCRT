/*
 * Entry: %c / %lc
 */

static int
__printf_entry_c(struct __printf_out *out, int *stream_len, uint16_t flags, int *width, int arg,
                 char *buf
#ifdef _NEED_IO_WCHAR
                 ,
                 wchar_t *wbuf, char *mb_buf
#endif
)
{
#ifdef _NEED_IO_SHRINK
    return __printf_emit(out, stream_len, (unsigned char) arg);
#else
#ifdef _NEED_IO_WCHAR
    if (flags & FL_LONG) {
        wbuf[0] = (wchar_t) arg;
        wbuf[1] = L'\0';
        return __printf_emit_string_common(out, stream_len, flags, width, 1, NULL, wbuf, mb_buf);
    }
#endif
    buf[0] = (char) arg;
    return __printf_emit_string_common(out, stream_len, flags, width, 1, buf
#ifdef _NEED_IO_WCHAR
                                       ,
                                       NULL, mb_buf
#endif
    );
#endif
}
