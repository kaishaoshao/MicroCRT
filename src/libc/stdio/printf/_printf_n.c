/*
 * Entry: %n
 */

static int
__printf_entry_n(uint16_t flags, va_list ap, int stream_len)
{
    if (flags & FL_LONG) {
        if (flags & FL_REPD_TYPE)
            *va_arg(ap, long long *) = stream_len;
        else
            *va_arg(ap, long *) = stream_len;
    } else if (flags & FL_SHORT) {
        if (flags & FL_REPD_TYPE)
            *va_arg(ap, char *) = stream_len;
        else
            *va_arg(ap, short *) = stream_len;
    } else {
        *va_arg(ap, int *) = stream_len;
    }
    return 0;
}
