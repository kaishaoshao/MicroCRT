/*
 * Entry: %u
 */

static int
__printf_entry_u(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec, int *width,
                 ultoa_unsigned_t x, char *buf)
{
    *flags &= ~FL_ALT;
    return __printf_format_int_base(out, stream_len, flags, prec, width, x, 10, '\0', buf);
}
