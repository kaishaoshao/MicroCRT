/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Int.h"
#include "limits.h"
#include "math.h"
#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "wchar.h"
#include "wctype.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

// Maximum number of leading decimal digits in a double.
#define MAX_FLOAT_DIGITS 16

#define isSHORT(flag)      (sizeof(short) < sizeof(int) && ((flag) & FORMAT_SHORT))
#define isLONG(flag)       (SUPPORT_LONG && sizeof(long) > sizeof(int) && ((flag) & FORMAT_LONG))
#define isLONGLONG(flag)   (SUPPORT_LONG_LONG && sizeof(uint64_t) >= sizeof(int) && ((flag) & FORMAT_LONG_LONG))
#define isLONGDOUBLE(flag) (sizeof(double) != sizeof(long double) && ((flag) & FORMAT_LONG_LONG))

#if __SEGGER_RTL_FORMAT_FLOAT_WIDTH == __WIDTH_DOUBLE
  typedef double xfloat_t;
  #define XFLOAT(X) X
#elif __SEGGER_RTL_FORMAT_FLOAT_WIDTH == __WIDTH_FLOAT
  typedef float xfloat_t;
  #define XFLOAT(X) X##f
#endif

#if __SEGGER_RTL_FORMAT_INT_WIDTH == __WIDTH_LONG_LONG

  // All long longs are 64 bits
  #define SUPPORT_LONG_LONG 1
  #define SUPPORT_LONG      1
  #define uvalue_t          uint64_t
  #define value_t           int64_t

#elif __SEGGER_RTL_FORMAT_INT_WIDTH == __WIDTH_LONG

  // Longs are wither 32 bits or 64 bits for 32-bit targets, depending upon compiler
  #define SUPPORT_LONG      1
  #define SUPPORT_LONG_LONG 0
  #define uvalue_t          unsigned long
  #define value_t           long

#elif __SEGGER_RTL_FORMAT_INT_WIDTH == __WIDTH_INT

  // Ints are either 16 or 32 bits
  #define SUPPORT_LONG      0
  #define SUPPORT_LONG_LONG 0
  #define uvalue_t          unsigned int
  #define value_t           int

#else

#error "Need to define __SEGGER_RTL_FORMAT_INT_WIDTH correctly"

#endif

#if (__STDC_VERSION__ >= 199901L) && !defined(__STDC_NO_VLA__)
  //
  // Derive size of buffer from the I/O layer
  //
  #define BUF_SIZE __SEGGER_RTL_X_file_bufsize(stream)
#else
  //
  // Derive size of buffer from the configuration
  //
  #define BUF_SIZE __SEGGER_RTL_STDOUT_BUFFER_LEN
#endif

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

typedef struct {
  __SEGGER_RTL_prin_t prin;
  FILE              * stream;
} __SEGGER_RTL_prin_stream_t;

typedef struct {
  __SEGGER_RTL_prin_t prin;
  char   * buf;
  size_t   bufcur; // cursor
  size_t   bufcap; // capacity
} __SEGGER_RTL_prin_alloc_t;

/*********************************************************************
*
*       Public const data
*
**********************************************************************
*/
#ifdef WITH_PUBLICS

__SEGGER_RTL_RODATA char __SEGGER_RTL_hex_uc[16] = "0123456789ABCDEF";
__SEGGER_RTL_RODATA char __SEGGER_RTL_hex_lc[16] = "0123456789abcdef";

__SEGGER_RTL_RODATA uint64_t __SEGGER_RTL_ipow10[] = {
  UINT64_C(1),
  UINT64_C(10),
  UINT64_C(100),
  UINT64_C(1000),
  UINT64_C(10000),
  UINT64_C(100000),
  UINT64_C(1000000),
  UINT64_C(10000000),
  UINT64_C(100000000),
  UINT64_C(1000000000),
  UINT64_C(10000000000),
  UINT64_C(100000000000),
  UINT64_C(1000000000000),
  UINT64_C(10000000000000),
  UINT64_C(100000000000000),
  UINT64_C(1000000000000000),
  UINT64_C(10000000000000000),
  UINT64_C(100000000000000000),
  UINT64_C(1000000000000000000),
  UINT64_C(10000000000000000000)
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_stream_write()
*
*  Function description
*    Put character to stream.
*
*  Parameters
*    ctx     - Pointer to user-supplied print context.
*    pData   - Pointer to data to write.
*    DataLen - Number of characters to write.
*
*  Return value
*    <  0 - Error code.
*    >= 0 - Number of characters written.
*/
static int __SEGGER_RTL_stream_write(struct __SEGGER_RTL_prin_tag *ctx, const char *pData, unsigned DataLen) {
  __SEGGER_RTL_prin_stream_t *pBuf;
  //
  pBuf = (__SEGGER_RTL_prin_stream_t *)ctx;
  return fwrite(pData, 1, DataLen, pBuf->stream);
}

/*********************************************************************
*
*       __SEGGER_RTL_stream_write()
*
*  Function description
*    Put character to stream.
*
*  Parameters
*    ctx     - Pointer to user-supplied print context.
*    pData   - Pointer to data to write.
*    DataLen - Number of characters to write.
*
*  Return value
*    <  0 - Error code.
*    >= 0 - Number of characters written.
*/
static int __SEGGER_RTL_alloc_write(struct __SEGGER_RTL_prin_tag *ctx, const char *pData, unsigned DataLen) {
  __SEGGER_RTL_prin_alloc_t * pIod;
  char                      * pNew;
  size_t                      newSize;
  //
  pIod = (__SEGGER_RTL_prin_alloc_t *)ctx;
  //
  // Buffer is allocated and new data will exceed capacity of current buffer?
  //
  if (pIod->bufcur + DataLen > pIod->bufcap) {
    //
    // Determine new capacity using existing capacity (not cursor), which is deliberate.
    //
    newSize = pIod->bufcap + pIod->bufcur + DataLen + 32;
    //
    // Try to reallocate buffer.
    //
    pNew = realloc(pIod->buf, newSize);
    //
    // If reallocation failed, the entire function must fail and without leaking.
    //
    if (pNew == NULL) {
      free(pIod->buf);
    }
    //
    // Update buffer.  The buffer may well be set to NULL if the allocation failed.
    //
    pIod->buf    = pNew;
    pIod->bufcap = newSize;
  }
  //
  // If there is a buffer defined, it is guaranteed to be large enough to hold
  // the new data.
  //
  if (pIod->buf != NULL) {
    (memcpy)(pIod->buf + pIod->bufcur, pData, DataLen);
    pIod->bufcur += DataLen;
  }
  //
  return DataLen;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_putc()
*
*/
void __SEGGER_RTL_putc(__SEGGER_RTL_prin_t *ctx, int ch) {
  char c;
  //
  c = (char)ch;
  //
  if (ctx->buffer.pdata != NULL) {
    if (ctx->charcount < ctx->maxchars) {
      ctx->buffer.pdata[ctx->buffer.index++] = c;
      if (ctx->buffer.index == ctx->buffer.capacity) {
        __SEGGER_RTL_prin_flush(ctx);
      }
    }
  } else if (ctx->string.narrow) {
    if (ctx->charcount+1 == ctx->maxchars) {
      c = 0;
    }
    if (ctx->charcount < ctx->maxchars) {
      ctx->string.narrow[ctx->charcount] = c;
    }
  } else if (ctx->string.wide) {
    if (ctx->charcount+1 == ctx->maxchars) {
      c = 0;
    }
    if (ctx->charcount < ctx->maxchars) {
      ctx->string.wide[ctx->charcount] = c;
    }
  } else if (ctx->output_fn) {
    if (ctx->charcount < ctx->maxchars) {
      ctx->output_fn(ctx, &c, 1);
    }
  }
  ++ctx->charcount;
}

/*********************************************************************
*
*       __SEGGER_RTL_prin_flush()
*
*/
void __SEGGER_RTL_prin_flush(__SEGGER_RTL_prin_t *ctx) {
  if (ctx->buffer.index > 0) {
    if (ctx->output_fn != NULL) {
      ctx->output_fn(ctx, ctx->buffer.pdata, ctx->buffer.index);
    }
    ctx->buffer.index = 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_print_padding()
*
*/
void __SEGGER_RTL_print_padding(__SEGGER_RTL_prin_t *ctx, int ch, int n) {
  while (--n >= 0) {
    __SEGGER_RTL_putc(ctx, ch);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_pre_padding()
*
*/
void __SEGGER_RTL_pre_padding(__SEGGER_RTL_prin_t *ctx, int flags, int width) {
  if ((flags & FORMAT_LEFT_JUSTIFY) == 0) {
    __SEGGER_RTL_print_padding(ctx, flags & FORMAT_ZERO_FILL ? '0' : ' ', width);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_compute_wide_metrics()
*
*/
void __SEGGER_RTL_compute_wide_metrics(const wchar_t *wstr, unsigned max_bytes, int *n_bytes, locale_t loc) {
  mbstate_t state;
  char buf[MB_LEN_MAX];
  //
  // Compute number of wide characters we can fit into max_bytes.
  //
  __SEGGER_RTL_init_mbstate(&state);
  *n_bytes = 0;
  while (*wstr) {
    //
    // Convert each character, one at a time.
    //
    size_t n = wctomb_l(buf, *wstr++, loc);
    if (*n_bytes + n > max_bytes) {
      break;
    }
    *n_bytes += n;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_print_wide_string()
*
*/
void __SEGGER_RTL_print_wide_string(__SEGGER_RTL_prin_t *ctx, const wchar_t *wstr, unsigned max_bytes, locale_t loc) {
  mbstate_t state;
  char      buf[MB_LEN_MAX];
  unsigned  nbytes;
  int       i, n;
  //
  // Compute number of wide characters we can fir into max_bytes.
  //
  __SEGGER_RTL_init_mbstate(&state);
  nbytes = 0;
  while (*wstr) {
    //
    // Convert each character, one at a time.
    //
    n = wctomb_l(buf, *wstr++, loc);
    if (nbytes + n > max_bytes) {
      break;
    }
    for (i = 0; i < n; ++i) {
      __SEGGER_RTL_putc(ctx, buf[i]);
    }
    nbytes += n;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_init_prin()
*
*/
void __SEGGER_RTL_init_prin(__SEGGER_RTL_prin_t *iod) {
  __SEGGER_RTL_init_prin_l(iod, __SEGGER_RTL_current_locale());
}

/*********************************************************************
*
*       __SEGGER_RTL_init_prin_l()
*
*/
void __SEGGER_RTL_init_prin_l(__SEGGER_RTL_prin_t *iod, locale_t loc) {
  (memset)(iod, 0, sizeof(*iod));
  iod->locale = loc;
}
#endif

#if !defined(WITH_PUBLICS) && !defined(WITH_PRINTF)
/*********************************************************************
*
*       __SEGGER_RTL_prin()
*
*/
int __SEGGER_RTL_prin(__SEGGER_RTL_prin_t *ctx, const char *fmt, ARGTYPE args) {
  char            ch;
  int             exp;
  char            buff[(8*sizeof(value_t)+2)/3];    // Number of characters that we must buffer for the largest octal value.
#if __SEGGER_RTL_FORMAT_FLOAT_WIDTH > __WIDTH_NONE
  uint64_t        u;
#endif
#if __SEGGER_RTL_FORMAT_WCHAR
  wchar_t         wc[2];
#endif

  union {
    uvalue_t        u;
    value_t         i;
    const char    * str;
    const wchar_t * wstr;
#if __SEGGER_RTL_FORMAT_FLOAT_WIDTH != __WIDTH_NONE
    xfloat_t        r;
#endif
  } v;
  //
  // No characters output yet.
  //
  ctx->charcount = 0;
  //
  while ((ch = *fmt++) != 0) {
    if (ch != '%') {
      __SEGGER_RTL_putc(ctx, ch);
    } else {
      int      len       = 0;
      int      flags     = 0;
      unsigned prefix    = 0;
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
      int      width     = 0;
      int      precision = 0;
#endif
      //
      // Gather flags.
      //
      for (;;) {
        switch (ch = *fmt++) {
        case ' ':  flags |= FORMAT_SPACE;        continue;
        case '#':  flags |= FORMAT_ALTERNATIVE;  continue;
        case '\'': flags |= FORMAT_TICK;         continue;
        case '^':  flags |= FORMAT_ENGINEERING;  continue;
        case '+':  flags |= FORMAT_SIGNED;       continue;
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
        case '-':  flags |= FORMAT_LEFT_JUSTIFY; continue;
        case '0':  flags |= FORMAT_ZERO_FILL;    continue;
#endif
        }
        break;
      }
      //
      // Find width specification.
      //
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
      if (ch == '*') {
        width = va_arg(args, int);
        if (width < 0) {
          width = -width;
          flags |= FORMAT_LEFT_JUSTIFY;
        }
        ch = *fmt++;
      } else {
        while ('0' <= ch && ch <= '9') {
          width = width*10 + (ch-'0');
          ch = *fmt++;
        }
      }
      if (width < 0) {
        width = 0;
      }
      //
      // Deal with precision specification.
      //
      if (ch == '.') {
        ch = *fmt++;
        if (ch == '*') {
          precision = va_arg(args, int);
          ch = *fmt++;
        } else {
          while ('0' <= ch && ch <= '9') {
            precision = precision*10 + (ch-'0');
            ch = *fmt++;
          }
        }
        if (precision >= 0) {
          flags |= FORMAT_HAVE_PRECISION;
        }
      }
#endif
      if (ch == 't' || ch == 'z') {
        //
        // Deal with 'z' and 't' specifiers which, for the architectures
        // we support, are the same size as a pointer and integer.  'z' is
        // a size_t and 't' is a ptrdiff_t.
        //
        ch = *fmt++;
      } else if (SUPPORT_LONG_LONG && ch == 'j') {
        //
        // Only support 'j' when we have long-long support as intmax_t
        // is long long.
        //
        flags |= FORMAT_LONG_LONG;
        ch = *fmt++;
      } else if ((SUPPORT_LONG || __SEGGER_RTL_FORMAT_WCHAR) && ch == 'l') {
        //
        // Deal with long specifiers.  For wide chars, we need to support
        // %lc and %ls but don't need to support %ld or %li, for instance.
        //
        ch = *fmt++;
        if (SUPPORT_LONG_LONG && ch == 'l') {
          flags |= FORMAT_LONG_LONG;
          ch = *fmt++;
        } else {
          flags |= FORMAT_LONG;
        }
      } else if (__SEGGER_RTL_FORMAT_FLOAT_WIDTH > __WIDTH_FLOAT && ch == 'L') {
        //
        // Uses ISO/IEC 8988:1999(E) semantics where 'L' is long double.
        //
        flags |= FORMAT_LONG_LONG;
        ch = *fmt++;
      } else if (ch == 'h') {
        //
        // Deal with short specifiers.
        //
        if ((ch = *fmt++) == 'h') {
          flags |= FORMAT_CHAR;
          ch = *fmt++;
        } else {
          flags |= FORMAT_SHORT;
        }
      }
      //
      switch (ch) {
        default:
          continue;
          //
        case 0:
          //
          // End of format string and no format character at end.
          // Do what C99 expects and return a -ve value.
          //
          __SEGGER_RTL_prin_flush(ctx);
          return -1;
          //
        case '%':
          __SEGGER_RTL_putc(ctx, ch);
          continue;
          //
        case 'c':
#if __SEGGER_RTL_FORMAT_WCHAR
          if (flags & FORMAT_LONG) {
            //
            // wint_t argument -- it can be considered long.
            //
            wc[0] = va_arg(args, long);
            wc[1] = 0;
            flags &= ~FORMAT_HAVE_PRECISION;
            v.wstr = wc;
            goto sformat;
          } else {
            ch = va_arg(args, int);
          }
#else
          ch = va_arg(args, int);
#endif
          //
          // Default handling of unrecognized characters is to print them.
          //
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
          --width;
          __SEGGER_RTL_pre_padding(ctx, flags, width);
#endif
          __SEGGER_RTL_putc(ctx, ch);
postpad:
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
          if (flags & FORMAT_LEFT_JUSTIFY) {
            __SEGGER_RTL_print_padding(ctx, ' ', width);
          }
#endif
          continue;
          //
        case 'n':
          if (flags & FORMAT_CHAR) {
            ARGTYPE_setCharPtrArg(args, ctx->charcount);
          } else if (SUPPORT_LONG_LONG && isLONGLONG(flags)) {
            ARGTYPE_setLongLongPtrArg(args, ctx->charcount);
          } else if (SUPPORT_LONG && isLONG(flags)) {
            ARGTYPE_setLongPtrArg(args, ctx->charcount);
          } else {
            ARGTYPE_setIntPtrArg(args, ctx->charcount);
          }
          continue;
          //
        case 's':
          //
          // Grab argument.
          //
          v.str = va_arg(args, char *);
          //
#if __SEGGER_RTL_FORMAT_WCHAR
sformat:
          if (v.str && (flags & FORMAT_LONG)) {
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
            //
            // Compute wide metrics.
            //
            if (width == 0 && (flags & FORMAT_HAVE_PRECISION) == 0) {
              //
              // Just %ls, whole string up to trailing zero goes out.
              //
              __SEGGER_RTL_compute_wide_metrics(v.wstr, INT_MAX, &len, ctx->locale);
            } else if (flags & FORMAT_HAVE_PRECISION) {
              //
              // No width specified, but precision is specified.
              //
              __SEGGER_RTL_compute_wide_metrics(v.wstr, precision, &len, ctx->locale);
            } else {
              //
              // Precision is not specified but width is specified.
              //
              __SEGGER_RTL_compute_wide_metrics(v.wstr, width, &len, ctx->locale);
            }
            //
            // Pre and post padding...
            //
            width -= len;
            __SEGGER_RTL_pre_padding(ctx, flags, width);
            __SEGGER_RTL_print_wide_string(ctx, v.wstr, len, ctx->locale);
#else
            __SEGGER_RTL_print_wide_string(ctx, v.wstr, INT_MAX, ctx->locale);
#endif
            goto postpad;
          }
#endif

// Format a narrow string...
fmtstr:
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
          // This is what glibc does, so do it too.
          if (v.str == 0) {
            v.str = "(null)";
          }
          //
          // We do not zero-fill strings.
          //
          flags &= ~FORMAT_ZERO_FILL;
          //
          // Subtle differences when a precision is specified.
          //
          if (flags & FORMAT_HAVE_PRECISION) {
            //
            // Precision specified; string does not necessarily
            // need to be terminated with a null.
            //
            len = strnlen(v.str, precision);
          } else {
            //
            // No precision specified, string is as long as it is...
            //
            len = (strlen)(v.str);
          }
          //
          width -= len;
          __SEGGER_RTL_pre_padding(ctx, flags, width);
          while (len--) {
            __SEGGER_RTL_putc(ctx, *v.str++);
          }
          goto postpad;
#else
          while (*v.str) {
            __SEGGER_RTL_putc(ctx, *v.str++);
          }
          continue;
#endif

        case 'p':
          v.u = (uvalue_t)(uintptr_t)va_arg(args, void *);
          if (flags & FORMAT_ALTERNATIVE) {
            prefix = '#';
          }
          flags |= FORMAT_HAVE_PRECISION;
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
          precision = sizeof(void *) * 2;  // Assumes sizeof() delivers bytes...
#endif
          break;

        case 'X':
          flags |= FORMAT_CAPITALS;
          //FALLTHRU
        case 'x':
          if (flags & FORMAT_ALTERNATIVE) {
            prefix = ch == 'x' ? ('0'*0x100+'x') : ('0'*0x100+'X'); // "0x" : "0X"
          }
          if (__SEGGER_RTL_FORMAT_WIDTH_PRECISION && (flags & FORMAT_HAVE_PRECISION)) {
            flags &= ~FORMAT_ZERO_FILL;
          }
          goto fmtint;
          //
        case 'o':
          if (flags & FORMAT_ALTERNATIVE) {
            prefix = '0';
          }
          if (__SEGGER_RTL_FORMAT_WIDTH_PRECISION && (flags & FORMAT_HAVE_PRECISION)) {
            flags &= ~FORMAT_ZERO_FILL;
          }
          goto fmtint;
          //
        case 'u':
          if (__SEGGER_RTL_FORMAT_WIDTH_PRECISION && (flags & FORMAT_HAVE_PRECISION)) {
            flags &= ~FORMAT_ZERO_FILL;
          }
          goto fmtint;
          //
        case 'i':
        case 'd':
          flags |= FORMAT_INPUT_SIGNED;
fmtint:

          if (flags & FORMAT_INPUT_SIGNED) {
            if (SUPPORT_LONG_LONG && isLONGLONG(flags)) {
              v.i = va_arg(args, long long);
            } else if (SUPPORT_LONG && isLONG(flags)) {
              v.i = va_arg(args, long);
            } else {
              v.i = va_arg(args, int);
            }
            //
            if (isSHORT(flags)) {
              v.i = (short)v.i;
            } else if (flags & FORMAT_CHAR) {
              v.i = (signed char)v.i;
            }
            //
            if (v.i < 0) {
              v.u = 0u - v.u;
              prefix = '-';
            } else {
              if (flags & FORMAT_SIGNED) {
                prefix = '+';
              } else if (flags & FORMAT_SPACE) {
                prefix = ' ';
              }
            }
          } else {
            if (SUPPORT_LONG_LONG && isLONGLONG(flags)) {
              v.u = va_arg(args, unsigned long long);
            } else if (SUPPORT_LONG && isLONG(flags)) {
              v.u = va_arg(args, unsigned long);
            } else {
              v.u = va_arg(args, unsigned);
            }
            if (isSHORT(flags)) {
              v.u = (unsigned short)v.u;
            } else if (flags & FORMAT_CHAR) {
              v.u = (unsigned char)v.u;
            }
          }
          //
          if (__SEGGER_RTL_FORMAT_WIDTH_PRECISION && (flags & FORMAT_HAVE_PRECISION)) {
            flags &= ~FORMAT_ZERO_FILL;
          }
          break;
          //
#if __SEGGER_RTL_FORMAT_FLOAT_WIDTH > __WIDTH_NONE
        case 'E':
          flags |= FORMAT_CAPITALS;
          //FALLTHRU
        case 'e':
          flags |= FORMAT_FLOAT_E;
          goto fmtreal;

        case 'F':
          flags |= FORMAT_CAPITALS;
          //FALLTHRU
        case 'f':
          flags |= FORMAT_FLOAT_F;
          goto fmtreal;

        case 'G':
          flags |= FORMAT_CAPITALS;
          //FALLTHRU
        case 'g':
          flags |= FORMAT_FLOAT_G;
fmtreal:
          //
          // Hate this inline, but for small micros, need to
          // conserve stack space.  Yerch.
          //
          if (isLONGDOUBLE(flags)) {
            v.r = va_arg(args, long double);
          } else {
            v.r = va_arg(args, double);
          }
          //
          // Supply default precision.
          //
          if ((flags & FORMAT_HAVE_PRECISION) == 0) {
            precision = 6;
          }
          //
          // Precision is defined as a minimum of 1 for %g with specified precision=0.
          //
          if (precision == 0 && (flags & FORMAT_FLOAT_G) == FORMAT_FLOAT_G) {
            precision = 1;
          }
          //
          if (isinf(v.r)) {
            if (v.r < 0) {
              v.str = flags & FORMAT_CAPITALS ? "-INF" : "-inf";
            } else {
              v.str = flags & FORMAT_CAPITALS ? "+INF" : "+inf";
              if ((flags & FORMAT_SIGNED) == 0) {
                ++v.str;
              }
            }
            flags &= ~FORMAT_HAVE_PRECISION;
            goto fmtstr;
            //
          } else if (isnan(v.r)) {
            v.str = flags & FORMAT_CAPITALS ? "NAN" : "nan";
            flags &= ~FORMAT_HAVE_PRECISION;
            goto fmtstr;
          } else if (!isnormal(v.r)) {
            v.r = 0;
          }
          //
          if (signbit(v.r)) {
            flags |= FORMAT_NEGATIVE;
            v.r = -v.r;
          }
          //
          // Get base-2 exponent.
          //
          XFLOAT(frexp)(v.r, &exp);
          //
          // Convert to base-10 exponent.  log10(2) is approximately
          // 77/256, but that means we need to use 32-bit arithmetic
          // to compute 0x3ff*77 without overflow, so use the cruder
          // value 3/10.
          //
          exp = exp * 3 / 10;
          //
          // Adjust to get true log10(v); correct for overestimation.
          //
          if (v.r) {
            while (v.r > XFLOAT(__SEGGER_RTL_pow10)(exp+1)) {
              ++exp;
            }
            while (v.r < XFLOAT(__SEGGER_RTL_pow10)(exp)) {
              --exp;
            }
          }
          //
          // If our value is out of range or requires exponential format, normalise it.
          // In this case, 1 <= r < 10.
          //
          if ((flags & FORMAT_FLOAT_G) == FORMAT_FLOAT_E ||
              ((flags & FORMAT_FLOAT_G) == FORMAT_FLOAT_G && !(precision > exp && exp >= -4)))
            {
              //
              // 'E' conversion. Can't represent 1e-308 and 1e-38 in respective formats.
              //
#if __SEGGER_RTL_FORMAT_FLOAT_WIDTH == __WIDTH_DOUBLE
              if (exp == 308) {
                v.r /= 1.e308;
              } else {
                v.r *= XFLOAT(__SEGGER_RTL_pow10)(-exp);
              }
#else
              if (exp == 38) {
                v.r /= 1.e38f;
              } else {
                v.r *= XFLOAT(__SEGGER_RTL_pow10)(-exp);
              }
#endif
              if (v.r) {
                if (isinf(v.r)) {
                  if (v.r < 0) {
                    v.str = flags & FORMAT_CAPITALS ? "-INF" : "-inf";
                  } else {
                    v.str = flags & FORMAT_CAPITALS ? "+INF" : "+inf";
                    if ((flags & FORMAT_SIGNED) == 0)
                      ++v.str;
                  }
                  flags &= ~FORMAT_HAVE_PRECISION;
                  goto fmtstr;
                }
                while (v.r >= 10.0) {
                  v.r /= 10.0;  // don't use "r *= 0.1" as 0.1 introduces errors
                  ++exp;
                }
                while (v.r < 1.0) {
                  v.r *= 10.0;
                  --exp;
                }
              }
              //
              // E format.
              //
              if (precision && (flags & FORMAT_FLOAT_G) == FORMAT_FLOAT_G) {
                --precision;
              }
              //
              // Round value.
              //
              v.r += XFLOAT(ldexp)(XFLOAT(__SEGGER_RTL_pow10)(-precision), -1);
              if (v.r >= 10.0) {
                ++exp;
                v.r /= 10.0;
              }
              //
              // G format adjustment; trim trailing zeroes.
              //
              if (precision && (flags & FORMAT_FLOAT_F) && (flags & FORMAT_ALTERNATIVE) == 0) {
                if (precision > MAX_FLOAT_DIGITS) {
                  precision = MAX_FLOAT_DIGITS;
                }
                u = (uint64_t)(v.r * XFLOAT(__SEGGER_RTL_pow10)(precision));
                if (u) {
                  while (precision && (u % 10 == 0)) {
                    --precision;
                    u /= 10;
                  }
                } else {
                  precision = 0;
                }
              }
              flags &= ~FORMAT_FLOAT_F;
              //
              // Figure out number of significant digits.
              //
              width -= (5 + precision);    // 5 chars minimum, dE+ee
              //
              // Decimal point required?
              //
              if (precision || (flags & FORMAT_ALTERNATIVE)) {
                --width;
              }
              //
              // Engineering notation?
              //
              if (flags & FORMAT_ENGINEERING) {
                switch (exp % 3) {
                case          0: len = 1;                                   break;
                case -2: case 1: len = 2; v.r *= 10;  exp -= 1; width -= 1; break;
                case -1: case 2: len = 3; v.r *= 100; exp -= 2; width -= 2; break;
                }
                if (precision < 0) {
                  precision = 0;
                }
              } else {
                len = 1;
              }
              //
              // Leading sign or blank?
              //
              if (flags & FORMAT_NEGATIVE) {
                --width;
              }
              //
              // Three digit exponent?
              //
              if (abs(exp) >= 100) {
                --width;
              }
              u = (uint64_t)v.r;
              v.r -= (xfloat_t)u;
            }
          else {
            //
            // 'F' conversion for '%f' or '%g' format.
            //
            if (flags & FORMAT_FLOAT_E) {
              //
              // 'g' format, precision > 0.
              //
              precision -= exp+1;
              if (precision > MAX_FLOAT_DIGITS) {
                precision = MAX_FLOAT_DIGITS;
              }
              //
              if (exp >= 15) {
                //
                // Don't you admire users' boundless optimism?
                //
                precision = 0;
              } else if ((flags & FORMAT_ALTERNATIVE) == 0) {
                //
                // How many zeros do we have to trim?
                //
                while (precision) {
                  xfloat_t d;
                  //
                  d = XFLOAT(floor)(v.r * XFLOAT(__SEGGER_RTL_pow10)(precision) + 0.5f);
                  if (XFLOAT(fmod)(d, 10) == 0) {
                    --precision;
                  } else {
                    break;
                  }
                }
              }
            }
            //
            // Remove 'G' format specified to go to 'F' format.
            //
            flags &= ~FORMAT_FLOAT_E;
            //
            // Round value to appropriate precision.
            //
            if (exp-MAX_FLOAT_DIGITS >= -precision) {
              v.r += XFLOAT(ldexp)(XFLOAT(__SEGGER_RTL_pow10)(exp-MAX_FLOAT_DIGITS), -1);
            } else {
              v.r += XFLOAT(ldexp)(XFLOAT(__SEGGER_RTL_pow10)(-precision), -1);
            }
            if (v.r >= XFLOAT(__SEGGER_RTL_pow10)(exp+1)) {
              ++exp;
            }
            //
            // Compute width of 'F' conversion.
            //
            if (exp >= 0) {
              if (exp > MAX_FLOAT_DIGITS) {
                exp -= MAX_FLOAT_DIGITS;
                u    = (uint64_t)(v.r * XFLOAT(__SEGGER_RTL_pow10)(-exp));
                v.r  = 0;
              } else {
                exp  = 0;
                u    = (uint64_t)v.r;
                v.r -= (xfloat_t)u;
              }
            } else {
              u   = 0;
              exp = 0;
            }
            //
            // Compute number of leading digits before decimal point.
            //
            len = 1;
            while (u >= __SEGGER_RTL_ipow10[len]) {
              ++len;
            }
            //
            // Reduce width specification by that.
            //
            width -= precision + len + exp;
            //
            // Decimal point required?
            //
            if (precision || (flags & FORMAT_ALTERNATIVE)) {
              --width;
            }
            //
            // Sign or blanker?
            //
            if (flags & FORMAT_NEGATIVE) {
              --width;
            }
          }
          //
          // Must print something...
          //
          if (width < 0) {
            width = 0;
          }
          //
          // Pad left.
          //
          if ((flags & (FORMAT_LEFT_JUSTIFY | FORMAT_ZERO_FILL)) == 0) {
            while (width) {
              --width;
              __SEGGER_RTL_putc(ctx, ' ');
            }
          }
          //
          // Sign.
          //
          if (flags & FORMAT_SIGNED) {
            __SEGGER_RTL_putc(ctx, flags & FORMAT_SPACE ? '-' : '+');
          } else if (flags & FORMAT_SPACE) {
            __SEGGER_RTL_putc(ctx, ' ');
          }
          //
          if ((flags & FORMAT_LEFT_JUSTIFY) == 0) {
            while (width) {
              --width, __SEGGER_RTL_putc(ctx, '0');
            }
          }
          //
          // Print digits before decimal point.
          //
          do {
            --len;
            ch = '0';
            while (u >= __SEGGER_RTL_ipow10[len]) {
              ++ch;
              u -= __SEGGER_RTL_ipow10[len];
            }
            __SEGGER_RTL_putc(ctx, ch);
          } while (len);
          //
          // Additional power-of-ten digits past significance.
          //
          if (flags & FORMAT_FLOAT_F) {
            while (exp > 0) {
              --exp;
              __SEGGER_RTL_putc(ctx, '0');
            }
          }
          //
          if (precision || flags & FORMAT_ALTERNATIVE) {
            __SEGGER_RTL_putc(ctx, '.');
            if (precision > 0) {
              if (precision > MAX_FLOAT_DIGITS) {
                len = MAX_FLOAT_DIGITS;
              } else {
                len = precision;
              }
            }
            //
            // Shift decimal part so that it's an integer.
            //
            precision -= len;
            if (flags & FORMAT_FLOAT_F) {
              u = (uint64_t)(v.r * XFLOAT(__SEGGER_RTL_pow10)(len-exp));
            } else {
              u = (uint64_t)(v.r * XFLOAT(__SEGGER_RTL_pow10)(len));
            }
            //
            // Print decimal digits.
            //
            while (len) {
              --len;
              ch = '0';
              while (u >= __SEGGER_RTL_ipow10[len]) {
                ++ch, u -= __SEGGER_RTL_ipow10[len];
              }
              __SEGGER_RTL_putc(ctx, ch);
            }
            //
            // Print trailing decimal digits past reasonable precision.
            //
            while (precision) {
              --precision, __SEGGER_RTL_putc(ctx, '0');
            }
          }
          //
          if (flags & FORMAT_FLOAT_E) {
            //
            // Deal with exponent...
            //
            __SEGGER_RTL_putc(ctx, flags & FORMAT_CAPITALS ? 'E' : 'e');
            if (exp < 0) {
              __SEGGER_RTL_putc(ctx, '-');
              exp = -exp;
            } else {
              __SEGGER_RTL_putc(ctx, '+');
            }
            //
            if (exp >= 100) {
              __SEGGER_RTL_putc(ctx, (exp / 100 + '0'));
              exp %= 100;
            }
            __SEGGER_RTL_putc(ctx, exp / 10 + '0');
            __SEGGER_RTL_putc(ctx, exp % 10 + '0');
          }
          //
          // Pad right.
          //
          while (width) {
            --width;
            __SEGGER_RTL_putc(ctx, ' ');
          }
          continue;
#endif
      }
      //
      len = 0;
      //
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
      if ((flags & FORMAT_HAVE_PRECISION) == 0) {
        precision = 1;
      }
#endif
      //
      switch (ch) {
      case 'p':
      case 'X':
      case 'x':
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION == 0
        if (v.u == 0) {
          buff[len++] = '0';
        }
#endif
        while (v.u) {
          if (flags & FORMAT_CAPITALS) {
            buff[len] = __SEGGER_RTL_hex_uc[v.u & 0xF];
          } else {
            buff[len] = __SEGGER_RTL_hex_lc[v.u & 0xF];
          }
          ++len;
          v.u >>= 4;
        }
        break;
        //
      case 'o':
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION == 0
        if (v.u == 0) {
          buff[len++] = '0';
        }
#endif
        while (v.u) {
          buff[len] = '0' + ((unsigned)v.u & 7);
          ++len;
          v.u >>= 3;
        }
        break;
        //
      case 'u':
      case 'i':
      case 'd':
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION == 0
        if (v.u == 0) {
          buff[len++] = '0';
        }
#endif
        while (v.u) {
          if (flags & FORMAT_TICK) {
            if ((len & 3) == 3) {
              buff[len] = ',';
              ++len;
            }
          }
          buff[len] = '0' + (unsigned)(v.u % 10u);
          ++len;
          v.u /= 10u;
        }
        break;
      }

#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
      if ((precision -= len) < 0) {
        precision = 0;
      }
      //
      // Compute padding.
      //
      width -= precision;
      width -= len;
      if (prefix >= 0x100) {
        --width;
      }
      if (prefix) {
        --width;
      }
      //
      if ((flags & FORMAT_ZERO_FILL) == 0) {
        __SEGGER_RTL_pre_padding(ctx, flags, width);
        width = 0;
      }
#endif
      if (prefix >= 0x100) {
        __SEGGER_RTL_putc(ctx, prefix >> 8);
      }
      if (prefix) {
        __SEGGER_RTL_putc(ctx, prefix);
      }
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
      __SEGGER_RTL_pre_padding(ctx, flags, width);
      __SEGGER_RTL_print_padding(ctx, '0', precision);
#endif
      while (--len >= 0) {
        __SEGGER_RTL_putc(ctx, buff[len]);
      }
#if __SEGGER_RTL_FORMAT_WIDTH_PRECISION
      if (flags & FORMAT_LEFT_JUSTIFY) {
        __SEGGER_RTL_print_padding(ctx, ' ', width);
      }
#endif
    }
  }
  //
  // Null terminate string if we haven't overflowed.
  //
  if (ctx->string.narrow && ctx->charcount < ctx->maxchars) {
    ctx->string.narrow[ctx->charcount] = 0;
  }
  //
  // Flush anything remaining in our local buffer.
  //
  __SEGGER_RTL_prin_flush(ctx);
  //
  return ctx->charcount;
}
#endif
#ifdef WITH_PUBLICS
/*********************************************************************
*
*       snprintf()
*
*  Function description
*    Formatted write to string, limit length.
*
*  Parameters
*    s      - Pointer to array that receives the formatted output.
*    n      - Maximum number of characters to write to the array pointed to by s.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns the number of characters that would have been written 
*    had n been sufficiently large, not counting the terminating 
*    null character, or a negative value if an encoding error occurred. Thus, the 
*    null-terminated output has been completely written if and only if the returned 
*    value is nonnegative and less than n.
*
*  Additional information
*    Writes to the string pointed to by s under control 
*    of the string pointed to by format that specifies how subsequent arguments 
*    are converted for output.
*    
*    If n is zero, nothing is written, and s can 
*    be a null pointer. Otherwise, output characters beyond count n-1 
*    are discarded rather than being written to the array, and a null character is 
*    written at the end of the characters actually written into the array. A null 
*    character is written at the end of the conversion; it is not counted as part 
*    of the returned value.
*    
*    If there are insufficient arguments for the format, the behavior is undefined. 
*    If the format is exhausted while arguments remain, the excess arguments are 
*    evaluated but are otherwise ignored.
*    
*    If copying takes place between objects that overlap, the behavior is undefined.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API snprintf(char *s, size_t n, const char *format, ...) {
  __SEGGER_RTL_prin_t iod;
  va_list             ap;
  //
  va_start(ap, format);
  __SEGGER_RTL_init_prin(&iod);
  iod.string.narrow = s;
  iod.maxchars      = n;
  return __SEGGER_RTL_prin(&iod, format, ap);
}

/*********************************************************************
*
*       snprintf_l()
*
*  Function description
*    Formatted write to string, limit length, with locale.
*
*  Parameters
*    s      - Pointer to array that receives the formatted output.
*    n      - Maximum number of characters to write to the array pointed to by s.
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns the number of characters that would have been written 
*    had n been sufficiently large, not counting the terminating 
*    null character, or a negative value if an encoding error occurred. Thus, the 
*    null-terminated output has been completely written if and only if the returned 
*    value is nonnegative and less than n.
*
*  Additional information
*    Writes to the string pointed to by s under control 
*    of the string pointed to by format that specifies how subsequent arguments 
*    are converted for output.
*    
*    If n is zero, nothing is written, and s can 
*    be a null pointer. Otherwise, output characters beyond count n-1 
*    are discarded rather than being written to the array, and a null character is 
*    written at the end of the characters actually written into the array. A null 
*    character is written at the end of the conversion; it is not counted as part 
*    of the returned value.
*    
*    If there are insufficient arguments for the format, the behavior is undefined. 
*    If the format is exhausted while arguments remain, the excess arguments are 
*    evaluated but are otherwise ignored.
*    
*    If copying takes place between objects that overlap, the behavior is undefined.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API snprintf_l(char *s, size_t n, locale_t loc, const char *format, ...) {
  __SEGGER_RTL_prin_t iod;
  va_list             ap;
  //
  va_start(ap, format);
  __SEGGER_RTL_init_prin_l(&iod, loc);
  iod.string.narrow = s;
  iod.maxchars      = n;
  return __SEGGER_RTL_prin(&iod, format, ap);
}

/*********************************************************************
*
*       sprintf()
*
*  Function description
*    Formatted write to string.
*
*  Parameters
*    s      - Pointer to array that receives the formatted output.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns number of characters written to s (not counting the
*    terminating null), or a negative value if an output or encoding
*    error occurred.
*
*  Additional information
*    Writes to the string pointed to by s under control of the
*    string pointed to by format that specifies how subsequent
*    arguments are converted for output. A null character is
*    written at the end of the characters written; it is not
*    counted as part of the returned value.
*    
*    If there are insufficient arguments for the format, the
*    behavior is undefined.   If the format is exhausted while
*    arguments remain, the excess arguments are evaluated but are
*    otherwise ignored.
*    
*    If copying takes place between objects that overlap, the
*    behavior is undefined.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API sprintf(char *s, const char *format, ...) {
  __SEGGER_RTL_prin_t iod;
  va_list             ap;
  //
  va_start(ap, format);
  __SEGGER_RTL_init_prin(&iod);
  iod.string.narrow = s;
  iod.maxchars      = INT_MAX;
  return __SEGGER_RTL_prin(&iod, format, ap);
}

/*********************************************************************
*
*       sprintf_l()
*
*  Function description
*    Formatted write to string, with locale.
*
*  Parameters
*    s      - Pointer to array that receives the formatted output.
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns number of characters written to s (not counting the
*    terminating null), or a negative value if an output or encoding
*    error occurred.
*
*  Additional information
*    Writes to the string pointed to by s under control of the
*    string pointed to by format that specifies how subsequent
*    arguments are converted for output. A null character is
*    written at the end of the characters written; it is not
*    counted as part of the returned value.
*    
*    If there are insufficient arguments for the format, the
*    behavior is undefined.   If the format is exhausted while
*    arguments remain, the excess arguments are evaluated but are
*    otherwise ignored.
*    
*    If copying takes place between objects that overlap, the
*    behavior is undefined.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API sprintf_l(char *s, locale_t loc, const char *format, ...) {
  __SEGGER_RTL_prin_t iod;
  va_list             ap;
  //
  va_start(ap, format);
  __SEGGER_RTL_init_prin_l(&iod, loc);
  iod.string.narrow = s;
  iod.maxchars      = INT_MAX;
  return __SEGGER_RTL_prin(&iod, format, ap);
}

/*********************************************************************
*
*       vsprintf()
*
*  Function description
*    Formatted write to string, variadic.
*
*  Parameters
*    s      - Pointer to array that receives the formatted output.
*    format - Pointer to zero-terminated format control string.
*    arg    - Variable parameter list.
*
*  Return value
*    Returns number of characters written to s (not counting the
*    terminating  null), or a negative value if an output or encoding
*    error occurred.
*
*  Additional information
*    Writes to the string pointed to by s under control of the string
*    pointed to by format that specifies how subsequent arguments are
*    converted for output.  A null character is written at the end of
*    the characters written; it is not counted as part of the returned
*    value.
*
*    Before calling vsprintf(), arg must be initialized by the va_start macro
*    (and possibly subsequent va_arg calls). vsprintf() does not invoke the
*    va_end macro.
*    
*    If there are insufficient arguments for the format, the behavior is undefined. 
*    If the format is exhausted while arguments remain, the excess arguments are 
*    evaluated but are otherwise ignored.
*    
*    If copying takes place between objects that overlap, the behavior is undefined.
*
*  Notes
*    This is equivalent to sprintf() with the variable argument list replaced by arg.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API vsprintf(char *s, const char *format, va_list arg) {
  __SEGGER_RTL_prin_t iod;
  //
  __SEGGER_RTL_init_prin(&iod);
  iod.string.narrow = s;
  iod.maxchars      = INT_MAX;
  return __SEGGER_RTL_prin(&iod, format, arg);
}

/*********************************************************************
*
*       vsprintf_l()
*
*  Function description
*    Formatted write to string, variadic, with locale.
*
*  Parameters
*    s      - Pointer to array that receives the formatted output.
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*    arg    - Variable parameter list.
*
*  Return value
*    Returns number of characters written to s (not counting the
*    terminating  null), or a negative value if an output or encoding
*    error occurred.
*
*  Additional information
*    Writes to the string pointed to by s under control of the string
*    pointed to by format that specifies how subsequent arguments are
*    converted for output.  A null character is written at the end of
*    the characters written; it is not counted as part of the returned
*    value.
*
*    Before calling vsprintf(), arg must be initialized by the va_start macro
*    (and possibly subsequent va_arg calls). vsprintf() does not invoke the
*    va_end macro.
*    
*    If there are insufficient arguments for the format, the behavior is undefined. 
*    If the format is exhausted while arguments remain, the excess arguments are 
*    evaluated but are otherwise ignored.
*    
*    If copying takes place between objects that overlap, the behavior is undefined.
*
*  Notes
*    This is equivalent to sprintf() with the variable argument list replaced by arg.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API vsprintf_l(char *s, locale_t loc, const char *format, va_list arg) {
  __SEGGER_RTL_prin_t iod;
  //
  __SEGGER_RTL_init_prin_l(&iod, loc);
  iod.string.narrow = s;
  iod.maxchars      = INT_MAX;
  return __SEGGER_RTL_prin(&iod, format, arg);
}

/*********************************************************************
*
*       vsnprintf()
*
*  Function description
*    Formatted write to string, limit length, variadic.
*
*  Parameters
*    s      - Pointer to array that receives the formatted output.
*    n      - Maximum number of characters to write to the array pointed to by s.
*    format - Pointer to zero-terminated format control string.
*    arg    - Variable parameter list.
*
*  Return value
*    Returns the number of characters that would have been written
*    had n been sufficiently large, not counting the terminating
*    null character, or a negative value if an encoding error occurred.
*    Thus, the null-terminated output has been completely written
*    if and only if the returned value is nonnegative and less than n.
*
*  Additional information
*    Writes to the string pointed to by s under control
*    of the string pointed to by format that specifies how subsequent arguments
*    are converted for output. Before calling vsnprintf(), arg must
*    be initialized by the va_start macro (and possibly subsequent va_arg() calls).
*    vsnprintf() does not invoke the va_end macro.
*    
*    If n is zero, nothing is written, and s can 
*    be a null pointer. Otherwise, output characters beyond count n-1
*    are discarded rather than being written to the array, and a null character is 
*    written at the end of the characters actually written into the array. A null 
*    character is written at the end of the conversion; it is not counted as part 
*    of the returned value.
*    
*    If there are insufficient arguments for the format, the behavior is undefined. 
*    If the format is exhausted while arguments remain, the excess arguments are 
*    evaluated but are otherwise ignored.
*
*    If copying takes place between objects that overlap, the behavior is undefined.
*
*  Notes
*    This is equivalent to snprintf() with the variable argument list replaced
*    by arg.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API vsnprintf(char *s, size_t n, const char *format, va_list arg) {
  __SEGGER_RTL_prin_t iod;
  //
  __SEGGER_RTL_init_prin(&iod);
  iod.string.narrow = s;
  iod.maxchars      = n;
  return __SEGGER_RTL_prin(&iod, format, arg);
}

/*********************************************************************
*
*       vsnprintf_l()
*
*  Function description
*    Formatted write to string, limit length, variadic, with locale.
*
*  Parameters
*    s      - Pointer to array that receives the formatted output.
*    n      - Maximum number of characters to write to the array pointed to by s.
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*    arg    - Variable parameter list.
*
*  Return value
*    Returns the number of characters that would have been written
*    had n been sufficiently large, not counting the terminating
*    null character, or a negative value if an encoding error occurred.
*    Thus, the null-terminated output has been completely written
*    if and only if the returned value is nonnegative and less than n.
*
*  Additional information
*    Writes to the string pointed to by s under control
*    of the string pointed to by format that specifies how subsequent arguments
*    are converted for output. Before calling vsnprintf(), arg must
*    be initialized by the va_start macro (and possibly subsequent va_arg() calls).
*    vsnprintf() does not invoke the va_end macro.
*    
*    If n is zero, nothing is written, and s can 
*    be a null pointer. Otherwise, output characters beyond count n-1
*    are discarded rather than being written to the array, and a null character is 
*    written at the end of the characters actually written into the array. A null 
*    character is written at the end of the conversion; it is not counted as part 
*    of the returned value.
*    
*    If there are insufficient arguments for the format, the behavior is undefined. 
*    If the format is exhausted while arguments remain, the excess arguments are 
*    evaluated but are otherwise ignored.
*
*    If copying takes place between objects that overlap, the behavior is undefined.
*
*  Notes
*    This is equivalent to snprintf() with the variable argument list replaced
*    by arg.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API vsnprintf_l(char *s, size_t n, locale_t loc, const char *format, va_list arg) {
  __SEGGER_RTL_prin_t iod;
  //
  __SEGGER_RTL_init_prin_l(&iod, loc);
  iod.string.narrow = s;
  iod.maxchars      = n;
  return __SEGGER_RTL_prin(&iod, format, arg);
}

/*********************************************************************
*
*       vfprintf()
*
*  Function description
*    Formatted write to file, variadic.
*
*  Parameters
*    stream - Pointer to file to write to.
*    format - Pointer to zero-terminated format control string.
*    arg    - Variable parameter list.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to the file stream using under control of the string
*    pointed to by format that specifies how subsequent arguments
*    are converted for output. Before calling vfprintf(),
*    arg must be initialized by the va_start macro (and possibly
*    subsequent va_arg calls).  vfprintf() does not invoke the
*    va_end macro.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API vfprintf(FILE *stream, const char *format, va_list arg) {
  return vfprintf_l(stream, __SEGGER_RTL_current_locale(), format, arg);
}

/*********************************************************************
*
*       vfprintf_l()
*
*  Function description
*    Formatted write to file, variadic, with locale.
*
*  Parameters
*    stream - Pointer to file to write to.
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*    arg    - Variable parameter list.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to the file stream using under control of the string
*    pointed to by format that specifies how subsequent arguments
*    are converted for output. Before calling vfprintf(),
*    arg must be initialized by the va_start macro (and possibly
*    subsequent va_arg calls).  vfprintf() does not invoke the
*    va_end macro.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API vfprintf_l(FILE *stream, locale_t loc, const char *format, va_list arg) {
  __SEGGER_RTL_prin_stream_t iod;
  int                        n;
  char                       acBuf[BUF_SIZE];
  //
  __SEGGER_RTL_init_prin_l(&iod.prin, loc);
  iod.prin.maxchars        = INT_MAX;
  iod.prin.output_fn       = __SEGGER_RTL_stream_write;
  iod.prin.buffer.pdata    = &acBuf[0];
  iod.prin.buffer.capacity = sizeof(acBuf);
  iod.stream               = stream;
  //
  n = __SEGGER_RTL_prin(&iod.prin, format, arg);
  //
  return n;
}

/*********************************************************************
*
*       fprintf()
*
*  Function description
*    Formatted write to file.
*
*  Parameters
*    stream - Pointer to file to write to.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to the file stream under control of the string pointed
*    to by format that specifies how subsequent arguments are
*    converted for output.
*    
*    If there are insufficient arguments for the format, the behavior
*    is undefined. If the format is exhausted while arguments remain,
*    the excess arguments are evaluated but are otherwise ignored.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fprintf(FILE *stream, const char *format, ...) {
  va_list ap;
  int     n;
  //
  va_start(ap, format);
  n = vfprintf(stream, format, ap);
  va_end(ap);
  //
  return n;
}

/*********************************************************************
*
*       fprintf_l()
*
*  Function description
*    Formatted write to file, with locale.
*
*  Parameters
*    stream - Pointer to file to write to.
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to the file stream under control of the string pointed
*    to by format that specifies how subsequent arguments are
*    converted for output.
*    
*    If there are insufficient arguments for the format, the behavior
*    is undefined. If the format is exhausted while arguments remain,
*    the excess arguments are evaluated but are otherwise ignored.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fprintf_l(FILE *stream, locale_t loc, const char *format, ...) {
  va_list ap;
  int     n;
  //
  va_start(ap, format);
  n = vfprintf_l(stream, loc, format, ap);
  va_end(ap);
  //
  return n;
}

/*********************************************************************
*
*       vprintf()
*
*  Function description
*    Formatted write to standard output, variadic.
*
*  Parameters
*    format - Pointer to zero-terminated format control string.
*    arg    - Variable parameter list.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to the standard output stream using under control of
*    the string pointed to by format that specifies how subsequent
*    arguments are converted for output. Before calling vprintf(),
*    arg must be initialized by the va_start macro (and possibly
*    subsequent va_arg calls).  vprintf() does not invoke the
*    va_end macro.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API vprintf(const char *format, va_list arg) {
  return vfprintf(stdout, format, arg);
}

/*********************************************************************
*
*       vprintf_l()
*
*  Function description
*    Formatted write to standard output, variadic, with locale.
*
*  Parameters
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*    arg    - Variable parameter list.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to the standard output stream using under control of
*    the string pointed to by format that specifies how subsequent
*    arguments are converted for output. Before calling vprintf(),
*    arg must be initialized by the va_start macro (and possibly
*    subsequent va_arg calls).  vprintf() does not invoke the
*    va_end macro.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API vprintf_l(locale_t loc, const char *format, va_list arg) {
  return vfprintf_l(stdout, loc, format, arg);
}

/*********************************************************************
*
*       printf()
*
*  Function description
*    Formatted write to standard output.
*
*  Parameters
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to the standard output stream under control of the string
*    pointed to by format that specifies how subsequent arguments are
*    converted for output.
*    
*    If there are insufficient arguments for the format, the behavior
*    is undefined. If the format is exhausted while arguments remain,
*    the excess arguments are evaluated but are otherwise ignored.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API printf(const char *format, ...) {
  va_list ap;
  int     n;
  //
  va_start(ap, format);
  n = vfprintf(stdout, format, ap);
  va_end(ap);
  //
  return n;
}

/*********************************************************************
*
*       printf_l()
*
*  Function description
*    Formatted write to standard output, with locale.
*
*  Parameters
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to the standard output stream under control of the string
*    pointed to by format that specifies how subsequent arguments are
*    converted for output.
*    
*    If there are insufficient arguments for the format, the behavior
*    is undefined. If the format is exhausted while arguments remain,
*    the excess arguments are evaluated but are otherwise ignored.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API printf_l(locale_t loc, const char *format, ...) {
  va_list ap;
  int     n;
  //
  va_start(ap, format);
  n = vfprintf_l(stdout, loc, format, ap);
  va_end(ap);
  //
  return n;
}

/*********************************************************************
*
*       vasprintf_l()
*
*  Function description
*    Print to newly allocated string, variadic, with locale.
*
*  Parameters
*    strp   - Pointer to object that receives the pointer to the output string.
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*    ap     - Variadic argument list.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to a newly allocated string, using malloc() and realloc()
*    if necessary, under control of the string pointed to by format that
*    specifies how subsequent arguments are converted for output.
*
*    The pointer to the newly allocated stirng is assigned to the object
*    pointed to by strp.  It is the client's responsibility to free this
*    pointer.
*    
*    If there are insufficient arguments for the format, the behavior
*    is undefined. If the format is exhausted while arguments remain,
*    the excess arguments are evaluated but are otherwise ignored.
*
*  Notes
*    Commonly found in Linux, BSD, and GNU C libraries.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API vasprintf_l(char **strp, locale_t loc, const char *format, va_list ap) {
  __SEGGER_RTL_prin_alloc_t iod;
  char                      acBuf[32];
  int                       n;
  //
  __SEGGER_RTL_init_prin_l(&iod.prin, loc);
  iod.prin.maxchars        = INT_MAX;
  iod.prin.output_fn       = __SEGGER_RTL_alloc_write;
  iod.prin.buffer.pdata    = &acBuf[0];
  iod.prin.buffer.capacity = sizeof(acBuf);
  iod.buf                  = NULL;
  iod.bufcur               = 0;
  iod.bufcap               = 0;
  //
  // Run the formatter.
  //
  n = __SEGGER_RTL_prin(&iod.prin, format, ap);
  __SEGGER_RTL_alloc_write(&iod.prin, "", 1);  // Terminating null
  //
  // If no memory, fail.
  //
  if (iod.buf == NULL) {
    return -1;
  }
  //
  // Return allocated buffer and number of characters written.
  //
  *strp = iod.buf;
  return n;
}

/*********************************************************************
*
*       vasprintf()
*
*  Function description
*    Print to newly allocated string, variadic.
*
*  Parameters
*    strp   - Pointer to object that receives the pointer to the output string.
*    format - Pointer to zero-terminated format control string.
*    ap     - Variadic argument list.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to a newly allocated string, using malloc() and realloc()
*    if necessary, under control of the string pointed to by format that
*    specifies how subsequent arguments are converted for output.
*
*    The pointer to the newly allocated stirng is assigned to the object
*    pointed to by strp.  It is the client's responsibility to free this
*    pointer.
*    
*    If there are insufficient arguments for the format, the behavior
*    is undefined. If the format is exhausted while arguments remain,
*    the excess arguments are evaluated but are otherwise ignored.
*
*  Notes
*    Commonly found in Linux, BSD, and GNU C libraries.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API vasprintf(char **strp, const char *format, va_list ap) {
  return vasprintf_l(strp, __SEGGER_RTL_current_locale(), format, ap);
}

/*********************************************************************
*
*       asprintf()
*
*  Function description
*    Print to newly allocated string.
*
*  Parameters
*    strp   - Pointer to object that receives the pointer to the output string.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to a newly allocated string, using malloc() and realloc()
*    if necessary, under control of the string pointed to by format that
*    specifies how subsequent arguments are converted for output.
*
*    The pointer to the newly allocated stirng is assigned to the object
*    pointed to by strp.  It is the client's responsibility to free this
*    pointer.
*    
*    If there are insufficient arguments for the format, the behavior
*    is undefined. If the format is exhausted while arguments remain,
*    the excess arguments are evaluated but are otherwise ignored.
*
*  Notes
*    Commonly found in Linux, BSD, and GNU C libraries.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API asprintf(char **strp, const char *format, ...) {
  va_list ap;
  int     n;
  //
  va_start(ap, format);
  n = vasprintf(strp, format, ap);
  va_end(ap);
  //
  return n;
}

/*********************************************************************
*
*       asprintf_l()
*
*  Function description
*    Print to newly allocated string, with locale.
*
*  Parameters
*    strp   - Pointer to object that receives the pointer to the output string.
*    loc    - Locale to use for conversion.
*    format - Pointer to zero-terminated format control string.
*
*  Return value
*    Returns the number of characters written, or a negative value
*    if an output or encoding error occurred.
*
*  Additional information
*    Writes to a newly allocated string, using malloc() and realloc()
*    if necessary, under control of the string pointed to by format that
*    specifies how subsequent arguments are converted for output.
*
*    The pointer to the newly allocated stirng is assigned to the object
*    pointed to by strp.  It is the client's responsibility to free this
*    pointer.
*    
*    If there are insufficient arguments for the format, the behavior
*    is undefined. If the format is exhausted while arguments remain,
*    the excess arguments are evaluated but are otherwise ignored.
*
*  Notes
*    Commonly found in Linux, BSD, and GNU C libraries.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API asprintf_l(char **strp, locale_t loc, const char *format, ...) {
  va_list ap;
  int     n;
  //
  va_start(ap, format);
  n = vasprintf_l(strp, loc, format, ap);
  va_end(ap);
  //
  return n;
}


#endif
/*************************** End of file ****************************/
