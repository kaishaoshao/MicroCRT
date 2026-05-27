/*
 * Shared text formatting core for %c / %s family output.
 */

#if PRINTF_CAP_WIDETOMB || PRINTF_CAP_MBTOWIDE
#include <bits/types/mbstate_t.h>
#endif

#if PRINTF_CAP_WIDETOMB
static size_t
_mbslen(const wchar_t *s, size_t maxlen)
{
    mbstate_t ps = { 0 };
    wchar_t c;
    char tmp[MB_LEN_MAX];
    size_t len = 0;

    while (len < maxlen && (c = *s++) != L'\0') {
        int clen = __WCTOMB(tmp, c, &ps);

        if (clen == -1)
            return (size_t) clen;
        len += (size_t) clen;
    }

    return len;
}
#endif

#if PRINTF_CAP_MBTOWIDE
static size_t
_wcslen(const char *s, size_t maxlen)
{
    mbstate_t ps = { 0 };
    wchar_t c;
    size_t len = 0;

    while (len < maxlen && *s != '\0') {
        size_t clen = mbrtowc(&c, s, MB_LEN_MAX, &ps);

        if (c == L'\0')
            break;
        if (clen == (size_t)-1)
            return clen;

        s += clen;
        ++len;
    }

    return len;
}
#endif

static inline int
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
    return __printf_out_write(out, stream_len, p, size);
}

#if PRINTF_CAP_WCHAR
static int
__printf_emit_wide_span(struct __printf_out *out, int *stream_len, const wchar_t *wstr, size_t size
#if PRINTF_CAP_WIDETOMB
                        ,
                        char *mb_buf
#endif
)
{
#if PRINTF_CAP_WIDETOMB
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
                            size_t size, const char *pnt, const wchar_t *wstr,
                            struct __printf_text_runtime *text)
{
#if !PRINTF_CAP_SHRINK
    if (!(flags & PRINTF_FLAG_LEFT_ADJ)) {
        if ((size_t) *width > size) {
            if (__printf_emit_repeat(out, stream_len, ' ', (size_t) *width - size) < 0)
                return -1;
        }
    }
    *width -= (int) size;
#endif

    if (wstr)
        return __printf_emit_wide_span(out, stream_len, wstr, size
#if PRINTF_CAP_WIDETOMB
                                       , text->mb_buf
#endif
        );
    return __printf_emit_narrow_span(out, stream_len, pnt, size);
}
