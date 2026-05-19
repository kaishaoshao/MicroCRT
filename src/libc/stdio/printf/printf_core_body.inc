#ifdef VFPRINTF_S
int
vfprintf_s(FILE * __restrict stream, const char * __restrict fmt, va_list ap_orig)
#else
int
vfprintf(FILE *stream, const CHAR *fmt, va_list ap_orig)
#endif
{
    unsigned c; /* holds a char from the format string */
    uint16_t flags;
#ifdef _NEED_IO_POS_ARGS
    int         argno;
    my_va_list  my_ap;
    const CHAR *fmt_orig = fmt;
#define ap my_ap.ap
#else
#define ap ap_orig
#endif
    union {
        char buf[PRINTF_BUF_SIZE]; /* size for -1 in smallest base, without '\0' */
#ifdef _NEED_IO_WCHAR
        wchar_t wbuf[PRINTF_BUF_SIZE / 2]; /* for wide char output */
#endif
#ifdef _NEED_IO_WIDETOMB
        char mb[MB_LEN_MAX];
#endif
#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
        struct dtoa dtoa;
#endif
    } u;
    const char *pnt;
#ifndef _NEED_IO_SHRINK
    size_t size;
#endif

    int stream_len = 0;

#ifdef VFPRINTF_S
    const char *msg;

    if (stream == NULL) {
        msg = "output stream is null";
        goto handle_error;
    } else if (fmt == NULL) {
        msg = "null format string";
        goto handle_error;
    }
#endif

#ifndef my_putc
#ifdef WIDE_CHARS
#define my_putc(c, stream)            \
    do {                              \
        ++stream_len;                 \
        if (putwc(c, stream) == WEOF) \
            goto fail;                \
    } while (0)
#else
    int (*put)(char, FILE *) = stream->put;
#define my_putc(c, stream)      \
    do {                        \
        ++stream_len;           \
        if (put(c, stream) < 0) \
            goto fail;          \
    } while (0)
#endif
#endif

    __flockfile(stream);

    if ((stream->flags & __SWR) == 0)
        __funlock_return(stream, EOF);

#ifdef _NEED_IO_POS_ARGS
    va_copy(ap, ap_orig);
#endif

    for (;;) {
#include "printf_core_parser.inc"
#include "printf_core_dispatch.inc"
    } /* for (;;) */

ret:
#ifdef _NEED_IO_POS_ARGS
    va_end(ap);
#endif
    __funlock_return(stream, stream_len);
#undef my_putc
#undef ap
fail:
    stream->flags |= __SERR;
    stream_len = -1;
    goto ret;
#ifdef VFPRINTF_S
handle_error:
    if (__cur_handler != NULL) {
        __cur_handler(msg, NULL, -1);
    }
    if (stream)
        stream->flags |= __SERR;
    return -1;
#endif
}
