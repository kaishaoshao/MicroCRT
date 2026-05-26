/*
 * Entry: %p
 */

static int
__printf_entry_pointer(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                       int *width, va_list ap, char *buf)
{
    ultoa_unsigned_t x;

    arg_to_unsigned(ap, *flags, x);
    return __printf_format_pointer(out, stream_len, flags, prec, width, x, buf);
}
