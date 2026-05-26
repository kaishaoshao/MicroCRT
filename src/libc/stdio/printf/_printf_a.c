/*
 * Entry: %a / %A
 */

static int
__printf_entry_float_a(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                       int *width, unsigned char conv, va_list ap, struct dtoa *dtoa)
{
    unsigned char case_convert = TOLOWER(conv) - conv;

    return __printf_format_fp_hex(out, stream_len, flags, prec, width, case_convert, ap, dtoa);
}
