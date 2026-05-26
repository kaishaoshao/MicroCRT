/*
 * Entry: %x / %X
 */

static int
__printf_entry_unsigned_hex(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                            int *width, unsigned char conv, va_list ap, char *buf)
{
    ultoa_unsigned_t x;
    int base = ('x' - conv) | 16;

    arg_to_unsigned(ap, *flags, x);
    return __printf_format_int_base(out, stream_len, flags, prec, width, x, base, conv, buf);
}
