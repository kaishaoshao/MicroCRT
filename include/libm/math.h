//===-- include/libm/math.h - MicroCRT Mathematical Functions ---*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares standard mathematical functions, floating-point
/// classification macros, mathematical constants, and numerical algorithms.
///
//===----------------------------------------------------------------------===//

#ifndef _MATH_H
#define _MATH_H

#ifdef __cplusplus
extern "C" {
#endif

//===----------------------------------------------------------------------===//
// 1. Mathematical Constants (POSIX & BSD Extensions)
//===----------------------------------------------------------------------===//

#define M_E         2.71828182845904523536028747135266250   ///< e
#define M_LOG2E     1.44269504088896340735992468100189214   ///< log_2 e
#define M_LOG10E    0.434294481903251827651128918916605082  ///< log_10 e
#define M_LN2       0.693147180559945309417232121458176568  ///< log_e 2
#define M_LN10      2.30258509299404568401799145468436421   ///< log_e 10
#define M_PI        3.14159265358979323846264338327950288   ///< pi
#define M_PI_2      1.57079632679489661923132169163975144   ///< pi/2
#define M_PI_4      0.785398163397448309615660845819875721  ///< pi/4
#define M_1_PI      0.318309886183790671537767526745028724  ///< 1/pi
#define M_2_PI      0.636619772367581343075535053490057448  ///< 2/pi
#define M_2_SQRTPI  1.12837916709551257389615890312154517   ///< 2/sqrt(pi)
#define M_SQRT2     1.41421356237309504880168872420969808   ///< sqrt(2)
#define M_SQRT1_2   0.707106781186547524400844362104849039  ///< 1/sqrt(2)

//===----------------------------------------------------------------------===//
// 2. Special Floating-Point Values and Classification
//===----------------------------------------------------------------------===//

#if defined(__GNUC__) || defined(__clang__)
#define HUGE_VAL    (__builtin_huge_val())
#define HUGE_VALF   (__builtin_huge_valf())
#define HUGE_VALL   (__builtin_huge_vall())
#define INFINITY    (__builtin_inff())
#define NAN         (__builtin_nanf(""))
#else
#define HUGE_VAL    ((double)1e500)
#define HUGE_VALF   ((float)1e50)
#define HUGE_VALL   ((long double)1e5000L)
#define INFINITY    HUGE_VALF
#define NAN         (0.0f / 0.0f)
#endif

/// Floating-point categories according to IEEE 754.
#define FP_NAN          0
#define FP_INFINITE     1
#define FP_ZERO         2
#define FP_SUBNORMAL    3
#define FP_NORMAL       4

#if defined(__GNUC__) || defined(__clang__)
/// Classifies argument into one of the FP_* categories.
#define fpclassify(x) __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL,     \
                                           FP_SUBNORMAL, FP_ZERO, x)
/// Determines whether argument is a finite value.
#define isfinite(x)   __builtin_isfinite(x)
/// Determines whether argument is positive or negative infinity.
#define isinf(x)      __builtin_isinf(x)
/// Determines whether argument is a quiet or signaling NaN.
#define isnan(x)      __builtin_isnan(x)
/// Determines whether argument is normal.
#define isnormal(x)   __builtin_isnormal(x)
/// Determines whether sign of argument is negative.
#define signbit(x)    __builtin_signbit(x)

/// Determines whether x is greater than y.
#define isgreater(x, y)      __builtin_isgreater(x, y)
/// Determines whether x is greater than or equal to y.
#define isgreaterequal(x, y) __builtin_isgreaterequal(x, y)
/// Determines whether x is less than y.
#define isless(x, y)         __builtin_isless(x, y)
/// Determines whether x is less than or equal to y.
#define islessequal(x, y)    __builtin_islessequal(x, y)
/// Determines whether x is less than or greater than y.
#define islessgreater(x, y)  __builtin_islessgreater(x, y)
/// Determines whether x and y are unordered with respect to each other.
#define isunordered(x, y)    __builtin_isunordered(x, y)
#else
#define isfinite(x)   (!isnan(x) && !isinf(x))
#define isnan(x)      ((x) != (x))
#define isinf(x)      (!isnan(x) && isnan((x) - (x)))
#define signbit(x)    ((x) < 0.0)
#endif

//===----------------------------------------------------------------------===//
// 3. Trigonometric Functions
//===----------------------------------------------------------------------===//

/// Computes cosine of x (in radians).
float       cosf(float x);
double      cos(double x);
long double cosl(long double x);

/// Computes sine of x (in radians).
float       sinf(float x);
double      sin(double x);
long double sinl(long double x);

/// Computes tangent of x (in radians).
float       tanf(float x);
double      tan(double x);
long double tanl(long double x);

/// Simultaneous computation of sine and cosine (GNU / DSP extension).
void sincosf(float x, float *s, float *c);
void sincos(double x, double *s, double *c);
void sincosl(long double x, long double *s, long double *c);

//===----------------------------------------------------------------------===//
// 4. Inverse Trigonometric Functions
//===----------------------------------------------------------------------===//

/// Computes principal value of arc cosine of x in radians [0, pi].
float       acosf(float x);
double      acos(double x);
long double acosl(long double x);

/// Computes principal value of arc sine of x in radians [-pi/2, pi/2].
float       asinf(float x);
double      asin(double x);
long double asinl(long double x);

/// Computes principal value of arc tangent of x in radians [-pi/2, pi/2].
float       atanf(float x);
double      atan(double x);
long double atanl(long double x);

/// Computes four-quadrant arc tangent of y / x in radians [-pi, pi].
float       atan2f(float y, float x);
double      atan2(double y, double x);
long double atan2l(long double y, long double x);

//===----------------------------------------------------------------------===//
// 5. Hyperbolic and Inverse Hyperbolic Functions
//===----------------------------------------------------------------------===//

/// Computes hyperbolic cosine of x.
float       coshf(float x);
double      cosh(double x);
long double coshl(long double x);

/// Computes hyperbolic sine of x.
float       sinhf(float x);
double      sinh(double x);
long double sinhl(long double x);

/// Computes hyperbolic tangent of x.
float       tanhf(float x);
double      tanh(double x);
long double tanhl(long double x);

/// Computes inverse hyperbolic cosine of x (x >= 1).
float       acoshf(float x);
double      acosh(double x);
long double acoshl(long double x);

/// Computes inverse hyperbolic sine of x.
float       asinhf(float x);
double      asinh(double x);
long double asinhl(long double x);

/// Computes inverse hyperbolic tangent of x (|x| < 1).
float       atanhf(float x);
double      atanh(double x);
long double atanhl(long double x);

//===----------------------------------------------------------------------===//
// 6. Exponential and Logarithmic Functions
//===----------------------------------------------------------------------===//

/// Computes natural exponential function e^x.
float       expf(float x);
double      exp(double x);
long double expl(long double x);

/// Computes base-2 exponential function 2^x.
float       exp2f(float x);
double      exp2(double x);
long double exp2l(long double x);

/// Computes e^x - 1, accurate even when x is near zero.
float       expm1f(float x);
double      expm1(double x);
long double expm1l(long double x);

/// Computes natural logarithm ln(x).
float       logf(float x);
double      log(double x);
long double logl(long double x);

/// Computes base-10 logarithm log10(x).
float       log10f(float x);
double      log10(double x);
long double log10l(long double x);

/// Computes base-2 logarithm log2(x).
float       log2f(float x);
double      log2(double x);
long double log2l(long double x);

/// Computes ln(1 + x), accurate even when x is near zero.
float       log1pf(float x);
double      log1p(double x);
long double log1pl(long double x);

/// Extracts radix-independent floating-point exponent.
float       logbf(float x);
double      logb(double x);
long double logbl(long double x);

/// Extracts exponent of x as a signed integer.
int         ilogbf(float x);
int         ilogb(double x);
int         ilogbl(long double x);

/// Decomposes x into normalized fraction [0.5, 1) and integer power of 2.
float       frexpf(float x, int *exp);
double      frexp(double x, int *exp);
long double frexpl(long double x, int *exp);

/// Computes x * 2^exp efficiently.
float       ldexpf(float x, int exp);
double      ldexp(double x, int exp);
long double ldexpl(long double x, int exp);

/// Multiplies x by FLT_RADIX^n.
float       scalbnf(float x, int n);
double      scalbn(double x, int n);
long double scalbnl(long double x, int n);

/// Multiplies x by FLT_RADIX^n (long integer exponent).
float       scalblnf(float x, long n);
double      scalbln(double x, long n);
long double scalblnl(long double x, long n);

/// Breaks x into integral and fractional parts, both with the same sign as x.
float       modff(float x, float *iptr);
double      modf(double x, double *iptr);
long double modfl(long double x, long double *iptr);

//===----------------------------------------------------------------------===//
// 7. Power, Root and Absolute Value Functions
//===----------------------------------------------------------------------===//

/// Computes x raised to the power y (x^y).
float       powf(float x, float y);
double      pow(double x, double y);
long double powl(long double x, long double y);

/// Computes square root of x.
float       sqrtf(float x);
double      sqrt(double x);
long double sqrtl(long double x);

/// Computes real cube root of x.
float       cbrtf(float x);
double      cbrt(double x);
long double cbrtl(long double x);

/// Computes hypotenuse sqrt(x^2 + y^2) avoiding unwarranted under/overflow.
float       hypotf(float x, float y);
double      hypot(double x, double y);
long double hypotl(long double x, long double y);

/// Computes absolute value of floating-point number x.
float       fabsf(float x);
double      fabs(double x);
long double fabsl(long double x);

//===----------------------------------------------------------------------===//
// 8. Error and Gamma Functions
//===----------------------------------------------------------------------===//

/// Computes error function of x.
float       erff(float x);
double      erf(double x);
long double erfl(long double x);

/// Computes complementary error function (1.0 - erf(x)).
float       erfcf(float x);
double      erfc(double x);
long double erfcl(long double x);

/// Computes natural logarithm of absolute value of Gamma function.
float       lgammaf(float x);
double      lgamma(double x);
long double lgammal(long double x);

/// Computes true Gamma function of x.
float       tgammaf(float x);
double      tgamma(double x);
long double tgammal(long double x);

/// Compatibility alias for historical typo.
#define tgamf tgammaf

//===----------------------------------------------------------------------===//
// 9. Nearest Integer and Rounding Functions
//===----------------------------------------------------------------------===//

/// Computes smallest integer value not less than x.
float       ceilf(float x);
double      ceil(double x);
long double ceill(long double x);

/// Computes largest integer value not greater than x.
float       floorf(float x);
double      floor(double x);
long double floorl(long double x);

/// Rounds x to nearest integer not larger in absolute value than x.
float       truncf(float x);
double      trunc(double x);
long double truncl(long double x);

/// Rounds x to nearest integer, rounding halfway cases away from zero.
float       roundf(float x);
double      round(double x);
long double roundl(long double x);

/// Rounds x away from zero and casts to long.
long        lroundf(float x);
long        lround(double x);
long        lroundl(long double x);

/// Rounds x away from zero and casts to long long.
long long   llroundf(float x);
long long   llround(double x);
long long   llroundl(long double x);

/// Rounds x to integer using current rounding direction.
float       rintf(float x);
double      rint(double x);
long double rintl(long double x);

/// Rounds x using current rounding mode and converts to long.
long        lrintf(float x);
long        lrint(double x);
long        lrintl(long double x);

/// Rounds x using current rounding mode and converts to long long.
long long   llrintf(float x);
long long   llrint(double x);
long long   llrintl(long double x);

/// Rounds x using current mode without raising inexact floating exception.
float       nearbyintf(float x);
double      nearbyint(double x);
long double nearbyintl(long double x);

//===----------------------------------------------------------------------===//
// 10. Remainder, FMA and Extremum Functions
//===----------------------------------------------------------------------===//

/// Computes floating-point remainder of x / y (truncated division).
float       fmodf(float x, float y);
double      fmod(double x, double y);
long double fmodl(long double x, long double y);

/// Computes IEEE remainder of x / y (rounded division).
float       remainderf(float x, float y);
double      remainder(double x, double y);
long double remainderl(long double x, long double y);

/// Computes remainder and records low-order bits of quotient in quo.
float       remquof(float x, float y, int *quo);
double      remquo(double x, double y, int *quo);
long double remquol(long double x, long double y, int *quo);

/// Computes fused multiply-add (x * y + z) without intermediate rounding.
float       fmaf(float x, float y, float z);
double      fma(double x, double y, double z);
long double fmal(long double x, long double y, long double z);

/// Computes positive difference max(x - y, 0).
float       fdimf(float x, float y);
double      fdim(double x, double y);
long double fdiml(long double x, long double y);

/// Returns larger of two floating-point numbers.
float       fmaxf(float x, float y);
double      fmax(double x, double y);
long double fmaxl(long double x, long double y);

/// Returns smaller of two floating-point numbers.
float       fminf(float x, float y);
double      fmin(double x, double y);
long double fminl(long double x, long double y);

//===----------------------------------------------------------------------===//
// 11. Floating-Point Manipulation and Stepping
//===----------------------------------------------------------------------===//

/// Produces value with magnitude of x and sign of y.
float       copysignf(float x, float y);
double      copysign(double x, double y);
long double copysignl(long double x, long double y);

/// Returns quiet NaN with content specified by tagp.
float       nanf(const char *tagp);
double      nan(const char *tagp);
long double nanl(const char *tagp);

/// Returns next representable floating-point value after x in direction of y.
float       nextafterf(float x, float y);
double      nextafter(double x, double y);
long double nextafterl(long double x, long double y);

/// Returns next representable floating-point value in direction of long double y.
float       nexttowardf(float x, long double y);
double      nexttoward(double x, long double y);
long double nexttowardl(long double x, long double y);

#ifdef __cplusplus
}
#endif

#endif // _MATH_H
