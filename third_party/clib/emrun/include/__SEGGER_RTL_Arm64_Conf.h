/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_ARM64_CONF_H
#define __SEGGER_RTL_ARM64_CONF_H

/*********************************************************************
*
*       Applicability checks
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif
 
 #define __SEGGER_RTL_TYPESET                  64
  #define __SEGGER_RTL_BYTE_ORDER               (-1)
  #define __SEGGER_RTL_MAX_ALIGN                8
  #define __SEGGER_RTL_FP_HW                    2
  #define __SEGGER_RTL_FP_ABI                   0
  #define __SEGGER_RTL_NO_BUILTIN
  #define __SEGGER_RTL_WORD                     unsigned long
  #define __SEGGER_RTL_VA_LIST                  __builtin_va_list  
  #ifndef   __SEGGER_RTL_FLOAT32_C_COMPLEX
    #define __SEGGER_RTL_FLOAT32_C_COMPLEX      float _Complex
  #endif
  #ifndef   __SEGGER_RTL_FLOAT64_C_COMPLEX
    #define __SEGGER_RTL_FLOAT64_C_COMPLEX      double _Complex
  #endif
  #ifndef   __SEGGER_RTL_LDOUBLE_C_COMPLEX
    #define __SEGGER_RTL_LDOUBLE_C_COMPLEX      long double _Complex
  #endif
  #define __SEGGER_RTL_WORD                       unsigned long
#define __SEGGER_RTL_ALIGN_PTR(X)               (void *)((__SEGGER_RTL_WORD)(X) & ~7uL)
#define __SEGGER_RTL_ALIGN_REM(X)               ((__SEGGER_RTL_WORD)(X) & 7u)
#define __SEGGER_RTL_ZBYTE_CHECK(X)             __SEGGER_RTL_ZBYTE_CHECK_func(X)
#define __SEGGER_RTL_ZBYTE_INDEX(X)             __SEGGER_RTL_ZBYTE_INDEX_func(X)
#define __SEGGER_RTL_DIFF_INDEX(X, Y)           __SEGGER_RTL_DIFF_INDEX_func(X, Y)
#define __SEGGER_RTL_DIFF_BYTE(X, N)            __SEGGER_RTL_DIFF_BYTE_func(X, N)
#define __SEGGER_RTL_BYTE_PATTERN(X)            __SEGGER_RTL_BYTE_PATTERN_func(X)
#define __SEGGER_RTL_FILL_HEAD(A, W, C)         __SEGGER_RTL_FILL_HEAD_func(A, W, C)
#define __SEGGER_RTL_FILL_TAIL(N, W, C)         __SEGGER_RTL_FILL_TAIL_func(N, W, C)
#define __SEGGER_RTL_RD_WORD(A)                 __SEGGER_RTL_RD_WORD_func(A)
#define __SEGGER_RTL_WR_WORD(A, W)              __SEGGER_RTL_WR_WORD_func(A, W)
#define __SEGGER_RTL_WR_PARTIAL_WORD(A, W, N)   __SEGGER_RTL_WR_PARTIAL_WORD_func(A, W, N)
#define __SEGGER_RTL_ALWAYS_INLINE            __inline__ __attribute__((__always_inline__))

#if !defined(__SEGGER_RTL_EXCLUDE_STATIC_FUNCTIONS)
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_RD_WORD_func(const void *addr) {
  const unsigned char *pAddr = (const unsigned char *)addr;
  //
  return pAddr[0] +
         pAddr[1] * 0x100u +
         pAddr[2] * 0x10000u +
         pAddr[3] * 0x1000000u +
         pAddr[4] * 0x100000000u +
         pAddr[5] * 0x10000000000u +
         pAddr[6] * 0x1000000000000u +
         pAddr[7] * 0x100000000000000u;
}

static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_WR_PARTIAL_WORD_func(char *addr, __SEGGER_RTL_WORD w, int n) {
  addr[0] = w;
  if (n >= 2) {
    addr[1] = w >> 8;
    if (n >= 3) {
      addr[2] = w >> 16;
      if (n >= 4) {
        addr[3] = w >> 24;
        if (n >= 5) {
          addr[4] = w >> 32;
          if (n >= 6) {
            addr[5] = w >> 40;
            if (n >= 7) {
              addr[6] = w >> 48;
              if (n >= 8) {
                addr[7] = w >> 56;
              }
            }
          }
        }
      }
    }
  }
}

static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_WR_WORD_func(char *addr, __SEGGER_RTL_WORD w) {
  addr[0] = w;
  addr[1] = w >> 8;
  addr[2] = w >> 16;
  addr[3] = w >> 24;
  addr[4] = w >> 32;
  addr[5] = w >> 40;
  addr[6] = w >> 48;
  addr[7] = w >> 56;
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_BYTE_PATTERN_func(unsigned x) {
  return x * 0x0101010101010101uL;
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_FILL_HEAD_func(const void *pOrigin, __SEGGER_RTL_WORD Word, unsigned Standin) {
  __SEGGER_RTL_WORD Mask;
  __SEGGER_RTL_WORD Fill;
  //
  Fill   = __SEGGER_RTL_BYTE_PATTERN(Standin);
  Mask   = 0xFFFFFFFFFFFFFFFFuL;
  Mask <<= 8 * ((__SEGGER_RTL_WORD)pOrigin & 7u);
  //
  return (Word & Mask) | (Fill & ~Mask);
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_FILL_TAIL_func(unsigned n, __SEGGER_RTL_WORD Word, unsigned Standin) {
  __SEGGER_RTL_WORD Mask;
  __SEGGER_RTL_WORD Fill;
  //
  if (n >= 8) {
    return Word;
  } else {
    Fill = __SEGGER_RTL_BYTE_PATTERN(Standin);
    Mask = 0xFFFFFFFFFFFFFFFFuL << (8 * n);
    //
    return (Fill & Mask) | (Word & ~Mask);
  }
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_ZBYTE_CHECK_func(__SEGGER_RTL_WORD x) {
  return ((x-0x0101010101010101uL) & ~x & 0x8080808080808080uL);
}

static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_ZBYTE_INDEX_func(__SEGGER_RTL_WORD x) {
  x = __SEGGER_RTL_ZBYTE_CHECK(x);
  if ((x & 0x00000000FFFFFFFF) != 0) {
    if ((x & 0x00000000000000FFuL) != 0) { return 0; }
    if ((x & 0x000000000000FF00uL) != 0) { return 1; }
    if ((x & 0x0000000000FF0000uL) != 0) { return 2; }
    return 3;
  } else {
    if ((x & 0x000000FF00000000uL) != 0) { return 4; }
    if ((x & 0x0000FF0000000000uL) != 0) { return 5; }
    if ((x & 0x00FF000000000000uL) != 0) { return 6; }
    if ((x & 0xFF00000000000000uL) != 0) { return 7; }
    return 8;
  }
}

static __SEGGER_RTL_ALWAYS_INLINE unsigned __SEGGER_RTL_DIFF_INDEX_func(__SEGGER_RTL_WORD x, __SEGGER_RTL_WORD y) {
  //
  if ((x & 0x00000000000000FFuL) != (y & 0x00000000000000FFuL)) { return 0; }
  if ((x & 0x000000000000FF00uL) != (y & 0x000000000000FF00uL)) { return 1; }
  if ((x & 0x0000000000FF0000uL) != (y & 0x0000000000FF0000uL)) { return 2; }
  if ((x & 0x00000000FF000000uL) != (y & 0x00000000FF000000uL)) { return 3; }
  if ((x & 0x000000FF00000000uL) != (y & 0x000000FF00000000uL)) { return 4; }
  if ((x & 0x0000FF0000000000uL) != (y & 0x0000FF0000000000uL)) { return 5; }
  if ((x & 0x00FF000000000000uL) != (y & 0x00FF000000000000uL)) { return 6; }
  if ((x & 0xFF00000000000000uL) != (y & 0xFF00000000000000uL)) { return 7; }
  return 8;
  //
}

static __SEGGER_RTL_ALWAYS_INLINE unsigned __SEGGER_RTL_DIFF_BYTE_func(__SEGGER_RTL_WORD x, int Index) {
  return (x >> (8*Index)) & 0xFF;
}
#endif

#define __SEGGER_RTL_INCLUDE_GNU_API 1

//#define __SEGGER_RTL_FLOAT16 _Float16

#define __SEGGER_RTL_PUBLIC_API               __attribute__((__weak__)) 

#ifdef __cplusplus
}
#endif

#endif
