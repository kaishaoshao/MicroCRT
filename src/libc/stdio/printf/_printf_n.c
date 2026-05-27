/*
 * Entry: %n
 */

static int __printf_entry_n(uint16_t flags, va_list ap, int stream_len);

static int
__printf_dispatch_percent_n(uint16_t flags, va_list ap, int stream_len, const char **msg_out)
{
    if (__printf_cap_secure_enabled()) {
        (void) flags;
        (void) ap;
        (void) stream_len;
        if (msg_out != NULL)
            *msg_out = "format string contains percent-n";
        return 1;
    }

    (void) msg_out;
    return __printf_entry_n(flags, ap, stream_len);
}

static int
__printf_entry_n(uint16_t flags, va_list ap, int stream_len)
{
    if (flags & PRINTF_FLAG_LONG) {
        if (flags & PRINTF_FLAG_REPEAT_TYPE)
            *va_arg(ap, long long *) = stream_len;
        else
            *va_arg(ap, long *) = stream_len;
    } else if (flags & PRINTF_FLAG_SHORT) {
        if (flags & PRINTF_FLAG_REPEAT_TYPE)
            *va_arg(ap, char *) = stream_len;
        else
            *va_arg(ap, short *) = stream_len;
    } else {
        *va_arg(ap, int *) = stream_len;
    }
    return 0;
}
