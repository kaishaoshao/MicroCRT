/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_XC16_CONF_H
#define __SEGGER_RTL_XC16_CONF_H

/*********************************************************************
*
*       Applicability checks
*
**********************************************************************
*/

#if !defined(__XC16__)
  #error This configuration file expects __XC16__ to be defined!
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef __SEGGER_RTL_OPTIMIZE
  #define __SEGGER_RTL_OPTIMIZE        -2
#endif

#define __SEGGER_RTL_NEVER_INLINE       __attribute__((__noinline__))
#define __SEGGER_RTL_ALWAYS_INLINE      __inline__ __attribute__((__always_inline__))
#define __SEGGER_RTL_REQUEST_INLINE     __inline__

//
// Configuration of byte order.
//
#define __SEGGER_RTL_BYTE_ORDER       (-1)

//
// Configuration of typeset.
//
#define __SEGGER_RTL_TYPESET          16

//
// Configuration of maximal type alignment
//
#define __SEGGER_RTL_MAX_ALIGN        2

//
// Configuration of floating-point ABI
//
#define __SEGGER_RTL_FP_ABI           0

//
// Configuration of full or Arm AEABI NaNs.
//
#define __SEGGER_RTL_NAN_FORMAT       __SEGGER_RTL_NAN_FORMAT_COMPACT

//
// Configuration of floating constant selection.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_FLT_SELECT(HEX, DEC)  HEX
#endif

//
// Configuration of static branch probability.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_UNLIKELY(X)         __builtin_expect((X), 0)
#endif

//
// Configuration of inlining.
//
#if defined(__GNUC__) || defined(__clang__)
  #ifndef   __SEGGER_RTL_NEVER_INLINE
    //
    // Clang doesn't know noclone...
    //
    #if defined(__clang__)
      #define __SEGGER_RTL_NEVER_INLINE    __attribute__((__noinline__))
    #else
      #define __SEGGER_RTL_NEVER_INLINE    __attribute__((__noinline__, __noclone__))
    #endif
  #endif
  //
  #ifndef   __SEGGER_RTL_ALWAYS_INLINE
    #define __SEGGER_RTL_ALWAYS_INLINE     __inline__ __attribute__((__always_inline__))
  #endif
  //
  #ifndef   __SEGGER_RTL_REQUEST_INLINE
    #define __SEGGER_RTL_REQUEST_INLINE    __inline__
  #endif
  //
#endif

//
// Configuration of public APIs.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_PUBLIC_API          __attribute__((__weak__)) 
#endif

#ifndef __SEGGER_RTL_FAST_CODE_SECTION
  #if defined(__GNUC__) || defined(__clang__)
    #define __SEGGER_RTL_FAST_CODE_SECTION(X)  __attribute__((__section__(".fast." X)))
  #endif
#endif

//
// GCC and clang provide a built-in va_list.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_VA_LIST        __builtin_va_list
#endif

#define __SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_FLOAT32_PSEUDO_COMPLEX
#define __SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_FLOAT64_PSEUDO_COMPLEX
#define __SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_LDOUBLE_PSEUDO_COMPLEX

#define __SEGGER_RTL_FLOAT64_SIGNBIT_XOR(X, Y)  \
  ({ SEGGER_RTL_float64_t u, v;                 \
     u.f = X;                                   \
     v.f = Y;                                   \
     u.u16.hi ^= v.u16.hi & 0x8000;             \
     u.f;                                       \
  })

#define __SEGGER_RTL_FLOAT64_SIGNBIT_COPY(X, Y) \
  ({ SEGGER_RTL_float64_t u, v;                 \
     u.f = X;                                   \
     v.f = Y;                                   \
     u.u16.hi &= 0x7FFF;                        \
     u.u16.hi |= v.u16.hi & 0x8000;             \
     u.f;                                       \
  })

#define __SEGGER_RTL_FLOAT64_ABS(X)            \
  ({ SEGGER_RTL_float64_t u;                   \
     u.f = X;                                  \
     u.u16.hi &= 0x7FFF;                       \
     u.f;                                      \
  })

#endif

/*************************** End of file ****************************/
