/*
 * Shared helpers for parser state and literal scanning.
 */

#include "printf_core_private.h"

static inline int
__printf_scan_literal(struct __printf_out *out, int *stream_len, const CHAR **fmt, unsigned *c)
{
    for (;;) {
        *c = *(*fmt)++;
        if (!*c)
            return 0;
        if (*c == '%') {
            *c = *(*fmt)++;
            if (*c != '%')
                return 1;
        }
        if (__printf_emit(out, stream_len, *c) < 0)
            return -1;
    }
}

static inline void
__printf_spec_normalize(struct __printf_spec *spec)
{
    __printf_spec_normalize_fields(spec);
}

static inline int
__printf_parse_flag_char(struct __printf_spec *spec, unsigned c)
{
    if (spec->flags >= PRINTF_FLAG_WIDTH)
        return 0;

    switch (c) {
    case '0':
        spec->flags |= PRINTF_FLAG_ZERO_FILL;
        return 1;
    case '+':
        spec->flags |= PRINTF_FLAG_PLUS;
        __fallthrough;
    case ' ':
        spec->flags |= PRINTF_FLAG_SPACE;
        return 1;
    case '-':
        spec->flags |= PRINTF_FLAG_LEFT_ADJ;
        return 1;
    case '#':
        spec->flags |= PRINTF_FLAG_ALT_FORM;
        return 1;
    case '\'':
        /* C/POSIX locale keeps thousands_sep empty for this implementation. */
        return 1;
    default:
        return 0;
    }
}

static inline int
__printf_parse_number_char(struct __printf_spec *spec, unsigned c)
{
    if (spec->flags >= PRINTF_FLAG_LONG || c < '0' || c > '9')
        return 0;

    __printf_spec_accumulate_digit(spec, c);
    return 1;
}

static inline int
__printf_parse_dynamic_field(struct __printf_spec *spec, va_list ap)
{
    if (spec->flags >= PRINTF_FLAG_LONG)
        return 0;

    __printf_spec_read_dynamic_field(spec, ap);

    if (!(spec->flags & PRINTF_FLAG_PRECISION))
        spec->flags |= PRINTF_FLAG_WIDTH;
    return 1;
}

static inline int
__printf_parse_precision_marker(struct __printf_spec *spec)
{
    if (spec->flags >= PRINTF_FLAG_LONG)
        return 0;
    if (spec->flags & PRINTF_FLAG_PRECISION)
        return -1;

    spec->flags |= PRINTF_FLAG_PRECISION;
    return 1;
}

static inline int
__printf_parse_size_modifier(struct __printf_spec *spec, unsigned c)
{
    uint16_t flags = __printf_apply_size_modifier(spec->flags, c);

    if (flags == spec->flags)
        return 0;

    spec->flags = flags;
    return 1;
}

static inline int
__printf_parse_positional_char(struct __printf_spec *spec, unsigned c, const CHAR *fmt_orig,
                               my_va_list *my_ap, va_list ap_orig)
{
    if (!__printf_cap_positional_enabled() || spec->flags >= PRINTF_FLAG_LONG || c != '$')
        return 0;

#if PRINTF_CAP_POSITIONAL
    if (spec->argno) {
        va_end(my_ap->ap);
        va_copy(my_ap->ap, ap_orig);
        skip_to_arg(fmt_orig, my_ap, (spec->flags & PRINTF_FLAG_PRECISION) ? spec->prec : spec->width);
        if (spec->flags & PRINTF_FLAG_PRECISION)
            spec->prec = va_arg(my_ap->ap, int);
        else
            spec->width = va_arg(my_ap->ap, int);
    } else {
        spec->argno = spec->width;
        __printf_spec_reset_for_argno(spec);
    }
#else
    (void) fmt_orig;
    (void) my_ap;
    (void) ap_orig;
#endif

    return 1;
}
