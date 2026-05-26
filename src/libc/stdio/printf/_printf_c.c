/*
 * Entry: %c / %lc
 */

static int
__printf_dispatch_char(struct __printf_out *out, int *stream_len, uint16_t flags, int *width,
                       va_list ap, struct __printf_text_runtime *text)
{
    return __printf_format_char(out, stream_len, flags, width, va_arg(ap, int), text);
}
