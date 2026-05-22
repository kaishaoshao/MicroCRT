/*
 * Entry: %p
 */

static int
__printf_entry_p(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec, int *width,
                 ultoa_unsigned_t x, char *buf)
{
    return __printf_format_pointer(out, stream_len, flags, prec, width, x, buf);
}
