/*
 * Entry: %a
 */

static int
__printf_entry_a(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec, int *width,
                 unsigned char case_convert, va_list ap, struct dtoa *dtoa)
{
    return __printf_format_fp_hex(out, stream_len, flags, prec, width, case_convert, ap, dtoa);
}
