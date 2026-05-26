/*
 * Shared formatter core for %s / %ls.
 */

static int
__printf_format_string(struct __printf_out *out, int *stream_len, uint16_t flags, int *prec,
                       int *width, const char *pnt, const wchar_t *wstr,
                       struct __printf_text_runtime *text)
{
    size_t size;

    if (wstr != NULL) {
        size = (flags & PRINTF_FLAG_PRECISION) ? (size_t) *prec : SIZE_MAX;
#if PRINTF_CAP_WIDETOMB
        size = _mbslen(wstr, size);
        if (size == (size_t)-1)
            return -1;
#else
        size_t n = 0;

        while (n < size && wstr[n] != L'\0')
            ++n;
        size = n;
#endif
        return __printf_emit_string_common(out, stream_len, flags, width, size, NULL, wstr, text);
    }

#if PRINTF_CAP_SHRINK
    while (*pnt != '\0') {
        if (__printf_emit(out, stream_len, (unsigned char) *pnt++) < 0)
            return -1;
    }
    return 0;
#else
    size = (flags & PRINTF_FLAG_PRECISION) ? (size_t) *prec : SIZE_MAX;
#if PRINTF_CAP_MBTOWIDE
    size = _wcslen(pnt, size);
#else
    size = strnlen(pnt, size);
#endif
    return __printf_emit_string_common(out, stream_len, flags, width, size, pnt, NULL, text);
#endif
}
