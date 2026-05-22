/*
 * Entry: %f
 */

static int
__printf_entry_f(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec, int *width,
                 unsigned char case_convert, va_list ap, struct dtoa *dtoa)
{
    return __printf_format_fp_dec(out, stream_len, flags, prec, width, 'f', case_convert, ap, dtoa);
}
