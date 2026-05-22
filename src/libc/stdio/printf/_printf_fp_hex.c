/*
 * Shared helper for %a hexadecimal float formatting.
 */

static int
__printf_format_fp_hex(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                       int *width, unsigned char case_convert, va_list ap, struct dtoa *dtoa)
{
#define TOCASE_LOCAL(ch) ((ch) - case_convert)
    uint16_t local_flags = *flags | FL_FLTEXP | FL_FLTHEX;
    int local_prec = *prec;
    int local_width = *width;
    uint8_t sign;
    uint8_t ndigs;
    int exp;
    int n;

#ifdef _NEED_IO_LONG_DOUBLE
    if ((local_flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE)) {
        PRINTF_LONG_DOUBLE_TYPE fval;

        fval = PRINTF_LONG_DOUBLE_ARG(ap);
        if (!(local_flags & FL_PREC))
            local_prec = -1;
        local_prec = __lfloat_x_engine(fval, dtoa, local_prec, case_convert);
        ndigs = local_prec + 1;
        exp = dtoa->exp;
    } else
#endif
    {
        FLOAT_UINT fval;

        fval = PRINTF_FLOAT_ARG(ap);
        if (!(local_flags & FL_PREC))
            local_prec = -1;

        ndigs = 1 + __float_x_engine(fval, dtoa, local_prec, case_convert);
        if (local_prec <= ndigs)
            local_prec = ndigs - 1;
        exp = dtoa->exp;
    }

    sign = 0;
    if (dtoa->flags & DTOA_MINUS)
        sign = '-';
    else if (local_flags & FL_PLUS)
        sign = '+';
    else if (local_flags & FL_SPACE)
        sign = ' ';

    if (dtoa->flags & (DTOA_NAN | DTOA_INF)) {
        const char *pnt;

        ndigs = sign ? 4 : 3;
        if (local_width > ndigs) {
            local_width -= ndigs;
            if (!(local_flags & FL_LPAD)) {
                do {
                    if (__printf_emit(out, stream_len, ' ') < 0)
                        return -1;
                } while (--local_width);
            }
        } else {
            local_width = 0;
        }
        if (sign && __printf_emit(out, stream_len, sign) < 0)
            return -1;
        pnt = (dtoa->flags & DTOA_NAN) ? "nan" : "inf";
        while (*pnt) {
            if (__printf_emit(out, stream_len, (unsigned char) TOCASE_LOCAL(*pnt)) < 0)
                return -1;
            pnt++;
        }
    } else {
        n = 3 + 2;
        if (sign)
            n += 1;
        if (local_prec)
            n += local_prec + 1;
        else if (local_flags & FL_ALT)
            n += 1;

        local_width = local_width > n ? local_width - n : 0;

        if (!(local_flags & (FL_LPAD | FL_ZFILL))) {
            while (local_width) {
                if (__printf_emit(out, stream_len, ' ') < 0)
                    return -1;
                local_width--;
            }
        }
        if (sign && __printf_emit(out, stream_len, sign) < 0)
            return -1;

        if (__printf_emit(out, stream_len, '0') < 0)
            return -1;
        if (__printf_emit(out, stream_len, (unsigned char) TOCASE_LOCAL('x')) < 0)
            return -1;

        if (!(local_flags & FL_LPAD)) {
            while (local_width) {
                if (__printf_emit(out, stream_len, '0') < 0)
                    return -1;
                local_width--;
            }
        }

        if (__printf_emit(out, stream_len, (unsigned char) dtoa->digits[0]) < 0)
            return -1;
        if (local_prec > 0) {
            int pos;

            if (__printf_emit(out, stream_len, '.') < 0)
                return -1;
            for (pos = 1; pos < 1 + local_prec; pos++) {
                if (__printf_emit(out, stream_len,
                                  (unsigned char) (pos < ndigs ? dtoa->digits[pos] : '0'))
                    < 0)
                    return -1;
            }
        } else if (local_flags & FL_ALT) {
            if (__printf_emit(out, stream_len, '.') < 0)
                return -1;
        }

        if (__printf_emit(out, stream_len, (unsigned char) TOCASE_LOCAL('p')) < 0)
            return -1;
        sign = '+';
        if (exp < 0) {
            exp = -exp;
            sign = '-';
        }
        if (__printf_emit(out, stream_len, sign) < 0)
            return -1;
        if (exp / 10 && __printf_emit(out, stream_len, exp / 10 + '0') < 0)
            return -1;
        if (__printf_emit(out, stream_len, '0' + exp % 10) < 0)
            return -1;
    }

    *flags = local_flags;
    *prec = local_prec;
    *width = local_width;
#undef TOCASE_LOCAL
    return 0;
}
