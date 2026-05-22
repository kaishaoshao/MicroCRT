/*
 * Stream/output layer for the shared core body.
 *
 * Responsibilities:
 * - validate basic FILE write capability
 * - define the active output primitive used by the core loop
 * - acquire the stream lock and initialize write dispatch state
 */

#ifndef my_putc
#ifdef WIDE_CHARS
#define my_putc(c, out)                         \
    do {                                        \
        ++stream_len;                           \
        if ((out)->put((unsigned)(c), (out)->cookie) < 0) \
            goto fail;                          \
    } while (0)
#else
#define my_putc(c, out)                                  \
    do {                                                 \
        ++stream_len;                                    \
        if ((out)->put((unsigned char) (c), (out)->cookie) < 0) \
            goto fail;                                   \
    } while (0)
#endif
#endif

    if (__printf_out_begin(out) < 0)
        return EOF;
