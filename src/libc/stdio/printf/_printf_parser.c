/*
 * Shared helpers for parser state and literal scanning.
 */

static inline int
__printf_scan_literal(struct __printf_out *out, int *stream_len, const CHAR **fmt, unsigned *c)
{
    for (;;) {
        *c = *(*fmt)++;
        if (!*c)
            return 0;
        if (*c == '%') {
            *c = *(*fmt)++;
            if (*c != '%')
                return 1;
        }
        if (__printf_emit(out, stream_len, *c) < 0)
            return -1;
    }
}

static inline void
__printf_spec_normalize(struct __printf_spec *spec)
{
#ifndef _NEED_IO_SHRINK
    if (spec->prec < 0) {
        spec->prec = 0;
        spec->flags &= ~FL_PREC;
    }

    if (spec->width < 0) {
        spec->width = -spec->width;
        spec->flags |= FL_LPAD;
    }
#else
    (void) spec;
#endif
}
