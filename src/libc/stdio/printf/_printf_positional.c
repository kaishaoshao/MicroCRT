/*
 * Shared helpers for positional-argument scanning.
 */

#include "printf_core_private.h"

/*
 * Repeatedly scan the format string until the argument vector points at
 * target_argno so the outer parser/dispatcher can consume the desired arg.
 */
static inline void
skip_to_arg(const CHAR *fmt_orig, my_va_list *ap, int target_argno)
{
    unsigned    c;
    uint16_t    flags;
    int         current_argno = 1;
    int         argno;
    int         width;
    const CHAR *fmt = fmt_orig;

    while (current_argno < target_argno) {
        for (;;) {
            c = *fmt++;
            if (c == '\0')
                return;
            if (c == '%') {
                c = *fmt++;
                if (c != '%')
                    break;
            }
        }

        flags = 0;
        width = 0;
        argno = 0;

        do {
            uint16_t size_flags;

            if (flags < PRINTF_FLAG_WIDTH) {
                switch (c) {
                case '0':
                case '+':
                case ' ':
                case '-':
                case '#':
                case '\'':
                    continue;
                }
            }

            if (flags < PRINTF_FLAG_LONG) {
                if (c >= '0' && c <= '9') {
                    c -= '0';
                    width = 10 * width + (int) c;
                    flags |= PRINTF_FLAG_WIDTH;
                    continue;
                }

                if (c == '$') {
                    if (argno != 0) {
                        if (width == current_argno) {
                            c = 'c';
                            argno = width;
                            break;
                        }
                    } else {
                        argno = width;
                    }
                    width = 0;
                    continue;
                }

                if (c == '*' || c == '.') {
                    width = 0;
                    continue;
                }
            }

            size_flags = __printf_apply_size_modifier(flags, c);
            if (size_flags != flags) {
                flags = size_flags;
                continue;
            }

            break;
        } while ((c = *fmt++) != '\0');

        if (argno == 0)
            break;

        if (argno == current_argno) {
            if ((TOLOWER(c) >= 'e' && TOLOWER(c) <= 'g')
#ifdef PRINTF_CAP_C99_FORMATS
                || TOLOWER(c) == 'a'
#endif
            ) {
                SKIP_FLOAT_ARG(flags, ap->ap);
            } else if (c == 'c') {
                (void) va_arg(ap->ap, int);
            } else if (c == 's') {
                (void) va_arg(ap->ap, char *);
            } else if (c == 'd' || c == 'i') {
                ultoa_signed_t x_s;
                arg_to_signed(ap->ap, flags, x_s);
            } else {
                ultoa_unsigned_t x;
                arg_to_unsigned(ap->ap, flags, x);
            }

            ++current_argno;
            fmt = fmt_orig;
        }
    }
}
