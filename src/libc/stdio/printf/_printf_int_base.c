/*
 * Shared helper for unsigned/octal/hex/base-N formatting.
 */

static int
__printf_format_int_base(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                         int *width, ultoa_unsigned_t x, int base, unsigned char prefix_c, char *buf)
{
    int local_flags = *flags;
    int local_prec = *prec;
    int local_width = *width;
    int buf_len;

    local_flags &= ~(FL_PLUS | FL_SPACE);

    if (x == 0)
        local_flags &= ~FL_ALT;

#ifndef _NEED_IO_SHRINK
    if (x == 0 && (local_flags & FL_PREC) && local_prec == 0)
        buf_len = 0;
    else
#endif
        buf_len = __ultoa_invert(x, buf, base) - buf;

#ifndef _NEED_IO_SHRINK
    {
        int len = buf_len;

        if (local_flags & FL_PREC) {
            local_flags &= ~FL_ZFILL;

            if (len < local_prec) {
                len = local_prec;

                if (prefix_c == '\0')
                    local_flags &= ~FL_ALT;
            }
        }

        if (local_flags & FL_ALT) {
            len += 1;
            if (prefix_c != '\0')
                len += 1;
        }

        if (!(local_flags & FL_LPAD)) {
            if (local_flags & FL_ZFILL) {
                local_prec = buf_len;
                if (len < local_width) {
                    local_prec += local_width - len;
                    len = local_width;
                }
            }
            while (len < local_width) {
                if (__printf_emit(out, stream_len, ' ') < 0)
                    return -1;
                len++;
            }
        }

        local_width -= len;

        if (local_flags & FL_ALT) {
            if (__printf_emit(out, stream_len, '0') < 0)
                return -1;
            if (prefix_c != '\0' && __printf_emit(out, stream_len, prefix_c) < 0)
                return -1;
        }

        while (local_prec > buf_len) {
            if (__printf_emit(out, stream_len, '0') < 0)
                return -1;
            local_prec--;
        }
    }
#else
    if (local_flags & FL_ALT) {
        if (__printf_emit(out, stream_len, '0') < 0)
            return -1;
        if (prefix_c != '\0' && __printf_emit(out, stream_len, prefix_c) < 0)
            return -1;
    }
#endif

    while (buf_len) {
        if (__printf_emit(out, stream_len, (unsigned char) buf[--buf_len]) < 0)
            return -1;
    }

    *flags = (uint16_t) local_flags;
    *prec = local_prec;
    *width = local_width;
    return 0;
}
