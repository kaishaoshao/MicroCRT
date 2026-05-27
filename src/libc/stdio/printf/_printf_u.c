/*
 * Entry: %u
 */

static int
__printf_entry_unsigned_decimal(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                                int *width, va_list ap, char *buf)
{
    ultoa_unsigned_t x = __printf_read_unsigned_arg(ap, *flags);
    *flags &= ~PRINTF_FLAG_ALT_FORM;
    return __printf_format_int_base(out, stream_len, flags, prec, width, x, 10, '\0', buf);
}
