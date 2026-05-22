/*
 * Shared helper for signed decimal formatting.
 */

static int
__printf_format_int_dec(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                        int *width, ultoa_signed_t x_s, char *buf)
{
    int local_flags = *flags;
    int local_prec = *prec;
    int local_width = *width;
    int buf_len;

    if (x_s < 0) {
        x_s = (ultoa_signed_t) - (ultoa_unsigned_t) x_s;
        local_flags |= FL_NEGATIVE;
    }

    local_flags &= ~FL_ALT;

#ifndef _NEED_IO_SHRINK
    if (x_s == 0 && (local_flags & FL_PREC) && local_prec == 0)
        buf_len = 0;
    else
#endif
        buf_len = __ultoa_invert(x_s, buf, 10) - buf;

#ifndef _NEED_IO_SHRINK
    {
        int len = buf_len;

        if (local_flags & FL_PREC) {
            local_flags &= ~FL_ZFILL;
            if (len < local_prec)
                len = local_prec;
        }

        if (local_flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE))
            len += 1;

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

        if (local_flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) {
            unsigned char z = ' ';
            if (local_flags & FL_PLUS)
                z = '+';
            if (local_flags & FL_NEGATIVE)
                z = '-';
            if (__printf_emit(out, stream_len, z) < 0)
                return -1;
        }

        while (local_prec > buf_len) {
            if (__printf_emit(out, stream_len, '0') < 0)
                return -1;
            local_prec--;
        }
    }
#else
    if (local_flags & FL_NEGATIVE) {
        if (__printf_emit(out, stream_len, '-') < 0)
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
