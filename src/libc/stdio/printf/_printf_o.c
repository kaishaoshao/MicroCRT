/*
 * Entry: %o
 */

static int
__printf_entry_o(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec, int *width,
                 ultoa_unsigned_t x, char *buf)
{
    return __printf_format_int_base(out, stream_len, flags, prec, width, x, 8, '\0', buf);
}
