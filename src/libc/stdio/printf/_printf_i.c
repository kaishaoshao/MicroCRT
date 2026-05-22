/*
 * Entry: %i
 */

static int
__printf_entry_i(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec, int *width,
                 ultoa_signed_t x_s, char *buf)
{
    return __printf_format_int_dec(out, stream_len, flags, prec, width, x_s, buf);
}
