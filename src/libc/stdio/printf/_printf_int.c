/*
 * Conversion: %d / %i / %u / %o / %x / %X / %p
 *
 * Shared integer-family dispatch block.
 *
 * Specifier-specific work is already delegated to:
 * - _printf_d.c / _printf_i.c / _printf_u.c
 * - _printf_o.c / _printf_x.c / _printf_p.c
 */

{
    if (c == 'd') {
        ultoa_signed_t x_s;

        arg_to_signed(ap, flags, x_s);
        if (__printf_entry_d(out, &stream_len, &flags, &prec, &width, x_s, u.buf) < 0)
            goto fail;
    } else if (c == 'i') {
        ultoa_signed_t x_s;

        arg_to_signed(ap, flags, x_s);
        if (__printf_entry_i(out, &stream_len, &flags, &prec, &width, x_s, u.buf) < 0)
            goto fail;
    } else if (c == 'u') {
        ultoa_unsigned_t x;

        arg_to_unsigned(ap, flags, x);
        if (__printf_entry_u(out, &stream_len, &flags, &prec, &width, x, u.buf) < 0)
            goto fail;
    } else if (c == 'o') {
        ultoa_unsigned_t x;

        arg_to_unsigned(ap, flags, x);
        if (__printf_entry_o(out, &stream_len, &flags, &prec, &width, x, u.buf) < 0)
            goto fail;
    } else if (c == 'p') {
        ultoa_unsigned_t x;

        arg_to_unsigned(ap, flags, x);
        if (__printf_entry_p(out, &stream_len, &flags, &prec, &width, x, u.buf) < 0)
            goto fail;
    } else if (TOLOWER(c) == 'x') {
        ultoa_unsigned_t x;

        arg_to_unsigned(ap, flags, x);
        if (__printf_entry_x(out, &stream_len, &flags, &prec, &width, x, c, u.buf) < 0)
            goto fail;
#ifdef _NEED_IO_PERCENT_B
    } else if (TOLOWER(c) == 'b') {
        ultoa_unsigned_t x;

        arg_to_unsigned(ap, flags, x);
        if (__printf_format_int_base(out, &stream_len, &flags, &prec, &width, x, 2, c, u.buf) < 0)
            goto fail;
#endif
    } else {
        my_putc('%', out);
        my_putc(c, out);
        continue;
    }
}
