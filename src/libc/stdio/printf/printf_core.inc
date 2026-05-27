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

    struct __printf_runtime_bundle {
        struct __printf_core_runtime io;
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
        struct dtoa dtoa;
#endif
    };

    /* Shared scratch space consumed by conversion helpers in this core call. */
    struct __printf_runtime_bundle runtime;
    struct __printf_core_context core_ctx;

    __printf_core_context_init(&core_ctx, &runtime.io, fmt);
    core_ctx.msg = __printf_validate_core_inputs(out, fmt);

    if (core_ctx.msg != NULL)
        return __printf_handle_core_error(out, core_ctx.msg);

    if (__printf_out_begin(out) < 0)
        return EOF;

    __printf_positional_begin(&core_ctx, ap_orig);

    for (;;) {
        int literal_status = __printf_scan_literal(out, &core_ctx.stream_len, &fmt, &c);

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

            if (spec.flags < PRINTF_FLAG_LONG) {
                if (__printf_parse_number_char(&spec, c))
                    continue;
                if (c == '*') {
                    /*
                     * Positional args must be used together, so wait
                     * for the value to appear before dealing with
                     * width and precision fields.
                     */
                    if (__printf_spec_has_positional_arg(&spec))
                        continue;
                    (void) __printf_parse_dynamic_field(&spec,
#if PRINTF_CAP_POSITIONAL
                                                        core_ctx.positional.ap
#else
                                                        ap_orig
#endif
                    );
                    continue;
                }
                if (c == '.') {
                    if (__printf_parse_precision_marker(&spec) < 0)
                        goto ret;
                    continue;
                }
                if (__printf_parse_positional_char(&spec, c, core_ctx.fmt_orig, &core_ctx.positional,
                                                  ap_orig))
                    continue;
            }

            if (__printf_parse_size_modifier(&spec, c))
                continue;

            break;
        } while ((c = *fmt++) != 0);

        if (__printf_spec_has_positional_arg(&spec)) {
            __printf_positional_rewind_to_arg(&core_ctx, ap_orig, spec.argno);
        }

        __printf_spec_normalize(&spec);

        {
            int dispatch_status;

            dispatch_status = __printf_dispatch_conversion(out, &core_ctx.stream_len, &spec.flags,
                                                           &spec.prec, &spec.width, c,
#if PRINTF_CAP_POSITIONAL
                                                           core_ctx.positional.ap,
#else
                                                           ap_orig,
#endif
                                                           &core_ctx.text
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
                                                           ,
                                                           &runtime.dtoa
#endif
                                                           ,
                                                           &core_ctx.msg
            );

            if (dispatch_status == __PRINTF_DISPATCH_ERROR)
                goto fail;
            if (dispatch_status == __PRINTF_DISPATCH_SECURE_FAIL)
                goto handle_error;
            if (dispatch_status == __PRINTF_DISPATCH_SKIP_TAIL)
                continue;
        }

        /* Left padding is handled during formatting; only right tail remains here. */
        if (!__printf_cap_shrink_enabled()
            && __printf_emit_width_tail(out, &core_ctx.stream_len, &spec.width) < 0)
            goto fail;
    }

ret:
    __printf_positional_end(&core_ctx);
    return __printf_out_finish(out, core_ctx.stream_len);

fail:
    __printf_positional_end(&core_ctx);
    return __printf_out_finish_failed(out);

handle_error:
    __printf_positional_end(&core_ctx);
    return __printf_handle_core_error(out, core_ctx.msg);
}
