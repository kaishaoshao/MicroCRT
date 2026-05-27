/*
 * Entry: %p
 */

static int
__printf_entry_pointer(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                       int *width, va_list ap, char *buf)
{
    ultoa_unsigned_t x = __printf_read_unsigned_arg(ap, *flags);
    return __printf_format_pointer(out, stream_len, flags, prec, width, x, buf);
}
