        for (;;) {
            c = *fmt++;
            if (!c)
                goto ret;
            if (c == '%') {
                c = *fmt++;
                if (c != '%')
                    break;
            }
            my_putc(c, stream);
        }

        flags = 0;
#ifndef _NEED_IO_SHRINK
        int width = 0;
        int prec = 0;
#endif
#ifdef _NEED_IO_POS_ARGS
        argno = 0;
#endif

        do {
            if (flags < FL_WIDTH) {
                switch (c) {
                case '0':
                    flags |= FL_ZFILL;
                    continue;
                case '+':
                    flags |= FL_PLUS;
                    __fallthrough;
                case ' ':
                    flags |= FL_SPACE;
                    continue;
                case '-':
                    flags |= FL_LPAD;
                    continue;
                case '#':
                    flags |= FL_ALT;
                    continue;
                case '\'':
                    /*
                     * C/POSIX locale has an empty thousands_sep
                     * value, so we can just ignore this character
                     */
                    continue;
                }
            }

            if (flags < FL_LONG) {
                if (c >= '0' && c <= '9') {
#ifndef _NEED_IO_SHRINK
                    c -= '0';
                    if (flags & FL_PREC) {
                        prec = 10 * prec + c;
                    } else {
                        width = 10 * width + c;
                        flags |= FL_WIDTH;
                    }
#endif
                    continue;
                }
                if (c == '*') {
#ifdef _NEED_IO_POS_ARGS
                    /*
                     * Positional args must be used together, so wait
                     * for the value to appear before dealing with
                     * width and precision fields
                     */
                    if (argno)
                        continue;
#endif
                    if (flags & FL_PREC) {
#ifdef _NEED_IO_SHRINK
                        (void)va_arg(ap, int);
#else
                        prec = va_arg(ap, int);
#endif
                    } else {
#ifdef _NEED_IO_SHRINK
                        (void)va_arg(ap, int);
#else
                        width = va_arg(ap, int);
#endif
                        flags |= FL_WIDTH;
                    }
                    continue;
                }
                if (c == '.') {
                    if (flags & FL_PREC)
                        goto ret;
                    flags |= FL_PREC;
                    continue;
                }
#ifdef _NEED_IO_POS_ARGS
                /* Check for positional args */
                if (c == '$') {
                    /* Check if we've already got the arg position and
                     * are adding width or precision fields
                     */
                    if (argno) {
                        va_end(ap);
                        va_copy(ap, ap_orig);
                        skip_to_arg(fmt_orig, &my_ap, (flags & FL_PREC) ? prec : width);
                        if (flags & FL_PREC)
                            prec = va_arg(ap, int);
                        else
                            width = va_arg(ap, int);
                    } else {
                        argno = width;
                        flags = 0;
                        width = 0;
                        prec = 0;
                    }
                    continue;
                }
#endif
            }

            CHECK_INT_SIZES(c, flags);

            break;
        } while ((c = *fmt++) != 0);

#ifdef _NEED_IO_POS_ARGS
        /* Set arg pointers for positional args */
        if (argno) {
            va_end(ap);
            va_copy(ap, ap_orig);
            skip_to_arg(fmt_orig, &my_ap, argno);
        }
#endif

#ifndef _NEED_IO_SHRINK
        /* This can happen only when prec is set via a '*'
         * specifier, in which case it works as if no precision
         * was specified. Set the precision to zero and clear the
         * flag.
         */
        if (prec < 0) {
            prec = 0;
            flags &= ~FL_PREC;
        }

        /* Handle negative width when provided via a '*' */
        if (width < 0) {
            width = -width;
            flags |= FL_LPAD;
        }
#endif
