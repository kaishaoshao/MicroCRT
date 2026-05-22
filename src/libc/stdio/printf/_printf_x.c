/*
 * Entry: %x / %X
 */

static int
__printf_entry_x(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec, int *width,
                 ultoa_unsigned_t x, unsigned char prefix_c, char *buf)
{
    int base = ('x' - prefix_c) | 16;

    return __printf_format_int_base(out, stream_len, flags, prec, width, x, base, prefix_c, buf);
}
