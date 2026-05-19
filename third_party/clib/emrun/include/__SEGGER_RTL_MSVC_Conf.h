/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_MSVC_CONF_H
#define __SEGGER_RTL_MSVC_CONF_H

/*********************************************************************
*
*       Applicability checks
*
**********************************************************************
*/

#if !defined(_MSC_VER)
  #error This configuration file expects _MSC_VER to be defined!
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// Don't cloud the issue by trying to link in a hybrid MSVC math library
// and a SEGGER math library.
//
#define __SEGGER_RTL_INCLUDE_SEGGER_API       1
#define __SEGGER_RTL_INCLUDE_C_API            0
#define __SEGGER_RTL_SIDE_BY_SIDE             1

#ifndef __SEGGER_RTL_VA_LIST
  //
  // va_list needs to be the MSVC one...

  typedef char * va_list;
  typedef char * __va_list;
#define __SEGGER_RTL_VA_LIST va_list
  #define __SEGGER_RTL_VA_LIST_DEFINED
  #define __SEGGER_RTL_STDARG_H

  #define _INTSIZEOF(n)           ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
  #define _ADDRESSOF(v)           (&(v))

  #define __crt_va_start_a(ap, v) ((void)(ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v)))
  #define __crt_va_arg(ap, t)     (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
  #define __crt_va_end(ap)        ((void)(ap = (va_list)0))
  #define __crt_va_start(ap, x)   __crt_va_start_a(ap, x)

  #define va_start __crt_va_start
  #define va_arg   __crt_va_arg
  #define va_end   __crt_va_end
  #define va_copy(destination, source) ((destination) = (source))
#endif

//
// All targets other than MSVC include the C API.
//
#ifndef __SEGGER_RTL_INCLUDE_C_API
  #ifdef _MSC_VER
  #else
    #define __SEGGER_RTL_INCLUDE_C_API      2
  #endif
#endif

#define __SEGGER_RTL_INCLUDE_C_API          0

//
// Configuration of byte order.
//
#define __SEGGER_RTL_BYTE_ORDER             (-1)

//
// Configuration of typeset.
//
#define __SEGGER_RTL_TYPESET                32
#define __SEGGER_RTL_WCHAR_T                unsigned short
#define __SEGGER_RTL_SIZEOF_WCHAR_T         2
#define __SEGGER_RTL_WINT_T                 unsigned short

//
// Configuration of maximal type alignment
//
#define __SEGGER_RTL_MAX_ALIGN              16

//
// Configuration of floating-point ABI.
//
#define __SEGGER_RTL_FP_ABI                 0

//
// Configuration of floating-point hardware.
//
#define __SEGGER_RTL_FP_ABI                 0

//
// Configuration of scaled integer mode.
//
#define __SEGGER_RTL_SCALED_INTEGER         0

//
// Configuration of inlining.
//
#define __SEGGER_RTL_NEVER_INLINE         __declspec(noinline)
#define __SEGGER_RTL_ALWAYS_INLINE        __forceinline
#define __SEGGER_RTL_REQUEST_INLINE       __inline

//
// Configuration of CLZ support.
//
#define __SEGGER_RTL_CLZ_U32(X)             __lzcnt(X)

//
// Configuration of special floating values.
//
#define __SEGGER_RTL_INFINITY               ((float)(1e+300 * 1e+300))
#define __SEGGER_RTL_NAN                    ((float)(INFINITY * 0.0f))
#define __SEGGER_RTL_HUGE_VALL              ((long double)INFINITY)
#define __SEGGER_RTL_HUGE_VAL               ((double)INFINITY)
#define __SEGGER_RTL_HUGE_VALF              ((float)INFINITY)

//
// Prototypes for intrinsics referenced.
//
unsigned int __lzcnt(unsigned int);

#endif

/*************************** End of file ****************************/
