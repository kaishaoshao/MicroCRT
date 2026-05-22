/*
 * Cleanup/return layer for the shared core body.
 *
 * Responsibilities:
 * - positional-arg cleanup
 * - unlock and return value
 * - mark stream errors on failure
 * - handle secure-printf error exits
 */

ret:
#ifdef _NEED_IO_POS_ARGS
    va_end(ap);
#endif
    return __printf_out_finish(out, stream_len);
#undef my_putc
#undef ap
#ifdef _NEED_IO_POS_ARGS
#undef argno
#endif
#ifndef _NEED_IO_SHRINK
#undef width
#undef prec
#endif
#undef flags
fail:
    __printf_out_fail(out);
    stream_len = -1;
    goto ret;
#ifdef VFPRINTF_S
handle_error:
    if (__cur_handler != NULL) {
        __cur_handler(msg, NULL, -1);
    }
    if (out)
        __printf_out_fail(out);
    return -1;
#endif
