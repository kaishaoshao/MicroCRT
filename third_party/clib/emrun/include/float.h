/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_FLOAT_H
#define __SEGGER_RTL_FLOAT_H

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
*       Common parameters
*
*  Description
*    Applies to single-precision and double-precision formats.
*/
#define FLT_ROUNDS                1  // Rounding mode of floating-point addition is round to nearest.
#define FLT_RADIX                 2  // Radix of the exponent representation.
#if !defined(__SEGGER_RTL_STDC_VERSION) || (__SEGGER_RTL_STDC_VERSION >= __SEGGER_RTL_STDC_VERSION_C99)
#define FLT_EVAL_METHOD           0  // All operations and constants are evaluated to the range and precision of the type.
#define DECIMAL_DIG              17  // Number of decimal digits that can be rounded to a floating-point number without change to the value.
#endif

/*********************************************************************
*
*       Float parameters
*
*  Description
*    IEEE 32-bit single-precision floating format parameters.
*/
#define FLT_MANT_DIG             24  // Number of base FLT_RADIX digits in the mantissa part of a float.
#define FLT_EPSILON 1.19209290E-07f  // Minimum positive number such that 1.0f + FLT_EPSILON != 1.0f.
#define FLT_DIG                   6  // Number of decimal digits of precision of a float.
#define FLT_MIN_EXP            -125  // Minimum value of base FLT_RADIX in the exponent part of a float.
#define FLT_MIN     __SEGGER_RTL_FLT_MIN    // Minimum value of a float.
#define FLT_MIN_10_EXP          -37  // Minimum value in base 10 of the exponent part of a float.
#define FLT_MAX_EXP            +128  // Maximum value of base FLT_RADIX in the exponent part of a float.
#define FLT_MAX     __SEGGER_RTL_FLT_MAX    // Maximum value of a float.
#define FLT_MAX_10_EXP          +38  // Maximum value in base 10 of the exponent part of a float.

/*********************************************************************
*
*       Double parameters
*
*  Description
*    IEEE 64-bit double-precision floating format parameters.
*/
#define DBL_MANT_DIG                    53  // Number of base DBL_RADIX digits in the mantissa part of a double.
#define DBL_EPSILON 2.2204460492503131E-16  // Minimum positive number such that 1.0 + DBL_EPSILON != 1.0.
#define DBL_DIG                         15  // Number of decimal digits of precision of a double.
#define DBL_MIN_EXP                  -1021  // Minimum value of base DBL_RADIX in the exponent part of a double.
#define DBL_MIN       __SEGGER_RTL_DBL_MIN  // Minimum value of a double.
#define DBL_MIN_10_EXP                -307  // Minimum value in base 10 of the exponent part of a double.
#define DBL_MAX_EXP                  +1024  // Maximum value of base DBL_RADIX in the exponent part of a double.
#define DBL_MAX       __SEGGER_RTL_DBL_MAX  // Maximum value of a double.
#define DBL_MAX_10_EXP                +308  // Maximum value in base 10 of the exponent part of a double.

/*********************************************************************
*
*       Long-double parameters
*
*  Description
*    Choose between 64-bit and 128-bit long double.
*/
#if __SEGGER_RTL_SIZEOF_LDOUBLE == 8
#define LDBL_MANT_DIG                    53  // Number of base LDBL_RADIX digits in the mantissa part of a long double.
#define LDBL_EPSILON 2.2204460492503131E-16  // Minimum positive number such that 1.0 + LDBL_EPSILON != 1.0.
#define LDBL_DIG                         15  // Number of decimal digits of precision of a long double.
#define LDBL_MIN_EXP                  -1021  // Minimum value of base LDBL_RADIX in the exponent part of a long double.
#define LDBL_MIN      __SEGGER_RTL_LDBL_MIN  // Minimum value of a long double.
#define LDBL_MIN_10_EXP                -307  // Minimum value in base 10 of the exponent part of a long double.
#define LDBL_MAX_EXP                  +1024  // Maximum value of base LDBL_RADIX in the exponent part of a long double.
#define LDBL_MAX      __SEGGER_RTL_LDBL_MAX  // Maximum value of a long double.
#define LDBL_MAX_10_EXP                +308  // Maximum value in base 10 of the exponent part of a long double.
#else
#define LDBL_MANT_DIG                                       113   // Number of base LDBL_RADIX digits in the mantissa part of a long double.
#define LDBL_EPSILON  1.92592994438723585305597794258492732e-34L  // Minimum positive number such that 1.0 + LDBL_EPSILON != 1.0.
#define LDBL_DIG                                             33   // Number of decimal digits of precision of a long double.
#define LDBL_MIN_EXP                                    (-16381)  // Minimum value of base LDBL_RADIX in the exponent part of a long double.
#define LDBL_MIN                          __SEGGER_RTL_LDBL_MIN   // Minimum value of a long double.
#define LDBL_MIN_10_EXP                                  (-4931)  // Minimum value in base 10 of the exponent part of a long double.
#define LDBL_MAX_EXP                                      16384   // Maximum value of base LDBL_RADIX in the exponent part of a long double.
#define LDBL_MAX                          __SEGGER_RTL_LDBL_MAX   // Maximum value of a long double.
#define LDBL_MAX_10_EXP                                    4932   // Maximum value in base 10 of the exponent part of a long double.
#endif

#endif

/*************************** End of file ****************************/
