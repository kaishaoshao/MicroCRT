/* Copyright (c) 2002, Alexander Popov (sasho@vip.bg)
   Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2005, Helmut Wallner
   Copyright (c) 2007, Dmitry Xmelkov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/* From: Id: printf_p_new.c,v 1.1.1.9 2002/10/15 20:10:28 joerg_wunsch Exp */
/* $Id: vfprintf.c 2191 2010-11-05 13:45:57Z arcanum $ */

#ifndef PRINTF_NAME
#define PRINTF_VARIANT __IO_VARIANT_DOUBLE
#define PRINTF_NAME __d_vfprintf
#endif

#include "../../libm/common/math_config.h"
#include "../stdlib/local.h"
#include "stdio_private.h"

/*
 * This file can be compiled into more than one flavour by setting
 * PRINTF_VARIANT to:
 *
 *  __IO_VARIANT_MINIMAL: limited integer-only support with option for long long
 *
 *  __IO_VARIANT_INTEGER: integer support except for long long with
 *              options for positional params
 *
 *  __IO_VARIANT_LLONG: full integer support including long long with
 *                options for positional params
 *
 *  __IO_VARIANT_FLOAT: full integer support along with float, but not double
 *
 *  __IO_VARIANT_DOUBLE: full support
 */

#if __IO_DEFAULT != PRINTF_VARIANT || defined(WIDE_CHARS)
#define vfprintf PRINTF_NAME
#endif

#ifdef WIDE_CHARS
#define CHAR wchar_t
#if __SIZEOF_WCHAR_T__ == 2
#define UCHAR uint16_t
#elif __SIZEOF_WCHAR_T__ == 4
#define UCHAR uint32_t
#endif
#else
#define CHAR char
#define UCHAR unsigned char
#endif

/*
 * Compute which features are required. This should match the _HAS
 * values computed in stdio.h
 */

#if PRINTF_VARIANT == __IO_VARIANT_MINIMAL

#define _NEED_IO_SHRINK
#if defined(__IO_MINIMAL_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#ifdef __IO_C99_FORMATS
#define _NEED_IO_C99_FORMATS
#endif

#elif PRINTF_VARIANT == __IO_VARIANT_INTEGER

#if defined(__IO_LONG_LONG) && __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#ifdef __IO_POS_ARGS
#define _NEED_IO_POS_ARGS
#endif
#ifdef __IO_C99_FORMATS
#define _NEED_IO_C99_FORMATS
#endif
#ifdef __IO_PERCENT_B
#define _NEED_IO_PERCENT_B
#endif

#elif PRINTF_VARIANT == __IO_VARIANT_LLONG

#if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#ifdef __IO_POS_ARGS
#define _NEED_IO_POS_ARGS
#endif
#ifdef __IO_C99_FORMATS
#define _NEED_IO_C99_FORMATS
#endif
#ifdef __IO_PERCENT_B
#define _NEED_IO_PERCENT_B
#endif

#elif PRINTF_VARIANT == __IO_VARIANT_FLOAT

#if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#define _NEED_IO_POS_ARGS
#define _NEED_IO_C99_FORMATS
#ifdef __IO_PERCENT_B
#define _NEED_IO_PERCENT_B
#endif
#define _NEED_IO_FLOAT

#elif PRINTF_VARIANT == __IO_VARIANT_DOUBLE

#if __SIZEOF_LONG_LONG__ > __SIZEOF_LONG__
#define _NEED_IO_LONG_LONG
#endif
#if defined(_HAS_IO_WCHAR) || defined(WIDE_CHARS)
#define _NEED_IO_WCHAR
#endif
#define _NEED_IO_POS_ARGS
#define _NEED_IO_C99_FORMATS
#ifdef __IO_PERCENT_B
#define _NEED_IO_PERCENT_B
#endif
#define _NEED_IO_DOUBLE
#if defined(__IO_LONG_DOUBLE) && __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#define _NEED_IO_LONG_DOUBLE
#endif

#else

#error invalid PRINTF_VARIANT

#endif

/* Figure out which multi-byte char support we need */
#if defined(_NEED_IO_WCHAR) && defined(__MB_CAPABLE)
#ifdef WIDE_CHARS
/* need to convert multi-byte chars to wide chars */
#define _NEED_IO_MBTOWIDE
#else
#define _NEED_IO_WIDETOMB
#endif
#endif

#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
#include "dtoa.h"
#endif

#if defined(_NEED_IO_FLOAT_LONG_DOUBLE) &&                                     \
    __SIZEOF_LONG_DOUBLE__ > __SIZEOF_DOUBLE__
#define SKIP_FLOAT_ARG(flags, ap)                                              \
  do {                                                                         \
    if ((flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE))        \
      (void)va_arg(ap, long double);                                           \
    else                                                                       \
      (void)va_arg(ap, double);                                                \
  } while (0)
#define FLOAT double
#elif defined(_NEED_IO_FLOAT)
#define SKIP_FLOAT_ARG(flags, ap) (void)va_arg(ap, uint32_t)
#define FLOAT_UINT uint32_t
#else
#define SKIP_FLOAT_ARG(flags, ap) (void)va_arg(ap, double)
#if __SIZEOF_DOUBLE__ == 8
#define FLOAT_UINT uint64_t
#elif __SIZEOF_DOUBLE__ == 4
#define FLOAT_UINT uint32_t
#endif
#endif

#ifdef _NEED_IO_LONG_LONG
typedef unsigned long long ultoa_unsigned_t;
typedef long long ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG_LONG__
#define arg_to_t(ap, flags, _s_, _result_)                                     \
  if ((flags) & FL_LONG) {                                                     \
    if ((flags) & FL_REPD_TYPE)                                                \
      (_result_) = va_arg(ap, _s_ long long);                                  \
    else                                                                       \
      (_result_) = va_arg(ap, _s_ long);                                       \
  } else {                                                                     \
    (_result_) = va_arg(ap, _s_ int);                                          \
    if ((flags) & FL_SHORT) {                                                  \
      if ((flags) & FL_REPD_TYPE)                                              \
        (_result_) = (_s_ char)(_result_);                                     \
      else                                                                     \
        (_result_) = (_s_ short)(_result_);                                    \
    }                                                                          \
  }
#else
typedef unsigned long ultoa_unsigned_t;
typedef long ultoa_signed_t;
#define SIZEOF_ULTOA __SIZEOF_LONG__
#define arg_to_t(ap, flags, _s_, _result_)                                     \
  if ((flags) & FL_LONG) {                                                     \
    if ((flags) & FL_REPD_TYPE)                                                \
      (_result_) = (_s_ long)va_arg(ap, _s_ long long);                        \
    else                                                                       \
      (_result_) = va_arg(ap, _s_ long);                                       \
  } else {                                                                     \
    (_result_) = va_arg(ap, _s_ int);                                          \
    if ((flags) & FL_SHORT) {                                                  \
      if ((flags) & FL_REPD_TYPE)                                              \
        (_result_) = (_s_ char)(_result_);                                     \
      else                                                                     \
        (_result_) = (_s_ short)(_result_);                                    \
    }                                                                          \
  }
#endif

#if SIZEOF_ULTOA <= 4
#ifdef _NEED_IO_PERCENT_B
#define PRINTF_BUF_SIZE 32
#else
#define PRINTF_BUF_SIZE 11
#endif
#else
#ifdef _NEED_IO_PERCENT_B
#define PRINTF_BUF_SIZE 64
#else
#define PRINTF_BUF_SIZE 22
#endif
#endif

// At the call site the address of the result_var is taken (e.g. "&ap")
// That way, it's clear that these macros *will* modify that variable
#define arg_to_unsigned(ap, flags, result_var)                                 \
  arg_to_t(ap, flags, unsigned, result_var)
#define arg_to_signed(ap, flags, result_var)                                   \
  arg_to_t(ap, flags, signed, result_var)

#include "ultoa_invert.c"

/* Order is relevant here and matches order in format string */

#ifdef _NEED_IO_SHRINK
#define FL_ZFILL 0x0000
#define FL_PLUS 0x0000
#define FL_SPACE 0x0000
#define FL_LPAD 0x0000
#else
#define FL_ZFILL 0x0001
#define FL_PLUS 0x0002
#define FL_SPACE 0x0004
#define FL_LPAD 0x0008
#endif /* else _NEED_IO_SHRINK */
#define FL_ALT 0x0010

#define FL_WIDTH 0x0020
#define FL_PREC 0x0040

#define FL_LONG 0x0080
#define FL_SHORT 0x0100
#define FL_REPD_TYPE 0x0200

#define FL_NEGATIVE 0x0400

#ifdef _NEED_IO_C99_FORMATS
#define FL_FLTHEX 0x0800
#endif
#define FL_FLTEXP 0x1000
#define FL_FLTFIX 0x2000

#ifdef _NEED_IO_C99_FORMATS

#define CHECK_INT_SIZE(c, flags, letter, type)                                 \
  if (c == letter) {                                                           \
    if (sizeof(type) == sizeof(int))                                           \
      ;                                                                        \
    else if (sizeof(type) == sizeof(long))                                     \
      flags |= FL_LONG;                                                        \
    else if (sizeof(type) == sizeof(long long))                                \
      flags |= FL_LONG | FL_REPD_TYPE;                                         \
    else if (sizeof(type) == sizeof(short))                                    \
      flags |= FL_SHORT;                                                       \
    continue;                                                                  \
  }

#define CHECK_C99_INT_SIZES(c, flags)                                          \
  CHECK_INT_SIZE(c, flags, 'j', intmax_t);                                     \
  CHECK_INT_SIZE(c, flags, 'z', size_t);                                       \
  CHECK_INT_SIZE(c, flags, 't', ptrdiff_t);

#else
#define CHECK_C99_INT_SIZES(c, flags)
#endif

#define CHECK_INT_SIZES(c, flags)                                              \
  {                                                                            \
    if (c == 'l') {                                                            \
      if (flags & FL_LONG)                                                     \
        flags |= FL_REPD_TYPE;                                                 \
      flags |= FL_LONG;                                                        \
      continue;                                                                \
    }                                                                          \
                                                                               \
    if (c == 'h') {                                                            \
      if (flags & FL_SHORT)                                                    \
        flags |= FL_REPD_TYPE;                                                 \
      flags |= FL_SHORT;                                                       \
      continue;                                                                \
    }                                                                          \
                                                                               \
    /* alias for 'll' */                                                       \
    if (c == 'L') {                                                            \
      flags |= FL_REPD_TYPE;                                                   \
      flags |= FL_LONG;                                                        \
      continue;                                                                \
    }                                                                          \
    CHECK_C99_INT_SIZES(c, flags);                                             \
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
static void skip_to_arg(const CHAR *fmt_orig, my_va_list *ap,
                        int target_argno) {
  unsigned c; /* holds a char from the format string */
  uint16_t flags;
  int current_argno = 1;
  int argno;
  int width;
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
static size_t _mbslen(const wchar_t *s, size_t maxlen) {
  mbstate_t ps = {0};
  wchar_t c;
  char tmp[MB_LEN_MAX];
  size_t len = 0;
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
static size_t _wcslen(const char *s, size_t maxlen) {
  mbstate_t ps = {0};
  wchar_t c;
  size_t len = 0;
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


/* ======================================================================== */
/* 现代格式化核心引擎架构 (上下文与路由设计)                                */
/* ======================================================================== */

/* 定义通用的输出回调接口 */
typedef int (*out_fct_t)(void *ctx, unsigned c);

/* 1. 统一的格式化状态上下文 */
typedef struct {
    void *ctx;              /* 输出目标上下文 */
    out_fct_t out_fn;       /* 底层字符输出回调 */
    int stream_len;         /* 当前已输出的总字符数 */
    const char *error_msg;  /* 用于安全模式的异常信息记录 */
} print_state_t;

/* 助手函数输出宏：自动累计字符数，并在失败时引发中断 */
#define HELPER_PUTC(ch) \
    do { \
        if (state->out_fn(state->ctx, (ch)) < 0) return -1; \
        chars_written++; \
    } while (0)


/* ======================================================================== */
/* 【模块分离】极简版处理器 (Shrink Mode) - 纯粹的内核逻辑，无视填充和精度  */
/* ======================================================================== */

static int print_char_shrink(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    int chars_written = 0;
    (void)c; (void)flags; (void)width; (void)prec;
    HELPER_PUTC(va_arg(*ap_ptr, int));
    return chars_written;
}

static int print_str_shrink(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    int chars_written = 0;
    (void)c; (void)flags; (void)width; (void)prec;
    const char *pnt = va_arg(*ap_ptr, char *);
    if (!pnt) pnt = "(null)";
    char ch;
    while ((ch = *pnt++)) HELPER_PUTC(ch);
    return chars_written;
}

static int print_int_shrink(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    int chars_written = 0;
    char buf[PRINTF_BUF_SIZE];
    int buf_len;
    (void)width; (void)prec;

    if (c == 'd' || c == 'i') {
        ultoa_signed_t x_s;
        arg_to_signed(*ap_ptr, flags, x_s);
        if (x_s < 0) {
            x_s = (ultoa_signed_t) - (ultoa_unsigned_t)x_s;
            flags |= FL_NEGATIVE;
        }
        flags &= ~FL_ALT;
        buf_len = __ultoa_invert(x_s, buf, 10) - buf;
    } else {
        int base;
        ultoa_unsigned_t x;
        if (c == 'u') { flags &= ~FL_ALT; base = 10; }
        else if (c == 'o') { base = 8; c = '\0'; }
        else if (c == 'p') { base = 16; flags |= FL_ALT; c = 'x'; if (sizeof(void*) > sizeof(int)) flags |= FL_LONG; }
        else if (TOLOWER(c) == 'x') { base = ('x' - c) | 16; }
#ifdef _NEED_IO_PERCENT_B
        else if (TOLOWER(c) == 'b') { base = 2; }
#endif
        else {
            HELPER_PUTC('%'); HELPER_PUTC(c);
            return chars_written;
        }

        flags &= ~(FL_PLUS | FL_SPACE);
        arg_to_unsigned(*ap_ptr, flags, x);
        if (x == 0) flags &= ~FL_ALT;
        buf_len = __ultoa_invert(x, buf, base) - buf;
    }

    if (flags & FL_ALT) {
        HELPER_PUTC('0');
        if (c != '\0') HELPER_PUTC(c);
    } else if (flags & FL_NEGATIVE) {
        HELPER_PUTC('-');
    }

    while (buf_len) HELPER_PUTC(buf[--buf_len]);
    return chars_written;
}


/* ======================================================================== */
/* 【模块分离】完整版处理器 (Full Mode) - 包含精细的填充、对齐和宽度控制    */
/* ======================================================================== */

static int print_char_full(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    int chars_written = 0;
    (void)c; (void)prec;
    size_t size = 1;
#ifdef _NEED_IO_WCHAR
    wchar_t wc = 0;
#ifdef _NEED_IO_WIDETOMB
    char mb[MB_LEN_MAX];
    int mb_len = 0;
#endif
    if (flags & FL_LONG) {
        wc = (wchar_t)va_arg(*ap_ptr, wint_t);
#ifdef _NEED_IO_WIDETOMB
        mbstate_t ps = { 0 };
        mb_len = __WCTOMB(mb, wc, &ps);
        size = (mb_len > 0) ? mb_len : 0;
#endif
    }
#endif

    if (!(flags & FL_LPAD)) {
        while ((size_t)width > size) { HELPER_PUTC(' '); width--; }
    }

#ifdef _NEED_IO_WCHAR
    if (flags & FL_LONG) {
#ifdef _NEED_IO_WIDETOMB
        for (int i = 0; i < mb_len; i++) HELPER_PUTC(mb[i]);
#else
        HELPER_PUTC((unsigned)wc);
#endif
    } else
#endif
    {
        HELPER_PUTC((unsigned)(char)va_arg(*ap_ptr, int));
    }

    if (flags & FL_LPAD) {
        while ((size_t)width > size) { HELPER_PUTC(' '); width--; }
    }
    return chars_written;
}

static int print_str_full(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    int chars_written = 0;
    (void)c;
    size_t size;
    const char *pnt = NULL;
#ifdef _NEED_IO_WCHAR
    wchar_t *wstr = NULL;
    if (flags & FL_LONG) {
        wstr = va_arg(*ap_ptr, wchar_t *);
        if (!wstr) {
#ifdef VFPRINTF_S
            state->error_msg = "arg corresponding to '%s' is null";
            return -2;
#endif
            pnt = "(null)"; wstr = NULL; goto handle_narrow;
        }
        size = (flags & FL_PREC) ? (size_t)prec : SIZE_MAX;
#ifdef _NEED_IO_WIDETOMB
        size = _mbslen(wstr, size);
        if (size == (size_t)-1) return -1;
#else
        size = wcsnlen(wstr, size);
#endif
    } else
#endif
    {
        pnt = va_arg(*ap_ptr, char *);
#ifdef _NEED_IO_WCHAR
    handle_narrow:
#endif
        if (!pnt) {
#ifdef VFPRINTF_S
            state->error_msg = "arg corresponding to '%s' is null";
            return -2;
#endif
            pnt = "(null)";
        }
        size = (flags & FL_PREC) ? (size_t)prec : SIZE_MAX;
#ifdef _NEED_IO_MBTOWIDE
        size = _wcslen(pnt, size);
#else
        size = strnlen(pnt, size);
#endif
    }

    if (!(flags & FL_LPAD)) {
        while ((size_t)width > size) { HELPER_PUTC(' '); width--; }
    }
    width -= size;

#ifdef _NEED_IO_WCHAR
    if (wstr) {
#ifdef _NEED_IO_WIDETOMB
        mbstate_t ps = { 0 };
        while (size) {
            wchar_t wc = *wstr++;
            char m[MB_LEN_MAX];
            int mb_len = __WCTOMB(m, wc, &ps);
            int i = 0;
            while (size && mb_len) { HELPER_PUTC(m[i++]); size--; mb_len--; }
        }
#else
        while (size--) HELPER_PUTC(*wstr++);
#endif
    } else
#endif
    {
#ifdef _NEED_IO_MBTOWIDE
        mbstate_t ps = { 0 };
        while (size--) {
            wchar_t wc; size_t mb_len = mbrtowc(&wc, pnt, MB_LEN_MAX, &ps);
            HELPER_PUTC(wc); pnt += mb_len;
        }
#else
        while (size--) HELPER_PUTC(*pnt++);
#endif
    }

    if (flags & FL_LPAD) {
        while (width > 0) { HELPER_PUTC(' '); width--; }
    }
    return chars_written;
}

static int print_int_full(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    int chars_written = 0;
    char buf[PRINTF_BUF_SIZE];
    int buf_len;

    if (c == 'd' || c == 'i') {
        ultoa_signed_t x_s;
        arg_to_signed(*ap_ptr, flags, x_s);
        if (x_s < 0) { x_s = (ultoa_signed_t) - (ultoa_unsigned_t)x_s; flags |= FL_NEGATIVE; }
        flags &= ~FL_ALT;
        if (x_s == 0 && (flags & FL_PREC) && prec == 0) buf_len = 0;
        else buf_len = __ultoa_invert(x_s, buf, 10) - buf;
    } else {
        int base; ultoa_unsigned_t x;
        if (c == 'u') { flags &= ~FL_ALT; base = 10; }
        else if (c == 'o') { base = 8; c = '\0'; }
        else if (c == 'p') { base = 16; flags |= FL_ALT; c = 'x'; if (sizeof(void*) > sizeof(int)) flags |= FL_LONG; }
        else if (TOLOWER(c) == 'x') { base = ('x' - c) | 16; }
#ifdef _NEED_IO_PERCENT_B
        else if (TOLOWER(c) == 'b') { base = 2; }
#endif
        else { HELPER_PUTC('%'); HELPER_PUTC(c); return chars_written; }

        flags &= ~(FL_PLUS | FL_SPACE);
        arg_to_unsigned(*ap_ptr, flags, x);
        if (x == 0) flags &= ~FL_ALT;
        if (x == 0 && (flags & FL_PREC) && prec == 0) buf_len = 0;
        else buf_len = __ultoa_invert(x, buf, base) - buf;
    }

    int len = buf_len;
    if (flags & FL_PREC) {
        flags &= ~FL_ZFILL;
        if (len < prec) { len = prec; if (c == '\0') flags &= ~FL_ALT; }
    }
    if (flags & FL_ALT) { len += 1; if (c != '\0') len += 1; }
    else if (flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) { len += 1; }

    if (!(flags & FL_LPAD)) {
        if (flags & FL_ZFILL) {
            prec = buf_len;
            if (len < width) { prec += width - len; len = width; }
        }
        while (len < width) { HELPER_PUTC(' '); len++; }
    }
    width -= len;

    if (flags & FL_ALT) {
        HELPER_PUTC('0'); if (c != '\0') HELPER_PUTC(c);
    } else if (flags & (FL_NEGATIVE | FL_PLUS | FL_SPACE)) {
        unsigned char z = ' ';
        if (flags & FL_PLUS) z = '+';
        if (flags & FL_NEGATIVE) z = '-';
        HELPER_PUTC(z);
    }

    while (prec > buf_len) { HELPER_PUTC('0'); prec--; }
    while (buf_len) HELPER_PUTC(buf[--buf_len]);

    if (flags & FL_LPAD) {
        while (width > 0) { HELPER_PUTC(' '); width--; }
    }
    return chars_written;
}


/* ======================================================================== */
/* 浮点数处理器区 (Float Handlers)                                          */
/* ======================================================================== */

#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
#define TOCASE(c) ((c) - case_convert)
static int print_float_full(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    int chars_written = 0;
    struct dtoa dtoa;
    uint8_t sign, ndigs, ndigs_exp;
    unsigned char case_convert;
    int exp, n;

    case_convert = TOLOWER(c) - c;
    c = TOLOWER(c);

#ifdef _NEED_IO_LONG_DOUBLE
    if ((flags & (FL_LONG | FL_REPD_TYPE)) == (FL_LONG | FL_REPD_TYPE)) {
        PRINTF_LONG_DOUBLE_TYPE fval = PRINTF_LONG_DOUBLE_ARG(*ap_ptr);
        ndigs = 0;
#ifdef _NEED_IO_C99_FORMATS
        if (c == 'a') {
            c = 'p'; flags |= FL_FLTEXP | FL_FLTHEX;
            if (!(flags & FL_PREC)) prec = -1;
            prec = __lfloat_x_engine(fval, &dtoa, prec, case_convert);
            ndigs = prec + 1; exp = dtoa.exp; ndigs_exp = 1;
        } else
#endif
        {
            int ndecimal = 0; bool fmode = false;
            if (!(flags & FL_PREC)) prec = 6;
            if (c == 'e') { ndigs = prec + 1; flags |= FL_FLTEXP; }
            else if (c == 'f') { ndigs = LONG_FLOAT_MAX_DIG; ndecimal = prec; flags |= FL_FLTFIX; fmode = true; }
            else { c += 'e' - 'g'; ndigs = prec; if (ndigs < 1) ndigs = 1; }
            if (ndigs > LONG_FLOAT_MAX_DIG) ndigs = LONG_FLOAT_MAX_DIG;
            ndigs = __lfloat_d_engine(fval, &dtoa, ndigs, fmode, ndecimal);
            exp = dtoa.exp; ndigs_exp = 2;
        }
    } else
#endif
    {
        FLOAT_UINT fval = PRINTF_FLOAT_ARG(*ap_ptr);
        ndigs = 0;
#ifdef _NEED_IO_C99_FORMATS
        if (c == 'a') {
            c = 'p'; flags |= FL_FLTEXP | FL_FLTHEX;
            if (!(flags & FL_PREC)) prec = -1;
            ndigs = 1 + __float_x_engine(fval, &dtoa, prec, case_convert);
            if (prec <= ndigs) prec = ndigs - 1;
            exp = dtoa.exp; ndigs_exp = 1;
        } else
#endif
        {
            int ndecimal = 0; bool fmode = false;
            if (!(flags & FL_PREC)) prec = 6;
            if (c == 'e') { ndigs = prec + 1; flags |= FL_FLTEXP; }
            else if (c == 'f') { ndigs = FLOAT_MAX_DIG; ndecimal = prec; flags |= FL_FLTFIX; fmode = true; }
            else { c += 'e' - 'g'; ndigs = prec; if (ndigs < 1) ndigs = 1; }
            if (ndigs > FLOAT_MAX_DIG) ndigs = FLOAT_MAX_DIG;
            ndigs = __float_d_engine(fval, &dtoa, ndigs, fmode, ndecimal);
            exp = dtoa.exp; ndigs_exp = 2;
        }
    }

    if (exp < -9 || 9 < exp) ndigs_exp = 2;
    if (exp < -99 || 99 < exp) ndigs_exp = 3;
#ifdef _NEED_IO_FLOAT64
    if (exp < -999 || 999 < exp) ndigs_exp = 4;
#ifdef _NEED_IO_FLOAT_LARGE
    if (exp < -9999 || 9999 < exp) ndigs_exp = 5;
#endif
#endif

    sign = 0;
    if (dtoa.flags & DTOA_MINUS) sign = '-';
    else if (flags & FL_PLUS)    sign = '+';
    else if (flags & FL_SPACE)   sign = ' ';

    if (dtoa.flags & (DTOA_NAN | DTOA_INF)) {
        ndigs = sign ? 4 : 3;
        if (width > ndigs) {
            width -= ndigs;
            if (!(flags & FL_LPAD)) do { HELPER_PUTC(' '); } while (--width);
        } else width = 0;
        if (sign) HELPER_PUTC(sign);
        const char *pnt = "inf";
        if (dtoa.flags & DTOA_NAN) pnt = "nan";
        while ((c = *pnt++)) HELPER_PUTC(TOCASE(c));
    } else {
        if (!(flags & (FL_FLTEXP | FL_FLTFIX))) {
            if (prec == 0) prec = 1;
            while (ndigs > 0 && dtoa.digits[ndigs - 1] == '0') ndigs--;
            int req_prec = prec;
            if (!(flags & FL_ALT)) prec = ndigs;
            if (-4 <= exp && exp < req_prec) {
                flags |= FL_FLTFIX;
                if (exp < prec) prec = prec - (exp + 1);
                else prec = 0;
            } else prec = prec - 1;
        }

        if (flags & FL_FLTFIX) n = (exp > 0 ? exp + 1 : 1);
        else {
            n = 3;
#ifdef _NEED_IO_C99_FORMATS
            if (flags & FL_FLTHEX) n += 2;
#endif
            n += ndigs_exp;
        }
        if (sign) n += 1;
        if (prec) n += prec + 1;
        else if (flags & FL_ALT) n += 1;

        width = width > n ? width - n : 0;
        if (!(flags & (FL_LPAD | FL_ZFILL))) while (width) { HELPER_PUTC(' '); width--; }
        if (sign) HELPER_PUTC(sign);

#ifdef _NEED_IO_C99_FORMATS
        if ((flags & FL_FLTHEX)) { HELPER_PUTC('0'); HELPER_PUTC(TOCASE('x')); }
#endif

        if (!(flags & FL_LPAD)) while (width) { HELPER_PUTC('0'); width--; }

        if (flags & FL_FLTFIX) {
            char out; n = exp > 0 ? exp : 0;
            do {
                if (n == -1) HELPER_PUTC('.');
                if (0 <= exp - n && exp - n < ndigs) out = dtoa.digits[exp - n];
                else out = '0';
                if (--n < -prec) break;
                HELPER_PUTC(out);
            } while (1);
            HELPER_PUTC(out);
            if ((flags & FL_ALT) && n == -1) HELPER_PUTC('.');
        } else {
            HELPER_PUTC(dtoa.digits[0]);
            if (prec > 0) {
                HELPER_PUTC('.');
                int pos = 1;
                for (pos = 1; pos < 1 + prec; pos++) HELPER_PUTC(pos < ndigs ? dtoa.digits[pos] : '0');
            } else if (flags & FL_ALT) HELPER_PUTC('.');

            HELPER_PUTC(TOCASE(c));
            sign = '+';
            if (exp < 0) { exp = -exp; sign = '-'; }
            HELPER_PUTC(sign);
#ifdef _NEED_IO_FLOAT_LARGE
            if (ndigs_exp > 4) { HELPER_PUTC(exp / 10000 + '0'); exp %= 10000; }
#endif
#ifdef _NEED_IO_FLOAT64
            if (ndigs_exp > 3) { HELPER_PUTC(exp / 1000 + '0'); exp %= 1000; }
#endif
            if (ndigs_exp > 2) { HELPER_PUTC(exp / 100 + '0'); exp %= 100; }
            if (ndigs_exp > 1) { HELPER_PUTC(exp / 10 + '0'); exp %= 10; }
            HELPER_PUTC('0' + exp);
        }
    }
    if (flags & FL_LPAD) while (width > 0) { HELPER_PUTC(' '); width--; }
    return chars_written;
}
#undef TOCASE
#else
static int print_float_fallback(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    int chars_written = 0;
    (void)c; (void)prec;
    SKIP_FLOAT_ARG(flags, *ap_ptr);
    const char *pnt = "*float*";
    size_t size = 7;
    if (!(flags & FL_LPAD)) while ((size_t)width > size) { HELPER_PUTC(' '); width--; }
    for (size_t i = 0; i < size; i++) HELPER_PUTC(pnt[i]);
    if (flags & FL_LPAD) while ((size_t)width > size) { HELPER_PUTC(' '); width--; }
    return chars_written;
}
#endif


/* ======================================================================== */
/* 杂项处理器区 (%n)                                                        */
/* ======================================================================== */

static int print_n_handler(print_state_t *state, unsigned c, uint16_t flags, int width, int prec, va_list *ap_ptr) {
    (void)c; (void)width; (void)prec;
#ifdef VFPRINTF_S
    state->error_msg = "format string contains percent-n";
    return -2;
#else
    if (flags & FL_LONG) {
        if (flags & FL_REPD_TYPE) *va_arg(*ap_ptr, long long *) = state->stream_len;
        else *va_arg(*ap_ptr, long *) = state->stream_len;
    } else if (flags & FL_SHORT) {
        if (flags & FL_REPD_TYPE) *va_arg(*ap_ptr, signed char *) = state->stream_len;
        else *va_arg(*ap_ptr, short *) = state->stream_len;
    } else {
        *va_arg(*ap_ptr, int *) = state->stream_len;
    }
    return 0;
#endif
}


/* ======================================================================== */
/* 核心引擎：宏路由与调度引擎 (Macro Router Engine)                         */
/* ======================================================================== */

#ifdef _NEED_IO_SHRINK
    #define HANDLE_CHAR print_char_shrink
    #define HANDLE_STR  print_str_shrink
    #define HANDLE_INT  print_int_shrink
#else
    #define HANDLE_CHAR print_char_full
    #define HANDLE_STR  print_str_full
    #define HANDLE_INT  print_int_full
#endif

#if IO_VARIANT_IS_FLOAT(PRINTF_VARIANT)
    #define HANDLE_FLOAT print_float_full
#else
    #define HANDLE_FLOAT print_float_fallback
#endif

#define HANDLE_N print_n_handler


/* ======================================================================== */
/* print_core : 主解析状态机，利用 Switch 提供 O(1) 跳转效率                */
/* ======================================================================== */

static int print_core(void *ctx, out_fct_t out_fn, const CHAR *fmt, va_list ap_orig)
{
    unsigned c;
    uint16_t flags;
    print_state_t state = { ctx, out_fn, 0, NULL };

#ifdef _NEED_IO_POS_ARGS
    int argno; my_va_list my_ap; const CHAR *fmt_orig = fmt;
#define ap my_ap.ap
#else
#define ap ap_orig
#endif

#define CONSUME_AND_PRINT_CHAR(ch) \
    do { \
        if (state.out_fn(state.ctx, (ch)) < 0) goto fail; \
        state.stream_len++; \
    } while (0)

#ifdef _NEED_IO_POS_ARGS
    va_copy(ap, ap_orig);
#endif

    for (;;) {
        for (;;) {
            c = *fmt++;
            if (!c) goto ret;
            if (c == '%') {
                c = *fmt++;
                if (c != '%') break;
            }
            CONSUME_AND_PRINT_CHAR(c);
        }

        flags = 0;
        int width = 0;
        int prec = 0;
#ifdef _NEED_IO_POS_ARGS
        argno = 0;
#endif

        do {
            if (flags < FL_WIDTH) {
                switch (c) {
                case '0': flags |= FL_ZFILL; continue;
                case '+': flags |= FL_PLUS;  __fallthrough;
                case ' ': flags |= FL_SPACE; continue;
                case '-': flags |= FL_LPAD;  continue;
                case '#': flags |= FL_ALT;   continue;
                case '\'': continue;
                }
            }

            if (flags < FL_LONG) {
                if (c >= '0' && c <= '9') {
                    if (flags & FL_PREC) prec = 10 * prec + (c - '0');
                    else { width = 10 * width + (c - '0'); flags |= FL_WIDTH; }
                    continue;
                }
                if (c == '*') {
#ifdef _NEED_IO_POS_ARGS
                    if (argno) continue;
#endif
                    if (flags & FL_PREC) prec = va_arg(ap, int);
                    else { width = va_arg(ap, int); flags |= FL_WIDTH; }
                    continue;
                }
                if (c == '.') { flags |= FL_PREC; continue; }
#ifdef _NEED_IO_POS_ARGS
                if (c == '$') {
                    if (argno) {
                        va_end(ap);
                        va_copy(ap, ap_orig);
                        skip_to_arg(fmt_orig, &my_ap, (flags & FL_PREC) ? prec : width);
                        if (flags & FL_PREC) prec = va_arg(ap, int);
                        else width = va_arg(ap, int);
                    } else {
                        argno = width; flags = 0; width = 0; prec = 0;
                    }
                    continue;
                }
#endif
            }
            CHECK_INT_SIZES(c, flags);
            break;
        } while ((c = *fmt++) != 0);

#ifdef _NEED_IO_POS_ARGS
        if (argno) {
            va_end(ap);
            va_copy(ap, ap_orig);
            skip_to_arg(fmt_orig, &my_ap, argno);
        }
#endif

        if (prec < 0) { prec = 0; flags &= ~FL_PREC; }
        if (width < 0) { width = -width; flags |= FL_LPAD; }

        int helper_res = 0;

        switch (c) {
            case 'c':
                helper_res = HANDLE_CHAR(&state, c, flags, width, prec, &ap);
                break;
            case 's':
                helper_res = HANDLE_STR(&state, c, flags, width, prec, &ap);
                break;
            case 'n':
#if defined(__IO_PERCENT_N) || defined(VFPRINTF_S)
                helper_res = HANDLE_N(&state, c, flags, width, prec, &ap);
#endif
                break;
            case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': case 'p': case 'b':
                helper_res = HANDLE_INT(&state, c, flags, width, prec, &ap);
                break;
            case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A':
                helper_res = HANDLE_FLOAT(&state, c, flags, width, prec, &ap);
                break;
            default:
                CONSUME_AND_PRINT_CHAR('%');
                CONSUME_AND_PRINT_CHAR(c);
                continue;
        }

        if (helper_res < 0) {
#ifdef VFPRINTF_S
            if (helper_res == -2) goto handle_error;
#endif
            goto fail;
        }

        state.stream_len += helper_res;
    }

ret:
#ifdef _NEED_IO_POS_ARGS
    va_end(ap);
#endif
#undef ap
    return state.stream_len;

fail:
    return -1;

#ifdef VFPRINTF_S
handle_error:
    if (__cur_handler != NULL) {
        __cur_handler(state.error_msg, NULL, -1);
    }
    return -1;
#endif
}


/* ======================================================================== */
/* 外部包装器: 针对 stdio FILE 流的 vfprintf                                */
/* ======================================================================== */

static int stdio_out_wrapper(void *ctx, unsigned c)
{
    FILE *stream = (FILE *)ctx;
#ifdef WIDE_CHARS
    return (putwc((wchar_t)c, stream) == WEOF) ? -1 : 0;
#else
    int (*put)(char, FILE *) = stream->put;
    return put((char)c, stream);
#endif
}

#ifdef VFPRINTF_S
int vfprintf_s(FILE * __restrict stream, const char * __restrict fmt, va_list ap_orig)
#else
int vfprintf(FILE *stream, const CHAR *fmt, va_list ap_orig)
#endif
{
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

    __flockfile(stream);

    if ((stream->flags & __SWR) == 0)
        __funlock_return(stream, EOF);

    stream_len = print_core((void *)stream, stdio_out_wrapper, fmt, ap_orig);

    if (stream_len < 0) {
        stream->flags |= __SERR;
    }

    __funlock_return(stream, stream_len);

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


/* ======================================================================== */
/* 附加功能：基于 print_core 实现的无锁内存安全 vsnprintf                 */
/* ======================================================================== */

typedef struct {
    char *buf;
    size_t max_len;
    size_t count;
} snprintf_ctx_t;

static int string_out_wrapper(void *ctx_ptr, unsigned c)
{
    snprintf_ctx_t *ctx = (snprintf_ctx_t *)ctx_ptr;
    if (ctx->max_len > 0 && ctx->count < (ctx->max_len - 1)) {
        ctx->buf[ctx->count] = (char)c;
    }
    ctx->count++;
    return 0;
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
    snprintf_ctx_t ctx = { str, size, 0 };
    print_core(&ctx, string_out_wrapper, fmt, ap);

    if (size > 0) {
        if (ctx.count < size) str[ctx.count] = '\0';
        else str[size - 1] = '\0';
    }

    return (int)ctx.count;
}

int snprintf(char *str, size_t size, const char *fmt, ...) {
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = vsnprintf(str, size, fmt, ap);
  va_end(ap);

  return ret;
}

#if !defined(VFPRINTF_S) && !defined(WIDE_CHARS)
#if PRINTF_VARIANT == __IO_DEFAULT
#undef vfprintf
#ifdef __strong_reference
__strong_reference(vfprintf, PRINTF_NAME);
#else
int PRINTF_NAME(FILE *stream, const char *fmt, va_list ap)
{
    return vfprintf(stream, fmt, ap);
}
#endif
#endif
#endif
