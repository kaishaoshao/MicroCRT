/*
 * Shared support layer for vfprintf variants.
 *
 * This file provides formatter support definitions that are shared by all
 * vfprintf shells after variant capability selection has completed.
 *
 * It owns:
 * - common FL_* parser/formatter flags
 * - C99 size-spec handling helpers
 * - positional-argument scanning support
 * - narrow/wide helper utilities used by conversion code
 *
 * It does not own:
 * - shell selection
 * - main vfprintf function body
 * - per-specifier formatting logic
 */

#include "ultoa_invert.c"

/* Order is relevant here and matches order in format string */

#ifdef _NEED_IO_SHRINK
#define FL_ZFILL 0x0000
#define FL_PLUS  0x0000
#define FL_SPACE 0x0000
#define FL_LPAD  0x0000
#else
#define FL_ZFILL 0x0001
#define FL_PLUS  0x0002
#define FL_SPACE 0x0004
#define FL_LPAD  0x0008
#endif /* else _NEED_IO_SHRINK */
#define FL_ALT       0x0010

#define FL_WIDTH     0x0020
#define FL_PREC      0x0040

#define FL_LONG      0x0080
#define FL_SHORT     0x0100
#define FL_REPD_TYPE 0x0200

#define FL_NEGATIVE  0x0400

#ifdef _NEED_IO_C99_FORMATS
#define FL_FLTHEX 0x0800
#endif
#define FL_FLTEXP 0x1000
#define FL_FLTFIX 0x2000

#ifdef _NEED_IO_C99_FORMATS

#define CHECK_INT_SIZE(c, flags, letter, type)      \
    if (c == letter) {                              \
        if (sizeof(type) == sizeof(int))            \
            ;                                       \
        else if (sizeof(type) == sizeof(long))      \
            flags |= FL_LONG;                       \
        else if (sizeof(type) == sizeof(long long)) \
            flags |= FL_LONG | FL_REPD_TYPE;        \
        else if (sizeof(type) == sizeof(short))     \
            flags |= FL_SHORT;                      \
        continue;                                   \
    }

#define CHECK_C99_INT_SIZES(c, flags)         \
    CHECK_INT_SIZE(c, flags, 'j', intmax_t);  \
    CHECK_INT_SIZE(c, flags, 'z', size_t);    \
    CHECK_INT_SIZE(c, flags, 't', ptrdiff_t);

#else
#define CHECK_C99_INT_SIZES(c, flags)
#endif

#define CHECK_INT_SIZES(c, flags)      \
    {                                  \
        if (c == 'l') {                \
            if (flags & FL_LONG)       \
                flags |= FL_REPD_TYPE; \
            flags |= FL_LONG;          \
            continue;                  \
        }                              \
                                       \
        if (c == 'h') {                \
            if (flags & FL_SHORT)      \
                flags |= FL_REPD_TYPE; \
            flags |= FL_SHORT;         \
            continue;                  \
        }                              \
                                       \
        /* alias for 'll' */           \
        if (c == 'L') {                \
            flags |= FL_REPD_TYPE;     \
            flags |= FL_LONG;          \
            continue;                  \
        }                              \
        CHECK_C99_INT_SIZES(c, flags); \
    }

#ifdef _NEED_IO_POS_ARGS

typedef struct {
    va_list ap;
} my_va_list;

/*
 * Repeatedly scan the format string finding argument position values
 * and types to slowly walk the argument vector until it points at the
 * target_argno so that the outer printf code can then extract it.
 */
static void
skip_to_arg(const CHAR *fmt_orig, my_va_list *ap, int target_argno)
{
    unsigned    c; /* holds a char from the format string */
    uint16_t    flags;
    int         current_argno = 1;
    int         argno;
    int         width;
    const CHAR *fmt = fmt_orig;

    while (current_argno < target_argno) {
        for (;;) {
            c = *fmt++;
            if (!c)
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

        /*
         * Scan the format string until we hit current_argno. This can
         * either be a value, width or precision field.
         */
        do {
            if (flags < FL_WIDTH) {
                switch (c) {
                case '0':
                    continue;
                case '+':
                case ' ':
                    continue;
                case '-':
                    continue;
                case '#':
                    continue;
                case '\'':
                    continue;
                }
            }

            if (flags < FL_LONG) {
                if (c >= '0' && c <= '9') {
                    c -= '0';
                    width = 10 * width + c;
                    flags |= FL_WIDTH;
                    continue;
                }
                if (c == '$') {
                    /*
                     * If we've already seen the value position, any
                     * other positions will be either width or
                     * precisions. We can handle those in the same
                     * fashion as they're both 'int' type
                     */
                    if (argno) {
                        if (width == current_argno) {
                            c = 'c';
                            argno = width;
                            break;
                        }
                    } else
                        argno = width;
                    width = 0;
                    continue;
                }

                if (c == '*') {
                    width = 0;
                    continue;
                }

                if (c == '.') {
                    width = 0;
                    continue;
                }
            }

            CHECK_INT_SIZES(c, flags);

            break;
        } while ((c = *fmt++) != 0);
        if (argno == 0)
            break;
        if (argno == current_argno) {
            if ((TOLOWER(c) >= 'e' && TOLOWER(c) <= 'g')
#ifdef _NEED_IO_C99_FORMATS
                || TOLOWER(c) == 'a'
#endif
            ) {
                SKIP_FLOAT_ARG(flags, ap->ap);
            } else if (c == 'c') {
                (void)va_arg(ap->ap, int);
            } else if (c == 's') {
                (void)va_arg(ap->ap, char *);
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
#endif

#ifdef _NEED_IO_WIDETOMB
/*
 * Compute the number of bytes to encode a wide string
 * in the current locale
 */
static size_t
_mbslen(const wchar_t *s, size_t maxlen)
{
    mbstate_t ps = { 0 };
    wchar_t   c;
    char      tmp[MB_LEN_MAX];
    size_t    len = 0;
    while (len < maxlen && (c = *s++) != L'\0') {
        int clen;
        clen = __WCTOMB(tmp, c, &ps);
        if (clen == -1)
            return (size_t)clen;
        len += clen;
    }
    return len;
}

#endif

#ifdef _NEED_IO_MBTOWIDE
/*
 * Compute the number of wide chars to encode a multi-byte string
 * in the current locale
 */
static size_t
_wcslen(const char *s, size_t maxlen)
{
    mbstate_t ps = { 0 };
    wchar_t   c;
    size_t    len = 0;
    while (len < maxlen && *s != '\0') {
        size_t clen = mbrtowc(&c, s, MB_LEN_MAX, &ps);
        if (c == L'\0')
            break;
        if (clen == (size_t)-1)
            return clen;
        s += clen;
        len++;
    }
    return len;
}
#endif

#include "_printf_int_dec.c"
#include "_printf_int_base.c"
#include "_printf_pointer.c"
#include "_printf_text.c"
#include "_printf_parser.c"
#include "_printf_c.c"
#include "_printf_s.c"
#include "_printf_n.c"
#include "_printf_d.c"
#include "_printf_i.c"
#include "_printf_u.c"
#include "_printf_o.c"
#include "_printf_x.c"
#include "_printf_p.c"
#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
#include "_printf_fp_dec.c"
#include "_printf_fp_hex.c"
#include "_printf_f.c"
#include "_printf_e.c"
#include "_printf_g.c"
#include "_printf_a.c"
#endif
