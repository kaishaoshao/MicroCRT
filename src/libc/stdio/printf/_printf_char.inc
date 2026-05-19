{
#ifdef _NEED_IO_SHRINK
    my_putc(va_arg(ap, int), stream);
#else
#ifdef _NEED_IO_WCHAR
    if (flags & FL_LONG) {
        u.wbuf[0] = va_arg(ap, wint_t);
        u.wbuf[1] = L'\0';
        wstr = u.wbuf;
        goto wstr_lpad;
    }
#endif
    u.buf[0] = va_arg(ap, int);
    pnt = u.buf;
    size = 1;
    goto str_lpad;
#endif
}
