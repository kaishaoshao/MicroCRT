/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_STDINT_H
#define __SEGGER_RTL_STDINT_H

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Signed integer minima, maxima, and bit width
*
*  Description
*    Minimum and maximum values for signed integer types.
*/
#define INT8_MIN              __SEGGER_RTL_INT8_MIN          // Minimum value of int8_t
#define INT8_MAX              __SEGGER_RTL_INT8_MAX          // Maximum value of int8_t
#define INT8_WIDTH            __SEGGER_RTL_INT8_WIDTH        // Bit width of int8_t
#define INT16_MIN             __SEGGER_RTL_INT16_MIN         // Minimum value of int16_t
#define INT16_MAX             __SEGGER_RTL_INT16_MAX         // Maximum value of int16_t
#define INT16_WIDTH           __SEGGER_RTL_INT16_WIDTH       // Bit width of int16_t
#define INT32_MIN             __SEGGER_RTL_INT32_MIN         // Minimum value of int32_t
#define INT32_MAX             __SEGGER_RTL_INT32_MAX         // Maximum value of int32_t
#define INT32_WIDTH           __SEGGER_RTL_INT32_WIDTH       // Bit width of int32_t
#define INT64_MIN             __SEGGER_RTL_INT64_MIN         // Minimum value of int64_t
#define INT64_MAX             __SEGGER_RTL_INT64_MAX         // Maximum value of int64_t
#define INT64_WIDTH           __SEGGER_RTL_INT64_WIDTH       // Bit width of int64_t
//
#if defined(__SEGGER_RTL_HAS_INT128)
  #define INT128_MIN          __SEGGER_RTL_INT128_MIN        // Minimum value of int128_t
  #define INT128_MAX          __SEGGER_RTL_INT128_MAX        // Maximum value of int128_t
  #define INT128_WIDTH        __SEGGER_RTL_INT128_WIDTH      // Bit width of int128_t
#endif

/*********************************************************************
*
*       Unsigned integer minima, maxima, and bit width
*
*  Description
*    Minimum and maximum values for unsigned integer types.
*/
#define UINT8_MAX             __SEGGER_RTL_UINT8_MAX         // Maximum value of uint8_t
#define UINT8_WIDTH           __SEGGER_RTL_UINT8_WIDTH       // Bit width of uint8_t
#define UINT16_MAX            __SEGGER_RTL_UINT16_MAX        // Maximum value of uint16_t
#define UINT16_WIDTH          __SEGGER_RTL_UINT16_WIDTH      // Bit width of uint16_t
#define UINT32_MAX            __SEGGER_RTL_UINT32_MAX        // Maximum value of uint32_t
#define UINT32_WIDTH          __SEGGER_RTL_UINT32_WIDTH      // Bit width of uint32_t
#define UINT64_MAX            __SEGGER_RTL_UINT64_MAX        // Maximum value of uint64_t
#define UINT64_WIDTH          __SEGGER_RTL_UINT64_WIDTH      // Bit width of uint64_t
//
#if defined(__SEGGER_RTL_HAS_INT128)
  #define UINT128_MAX         __SEGGER_RTL_UINT128_MAX    // Maximum value of uint128_t
  #define UINT128_WIDTH       __SEGGER_RTL_UINT128_WIDTH  // Bit width of uint128_t
#endif

/*********************************************************************
*
*       Maximal integer minima, maxima, and bit width
*
*  Description
*    Minimum and maximum values for signed and unsigned
*    maximal-integer types.
*/
#define INTMAX_MIN            __SEGGER_RTL_INTMAX_MIN         // Minimum value of intmax_t
#define INTMAX_MAX            __SEGGER_RTL_INTMAX_MAX         // Maximum value of intmax_t
#define INTMAX_WIDTH          __SEGGER_RTL_INTMAX_WIDTH       // Bit width of intmax_t
#define UINTMAX_MAX           __SEGGER_RTL_UINTMAX_MAX        // Maximum value of uintmax_t
#define UINTMAX_WIDTH         __SEGGER_RTL_UINTMAX_WIDTH      // Bit width of uintmax_t

/*********************************************************************
*
*       Least integer minima, maxima, and bit width
*
*  Description
*    Minimum and maximum values for signed and unsigned
*    least-integer types.
*/
#define INT_LEAST8_MIN        __SEGGER_RTL_INT_LEAST8_MIN              // Minimum value of int_least8_t
#define INT_LEAST8_MAX        __SEGGER_RTL_INT_LEAST8_MAX              // Maximum value of int_least8_t
#define INT_LEAST8_WIDTH      __SEGGER_RTL_INT_LEAST8_WIDTH            // Bit width of int_least8_t
#define INT_LEAST16_MIN       __SEGGER_RTL_INT_LEAST16_MIN             // Minimum value of int_least16_t
#define INT_LEAST16_MAX       __SEGGER_RTL_INT_LEAST16_MAX             // Maximum value of int_least16_t
#define INT_LEAST16_WIDTH     __SEGGER_RTL_INT_LEAST16_WIDTH           // Bit width of int_least16_t
#define INT_LEAST32_MIN       __SEGGER_RTL_INT_LEAST32_MIN             // Minimum value of int_least32_t
#define INT_LEAST32_MAX       __SEGGER_RTL_INT_LEAST32_MAX             // Maximum value of int_least32_t
#define INT_LEAST32_WIDTH     __SEGGER_RTL_INT_LEAST32_WIDTH           // Bit width of int_least32_t
#define INT_LEAST64_MIN       __SEGGER_RTL_INT_LEAST64_MIN             // Minimum value of int_least64_t
#define INT_LEAST64_MAX       __SEGGER_RTL_INT_LEAST64_MAX             // Maximum value of int_least64_t
#define INT_LEAST64_WIDTH     __SEGGER_RTL_INT_LEAST64_WIDTH           // Bit width of int_least64_t
#define UINT_LEAST8_MAX       __SEGGER_RTL_UINT_LEAST8_MAX             // Maximum value of uint_least8_t
#define UINT_LEAST8_WIDTH     __SEGGER_RTL_UINT_LEAST8_WIDTH           // Bit width of uint_least8_t
#define UINT_LEAST16_MAX      __SEGGER_RTL_UINT_LEAST16_MAX            // Maximum value of uint_least16_t
#define UINT_LEAST16_WIDTH    __SEGGER_RTL_UINT_LEAST16_WIDTH          // Bit width of uint_least16_t
#define UINT_LEAST32_MAX      __SEGGER_RTL_UINT_LEAST32_MAX            // Maximum value of uint_least32_t
#define UINT_LEAST32_WIDTH    __SEGGER_RTL_UINT_LEAST32_WIDTH          // Bit width of uint_least32_t
#define UINT_LEAST64_MAX      __SEGGER_RTL_UINT_LEAST64_MAX            // Maximum value of uint_least64_t
#define UINT_LEAST64_WIDTH    __SEGGER_RTL_UINT_LEAST64_WIDTH          // Bit width of uint_least64_t
//
#if defined(__SEGGER_RTL_HAS_INT128)
  #define INT_LEAST128_MIN    __SEGGER_RTL_INT_LEAST128_MIN            // Minimum value of int_least128_t
  #define INT_LEAST128_MAX    __SEGGER_RTL_INT_LEAST128_MAX            // Maximum value of int_least128_t
  #define INT_LEAST128_WIDTH  __SEGGER_RTL_INT_LEAST128_WIDTH          // Bit width of int_least128_t
  #define UINT_LEAST128_MAX   __SEGGER_RTL_UINT_LEAST128_MAX          // Maximum value of int_least128_t
  #define UINT_LEAST128_WIDTH __SEGGER_RTL_UINT_LEAST128_WIDTH        // Bit width of int_least128_t
#endif

/*********************************************************************
*
*       Fast integer minima, maxima, and bit width
*
*  Description
*    Minimum and maximum values for signed and unsigned
*    fast-integer types.
*/
#define INT_FAST8_MIN         __SEGGER_RTL_INT_FAST8_MIN               // Minimum value of int_fast8_t
#define INT_FAST8_MAX         __SEGGER_RTL_INT_FAST8_MAX               // Maximum value of int_fast8_t
#define INT_FAST8_WIDTH       __SEGGER_RTL_INT_FAST8_WIDTH             // Bit width of int_fast8_t
#define INT_FAST16_MIN        __SEGGER_RTL_INT_FAST16_MIN              // Minimum value of int_fast16_t
#define INT_FAST16_MAX        __SEGGER_RTL_INT_FAST16_MAX              // Maximum value of int_fast16_t
#define INT_FAST16_WIDTH      __SEGGER_RTL_INT_FAST16_WIDTH            // Bit width of int_fast16_t
#define INT_FAST32_MIN        __SEGGER_RTL_INT_FAST32_MIN              // Minimum value of int_fast32_t
#define INT_FAST32_MAX        __SEGGER_RTL_INT_FAST32_MAX              // Maximum value of int_fast32_t
#define INT_FAST32_WIDTH      __SEGGER_RTL_INT_FAST32_WIDTH            // Bit width of int_fast32_t
#define INT_FAST64_MIN        __SEGGER_RTL_INT_FAST64_MIN              // Minimum value of int_fast64_t
#define INT_FAST64_MAX        __SEGGER_RTL_INT_FAST64_MAX              // Maximum value of int_fast64_t
#define INT_FAST64_WIDTH      __SEGGER_RTL_INT_FAST64_WIDTH            // Bit width of int_fast64_t
#define UINT_FAST8_MAX        __SEGGER_RTL_UINT_FAST8_MAX              // Maximum value of uint_fast8_t
#define UINT_FAST8_WIDTH      __SEGGER_RTL_UINT_FAST8_WIDTH            // Bit width of uint_fast8_t
#define UINT_FAST16_MAX       __SEGGER_RTL_UINT_FAST16_MAX             // Maximum value of uint_fast16_t
#define UINT_FAST16_WIDTH     __SEGGER_RTL_UINT_FAST16_WIDTH           // Bit width of uint_fast16_t
#define UINT_FAST32_MAX       __SEGGER_RTL_UINT_FAST32_MAX             // Maximum value of uint_fast32_t
#define UINT_FAST32_WIDTH     __SEGGER_RTL_UINT_FAST32_WIDTH           // Bit width of uint_fast32_t
#define UINT_FAST64_MAX       __SEGGER_RTL_UINT_FAST64_MAX             // Maximum value of uint_fast64_t
#define UINT_FAST64_WIDTH     __SEGGER_RTL_UINT_FAST64_WIDTH           // Bit width of uint_fast64_t
//
#if defined(__SEGGER_RTL_HAS_INT128)
  #define INT_FAST128_MIN     __SEGGER_RTL_INT_FAST128_MIN      // Minimum value of int_fast128_t
  #define INT_FAST128_MAX     __SEGGER_RTL_INT_FAST128_MAX      // Maximum value of int_fast128_t
  #define INT_FAST128_WIDTH   __SEGGER_RTL_INT_FAST128_WIDTH    // Bit width of int_fast128_t
  #define UINT_FAST128_MAX    __SEGGER_RTL_UINT_FAST128_MAX     // Maximum value of uint_fast128_t
#endif

/*********************************************************************
*
*       Pointer types minima, maxima, and bit width
*
*  Description
*    Minimum and maximum values for pointer-related types.
*/
#define PTRDIFF_MIN           __SEGGER_RTL_PTRDIFF_MIN        // Minimum value of ptrdiff_t
#define PTRDIFF_MAX           __SEGGER_RTL_PTRDIFF_MAX        // Maximum value of ptrdiff_t
#define PTRDIFF_WIDTH         __SEGGER_RTL_PTRDIFF_WIDTH      // Bit width of ptrdiff_t
#define SIZE_MAX              __SEGGER_RTL_SIZE_MAX           // Maximum value of size_t
#define SIZE_WIDTH            __SEGGER_RTL_SIZE_WIDTH         // Bit width of size_t
#define INTPTR_MIN            __SEGGER_RTL_INTPTR_MIN         // Minimum value of intptr_t
#define INTPTR_MAX            __SEGGER_RTL_INTPTR_MAX         // Maximum value of intptr_t
#define INTPTR_WIDTH          __SEGGER_RTL_INTPTR_WIDTH       // Bit width of intptr_t
#define UINTPTR_MAX           __SEGGER_RTL_UINTPTR_MAX        // Maximum value of uintptr_t
#define UINTPTR_WIDTH         __SEGGER_RTL_UINTPTR_WIDTH      // Bit width of uintptr_t

/*********************************************************************
*`
*       Maximum size for safe functions
*
*  Description
*    Objects larger than RSIZE_MAX characters are considered
*    erroneous by the safe functions.
*/
#define RSIZE_MAX        (SIZE_MAX / 2)

/*********************************************************************
*
*       Wide integer minima, maxima, and bit width
*
*  Description
*    Minimum and maximum values for the wint_t type.
*/
#define WINT_MIN             __SEGGER_RTL_WINT_MIN        // Minimum value of wint_t
#define WINT_MAX             __SEGGER_RTL_WINT_MAX        // Maximum value of wint_t
#define WINT_WIDTH           __SEGGER_RTL_WINT_WIDTH      // Bit width of wint_t

/*********************************************************************
*
*       Signed integer construction macros
*
*  Description
*    Macros that create constants of type intx_t.
*/
#define INT8_C(x)             (x)                      // Create constant of type int8_t
#define INT16_C(x)            (x)                      // Create constant of type int16_t
#define INT32_C(x)            (x)                      // Create constant of type int32_t
#define INT64_C(x)            __SEGGER_RTL_INT64_C(x)  // Create constant of type int64_t
//
#if defined(__SEGGER_RTL_HAS_INT128)
  #define INT128_C(x)         __SEGGER_RTL_INT128_C(x)  // Create constant of type int128_t
#endif

/*********************************************************************
*
*       Unsigned integer construction macros
*
*  Description
*    Macros that create constants of type uintx_t.
*/
#define UINT8_C(x)            __SEGGER_RTL_UINT8_C(x)   // Create constant of type uint8_t
#define UINT16_C(x)           __SEGGER_RTL_UINT16_C(x)  // Create constant of type uint16_t
#define UINT32_C(x)           __SEGGER_RTL_UINT32_C(x)  // Create constant of type uint32_t
#define UINT64_C(x)           __SEGGER_RTL_UINT64_C(x)  // Create constant of type uint64_t
//
#if defined(__SEGGER_RTL_HAS_INT128)
  #define UINT128_C(x)        __SEGGER_RTL_UINT128_C(x)  // Create constant of type uint128_t
#endif

/*********************************************************************
*
*       Maximal integer construction macros
*
*  Description
*    Macros that create constants of type intmax_t and uintmax_t.
*/
#define INTMAX_C(x)           __SEGGER_RTL_INTMAX_C(x)     // Create constant of type intmax_t
#define UINTMAX_C(x)          __SEGGER_RTL_UINTMAX_C(x)    // Create constant of type uintmax_t

/*********************************************************************
*
*       Wide character minima, maxima, width
*
*  Description
*    Minimum and maximum values for the wchar_t type.
*/
#define WCHAR_MIN             __SEGGER_RTL_WCHAR_MIN
#define WCHAR_MAX             __SEGGER_RTL_WCHAR_MAX
#define WCHAR_WIDTH           __SEGGER_RTL_WCHAR_WIDTH

/*********************************************************************
*
*       Signal atomic minima, maxima, width
*
*  Description
*    Minimum and maximum values for the sig_atomic_t type.
*/
#define SIG_ATOMIC_MIN        __SEGGER_RTL_SIG_ATOMIC_MIN
#define SIG_ATOMIC_MAX        __SEGGER_RTL_SIG_ATOMIC_MAX

/*********************************************************************
*
*       Data types
*
**********************************************************************
*/

typedef __SEGGER_RTL_INT8_T             int8_t;
typedef __SEGGER_RTL_UINT8_T            uint8_t;
typedef __SEGGER_RTL_INT16_T            int16_t;
typedef __SEGGER_RTL_UINT16_T           uint16_t;
typedef __SEGGER_RTL_INT32_T            int32_t;
typedef __SEGGER_RTL_UINT32_T           uint32_t;
typedef __SEGGER_RTL_INT64_T            int64_t;
typedef __SEGGER_RTL_UINT64_T           uint64_t;
//
typedef __SEGGER_RTL_INT_LEAST8_T       int_least8_t;
typedef __SEGGER_RTL_INT_LEAST16_T      int_least16_t;
typedef __SEGGER_RTL_INT_LEAST32_T      int_least32_t;
typedef __SEGGER_RTL_INT_LEAST64_T      int_least64_t;
typedef __SEGGER_RTL_UINT_LEAST8_T      uint_least8_t;
typedef __SEGGER_RTL_UINT_LEAST16_T     uint_least16_t;
typedef __SEGGER_RTL_UINT_LEAST32_T     uint_least32_t;
typedef __SEGGER_RTL_UINT_LEAST64_T     uint_least64_t;
//
typedef __SEGGER_RTL_INT_FAST8_T        int_fast8_t;
typedef __SEGGER_RTL_INT_FAST16_T       int_fast16_t;
typedef __SEGGER_RTL_INT_FAST32_T       int_fast32_t;
typedef __SEGGER_RTL_INT_FAST64_T       int_fast64_t;
typedef __SEGGER_RTL_UINT_FAST8_T       uint_fast8_t;
typedef __SEGGER_RTL_UINT_FAST16_T      uint_fast16_t;
typedef __SEGGER_RTL_UINT_FAST32_T      uint_fast32_t;
typedef __SEGGER_RTL_UINT_FAST64_T      uint_fast64_t;
//
typedef __SEGGER_RTL_INTPTR_T           intptr_t;
typedef __SEGGER_RTL_UINTPTR_T          uintptr_t;
//
typedef __SEGGER_RTL_INTMAX_T           intmax_t;
typedef __SEGGER_RTL_UINTMAX_T          uintmax_t;
//
#if defined(__SEGGER_RTL_HAS_INT128)
typedef __SEGGER_RTL_INT128_T           int128_t;
typedef __SEGGER_RTL_UINT128_T          uint128_t; 
typedef __SEGGER_RTL_INT_LEAST128_T     int_least128_t;
typedef __SEGGER_RTL_UINT_LEAST128_T    uint_least128_t;
typedef __SEGGER_RTL_INT_FAST128_T      int_fast128_t;
typedef __SEGGER_RTL_UINT_FAST128_T     uint_fast128_t;
#endif

#endif

/*************************** End of file ****************************/
