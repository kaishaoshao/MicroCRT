/*
 * Conversion: %s / %ls
 *
 * Responsibilities:
 * - extract string pointer
 * - handle null-string policy
 * - compute visible length under precision
 * - perform left/right padding and wide/narrow emission
 */

{
#ifdef _NEED_IO_WCHAR
    if (flags & FL_LONG) {
        wstr = va_arg(ap, wchar_t *);
        if (!wstr)
            goto str_null;
    wstr_lpad:
        size = (flags & FL_PREC) ? (size_t)prec : SIZE_MAX;
#ifdef _NEED_IO_WIDETOMB
        size = _mbslen(wstr, size);
        if (size == (size_t)-1)
            goto ret;
#else
        size = wcsnlen(wstr, size);
#endif
        goto str_lpad;
    } else
#endif
        pnt = va_arg(ap, char *);
    if (!pnt) {
#ifdef _NEED_IO_WCHAR
    str_null:
#endif
#ifdef VFPRINTF_S
        msg = "arg corresponding to '%s' is null";
        goto handle_error;
#endif
        pnt = "(null)";
    }
#ifdef _NEED_IO_SHRINK
    char c;
    while ((c = *pnt++))
        my_putc(c, stream);
#else
    size = (flags & FL_PREC) ? (size_t)prec : SIZE_MAX;
#ifdef _NEED_IO_MBTOWIDE
    size = _wcslen(pnt, size);
#else
    size = strnlen(pnt, size);
#endif
str_lpad:
    if (!(flags & FL_LPAD)) {
        while ((size_t)width > size) {
            my_putc(' ', stream);
            width--;
        }
    }
    width -= size;
#ifdef _NEED_IO_WCHAR
    if (wstr) {
#ifdef _NEED_IO_WIDETOMB
        mbstate_t ps = { 0 };
        while (size) {
            wchar_t c = *wstr++;
            char   *m = u.mb;
            int     mb_len = __WCTOMB(m, c, &ps);
            while (size && mb_len) {
                my_putc(*m++, stream);
                size--;
                mb_len--;
            }
        }
#else
        while (size--)
            my_putc(*wstr++, stream);
#endif
    } else
#endif
    {
#ifdef _NEED_IO_MBTOWIDE
        mbstate_t ps = { 0 };
        while (size--) {
            wchar_t c;
            size_t  mb_len = mbrtowc(&c, pnt, MB_LEN_MAX, &ps);
            my_putc(c, stream);
            pnt += mb_len;
        }
#else
        while (size--)
            my_putc(*pnt++, stream);
#endif
    }
#endif
}
