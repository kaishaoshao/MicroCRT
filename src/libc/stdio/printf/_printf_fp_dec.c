/*
 * Shared helper for %f / %e / %g decimal float formatting.
 */

static int
__printf_format_fp_dec(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                       int *width, unsigned char conv, unsigned char case_convert, va_list ap,
                       struct dtoa *dtoa)
{
#define TOCASE_LOCAL(ch) ((ch) - case_convert)
    uint16_t local_flags = *flags;
    int local_prec = *prec;
    int local_width = *width;
    uint8_t sign;
    uint8_t ndigs;
    int exp;
    int n;
    uint8_t ndigs_exp;

#if PRINTF_CAP_LONG_DOUBLE
    if ((local_flags & (PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE)) == (PRINTF_FLAG_LONG | PRINTF_FLAG_REPEAT_TYPE)) {
        PRINTF_LONG_DOUBLE_TYPE fval;
        int ndecimal = 0;
        bool fmode = false;

        fval = PRINTF_LONG_DOUBLE_ARG(ap);

        if (!(local_flags & PRINTF_FLAG_PRECISION))
            local_prec = 6;
        if (conv == 'e') {
            ndigs = local_prec + 1;
            local_flags |= PRINTF_FLAG_FLOAT_EXP;
        } else if (conv == 'f') {
            ndigs = LONG_FLOAT_MAX_DIG;
            ndecimal = local_prec;
            local_flags |= PRINTF_FLAG_FLOAT_FIX;
            fmode = true;
        } else {
            conv += 'e' - 'g';
            ndigs = local_prec;
            if (ndigs < 1)
                ndigs = 1;
        }

        if (ndigs > LONG_FLOAT_MAX_DIG)
            ndigs = LONG_FLOAT_MAX_DIG;

        ndigs = __lfloat_d_engine(fval, dtoa, ndigs, fmode, ndecimal);
        exp = dtoa->exp;
        ndigs_exp = 2;
    } else
#endif
    {
        FLOAT_UINT fval;
        int ndecimal = 0;
        bool fmode = false;

        fval = PRINTF_FLOAT_ARG(ap);

        if (!(local_flags & PRINTF_FLAG_PRECISION))
            local_prec = 6;
        if (conv == 'e') {
            ndigs = local_prec + 1;
            local_flags |= PRINTF_FLAG_FLOAT_EXP;
        } else if (conv == 'f') {
            ndigs = FLOAT_MAX_DIG;
            ndecimal = local_prec;
            local_flags |= PRINTF_FLAG_FLOAT_FIX;
            fmode = true;
        } else {
            conv += 'e' - 'g';
            ndigs = local_prec;
            if (ndigs < 1)
                ndigs = 1;
        }

        if (ndigs > FLOAT_MAX_DIG)
            ndigs = FLOAT_MAX_DIG;

        ndigs = __float_d_engine(fval, dtoa, ndigs, fmode, ndecimal);
        exp = dtoa->exp;
        ndigs_exp = 2;
    }

    if (exp < -9 || 9 < exp)
        ndigs_exp = 2;
    if (exp < -99 || 99 < exp)
        ndigs_exp = 3;
#if PRINTF_FLOAT_CAP_64
    if (exp < -999 || 999 < exp)
        ndigs_exp = 4;
#if PRINTF_FLOAT_CAP_LARGE
    if (exp < -9999 || 9999 < exp)
        ndigs_exp = 5;
#endif
#endif

    sign = 0;
    if (dtoa->flags & DTOA_MINUS)
        sign = '-';
    else if (local_flags & PRINTF_FLAG_PLUS)
        sign = '+';
    else if (local_flags & PRINTF_FLAG_SPACE)
        sign = ' ';

    if (dtoa->flags & (DTOA_NAN | DTOA_INF)) {
        const char *pnt;

        ndigs = sign ? 4 : 3;
        if (local_width > ndigs) {
            local_width -= ndigs;
            if (!(local_flags & PRINTF_FLAG_LEFT_ADJ)) {
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
        if (!(local_flags & (PRINTF_FLAG_FLOAT_EXP | PRINTF_FLAG_FLOAT_FIX))) {
            int req_prec;

            if (local_prec == 0)
                local_prec = 1;

            while (ndigs > 0 && dtoa->digits[ndigs - 1] == '0')
                ndigs--;

            req_prec = local_prec;

            if (!(local_flags & PRINTF_FLAG_ALT_FORM))
                local_prec = ndigs;

            if (-4 <= exp && exp < req_prec) {
                local_flags |= PRINTF_FLAG_FLOAT_FIX;

                if (exp < local_prec)
                    local_prec = local_prec - (exp + 1);
                else
                    local_prec = 0;
            } else {
                local_prec = local_prec - 1;
            }
        }

        if (local_flags & PRINTF_FLAG_FLOAT_FIX)
            n = (exp > 0 ? exp + 1 : 1);
        else
            n = 3 + ndigs_exp;

        if (sign)
            n += 1;
        if (local_prec)
            n += local_prec + 1;
        else if (local_flags & PRINTF_FLAG_ALT_FORM)
            n += 1;

        local_width = local_width > n ? local_width - n : 0;

        if (!(local_flags & (PRINTF_FLAG_LEFT_ADJ | PRINTF_FLAG_ZERO_FILL))) {
            while (local_width) {
                if (__printf_emit(out, stream_len, ' ') < 0)
                    return -1;
                local_width--;
            }
        }
        if (sign && __printf_emit(out, stream_len, sign) < 0)
            return -1;

        if (!(local_flags & PRINTF_FLAG_LEFT_ADJ)) {
            while (local_width) {
                if (__printf_emit(out, stream_len, '0') < 0)
                    return -1;
                local_width--;
            }
        }

        if (local_flags & PRINTF_FLAG_FLOAT_FIX) {
            char digit_out;

            n = exp > 0 ? exp : 0;
            do {
                if (n == -1 && __printf_emit(out, stream_len, '.') < 0)
                    return -1;

                if (0 <= exp - n && exp - n < ndigs)
                    digit_out = dtoa->digits[exp - n];
                else
                    digit_out = '0';
                if (--n < -local_prec)
                    break;
                if (__printf_emit(out, stream_len, (unsigned char) digit_out) < 0)
                    return -1;
            } while (1);
            if (__printf_emit(out, stream_len, (unsigned char) digit_out) < 0)
                return -1;
            if ((local_flags & PRINTF_FLAG_ALT_FORM) && n == -1 && __printf_emit(out, stream_len, '.') < 0)
                return -1;
        } else {
            int pos;

            if (__printf_emit(out, stream_len, (unsigned char) dtoa->digits[0]) < 0)
                return -1;
            if (local_prec > 0) {
                if (__printf_emit(out, stream_len, '.') < 0)
                    return -1;
                for (pos = 1; pos < 1 + local_prec; pos++) {
                    if (__printf_emit(out, stream_len,
                                      (unsigned char) (pos < ndigs ? dtoa->digits[pos] : '0'))
                        < 0)
                        return -1;
                }
            } else if (local_flags & PRINTF_FLAG_ALT_FORM) {
                if (__printf_emit(out, stream_len, '.') < 0)
                    return -1;
            }

            if (__printf_emit(out, stream_len, (unsigned char) TOCASE_LOCAL(conv)) < 0)
                return -1;
            sign = '+';
            if (exp < 0) {
                exp = -exp;
                sign = '-';
            }
            if (__printf_emit(out, stream_len, sign) < 0)
                return -1;
#if PRINTF_FLOAT_CAP_LARGE
            if (ndigs_exp > 4) {
                if (__printf_emit(out, stream_len, exp / 10000 + '0') < 0)
                    return -1;
                exp %= 10000;
            }
#endif
#if PRINTF_FLOAT_CAP_64
            if (ndigs_exp > 3) {
                if (__printf_emit(out, stream_len, exp / 1000 + '0') < 0)
                    return -1;
                exp %= 1000;
            }
#endif
            if (ndigs_exp > 2) {
                if (__printf_emit(out, stream_len, exp / 100 + '0') < 0)
                    return -1;
                exp %= 100;
            }
            if (ndigs_exp > 1) {
                if (__printf_emit(out, stream_len, exp / 10 + '0') < 0)
                    return -1;
                exp %= 10;
            }
            if (__printf_emit(out, stream_len, '0' + exp) < 0)
                return -1;
        }
    }

    *flags = local_flags;
    *prec = local_prec;
    *width = local_width;
#undef TOCASE_LOCAL
    return 0;
}
