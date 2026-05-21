/*
 * Conversion: %n
 *
 * Responsibilities:
 * - write the current output count back through the typed pointer
 * - honor size modifiers
 * - reject %n under the secure VFPRINTF_S path
 */

{
#ifdef VFPRINTF_S
    msg = "format string contains percent-n";
    goto handle_error;
#else
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
#endif
}
