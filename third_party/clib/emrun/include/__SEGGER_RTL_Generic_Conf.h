/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_GENERIC_CONF_H
#define __SEGGER_RTL_GENERIC_CONF_H

/*********************************************************************
*
*       Applicability checks
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Configuration of compiler features.
*
**********************************************************************
*/

#if !defined(__SEGGER_RTL_NO_BUILTIN)
  #if defined(__clang__)
    #define __SEGGER_RTL_NO_BUILTIN
  #elif defined(__GNUC__)
    #define __SEGGER_RTL_NO_BUILTIN             __attribute__((__optimize__("-fno-tree-loop-distribute-patterns")))
  #endif
#endif

#if !defined(__SEGGER_RTL_NO_RETURN)
  #if __STDC_VERSION__ >= 201112L
    #define __SEGGER_RTL_NO_RETURN              _Noreturn
  #elif defined(__clang__)
    #define __SEGGER_RTL_NO_RETURN              __attribute__((__noreturn__))
  #elif defined(__GNUC__)
    #define __SEGGER_RTL_NO_RETURN              __attribute__((__noreturn__))
  #endif
#endif

#if defined(__GNUC__) || defined(__clang__)
  #if defined(__has_builtin)
    #if __has_builtin(__builtin_unreachable)
      #define __SEGGER_RTL_UNREACHABLE()        __builtin_unreachable()
    #endif
  #endif
#endif

//
// Configuration of byte order, default to little endian.
//
#define __SEGGER_RTL_BYTE_ORDER                 (-1)

//
// Configuration of maximal type alignment
//
#define __SEGGER_RTL_MAX_ALIGN                  16

//
// Configuration of floating-point ABI.
//
#define __SEGGER_RTL_FP_ABI                     0

//
// Configuration of floating-point hardware.
//
#define __SEGGER_RTL_FP_HW                      0

//
// Configuration of long double size
//
#ifndef   __SEGGER_RTL_SIZEOF_LDOUBLE
  #define __SEGGER_RTL_SIZEOF_LDOUBLE           16
#endif

//
// Configuration of full or compact NaNs.
//
#define __SEGGER_RTL_NAN_FORMAT                 __SEGGER_RTL_NAN_FORMAT_IEEE

//
// Configuration of floating constant selection.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_FLT_SELECT(HEX, DEC)     HEX
#endif

//
// Configuration of thread-local storage
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_THREAD     __thread
#endif

//
// Configuration of inlining.
//
#if defined(__GNUC__) || defined(__clang__)
  #if defined(__clang__)
    #define __SEGGER_RTL_NEVER_INLINE           __attribute__((__noinline__))  // clang does not support noclone.
  #else
    #define __SEGGER_RTL_NEVER_INLINE           __attribute__((__noinline__, __noclone__))
  #endif
  //
  #define __SEGGER_RTL_ALWAYS_INLINE            __inline__ __attribute__((__always_inline__))
  #define __SEGGER_RTL_REQUEST_INLINE           __inline__
#endif

//
// Configuration of static branch probability.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_UNLIKELY(X)              __builtin_expect((X), 0)
  #define __SEGGER_RTL_LIKELY(X)                __builtin_expect((X), 1)
#endif

//
// GCC and clang provide a built-in support for _Complex.
//
#if defined(__GNUC__) || defined(__clang__)
  #ifndef   __SEGGER_RTL_FLOAT32_C_COMPLEX
    #define __SEGGER_RTL_FLOAT32_C_COMPLEX      float _Complex
  #endif
  #ifndef   __SEGGER_RTL_FLOAT64_C_COMPLEX
    #define __SEGGER_RTL_FLOAT64_C_COMPLEX      double _Complex
  #endif
  #ifndef   __SEGGER_RTL_LDOUBLE_C_COMPLEX
    #define __SEGGER_RTL_LDOUBLE_C_COMPLEX      long double _Complex
  #endif
#endif

//
// Configuration of public APIs.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_PUBLIC_API               __attribute__((__weak__))
#endif

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
