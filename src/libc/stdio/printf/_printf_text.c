/*
 * Shared helper for %c / %s text emission.
 */

static int
__printf_emit_repeat(struct __printf_out *out, int *stream_len, unsigned ch, size_t count)
{
    while (count--) {
        if (__printf_emit(out, stream_len, ch) < 0)
            return -1;
    }
    return 0;
}

static int
__printf_emit_width_tail(struct __printf_out *out, int *stream_len, int *width)
{
    while ((*width)-- > 0) {
        if (__printf_emit(out, stream_len, ' ') < 0)
            return -1;
    }
    return 0;
}

static int
__printf_emit_narrow_span(struct __printf_out *out, int *stream_len, const char *p, size_t size)
{
    while (size--) {
        if (__printf_emit(out, stream_len, (unsigned char) *p++) < 0)
            return -1;
    }
    return 0;
}

#ifdef _NEED_IO_WCHAR
static int
__printf_emit_wide_span(struct __printf_out *out, int *stream_len, const wchar_t *wstr, size_t size,
                        char *mb_buf)
{
#ifdef _NEED_IO_WIDETOMB
    mbstate_t ps = { 0 };

    while (size) {
        wchar_t c = *wstr++;
        char *m = mb_buf;
        int mb_len = __WCTOMB(m, c, &ps);
        while (size && mb_len) {
            if (__printf_emit(out, stream_len, (unsigned char) *m++) < 0)
                return -1;
            size--;
            mb_len--;
        }
    }
#else
    while (size--) {
        if (__printf_emit(out, stream_len, (unsigned) *wstr++) < 0)
            return -1;
    }
#endif
    return 0;
}
#endif

static int
__printf_emit_string_common(struct __printf_out *out, int *stream_len, uint16_t flags, int *width,
                            size_t size, const char *pnt
#ifdef _NEED_IO_WCHAR
                            ,
                            const wchar_t *wstr, char *mb_buf
#endif
)
{
#ifndef _NEED_IO_SHRINK
    if (!(flags & FL_LPAD)) {
        if ((size_t) *width > size) {
            if (__printf_emit_repeat(out, stream_len, ' ', (size_t) *width - size) < 0)
                return -1;
        }
    }
    *width -= (int) size;
#endif

#ifdef _NEED_IO_WCHAR
    if (wstr)
        return __printf_emit_wide_span(out, stream_len, wstr, size, mb_buf);
#endif
    return __printf_emit_narrow_span(out, stream_len, pnt, size);
}
