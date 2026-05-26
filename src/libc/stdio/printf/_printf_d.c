/*
 * Entry: %d / %i
 */

static int
__printf_entry_signed_decimal(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                              int *width, va_list ap, char *buf)
{
    ultoa_signed_t x_s;

    arg_to_signed(ap, *flags, x_s);
    return __printf_format_int_dec(out, stream_len, flags, prec, width, x_s, buf);
}
