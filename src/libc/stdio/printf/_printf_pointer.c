/*
 * Shared helper for pointer formatting.
 */

static int
__printf_format_pointer(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                        int *width, ultoa_unsigned_t x, char *buf)
{
    uint16_t local_flags = *flags | FL_ALT;

    if (sizeof(void *) > sizeof(int))
        local_flags |= FL_LONG;

    return __printf_format_int_base(out, stream_len, &local_flags, prec, width, x, 16, 'x', buf);
}
