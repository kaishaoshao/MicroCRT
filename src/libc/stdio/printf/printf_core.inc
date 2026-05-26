/*
 * Shared printf core template.
 *
 * This file contains the full core execution path for one configured
 * printf variant:
 * - function entry and local runtime state
 * - output stream initialization
 * - main parse/dispatch loop
 * - unified return and error exits
 */

int
PRINTF_CORE_SYMBOL(struct __printf_out *out, const CHAR *fmt, va_list ap_orig)
{
    /* Current conversion character after literal scanning. */
    unsigned c;

    /* Reused parse descriptor for each '%' sequence. */
    struct __printf_spec spec;

    /* Local aliases kept only to preserve existing helper signatures. */
#define flags spec.flags
#if PRINTF_CAP_POSITIONAL
    my_va_list  my_ap;
    const CHAR *fmt_orig = fmt;
#define argno spec.argno
#define ap my_ap.ap
#else
#define ap ap_orig
#endif
#if !PRINTF_CAP_SHRINK
#define width spec.width
#define prec spec.prec
#endif

    struct __printf_runtime {
        char buf[PRINTF_BUF_SIZE];
#if PRINTF_CAP_WCHAR
        wchar_t wbuf[PRINTF_BUF_SIZE / 2];
#endif
#if PRINTF_CAP_WIDETOMB
        char mb[MB_LEN_MAX];
#endif
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
        struct dtoa dtoa;
#endif
    };

    /* Shared scratch space consumed by conversion helpers in this core call. */
    struct __printf_runtime runtime;
    struct __printf_text_runtime text_runtime = {
        .buf = runtime.buf,
#if PRINTF_CAP_WCHAR
        .wbuf = runtime.wbuf,
#else
        .wbuf = NULL,
#endif
#if PRINTF_CAP_WIDETOMB
        .mb_buf = runtime.mb,
#endif
    };
    int stream_len = 0;

#if PRINTF_CAP_SECURE
    const char *msg = __printf_validate_core_inputs(out, fmt);

    if (msg != NULL)
        return __printf_handle_core_error(out, msg);
#endif

    if (__printf_out_begin(out) < 0)
        return EOF;

#if PRINTF_CAP_POSITIONAL
    va_copy(ap, ap_orig);
#endif

    for (;;) {
        int literal_status = __printf_scan_literal(out, &stream_len, &fmt, &c);

        switch (literal_status) {
        case -1:
            goto fail;
        case 0:
            goto ret;
        default:
            break;
        }

        __printf_spec_reset(&spec);

        do {
            if (__printf_parse_flag_char(&spec, c))
                continue;

            if (flags < PRINTF_FLAG_LONG) {
                if (__printf_parse_number_char(&spec, c))
                    continue;
                if (c == '*') {
#if PRINTF_CAP_POSITIONAL
                    /*
                     * Positional args must be used together, so wait
                     * for the value to appear before dealing with
                     * width and precision fields.
                     */
                    if (argno)
                        continue;
#endif
                    (void) __printf_parse_dynamic_field(&spec, ap);
                    continue;
                }
                if (c == '.') {
                    if (__printf_parse_precision_marker(&spec) < 0)
                        goto ret;
                    continue;
                }
#if PRINTF_CAP_POSITIONAL
                if (__printf_parse_positional_char(&spec, c, fmt_orig, &my_ap, ap_orig))
                    continue;
#endif
            }

            if (__printf_parse_size_modifier(&spec, c))
                continue;

            break;
        } while ((c = *fmt++) != 0);

#if PRINTF_CAP_POSITIONAL
        if (argno) {
            va_end(ap);
            va_copy(ap, ap_orig);
            skip_to_arg(fmt_orig, &my_ap, argno);
        }
#endif

#if !PRINTF_CAP_SHRINK
        /*
         * This can happen only when precision is set via '*', in which case
         * it behaves as if no precision had been specified.
         */
        __printf_spec_normalize(&spec);
#endif

        {
            int dispatch_status;

            dispatch_status = __printf_dispatch_conversion(out, &stream_len, &flags, &prec, &width, c,
                                                           ap, &text_runtime
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
                                                           ,
                                                           &runtime.dtoa
#endif
#if PRINTF_CAP_SECURE
                                                           ,
                                                           &msg
#endif
            );

            if (dispatch_status == __PRINTF_DISPATCH_ERROR)
                goto fail;
#if PRINTF_CAP_SECURE
            if (dispatch_status == __PRINTF_DISPATCH_SECURE_FAIL)
                goto handle_error;
#endif
            if (dispatch_status == __PRINTF_DISPATCH_SKIP_TAIL)
                continue;
        }

#if !PRINTF_CAP_SHRINK
        /* Left padding is handled during formatting; only right tail remains here. */
        if (__printf_emit_width_tail(out, &stream_len, &width) < 0)
            goto fail;
#endif
    }

ret:
#if PRINTF_CAP_POSITIONAL
    va_end(ap);
#endif
    return __printf_out_finish(out, stream_len);

fail:
#if PRINTF_CAP_POSITIONAL
    va_end(ap);
#endif
    return __printf_out_finish_failed(out);

#if PRINTF_CAP_SECURE
handle_error:
    return __printf_handle_core_error(out, msg);
#endif

/* Tear down local alias macros introduced in this template. */
#undef ap
#if PRINTF_CAP_POSITIONAL
#undef argno
#endif
#if !PRINTF_CAP_SHRINK
#undef width
#undef prec
#endif
#undef flags
}
