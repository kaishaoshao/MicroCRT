#include "my_printf.h"
#include <stdbool.h>
#include <stdint.h>

// --- Internal Flags and Types ---
#define FLAG_LEFT_JUSTIFY (1 << 0)
#define FLAG_SIGN         (1 << 1)
#define FLAG_SPACE        (1 << 2)
#define FLAG_ALTERNATIVE  (1 << 3)
#define FLAG_ZERO_PAD     (1 << 4)

typedef enum {
    LEN_DEFAULT, LEN_CHAR, LEN_SHORT, LEN_LONG, LEN_LONG_LONG, LEN_SIZE_T, LEN_PTRDIFF_T
} len_modifier_t;

// --- Helper Functions ---
static void _putc(char c, my_printf_backend_t* backend) {
    backend->putc_f(c, backend->context);
}

static void _puts(const char* s, int len, my_printf_backend_t* backend) {
    for (int i = 0; i < len; ++i) {
        _putc(s[i], backend);
    }
}

static void print_number(my_printf_backend_t* backend, unsigned long long num, int base, bool is_upper, int width, int precision, int flags) {
    char buffer[21];
    int i = 0;
    const char* digits = is_upper ? "0123456789ABCDEF" : "0123456789abcdef";

    if (num == 0) {
        if (precision != 0) buffer[i++] = '0';
    } else {
        while (num > 0) {
            buffer[i++] = digits[num % base];
            num /= base;
        }
    }

    int num_len = i;
    int precision_pad = (precision > num_len) ? (precision - num_len) : 0;
    
    int total_len = num_len + precision_pad;
    if ((flags & FLAG_SIGN) || (flags & FLAG_SPACE)) total_len++;

    int width_pad = (width > total_len) ? (width - total_len) : 0;

    if (!(flags & FLAG_LEFT_JUSTIFY) && !(flags & FLAG_ZERO_PAD)) {
        while (width_pad-- > 0) _putc(' ', backend);
    }

    if (flags & FLAG_SIGN) _putc('+', backend);
    else if (flags & FLAG_SPACE) _putc(' ', backend);

    if (!(flags & FLAG_LEFT_JUSTIFY) && (flags & FLAG_ZERO_PAD)) {
        while (width_pad-- > 0) _putc('0', backend);
    }
    
    while (precision_pad-- > 0) _putc('0', backend);
    while (i > 0) _putc(buffer[--i], backend);

    if (flags & FLAG_LEFT_JUSTIFY) {
        while (width_pad-- > 0) _putc(' ', backend);
    }
}

// --- Core Implementation ---
void my_vprintf_core(my_printf_backend_t* backend, const char* format, va_list args) {
    const char* p = format;
    const char* segment_start = p;

    while (*p) {
        if (*p != '%') {
            p++;
            continue;
        }

        if (p > segment_start) {
            _puts(segment_start, p - segment_start, backend);
        }

        p++; // Skip '%'
        const char* format_start = p - 1;

        int flags = 0, width = 0, precision = -1;
        len_modifier_t len_mod = LEN_DEFAULT;

        bool parsing = true;
        while (parsing) {
            switch (*p) {
                case '-': flags |= FLAG_LEFT_JUSTIFY; p++; break;
                case '+': flags |= FLAG_SIGN; p++; break;
                case ' ': flags |= FLAG_SPACE; p++; break;
                case '#': flags |= FLAG_ALTERNATIVE; p++; break;
                case '0': flags |= FLAG_ZERO_PAD; p++; break;
                default: parsing = false; break;
            }
        }

        if (*p == '*') {
            width = va_arg(args, int);
            if (width < 0) { flags |= FLAG_LEFT_JUSTIFY; width = -width; }
            p++;
        } else {
            while (*p >= '0' && *p <= '9') width = width * 10 + (*p++ - '0');
        }

        if (*p == '.') {
            p++;
            precision = 0;
            if (*p == '*') { precision = va_arg(args, int); p++; }
            else { while (*p >= '0' && *p <= '9') precision = precision * 10 + (*p++ - '0'); }
        }

        switch (*p) {
            case 'h': p++; if (*p == 'h') { len_mod = LEN_CHAR; p++; } else { len_mod = LEN_SHORT; } break;
            case 'l': p++; if (*p == 'l') { len_mod = LEN_LONG_LONG; p++; } else { len_mod = LEN_LONG; } break;
            case 'z': len_mod = LEN_SIZE_T; p++; break;
            case 't': len_mod = LEN_PTRDIFF_T; p++; break;
        }

        bool handled = true;
        switch (*p) {
            case 'c': {
                char c = (char)va_arg(args, int);
                if (!(flags & FLAG_LEFT_JUSTIFY)) for (int i = 0; i < width - 1; ++i) _putc(' ', backend);
                _putc(c, backend);
                if (flags & FLAG_LEFT_JUSTIFY) for (int i = 0; i < width - 1; ++i) _putc(' ', backend);
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                if (!s) s = "(null)";
                int s_len = 0;
                for(const char* sp = s; *sp; ++sp) s_len++;
                if (precision >= 0 && precision < s_len) s_len = precision;
                if (!(flags & FLAG_LEFT_JUSTIFY)) for (int i = 0; i < width - s_len; ++i) _putc(' ', backend);
                _puts(s, s_len, backend);
                if (flags & FLAG_LEFT_JUSTIFY) for (int i = 0; i < width - s_len; ++i) _putc(' ', backend);
                break;
            }
            case 'd': case 'i': {
                long long val;
                if (len_mod == LEN_LONG_LONG) val = va_arg(args, long long);
                else if (len_mod == LEN_LONG) val = va_arg(args, long);
                else val = va_arg(args, int);
                if (val < 0) { _putc('-', backend); val = -val; if(width > 0) width--; }
                print_number(backend, (unsigned long long)val, 10, false, width, precision, flags);
                break;
            }
            case 'u': case 'o': case 'x': case 'X': {
                unsigned long long val;
                if (len_mod == LEN_LONG_LONG) val = va_arg(args, unsigned long long);
                else if (len_mod == LEN_LONG) val = va_arg(args, unsigned long);
                else val = va_arg(args, unsigned int);
                int base = (*p == 'o') ? 8 : ((*p == 'x' || *p == 'X') ? 16 : 10);
                print_number(backend, val, base, (*p == 'X'), width, precision, flags);
                break;
            }
            case 'p': {
                uintptr_t ptr = (uintptr_t)va_arg(args, void*);
                _puts("0x", 2, backend);
                print_number(backend, ptr, 16, false, 0, -1, 0); // Simplified pointer printing
                break;
            }
            case '%': _putc('%', backend); break;
            default: handled = false; break;
        }

        if (handled) {
            p++;
        } else {
            for (const char* pp = format_start; pp != p; ++pp) _putc(*pp, backend);
        }
        segment_start = p;
    }

    if (p > segment_start) {
        _puts(segment_start, p - segment_start, backend);
    }
}