/*
 * Core body layer.
 *
 * This is the actual vfprintf function skeleton. It owns:
 * - function entry / local state
 * - stream locking and write callbacks
 * - main format-string loop
 * - final error/return handling
 *
 * It delegates:
 * - format parsing to printf_core_parser.inc
 * - conversion dispatch to printf_core_dispatch.inc
 */

#ifdef VFPRINTF_S
int
vfprintf_s(struct __printf_out * __restrict out, const char * __restrict fmt, va_list ap_orig)
#else
int
PRINTF_NAME(struct __printf_out *out, const CHAR *fmt, va_list ap_orig)
#endif
{
    unsigned c; /* holds a char from the format string */
    struct __printf_spec spec;
#define flags spec.flags
#ifdef _NEED_IO_POS_ARGS
    my_va_list  my_ap;
    const CHAR *fmt_orig = fmt;
#define argno spec.argno
#define ap my_ap.ap
#else
#define ap ap_orig
#endif
#ifndef _NEED_IO_SHRINK
#define width spec.width
#define prec spec.prec
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
    int stream_len = 0;

#ifdef VFPRINTF_S
    const char *msg;

    if (out == NULL) {
        msg = "output sink is null";
        goto handle_error;
    } else if (fmt == NULL) {
        msg = "null format string";
        goto handle_error;
    }
#endif

#include "printf_core_stream.inc"

#ifdef _NEED_IO_POS_ARGS
    va_copy(ap, ap_orig);
#endif

    for (;;) {
#include "printf_core_parser.inc"
#include "printf_core_dispatch.inc"
    } /* for (;;) */

#include "printf_core_epilogue.inc"
}
