/*
 * Shared conversion dispatcher for one parsed conversion descriptor.
 */

#define __PRINTF_DISPATCH_ERROR       (-1)
#define __PRINTF_DISPATCH_OK          0
#define __PRINTF_DISPATCH_SKIP_TAIL   1
#define __PRINTF_DISPATCH_SECURE_FAIL 2

static int
__printf_emit_unknown_conversion(struct __printf_out *out, int *stream_len, unsigned char conv)
{
    if (__printf_emit(out, stream_len, '%') < 0)
        return -1;
    if (__printf_emit(out, stream_len, conv) < 0)
        return -1;
    return 0;
}

static int
__printf_dispatch_float_conversion(struct __printf_out *out, int *stream_len, uint16_t *flags,
                                   int *prec, int *width, unsigned char conv, va_list ap,
                                   struct __printf_text_runtime *text
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
                                   ,
                                   struct dtoa *dtoa
#endif
)
{
    if (!__printf_conversion_is_float_family(conv))
        return 0;

#if !PRINTF_CAP_SHRINK
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
    switch (TOLOWER(conv)) {
        case 'f':
        return __printf_entry_float_f(out, stream_len, flags, prec, width, conv, ap, dtoa);
        case 'e':
        return __printf_entry_float_e(out, stream_len, flags, prec, width, conv, ap, dtoa);
        case 'g':
        return __printf_entry_float_g(out, stream_len, flags, prec, width, conv, ap, dtoa);
        case 'a':
        if (__printf_cap_c99_formats_enabled())
            return __printf_entry_float_a(out, stream_len, flags, prec, width, conv, ap, dtoa);
        return 0;
        default:
        return 0;
    }
#else
    __printf_skip_float_arg(*flags, ap);
    if (__printf_emit_string_common(out, stream_len, *flags, width, sizeof("*float*") - 1,
                                    "*float*", NULL, text)
        < 0)
        return -1;
    return 1;
#endif
#else
    (void) out;
    (void) stream_len;
    (void) flags;
    (void) prec;
    (void) width;
    (void) conv;
    (void) ap;
    (void) text;
    return 0;
#endif
}

static int
__printf_dispatch_string_conversion(struct __printf_out *out, int *stream_len, uint16_t flags,
                                    int *prec, int *width, va_list ap,
                                    const char **msg_out,
                                    struct __printf_text_runtime *text)
{
    int dispatch_ret = __printf_dispatch_string(out, stream_len, flags, prec, width, ap,
                                                msg_out, text);

    if (dispatch_ret < 0)
        return __PRINTF_DISPATCH_ERROR;
    if (__printf_cap_secure_enabled() && dispatch_ret > 0)
        return __PRINTF_DISPATCH_SECURE_FAIL;
    return __PRINTF_DISPATCH_OK;
}

static int
__printf_dispatch_percent_n_conversion(uint16_t flags, va_list ap, int stream_len,
                                       const char **msg_out
)
{
    int dispatch_ret;

    if (!__printf_cap_percent_n_enabled() && !__printf_cap_secure_enabled())
        return 0;

    dispatch_ret = __printf_dispatch_percent_n(flags, ap, stream_len, msg_out);
    if (dispatch_ret < 0)
        return __PRINTF_DISPATCH_ERROR;
    if (__printf_cap_secure_enabled() && dispatch_ret > 0)
        return __PRINTF_DISPATCH_SECURE_FAIL;
    return __PRINTF_DISPATCH_OK;
}

static int
__printf_dispatch_integer_conversion(struct __printf_out *out, int *stream_len, uint16_t *flags,
                                     int *prec, int *width, unsigned char conv, va_list ap,
                                     char *buf)
{
    switch (TOLOWER(conv)) {
    case 'd':
    case 'i':
        if (__printf_entry_signed_decimal(out, stream_len, flags, prec, width, ap, buf) < 0)
            return __PRINTF_DISPATCH_ERROR;
        return __PRINTF_DISPATCH_OK;
    case 'u':
        if (__printf_entry_unsigned_decimal(out, stream_len, flags, prec, width, ap, buf) < 0)
            return __PRINTF_DISPATCH_ERROR;
        return __PRINTF_DISPATCH_OK;
    case 'o':
        if (__printf_entry_unsigned_octal(out, stream_len, flags, prec, width, ap, buf) < 0)
            return __PRINTF_DISPATCH_ERROR;
        return __PRINTF_DISPATCH_OK;
    case 'x':
        if (__printf_entry_unsigned_hex(out, stream_len, flags, prec, width, conv, ap, buf) < 0)
            return __PRINTF_DISPATCH_ERROR;
        return __PRINTF_DISPATCH_OK;
    case 'p':
        if (__printf_entry_pointer(out, stream_len, flags, prec, width, ap, buf) < 0)
            return __PRINTF_DISPATCH_ERROR;
        return __PRINTF_DISPATCH_OK;
#if PRINTF_CAP_BINARY
    case 'b':
        if (!__printf_cap_binary_enabled())
            break;
        if (__printf_entry_unsigned_binary(out, stream_len, flags, prec, width, conv, ap, buf) < 0)
            return __PRINTF_DISPATCH_ERROR;
        return __PRINTF_DISPATCH_OK;
#endif
    default:
        break;
    }

    if (__printf_emit_unknown_conversion(out, stream_len, conv) < 0)
        return __PRINTF_DISPATCH_ERROR;
    return __PRINTF_DISPATCH_SKIP_TAIL;
}

static int
__printf_dispatch_conversion(struct __printf_out *out, int *stream_len, uint16_t *flags, int *prec,
                             int *width, unsigned char conv, va_list ap,
                             struct __printf_text_runtime *text
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
                             ,
                             struct dtoa *dtoa
#endif
                             ,
                             const char **msg_out
)
{
    int dispatch_status;

    dispatch_status = __printf_dispatch_float_conversion(out, stream_len, flags, prec, width, conv,
                                                         ap, text
#if IO_VARIANT_IS_FLOAT(PRINTF_CORE_PROFILE)
                                                         ,
                                                         dtoa
#endif
    );
    if (dispatch_status != 0)
        return dispatch_status < 0 ? __PRINTF_DISPATCH_ERROR : __PRINTF_DISPATCH_OK;

    if (conv == 'c') {
        if (__printf_dispatch_char(out, stream_len, *flags, width, ap, text) < 0)
            return __PRINTF_DISPATCH_ERROR;
        return __PRINTF_DISPATCH_OK;
    }

    if (conv == 's')
        return __printf_dispatch_string_conversion(out, stream_len, *flags, prec, width, ap, msg_out,
                                                   text);

    if (conv == 'n')
        return __printf_dispatch_percent_n_conversion(*flags, ap, *stream_len, msg_out);

    return __printf_dispatch_integer_conversion(out, stream_len, flags, prec, width, conv, ap,
                                                text->buf);
}
