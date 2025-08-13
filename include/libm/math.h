#ifndef _MATH_H
#define _MATH_H

// 
float       acosf(float x);
double      acos(double x);
long double acosl(long double x);

//
float       acoshf(float x);
double      acosh(double x);
long double acoshl(long double x);

//
float       asinf(float x);
double      asin(double x);
long double asinl(long double x);

// 
float       asinhf(float x);
double      asinh(double x);
long double asinhl(long double x);

//
float       atanf(float x);
double      atan(double x);
long double atanl(long double x);

//
float       atan2f(float y, float x);
double      atan2(double y, double x);
long double atan2l(long double y, long double x);

//
float       atanhf(float x);
double      atanh(double x);
long double atanhl(long double x);

//
float       cbrtf(float x);
double      cbrt(double x);
long double cbrtl(long double x);

// 
float       ceilf(float x);
double      ceil(double x);
long double ceil(long double x);

//
float       copysignf(float x, float y);
double      copysign(double x, double y);
long double copysignl(long double x, long double y);

//
float       cosf(float x);
double      cos(double x);
long double cosl(long double x);

// 
float       erff(float x);
double      erf(double x);
long double erfl(long double x);

//
float       erfcf(float x);
double      erfc(double x);
long double erfcl(long double x);

// 
float       expf(float x);
double      exp(double x);
long double expl(long double x);

//
float       expm1f(float x);
double      expm1(double x);
long double expm1l(long double x);

//
float       fabsf(float x);
double      fabs(double x);
long double fabsl(long double x);

//
float       fdimf(float x, float y);
double      fdim(double x, double y);
long double fdiml(long double x, long double y);

//
float       floorf(float x);
double      floor(double x);
long double floorl(long double x);

//
float       fmaf(float x, float y, float z);
double      fma(double x, double y, double z);
long double fmal(long double x, long double y, long double z);

//
float       fmaxf(float x, float y);
double      fmax(double x, double y);
long double fmaxl(long double x, long double y);

//
float       fminf(float x, float y);
double      fmin(double x, double y);
long double fminl(long double x, long double y);

//
float       fmodf(float x, float y);
double      fmod(double x, double y);
long double fmodl(long double x, long double y);

//
float       frexpf(float x, int *exp);
double      frexp(double x, int *exp);
long double frexpl(long double x, int *exp);

//
float       hypotf(float x, float y);
double      hypot(double x, double y);
long double hypotl(long double x, long double y);

//
int         ilogbf(float x);
int         ilogb(double x);
int         ilogbl(long double x);

//
float       ldexpf(float x, int exp);
double      ldexp(double x, int exp);
long double ldexpl(long double x, int exp);

// 
float       lgammaf(float x);
double      lgamma(double x);
long double lgammal(long double x);

//
long long   llrintf(float x);
long long   llrint(double x);
long long   llrintl(long double x);

//
long long   llroundf(float x);
long long   llround(double x);
long long   llroundl(long double x);

//
float       logf(float x);
double      log(double x);
long double logl(long double x);

//
float       log10f(float x);
double      log10(double x);
long double log10l(long double x);

//
float       log1pf(float x);
double      log1p(double x);
long double log1pl(long double x);

//
float       log2f(float x);
double      log2(double x);
long double log2l(long double x);

//
float       logbf(float x);
double      logb(double x);
long double logbl(long double x);

//
long        lrintf(float x);
long        lrint(double x);
long        lrintl(long double x);

//
long        lroundf(float x);
long        lround(double x);
long        lroundl(long double x);

//
float       modff(float x, float *iptr);
double      modf(double x, double *iptr);
long double modfl(long double x, long double *iptr);

//
float       nanf(const char *tagp);
double      nan(const char *tagp);
long double nanl(const char *tagp);

//
float       nearbyintf(float x);
double      nearbyint(double x);
long double nearbyintl(long double x);

//
float       nextafterf(float x, float y);
double      nextafter(double x, double y);
long double nextafterl(long double x, long double y);

//
float       nexttowardf(float x, long double y);
double      nexttoward(double x, long double y);
long double nexttowardl(long double x, long double y);

//
float       powf(float x, float y);
double      pow(double x, double y);
long double powl(long double x, long double y);

//
float       remainderf(float x, float y);
double      remainder(double x, double y);
long double remainderl(long double x, long double y);

//
float       remquof(float x, float y, int *quo);
double      remquo(double x, double y, int *quo);
long double remquol(long double x, long double y, int *quo);

//
float       rintf(float x);
double      rint(double x);
long double rintl(long double x);

//
float       roundf(float x);
double      round(double x);
long double roundl(long double x);

//
float       scalblnf(float x, long n);
double      scalbln(double x, long n);
long double scalblnl(long double x, long n);

//
float       scalbnf(float x, int n);
double      scalbn(double x, int n);
long double scalbnl(long double x, int n);

//
float       sinf(float x);
double      sin(double x);
long double sinl(long double x);

//
float       sinhf(float x);
double      sinh(double x);
long double sinhl(long double x);

//
float       sqrtf(float x);
double      sqrt(double x);
long double sqrtl(long double x);

//
float       tanf(float x);
double      tan(double x);
long double tanl(long double x);

//
float       tanhf(float x);
double      tanh(double x);
long double tanhl(long double x);

//
float       tgamf(float x);
double      tgamma(double x);
long double tgamma(long double x);

//
float       truncf(float x);
double      trunc(double x);
long double truncl(long double x);

#endif // _MATH_H
