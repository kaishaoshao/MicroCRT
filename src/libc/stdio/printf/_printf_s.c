/*
 * Entry: %s / %ls
 */

static int
__printf_entry_s(struct __printf_out *out, int *stream_len, uint16_t flags, int *prec, int *width,
                 const char *pnt
#ifdef _NEED_IO_WCHAR
                 ,
                 const wchar_t *wstr, char *mb_buf
#endif
)
{
    size_t size;

#ifdef _NEED_IO_WCHAR
    if (wstr) {
        size = (flags & FL_PREC) ? (size_t) *prec : SIZE_MAX;
#ifdef _NEED_IO_WIDETOMB
        size = _mbslen(wstr, size);
        if (size == (size_t) -1)
            return -1;
#else
        size = wcsnlen(wstr, size);
#endif
        return __printf_emit_string_common(out, stream_len, flags, width, size, NULL, wstr, mb_buf);
    }
#endif

#ifdef _NEED_IO_SHRINK
    while (*pnt) {
        if (__printf_emit(out, stream_len, (unsigned char) *pnt++) < 0)
            return -1;
    }
    return 0;
#else
    size = (flags & FL_PREC) ? (size_t) *prec : SIZE_MAX;
#ifdef _NEED_IO_MBTOWIDE
    size = _wcslen(pnt, size);
#else
    size = strnlen(pnt, size);
#endif
    return __printf_emit_string_common(out, stream_len, flags, width, size, pnt
#ifdef _NEED_IO_WCHAR
                                       ,
                                       NULL, mb_buf
#endif
    );
#endif
}
