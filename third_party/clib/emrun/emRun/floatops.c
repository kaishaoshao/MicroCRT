/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Notes:
    For scaled-integer floating-point approximations, the floating value
    x is held as a pair (i, q) with x = i / (2^q).  Hence, normalizing
    by n bits requires shifting i left by n and increasing the scale
    factor by n, i.e. i <<= n, q += n.  NOTE that this representation
    is opposite to the standard IEEE representation where increasing the
    exponent by n multiplies x by 2^n.

References:
    Many published references were used in the construction of this software:

    [C&W]       - Software Manual for the Elementary Functions, Cody, William J & Waite, William, Prentice Hall.
    [BEEBE]     - The Mathematical-Function Computation Handbook, Beebe, Nelson H.F., Springer International Publishing.
    [IA64]      - IA-64 And Elementary Functions, Peter Markstein, Hewlett-Packard Professional Books.
    [APPLE]     - Apple Technical Report No. 95: Computing the Inverse Square Root, Ken Turkowski.
    [TANG]      - An Overview of Floating-Point Support and Math Library on the Intel XScale Architecture, Ping Tak Peter Tang.
    [EXP1]      - Table-driven implementation of the Expm1 function in IEEE floating-point arithmetic, Ping Tak Peter Tang.
    [MARKSTEIN] - Computation of elementary functions on the IBM RISC System/6000 Processor, Peter W. Markstein, IBM Journal of Research and Development.  Computer Arithmetic Volume III, p. 325.
    [TANG2]     - Table-Lookup Algorithms for Elementary Functions and Their Error Analysis, Proceedings of the 10th Symposium on Computer Arithmetic, Vol 15, No 2, June 1989.
    [TANG3]     - Implementing complex elementary functions using exception handling, Tang.
    [PAYNE]     - Radian Reduction for Trigonometric Functions, M. Payne and R. Hanek, Signum, p19-24, Jan 1983.
    [NG]        - Argument reduction for huge arguments, Ng.
    [WARREN]    - Hacker's Delight, Henry Warren.
    [C99]       - ISO/IEC 9899, Programming Languages - C
    [MOELLER]   - Improved division by invariant integers, Niels Moeller & Torbjorn Granlund.

    The complete archive of papers published in the Proceedings of IEEE Symposium
    on Computer Arithmetic is available from http://www.acsel-lab.com/arithmetic/.

    An excellent exposition of floating-point is that provided by the ancient
    Sun Microsystems Numerical Computation Guide at https://docs.oracle.com/cd/E19422-01/819-3693/.

*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_FP.h"
#include "__SEGGER_RTL_FP_Int.h"
#include "__SEGGER_RTL_Inlines.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

//
// Choose how arithmetic and functions are invoked.
//
// * In side-by-size mode, floating-point arithmetic and expressions
//   expand to calls to SEGGER API's C-coded floating-point functions.
//
// * In fast mode, floating-point arithmetic and expressions expand
//   to the regular C-language expressions, and all high-level function
//   calls are expanded inline.
//
// * In regular mode, floating-point arithmetic and expressions expand
//   to the regular C-language expressions, and all high-level function
//   calls expand to the outlined version of the function to reduce size.
//
#if __SEGGER_RTL_SIDE_BY_SIDE > 0
  #define __CHOOSE(F, C, I)  (F)
#elif __SEGGER_RTL_OPTIMIZE >= 3
  #define __CHOOSE(F, C, I)  (I)
#else
  #define __CHOOSE(F, C, I)  (C)
#endif

#define CONCAT3(X, Y, Z) X##Y##Z
#define CONCAT2(X, Y)    X##Y

#if __SEGGER_RTL_SIDE_BY_SIDE > 0
  //
  // In side-by-size mode, internal function calls use public SEGGER API entry points.
  //
  #define ENTRY_F32(X)   CONCAT3(SEGGER_,X,f)
  #define ENTRY_F64(X)   CONCAT2(SEGGER_,X)
  //
#elif __SEGGER_RTL_INCLUDE_C_API
  //
  // If the C API is activated, internal function calls use the standard C API entry points.
  //
  #define ENTRY_F32(X)   CONCAT2(X,f)
  #define ENTRY_F64(X)   X
  //
#else
  //
  // Otherwise we use the internal outlined functions so code size is not bloated.
  //
  #define ENTRY_F32(X)   CONCAT3(__SEGGER_RTL_float32_,X,_outline)
  #define ENTRY_F64(X)   CONCAT3(__SEGGER_RTL_float64_,X,_outline)
  //
#endif

//
// Configuration of internal arithmetic and internal function calls.
//
  
#define SEGGER_ADDF(X, Y)   __CHOOSE(SEGGER_addf((X), (Y)),        (X) + (Y),                                            (X) + (Y)                                    )
#define SEGGER_ADD(X, Y)    __CHOOSE(SEGGER_add ((X), (Y)),        (X) + (Y),                                            (X) + (Y)                                    )
#define SEGGER_SUBF(X, Y)   __CHOOSE(SEGGER_subf((X), (Y)),        (X) - (Y),                                            (X) - (Y)                                    )
#define SEGGER_SUB(X, Y)    __CHOOSE(SEGGER_sub ((X), (Y)),        (X) - (Y),                                            (X) - (Y)                                    )
#define SEGGER_MULF(X, Y)   __CHOOSE(SEGGER_mulf((X), (Y)),        (X) * (Y),                                            (X) * (Y)                                    )
#define SEGGER_MUL(X, Y)    __CHOOSE(SEGGER_mul ((X), (Y)),        (X) * (Y),                                            (X) * (Y)                                    )
#define SEGGER_DIVF(X, Y)   __CHOOSE(SEGGER_divf((X), (Y)),        (X) / (Y),                                            (X) / (Y)                                    )
#define SEGGER_DIV(X, Y)    __CHOOSE(SEGGER_div ((X), (Y)),        (X) / (Y),                                            (X) / (Y)                                    )
//
#define SEGGER_NEGF(X)      __CHOOSE(SEGGER_negf(X),               (-(X)),                                               (-(X))                                       )
#define SEGGER_NEG(X)       __CHOOSE(SEGGER_neg(X),                (-(X)),                                               (-(X))                                       )
#if defined(__SEGGER_RTL_FLOAT32_ABS)
  #define SEGGER_FABSF(X)   __SEGGER_RTL_FLOAT32_ABS(X)
#else
  #define SEGGER_FABSF(X)   __CHOOSE(SEGGER_fabsf(X),              __SEGGER_RTL_float32_abs_inline(X),                   __SEGGER_RTL_float32_abs_inline(X)           )
#endif
#if defined(__SEGGER_RTL_FLOAT64_ABS)
  #define SEGGER_FABS(X)    __SEGGER_RTL_FLOAT64_ABS(X)
#else
  #define SEGGER_FABS(X)    __CHOOSE(SEGGER_fabs(X),               __SEGGER_RTL_float64_abs_inline(X),                   __SEGGER_RTL_float64_abs_inline(X)           )
#endif
#define SEGGER_SINF(X)      __CHOOSE(SEGGER_sinf(X),               ENTRY_F32(sin)(X),                                    __SEGGER_RTL_float32_sin_inline(X)           )
#define SEGGER_SIN(X)       __CHOOSE(SEGGER_sin(X),                ENTRY_F64(sin)(X),                                    __SEGGER_RTL_float64_sin_inline(X)           )
#define SEGGER_COSF(X)      __CHOOSE(SEGGER_cosf(X),               ENTRY_F32(cos)(X),                                    __SEGGER_RTL_float32_cos_inline(X)           )
#define SEGGER_COS(X)       __CHOOSE(SEGGER_cos(X),                ENTRY_F64(cos)(X),                                    __SEGGER_RTL_float64_cos_inline(X)           )
#define SEGGER_TANF(X)      __CHOOSE(SEGGER_tanf(X),               ENTRY_F32(tan)(X),                                    __SEGGER_RTL_float32_tan_inline(X)           )
#define SEGGER_TAN(X)       __CHOOSE(SEGGER_tan(X),                ENTRY_F64(tan)(X),                                    __SEGGER_RTL_float64_tan_inline(X)           )

#define SEGGER_SINHF(X)     __CHOOSE(SEGGER_sinhf(X),              ENTRY_F32(sinh)(X),                                   __SEGGER_RTL_float32_sinh_inline(X)          )
#define SEGGER_SINH(X)      __CHOOSE(SEGGER_sinh(X),               ENTRY_F64(sinh)(X),                                   __SEGGER_RTL_float64_sinh_inline(X)          )
#define SEGGER_COSHF(X)     __CHOOSE(SEGGER_coshf(X),              ENTRY_F32(cosh)(X),                                   __SEGGER_RTL_float32_cosh_inline(X)          )
#define SEGGER_COSH(X)      __CHOOSE(SEGGER_cosh(X),               ENTRY_F64(cosh)(X),                                   __SEGGER_RTL_float64_cosh_inline(X)          )
#define SEGGER_TANH(X)      __CHOOSE(SEGGER_tanh(X),               ENTRY_F64(tanh)(X),                                   __SEGGER_RTL_float64_tanh_inline(X)          )
#define SEGGER_TANHF(X)     __CHOOSE(SEGGER_tanhf(X),              ENTRY_F32(tanh)(X),                                   __SEGGER_RTL_float32_tanh_inline(X)          )

#define SEGGER_SQRTF(X)     __CHOOSE(SEGGER_sqrtf(X),              ENTRY_F32(sqrt)(X),                                   __SEGGER_RTL_float32_sqrt_inline(X)          )
#define SEGGER_SQRT(X)      __CHOOSE(SEGGER_sqrt(X),               ENTRY_F64(sqrt)(X),                                   __SEGGER_RTL_float64_sqrt_inline(X)          )
#define SEGGER_EXPF(X)      __CHOOSE(SEGGER_expf(X),               ENTRY_F32(exp)(X),                                    __SEGGER_RTL_float32_exp_inline(X)           )
#define SEGGER_EXP(X)       __CHOOSE(SEGGER_exp(X),                ENTRY_F64(exp)(X),                                    __SEGGER_RTL_float64_exp_inline(X)           )
#define SEGGER_LOGF(X)      __CHOOSE(SEGGER_logf(X),               ENTRY_F32(log)(X),                                    __SEGGER_RTL_float32_log_inline(X)           )
#define SEGGER_LOG(X)       __CHOOSE(SEGGER_log(X),                ENTRY_F64(log)(X),                                    __SEGGER_RTL_float64_log_inline(X)           )
#define SEGGER_LOG1PF(X)    __CHOOSE(SEGGER_log1pf(X),             ENTRY_F32(log1p)(X),                                  __SEGGER_RTL_float32_log1p_inline(X)         )
#define SEGGER_LOG1P(X)     __CHOOSE(SEGGER_log1p(X),              ENTRY_F64(log1p)(X),                                  __SEGGER_RTL_float64_log1p_inline(X)         )
#define SEGGER_ILOGBF(X)    __CHOOSE(SEGGER_ilogbf(X),             ENTRY_F32(ilogb)(X),                                  __SEGGER_RTL_float32_ilogb_inline(X)         )
#define SEGGER_ILOGB(X)     __CHOOSE(SEGGER_ilogb(X),              ENTRY_F64(ilogb)(X),                                  __SEGGER_RTL_float64_ilogb_inline(X)         )
#define SEGGER_POWF(X, Y)   __CHOOSE(SEGGER_powf(X, Y),            ENTRY_F32(pow)(X, Y),                                 __SEGGER_RTL_float32_pow_inline(X, Y)        )
#define SEGGER_POW(X, Y)    __CHOOSE(SEGGER_pow(X, Y),             ENTRY_F64(pow)(X, Y),                                 __SEGGER_RTL_float64_pow_inline(X, Y)        )
#define SEGGER_ATANF(X)     __CHOOSE(SEGGER_atanf(X),              ENTRY_F32(atan)(X),                                   __SEGGER_RTL_float32_atan_inline(X)          )
#define SEGGER_ATAN(X)      __CHOOSE(SEGGER_atan(X),               ENTRY_F64(atan)(X),                                   __SEGGER_RTL_float64_atan_inline(X)          )
#define SEGGER_ATAN2F(Y, X) __CHOOSE(SEGGER_atan2f(Y, X),          ENTRY_F32(atan2)(Y, X),                               __SEGGER_RTL_float32_atan2_inline(Y, X)      )
#define SEGGER_ATAN2(Y, X)  __CHOOSE(SEGGER_atan2(Y, X),           ENTRY_F64(atan2)(Y, X),                               __SEGGER_RTL_float64_atan2_inline(Y, X)      )
#define SEGGER_HYPOTF(Y, X) __CHOOSE(SEGGER_hypotf(Y, X),          ENTRY_F32(hypot)(Y, X),                               __SEGGER_RTL_float32_hypot_inline(Y, X)      )
#define SEGGER_HYPOT(Y, X)  __CHOOSE(SEGGER_hypot(Y, X),           ENTRY_F64(hypot)(Y, X),                               __SEGGER_RTL_float64_hypot_inline(Y, X)      )
#define SEGGER_FMAF(X,Y,Z)  __CHOOSE(SEGGER_fmaf(X, Y, Z),         __SEGGER_RTL_float32_fma_inline(X, Y, Z),             __SEGGER_RTL_float32_fma_inline(X, Y, Z)     )
#define SEGGER_FMA(X,Y,Z)   __CHOOSE(SEGGER_fma(X, Y, Z),          __SEGGER_RTL_float64_fma_inline(X, Y, Z),             __SEGGER_RTL_float64_fma_inline(X, Y, Z)     )
#define SEGGER_FMSF(X,Y,Z)  __CHOOSE(SEGGER_fmsf(X, Y, Z),         __SEGGER_RTL_float32_fma_inline(X, Y, -(Z)),          __SEGGER_RTL_float32_fma_inline(X, Y, -(Z))  )
#define SEGGER_FMS(X,Y,Z)   __CHOOSE(SEGGER_fms(X, Y, Z),          __SEGGER_RTL_float64_fma_inline(X, Y, -(Z)),          __SEGGER_RTL_float64_fma_inline(X, Y, -(Z))  )

#define SEGGER_MODFF(X,Y)   __CHOOSE(SEGGER_modff(X, Y),           ENTRY_F32(modf)(X, Y),                                __SEGGER_RTL_float32_modf_inline(X, Y)       )
#define SEGGER_MODF(X,Y)    __CHOOSE(SEGGER_modf(X, Y),            ENTRY_F64(modf)(X, Y),                                __SEGGER_RTL_float64_modf_inline(X, Y)       )
//
//
#define SEGGER_LTF(X, Y)    __CHOOSE(SEGGER_ltf((X), (Y)),         ((X) <  (Y)),                                         ((X) <  (Y))                                 )
#define SEGGER_LT(X, Y)     __CHOOSE(SEGGER_lt ((X), (Y)),         ((X) <  (Y)),                                         ((X) <  (Y))                                 )
#define SEGGER_LEF(X, Y)    __CHOOSE(SEGGER_lef((X), (Y)),         ((X) <= (Y)),                                         ((X) <= (Y))                                 )
#define SEGGER_LE(X, Y)     __CHOOSE(SEGGER_le ((X), (Y)),         ((X) <= (Y)),                                         ((X) <= (Y))                                 )
#define SEGGER_GTF(X, Y)    __CHOOSE(SEGGER_gtf((X), (Y)),         ((X) >  (Y)),                                         ((X) >  (Y))                                 )
#define SEGGER_GT(X, Y)     __CHOOSE(SEGGER_gt ((X), (Y)),         ((X) >  (Y)),                                         ((X) >  (Y))                                 )
#define SEGGER_GEF(X, Y)    __CHOOSE(SEGGER_gef((X), (Y)),         ((X) >= (Y)),                                         ((X) >= (Y))                                 )
#define SEGGER_GE(X, Y)     __CHOOSE(SEGGER_ge ((X), (Y)),         ((X) >= (Y)),                                         ((X) >= (Y))                                 )
#define SEGGER_EQF(X, Y)    __CHOOSE(SEGGER_eqf((X), (Y)),         ((X) == (Y)),                                         ((X) == (Y))                                 )
#define SEGGER_EQ(X, Y)     __CHOOSE(SEGGER_eq ((X), (Y)),         ((X) == (Y)),                                         ((X) == (Y))                                 )
#define SEGGER_NEF(X, Y)    __CHOOSE(SEGGER_nef((X), (Y)),         ((X) != (Y)),                                         ((X) != (Y))                                 )
#define SEGGER_NE(X, Y)     __CHOOSE(SEGGER_ne ((X), (Y)),         ((X) != (Y)),                                         ((X) != (Y))                                 )
//
#define SEGGER_LT0F(X)      __CHOOSE(SEGGER_ltf((X), 0),           ((X) < 0),                                            ((X) < 0)                                    )
#define SEGGER_LT0(X)       __CHOOSE(SEGGER_lt ((X), 0),           ((X) < 0),                                            ((X) < 0)                                    )
#define SEGGER_LE0F(X)      __CHOOSE(SEGGER_lef((X), 0),           ((X) <= 0),                                           ((X) <= 0)                                   )
#define SEGGER_LE0(X)       __CHOOSE(SEGGER_le ((X), 0),           ((X) <= 0),                                           ((X) <= 0)                                   )
#define SEGGER_EQ0F(X)      __CHOOSE(SEGGER_eqf((X), 0),           __SEGGER_RTL_float32_putative_iszero(X),              __SEGGER_RTL_float32_putative_iszero(X)      )
#define SEGGER_EQ0(X)       __CHOOSE(SEGGER_eq ((X), 0),           ((X) == 0),                                           ((X) == 0)                                   )
#define SEGGER_NE0F(X)      __CHOOSE(SEGGER_nef((X), 0),           ((X) != 0),                                           ((X) != 0)                                   )
#define SEGGER_NE0(X)       __CHOOSE(SEGGER_ne ((X), 0),           ((X) != 0),                                           ((X) != 0)                                   )
#define SEGGER_GT0F(X)      __CHOOSE(SEGGER_gtf((X), 0),           ((X) > 0),                                            ((X) > 0)                                    )
#define SEGGER_GT0(X)       __CHOOSE(SEGGER_gt ((X), 0),           ((X) > 0),                                            ((X) > 0)                                    )
#define SEGGER_GE0F(X)      __CHOOSE(SEGGER_gef((X), 0),           ((X) >= 0),                                           ((X) >= 0)                                   )
#define SEGGER_GE0(X)       __CHOOSE(SEGGER_ge ((X), 0),           ((X) >= 0),                                           ((X) >= 0)                                   )
//
#define SEGGER_F2I(X)       __CHOOSE(SEGGER_float_to_int(X),       (int)(float)(X),                                      (int)(float)(X)                              )
#define SEGGER_I2F(X)       __CHOOSE(SEGGER_int_to_float(X),       (float)(int)(X),                                      (float)(int)(X)                              )
#define SEGGER_F2L(X)       __CHOOSE(SEGGER_float_to_long(X),      (long)(float)(X),                                     (long)(float)(X)                             )
#define SEGGER_F2LL(X)      __CHOOSE(SEGGER_float_to_llong(X),     (long long)(float)(X),                                (long long)(float)(X)                        )
#define SEGGER_F2U(X)       __CHOOSE(SEGGER_float_to_uint(X),      (unsigned)(float)(X),                                 (unsigned)(float)(X)                         )
#define SEGGER_D2I(X)       __CHOOSE(SEGGER_double_to_int(X),      (int)(double)(X),                                     (int)(double)(X)                             )
#define SEGGER_D2L(X)       __CHOOSE(SEGGER_double_to_long(X),     (long)(double)(X),                                    (long)(double)(X)                            )
#define SEGGER_D2LL(X)      __CHOOSE(SEGGER_double_to_llong(X),    (long long)(double)(X),                               (long long)(double)(X)                       )
#define SEGGER_I2D(X)       __CHOOSE(SEGGER_int_to_double(X),      (double)(int)(X),                                     (double)(int)(X)                             )
#define SEGGER_D2U(X)       __CHOOSE(SEGGER_double_to_uint(X),     (unsigned)(double)(X),                                (unsigned)(double)(X)                        )
#define SEGGER_U2D(X)       __CHOOSE(SEGGER_uint_to_double(X),     (double)(unsigned)(X),                                (double)(unsigned)(X)                        )
#define SEGGER_LD2D(X)      __CHOOSE(SEGGER_ldouble_to_double(X),  (double)(long double)(X),                             (double)(long double)(X)                     )
#define SEGGER_LD2F(X)      __CHOOSE(SEGGER_ldouble_to_float(X),   (float)(long double)(X),                              (float)(long double)(X)                      )
//
#define SEGGER_LDEXPF(X, N) __CHOOSE(SEGGER_ldexpf(X, N),          ENTRY_F32(ldexp)(X, N),                               __SEGGER_RTL_float32_ldexp_inline(X, N)      )
#define SEGGER_LDEXP(X, N)  __CHOOSE(SEGGER_ldexp(X, N),           ENTRY_F64(ldexp)(X, N),                               __SEGGER_RTL_float64_ldexp_inline(X, N)      )
#define SEGGER_FREXPF(X, N) __CHOOSE(SEGGER_frexpf(X, N),          ENTRY_F32(frexp)(X, N),                               __SEGGER_RTL_float32_frexp_inline(X, N)      )
#define SEGGER_FREXP(X, N)  __CHOOSE(SEGGER_frexp(X, N),           ENTRY_F64(frexp)(X, N),                               __SEGGER_RTL_float64_frexp_inline(X, N)      )

#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  #define SEGGER_MUL2F(X)   ((X) *  2.0f)
  #define SEGGER_MULM2F(X)  ((X) * -2.0f)
  #define SEGGER_DIV2F(X)   ((X) *  0.5f)
#else
  #define SEGGER_MUL2F(X)   __CHOOSE(SEGGER_ldexpf(X, +1),              ENTRY_F32(ldexp)(X, +1),                    __SEGGER_RTL_float32_ldexp_inline(X, +1)              )
  #define SEGGER_MULM2F(X)  __CHOOSE(SEGGER_NEGF(SEGGER_ldexpf(X, +1)), SEGGER_NEGF(ENTRY_F32(ldexp)((X), +1)),     SEGGER_NEGF(__SEGGER_RTL_float32_ldexp_inline(X, +1)) )
  #define SEGGER_DIV2F(X)   __CHOOSE(SEGGER_ldexpf(X, -1),              ENTRY_F32(ldexp)(X, -1),                    __SEGGER_RTL_float32_ldexp_inline(X, -1)              )
#endif

#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  #define SEGGER_MUL2(X)    ((X) *  2.0)
  #define SEGGER_MULM2(X)   ((X) * -2.0)
  #define SEGGER_DIV2(X)    ((X) *  0.5)
#else
  #define SEGGER_MUL2(X)    __CHOOSE(SEGGER_ldexp(X, +1),             ENTRY_F64(ldexp)(X, +1),                     __SEGGER_RTL_float64_ldexp_inline(X, +1)             )
  #define SEGGER_MULM2(X)   __CHOOSE(SEGGER_NEG(SEGGER_ldexp(X, +1)), SEGGER_NEG(ENTRY_F64(ldexp)(X, +1)),         SEGGER_NEG(__SEGGER_RTL_float64_ldexp_inline(X, +1)) )
  #define SEGGER_DIV2(X)    __CHOOSE(SEGGER_ldexp(X, -1),             ENTRY_F64(ldexp)(X, -1),                     __SEGGER_RTL_float64_ldexp_inline(X, -1)             )
#endif

#define M_E                2.7182818284590452354
#define M_LOG2E            1.4426950408889634074
#define M_LOG10E           0.43429448190325182765
#define M_LN2              0.693147180559945309417
#define M_LN10             2.30258509299404568402

#define M_PI               3.14159265358979323846   // Pi
#define M_PI_2             1.57079632679489661923   // Pi/2
#define M_PI_3             1.04719755119659774615   // Pi/3
#define M_PI_4             0.78539816339744830962   // Pi/4
#define M_PI_6             0.52359877559829887038   // Pi/6
#define M_1_PI             0.31830988618379067154
#define M_2_PI             0.63661977236758134308
#define M_3_PI_4           2.3561944901923449288469825374596   // 3Pi/4
#define M_SQRT_1_2         0.70710678118654752440   // Sqrt[0.5]
#define M_SQRT_3           1.73205080756887729353   // Sqrt[3]
#define M_INV_PI           0.31830988618379067154   // 1/Pi
#define M_LN_HUGE          709.78271289338397       // Ln[HUGE_VAL]
#define M_LN_HUGEF         88.7228f                 // Ln[HUGE_VALF]

#define M_LN2_DBL          6.93147180559945286227e-01                                             // 0x3FE62E42FEFA39EF
#define M_LN2_FLT          6.9314718246e-01f                                                      // 0x3F317218
#define M_LN10_DBL         2.30258509299404590109361379290930926799774169921875                   // 0x40026BB1BBB55516
#define M_LN10_FLT         2.302585124969482421875f                                               // 0x40135D8E
#define M_INV_LN2_DBL      1.44269504088896338700465094007086008787155151367187                   // 0x3FF71547652B82FE
#define M_INV_LN2_FLT      __SEGGER_RTL_FLT_SELECT(0x1.715476p0f, 1.44269502162933349609375f)      // 0x3FB8AA3B
#define M_INV_LN10_DBL     0.43429448190325181666793241674895398318767547607422                   // 0x3FDBCB7B1526E50E
#define M_INV_LN10_FLT     __SEGGER_RTL_FLT_SELECT(0x1.bcb7b2p-2f, 0.4342944920063018798828125f)   // 0x3EDE5BD9

#define M_SQRT_PI          1.772453850905516027298167         // Sqrt(Pi)
#define M_LOG_2PI          1.8378770664093454835606594728112  // Log(2Pi)

#define K_SINCOS_P_DBL    -0.16666666666666665052e+0, 0.83333333333331650315e-2, -0.19841269841201840457e-3, 0.27557319210152756119e-5, -0.25052106798274584544e-7, 0.16058936490371589114e-9, -0.76429178068910467734e-12, 0.27204790957888846175e-14

// Sollya: prec=200!; fpminimax(sin(x),[|1,3,5,7,9,11|], [|single...|], [1e-50;pi/2], floating, absolute);
#define K_SINCOS_P_FLT    __SEGGER_RTL_FLT_SELECT(-0x1.555556p-3f,  -0.16666667163372039794921875f), __SEGGER_RTL_FLT_SELECT( 0x1.11112ep-7f,   8.333346806466579437255859375e-3f), __SEGGER_RTL_FLT_SELECT(-0x1.a0205p-13f,  -1.98424444533884525299072265625e-4f), __SEGGER_RTL_FLT_SELECT( 0x1.725326p-19f,  2.759134758889558725059032440185546875e-6f), __SEGGER_RTL_FLT_SELECT(-0x1.ab55a8p-26f, -2.487414718643776723183691501617431640625e-8f)

#define K_ASINACOS_P_FLT  -0.27368494524164255994e2f, 0.57208227877891731407e2f, -0.39688862997504877339e2f, 0.10152522233806463645e2f, -0.69674573447350646411e0f
#define K_ASINACOS_Q_FLT  -0.16421096714498560795e3f, 0.41714430248260412556e3f, -0.38186303361750149284e3f, 0.15095270841030604719e3f, -0.23823859153670238830e2f

#define K_ASINACOS_P_DBL  -0.27368494524164255994e2, 0.57208227877891731407e2, -0.39688862997504877339e2, 0.10152522233806463645e2, -0.69674573447350646411e0
#define K_ASINACOS_Q_DBL  -0.16421096714498560795e3, 0.41714430248260412556e3, -0.38186303361750149284e3, 0.15095270841030604719e3, -0.23823859153670238830e2

#define K_TAN_P_FLT       -0.13338350006421960681e+0f, 0.34248878235890589960e-2f, -0.17861707342254426711e-4f
#define K_TAN_Q_FLT       -0.46671683339755294240e+0f, 0.25663832289440112864e-1f, -0.31181531907010027307e-3f, 0.49819433993786512270e-6f

#define K_TAN_P_DBL       -0.13338350006421960681e+0, 0.34248878235890589960e-2, -0.17861707342254426711e-4
#define K_TAN_Q_DBL       -0.46671683339755294240e+0, 0.25663832289440112864e-1, -0.31181531907010027307e-3, 0.49819433993786512270e-6

#define K_ATAN_P_FLT      -0.13688768894191926929e2f, -0.20505855195861651981e2f, -0.84946240351320683534e1f, -0.83758299368150059274f
#define K_ATAN_Q_FLT       0.41066306682575781263e2f,  0.86157349597130242515e2f,  0.59578436142597344465e2f,  0.15024001160028576121e2f

#define K_ATAN_P_DBL      -0.13688768894191926929e2, -0.20505855195861651981e2, -0.84946240351320683534e1, -0.83758299368150059274   
#define K_ATAN_Q_DBL       0.41066306682575781263e2,  0.86157349597130242515e2,  0.59578436142597344465e2,  0.15024001160028576121e2

#define K_TANH_P_FLT      -0.16134119023996228053e4f, -0.99225929672236083313e2f, -0.96437492777225469787e0f
#define K_TANH_Q_FLT       0.48402357071988688686e4f,  0.22337720718962312926e4f,  0.11274474380534949335e3f

#define K_TANH_P_DBL      -0.16134119023996228053e4, -0.99225929672236083313e2, -0.96437492777225469787e0
#define K_TANH_Q_DBL       0.48402357071988688686e4,  0.22337720718962312926e4,  0.11274474380534949335e3

#define K_LOG_P_DBL        0.16383943563021534222e2, -0.78956112887491257267e0
#define K_LOG_Q_DBL       -0.76949932108494879777e3,  0.31203222091924532844e3, -0.35667977739034646171e2

#define K_SINH_P_DBL      -0.35181283430177117881e6, -0.11563521196851768270e5, -0.16375798202630751372e3, -0.78966127417357099479e0
#define K_SINH_Q_DBL      -0.21108770058106271242e7,  0.36162723109421836460e5, -0.27773523119650701667e3

#define K_INT_MIN         (int)((~0u >> 1) + 1u)
#define K_INT_MAX         (int)(~0u >> 1)

#define K_FP_ILOGB0       K_INT_MIN
#define K_FP_ILOGBNAN     K_INT_MAX

//
// Constants for fixed-point representation 
//
#define K_one_U32            __SEGGER_RTL_U32_C(0x3F800000)          // 1.0
#define K_one_U64            __SEGGER_RTL_U64_C(0x3FF0000000000000)  // 1.0
#define K_PiOver2_U32        __SEGGER_RTL_U32_C(0x3FC90FDB)          // Pi/2
#define K_Pi_U32             __SEGGER_RTL_U32_C(0x40490FDB)          // Pi ditto.
#define K_Sqrt_0v75_U32      __SEGGER_RTL_U32_C(0x3F5DB3D8)          // Sqrt[1-(0.5)^2] = Sqrt[1-0.25] = Sqrt[0.75]
#define K_log_HUGE_U32       __SEGGER_RTL_U32_C(0x42B17217)          // Log(HUGE),  where huge is the most positive normal.  Numbers greater than this overflow in Exp[].
#define K_log_SMALL_U32      __SEGGER_RTL_U32_C(0xC2AEAC4F)          // Log(SMALL), where small is the smallest positive normal.  Numbers smaller than this underflow in Exp[].
#define K_Pi_2Q29            __SEGGER_RTL_U32_C(0x6487ED51)          // Pi in Q2.29 format: BaseForm[N[Pi, 10]*2^29, 16]

// Integer versions of special floating values.
#define K_NAN_U32            __SEGGER_RTL_U32_C(0x7FC00000)          // Quiet NaN
#define K_NAN_U64            __SEGGER_RTL_U64_C(0x7FF8000000000000)  // Quiet NaN
#define K_INF_U32            __SEGGER_RTL_U32_C(0x7F800000)
#define K_INF_U64            __SEGGER_RTL_U64_C(0x7FF0000000000000)
#define K_MINUS_INF_U32      __SEGGER_RTL_U32_C(0xFF800000)
#define K_MINUS_INF_U64      __SEGGER_RTL_U64_C(0xFFF0000000000000)
#define K_ZERO_U32           __SEGGER_RTL_U32_C(0x00000000)
#define K_ZERO_U64           __SEGGER_RTL_U64_C(0x0000000000000000)
#define K_MINUS_ZERO_U32     __SEGGER_RTL_U32_C(0x00000000)
#define K_MINUS_ZERO_U64     __SEGGER_RTL_U64_C(0x8000000000000000)

// Floating versions of special floating values.
#define K_INF_F32            __SEGGER_RTL_BitcastToF32(K_INF_U32)
#define K_INF_F64            __SEGGER_RTL_BitcastToF64(K_INF_U64)
#define K_NAN_F32            __SEGGER_RTL_BitcastToF32(K_NAN_U32)
#define K_NAN_F64            __SEGGER_RTL_BitcastToF64(K_NAN_U64)

//
// Log2(e) to 94 bits: 0x1.71547652B82FE1777D0FFDA0p0
//
#define K_LOG2_E_H           0x2E2A8ECA
#define K_LOG2_E_M           0x5705FC2F
#define K_LOG2_E_L           0xEFA1FFB4

#define FLOAT16_TOTAL_BITS                 16
#define FLOAT16_SIGNIFICAND_BITS           10
#define FLOAT16_EXPONENT_BITS              5
#define FLOAT16_EXPONENT_INF               ((1u << FLOAT16_EXPONENT_BITS) - 1)
#define FLOAT16_EXPONENT_BIAS              (FLOAT16_EXPONENT_INF >> 1)
#define FLOAT16_SIGNIFICAND_MASK           ((UINT16_C(1) << FLOAT16_SIGNIFICAND_BITS) - 1)
#define FLOAT16_SIGN_MASK                  (UINT16_C(1) << (FLOAT16_TOTAL_BITS-1))
#define FLOAT16_HIDDEN_MASK                ((UINT16_C(1) << FLOAT16_SIGNIFICAND_BITS))
#define FLOAT16_SIGN(x)                    ((x) & FLOAT16_SIGN_MASK)
#define FLOAT16_EXPONENT(x)                ((__SEGGER_RTL_INT_LEAST16_T)((x) >> FLOAT16_SIGNIFICAND_BITS) & FLOAT16_EXPONENT_INF)
#define FLOAT16_MK(S,E,M)                  (((S) << (FLOAT16_TOTAL_BITS-1)) | ((E) << FLOAT16_SIGNIFICAND_BITS) | (M))

#define FLOAT32_TOTAL_BITS                 32
#define FLOAT32_SIGNIFICAND_BITS           23
#define FLOAT32_EXPONENT_BITS              8
#define FLOAT32_EXPONENT_INF               ((1u << FLOAT32_EXPONENT_BITS) - 1)
#define FLOAT32_EXPONENT_BIAS              (FLOAT32_EXPONENT_INF >> 1)
#define FLOAT32_SIGNIFICAND_MASK           ((__SEGGER_RTL_U32_C(1) << FLOAT32_SIGNIFICAND_BITS) - 1)
#define FLOAT32_SIGNIFICAND_X_MASK         ((__SEGGER_RTL_U32_C(1) << (FLOAT32_SIGNIFICAND_BITS+1)) - 1)  // Significand plus hidden bit mask
#define FLOAT32_SIGN_MASK                  (__SEGGER_RTL_U32_C(1) << (FLOAT32_TOTAL_BITS-1))
#define FLOAT32_HIDDEN_MASK                ((__SEGGER_RTL_U32_C(1) << FLOAT32_SIGNIFICAND_BITS))
#define FLOAT32_EXPONENT_MASK              ((__SEGGER_RTL_U32)(FLOAT32_EXPONENT_INF) << FLOAT32_SIGNIFICAND_BITS)
#define FLOAT32_EXPONENT(x)                ((__SEGGER_RTL_INT_LEAST16_T)((x) >> FLOAT32_SIGNIFICAND_BITS) & FLOAT32_EXPONENT_INF)
#define FLOAT32_SIGN(x)                    ((x) & FLOAT32_SIGN_MASK)
#define FLOAT32_MK(E,M,S)                  (((__SEGGER_RTL_U32)(S) << (FLOAT32_TOTAL_BITS-1)) | ((__SEGGER_RTL_U32)(E) << FLOAT32_SIGNIFICAND_BITS) | (__SEGGER_RTL_U32)(M))

#define FLOAT64_TOTAL_BITS                 64
#define FLOAT64_SIGNIFICAND_BITS           52
#define FLOAT64_EXPONENT_BITS              11
#define FLOAT64_EXPONENT_INF               ((1 << FLOAT64_EXPONENT_BITS) - 1)
#define FLOAT64_EXPONENT_BIAS              (FLOAT64_EXPONENT_INF >> 1)
#define FLOAT64_SIGNIFICAND_MASK           ((__SEGGER_RTL_U64_C(1) << FLOAT64_SIGNIFICAND_BITS) - 1)
#define FLOAT64_SIGNIFICAND_X_MASK         ((__SEGGER_RTL_U64_C(1) << (FLOAT64_SIGNIFICAND_BITS+1)) - 1)  // Significand plus hidden bit mask
#define FLOAT64_SIGN_MASK                  (__SEGGER_RTL_U64_C(1) << (FLOAT64_TOTAL_BITS-1))
#define FLOAT64_HIDDEN_MASK                ((__SEGGER_RTL_U64_C(1) << FLOAT64_SIGNIFICAND_BITS))
#define FLOAT64_EXPONENT_MASK              ((__SEGGER_RTL_U64)(FLOAT64_EXPONENT_INF) << FLOAT64_SIGNIFICAND_BITS)
#define FLOAT64_EXPONENT(x)                ((__SEGGER_RTL_INT_LEAST16_T)((x) >> FLOAT64_SIGNIFICAND_BITS) & FLOAT64_EXPONENT_INF)
#define FLOAT64_SIGN(x)                    ((x) & FLOAT64_SIGN_MASK)
#define FLOAT64_MK(E,M,S)                  (((__SEGGER_RTL_U64)(S) << (FLOAT64_TOTAL_BITS-1)) | ((__SEGGER_RTL_U64)(E) << FLOAT64_SIGNIFICAND_BITS) | (__SEGGER_RTL_U64)(M))

#define FLOAT128_TOTAL_BITS                128
#define FLOAT128_SIGNIFICAND_BITS          112
#define FLOAT128_EXPONENT_BITS             15
#define FLOAT128_EXPONENT_INF              ((1u << FLOAT128_EXPONENT_BITS) - 1)
#define FLOAT128_EXPONENT_BIAS             (FLOAT128_EXPONENT_INF >> 1)
#define FLOAT128_SIGNIFICAND_HI_MASK       ((__SEGGER_RTL_U64_C(1) << (FLOAT128_SIGNIFICAND_BITS-64)) - 1)
#define FLOAT128_SIGN_HI_MASK              (__SEGGER_RTL_U64_C(1) << (FLOAT128_TOTAL_BITS-64-1))
#define FLOAT128_EXPONENT_HI_MASK          ((__SEGGER_RTL_U64)(FLOAT128_EXPONENT_INF) << (FLOAT128_SIGNIFICAND_BITS-64))
#define FLOAT128_EXPONENT_HI(x)            ((__SEGGER_RTL_INT_LEAST16_T)((x) >> (FLOAT128_SIGNIFICAND_BITS-64)) & FLOAT128_EXPONENT_INF)
#define FLOAT128_SIGN_HI(x)                ((x) & FLOAT128_SIGN_HI_MASK)

#define __SEGGER_RTL_SIGN_EXTEND(x, n)     ((__SEGGER_RTL_I32)(x) << (32-(n)) >> (32-(n)))
#define __SEGGER_RTL_X2(X)                 ((__SEGGER_RTL_U32)(X) << 1)
#define __SEGGER_RTL_NORMALIZE_1UP(X, Y)   if (((X) & __SEGGER_RTL_U32_C(0x40000000)) == 0) { (X) <<= 1; ++(Y); }
#define __SEGGER_RTL_NORMALIZE_1DN(X, Y)   if ((X) & __SEGGER_RTL_U32_C(0x80000000))        { (X) >>= 1; --(Y); }

#define __SEGGER_RTL_ABS_I32(X)            (((__SEGGER_RTL_I32)(X) ^ ((__SEGGER_RTL_I32)(X) >> 31)) - ((__SEGGER_RTL_I32)(X) >> 31))
#define __SEGGER_RTL_ABSX_I32(X)           (((__SEGGER_RTL_I32)(X) ^ ((__SEGGER_RTL_I32)(X) >> 31)))
#define __SEGGER_RTL_SAFE_ASR_I32(X, N)    ((__SEGGER_RTL_I32)(X) >> ((N) < 32 ? (N) : 31))
#define __SEGGER_RTL_SAFE_LSL_U32(X, N)    ((__SEGGER_RTL_U32)(N) < 32 ? (X) << (__SEGGER_RTL_U32)(N) : 0)
#define __SEGGER_RTL_SAFE_LSR_U32(X, N)    ((__SEGGER_RTL_U32)(N) < 32 ? (__SEGGER_RTL_U32)(X) >> (__SEGGER_RTL_U32)(N) : 0)
#define __SEGGER_RTL_LSL_U64_HI(H, L, N)   (((H) << (N)) | ((L) >> (32-N)))
#define __SEGGER_RTL_PACK(EXP, SIG, SGN)   (((__SEGGER_RTL_U32)(EXP) << 23) + (((SIG) + 64) >> 7) + ((SGN) & FLOAT32_SIGN_MASK))
#define __SEGGER_RTL_U64_ROUND(X)          ((__SEGGER_RTL_U32)((X) >> 32) + ((__SEGGER_RTL_U32)(X) >> 31))

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

#if defined(__SEGGER_RTL_FLOAT16)
typedef union {
  __SEGGER_RTL_U16     l;
  __SEGGER_RTL_FLOAT16 f;
} SEGGER_RTL_float16_t;
#endif

typedef union {
  __SEGGER_RTL_U32 l;
  float            f;
} SEGGER_RTL_float32_t;

typedef union {
  __SEGGER_RTL_U64 l;
  double           f;
#if __SEGGER_RTL_BYTE_ORDER < 0
  struct {
    __SEGGER_RTL_U32 lo;
    __SEGGER_RTL_U32 hi;
  } part;
  struct {
    __SEGGER_RTL_U16 hi;
    __SEGGER_RTL_U16 mohi;
    __SEGGER_RTL_U16 molo;
    __SEGGER_RTL_U16 lo;
  } u16;
#else
  struct {
    __SEGGER_RTL_U32 hi;
    __SEGGER_RTL_U32 lo;
  } part;
#endif
} SEGGER_RTL_float64_t;

typedef union {
  long double f;
#if __SEGGER_RTL_BYTE_ORDER < 0
  struct {
    __SEGGER_RTL_U64 lo;
    __SEGGER_RTL_U64 hi;
  } part;
#else
  struct {
    __SEGGER_RTL_U64 hi;
    __SEGGER_RTL_U64 lo;
  } part;
#endif
} SEGGER_RTL_float128_t;

typedef struct {
  __SEGGER_RTL_U32 S_value; unsigned S_scale;  // True value is S_value/2^S_Scale
  __SEGGER_RTL_U32 C_value; unsigned C_scale;
} SEGGER_RTL_SIN_COS_PARA;

typedef struct {
  __SEGGER_RTL_U32 Value[16];
  __SEGGER_RTL_U8  Scale[16];
} SEGGER_RTL_LOGF_PARA;

typedef struct {
  __SEGGER_RTL_U32 Value[17];
  __SEGGER_RTL_U8  Scale[17];
} SEGGER_RTL_EXPM1F_PARA;

/*********************************************************************
*
*       Public const data
*
**********************************************************************
*/

// Number of leading zeros for input byte.
__SEGGER_RTL_RODATA __SEGGER_RTL_U8 __SEGGER_RTL_clz_lut[256] = {
  8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,  // 0x
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 1x
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // 2x
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // 3x
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 4x
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 5x
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 6x
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 7x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 8x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 9x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // Ax
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // Bx
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // Cx
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // Dx
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // Ex
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // Fx
};

// 1/(1+j/16) for j=[0, 16].  Scaled by 256 for integers.
static __SEGGER_RTL_RODATA __SEGGER_RTL_U16 __SEGGER_RTL_logf_1_over_1_plus_j_by_16[] = {
  0x0100, 0x00F1, 0x00E4, 0x00D8,
  0x00CD, 0x00C3, 0x00BA, 0x00B2,
  0x00AB, 0x00A4, 0x009E, 0x0098,
  0x0092, 0x008D, 0x0089, 0x0084,
  0x0080
};

// Log[1+j/16] for j=[0, 16].
static __SEGGER_RTL_RODATA SEGGER_RTL_LOGF_PARA __SEGGER_RTL_logf_log_1_plus_j_by_16 = {
  { __SEGGER_RTL_U32_C(0x00000000), __SEGGER_RTL_U32_C(0x3DD46380), __SEGGER_RTL_U32_C(0x3B4E4EC7), __SEGGER_RTL_U32_C(0x56FD01AD),
    __SEGGER_RTL_U32_C(0x38DFF78E), __SEGGER_RTL_U32_C(0x45AD732F), __SEGGER_RTL_U32_C(0x51C6370A), __SEGGER_RTL_U32_C(0x5D0761DB),
    __SEGGER_RTL_U32_C(0xB5DA97B1), __SEGGER_RTL_U32_C(0xC08DCF25), __SEGGER_RTL_U32_C(0x9430DAC6), __SEGGER_RTL_U32_C(0xA8033E3D),
    __SEGGER_RTL_U32_C(0xBCA2060D), __SEGGER_RTL_U32_C(0xCE797454), __SEGGER_RTL_U32_C(0xBA6B2445), __SEGGER_RTL_U32_C(0xE07D64F2) },
  { 0,          34,         33,         33,
    32,         32,         32,         32,
    32,         32,         33,         33,
    33,         33,         34,         34 }
};

static __SEGGER_RTL_RODATA SEGGER_RTL_EXPM1F_PARA __SEGGER_RTL_float32_Expfm1_exp_j_by_8 = {
  { __SEGGER_RTL_U32_C(0xAF16AC6C), __SEGGER_RTL_U32_C(0xB55BBC13), __SEGGER_RTL_U32_C(0xBC7681D8), __SEGGER_RTL_U32_C(0x8906E49A),
    __SEGGER_RTL_U32_C(0x9B4597E3), __SEGGER_RTL_U32_C(0xAFF230AF), __SEGGER_RTL_U32_C(0xC75F7CF5), __SEGGER_RTL_U32_C(0xC3D6A24F),
    __SEGGER_RTL_U32_C(0x00000000),
    __SEGGER_RTL_U32_C(0x442C08B7), __SEGGER_RTL_U32_C(0x48B5E3C4), __SEGGER_RTL_U32_C(0x3A3D289F), __SEGGER_RTL_U32_C(0x53094C71),
    __SEGGER_RTL_U32_C(0x6F22AEFC), __SEGGER_RTL_U32_C(0x477CEDA3), __SEGGER_RTL_U32_C(0x59872C3E), __SEGGER_RTL_U32_C(0x6DF85459)},
  { 31,         31,         31,         32,
    32,         32,         32,         33,
     0,
    33,         32,         31,         31,
    31,         30,         30,         30 }
};

// See [APPLE].
static __SEGGER_RTL_RODATA __SEGGER_RTL_U8 __SEGGER_RTL_rsqrt_approx_bits[256] = {
  0xFE, 0xFC, 0xFA, 0xF8, 0xF6, 0xF4, 0xF2, 0xF0,
  0xEE, 0xED, 0xEB, 0xE9, 0xE7, 0xE6, 0xE4, 0xE2,
  0xE1, 0xDF, 0xDD, 0xDC, 0xDA, 0xD8, 0xD7, 0xD5,
  0xD4, 0xD2, 0xD1, 0xCF, 0xCE, 0xCC, 0xCB, 0xC9,
  0xC8, 0xC7, 0xC5, 0xC4, 0xC2, 0xC1, 0xC0, 0xBE,
  0xBD, 0xBC, 0xBA, 0xB9, 0xB8, 0xB7, 0xB5, 0xB4,
  0xB3, 0xB2, 0xB0, 0xAF, 0xAE, 0xAD, 0xAC, 0xAB,
  0xA9, 0xA8, 0xA7, 0xA6, 0xA5, 0xA4, 0xA3, 0xA2,
  0xA0, 0x9F, 0x9E, 0x9D, 0x9C, 0x9B, 0x9A, 0x99,
  0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0x92, 0x91,
  0x90, 0x8F, 0x8E, 0x8D, 0x8C, 0x8B, 0x8B, 0x8A,
  0x89, 0x88, 0x87, 0x86, 0x85, 0x84, 0x83, 0x83,
  0x82, 0x81, 0x80, 0x7F, 0x7E, 0x7D, 0x7D, 0x7C,
  0x7B, 0x7A, 0x79, 0x79, 0x78, 0x77, 0x76, 0x75,
  0x75, 0x74, 0x73, 0x72, 0x72, 0x71, 0x70, 0x6F,
  0x6F, 0x6E, 0x6D, 0x6C, 0x6C, 0x6B, 0x6A, 0x6A,
  0x68, 0x67, 0x65, 0x64, 0x63, 0x61, 0x60, 0x5F,
  0x5D, 0x5C, 0x5B, 0x5A, 0x58, 0x57, 0x56, 0x55,
  0x54, 0x52, 0x51, 0x50, 0x4F, 0x4E, 0x4D, 0x4C,
  0x4B, 0x4A, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43,
  0x42, 0x41, 0x40, 0x3F, 0x3E, 0x3D, 0x3C, 0x3C,
  0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34,
  0x33, 0x33, 0x32, 0x31, 0x30, 0x2F, 0x2E, 0x2D,
  0x2D, 0x2C, 0x2B, 0x2A, 0x29, 0x29, 0x28, 0x27,
  0x26, 0x26, 0x25, 0x24, 0x23, 0x23, 0x22, 0x21,
  0x20, 0x20, 0x1F, 0x1E, 0x1E, 0x1D, 0x1C, 0x1C,
  0x1B, 0x1A, 0x19, 0x19, 0x18, 0x17, 0x17, 0x16,
  0x16, 0x15, 0x14, 0x14, 0x13, 0x12, 0x12, 0x11,
  0x11, 0x10, 0x0F, 0x0F, 0x0E, 0x0E, 0x0D, 0x0C,
  0x0C, 0x0B, 0x0B, 0x0A, 0x0A, 0x09, 0x08, 0x08,
  0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04,
  0x03, 0x03, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00,
};

//
// Two tables to support single and double precision square root.
//
// First is an 8-in-8-out table, estimation g=Sqrt[x]. Indexed by single
// low-order bit of exponent and leading 7 bits of significand (0.25 <= x < 1).
//
// Second is a 7-in-8-out table, with estimation y=1/2g.
//
static __SEGGER_RTL_RODATA __SEGGER_RTL_U8 __SEGGER_RTL_aSqrtData[] = {
  0xB5, 0xB5, 0xB6, 0xB7, 0xB7, 0xB8, 0xB9, 0xB9, 0xBA, 0xBB, 0xBB, 0xBC, 0xBD, 0xBD, 0xBE, 0xBF,
  0xC0, 0xC0, 0xC1, 0xC1, 0xC2, 0xC3, 0xC3, 0xC4, 0xC5, 0xC5, 0xC6, 0xC7, 0xC7, 0xC8, 0xC9, 0xC9,
  0xCA, 0xCB, 0xCB, 0xCC, 0xCC, 0xCD, 0xCE, 0xCE, 0xCF, 0xD0, 0xD0, 0xD1, 0xD1, 0xD2, 0xD3, 0xD3,
  0xD4, 0xD4, 0xD5, 0xD6, 0xD6, 0xD7, 0xD7, 0xD8, 0xD9, 0xD9, 0xDA, 0xDA, 0xDB, 0xDB, 0xDC, 0xDD,
  0xDD, 0xDE, 0xDE, 0xDF, 0xE0, 0xE0, 0xE1, 0xE1, 0xE2, 0xE2, 0xE3, 0xE3, 0xE4, 0xE5, 0xE5, 0xE6,
  0xE6, 0xE7, 0xE7, 0xE8, 0xE8, 0xE9, 0xEA, 0xEA, 0xEB, 0xEB, 0xEC, 0xEC, 0xED, 0xED, 0xEE, 0xEE,
  0xEF, 0xF0, 0xF0, 0xF1, 0xF1, 0xF2, 0xF2, 0xF3, 0xF3, 0xF4, 0xF4, 0xF5, 0xF5, 0xF6, 0xF6, 0xF7,
  0xF7, 0xF8, 0xF8, 0xF9, 0xF9, 0xFA, 0xFA, 0xFB, 0xFB, 0xFC, 0xFC, 0xFD, 0xFD, 0xFE, 0xFE, 0xFF,
  0x80, 0x80, 0x80, 0x81, 0x81, 0x82, 0x82, 0x83, 0x83, 0x84, 0x84, 0x85, 0x85, 0x86, 0x86, 0x87,
  0x87, 0x88, 0x88, 0x89, 0x89, 0x8A, 0x8A, 0x8B, 0x8B, 0x8B, 0x8C, 0x8C, 0x8D, 0x8D, 0x8E, 0x8E,
  0x8F, 0x8F, 0x90, 0x90, 0x90, 0x91, 0x91, 0x92, 0x92, 0x93, 0x93, 0x93, 0x94, 0x94, 0x95, 0x95,
  0x96, 0x96, 0x96, 0x97, 0x97, 0x98, 0x98, 0x99, 0x99, 0x99, 0x9A, 0x9A, 0x9B, 0x9B, 0x9B, 0x9C,
  0x9C, 0x9D, 0x9D, 0x9D, 0x9E, 0x9E, 0x9F, 0x9F, 0xA0, 0xA0, 0xA0, 0xA1, 0xA1, 0xA1, 0xA2, 0xA2,
  0xA3, 0xA3, 0xA3, 0xA4, 0xA4, 0xA5, 0xA5, 0xA5, 0xA6, 0xA6, 0xA7, 0xA7, 0xA7, 0xA8, 0xA8, 0xA8,
  0xA9, 0xA9, 0xAA, 0xAA, 0xAA, 0xAB, 0xAB, 0xAB, 0xAC, 0xAC, 0xAD, 0xAD, 0xAD, 0xAE, 0xAE, 0xAE,
  0xAF, 0xAF, 0xB0, 0xB0, 0xB0, 0xB1, 0xB1, 0xB1, 0xB2, 0xB2, 0xB2, 0xB3, 0xB3, 0xB3, 0xB4, 0xB4,
  //
  0xB3, 0xB2, 0xB0, 0xAF, 0xAE, 0xAD, 0xAB, 0xAA, 0xA9, 0xA8, 0xA7, 0xA6, 0xA5, 0xA3, 0xA2, 0xA1,
  0xA0, 0x9F, 0x9E, 0x9E, 0x9D, 0x9C, 0x9B, 0x9A, 0x99, 0x98, 0x97, 0x96, 0x96, 0x95, 0x94, 0x93,
  0x93, 0x92, 0x91, 0x90, 0x90, 0x8F, 0x8E, 0x8E, 0x8D, 0x8C, 0x8B, 0x8B, 0x8A, 0x8A, 0x89, 0x88,
  0x88, 0x87, 0x87, 0x86, 0x85, 0x85, 0x84, 0x84, 0x83, 0x83, 0x82, 0x82, 0x81, 0x81, 0x80, 0x80,
  0xFE, 0xFC, 0xFA, 0xF8, 0xF6, 0xF4, 0xF3, 0xF1, 0xEF, 0xEE, 0xEC, 0xEA, 0xE9, 0xE7, 0xE6, 0xE4,
  0xE3, 0xE2, 0xE0, 0xDF, 0xDE, 0xDC, 0xDB, 0xDA, 0xD9, 0xD7, 0xD6, 0xD5, 0xD4, 0xD3, 0xD2, 0xD1,
  0xCF, 0xCE, 0xCD, 0xCC, 0xCB, 0xCA, 0xC9, 0xC8, 0xC7, 0xC6, 0xC5, 0xC5, 0xC4, 0xC3, 0xC2, 0xC1,
  0xC0, 0xBF, 0xBE, 0xBE, 0xBD, 0xBC, 0xBB, 0xBA, 0xBA, 0xB9, 0xB8, 0xB7, 0xB7, 0xB6, 0xB5, 0xB5,
};

static __SEGGER_RTL_RODATA struct {
  __SEGGER_RTL_U8  aInitialApprox[32];
  __SEGGER_RTL_U32 aMidRoot      [32];
  __SEGGER_RTL_U32 aMult         [ 3];  // cbrt(2^j)
} __SEGGER_RTL_cbrt_paras = {
  { 0xFC, 0xF5, 0xED, 0xE7, 0xE0, 0xDA, 0xD5, 0xCF,
    0xCA, 0xC5, 0xC1, 0xBC, 0xB8, 0xB4, 0xB0, 0xAC,
    0xA9, 0xA5, 0xA2, 0x9F, 0x9C, 0x99, 0x96, 0x94,
    0x91, 0x8E, 0x8C, 0x8A, 0x87, 0x85, 0x83, 0x81
  },
  {
    __SEGGER_RTL_U32_C(0x40563BB0),
    __SEGGER_RTL_U32_C(0x40F19F34),
    __SEGGER_RTL_U32_C(0x41AA9F86),
    __SEGGER_RTL_U32_C(0x423AED69),
    __SEGGER_RTL_U32_C(0x42E9BB97),
    __SEGGER_RTL_U32_C(0x43857757),
    __SEGGER_RTL_U32_C(0x440BACEB),
    __SEGGER_RTL_U32_C(0x44B2618D),
    __SEGGER_RTL_U32_C(0x45424D5D),
    __SEGGER_RTL_U32_C(0x45D70DA4),
    __SEGGER_RTL_U32_C(0x4651B9F8),
    __SEGGER_RTL_U32_C(0x46EFEC05),
    __SEGGER_RTL_U32_C(0x477292C8),
    __SEGGER_RTL_U32_C(0x47F91155),
    __SEGGER_RTL_U32_C(0x48839B75),
    __SEGGER_RTL_U32_C(0x491268EC),
    __SEGGER_RTL_U32_C(0x49807339),
    __SEGGER_RTL_U32_C(0x4A174A2F),
    __SEGGER_RTL_U32_C(0x4A8BA80C),
    __SEGGER_RTL_U32_C(0x4B02EEB1),
    __SEGGER_RTL_U32_C(0x4B7D3F53),
    __SEGGER_RTL_U32_C(0x4BFABD4F),
    __SEGGER_RTL_U32_C(0x4C7B8E5C),
    __SEGGER_RTL_U32_C(0x4CD35BA4),
    __SEGGER_RTL_U32_C(0x4D5A11F2),
    __SEGGER_RTL_U32_C(0x4DE48CF5),
    __SEGGER_RTL_U32_C(0x4E430E98),
    __SEGGER_RTL_U32_C(0x4EA36181),
    __SEGGER_RTL_U32_C(0x4F37693A),
    __SEGGER_RTL_U32_C(0x4F9C8E92),
    __SEGGER_RTL_U32_C(0x5003C05A),
    __SEGGER_RTL_U32_C(0x506D1171),
  },
  {
    __SEGGER_RTL_U32_C(0x40000000),  // cbrt(1)
    __SEGGER_RTL_U32_C(0x50A28BE6),  // cbrt(2)
    __SEGGER_RTL_U32_C(0x6597FA94),  // cbrt(4)
  }
};

static __SEGGER_RTL_RODATA __SEGGER_RTL_U16 __SEGGER_RTL_inv_pi[] = {
  0x0000,
  0x517C, 0xC1B7, 0x2722, 0x0A94,
  0xFE13, 0xABE8, 0xFA9A, 0x6EE0,
  0x6DB1, 0x4ACC, 0x9E21, 0xC820,
};

static __SEGGER_RTL_RODATA SEGGER_RTL_SIN_COS_PARA __SEGGER_RTL_float32_SinCosParas[] = {
  { __SEGGER_RTL_U32_C(0x00000000), 32, 0x40000000, -2 },
  { __SEGGER_RTL_U32_C(0x31F17079), 32, 0x7D8A5F40, -1 },
  { __SEGGER_RTL_U32_C(0x61F78A9B), 32, 0x7641AF3D, -1 },
  { __SEGGER_RTL_U32_C(0x471CECE7), 31, 0x6A6D98A4, -1 },
  { __SEGGER_RTL_U32_C(0x5A82799A), 31, 0x5A82799A, -1 },
  { __SEGGER_RTL_U32_C(0x6A6D98A4), 31, 0x471CECE7, -1 },
  { __SEGGER_RTL_U32_C(0x7641AF3D), 31, 0x61F78A9B,  0 },
  { __SEGGER_RTL_U32_C(0x7D8A5F40), 31, 0x31F17079,  0 },
  { __SEGGER_RTL_U32_C(0x40000000), 30, 0x00000000,  0 },
  { __SEGGER_RTL_U32_C(0x7D8A5F40), 31, 0xCE0E8F87,  0 },
  { __SEGGER_RTL_U32_C(0x7641AF3D), 31, 0x9E087565,  0 },
  { __SEGGER_RTL_U32_C(0x6A6D98A4), 31, 0xB8E31319, -1 },
  { __SEGGER_RTL_U32_C(0x5A82799A), 31, 0xA57D8666, -1 },
  { __SEGGER_RTL_U32_C(0x471CECE7), 31, 0x9592675C, -1 },
  { __SEGGER_RTL_U32_C(0x61F78A9B), 32, 0x89BE50C3, -1 },
  { __SEGGER_RTL_U32_C(0x31F17079), 32, 0x8275A0C0, -1 },
};

static __SEGGER_RTL_RODATA __SEGGER_RTL_U64 __SEGGER_RTL_float64_aExpCoeff[] = {
  __SEGGER_RTL_U64_C(0x4000000000000000),
  __SEGGER_RTL_U64_C(0x40B268F9DE0183BA),
  __SEGGER_RTL_U64_C(0x4166C34C5615D0EC),
  __SEGGER_RTL_U64_C(0x421D1461D66F2023),
  __SEGGER_RTL_U64_C(0x42D561B3E6243D8A),
  __SEGGER_RTL_U64_C(0x438FB0CB4F468808),
  __SEGGER_RTL_U64_C(0x444C0740496D4294),
  __SEGGER_RTL_U64_C(0x450A6ABAA4B77ECD),
  __SEGGER_RTL_U64_C(0x45CAE0F1F545EB73),
  __SEGGER_RTL_U64_C(0x468D6FADBF2DD4F3),
  __SEGGER_RTL_U64_C(0x47521CC5A2E6A9E0),
  __SEGGER_RTL_U64_C(0x4818EE218A3358EE),
  __SEGGER_RTL_U64_C(0x48E1E9B9D588E19B),
  __SEGGER_RTL_U64_C(0x49AD159789F37496),
  __SEGGER_RTL_U64_C(0x4A7A77D47F7B84B1),
  __SEGGER_RTL_U64_C(0x4B4A169B900C2D00),
  __SEGGER_RTL_U64_C(0x4C1BF828C6DC54B8),
  __SEGGER_RTL_U64_C(0x4CF022C9905BFD32),
  __SEGGER_RTL_U64_C(0x4DC69CDCEAA72A9C),
  __SEGGER_RTL_U64_C(0x4E9F6CD3967FDBA8),
  __SEGGER_RTL_U64_C(0x4F7A993048D088D7),
  __SEGGER_RTL_U64_C(0x50582887DCB8A7E1),
  __SEGGER_RTL_U64_C(0x513821818624B40C),
  __SEGGER_RTL_U64_C(0x521A8AD704F3404F),
  __SEGGER_RTL_U64_C(0x52FF6B54D8A89C75),
  __SEGGER_RTL_U64_C(0x53E6C9DA74B29AB5),
  __SEGGER_RTL_U64_C(0x54D0AD5A753E077C),
  __SEGGER_RTL_U64_C(0x55BD1CDAD49F699C),
  __SEGGER_RTL_U64_C(0x56AC1F752150A563),
  __SEGGER_RTL_U64_C(0x579DBC56B48521BA),
  __SEGGER_RTL_U64_C(0x5891FAC0E95612C8),
  __SEGGER_RTL_U64_C(0x5988E20954889245),
  __SEGGER_RTL_U64_C(0x5A827999FCEF3242),
  __SEGGER_RTL_U64_C(0x5B7EC8F19468BBC9),
  __SEGGER_RTL_U64_C(0x5C7DD7A3B17DCF75),
  __SEGGER_RTL_U64_C(0x5D7FAD59099F22FE),
  __SEGGER_RTL_U64_C(0x5E8451CFAC061B5F),
  __SEGGER_RTL_U64_C(0x5F8BCCDB3D398841),
  __SEGGER_RTL_U64_C(0x6096266533384A2B),
  __SEGGER_RTL_U64_C(0x61A3666D124BB204),
  __SEGGER_RTL_U64_C(0x62B39508AA836D6F),
  __SEGGER_RTL_U64_C(0x63C6BA6455DCD8AE),
  __SEGGER_RTL_U64_C(0x64DCDEC3371793D1),
  __SEGGER_RTL_U64_C(0x65F60A7F79393E2E),
  __SEGGER_RTL_U64_C(0x6712460A8FC24072),
  __SEGGER_RTL_U64_C(0x683199ED779592CA),
  __SEGGER_RTL_U64_C(0x69540EC8F895722D),
  __SEGGER_RTL_U64_C(0x6A79AD55E7F6FD10),
  __SEGGER_RTL_U64_C(0x6BA27E656B4EB57A),
  __SEGGER_RTL_U64_C(0x6CCE8AE13C57EBDB),
  __SEGGER_RTL_U64_C(0x6DFDDBCBED791BAB),
  __SEGGER_RTL_U64_C(0x6F307A412F074892),
  __SEGGER_RTL_U64_C(0x70666F76154A7089),
  __SEGGER_RTL_U64_C(0x719FC4B95F452D29),
  __SEGGER_RTL_U64_C(0x72DC8373BE41A454),
  __SEGGER_RTL_U64_C(0x741CB5281E25EE34),
  __SEGGER_RTL_U64_C(0x75606373EE921C97),
  __SEGGER_RTL_U64_C(0x76A7980F6CCA15C2),
  __SEGGER_RTL_U64_C(0x77F25CCDEE6D7AE6),
  __SEGGER_RTL_U64_C(0x7940BB9E2CFFD89D),
  __SEGGER_RTL_U64_C(0x7A92BE8A92436616),
  __SEGGER_RTL_U64_C(0x7BE86FB985689DDC),
  __SEGGER_RTL_U64_C(0x7D41D96DB915019D),
  __SEGGER_RTL_U64_C(0x7E9F06067A4360BA),
};

static __SEGGER_RTL_RODATA __SEGGER_RTL_U32 __SEGGER_RTL_kahan_aT2[64] = {
  __SEGGER_RTL_U32_C(0x01500),
  __SEGGER_RTL_U32_C(0x02EF8),
  __SEGGER_RTL_U32_C(0x04D67),
  __SEGGER_RTL_U32_C(0x06B02),
  __SEGGER_RTL_U32_C(0x087BE),
  __SEGGER_RTL_U32_C(0x0A395),
  __SEGGER_RTL_U32_C(0x0BE7A),
  __SEGGER_RTL_U32_C(0x0D866),
  __SEGGER_RTL_U32_C(0x0F14A),
  __SEGGER_RTL_U32_C(0x1091B),
  __SEGGER_RTL_U32_C(0x11FCD),
  __SEGGER_RTL_U32_C(0x13552),
  __SEGGER_RTL_U32_C(0x14999),
  __SEGGER_RTL_U32_C(0x15C98),
  __SEGGER_RTL_U32_C(0x16E34),
  __SEGGER_RTL_U32_C(0x17E5F),
  __SEGGER_RTL_U32_C(0x18D03),
  __SEGGER_RTL_U32_C(0x19A01),
  __SEGGER_RTL_U32_C(0x1A545),
  __SEGGER_RTL_U32_C(0x1AE8A),
  __SEGGER_RTL_U32_C(0x1B5C4),
  __SEGGER_RTL_U32_C(0x1BB01),
  __SEGGER_RTL_U32_C(0x1BFDE),
  __SEGGER_RTL_U32_C(0x1C28D),
  __SEGGER_RTL_U32_C(0x1C2DE),
  __SEGGER_RTL_U32_C(0x1C0DB),
  __SEGGER_RTL_U32_C(0x1BA73),
  __SEGGER_RTL_U32_C(0x1B11C),
  __SEGGER_RTL_U32_C(0x1A4B5),
  __SEGGER_RTL_U32_C(0x1953D),
  __SEGGER_RTL_U32_C(0x18266),
  __SEGGER_RTL_U32_C(0x16BE0),
  __SEGGER_RTL_U32_C(0x1683E),
  __SEGGER_RTL_U32_C(0x179D8),
  __SEGGER_RTL_U32_C(0x18A4D),
  __SEGGER_RTL_U32_C(0x19992),
  __SEGGER_RTL_U32_C(0x1A789),
  __SEGGER_RTL_U32_C(0x1B445),
  __SEGGER_RTL_U32_C(0x1BF61),
  __SEGGER_RTL_U32_C(0x1C989),
  __SEGGER_RTL_U32_C(0x1D16D),
  __SEGGER_RTL_U32_C(0x1D77B),
  __SEGGER_RTL_U32_C(0x1DDDF),
  __SEGGER_RTL_U32_C(0x1E2AD),
  __SEGGER_RTL_U32_C(0x1E5BF),
  __SEGGER_RTL_U32_C(0x1E6E8),
  __SEGGER_RTL_U32_C(0x1E654),
  __SEGGER_RTL_U32_C(0x1E3CD),
  __SEGGER_RTL_U32_C(0x1DF2A),
  __SEGGER_RTL_U32_C(0x1D635),
  __SEGGER_RTL_U32_C(0x1CB16),
  __SEGGER_RTL_U32_C(0x1BE2C),
  __SEGGER_RTL_U32_C(0x1AE4E),
  __SEGGER_RTL_U32_C(0x19BDE),
  __SEGGER_RTL_U32_C(0x1868E),
  __SEGGER_RTL_U32_C(0x16E2E),
  __SEGGER_RTL_U32_C(0x1527F),
  __SEGGER_RTL_U32_C(0x1334A),
  __SEGGER_RTL_U32_C(0x11051),
  __SEGGER_RTL_U32_C(0x0E951),
  __SEGGER_RTL_U32_C(0x0BE01),
  __SEGGER_RTL_U32_C(0x08E0D),
  __SEGGER_RTL_U32_C(0x05924),
  __SEGGER_RTL_U32_C(0x01EDD)
};

static __SEGGER_RTL_RODATA struct {
  struct { double P[8]; } Poly;
} __SEGGER_RTL_float64_SinCos = {
  { { K_SINCOS_P_DBL } }
};

static __SEGGER_RTL_RODATA struct {
  struct { float P[5]; } Poly;
} __SEGGER_RTL_float32_SinCos = {
  { { K_SINCOS_P_FLT } }
};

static __SEGGER_RTL_RODATA struct {
  struct { float A[2]; float B[2]; } Quadrant;
  struct { float P[5]; float Q[5]; } Poly;
} __SEGGER_RTL_float32_ASinACos = {
  { {          0.0f, (float)M_PI_4 },
    { (float)M_PI_2, (float)M_PI_4 } },
  { { K_ASINACOS_P_FLT },
    { K_ASINACOS_Q_FLT } },
};

static __SEGGER_RTL_RODATA struct {
  double P[4];
  double Q[3];
} __SEGGER_RTL_float64_Sinh = {
  { K_SINH_P_DBL },
  { K_SINH_Q_DBL }
};

static __SEGGER_RTL_RODATA struct {
  struct { double A[2]; double B[2]; } Quadrant;
  struct { double P[5]; double Q[5]; } Poly;
} __SEGGER_RTL_float64_ASinACos = {
  { {    0.0, M_PI_4 },
    { M_PI_2, M_PI_4 } },
  { { K_ASINACOS_P_DBL },
    { K_ASINACOS_Q_DBL } }
};

static __SEGGER_RTL_RODATA struct {
  struct { double P[2]; double Q[3]; } Poly;
} __SEGGER_RTL_float64_Log = {
  { { K_LOG_P_DBL },
    { K_LOG_Q_DBL } }
};

static __SEGGER_RTL_RODATA struct {
  struct { double P[3]; double Q[4]; } Poly;
} __SEGGER_RTL_float64_Tan = {
  { { K_TAN_P_DBL },
    { K_TAN_Q_DBL } }
};

static __SEGGER_RTL_RODATA struct {
  struct { float P[3]; float Q[4]; } Poly;
} __SEGGER_RTL_float32_Tan = {
  { { K_TAN_P_FLT },
    { K_TAN_Q_FLT } }
};

static __SEGGER_RTL_RODATA struct {
  struct { double A[4];              } Quadrant;
  struct { double P[4]; double Q[4]; } Poly;
} __SEGGER_RTL_float64_ATan = {
  { { 0.0, M_PI_6, M_PI_2, M_PI_3 } },
  { { K_ATAN_P_DBL },
    { K_ATAN_Q_DBL } }
};

static __SEGGER_RTL_RODATA struct {
  struct { float A[4];             } Quadrant;
  struct { float P[4]; float Q[4]; } Poly;
} __SEGGER_RTL_float32_ATan = {
  { { 0.0, (float)M_PI_6, (float)M_PI_2, (float)M_PI_3 } },
  { { K_ATAN_P_FLT },
    { K_ATAN_Q_FLT } }
};

static __SEGGER_RTL_RODATA struct {
  struct { double P[3]; double Q[3]; } Poly;
} __SEGGER_RTL_float64_Tanh = {
  { { K_TANH_P_DBL },
    { K_TANH_Q_DBL } }
};

static __SEGGER_RTL_RODATA struct {
  struct { float P[3]; float Q[3]; } Poly;
} __SEGGER_RTL_float32_Tanh = {
  { { K_TANH_P_FLT },
    { K_TANH_Q_FLT } }
};

//
// Calculated using Mathematica:
//    BaseForm[NumberForm[N[2^(x/8)*2^30, 10], ExponentFunction -> (Null &)], 16]
// for x from -4 to +4.
//
static __SEGGER_RTL_RODATA __SEGGER_RTL_U32 __SEGGER_RTL_float32_Exp_std_2tojby8[] = {
  __SEGGER_RTL_U32_C(0x2D413CCD),  // -4
  __SEGGER_RTL_U32_C(0x3159CA84),  // -3
  __SEGGER_RTL_U32_C(0x35D13F33),  // -2
  __SEGGER_RTL_U32_C(0x3AB031BA),  // -1
  __SEGGER_RTL_U32_C(0x40000000),  //  0
  __SEGGER_RTL_U32_C(0x45CAE0F2),  // +1
  __SEGGER_RTL_U32_C(0x4C1BF829),  // +2
  __SEGGER_RTL_U32_C(0x52FF6B55),  // +3
  __SEGGER_RTL_U32_C(0x5A82799A),  // +4   
};

//
// Calculated using Mathematica:
//    BaseForm[NumberForm[N[2^(x/8)*2^30, 10], ExponentFunction -> (Null &)], 16]
// for x from -4 to +3.
//
// As above but we permute this table so that it can be indexed directly without
// adding 4 to the index when it is convenient to use the low 3 bits of the index
// directly.
//
static __SEGGER_RTL_RODATA __SEGGER_RTL_U32 __SEGGER_RTL_float32_Exp_rot_twojby8[] = {
  __SEGGER_RTL_U32_C(0x40000000),  //  0   0b000
  __SEGGER_RTL_U32_C(0x45CAE0F2),  // +1   0b001
  __SEGGER_RTL_U32_C(0x4C1BF829),  // +2   0b010
  __SEGGER_RTL_U32_C(0x52FF6B55),  // +3   0b011
  __SEGGER_RTL_U32_C(0x2D413CCD),  // -4   0b100
  __SEGGER_RTL_U32_C(0x3159CA84),  // -3   0b101
  __SEGGER_RTL_U32_C(0x35D13F33),  // -2   0b110
  __SEGGER_RTL_U32_C(0x3AB031BA),  // -1   0b111
};

// These are, in fact, signed but easier to read in hex.
static __SEGGER_RTL_RODATA __SEGGER_RTL_U8 __SEGGER_RTL_float32_tanh_Reciprocal[] = {
  0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
  0xC8, 0xC8, 0xC9, 0xCA, 0xCA, 0xCB, 0xCC, 0xCC,
  0xCD, 0xCE, 0xCE, 0xCF, 0xD0, 0xD0, 0xD1, 0xD1,
  0xD2, 0xD2, 0xD3, 0xD3, 0xD4, 0xD4, 0xD5, 0xD5,
  0xD6, 0xD6, 0xD6, 0xD7, 0xD7, 0xD8, 0xD8, 0xD8,
  0xD9, 0xD9, 0xDA, 0xDA, 0xDA, 0xDB, 0xDB, 0xDB,
  0xDC, 0xDC, 0xDC, 0xDD, 0xDD, 0xDD, 0xDD, 0xDE,
  0xDE, 0xDE, 0xDF, 0xDF, 0xDF, 0xDF, 0xE0, 0xE0,
};

/*********************************************************************
*
*       Static code - support functions
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_Div64by32_Moeller()
*
*  Function description
*    64-bit by 32-bit divide, specialized.
*
*  Parameters
*    u1 - High 31 bits of dividend, 0...0x7FFFFFFFF; low 32 bits are zero.
*    d  - 32 bits of divisor, normalized such that bit 31 is 1.
*
*  Return value
*     u1 * 2^32 / v.
*
*  Additional information
*    See [MOELLER].
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div64by32_Moeller(__SEGGER_RTL_U32 u1, __SEGGER_RTL_U32 d) {
  __SEGGER_RTL_U32 q0, q1;
  __SEGGER_RTL_U32 p0, p1;
  __SEGGER_RTL_U32 d0;
  __SEGGER_RTL_U32 v0, v1, v2;
  __SEGGER_RTL_U32 e;
  __SEGGER_RTL_U32 r;
  //
  // Calculate reciprocal.
  //
  d0 = d & 1;
  v0 = ((1uL << 24) - (1uL<<14) + (1uL << 9)) / (d >> 22);           // 15 bits; we use division rather than a LUT as suggested by Moeller
  v1 = (v0 << 4) - __SEGGER_RTL_UMULL_HI(v0*v0, (d >> 11) + 1) - 1;  // 18 bits
  e  = 0u - (v1 * ((d >> 1) + d0));
  if (d0) {
    e += v1 >> 1;
  } 
  v2 = (v1 << 15) + (__SEGGER_RTL_UMULL_HI(v1, e) >> 1);
  //
  p0 = d;
  p1 = d;
  __SEGGER_RTL_UMLAL(p0, p1, v2, d);
  //
  // Reciprocal is v2-p1.  Now compute quotient estimate.
  //
  __SEGGER_RTL_UMULL(q0, q1, v2 - p1, u1);
  q1 += u1 + 1;
  //
  // Adjust quotient estimate to find true quotient.
  //
  r = 0u - q1*d;
  if (r > q0) {
    q1 -= 1;
    r  += d;
  }
  if (r >= d) {
    q1 += 1;
  //r  -= d;       // remainder is not required, but this is how to adjust it
  }
  return q1;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div64by32_Warren()
*
*  Function description
*    64-bit by 32-bit divide, specialized.
*
*  Parameters
*    u1 - High 31 bits of dividend, 0...0x7FFFFFFFF; low 32 bits are zero.
*    v  - 32 bits of divisor, normalized such that bits 31 is 1.
*
*  Return value
*     u1 * 2^32 / v.
*
*  Additional information
*    This is adapted from [WARREN].
*
*  Notes
*    Kept here for now, but is inferior to [MOELLER].  Deprecated.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div64by32_Warren(__SEGGER_RTL_U32 u1, __SEGGER_RTL_U32 v) {
#if __SEGGER_RTL_OPTIMIZE < 0
  //
  // Use built-in 64-bit division capability.  This is usually slower than
  // using the special-case division below, but also smaller.
  //
  return (((__SEGGER_RTL_U64)u1) << 32) / v;
  //
#else
  //
  // Use synthesized division capability.
  //
  __SEGGER_RTL_U32 base = 65536;       // Number base (16 bits).
  __SEGGER_RTL_U32 vn1, vn0;           // Normalized divisor digits.
  __SEGGER_RTL_U32 q1, q0;             // Quotient digits.
  __SEGGER_RTL_U32 un21;               // Dividend digits.
  __SEGGER_RTL_U32 rhat;               // Remainder estimate.
  //
  // Break divisor into two 16-bit digits.
  //
  vn1 = v >> 16;
  vn0 = v & 0xFFFF;
  //
  // Compute the first quotient digit, q1.
  //
  __SEGGER_RTL_DIVMOD_U32(q1, rhat, u1, vn1);
  for (;;) {
    if (q1 >= base || q1*vn0 > base*rhat) {
      q1   -= 1;
      rhat += vn1;
      if (rhat >= base) {
        break;
      }
    } else {
      break;
    }
  }
  //
  // Compute the second quotient digit, q0.
  //
  un21 = u1*base - q1*v;
  __SEGGER_RTL_DIVMOD_U32(q0, rhat, un21, vn1);
  //
  for (;;) {
    if (q0 >= base || q0*vn0 > base*rhat) {
      q0   -= 1;
      rhat += vn1;
      if (rhat >= base) {
        break;
      }
    } else {
      break;
    }
  }
  //
  // Return quotient.
  //
  return q1*base + q0;
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_Div128by64()
*
*  Function description
*    128-bit by 64-bit divide, specialized.
*
*  Parameters
*    u1 - High 64 bits of dividend; low 64 bits are zero.
*    v  - 64 bits of divisor, normalized such that bit 63 is 1.
*
*  Return value
*    u1 * 2^64 / v.
*
*  Additional information
*    This is adapted from [WARREN].
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div128by64(__SEGGER_RTL_U64 u1, __SEGGER_RTL_U64 v) {
  __SEGGER_RTL_U64 base = __SEGGER_RTL_U64_C(1) << 32;  // Number base (32 bits).
  __SEGGER_RTL_U64 vn1, vn0;                            // Normalized divisor digits.
  __SEGGER_RTL_U64 q1, q0;                              // Quotient digits.
  __SEGGER_RTL_U64 un21;                                // Dividend digits.
  __SEGGER_RTL_U64 rhat;                                // Remainder estimate.
  //
  // Break divisor into two 32-bit digits.
  //
  vn1 = __SEGGER_RTL_U64_H(v);
  vn0 = __SEGGER_RTL_U64_L(v);
  //
  // Compute the first quotient digit, q1.
  //
  __SEGGER_RTL_DIVMOD_U64(q1, rhat, u1, vn1);
  //
  do {
    if (q1 < base && q1*vn0 <= base*rhat) {
      break;
    }
    q1   -= 1;
    rhat += vn1;
  } while (rhat < base);
  //
  // Compute the second quotient digit, q0.
  //
  un21 = u1*base - q1*v;
  __SEGGER_RTL_DIVMOD_U64(q0, rhat, un21, vn1);
  //
  do {
    if (q0 < base && q0*vn0 <= base*rhat) {
      break;
    }
    q0   -= 1;
    rhat += vn1;
  } while (rhat < base);
  //
  // Return quotient.
  //
  return q1*base + q0;
}

/*********************************************************************
*
*       __SEGGER_RTL_SquareHi_U64()
*
*  Function description
*    64x64 to 64-high multiply.
*
*  Parameters
*    x - Value to square.
*
*  Return value
*    x*x >> 64, i.e. the high 64 bits of x^2.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_SquareHi_U64(__SEGGER_RTL_U64 x) {
  __SEGGER_RTL_U64 hh;
  __SEGGER_RTL_U32 hl;
  //
  hh = __SEGGER_RTL_UMULL_X (__SEGGER_RTL_U64_H(x), __SEGGER_RTL_U64_H(x));
  hl = __SEGGER_RTL_UMULL_HI(__SEGGER_RTL_U64_H(x), __SEGGER_RTL_U64_L(x));
  //
  return hh + hl + hl;
}

#if defined(__SEGGER_RTL_FLOAT16)
/*********************************************************************
*
*       __SEGGER_RTL_BitcastToU16_inline()
*
*  Function description
*    Bitcast 16-bit float to 16-bit integer.
*
*  Parameters
*    x - Value to convert.
*
*  Return value
*    Binary representation of x.
*  
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U16 __SEGGER_RTL_BitcastToU16_inline(__SEGGER_RTL_FLOAT16 x) {
  SEGGER_RTL_float16_t xx;
  //
  xx.f = x;
  return xx.l;
}
#endif

#if defined(__SEGGER_RTL_FLOAT16)
/*********************************************************************
*
*       __SEGGER_RTL_BitcastToF16_inline()
*
*  Function description
*    Bitcast 32-bit float to 32-bit integer.
*
*  Parameters
*    x - Value to convert.
*
*  Return value
*    Floating representation of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_FLOAT16 __SEGGER_RTL_BitcastToF16_inline(__SEGGER_RTL_U16 x) {
  SEGGER_RTL_float16_t xx;
  //
  xx.l = x;
  return xx.f;
}
#endif

/*********************************************************************
*
*       __SEGGER_RTL_BitcastToU32_inline()
*
*  Function description
*    Bitcast 32-bit float to 32-bit integer.
*
*  Parameters
*    x - Value to convert.
*
*  Return value
*    Binary representation of x.
*  
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_BitcastToU32_inline(float x) {
  SEGGER_RTL_float32_t xx;
  //
  xx.f = x;
  return xx.l;
}

/*********************************************************************
*
*       __SEGGER_RTL_BitcastToF32_inline()
*
*  Function description
*    Bitcast 32-bit float to 32-bit integer.
*
*  Parameters
*    x - Value to convert.
*
*  Return value
*    Floating representation of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_BitcastToF32_inline(__SEGGER_RTL_U32 x) {
  SEGGER_RTL_float32_t xx;
  //
  xx.l = x;
  return xx.f;
}

/*********************************************************************
*
*       __SEGGER_RTL_BitcastToU64_inline()
*
*  Function description
*    Bitcast 64-bit float to 64-bit integer.
*
*  Parameters
*    x - Value to convert.
*
*  Return value
*    Floating representation of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_BitcastToU64_inline(double x) {
  SEGGER_RTL_float64_t xx;
  //
  xx.f = x;
  return xx.l;
}

/*********************************************************************
*
*       __SEGGER_RTL_BitcastToF64_inline()
*
*  Function description
*    Bitcast 64-bit integer to 64-bit float.
*
*  Parameters
*    x - Value to convert.
*
*  Return value
*    Floating representation of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_BitcastToF64_inline(__SEGGER_RTL_U64 x) {
  SEGGER_RTL_float64_t xx;
  //
  xx.l = x;
  return xx.f;
}

/*********************************************************************
*
*       Static code - classification functions
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_float32_isspecial_soft()
*
*  Function description
*    Is value an operand that deserves special processing?
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    == 0 - Operand does not require special processing.
*    != 0 - Operand requires special processing.
*
*  Additional information
*    Special processing in this context means that the operand is
*    an infinity or a NaN, or is a zero or subnormal, ignoring the
*    sign bit.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isspecial_soft(__SEGGER_RTL_U32 x) {
  return (x << 1 >> 24) - 1u >= 0xFE;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isspecial()
*
*  Function description
*    Is value an operand that deserves special processing?
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    == 0 - Operand does not require special processing.
*    != 0 - Operand requires special processing.
*
*  Additional information
*    Special processing in this context means that the operand is
*    an infinity or a NaN, or is a zero or subnormal, ignoring the
*    sign bit.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isspecial(float x) {
#if defined(__SEGGER_RTL_FLOAT32_ISSPECIAL)
  return __SEGGER_RTL_FLOAT32_ISSPECIAL(x);
#else
  return __SEGGER_RTL_float32_isspecial_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isspecial_or_negative_soft()
*
*  Function description
*    Is value an operand that deserves special processing?
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    == 0 - Operand does not require special processing.
*    != 0 - Operand requires special processing.
*
*  Additional information
*    Special processing in this context means that the operand is
*    an infinity or a NaN, or is a zero or subnormal, ignoring the
*    sign bit.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isspecial_or_negative_soft(__SEGGER_RTL_U32 x) {
  return (__SEGGER_RTL_UINT_LEAST16_T)(x >> 23) - 1u >= 0xFE;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isspecial_or_negative()
*
*  Function description
*    Is value an operand that deserves special processing?
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    == 0 - Operand does not require special processing.
*    != 0 - Operand requires special processing.
*
*  Additional information
*    Special processing in this context means that the operand is
*    an infinity or a NaN, or is a zero or subnormal, ignoring the
*    sign bit.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isspecial_or_negative(float x) {
#if defined(__SEGGER_RTL_FLOAT32_ISSPECIAL_OR_NEGATIVE)
  return __SEGGER_RTL_FLOAT32_ISSPECIAL_OR_NEGATIVE(x);
#else
  return __SEGGER_RTL_float32_isspecial_or_negative_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isspecial_soft()
*
*  Function description
*    Is value an operand that deserves special processing?
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    == 0 - Operand does not require special processing.
*    != 0 - Operand requires special processing.
*
*  Additional information
*    Special processing in this context means that the operand is
*    an infinity or a NaN, or is a zero or subnormal, ignoring the
*    sign bit.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isspecial_soft(__SEGGER_RTL_U64 x) {
  return ((__SEGGER_RTL_UINT_LEAST16_T)(x >> 52) & 0x7FFu) - 1u >= 0x7FEu;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isspecial()
*
*  Function description
*    Is value an operand that deserves special processing?
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    == 0 - Operand does not require special processing.
*    != 0 - Operand requires special processing.
*
*  Additional information
*    Special processing in this context means that the operand is
*    an infinity or a NaN, or is a zero or subnormal, ignoring the
*    sign bit.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isspecial(double x) {
#if defined(__SEGGER_RTL_FLOAT64_ISSPECIAL)
  return __SEGGER_RTL_FLOAT64_ISSPECIAL(x);
#else
  return __SEGGER_RTL_float64_isspecial_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isspecial_or_negative()
*
*  Function description
*    Is value an operand that deserves special processing?
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    == 0 - Operand does not require special processing.
*    != 0 - Operand requires special processing.
*
*  Additional information
*    Special processing in this context means that the operand is
*    an infinity or a NaN, or is a zero or subnormal, or is negative
*    according to the sign bit.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isspecial_or_negative(double x) {
#if defined(__SEGGER_RTL_FLOAT64_ISSPECIAL_OR_NEGATIVE)
  return __SEGGER_RTL_FLOAT64_ISSPECIAL_OR_NEGATIVE(x);
#else
  return (__SEGGER_RTL_UINT_LEAST16_T)(__SEGGER_RTL_BitcastToU64(x) >> 52) - 1u >= 0x7FEu;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isinf_soft()
*
*  Function description
*    Infinity query, float.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not infinite.
*    != 0 - Is infinite.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isinf_soft(__SEGGER_RTL_U32 x) {
  return (x << 1) == __SEGGER_RTL_X2(K_INF_U32);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isinf_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isinf_inline(float x) {
#if defined(__SEGGER_RTL_FLOAT32_ISINF)
  return __SEGGER_RTL_FLOAT32_ISINF(x);
#else
  return __SEGGER_RTL_float32_isinf_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isposinf_soft()
*
*  Function description
*    Infinity query, float.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not positive infinity.
*    != 0 - Is positive infinity.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isposinf_soft(__SEGGER_RTL_U32 x) {
  return x == K_INF_U32;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isposinf_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isposinf_inline(float x) {
#if defined(__SEGGER_RTL_FLOAT32_ISPOSINF)
  return __SEGGER_RTL_FLOAT32_ISPOSINF(x);
#else
  return __SEGGER_RTL_float32_isposinf_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_iszero_inline()
*
*  Function description
*    Zero query, signless, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not zero.
*    != 0 - Is zero.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_iszero_inline(float x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == 0;
#else
  return (__SEGGER_RTL_BitcastToU32(x) << 1) == 0;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_putative_iszero_soft()
*
*  Function description
*    Putative zero query, signless, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Additional information
*    A putative zero is a value that the library (or hardware) considers
*    as zero.  For VFP in RunFast mode, subnormal inputs are flushed to
*    zero.
*
*  Return value
*    == 0 - Is not a putative zero.
*    != 0 - Is a putative zero.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_putative_iszero_soft(__SEGGER_RTL_U32 x) {
#if __SEGGER_RTL_TYPESET == 16
  return FLOAT32_EXPONENT(x) == 0;
#else
  return (x << 1) < __SEGGER_RTL_U32_C(0x01000000);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_putative_iszero_soft()
*
*  Function description
*    Putative zero query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Additional information
*    A putative zero is a value that the library (or hardware) considers
*    as zero.  For VFP in RunFast mode, subnormal inputs are flushed to
*    zero.
*
*  Return value
*    == 0 - Is not a putative zero.
*    != 0 - Is a putative zero.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_putative_iszero_soft(__SEGGER_RTL_U64 x) {
#if __SEGGER_RTL_TYPESET == 16
  return FLOAT64_EXPONENT(x) == 0;
#else
  return (x << 1) < __SEGGER_RTL_U64_C(0x0020000000000000);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_putative_iszero()
*
*  Function description
*    Putative zero query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Additional information
*    A putative zero is a value that the library (or hardware) considers
*    as zero.  For VFP in RunFast mode, subnormal inputs are flushed to
*    zero.
*
*    When compiled for a target that has full IEEE conformance, this
*    function returns true only when the input is true IEEE zero.
*
*  Return value
*    == 0 - Is not a putative zero.
*    != 0 - Is a putative zero.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_putative_iszero(float x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == 0;
#else
  return (__SEGGER_RTL_BitcastToU32(x) << 1) < __SEGGER_RTL_U32_C(0x01000000);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_putative_iszero()
*
*  Function description
*    Putative zero query, putative, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Additional information
*    A putative zero is a value that the library (or hardware) considers
*    as zero.  For VFP in RunFast mode, subnormal inputs are flushed to
*    zero.
*
*    When compiled for a target that has full IEEE conformance, this
*    function returns true only when the input is true IEEE zero.
*
*  Return value
*    == 0 - Is not a putative zero.
*    != 0 - Is a putative zero.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_putative_iszero(double x) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == 0;
#else
  return __SEGGER_RTL_float64_putative_iszero_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_exact_iszero()
*
*  Function description
*    Putative zero query, exact, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Additional information
*    An exact zero is matches the bit pattern for +0 or -0.
*
*  Return value
*    == 0 - Is not an exact zero.
*    != 0 - Is an exact zero.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_exact_iszero(float x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == 0;
#else
  return (__SEGGER_RTL_BitcastToU32(x) << 1) == 0;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_exact_iszero()
*
*  Function description
*    Putative zero query, exact, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Additional information
*    An exact zero is matches the bit pattern for +0 or -0.
*
*  Return value
*    == 0 - Is not an exact zero.
*    != 0 - Is an exact zero.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_exact_iszero(double x) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == 0;
#else
  return (__SEGGER_RTL_BitcastToU64(x) << 1) == 0;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isnan_soft()
*
*  Function description
*    NaN query, float.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not a NaN.
*    != 0 - Is a NaN.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isnan_soft(__SEGGER_RTL_U32 x) {
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
  return (x << 1) > __SEGGER_RTL_U32_C(0xFF000000);
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_FAST
  return (x << 1) > __SEGGER_RTL_U32_C(0xFF000000);
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_COMPACT
  return (__SEGGER_RTL_U16)((__SEGGER_RTL_U16)(x >> 16) << 1) > 0xFF00u;
#else
  #error should not happen: misconfigured __SEGGER_RTL_NAN_FORMAT
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isnan_inline()
*
*  Function description
*    NaN query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not a NaN.
*    != 0 - Is a NaN.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isnan_inline(float x) {
#if defined(__SEGGER_RTL_FLOAT32_ISNAN)
  return __SEGGER_RTL_FLOAT32_ISNAN(x);
#elif __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x != x;
#else
  return __SEGGER_RTL_float32_isnan_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isfinite_soft()
*
*  Function description
*    Finite query, float.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not finite.
*    != 0 - Is finite.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_isfinite_soft(__SEGGER_RTL_U32 x) {
  return (x << 1) < __SEGGER_RTL_U32_C(0xFF000000);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isfinite_inline()
*
*  Parameters
*    x - Value to test as a float.
*
*  Return value
*    == 0 - Not finite.
*    != 0 - Is finite.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isfinite_inline(float x) {
#if defined(__SEGGER_RTL_FLOAT32_ISFINITE)
  return __SEGGER_RTL_FLOAT32_ISFINITE(x);
#else
  return __SEGGER_RTL_float32_isfinite_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isnormal_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isnormal_inline(float x) {
#if defined(__SEGGER_RTL_FLOAT32_ISNORMAL)
  return __SEGGER_RTL_FLOAT32_ISNORMAL(x);
#else
  return (__SEGGER_RTL_BitcastToU32(x) << 1) <  __SEGGER_RTL_U32_C(0xFF000000) &&
         (__SEGGER_RTL_BitcastToU32(x) << 1) >= __SEGGER_RTL_U32_C(0x01000000);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lt0_true()
*
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_lt0_true(float x) {
#if __SEGGER_RTL_FP_HW >= 1
  return x < 0;
#else
  return !__SEGGER_RTL_float32_iszero_inline(x) && (__SEGGER_RTL_BitcastToU32(x) & FLOAT32_SIGN_MASK);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lt0_nonzero_finite()
*
*  Function description
*    Test less than zero, finite input.
*
*  Parameters
*    x - Value to test as a float.  This value is known to be finite
*        and nonzero.
*
*  Additional information
*    The nonzero requirement is important as for floating instructions
*    that conform to the IEEE standard, +0 and -0 both compare equal
*    to zero and never compare less than zero.  In essence this function
*    is a specialist sign-test that works efficiently on floating-point
*    hardware and in software.
*
*  Return value
*    == 0 - Not less than zero.
*    != 0 - Less than zero.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_lt0_nonzero_finite(float x) {
#if __SEGGER_RTL_FP_HW >= 1
  return x < 0;
#else
  return __SEGGER_RTL_BitcastToU32(x) & FLOAT32_SIGN_MASK;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lt0_nonzero_finite()
*
*  Function description
*    Test less than zero, finite input.
*
*  Parameters
*    x - Value to test as a float.  This value is known to be finite
*        and nonzero.
*
*  Additional information
*    The nonzero requirement is important as for floating instructions
*    that conform to the IEEE standard, +0 and -0 both compare equal
*    to zero and never compare less than zero.  In essence this function
*    is a specialist sign-test that works efficiently on floating-point
*    hardware and in software.
*
*  Return value
*    == 0 - Not less than zero.
*    != 0 - Less than zero.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_lt0_nonzero_finite(double x) {
#if __SEGGER_RTL_FP_HW >= 2
  return x < 0;
#else
  return (__SEGGER_RTL_BitcastToU64(x) & FLOAT64_SIGN_MASK) != 0;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isinf_soft()
*
*  Function description
*    Infinity query, double.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not infinite.
*    != 0 - Is infinite.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isinf_soft(__SEGGER_RTL_U64 x) {
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
  //
  #if 1
    //
    // On some compilers and processors, this version is faster than a shift
    // because it doesn't need a temporary to compute the shifted value.  The
    // comparison of two values here causes GCC to throw away the sign bit
    // and compare the remaining 31 bits to +Inf, which is a neat trick.
    //
    return (x == K_INF_U64) || (x == K_MINUS_INF_U64);
  #else
    return (x << 1) == __SEGGER_RTL_U64_C(0xFFE0000000000000);
  #endif
  //
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_FAST
  return (__SEGGER_RTL_U64_H(x) << 1) == __SEGGER_RTL_U32_C(0xFFE00000);
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_COMPACT
  return (__SEGGER_RTL_U16)((__SEGGER_RTL_U16)(__SEGGER_RTL_U64_H(x) >> 16) << 1) == (__SEGGER_RTL_U16)0xFFE0;
#else
  #error should not happen: misconfigured __SEGGER_RTL_NAN_FORMAT
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isinf_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isinf_inline(double x) {
#if defined(__SEGGER_RTL_FLOAT64_ISINF)
  return __SEGGER_RTL_FLOAT64_ISINF(x);
#else
  return __SEGGER_RTL_float64_isinf_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isposinf_soft()
*
*  Function description
*    Positive infinity query, double.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not positive infinity.
*    != 0 - Is positive infinity.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isposinf_soft(__SEGGER_RTL_U64 x) {
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
  return x == K_INF_U64;
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_FAST
  return __SEGGER_RTL_U64_H(x) == __SEGGER_RTL_U64_H(K_INF_U64);
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_COMPACT
  return (__SEGGER_RTL_U16)(__SEGGER_RTL_U64_H(x) >> 16) == (__SEGGER_RTL_U16)(__SEGGER_RTL_U64_H(K_INF_U64) >> 16);
#else
  #error should not happen: misconfigured __SEGGER_RTL_NAN_FORMAT
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isposinf_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isposinf_inline(double x) {
#if defined(__SEGGER_RTL_FLOAT64_ISPOSINF)
  return __SEGGER_RTL_FLOAT64_ISPOSINF(x);
#else
  return __SEGGER_RTL_float64_isposinf_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isneginf_soft()
*
*  Function description
*    Negative infinity query, float.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not positive infinity.
*    != 0 - Is positive infinity.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isneginf_soft(__SEGGER_RTL_U32 x) {
  return x == K_MINUS_INF_U32;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isneginf_soft()
*
*  Function description
*    Negative infinity query, double.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not positive infinity.
*    != 0 - Is positive infinity.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isneginf_soft(__SEGGER_RTL_U64 x) {
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
  return x == K_MINUS_INF_U64;
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_FAST
  return __SEGGER_RTL_U64_H(x) == __SEGGER_RTL_U64_H(K_MINUS_INF_U64);
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_COMPACT
  return (__SEGGER_RTL_U16)(__SEGGER_RTL_U64_H(x) >> 16) == (__SEGGER_RTL_U16)(__SEGGER_RTL_U64_H(K_MINUS_INF_U64) >> 16);
#else
  #error should not happen: misconfigured __SEGGER_RTL_NAN_FORMAT
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isneginf_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_isneginf_inline(float x) {
#if defined(__SEGGER_RTL_FLOAT32_ISNEGINF)
  return __SEGGER_RTL_FLOAT32_ISNEGINF(x);
#else
  return __SEGGER_RTL_float32_isneginf_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isneginf_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isneginf_inline(double x) {
#if defined(__SEGGER_RTL_FLOAT64_ISNEGINF)
  return __SEGGER_RTL_FLOAT64_ISNEGINF(x);
#else
  return __SEGGER_RTL_float64_isneginf_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_iszero_inline()
*
*  Function description
*    Zero query, double.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not zero.
*    != 0 - Is zero.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_iszero_inline(double x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == 0;
#else
  return (__SEGGER_RTL_BitcastToU64(x) << 1) == 0;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isnan_soft()
*
*  Function description
*    NaN query, double.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not a NaN.
*    != 0 - Is a NaN.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isnan_soft(__SEGGER_RTL_U64 x) {
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
  return (x << 1) > __SEGGER_RTL_U64_C(0xFFE0000000000000);
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_FAST
  return (__SEGGER_RTL_U64_H(x) << 1) > __SEGGER_RTL_U32_C(0xFFE00000);
#elif __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_COMPACT
  return (__SEGGER_RTL_U16)((__SEGGER_RTL_U16)(__SEGGER_RTL_U64_H(x) >> 16) << 1) > (__SEGGER_RTL_U16)(0xFFE0u);
#else
  #error should not happen: misconfigured __SEGGER_RTL_NAN_FORMAT
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isnan_inline()
*
*  Function description
*    NaN query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Not NaN.
*    != 0 - Is NaN.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isnan_inline(double x) {
#if defined(__SEGGER_RTL_FLOAT64_ISNAN)
  return __SEGGER_RTL_FLOAT64_ISNAN(x);
#elif __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x != x;
#else
  return __SEGGER_RTL_float64_isnan_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isfinite_soft()
*
*  Function description
*    Finite query, double.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    == 0 - Not finite.
*    != 0 - Is finite.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_isfinite_soft(__SEGGER_RTL_U64 x) {
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
  return (x << 1) < __SEGGER_RTL_U64_C(0xFFE0000000000000);
#else
  return (__SEGGER_RTL_U64_H(x) << 1) < __SEGGER_RTL_U32_C(0xFFE00000);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isfinite_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isfinite_inline(double x) {
#if defined(__SEGGER_RTL_FLOAT64_ISFINITE)
  return __SEGGER_RTL_FLOAT64_ISFINITE(x);
#else
  return __SEGGER_RTL_float64_isfinite_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isnormal_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_isnormal_inline(double x) {
#if defined(__SEGGER_RTL_FLOAT64_ISNORMAL)
  return __SEGGER_RTL_FLOAT64_ISNORMAL(x);
#else
  return (__SEGGER_RTL_BitcastToU64(x) << 1) <  __SEGGER_RTL_U64_C(0xFFE0000000000000) &&
         (__SEGGER_RTL_BitcastToU64(x) << 1) >= __SEGGER_RTL_U64_C(0x0020000000000000);
#endif
}

/*********************************************************************
*
*       Static code - subnormal and normalization handling
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_float32_subnormal_flush_soft()
*
*  Function description
*    Flush subnormals to zero.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    Flushed value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_subnormal_flush_soft(__SEGGER_RTL_U32 x) {
  return x & FLOAT32_SIGN_MASK;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_subnormal_flush_soft()
*
*  Function description
*    Flush subnormals to zero.
*
*  Parameters
*    x - Value to test as bitstring.
*
*  Return value
*    Flushed value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float64_subnormal_flush_soft(__SEGGER_RTL_U64 x) {
  return x & FLOAT64_SIGN_MASK;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_subnormal_flush()
*
*  Function description
*    Flush subnormals to zero.
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    Flushed value.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_subnormal_flush(float x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x;
#else
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_float32_subnormal_flush_soft(__SEGGER_RTL_BitcastToU32(x)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_subnormal_flush()
*
*  Function description
*    Flush subnormals to zero.
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    Flushed value.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_subnormal_flush(double x) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x;
#else
  return __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_float64_subnormal_flush_soft(__SEGGER_RTL_BitcastToU64(x)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_conditional_subnormal_flush_soft()
*
*  Function description
*    Flush subnormals to zero.
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    Flushed value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_conditional_subnormal_flush_soft(__SEGGER_RTL_U32 x) {
  if (__SEGGER_RTL_float32_putative_iszero_soft(x)) {
    return __SEGGER_RTL_float32_subnormal_flush_soft(x);
  } else {
    return x;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_conditional_subnormal_flush_soft()
*
*  Function description
*    Flush subnormals to zero.
*
*  Parameters
*    x - Value to test.
*
*  Return value
*    Flushed value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float64_conditional_subnormal_flush_soft(__SEGGER_RTL_U64 x) {
  if (__SEGGER_RTL_float64_putative_iszero_soft(x)) {
    return __SEGGER_RTL_float64_subnormal_flush_soft(x);
  } else {
    return x;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_normalize()
*
*  Function description
*    Normalize 64-bit integer part of double.
*
*  Parameters
*    pI - Pointer to scaled integer to normalize which must be nonzero.
*    q0 - Current scaling value of scaled integer.
*
*  Return value
*    New scaling factor of normalized scaled integer.
*
*  Additional information
*    The scaled integer is normalized to have bit 53 set.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_normalize(__SEGGER_RTL_U64 *pI, int q0) {
  //
#if defined(__SEGGER_RTL_CLZ_U32) && !defined(__SEGGER_RTL_CLZ_U32_SYNTHESIZED)
  //
  __SEGGER_RTL_U64 i0;
  unsigned         q1;
  //
  i0 = *pI;
  //
  q1 = __SEGGER_RTL_CLZ_U64(i0) - 11;
  i0 <<= q1;
  q0  -= q1;
  //
#else
  //
  __SEGGER_RTL_U64 i0;
  //
  i0 = *pI;
  //
  while (i0 < __SEGGER_RTL_U64_C(0x0010000000000000)) {
    i0 <<= 1;
    q0  -= 1;
  }
  //
#endif
  //
  *pI = i0;
  //
  return q0;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_normalize()
*
*  Function description
*    Normalize 32-bit integer part of float.
*
*  Parameters
*    pI - Pointer to scaled integer to normalize which must be nonzero.
*    q0 - Current scaling value of scaled integer.
*
*  Return value
*    New scaling factor of normalized scaled integer.
*
*  Additional information
*    The scaled integer is normalized to have bit 24 set.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_normalize(__SEGGER_RTL_U32 *pI, int q0) {
  //
#if defined(__SEGGER_RTL_CLZ_U32) && !defined(__SEGGER_RTL_CLZ_U32_SYNTHESIZED)
  //
  __SEGGER_RTL_U32 i0;
  unsigned q1;
  //
  i0 = *pI;
  //
  q1 = __SEGGER_RTL_CLZ_U32(i0) - 8;
  i0 <<= q1;
  q0  -= q1;
  //
#else
  //
  __SEGGER_RTL_U32 i0;
  //
  i0 = *pI;
  //
  while (i0 < __SEGGER_RTL_U32_C(0x00800000)) {
    i0 <<= 1;
    q0  -= 1;
  }
  //
#endif
  //
  *pI = i0;
  //
  return q0;
}

/*********************************************************************
*
*       Static code - sign bit handling
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_float32_signbit_soft()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_signbit_soft(__SEGGER_RTL_U32 x) {
  return x >> 31;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_signbit_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_signbit_inline(float x) {
  return __SEGGER_RTL_float32_signbit_soft(__SEGGER_RTL_BitcastToU32(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_signbit_soft()
*
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_signbit_soft(__SEGGER_RTL_U64 x) {
  return x >> 63;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_signbit_inline()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_signbit_inline(double x) {
  return __SEGGER_RTL_float64_signbit_soft(__SEGGER_RTL_BitcastToU64(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_signbit_xor_soft()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_signbit_xor_soft(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return x ^ (y & FLOAT32_SIGN_MASK);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_signbit_xor_soft()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float64_signbit_xor_soft(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return x ^ (y & FLOAT64_SIGN_MASK);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_signbit_xor()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_signbit_xor(float x, float y) {
#if defined(__SEGGER_RTL_FLOAT32_SIGNBIT_XOR)
  return __SEGGER_RTL_FLOAT32_SIGNBIT_XOR(x, y);
#else
  return __SEGGER_RTL_BitcastToF32(
           __SEGGER_RTL_float32_signbit_xor_soft(
             __SEGGER_RTL_BitcastToU32(x),
             __SEGGER_RTL_BitcastToU32(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_signbit_xor()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_signbit_xor(double x, double y) {
#if defined(__SEGGER_RTL_FLOAT64_SIGNBIT_XOR)
  return __SEGGER_RTL_FLOAT64_SIGNBIT_XOR(x, y);
#else
  return __SEGGER_RTL_BitcastToF64(
           __SEGGER_RTL_float64_signbit_xor_soft(
             __SEGGER_RTL_BitcastToU64(x),
             __SEGGER_RTL_BitcastToU64(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_signbit_copy_soft()
*
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_signbit_copy_soft(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return (x & ~FLOAT32_SIGN_MASK) | (y & FLOAT32_SIGN_MASK);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_signbit_copy_soft()
*
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float64_signbit_copy_soft(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return (x & ~FLOAT64_SIGN_MASK) | (y & FLOAT64_SIGN_MASK);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_signbit_copy()
*
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_signbit_copy(float x, float y) {
#if defined(__SEGGER_RTL_FLOAT32_SIGNBIT_COPY)
  //
  return __SEGGER_RTL_FLOAT32_SIGNBIT_COPY(x, y);
  //
#elif (__SEGGER_RTL_FP_HW >= 1) && defined(__SEGGER_RTL_FLOAT32_ABS)
  //
  x = __SEGGER_RTL_FLOAT32_ABS(x);
  if (__SEGGER_RTL_BitcastToU32(y) & FLOAT32_SIGN_MASK) {
    return -x;
  } else {
    return x;
  }
#else
  return __SEGGER_RTL_BitcastToF32(
           __SEGGER_RTL_float32_signbit_copy_soft(
             __SEGGER_RTL_BitcastToU32(x),
             __SEGGER_RTL_BitcastToU32(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_signbit_copy()
*
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_signbit_copy(double x, double y) {
#if defined(__SEGGER_RTL_FLOAT64_SIGNBIT_COPY)
  return __SEGGER_RTL_FLOAT64_SIGNBIT_COPY(x, y);
#else
  return __SEGGER_RTL_BitcastToF64(
           __SEGGER_RTL_float64_signbit_copy_soft(
             __SEGGER_RTL_BitcastToU64(x),
             __SEGGER_RTL_BitcastToU64(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_opposite_signs()
*
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_opposite_signs(float x, float y) {
  return ((__SEGGER_RTL_BitcastToU32(x) ^ __SEGGER_RTL_BitcastToU32(y)) & FLOAT32_SIGN_MASK) != 0;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_opposite_signs()
*
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_opposite_signs(double x, double y) {
  return ((__SEGGER_RTL_BitcastToU64(x) ^ __SEGGER_RTL_BitcastToU64(y)) & FLOAT64_SIGN_MASK) != 0;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lt_soft()
*
*  Function description
*    Less than query, float.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is less than y.
*    == 0 - x is not less than y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_lt_soft(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_soft(x) || __SEGGER_RTL_float32_isnan_soft(y))) {
    return 0;
  } else if (__SEGGER_RTL_UNLIKELY(((x | y) << 1) == 0)) {  // -0 == +0
    return 0;
  } else if (FLOAT32_SIGN(x | y)) {
    return y < x;
  } else {
    return x < y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lt()
*
*  Function description
*    Less than query, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is less than y.
*    == 0 - x is not less than y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_lt(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x < y;
#else
  return __SEGGER_RTL_float32_lt_soft(__SEGGER_RTL_BitcastToU32(x),
                                      __SEGGER_RTL_BitcastToU32(y));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lt_soft()
*
*  Function description
*    Less than query, float.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is less than y.
*    == 0 - x is not less than y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_lt_soft(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_soft(x) || __SEGGER_RTL_float64_isnan_soft(y))) {
    return 0;
  } else if (__SEGGER_RTL_UNLIKELY(((x | y) << 1) == 0)) {  // -0 == +0
    return 0;
  } else if (FLOAT64_SIGN(x | y)) {
    return y < x;
  } else {
    return x < y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lt()
*
*  Function description
*    Less than query, float.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is less than y.
*    == 0 - x is not less than y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_lt(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x < y;
#else
  return __SEGGER_RTL_float64_lt_soft(__SEGGER_RTL_BitcastToU64(x),
                                      __SEGGER_RTL_BitcastToU64(y));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_gt()
*
*  Function description
*    Greater than query, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is greater than or equal to y.
*    == 0 - x is not greater than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_gt(float x, float y) {
  return __SEGGER_RTL_float32_lt(y, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_gt()
*
*  Function description
*    Greater than query, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is greater than or equal to y.
*    == 0 - x is not greater than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_gt(double x, double y) {
  return __SEGGER_RTL_float64_lt(y, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_le_soft()
*
*  Function description
*    Less than query, float.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_le_soft(__SEGGER_RTL_I32 x, __SEGGER_RTL_I32 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_soft(x) || __SEGGER_RTL_float32_isnan_soft(y))) {
    return 0;
  } else if (__SEGGER_RTL_UNLIKELY(((x | y) << 1) == 0)) {  // -0 == +0
    return 1;
  } else if (x < 0 && y < 0) {
    return y <= x;
  } else {
    return x <= y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_le()
*
*  Function description
*    Less than query, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_le(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x <= y;
#else
  return __SEGGER_RTL_float32_le_soft(__SEGGER_RTL_BitcastToU32(x),
                                      __SEGGER_RTL_BitcastToU32(y));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_le_soft()
*
*  Function description
*    Less than query, double.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_le_soft(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_soft(x) || __SEGGER_RTL_float64_isnan_soft(y))) {
    return 0;
  } else if (__SEGGER_RTL_UNLIKELY(((x | y) << 1) == 0)) {  // -0 == +0
    return 1;
  } else if (FLOAT64_SIGN(x | y)) {
    return y <= x;
  } else {
    return x <= y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_le()
*
*  Function description
*    Less than query, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_le(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x <= y;
#else
  return __SEGGER_RTL_float64_le_soft(__SEGGER_RTL_BitcastToU64(x),
                                      __SEGGER_RTL_BitcastToU64(y));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ge()
*
*  Function description
*    Greater than or equal query, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is greater than or equal to y.
*    == 0 - x is not greater than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_ge(float x, float y) {
  return __SEGGER_RTL_float32_le(y, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ge()
*
*  Function description
*    Greater than or equal query, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is greater than or equal to y.
*    == 0 - x is not greater than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_ge(double x, double y) {
  return __SEGGER_RTL_float64_le(y, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_eq_soft()
*
*  Function description
*    Equality query, float.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is equal to y (with -0 and +0 comparing equal) with neither x nor y as NaN.
*    == 0 - x is not equal to y, or x is NaN, or y is NaN.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_eq_soft(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_soft(x) || __SEGGER_RTL_float32_isnan_soft(y))) {
    return 0;
  } else {
    return (x == y) || __SEGGER_RTL_UNLIKELY(((x | y) << 1) == 0);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_eq()
*
*  Function description
*    Equality query, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is equal to x (with -0 and +0 comparing equal) with neither x nor y as NaN.
*    == 0 - x is not equal to y, or x is NaN, or y is NaN.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_eq(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == y;
#else
  return __SEGGER_RTL_float32_eq_soft(__SEGGER_RTL_BitcastToU32(x),
                                      __SEGGER_RTL_BitcastToU32(y));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_eq_soft()
*
*  Function description
*    Equality query, double.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is equal to y (with -0 and +0 comparing equal).
*    == 0 - x is not equal to y.
*/
static int __SEGGER_RTL_float64_eq_soft(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_soft(x) || __SEGGER_RTL_float64_isnan_soft(y))) {
    return 0;
  } else {
    return x == y || __SEGGER_RTL_UNLIKELY(((x | y) << 1) == 0);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_eq()
*
*  Function description
*    Equality query, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    != 0 - x is equal to y (with -0 and +0 comparing equal).
*    == 0 - x is not equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_eq(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == y;
#else
  return __SEGGER_RTL_float64_eq_soft(__SEGGER_RTL_BitcastToU64(x),
                                      __SEGGER_RTL_BitcastToU64(y));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ne_soft()
*
*  Function description
*    Equality query, float.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is not equal to y (with -0 and +0 comparing not equal).
*    == 0 - x is equal to y or x or y are a NaN.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_ne_soft(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_soft(x) || __SEGGER_RTL_float32_isnan_soft(y))) {
    return 1;
  } else {
    return x == y ? 0 : ((x | y) << 1) != 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ne()
*
*  Function description
*    Equality query, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is not equal to y (with -0 and +0 comparing not equal).
*    == 0 - x is equal to y or x or y are a NaN.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_ne(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x != y;
#else
  return __SEGGER_RTL_float32_ne_soft(__SEGGER_RTL_BitcastToU32(x),
                                      __SEGGER_RTL_BitcastToU32(y));
#endif
}
/*********************************************************************
*
*       __SEGGER_RTL_float64_ne_soft()
*
*  Function description
*    Equality query, double.
*
*  Parameters
*    x - Left-hand value as bitstring.
*    y - Right-hand value as bitstring.
*
*  Return value
*    == 1 - x is not equal to y (with -0 and +0 comparing not equal).
*    == 0 - x is equal to y or x or y are a NaN.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_ne_soft(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_soft(x) || __SEGGER_RTL_float64_isnan_soft(y))) {
    return 1;
  } else {
    return x == y ? 0 : ((x | y) << 1) != 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ne()
*
*  Function description
*    Equality query, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is not equal to y (with -0 and +0 comparing not equal).
*    == 0 - x is equal to y or x or y are a NaN.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_ne(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x != y;
#else
  return __SEGGER_RTL_float64_ne_soft(__SEGGER_RTL_BitcastToU64(x),
                                      __SEGGER_RTL_BitcastToU64(y));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_eq_bitwise()
*
*  Function description
*    Equal to query, finite operands, float.
*
*  Parameters
*    x - Left-hand value, must be finite.
*    y - Right-hand value, must be finite.
*
*  Return value
*    == 1 - x is equal to y.
*    == 0 - x is not equal to y.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_eq_bitwise(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == y;
#else
  return __SEGGER_RTL_BitcastToU32(x) == __SEGGER_RTL_BitcastToU32(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_eq_bitwise()
*
*  Function description
*    Equal to query, finite operands, double.
*
*  Parameters
*    x - Left-hand value, must be finite.
*    y - Right-hand value, must be finite.
*
*  Return value
*    == 1 - x is equal to y.
*    == 0 - x is not equal to y.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_eq_bitwise(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == y;
#else
  return __SEGGER_RTL_BitcastToU64(x) == __SEGGER_RTL_BitcastToU64(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_eq_bitwise_zeros_compare_equal()
*
*  Function description
*    Equal to query, finite operands, double.
*
*  Parameters
*    x - Left-hand value, must be finite.
*    y - Right-hand value, must be finite.
*
*  Return value
*    == 1 - x is equal to y.
*    == 0 - x is not equal to y.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_eq_bitwise_zeros_compare_equal(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x == y;
#else
  if (__SEGGER_RTL_BitcastToU64(x) == __SEGGER_RTL_BitcastToU64(y)) {
    return 1;
  } else {
    return __SEGGER_RTL_float64_exact_iszero(x) && __SEGGER_RTL_float64_exact_iszero(y);
  }
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_eq_finite()
*
*  Function description
*    Equal to query, finite operands, double.
*
*  Parameters
*    x - Left-hand value, must be finite.
*    y - Right-hand value, must be finite.
*
*  Return value
*    == 1 - x is equal to y.
*    == 0 - x is not equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_eq_finite(double x, double y) {
#if (__SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT) || __SEGGER_RTL_CONFIG_CODE_COVERAGE
  return x == y;
#else
  return __SEGGER_RTL_BitcastToU64(x) == __SEGGER_RTL_BitcastToU64(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lt_rhs_positive()
*
*  Function description
*    Less than query, restricted, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value, must be positive.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*
*  Additional information
*    This comparison delivers correct results for a limited set of
*    inputs.  It does not accept NaN inputs and, in such cases,
*    the result is undefined.
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_lt_rhs_positive(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x < y;
#else
  return (__SEGGER_RTL_I32)__SEGGER_RTL_BitcastToU32(x) < (__SEGGER_RTL_I32)__SEGGER_RTL_BitcastToU32(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lt_rhs_positive()
*
*  Function description
*    Less than query, restricted, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value, must be positive.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*
*  Additional information
*    This comparison delivers correct results for a limited set of
*    inputs.  It does not accept NaN inputs and, in such cases,
*    the result is undefined.
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_lt_rhs_positive(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x < y;
#else
  return (__SEGGER_RTL_I64)__SEGGER_RTL_BitcastToU64(x) < (__SEGGER_RTL_I64)__SEGGER_RTL_BitcastToU64(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lt_bitwise_unsigned()
*
*  Function description
*    Less than query, restricted, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value, must be positive.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*
*  Additional information
*    This comparison delivers correct results for a limited set of
*    inputs.  It does not accept NaN inputs and, in such cases,
*    the result is undefined.
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_lt_bitwise_unsigned(float x, float y) {
  return (__SEGGER_RTL_I32)__SEGGER_RTL_BitcastToU32(x) < (__SEGGER_RTL_I32)__SEGGER_RTL_BitcastToU32(y);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lt_bitwise_unsigned()
*
*  Function description
*    Less than query, restricted, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value, must be positive.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*
*  Additional information
*    This comparison delivers correct results for a limited set of
*    inputs.  It does not accept NaN inputs and, in such cases,
*    the result is undefined.
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_lt_bitwise_unsigned(double x, double y) {
  return (__SEGGER_RTL_I64)__SEGGER_RTL_BitcastToU64(x) < (__SEGGER_RTL_I64)__SEGGER_RTL_BitcastToU64(y);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_gt_rhs_positive()
*
*  Function description
*    Less than query, restricted, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value, must be positive.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*
*  Additional information
*    This comparison delivers correct results for a limited set of
*    inputs.  It does not accept NaN inputs and, in such cases,
*    the result is undefined.
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_gt_rhs_positive(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x > y;
#else
  return (__SEGGER_RTL_I64)__SEGGER_RTL_BitcastToU64(x) > (__SEGGER_RTL_I64)__SEGGER_RTL_BitcastToU64(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ge_rhs_positive()
*
*  Function description
*    Less than query, restricted, double.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value, must be positive.
*
*  Return value
*    == 1 - x is greater than or equal to y.
*    == 0 - x is not greater than or equal to y.
*
*  Additional information
*    This comparison delivers correct results for a limited set of
*    inputs.  It does not accept NaN inputs and, in such cases,
*    the result is undefined.
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_ge_rhs_positive(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x >= y;
#else
  return (__SEGGER_RTL_I64)__SEGGER_RTL_BitcastToU64(x) >= (__SEGGER_RTL_I64)__SEGGER_RTL_BitcastToU64(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lt_one_negative()
*
*  Function description
*    Less than query, restricted, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*
*  Additional information
*    This comparison delivers correct results for a limited set of
*    inputs.  It does not accept NaN inputs and, in such cases,
*    the result is undefined.
*
*    Note the +0/-0 comparison case: this would usually deliver a
*    0 result for true IEEE arithmetic but delivers a strict ordering
*    here where -0 is less than +0.
*
*    The valid inputs, and their ordering, are shown below:
*
*    +---------------+---------------+-------------------------------+
*    | Left          | Right         | Result                        |
*    +---------------+---------------+-------------------------------+
*    | NaN           | Any           | Undefined                     |
*    | Any           | NaN           | Undefined                     |
*    | +ve           | +ve           | Undefined                     |
*    | +0            | -0            | 0                             |
*    | -0            | +0            | 1                             |
*    | -0            | -0            | 0                             |
*    | +ve           | -ve           | 0                             |
*    | -ve           | +ve           | 1                             |
*    | -ve           | -ve           | 0/1 based on input            |
*    +---------------+---------------+-------------------------------+
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_lt_one_negative(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x < y;
#else
  return __SEGGER_RTL_BitcastToU32(y) < __SEGGER_RTL_BitcastToU32(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lt_one_negative()
*
*  Function description
*    Less than query, restricted, float.
*
*  Parameters
*    x - Left-hand value.
*    y - Right-hand value.
*
*  Return value
*    == 1 - x is less than or equal to y.
*    == 0 - x is not less than or equal to y.
*
*  Additional information
*    This comparison delivers correct results for a limited set of
*    inputs.  It does not accept NaN inputs and, in such cases,
*    the result is undefined.
*
*    Note the +0/-0 comparison case: this would usually deliver a
*    0 result for true IEEE arithmetic but delivers a strict ordering
*    here where -0 is less than +0.
*
*    The valid inputs, and their ordering, are shown below:
*
*    +---------------+---------------+-------------------------------+
*    | Left          | Right         | Result                        |
*    +---------------+---------------+-------------------------------+
*    | NaN           | Any           | Undefined                     |
*    | Any           | NaN           | Undefined                     |
*    | +ve           | +ve           | Undefined                     |
*    | +0            | -0            | 0                             |
*    | -0            | +0            | 1                             |
*    | -0            | -0            | 0                             |
*    | +ve           | -ve           | 0                             |
*    | -ve           | +ve           | 1                             |
*    | -ve           | -ve           | 0/1 based on input            |
*    +---------------+---------------+-------------------------------+
*
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_lt_one_negative(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x < y;
#else
  return __SEGGER_RTL_BitcastToU64(y) < __SEGGER_RTL_BitcastToU64(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_gt_rhs_positive()
*
*  Function description
*    Greater than query, finite operands, float.
*
*  Parameters
*    x - Left-hand value, must be finite.
*    y - Right-hand value, must be positive.
*
*  Return value
*    == 1 - x is greater than y.
*    == 0 - x is not greater than y.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_gt_rhs_positive(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x > y;
#else
  return (__SEGGER_RTL_I32)__SEGGER_RTL_BitcastToU32(x) > (__SEGGER_RTL_I32)__SEGGER_RTL_BitcastToU32(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_gt_finite()
*
*  Function description
*    Greater than query, finite operands, double.
*
*  Parameters
*    x - Left-hand value, must be finite.
*    y - Right-hand value, must be finite.
*
*  Return value
*    == 1 - x is greater than y.
*    == 0 - x is not greater than y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_gt_finite(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x > y;
#else
  return __SEGGER_RTL_BitcastToU64(x) > __SEGGER_RTL_BitcastToU64(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ge_finite()
*
*  Function description
*    Greater than or equal to query, finite operands, float.
*
*  Parameters
*    x - Left-hand value, must be finite.
*    y - Right-hand value, must be finite.
*
*  Return value
*    == 1 - x is greater than or equal to y.
*    == 0 - x is not greater than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float32_ge_finite(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x >= y;
#else
  return __SEGGER_RTL_BitcastToU32(x) >= __SEGGER_RTL_BitcastToU32(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ge_finite()
*
*  Function description
*    Greater than query, finite operands, double.
*
*  Parameters
*    x - Left-hand value, must be finite.
*    y - Right-hand value, must be finite.
*
*  Return value
*    == 1 - x is greater than or equal to y.
*    == 0 - x is not greater than or equal to y.
*/
static __SEGGER_RTL_INLINE int __SEGGER_RTL_float64_ge_finite(double x, double y) {
#if (__SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT) || __SEGGER_RTL_CONFIG_CODE_COVERAGE
  return x >= y;
#else
  return __SEGGER_RTL_BitcastToU64(x) >= __SEGGER_RTL_BitcastToU64(y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_add_soft()
*
*  Function description
*    Add, float.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_add_soft(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  unsigned cx, rs;
  int      bx, shift;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial_soft(x) || __SEGGER_RTL_float32_isspecial_soft(y))) {
    //
    // Flush subnormals to correctly-signed zero.
    //
    x = __SEGGER_RTL_float32_conditional_subnormal_flush_soft(x);
    y = __SEGGER_RTL_float32_conditional_subnormal_flush_soft(y);
    //
    // Isolate Infs and NaNs.
    //
    if (__SEGGER_RTL_float32_isfinite_soft(x)) {
      if (__SEGGER_RTL_float32_isnan_soft(y)) {
        return K_NAN_U32;
      } else if (__SEGGER_RTL_float32_isinf_soft(y)) {
        return y;
      }
    } else {
      if (__SEGGER_RTL_float32_isnan_soft(x)) {
        return K_NAN_U32;
      } else if (__SEGGER_RTL_float32_isnan_soft(y)) {
        return K_NAN_U32;
      } else if (x == K_INF_U32 && y == K_MINUS_INF_U32) {
        return K_NAN_U32;
      } else if (x == K_MINUS_INF_U32 && y == K_INF_U32) {
        return K_NAN_U32;
      } else if (__SEGGER_RTL_float32_isinf_soft(x)) {
        return x;
      }
    }
    //
    // Take care of subtraction between zeroes.
    //
    if ((x ^ y) & FLOAT32_SIGN_MASK) {
      if (x == __SEGGER_RTL_U32_C(0x80000000)) {
        return y;
      } else if (y == __SEGGER_RTL_U32_C(0x80000000)) {
        return x;
      } else if (x == __SEGGER_RTL_U32_C(0x00000000)) {
        return y;
      } else if (y == __SEGGER_RTL_U32_C(0x00000000)) {
        return x;
      }
    }
  }
  //
  // At this point we only have finite summands.
  //
  if ((x ^ y) & FLOAT32_SIGN_MASK) {
    //
    // Make signs identical.
    //
    y ^= FLOAT32_SIGN_MASK;
    //
    // Make b the largest.
    //
    if (x < y) {
      //
      // Swap b and c.
      //
      x ^= y;
      y ^= x;
      x ^= y;
      //
      // Flip sign.
      //
      x ^= FLOAT32_SIGN_MASK;
      y ^= FLOAT32_SIGN_MASK;
    }
    //
    // Extract exponents.
    //
    bx = (x >> 23) & 0xFF;
    cx = (y >> 23) & 0xFF;
    rs = x >> 23;
    //
    shift = bx - cx;
    if (shift > 25) {
      return x;
    }
    //
    // Extract significand and set hidden bit.
    //
    x &= FLOAT32_SIGNIFICAND_MASK;
    x |= FLOAT32_HIDDEN_MASK;
    y &= FLOAT32_SIGNIFICAND_MASK;
    y |= FLOAT32_HIDDEN_MASK;
    //
    // Form difference.
    //
    x -= y >> shift;
    y = 0 - y;
    if (shift == 0) {
      y = 0;
    } else {
      y <<= 32 - shift;
    }
    if (y) {
      --x;
      if (x == 0) {
        x = y & __SEGGER_RTL_U32_C(0x80000000) ? 1 : 0;
        --bx;
        y <<= 1;
      }
    }
    //
    if (x == 0) {
      return 0;
    }
    //
    // Normalize.
    //
    while ((x & __SEGGER_RTL_U32_C(0x00800000)) == 0) {
      x <<= 1;
      if (y & __SEGGER_RTL_U32_C(0x80000000)) {
        ++x;
      }
      y <<= 1;
      --bx;
    }
    //
    // Perform round to nearest.
    //
    if (y >= __SEGGER_RTL_U32_C(0x80000000)) {
      ++x;
      if (y == __SEGGER_RTL_U32_C(0x80000000)) {
        x &= ~__SEGGER_RTL_U32_C(1);
      }
    }
    if (x & __SEGGER_RTL_U32_C(0x01000000)) {
      x >>= 1;
      ++bx;
    }
  } else {
    //
    // Addition.  Make b the largest.
    //
    if (x < y) {
      //
      // Swap b and c.
      //
      x ^= y;
      y ^= x;
      x ^= y;
    }
    //
    // x + +/-0 == x
    //
    if ((y << 1) == 0) {
      return x;
    }
    //
    // Extract exponents.
    //
    bx = (x >> 23) & 0xFF;
    cx = (y >> 23) & 0xFF;
    rs = x >> 23;
    //
    // Compute alignment.  If exponents differ by more than 25 bits, with b > c,
    // all significance in c is lost.
    //
    shift = bx - cx;
    if (shift > 25) {
      return x;
    }
    //
    // Extract significand and set hidden bit.
    //
    x &= FLOAT32_SIGNIFICAND_MASK;
    x |= FLOAT32_HIDDEN_MASK;
    y &= FLOAT32_SIGNIFICAND_MASK;
    y |= FLOAT32_HIDDEN_MASK;
    //
    // Form sum and compute bits that didn't form part of the sum.
    //
    x += y >> shift;
    if (shift == 0) {
      y = 0;
    } else {
      y <<= 32-shift;
    }
    //
    // Compensate for carry into higher order bit.
    //
    if (x & __SEGGER_RTL_U32_C(0x01000000)) {
      //
      // Readjust significand.
      //
      y >>= 1;
      if (x & 1) {
        y |= __SEGGER_RTL_U32_C(0x80000000);
      }
      x >>= 1;
      ++bx;
    }
    //
    // Perform round to nearest.
    //
    if (y >= __SEGGER_RTL_U32_C(0x80000000)) {
      ++x;
      if (y == __SEGGER_RTL_U32_C(0x80000000)) {
        x &= ~__SEGGER_RTL_U32_C(1);
      }
      if (x & __SEGGER_RTL_U32_C(0x01000000)) {
        x >>= 1;
        ++bx;
      }
    }
  }
  //
  // Check for overflow, saturate at Inf.
  // Flush underflows and subnormals to zero.
  //
  if (bx >= 0xFF) {
    bx = 0xFF;
    x  = 0;
  } else if (bx <= 0) {
    bx = 0;
    x  = 0;
  }
  //
  // Pack result sign.
  //
  bx |= (rs & 0x100);
  //
  // Pack exponent and significand.
  //
  return (x & ~__SEGGER_RTL_U32_C(0x800000)) | ((__SEGGER_RTL_U32)bx << 23);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_add()
*
*  Function description
*    Add, float.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_add(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x + y;
#else
  return __SEGGER_RTL_BitcastToF32(
           __SEGGER_RTL_float32_add_soft(
             __SEGGER_RTL_BitcastToU32(x),
             __SEGGER_RTL_BitcastToU32(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_add_soft()
*
*  Function description
*    Add, double.
*
*  Parameters
*    b - Augend.
*    c - Addend.
*
*  Return value
*    Sum.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float64_add_soft(__SEGGER_RTL_U64 b, __SEGGER_RTL_U64 c) {
  unsigned cx;
  unsigned rs;
  int      bx;
  int      shift;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial_soft(b) || __SEGGER_RTL_float64_isspecial_soft(c))) {
    //
    // Flush subnormals to correctly-signed zero.
    //
    b = __SEGGER_RTL_float64_conditional_subnormal_flush_soft(b);
    c = __SEGGER_RTL_float64_conditional_subnormal_flush_soft(c);
    //
    // Isolate Infs and NaNs.
    //
    if (__SEGGER_RTL_float64_isfinite_soft(b)) {
      if (__SEGGER_RTL_float64_isnan_soft(c)) {
        return K_NAN_U64;
      } else if (__SEGGER_RTL_float64_isinf_soft(c)) {
        return c;
      }
    } else {
      if (__SEGGER_RTL_float64_isnan_soft(b)) {
        return K_NAN_U64;
      } else if (__SEGGER_RTL_float64_isnan_soft(c)) {
        return K_NAN_U64;
      } else if (b == __SEGGER_RTL_U64_C(0x7FF0000000000000) && c == __SEGGER_RTL_U64_C(0xFFF0000000000000)) { // +Inf + -Inf
        return K_NAN_U64;
      } else if (b == __SEGGER_RTL_U64_C(0xFFF0000000000000) && c == __SEGGER_RTL_U64_C(0x7FF0000000000000)) { // -Inf + +Inf
        return K_NAN_U64;
      } else if (__SEGGER_RTL_float64_isinf_soft(b)) {
        return b;
      }
    }
    //
    // Take care of subtraction between zeroes.
    //
    if ((b ^ c) & FLOAT64_SIGN_MASK) {
      if (b == K_MINUS_ZERO_U64) {
        return c;
      } else if (c == K_MINUS_ZERO_U64) {
        return b;
      } else if (b == K_ZERO_U64) {
        return c;
      } else if (c == K_ZERO_U64) {
        return b;
      }
    }
  }
  //
  // At this point we only have finite summands.
  //
  // Determine if addition or subtraction.
  //
  if ((b ^ c) & FLOAT64_SIGN_MASK) {
    //
    // Subtraction.  Form difference.
    //
    // Make signs identical.
    //
    c ^= FLOAT64_SIGN_MASK;
    //
    // Make b the largest.
    //
    if (b < c) {
      //
      // Swap b and c.
      //
      b ^= c;
      c ^= b;
      b ^= c;
      //
      // Flip sign.
      //
      b ^= FLOAT64_SIGN_MASK;
      c ^= FLOAT64_SIGN_MASK;
    }
    //
    // Remember sign.
    //
    rs = b >> FLOAT64_SIGNIFICAND_BITS;
    //
    // Extract exponents and signs.
    //
    bx = FLOAT64_EXPONENT(b);
    cx = FLOAT64_EXPONENT(c);
    //
    shift = bx - cx;
    if (shift > 54) {
      return b;
    }
    //
    // Extract significand, set hidden bit.
    //
    b &= FLOAT64_SIGNIFICAND_MASK;
    b |= FLOAT64_HIDDEN_MASK;
    c &= FLOAT64_SIGNIFICAND_MASK;
    c |= FLOAT64_HIDDEN_MASK;
    //
    // Do subtraction, fold in sticky bit.
    //
    b -= (c >> shift);
    c = 0u - c;
    if (shift == 0) {
      c = 0;
    } else {
      c <<= (64 - shift);
    }
    if (c) {
      --b;
      if (b == 0) {
        b = c & __SEGGER_RTL_U64_C(0x8000000000000000) ? 1 : 0;
        --bx;
        c <<= 1;
      }
    }
    //
    // Underflowed to zero?
    //
    if (b == 0) {
      return 0;
    }
    //
    // Normalize.
    //
#if defined(__SEGGER_RTL_CLZ_U64) && !defined(__SEGGER_RTL_CLZ_U64_SYNTHESIZED)
    //
    shift  = __SEGGER_RTL_CLZ_U64(b) - 11;
    if (shift) {
      b <<= shift;
      b  |= c >> (64 - shift);
      c <<= shift;
      bx -= shift;
    }
    //
#else
    //
    while ((b & __SEGGER_RTL_U64_C(0x10000000000000)) == 0) {
      b <<= 1;
      if (c & __SEGGER_RTL_U64_C(0x8000000000000000)) {
        ++b;
      }
      c <<= 1;
      --bx;
    }
    //
#endif
    //
    // Perform round to nearest.
    //
    if (c >= __SEGGER_RTL_U64_C(0x8000000000000000)) {
      ++b;
      if (c == __SEGGER_RTL_U64_C(0x8000000000000000)) {
        b &= ~__SEGGER_RTL_U64_C(1);
      }
    }
    //
    // Renormalize.
    //
    if (b & __SEGGER_RTL_U64_C(0x20000000000000)) {
      b >>= 1;
      ++bx;
    }
  } else {
    //
    // Addition.  Make b the largest.
    //
    if (b < c) {
      //
      // Swap b and c.
      //
      b ^= c;
      c ^= b;
      b ^= c;
    }
    //
    // x + +/-0 == x
    //
    if (c == __SEGGER_RTL_U64_C(0x0000000000000000) || c == __SEGGER_RTL_U64_C(0x8000000000000000)) {
      return b;
    }
    //
    // Remember result sign.
    //
    rs = b >> FLOAT64_SIGNIFICAND_BITS;
    //
    // Extract exponents and signs.
    //
    bx = FLOAT64_EXPONENT(b);
    cx = FLOAT64_EXPONENT(c);
    //
    // Compute alignment.  If exponents differ by more than 54 bits, with b > c,
    // all significance in c is lost.
    //
    shift = bx - cx;
    if (shift > 54) {
      return b;
    }
    //
    // Extract significand, set hidden bit, shift for significance.
    //
    b &= FLOAT64_SIGNIFICAND_MASK;
    b |= FLOAT64_HIDDEN_MASK;
    c &= FLOAT64_SIGNIFICAND_MASK;
    c |= FLOAT64_HIDDEN_MASK;
    //
    // Form sum and compute bits that didn't form part of the sum.
    //
    b += c >> shift;
    if (shift == 0) {
      c = 0;
    } else {
      c <<= 64-shift;
    }
    //
    // Compensate for carry into higher order bit.
    //
    if (b & __SEGGER_RTL_U64_C(0x20000000000000)) {
      //
      // Readjust significand.
      //
      c >>= 1;
      if (b & 1) {
        c |= __SEGGER_RTL_U64_C(0x8000000000000000);
      }
      b >>= 1;
      ++bx;
    }
    //
    // Perform round to nearest.
    //
    if (c >= __SEGGER_RTL_U64_C(0x8000000000000000)) {
      ++b;
      if (c == __SEGGER_RTL_U64_C(0x8000000000000000)) {
        b &= ~__SEGGER_RTL_U64_C(1);
      }
      if (b & __SEGGER_RTL_U64_C(0x20000000000000)) {
        b >>= 1;
        ++bx;
      }
    }
  }
  //
  // Check for overflow, saturate at Inf.
  // Flush underflows and subnormals to zero.
  //
  if (bx >= 0x7FF) {
    b  = 0;
    bx = 0x7FF;
  } else if (bx <= 0) {
    b  = 0;
    bx = 0;
  }
  //
  // Pack result sign.
  //
  bx |= (rs & 0x800);
  //
  // Align and remove hidden bit.
  //
  b &= ~FLOAT64_HIDDEN_MASK;
  //
  // Pack exponent and significand.
  //
  return b | ((__SEGGER_RTL_U64)bx << FLOAT64_SIGNIFICAND_BITS);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_add()
*
*  Function description
*    Add, double.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_add(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x + y;
#else
  return __SEGGER_RTL_BitcastToF64(
           __SEGGER_RTL_float64_add_soft(
             __SEGGER_RTL_BitcastToU64(x),
             __SEGGER_RTL_BitcastToU64(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sub()
*
*  Function description
*    Subtract, float.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_sub(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x - y;
#else
  return __SEGGER_RTL_BitcastToF32(
           __SEGGER_RTL_float32_add_soft(
             __SEGGER_RTL_BitcastToU32(x),
             __SEGGER_RTL_BitcastToU32(y) ^ FLOAT32_SIGN_MASK));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sub()
*
*  Function description
*    Subtract, double.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sub(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x - y;
#else
  return __SEGGER_RTL_BitcastToF64(
           __SEGGER_RTL_float64_add_soft(
             __SEGGER_RTL_BitcastToU64(x),
             __SEGGER_RTL_BitcastToU64(y) ^ FLOAT64_SIGN_MASK));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_mul_soft()
*
*  Function description
*    Multiply, float.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_mul_soft(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  unsigned               xx;
  unsigned               yx;
  unsigned               rs;
  __SEGGER_RTL_UINT_LEAST32_T rb;
  __SEGGER_RTL_U64       p;
  //
  // Extract exponents and signs.
  //
  xx = (unsigned)(x >> 23);
  yx = (unsigned)(y >> 23);
  rs = (xx ^ yx) & 0x100;
  xx &= 0xFF;
  yx &= 0xFF;
  //
  // Handle Inf and NaN inputs.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial_soft(x) || __SEGGER_RTL_float32_isspecial_soft(y))) {
    if (xx == 0 && !__SEGGER_RTL_float32_isfinite_soft(y)) {
       return K_NAN_U32;
    } else if (yx == 0 && !__SEGGER_RTL_float32_isfinite_soft(x)) {
       return K_NAN_U32;
    } else if (__SEGGER_RTL_float32_isnan_soft(x) || __SEGGER_RTL_float32_isnan_soft(y)) {
       return K_NAN_U32;
    } else if (__SEGGER_RTL_float32_isinf_soft(x) || __SEGGER_RTL_float32_isinf_soft(y)) {
      return ((x ^ y) & FLOAT32_SIGN_MASK) | K_INF_U32;
    } else if (xx == 0 || yx == 0) {
      return (x ^ y) & FLOAT32_SIGN_MASK;
    }
  }
  //
  // Calculate product exponent.
  //
  xx += yx - 0x7E;
  //
  // Extract significand and set hidden bit.
  //
  x &= __SEGGER_RTL_U32_C(0x7FFFFF);
  x |= __SEGGER_RTL_U32_C(0x800000);
  y &= __SEGGER_RTL_U32_C(0x7FFFFF);
  y |= __SEGGER_RTL_U32_C(0x800000);
  //
  // Multiply in extended precision.
  //
  p = __SEGGER_RTL_UMULL_X(x, y);
  //
  // Round.
  //
  if ((p & __SEGGER_RTL_U64_C(0x800000000000)) == 0) {
    p <<= 1;
    --xx;
  }
  rb = __SEGGER_RTL_U64_L(p) & __SEGGER_RTL_U32_C(0xFFFFFF);    // Rounding bits
  p >>= 24;                                                     // Drop guard bits
  if (rb >= __SEGGER_RTL_U32_C(0x800000)) {
    ++p;
    if (rb == __SEGGER_RTL_U32_C(0x800000)) {
      p &= ~__SEGGER_RTL_U64_C(1);
    }
    if (p & __SEGGER_RTL_U32_C(0x1000000)) {
      p >>= 1;
      ++xx;
    }
  }
  //
  // If underflow, return signed zero.
  // If overflow, return Inf.
  //
  if ((__SEGGER_RTL_I16)xx <= 0) {
    xx = 0;
    p  = 0;
  } else if (xx >= 0xFF) {
    xx = 0xFF;
    p  = 0;
  }
  //
  // Put product's sign into bx.
  //
  xx |= rs;
  //
  // Pack and return.
  //
  return (__SEGGER_RTL_U64_L(p) & ~__SEGGER_RTL_U32_C(0x800000)) | ((__SEGGER_RTL_U32)xx << 23);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_mul()
*
*  Function description
*    Multiply, float.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_mul(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x * y;
#else
  return __SEGGER_RTL_BitcastToF32(
           __SEGGER_RTL_float32_mul_soft(
             __SEGGER_RTL_BitcastToU32(x),
             __SEGGER_RTL_BitcastToU32(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_mul_soft()
*
*  Function description
*    Multiply, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float64_mul_soft(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  //
  __SEGGER_RTL_INT_LEAST16_T  xx;
  __SEGGER_RTL_UINT_LEAST16_T yx, rs;
  //
#if defined(__SEGGER_RTL_U128) && defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  __SEGGER_RTL_U128           product;
#else
  __SEGGER_RTL_U64       x0, x1, ma, mb;
#endif
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial_soft(x) || __SEGGER_RTL_float64_isspecial_soft(y))) {
    //
    // Flush subnormals to correctly-signed zero.
    //
    x = __SEGGER_RTL_float64_conditional_subnormal_flush_soft(x);
    y = __SEGGER_RTL_float64_conditional_subnormal_flush_soft(y);
    //
    // Propagate NaNs.
    //
    if (__SEGGER_RTL_float64_isnan_soft(x)) {
      return K_NAN_U64;
    } else if (__SEGGER_RTL_float64_isnan_soft(y)) {
      return K_NAN_U64;
    } else if (__SEGGER_RTL_float64_isinf_soft(x)) {
      //
      // Inf * 0 = NaN, Inf * x = Inf
      //
      if ((y << 1) == 0) {
        return K_NAN_U64;
      } else {
        return x ^ (y & __SEGGER_RTL_U64_C(0x8000000000000000));
      }
    } else if (__SEGGER_RTL_float64_isinf_soft(y)) {
      //
      // Inf * 0 = NaN, Inf * x = Inf
      //
      if ((x << 1) == 0) {
        return K_NAN_U64;
      } else {
        return y ^ (x & __SEGGER_RTL_U64_C(0x8000000000000000));
      }
    }
    //
    // Take care of zeros.
    //
    if (x == __SEGGER_RTL_U64_C(0x8000000000000000)) {
      return (y & __SEGGER_RTL_U64_C(0x8000000000000000)) ^ __SEGGER_RTL_U64_C(0x8000000000000000);
    } else if (y == __SEGGER_RTL_U64_C(0x8000000000000000)) {
      return (x & __SEGGER_RTL_U64_C(0x8000000000000000)) ^ __SEGGER_RTL_U64_C(0x8000000000000000);
    } else if (x == __SEGGER_RTL_U64_C(0x0000000000000000)) {
      return y & __SEGGER_RTL_U64_C(0x8000000000000000);
    } else if (y == __SEGGER_RTL_U64_C(0x0000000000000000)) {
      return x & __SEGGER_RTL_U64_C(0x8000000000000000);
    }
  }
  //
  // Extract exponents and signs.
  //
  xx  = (__SEGGER_RTL_INT_LEAST16_T)(x >> FLOAT64_SIGNIFICAND_BITS);
  yx  = (__SEGGER_RTL_INT_LEAST16_T)(y >> FLOAT64_SIGNIFICAND_BITS);
  rs  = xx ^ yx;
  xx &= 0x7FF;
  yx &= 0x7FF;
  xx += yx - 0x3FF;
  //
  // Extract significands and set hidden bit.
  //
  x &= __SEGGER_RTL_U64_C(0x0FFFFFFFFFFFFF);
  x |= __SEGGER_RTL_U64_C(0x10000000000000);
  y &= __SEGGER_RTL_U64_C(0x0FFFFFFFFFFFFF);
  y |= __SEGGER_RTL_U64_C(0x10000000000000);
  //
  // Form 128-bit product.
  //
  x <<= 10;
  y <<= 11;
  //
#if defined(__SEGGER_RTL_U128) && defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  //
  // Generate full 128-bit product using 128-bit type.
  //
  product = (__SEGGER_RTL_U128)x * (__SEGGER_RTL_U128)y;
  //
  // Recover most-significant 64 bits with sticky bits folding.
  //
  x  = product >> 64;
  x |= (product & __SEGGER_RTL_U64_C(0xFFFFFFFFFFFFFFFF)) != 0;
  //
#else
  //
  // Generate partial products.
  //
  x1 = __SEGGER_RTL_UMULL_X(__SEGGER_RTL_U64_L(x), __SEGGER_RTL_U64_L(y));
  ma = __SEGGER_RTL_UMULL_X(__SEGGER_RTL_U64_L(x), __SEGGER_RTL_U64_H(y));
  mb = __SEGGER_RTL_UMULL_X(__SEGGER_RTL_U64_H(x), __SEGGER_RTL_U64_L(y));
  x0 = __SEGGER_RTL_UMULL_X(__SEGGER_RTL_U64_H(x), __SEGGER_RTL_U64_H(y));
  //
  // Sum partial products.
  //
  ma += mb;
  x0 += (ma >> 32);
  if (ma < mb) {
    x0 += __SEGGER_RTL_U64_C(0x100000000);
  }
  ma <<= 32;
  x1 += ma;
  if (x1 < ma) {
    ++x0;
  }
  //
  // Fold in sticky bits.
  //
  if (x1) {
    x0 |= 1;
  }
  x = x0;
  //
#endif
  //
  // Normalize correctly.
  //
  ++xx;
  if ((x & __SEGGER_RTL_U64_C(0x4000000000000000)) == 0) {
    x <<= 1;
    --xx;
  }
  //
  // Detect tininess before rounding.
  //
  if (xx <= 0) {
    xx = 0;
    x = 0;
  }
  //
  // Round.
  //
  if ((x & 0x3FF) > 0x200) {
    x += __SEGGER_RTL_U64_C(0x200);
  } else if ((x & 0x3FF) == 0x200) {
    x = (x + 0x400) & ~0x7FF;
  }
  if (x & __SEGGER_RTL_U64_C(0x8000000000000000)) {
    x >>= 1;
    ++xx;
  }
  //
  // Drop guard bits.
  //
  x >>= 10;
  //
  // Remove hidden bit.
  //
  x &= ~__SEGGER_RTL_U64_C(0x0010000000000000);
  //
  // Handle overflow.
  //
  if (xx >= 0x7FF) {
    xx = 0x7FF;
    x  = 0;
  }
  //
  // Apply sign.
  //
  xx |= rs & 0x800;
  //
  // Pack significand and exponent.
  //
  return x | ((__SEGGER_RTL_U64)xx << FLOAT64_SIGNIFICAND_BITS);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_mul()
*
*  Function description
*    Multiply, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_mul(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x * y;
#else
  return __SEGGER_RTL_BitcastToF64(
           __SEGGER_RTL_float64_mul_soft(
             __SEGGER_RTL_BitcastToU64(x),
             __SEGGER_RTL_BitcastToU64(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_div_soft()
*
*  Function description
*    Divide, float.
*
*  Parameters
*    b - Dividend.
*    c - Divisor.
*
*  Return value
*    Quotient.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_div_soft(__SEGGER_RTL_U32 b, __SEGGER_RTL_U32 c) {
  unsigned bx;
  unsigned cx;
  unsigned rs;
  __SEGGER_RTL_U64 q;
  //
  bx = (unsigned)(b >> 23);
  cx = (unsigned)(c >> 23);
  rs = (bx ^ cx) & 0x100;
  bx &= 0xFF;
  cx &= 0xFF;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial_soft(b) || __SEGGER_RTL_float32_isspecial_soft(c))) {
    if (__SEGGER_RTL_float32_isnan_soft(b)) {
      return K_NAN_U32;
    } else if (__SEGGER_RTL_float32_isnan_soft(c)) {
      return K_NAN_U32;
    } else if (cx == 0) {  // Divide by zero
      if (bx == 0) {
        return K_NAN_U32;
      } else {
        return K_INF_U32 | ((__SEGGER_RTL_U32)rs << 23);
      }
    } else if (bx == 0) {
      return ((__SEGGER_RTL_U32)rs << 23);
    } else if (__SEGGER_RTL_float32_isinf_soft(b)) {
      if (__SEGGER_RTL_float32_isinf_soft(c)) {
        return K_NAN_U32;  // Inf/Inf -> NaN
      } else {
        return b ^ (c & FLOAT32_SIGN_MASK);
      }
    } else if (__SEGGER_RTL_float32_isinf_soft(c)) {
      return (__SEGGER_RTL_U32)rs << 23;
    }
  }
  //
  // Extract significand and set hidden bit.
  //
  b &= __SEGGER_RTL_U32_C(0x7FFFFF);
  b |= __SEGGER_RTL_U32_C(0x800000);
  c &= __SEGGER_RTL_U32_C(0x7FFFFF);
  c |= __SEGGER_RTL_U32_C(0x800000);
  //
  // Compute quotient's exponent.
  //
  bx = bx-cx + 0x7F;
  //
  // Divide in extended precision.
  //
  if (sizeof(long) == 4) {
    q = __SEGGER_RTL_Div64by32_Moeller(b << 7, c << 8);
  } else {
    q = ((__SEGGER_RTL_U64)b << 31) / c;
  }
  //
  // Round result.
  //
  if ((q & __SEGGER_RTL_U64_C(0x080000000)) == 0) {
    q <<= 1;
    --bx;
  }
  q += (q & 0x80) << 1;
  //
  // Check for overflow and underflow and return sensible numbers.
  //
  if ((__SEGGER_RTL_I16)bx <= 0) {
    bx = 0;
    q  = 0;
  } else if ((__SEGGER_RTL_I16)bx >= 0xFF) {
    bx = 0xFF;
    q  = 0;
  }
  //
  // Pack and return.
  //
  bx |= rs;
  return (__SEGGER_RTL_U32)((q >> 8) & ~__SEGGER_RTL_U32_C(0x800000)) | ((__SEGGER_RTL_U32)bx << 23);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_div()
*
*  Function description
*    Divide, float.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_div(float x, float y) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x / y;
#else
  return __SEGGER_RTL_BitcastToF32(
           __SEGGER_RTL_float32_div_soft(
             __SEGGER_RTL_BitcastToU32(x),
             __SEGGER_RTL_BitcastToU32(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_Div53by53()
*
*  Function description
*    53-bit by 63-bit fractional divide, specialized.
*
*  Parameters
*    u         - Dividend, 53 bits, "X".
*    v         - Divisor, 53 bits, "Y".
*    pExponent - Pointer to quotient exponent.
*
*  Return value
*    u / v.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div53by53(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v, int *pExponent) {
  __SEGGER_RTL_U32 Q;
  __SEGGER_RTL_U32 R;
  __SEGGER_RTL_U64 S;
  //
  // We require dividend > divisor, but this might not be the case.
  // Generating leading bit by trial subtraction.
  //
  // Shift dividend significand to have msb=1 at bit position 62.
  //
  if (u < v) {
    u <<= 1;
  } else {
    ++(*pExponent);
  }
  //
  u <<= 10;
  //
  // Let Q' = X/Y' be an approximation to X/Y where Y' is smaller than Y.
  // We test the correctness of the quotient estimate Q' by calculating
  // Q' * Y with Y to full precision.  If X < Q' * Y then Q' is overestimated
  // with the quotient too big and an adjustment must be made with Q' = X/Y' - 1.
  // 
  // This is simmple to code by calculating Q' := X/Y'; X := X - Q' * Y.
  // If X is nonnegative, then then Q' is exact.  If X is negative, then Q'
  // is too big and the adjustment Q := Q'-1 is made.  At the same time, the
  // dividend update X := X - Q' * Y must also be adjusted.  We require
  // X := X - (Q'-1) * Y but have X := X - Q' * Y, hence the correction
  // is X := X - Y (as X - (Q'+1)*Y is X - Q'*Y - Y).
  //
  // The true quotient is developed over five division steps, developing
  // 10, 10, 11, 11, 11 bits at a time to produce the 53-bit quotient
  // with a single rounding bit.
  //
  __SEGGER_RTL_DIVMOD_U32(Q, R, __SEGGER_RTL_U64_H(u), __SEGGER_RTL_U64_H(v));  // 10-bit estimate of quotient Q' using 31 bits of Y'.
  u  = __SEGGER_RTL_U64_MK(R, __SEGGER_RTL_U64_L(u)) - __SEGGER_RTL_UMULL_X(Q, __SEGGER_RTL_U64_L(v));
  if (u & __SEGGER_RTL_U64_C(0x8000000000000000)) {
    Q -= 1;
    u += v;
  }
  S = (__SEGGER_RTL_U64)Q << (10+32);
  //
  // Now continue with division.  The 10 leading bits of dividend are zero (after
  // the subtraction), and so the dividend is shifted 10 bits ready to generate the
  // following group of 10 quotient bits.
  //
  u <<= 10;
  __SEGGER_RTL_DIVMOD_U32(Q, R, __SEGGER_RTL_U64_H(u), __SEGGER_RTL_U64_H(v));  // 10-bit estimate of quotient Q' using 31 bits of Y'.
  u  = __SEGGER_RTL_U64_MK(R, __SEGGER_RTL_U64_L(u)) - __SEGGER_RTL_UMULL_X(Q, __SEGGER_RTL_U64_L(v));
  if (u & __SEGGER_RTL_U64_C(0x8000000000000000)) {
    Q -= 1;
    u += v;
  }
  S |= (__SEGGER_RTL_U64)Q << 32;
  //
  u <<= 11;
  __SEGGER_RTL_DIVMOD_U32(Q, R, __SEGGER_RTL_U64_H(u), __SEGGER_RTL_U64_H(v));  // 11-bit estimate of quotient Q' using 31 bits of Y'.
  u  = __SEGGER_RTL_U64_MK(R, __SEGGER_RTL_U64_L(u)) - __SEGGER_RTL_UMULL_X(Q, __SEGGER_RTL_U64_L(v));
  if (u & __SEGGER_RTL_U64_C(0x8000000000000000)) {
    Q -= 1;
    u += v;
  }
  S |= Q << 21;
  //
  u <<= 11;
  __SEGGER_RTL_DIVMOD_U32(Q, R, __SEGGER_RTL_U64_H(u), __SEGGER_RTL_U64_H(v));  // 11-bit estimate of quotient Q' using 31 bits of Y'.
  u  = __SEGGER_RTL_U64_MK(R, __SEGGER_RTL_U64_L(u)) - __SEGGER_RTL_UMULL_X(Q, __SEGGER_RTL_U64_L(v));
  if (u & __SEGGER_RTL_U64_C(0x8000000000000000)) {
    Q -= 1;
    u += v;
  }
  S |= Q << 10;
  //
  u <<= 11;
  __SEGGER_RTL_DIVMOD_U32(Q, R, __SEGGER_RTL_U64_H(u), __SEGGER_RTL_U64_H(v));  // 11-bit estimate of quotient Q' using 31 bits of Y'.
  u  = __SEGGER_RTL_U64_MK(R, __SEGGER_RTL_U64_L(u)) - __SEGGER_RTL_UMULL_X(Q, __SEGGER_RTL_U64_L(v));
  if (u & __SEGGER_RTL_U64_C(0x8000000000000000)) {
    Q -= 1;
  }
  //
  return (S << 1) | Q;  // Includes final rounding bit in bit #0
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_div_soft()
*
*  Function description
*    Divide, double.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float64_div_soft(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  __SEGGER_RTL_U64 sign;
  __SEGGER_RTL_U64 a;
  unsigned         xx;
  unsigned         yx;
  int              ax;
  //
  // Extract exponents and sign.
  //
  xx   = FLOAT64_EXPONENT(x);
  yx   = FLOAT64_EXPONENT(y);
  sign = (x ^ y) >> 63 << 63;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial_soft(x) || __SEGGER_RTL_float64_isspecial_soft(y))) {
    if (__SEGGER_RTL_float64_isnan_soft(x)) {
      return K_NAN_U64;
    } else if (__SEGGER_RTL_float64_isnan_soft(y)) {
      return K_NAN_U64;
    } else if (yx == 0) {  // Divide by zero
      if (xx == 0) {
        return K_NAN_U64;
      } else {
        return K_INF_U64 | sign;
      }
    } else if (xx == 0) {
      return sign;
    } else if (__SEGGER_RTL_float64_isinf_soft(x)) {
      if (__SEGGER_RTL_float64_isinf_soft(y)) {
        return K_NAN_U64;  // Inf/Inf -> NaN
      } else {
        return x ^ (y & __SEGGER_RTL_U64_C(0x8000000000000000));
      }
    } else if (__SEGGER_RTL_float64_isinf_soft(y)) {
      return sign;
    }
  }
  //
  // Extract significand and set hidden bit.
  //
  x &= FLOAT64_SIGNIFICAND_MASK;
  x |= FLOAT64_HIDDEN_MASK;
  y &= FLOAT64_SIGNIFICAND_MASK;
  y |= FLOAT64_HIDDEN_MASK;
  //
  ax = xx - yx + 0x3FD;
  a = 0;
  //
  // Divide.
  //
  if (sizeof(long) <= 4) {
    //
#if (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
    //
    // Synthesize division using schoolbook method and 32-bit division.
    //
    a = __SEGGER_RTL_Div53by53(x, y, &ax);
    //
#else
    //
    // Divide by clockwork.
    //
    int i;
    //
    for (i = 0; i < 55; ++i) {
      a <<= 1;
      if (x >= y) {
        x -= y;
        a |= 1;
      }
      x <<= 1;
    }
    //
#endif
    //
  } else {
    //
    // For 64-bit targets it's faster to divide using division instructions.
    //
    a = __SEGGER_RTL_Div128by64(x, y << 10);
  }
  //
  // Normalize.
  //
  if (a & __SEGGER_RTL_U64_C(0x0040000000000000)) {
    a >>= 1;
    ++ax;
  }
  //
  // Round.
  //
  if (a & 1) {
    ++a;
    if (a & __SEGGER_RTL_U64_C(0x0040000000000000)) {
      a >>= 1;
      ++ax;
    }
  }
  //
  // Check for overflow and underflow and return sensible numbers.
  //
  if (__SEGGER_RTL_UNLIKELY(ax < 0)) {
    //
    // Denormals flushed to zero.
    //
    return sign;
    //
  } else if (__SEGGER_RTL_UNLIKELY(ax >= 0x7FE)) {
    //
    // Overflow flushed to Inf.
    //
    return K_INF_U64 | sign;
    //
  } else {
    //
    // Combine sign, exponent, significand.
    //
    return ((__SEGGER_RTL_I64)ax << FLOAT64_SIGNIFICAND_BITS) + (a>>1) + sign;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_div()
*
*  Function description
*    Divide, double.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_div(double x, double y) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return x / y;
#else
  return __SEGGER_RTL_BitcastToF64(
           __SEGGER_RTL_float64_div_soft(
             __SEGGER_RTL_BitcastToU64(x),
             __SEGGER_RTL_BitcastToU64(y)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fma_inline()
*
*  Function description
*    Compute fused multiply-add, float.
*
*  Parameters
*    x - Multiplier.
*    y - Multiplicand.
*    z - Summand.
*
*  Return value
*    Return (x * y) + z.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_fma_inline(float x, float y, float z) {
#if defined(__SEGGER_RTL_FLOAT32_FMA)
  return __SEGGER_RTL_FLOAT32_FMA(x, y, z);
#else
  return SEGGER_ADDF(SEGGER_MULF(x, y), z);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_fma_inline()
*
*  Function description
*    Compute fused multiply-add, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*    z - Summand.
*
*  Return value
*    Return (x * y) + z.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_fma_inline(double x, double y, double z) {
#if defined(__SEGGER_RTL_FLOAT64_FMA)
  return __SEGGER_RTL_FLOAT64_FMA(x, y, z);
#else
  return SEGGER_ADD(SEGGER_MUL(x, y), z);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_int32_soft()
*
*  Function description
*    Convert float to int.
*
*  Parameters
*    x - Floating value to convert as a bitstring.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_I32 __SEGGER_RTL_float_to_int32_soft(__SEGGER_RTL_U32 x) {
  unsigned bs;
  unsigned bx;
  //
  // Extract exponent and sign.
  //
  bs = (unsigned)(x >> 23);
  bx = bs & 0xFF;
  //
  // (-1, 1) is truncated to zero.
  //
  if (bx < 0x7F) {
    return 0;
  } else if (bx >= 0x7F+31) {
    if ((x << 1) > __SEGGER_RTL_U32_C(0xFF000000)) {
      return __SEGGER_RTL_I32_C(0x7FFFFFFF);
    } else if (bs >= 0x100) {
      return __SEGGER_RTL_I32_C(0x80000000);
    } else {
      return __SEGGER_RTL_I32_C(0x7FFFFFFF);
    }
  }
  //
  // Extract significand and set hidden bit.
  //
  x &= __SEGGER_RTL_U32_C(0x7FFFFF);
  x |= __SEGGER_RTL_U32_C(0x800000);
  //
  // Align correctly.
  //
  if (bx <= 0x7F+23) {
    x >>= (0x7F+23) - bx;
  } else {
    x <<= (bx-(0x7F+23));
  }
  //
  // Apply sign.
  //
  if (bs >= 0x100) {
    x = 0u - x;
  }
  //
  return x;
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_int32()
*
*  Function description
*    Convert float to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_I32 __SEGGER_RTL_float_to_int32(float x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (__SEGGER_RTL_I32)x;
#else
  return __SEGGER_RTL_float_to_int32_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_int32_soft()
*
*  Function description
*    Convert double to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_I32 __SEGGER_RTL_double_to_int32_soft(__SEGGER_RTL_U64 x) {
  unsigned xs;
  unsigned xx;
  //
  // Extract exponent and sign.
  //
  xs = (unsigned)(x >> FLOAT64_SIGNIFICAND_BITS);
  xx = xs & 0x7FF;
  //
  // (-1, 1) is truncated to zero.
  //
  if (xx < 0x3FF) {
    return 0;
  } else if (xx >= 0x3FF+31) {
    if ((x << 1) > __SEGGER_RTL_U64_C(0xFFE0000000000000)) {
      return (__SEGGER_RTL_I32)0x7FFFFFFF;
    } else if (xs >= 0x800) {
      return (__SEGGER_RTL_I32)0x80000000;
    } else {
      return (__SEGGER_RTL_I32)0x7FFFFFFF;
    }
  }
  //
  // Extract significand and set hidden bit.
  //
  x &= FLOAT64_SIGNIFICAND_MASK;
  x |= FLOAT64_HIDDEN_MASK;
  //
  // Align correctly.
  //
  x >>= (0x3FF + FLOAT64_SIGNIFICAND_BITS) - xx;
  //
  // Apply sign.
  if (xs >= 0x800) {
    x = 0u - x;
  }
  //
  return (__SEGGER_RTL_I32)x;
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_int32()
*
*  Function description
*    Convert double to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_I32 __SEGGER_RTL_double_to_int32(double x) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (__SEGGER_RTL_I32)x;
#else
  return __SEGGER_RTL_double_to_int32_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_uint32_soft()
*
*  Function description
*    Convert float to unsigned.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float_to_uint32_soft(__SEGGER_RTL_U32 x) {
  unsigned xs;
  unsigned xx;
  //
  // Extract exponent.
  //
  xs = (unsigned)(x >> 23);
  xx = xs & 0xFF;
  //
  // [-Inf..1) is truncated to zero.
  //
  if (xx < 0x7F || x >= __SEGGER_RTL_U32_C(0x80000000)) {
    return 0;
  } else if (xx >= 0x7F+32) {
    return __SEGGER_RTL_U32_C(0xFFFFFFFF);  // Too large, saturate
  }
  //
  // Extract significand and set hidden bit.
  //
  x &= __SEGGER_RTL_U32_C(0x7FFFFF);
  x |= __SEGGER_RTL_U32_C(0x800000);
  //
  // Align correctly.
  //
  if (xx <= 0x7F+23) {
    x >>= (0x7F+23) - xx;
  } else {
    x <<= xx - (0x7F+23);
  }
  //
  // Apply sign.
  //
  return (__SEGGER_RTL_U32)x;
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_uint32()
*
*  Function description
*    Convert float to unsigned.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float_to_uint32(float x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (__SEGGER_RTL_U32)x;
#else
  return __SEGGER_RTL_float_to_uint32_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_uint32_soft()
*
*  Function description
*    Convert double to __SEGGER_RTL_U32.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_double_to_uint32_soft(__SEGGER_RTL_U64 x) {
  unsigned xs;
  unsigned xx;
  //
  // Extract exponent and sign.
  //
  xs = (unsigned)(x >> FLOAT64_SIGNIFICAND_BITS);
  xx = xs & 0x7FF;
  //
  // (-1, 1) is truncated to zero.
  //
  if (xx < 0x3FF) {
    return 0;
  } else if (xx >= 0x3FF+32) {
    if (xs >= 0x800) {
      return 0;
    } else {
      return __SEGGER_RTL_U32_C(0xFFFFFFFF);
    }
  } else if (xs >= 0x800) {
    return 0;
  }
  //
  // Extract significand and set hidden bit.
  //
  x &= FLOAT64_SIGNIFICAND_MASK;
  x |= FLOAT64_HIDDEN_MASK;
  //
  // Align correctly.
  //
  x >>= (0x3FF + FLOAT64_SIGNIFICAND_BITS) - xx;
  //
  // Apply sign.
  if (xs >= 0x800) {
    return __SEGGER_RTL_U32_C(0) - (__SEGGER_RTL_U32)x;
  } else {
    return (__SEGGER_RTL_U32)x;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_uint32()
*
*  Function description
*    Convert double to __SEGGER_RTL_U32.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_double_to_uint32(double x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (__SEGGER_RTL_U32)x;
#else
  return __SEGGER_RTL_double_to_uint32_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}
  
/*********************************************************************
*
*       __SEGGER_RTL_float_to_int64_soft()
*
*  Function description
*    Convert float to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    The RV32 compiler converts a float to a 64-bit integer
*    by calling runtime support to handle it.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_I64 __SEGGER_RTL_float_to_int64_soft(__SEGGER_RTL_U32 x) {
  unsigned xx;
  __SEGGER_RTL_U64 xl;
  //
  // Extract exponent and sign.
  //
  xx = (x >> 23) & 0xFF;
  //
  // (-1, 1) is truncated to zero.
  //
  if (xx < 0x7F) {
    return 0;
  } else if (xx >= 0x7F+63) {
    if ((x << 1) > __SEGGER_RTL_U32_C(0xFF000000)) {
      return __SEGGER_RTL_I64_C(0x7FFFFFFFFFFFFFFF);
    } else if (x & __SEGGER_RTL_U32_C(0x80000000)) {
      return __SEGGER_RTL_I64_C(0x8000000000000000);
    } else {
      return __SEGGER_RTL_I64_C(0x7FFFFFFFFFFFFFFF);
    }
  }
  //
  // Extract significand and set hidden bit.
  //
  xl = x & FLOAT32_SIGNIFICAND_MASK;
  xl |= FLOAT32_HIDDEN_MASK;
  //
  // Align correctly.
  //
  if (xx <= 0x7F+23) {
    xl >>= (0x7F+23) - xx;
  } else {
    xl <<= xx - (0x7F+23);
  }
  //
  // Apply sign.
  //
  if (x & __SEGGER_RTL_U32_C(0x80000000)) {
    xl = 0u - xl;
  }
  //
  return (__SEGGER_RTL_I64)xl;
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_int64()
*
*  Function description
*    Convert float to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    The RV32 compiler converts a float to a 64-bit integer
*    by calling runtime support to handle it.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_I64 __SEGGER_RTL_float_to_int64(float x) {
#if __SEGGER_RTL_FP_HW >= 1 && (__SEGGER_RTL_SIZEOF_INT == 8) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (__SEGGER_RTL_I64)x;
#else
  return __SEGGER_RTL_float_to_int64_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_uint64_soft()
*
*  Function description
*    Convert float to unsigned long long.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_U64 __SEGGER_RTL_float_to_uint64_soft(__SEGGER_RTL_U32 x) {
  unsigned         xx;
  __SEGGER_RTL_U64 xl;
  //
  // Extract exponent and sign.
  //
  xx = (unsigned)(x >> 23) & 0xFF;
  //
  // [-Inf, 1) is truncated to zero.
  //
  if (xx < 0x7F || x >= __SEGGER_RTL_U32_C(0x80000000)) {
    return 0;
  } else if (xx >= 0x7F+64) {
    return ~__SEGGER_RTL_U64_C(0);
  } 
  //
  // Extract significand and set hidden bit.
  //
  xl  = x & FLOAT32_SIGNIFICAND_MASK;
  xl |= FLOAT32_HIDDEN_MASK;
  //
  // Align correctly.
  //
  if (xx <= 0x7F+23) {
    xl >>= (0x7F+23) - xx;
  } else {
    xl <<= (xx-(0x7F+23));
  }
  //
  return (__SEGGER_RTL_U64)xl;
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_uint64()
*
*  Function description
*    Convert float to unsigned long long.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float_to_uint64(float x) {
#if __SEGGER_RTL_FP_HW >= 1 && (__SEGGER_RTL_SIZEOF_INT == 8) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (__SEGGER_RTL_U64)x;
#else
  return __SEGGER_RTL_float_to_uint64_soft(__SEGGER_RTL_BitcastToU32(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_int64_soft()
*
*  Function description
*    Convert double to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    RV32 always calls runtime for double to int64 conversion.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_I64 __SEGGER_RTL_double_to_int64_soft(__SEGGER_RTL_U64 x) {
  unsigned xs;
  unsigned xx;
  //
  // Extract exponent and sign.
  //
  xs = (unsigned)(x >> FLOAT64_SIGNIFICAND_BITS);
  xx = xs & 0x7FF;
  //
  // (-1, 1) is truncated to zero.
  if (xx < 0x3FF) {
    return 0;
  } else if (xx >= 0x3FF+63) {
    if ((x << 1) > __SEGGER_RTL_U64_C(0xFFE0000000000000)) {
      return __SEGGER_RTL_I64_C(0x7FFFFFFFFFFFFFFF);
    } else if (x & __SEGGER_RTL_U64_C(0x8000000000000000)) {
      return __SEGGER_RTL_I64_C(0x8000000000000000);
    } else {
      return __SEGGER_RTL_I64_C(0x7FFFFFFFFFFFFFFF);
    }
  }
  //
  // Extract significand and set hidden bit.
  //
  x &= FLOAT64_SIGNIFICAND_MASK;
  x |= FLOAT64_HIDDEN_MASK;
  //
  // Align correctly.
  if (xx <= 0x3FF + FLOAT64_SIGNIFICAND_BITS) {
    x >>= (0x3FF + FLOAT64_SIGNIFICAND_BITS) - xx;
  } else {
    x <<= xx - (0x3FF + FLOAT64_SIGNIFICAND_BITS);
  }
  //
  // Apply sign.
  //
  if (xs >= 0x800) {
    x = 0u - x;
  }
  //
  return x;
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_int64()
*
*  Function description
*    Convert double to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    RV32 always calls runtime for double to int64 conversion.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_I64 __SEGGER_RTL_double_to_int64(double x) {
#if __SEGGER_RTL_FP_HW >= 2 && (__SEGGER_RTL_SIZEOF_INT == 8) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (__SEGGER_RTL_I64)x;
#else
  return __SEGGER_RTL_double_to_int64_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_uint64_soft()
*
*  Function description
*    Convert double to unsigned long long.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_double_to_uint64_soft(__SEGGER_RTL_U64 x) {
  unsigned xs;
  unsigned xx;
  //
  // Extract exponent and sign.
  //
  xs = (unsigned)(x >> FLOAT64_SIGNIFICAND_BITS);
  xx = xs & 0x7FF;
  //
  // (-1, 1) is truncated to zero.
  //
  if (xx < 0x3FF) {
    return 0;
  } else if (xx > 0x3FF+63) {
    if (xs >= 0x800) {
      return 0;
    } else {
      return __SEGGER_RTL_U64_C(0xFFFFFFFFFFFFFFFF);
    }
  } else if (xs >= 0x800) {
    return 0;
  }
  //
  // Extract significand and set hidden bit.
  //
  x &= FLOAT64_SIGNIFICAND_MASK;
  x |= FLOAT64_HIDDEN_MASK;
  //
  // Align correctly.
  if (xx <= 0x3FF + FLOAT64_SIGNIFICAND_BITS) {
    x >>= (0x3FF + FLOAT64_SIGNIFICAND_BITS) - xx;
  } else {
    x <<= xx - (0x3FF + FLOAT64_SIGNIFICAND_BITS);
  }
  //
  return x;
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_uint64()
*
*  Function description
*    Convert double to __SEGGER_RTL_U64.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    RV32 always calls runtime for double to int64 conversion.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_double_to_uint64(double x) {
#if __SEGGER_RTL_FP_HW >= 2 && (__SEGGER_RTL_SIZEOF_INT == 8) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (__SEGGER_RTL_U64)x;
#else
  return __SEGGER_RTL_double_to_uint64_soft(__SEGGER_RTL_BitcastToU64(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_int32_to_float_soft()
*
*  Function description
*    Convert int to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_int32_to_float_soft(__SEGGER_RTL_U32 x) {
  __SEGGER_RTL_UINT_LEAST8_T  sign;
  __SEGGER_RTL_UINT_LEAST16_T exp;
  __SEGGER_RTL_U32            frac;
  //
  // Special case.
  //
  if (x == 0) {
    return 0;
  }
  // 
  // Cater for sign.
  //
  if (x & FLOAT32_SIGN_MASK) {
    x = 0u - x;
    sign = 1;
  } else {
    sign = 0;
  }
  //
  // Normalize.
  //
  if (x < 0x1000000) {
    //
    // Number is small.
    //
    exp = __SEGGER_RTL_CLZ_U32(x) - 8;
    x <<= exp;
    exp = 0x7F + 23 - 1 - exp;
  } else {
    //
    // Number is large.
    //
    exp = __SEGGER_RTL_CLZ_U32(x);
    x <<= exp;
    //
    // Round.
    //
    exp = 0x7F + 31 - 1 - exp;
    frac = x & 0xFF;
    x >>= 8;
    if (frac >= 0x80) {
      ++x;
      if (frac == 0x80) {
        x &= ~__SEGGER_RTL_U32_C(1);
      }
      if (x > __SEGGER_RTL_U32_C(0xFFFFFF)) {
        x >>= 1;
        ++exp;
      }
    }
  }
  //
  // Add sign to float.
  //
  if (sign) {
    x |= __SEGGER_RTL_U32_C(0x80000000);
  }
  // 
  // Return the floated value.
  //
  return x + ((__SEGGER_RTL_U32)exp << 23);
}

/*********************************************************************
*
*       __SEGGER_RTL_int32_to_float()
*
*  Function description
*    Convert int to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_int32_to_float(__SEGGER_RTL_I32 x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (float)x;
#else
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_int32_to_float_soft(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_uint32_to_float_soft()
*
*  Function description
*    Convert unsigned to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_uint32_to_float_soft(__SEGGER_RTL_U32 x) {
  __SEGGER_RTL_UINT_LEAST16_T exp;
  __SEGGER_RTL_UINT_LEAST8_T  frac;
  //
  // Special case.
  //
  if (x == 0) {
    return x;
  }
  //
  // Normalize.
  //
  if (x < 0x1000000) {
    //
    // Number is small.
    //
    exp = __SEGGER_RTL_CLZ_U32(x) - 8;
    x <<= exp;
    exp = 0x7F + 23 - 1 - exp;
    //
  } else {
    //
    // Number is large.
    //
    exp = __SEGGER_RTL_CLZ_U32(x);
    x <<= exp;
    //
    // Round.
    //
    frac = x & 0xFF;
    exp = 0x7F + 31 - 1 - exp;
    x >>= 8;
    if (frac >= 0x80) {
      ++x;
      if (frac == 0x80) {
        x &= ~__SEGGER_RTL_U32_C(1);
      }
    }
  }
  // 
  // Return the floated value.
  //
  return x + ((__SEGGER_RTL_U32)exp << 23);
}

/*********************************************************************
*
*       __SEGGER_RTL_uint32_to_float()
*
*  Function description
*    Convert unsigned to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_uint32_to_float(__SEGGER_RTL_U32 x) {
#if __SEGGER_RTL_FP_HW >= 1 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (float)x;
#else
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_uint32_to_float_soft(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_int32_to_double_soft()
*
*  Function description
*    Convert int to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_int32_to_double_soft(__SEGGER_RTL_U32 x) {
  __SEGGER_RTL_UINT_LEAST8_T sign;
  __SEGGER_RTL_U64           f;
  int                        exp;
  //
  // Special case.
  //
  if (x == 0) {
    return 0;
  }
  //  
  // Cater for sign.
  //
  if (x & FLOAT32_SIGN_MASK) {
    f = ((__SEGGER_RTL_U64)(0u - x)) << 21;
    sign = 1;
  } else {
    f = ((__SEGGER_RTL_U64)x) << 21;
    sign = 0;
  }
  //
  // Compute power of two for exponent.
  //
  exp = __SEGGER_RTL_float64_normalize(&f, 0x3FF + 31);
  //    
  // Mask off hidden bit.
  //
  f &= ~FLOAT64_HIDDEN_MASK;
  // 
  // Add sign to float.
  //
  if (sign) {
    f |= FLOAT64_SIGN_MASK;
  }
  // 
  // Return the floated value.
  //
  return f | (((__SEGGER_RTL_U64)exp) << FLOAT64_SIGNIFICAND_BITS);
}

/*********************************************************************
*
*       __SEGGER_RTL_int32_to_double()
*
*  Function description
*    Convert int to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_int32_to_double(__SEGGER_RTL_I32 x) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (double)x;
#else
  return __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_int32_to_double_soft(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_uint32_to_double_soft()
*
*  Function description
*    Convert unsigned to double.
*
*  Parameters
*    x - Unsigned value to convert.
*
*  Return value
*    Double value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_uint32_to_double_soft(__SEGGER_RTL_U32 x) {
  __SEGGER_RTL_UINT_LEAST16_T exp;
  __SEGGER_RTL_U64       f;
  //
  // Special case.
  //
  if (__SEGGER_RTL_UNLIKELY(x == 0)) {
    return 0;
  }
  // 
  // Compute power of two for exponent.
  //
  f = (__SEGGER_RTL_U64)x << 21;
  exp = __SEGGER_RTL_float64_normalize(&f, 0x3FF + 31 - 1);
  //
  // Return the floated value.
  //
  return f + ((__SEGGER_RTL_U64)exp << FLOAT64_SIGNIFICAND_BITS);
}

/*********************************************************************
*
*       __SEGGER_RTL_uint32_to_double()
*
*  Function description
*    Convert unsigned to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_uint32_to_double(__SEGGER_RTL_U32 x) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (double)x;
#else
  return __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_uint32_to_double_soft(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_int64_to_float_soft()
*
*  Function description
*    Convert long long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_int64_to_float_soft(__SEGGER_RTL_U64 x) {
  __SEGGER_RTL_UINT_LEAST8_T  sign;
  __SEGGER_RTL_UINT_LEAST16_T exp;
  __SEGGER_RTL_U64            frac;
  //
  // Special case.
  //
  if (x == 0) {
    return 0;
  }
  //
  // Cater for sign.
  //
  if (x & __SEGGER_RTL_U64_C(0x8000000000000000)) {
    x = 0u - x;
    sign = 1;
  } else {
    sign = 0;
  }
  //
  // Normalise.
  //
  if (x < __SEGGER_RTL_U64_C(0x1000000)) {
    //
    // Number is small.
    //
    exp = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_U64_L(x)) - 8;
    x <<= exp;
    exp = 0x7F + 23 - 1 - exp;
    //
  } else {
    //
    // Number is large.
    //
    exp = __SEGGER_RTL_CLZ_U64(x);
    x <<= exp;
    //
    // Shift into place with correct exponent.
    //
    frac = x << 23;
    if (frac & (0x8000000000000000)) {
      frac <<= 1;
      frac |= 1;
    } else {
      frac <<= 1;
    }
    exp = 0x7F + 63 - 1 - exp;
    x >>= 40;
    //
    // Round.
    //
    if (frac >= 0x8000000000000000) {
      ++x;
      if (frac == __SEGGER_RTL_U64_C(0x8000000000000000)) {
        x &= ~__SEGGER_RTL_U64_C(1);
      }
    }
  }
  //
  // Add sign to float.
  //
  if (sign) {
    x |= FLOAT32_SIGN_MASK;
  }
  // 
  // Return the floated value.
  //
  return (__SEGGER_RTL_U32)x + (((__SEGGER_RTL_U32)exp)<<23);
}

/*********************************************************************
*
*       __SEGGER_RTL_int64_to_float()
*
*  Function description
*    Convert long long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_int64_to_float(__SEGGER_RTL_I64 x) {
#if __SEGGER_RTL_FP_HW >= 1 && (__SEGGER_RTL_SIZEOF_INT == 8) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (double)x;
#else
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_int64_to_float_soft(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_int64_to_double_soft()
*
*  Function description
*    Convert long long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_int64_to_double_soft(__SEGGER_RTL_U64 x) {
  __SEGGER_RTL_UINT_LEAST16_T exp;
  __SEGGER_RTL_UINT_LEAST8_T  sign;
  __SEGGER_RTL_U32            frac;
  //
  // Special case.
  //
  if (x == 0) {
    return 0;
  }
  // 
  // Cater for sign.
  //
  if (x & FLOAT64_SIGN_MASK) {
    x = 0u - x;
    sign = 1;
  } else {
    sign = 0;
  }
  //
  // Normalise.
  //
  if (x < __SEGGER_RTL_U64_C(0x20000000000000)) {
    //
    // Number is small.
    //
    exp = __SEGGER_RTL_CLZ_U64(x) - 11;
    x <<= exp;
    exp = 0x3FF - 1 + FLOAT64_SIGNIFICAND_BITS - exp;
    //
  } else {
    //
    // Number is large.
    //
    exp = __SEGGER_RTL_CLZ_U64(x);
    x <<= exp;
    //
    // Shift into place with correct exponent.
    //
    frac = (__SEGGER_RTL_U32)x << (FLOAT64_SIGNIFICAND_BITS - 32);
    if (frac & __SEGGER_RTL_U32_C(0x80000000)) {
      frac <<= 1;
      frac |= 1;
    } else {
      frac <<= 1;
    }
    exp = 0x3FF + 63 - 1 - exp;
    x >>= 11;
    //
    // Round.
    //
    if (frac >= __SEGGER_RTL_U32_C(0x80000000)) {
      ++x;
      if (frac == __SEGGER_RTL_U32_C(0x80000000)) {
        x &= ~__SEGGER_RTL_U64_C(1);
      }
    }
  }
  //
  // Add sign to float.
  //
  if (sign) {
    x |= FLOAT64_SIGN_MASK;
  }
  // 
  // Return the floated value.
  //
  return x + ((__SEGGER_RTL_U64)exp << FLOAT64_SIGNIFICAND_BITS);
}

/*********************************************************************
*
*       __SEGGER_RTL_int64_to_double()
*
*  Function description
*    Convert long long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_int64_to_double(__SEGGER_RTL_I64 x) {
#if __SEGGER_RTL_FP_HW >= 2 && (__SEGGER_RTL_SIZEOF_INT == 8) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (double)x;
#else
  return __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_int64_to_double_soft(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_uint64_to_float_soft()
*
*  Function description
*    Convert unsigned long long to float.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Float value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_uint64_to_float_soft(__SEGGER_RTL_U64 x) {
  __SEGGER_RTL_UINT_LEAST16_T exp;
  __SEGGER_RTL_U64       frac;
  //
  // Special case.
  //
  if (x == 0) {
    return 0;
  }
  //
  // Normalize.
  //
  if (x < __SEGGER_RTL_U64_C(0x1000000)) {
    //
    // Number is small.
    //
    exp = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_U64_L(x)) - 8;
    x <<= exp;
    exp = 0x7F + 23 - 1 - exp;
    //
  } else {
    //
    // Number is large.
    //
    exp = __SEGGER_RTL_CLZ_U64(x);
    x <<= exp;
    //
    // Shift into place with correct exponent.
    //
    frac = x << 23;
    if (frac & __SEGGER_RTL_U64_C(0x8000000000000000)) {
      frac <<= 1;
      frac |= 1;
    } else {
      frac <<= 1;
    }
    exp = 0x7F + 63 - 1 - exp;
    x >>= 40;
    //
    // Round.
    //
    if (frac >= __SEGGER_RTL_U64_C(0x8000000000000000)) {
      ++x;
      if (frac == __SEGGER_RTL_U64_C(0x8000000000000000)) {
        x &= ~__SEGGER_RTL_U64_C(1);
      }
    }
  }
  // 
  // Return the floated value.
  //
  return (__SEGGER_RTL_U32)x + ((__SEGGER_RTL_U32)exp << 23);
}

/*********************************************************************
*
*       __SEGGER_RTL_uint64_to_float()
*
*  Function description
*    Convert unsigned long long to float.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Float value.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_uint64_to_float(__SEGGER_RTL_U64 x) {
#if __SEGGER_RTL_FP_HW >= 2 && (__SEGGER_RTL_SIZEOF_INT == 8) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (float)x;
#else
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_uint64_to_float_soft(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_uint64_to_double_soft()
*
*  Function description
*    Convert unsigned long long to double.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Double value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_uint64_to_double_soft(__SEGGER_RTL_U64 x) {
  __SEGGER_RTL_UINT_LEAST16_T exp;
  __SEGGER_RTL_U32       frac;
  //
  // Special case.
  //
  if (x == 0) {
    return 0;
  }
  //
  // Normalise.
  //
  if (x < __SEGGER_RTL_U64_C(0x20000000000000)) {
    //
    // Number is small.
    //
    exp = __SEGGER_RTL_CLZ_U64(x) - 11;
    x <<= exp;
    exp = 0x3FF - 1 + FLOAT64_SIGNIFICAND_BITS - exp;
  } else {
    //
    // Number is large.
    //
    exp = __SEGGER_RTL_CLZ_U64(x);
    x <<= exp;
    //
    // Shift into place with correct exponent.
    //
    frac = (__SEGGER_RTL_U32)x << (FLOAT64_SIGNIFICAND_BITS - 32);
    if (frac & __SEGGER_RTL_U32_C(0x80000000)) {
      frac <<= 1;
      frac  |= 1;
    } else {
      frac <<= 1;
    }
    exp = 0x3FF + 63 - 1 - exp;
    x >>= 11;
    //
    // Round.
    //
    if (frac >= __SEGGER_RTL_U32_C(0x80000000)) {
      ++x;
      if (frac == __SEGGER_RTL_U32_C(0x80000000)) {
        x &= ~__SEGGER_RTL_U64_C(1);
      }
    }
  }
  // 
  // Return the floated value.
  //
  return x + ((__SEGGER_RTL_U64)exp << FLOAT64_SIGNIFICAND_BITS);
}

/*********************************************************************
*
*       __SEGGER_RTL_uint64_to_double()
*
*  Function description
*    Convert unsigned long long to double.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Double value.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_uint64_to_double(__SEGGER_RTL_U64 x) {
#if __SEGGER_RTL_FP_HW >= 2 && (__SEGGER_RTL_SIZEOF_INT == 8) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (double)x;
#else
  return __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_uint64_to_double_soft(x));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_double_soft()
*
*  Function description
*    Extend float to double.
*
*  Parameters
*    x - Float value to extend.
*
*  Return value
*    Double value.
*/
static __SEGGER_RTL_U64 __SEGGER_RTL_float_to_double_soft(__SEGGER_RTL_U32 x) {
  __SEGGER_RTL_U32 xs;
  __SEGGER_RTL_U64 v;
  unsigned         xx;
  //
  // Extract exponent and sign.
  //
  xs = x & FLOAT32_SIGN_MASK;
  xx = (x >> 23) & 0xFF;
  //
  // Deal with special cases.
  //
  if (xx == 0) {
    return (__SEGGER_RTL_U64)xs << 32;
  } else if (xx == 0xFF) {
    //
    // Inf or NaN.
    //
    v = (__SEGGER_RTL_U64_C(0x7000000000000000) + ((__SEGGER_RTL_U64)x << 33 >> 4)) | ((__SEGGER_RTL_U64)xs << 32);
    if (x << 9) {
      v |= __SEGGER_RTL_U64_C(0x0008000000000000);  // NaN
    }
    return v;
  }
  //
  // Pack sign bit.
  //
  if (xs) {
    xx |= __SEGGER_RTL_U32_C(0x800);
  }
  //
  // Pack exponent, sign, and significand.
  //
  return ((__SEGGER_RTL_U64)(xx + (0x3FF - 0x7F)) << FLOAT64_SIGNIFICAND_BITS) |
         (__SEGGER_RTL_U64)(x & __SEGGER_RTL_U32_C(0x7FFFFF)) << (FLOAT64_SIGNIFICAND_BITS-FLOAT32_SIGNIFICAND_BITS);
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_double()
*
*  Function description
*    Extend float to double.
*
*  Parameters
*    x - Float value to extend.
*
*  Return value
*    Double value.
*/
static double __SEGGER_RTL_float_to_double(float x) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (double)x;
#else
  return __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_float_to_double_soft(__SEGGER_RTL_BitcastToU32(x)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_float_soft()
*
*  Function description
*    Truncate double to float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Float value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_double_to_float_soft(__SEGGER_RTL_U64 x) {
  __SEGGER_RTL_U32 frac;
  unsigned         xs;
  int              xx;
  __SEGGER_RTL_U32 r;
  //
  // Extract exponent and sign.
  //
  xs = (unsigned)(x >> FLOAT64_SIGNIFICAND_BITS);
  xx = xs & 0x7FF;
  //
  // Adjust exponent.
  //
  xx -= (0x3FF - 0x7F);
  //
  // Check for specials.
  //
  if (xx <= 0) {
    return (__SEGGER_RTL_U32)(x >> 32) & __SEGGER_RTL_U32_C(0x80000000);
  } else if (xx >= 0xFF) {
    if (__SEGGER_RTL_float64_isnan_soft(x)) {
      r = (__SEGGER_RTL_U32)(x << 4 >> 33);
      r |= __SEGGER_RTL_U32_C(0x00400000);
      if (xs >= 0x800) {
        r |= FLOAT32_SIGN_MASK;
      }
    } else {
      r = K_INF_U32 | (__SEGGER_RTL_U64_H(x) & FLOAT32_SIGN_MASK);
    }
    return r;
  }
  //
  // Extract significand.
  //
  r = (__SEGGER_RTL_U32)(x >> 29) & __SEGGER_RTL_I32_C(0x7FFFFF);  // 29 == 52-23
  frac = (__SEGGER_RTL_U32)x << 3;
  if (frac >= __SEGGER_RTL_U32_C(0x80000000)) {
    ++r;
    if (frac == __SEGGER_RTL_U32_C(0x80000000)) {
      r &= ~__SEGGER_RTL_U32_C(1);
    }
  }
  //
  // Apply sign.
  //
  if (xs >= 0x800) {
    xx |= 0x100;
  }
  //
  // Pack and return.
  //
  return r + ((__SEGGER_RTL_U32)xx << 23);
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_ldouble()
*
*  Function description
*    Extend double to long double.
*
*  Parameters
*    x - Double value to extend.
*
*  Return value
*    Long double value.
*/
static __SEGGER_RTL_INLINE long double __SEGGER_RTL_double_to_ldouble(double x) {
  //
  if (sizeof(double) == sizeof(long double)) {
    return x;
  } else {
    SEGGER_RTL_float128_t v;
    __SEGGER_RTL_U64      u;
    int                   exponent;
    //
    u = __SEGGER_RTL_BitcastToU64(x);
    exponent = FLOAT64_EXPONENT(u);
    //
    // Check for subnormal and overflow, clamp to signed zero and signed Inf.
    //
    if (exponent == 0) {
      v.part.hi = u & FLOAT64_SIGN_MASK;
      v.part.lo = 0;
    } else {
      //
      // Rebias exponent.
      //
      if (exponent == FLOAT64_EXPONENT_INF) {
        exponent = FLOAT128_EXPONENT_INF;
      } else {
        exponent -= FLOAT64_EXPONENT_BIAS;
        exponent += FLOAT128_EXPONENT_BIAS;
      }
      //
      v.part.hi  = u & FLOAT64_SIGN_MASK;
      v.part.hi |= (__SEGGER_RTL_U64)exponent << (FLOAT128_SIGNIFICAND_BITS - 64);
      v.part.hi |= (u & FLOAT64_SIGNIFICAND_MASK) >> (FLOAT64_SIGNIFICAND_BITS - (FLOAT128_SIGNIFICAND_BITS - 64));
      v.part.lo  = (u & FLOAT64_SIGNIFICAND_MASK) << (64 - (FLOAT64_SIGNIFICAND_BITS - (FLOAT128_SIGNIFICAND_BITS - 64)));
    }
    //
    return v.f;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_ldouble_to_double()
*
*  Function description
*    Truncate long double to double.
*
*  Parameters
*    x - Long double value to truncate.
*
*  Return value
*    Double value.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_ldouble_to_double(long double x) {
  //
  if (sizeof(double) == sizeof(long double)) {
    return x;
  } else {
    SEGGER_RTL_float128_t v;
    __SEGGER_RTL_U64      u;
    int                   exponent;
    //
    v.f = x;
    exponent = FLOAT128_EXPONENT_HI(v.part.hi);
    exponent -= FLOAT128_EXPONENT_BIAS;
    exponent += FLOAT64_EXPONENT_BIAS;
    //
    if (exponent <= 0) {
      //
      // Exponent underflow case.
      //
      u  = v.part.hi;
      u &= FLOAT128_SIGN_HI_MASK;
    } else { 
      if (exponent == FLOAT128_EXPONENT_INF - FLOAT128_EXPONENT_BIAS + FLOAT64_EXPONENT_BIAS) {
        //
        // Inf and NaN case...
        //
        exponent = FLOAT64_EXPONENT_INF;
        //
      } else if (exponent > FLOAT64_EXPONENT_INF) {
        //
        // Exponent overflow case...
        //
        exponent = FLOAT64_EXPONENT_INF;
        v.part.hi = 0;
        v.part.lo = 0;
      }
      //
      u  = v.part.hi;
      u &= FLOAT128_SIGN_HI_MASK;
      u |= (__SEGGER_RTL_U64)exponent << FLOAT64_SIGNIFICAND_BITS;
      u |= (v.part.hi & FLOAT128_SIGNIFICAND_HI_MASK) << (FLOAT64_SIGNIFICAND_BITS - (FLOAT128_SIGNIFICAND_BITS - 64));
      u |= v.part.lo >> (FLOAT128_SIGNIFICAND_BITS - FLOAT64_SIGNIFICAND_BITS);
    }
    //
    return __SEGGER_RTL_BitcastToF64(u);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_half_ieee_soft()
*
*  Function description
*    Truncate float to IEEE half-precision float.
*
*  Parameters
*    x - Float value to truncate.
*
*  Return value
*    Float value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U16 __SEGGER_RTL_float_to_half_ieee_soft(__SEGGER_RTL_U32 x) {
  __SEGGER_RTL_UINT_LEAST16_T half;
  __SEGGER_RTL_U32       sign;
  int                    shift;
  //
  // Extract sign and magnitude.
  //
  sign = x &  FLOAT32_SIGN_MASK;
  x    = x & ~FLOAT32_SIGN_MASK;
  //
  if (x >= K_INF_U32) {
    //
    // Handle NaN and Inf.
    //
    half = FLOAT16_EXPONENT_INF << FLOAT16_SIGNIFICAND_BITS;
    if (x > K_INF_U32) {
      half |= (x & FLOAT32_SIGNIFICAND_MASK) >> (FLOAT32_SIGNIFICAND_BITS - FLOAT16_SIGNIFICAND_BITS);
    }
  } else if (x - FLOAT32_MK(FLOAT32_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS + 1, 0, 0) < x - FLOAT32_MK(FLOAT32_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS + FLOAT16_EXPONENT_INF, 0, 0)) {
    //
    // Normal in half-float format: shift significand and adjust exponent
    //
    half  = x >> (FLOAT32_SIGNIFICAND_BITS - FLOAT16_SIGNIFICAND_BITS);
    half -= (__SEGGER_RTL_UINT_LEAST16_T)((FLOAT32_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS) << FLOAT16_SIGNIFICAND_BITS);
    //
    // Round to nearest.
    //
    x <<= (FLOAT32_TOTAL_BITS - FLOAT32_SIGNIFICAND_BITS + FLOAT16_SIGNIFICAND_BITS);
    if (x >= __SEGGER_RTL_U32_C(0x80000000)) {
      half += 1;
      if (x == __SEGGER_RTL_U32_C(0x80000000)) {
        half &= ~1;
      }
    }
  } else {
    //
    // Subnormal or zero.
    //
    shift = FLOAT32_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS - FLOAT32_EXPONENT(x) + 1;
    if (shift > FLOAT32_SIGNIFICAND_BITS) {
      //
      // Underflows to zero.
      //
      half = 0;
    } else {
      //
      // Subnormal.  Generate unrounded result.
      //
      x &= FLOAT32_SIGNIFICAND_MASK;
      x |= FLOAT32_HIDDEN_MASK;                                          // Make hidden bit explicit
      x = (x >> shift) | ((x << (FLOAT32_TOTAL_BITS - shift)) != 0);     // Shift right jamming
      half = x >> (FLOAT32_SIGNIFICAND_BITS - FLOAT16_SIGNIFICAND_BITS);
      //
      // Apply rounding.
      //
      x <<= (FLOAT32_TOTAL_BITS - FLOAT32_SIGNIFICAND_BITS + FLOAT16_SIGNIFICAND_BITS);
      if (x >= __SEGGER_RTL_U32_C(0x80000000)) {
        half += 1;
        if (x == __SEGGER_RTL_U32_C(0x80000000)) {
          half &= ~1;
        }
      }
    }
  }
  //
  // Merge sign.
  //
  return half | (sign >> (FLOAT32_TOTAL_BITS - FLOAT16_TOTAL_BITS));
}

/*********************************************************************
*
*       __SEGGER_RTL_float_to_half_ieee()
*
*  Function description
*    Truncate float to IEEE half-precision float.
*
*  Parameters
*    x - Float value to truncate.
*
*  Return value
*    Float value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U16 __SEGGER_RTL_float_to_half_ieee(float x) {
  return __SEGGER_RTL_float_to_half_ieee_soft(__SEGGER_RTL_BitcastToU32(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_half_ieee_soft()
*
*  Function description
*    Truncate double to IEEE half-precision float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Float value.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U16 __SEGGER_RTL_double_to_half_ieee_soft(__SEGGER_RTL_U64 x) {
  __SEGGER_RTL_UINT_LEAST16_T half;
  __SEGGER_RTL_U64       sign;
  int                    shift;
  //
  // Extract sign and magnitude.
  //
  sign = x &  FLOAT64_SIGN_MASK;
  x    = x & ~FLOAT64_SIGN_MASK;
  //
  if (x >= K_INF_U64) {
    //
    // Handle NaN and Inf.
    //
    half = FLOAT16_EXPONENT_INF << FLOAT16_SIGNIFICAND_BITS;
    if (x > K_INF_U64) {
      half |= (x & FLOAT64_SIGNIFICAND_MASK) >> (FLOAT64_SIGNIFICAND_BITS - FLOAT16_SIGNIFICAND_BITS);
    }
  } else if (x - FLOAT64_MK(FLOAT64_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS + 1, 0, 0) < x - FLOAT64_MK(FLOAT64_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS + FLOAT16_EXPONENT_INF, 0, 0)) {
    //
    // Normal in half-float format: shift significand and adjust exponent
    //
    half  = (__SEGGER_RTL_UINT_LEAST16_T)(x >> (FLOAT64_SIGNIFICAND_BITS - FLOAT16_SIGNIFICAND_BITS));
    half -= (__SEGGER_RTL_UINT_LEAST16_T)((FLOAT64_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS) << FLOAT16_SIGNIFICAND_BITS);
    //
    // Round to nearest.
    //
    x <<= (FLOAT64_TOTAL_BITS - FLOAT64_SIGNIFICAND_BITS + FLOAT16_SIGNIFICAND_BITS);
    if (x >= __SEGGER_RTL_U64_C(0x8000000000000000)) {
      half += 1;
      if (x == __SEGGER_RTL_U64_C(0x8000000000000000)) {
        half &= ~1;
      }
    }
  } else {
    //
    // Subnormal or zero.
    //
    shift = FLOAT64_EXPONENT_BIAS - FLOAT16_EXPONENT_BIAS - FLOAT64_EXPONENT(x) + 1;
    if (shift > FLOAT64_SIGNIFICAND_BITS) {
      //
      // Underflows to zero.
      //
      half = 0;
    } else {
      //
      // Subnormal.  Generate unrounded result.
      //
      x &= FLOAT64_SIGNIFICAND_MASK;
      x |= FLOAT64_HIDDEN_MASK;                                          // Make hidden bit explicit
      x = (x >> shift) | ((x << (FLOAT64_TOTAL_BITS - shift)) != 0);     // Shift right jamming
      half = (__SEGGER_RTL_UINT_LEAST16_T)(x >> (FLOAT64_SIGNIFICAND_BITS - FLOAT16_SIGNIFICAND_BITS));
      //
      // Apply rounding.
      //
      x <<= (FLOAT64_TOTAL_BITS - FLOAT64_SIGNIFICAND_BITS + FLOAT16_SIGNIFICAND_BITS);
      if (x >= __SEGGER_RTL_U64_C(0x8000000000000000)) {
        half += 1;
        if (x == __SEGGER_RTL_U64_C(0x8000000000000000)) {
          half &= ~1;
        }
      }
    }
  }
  //
  // Merge sign.
  //
  return half | (__SEGGER_RTL_U16)(sign >> (FLOAT64_TOTAL_BITS - FLOAT16_TOTAL_BITS));
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_half_ieee()
*
*  Function description
*    Truncate double to IEEE half-precision float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Float value.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U16 __SEGGER_RTL_double_to_half_ieee(double x) {
  return __SEGGER_RTL_double_to_half_ieee_soft(__SEGGER_RTL_BitcastToU64(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_double_to_float()
*
*  Function description
*    Truncate double to float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Float value.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_double_to_float(double x) {
#if __SEGGER_RTL_FP_HW >= 2 && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return (float)x;
#else
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_double_to_float_soft(__SEGGER_RTL_BitcastToU64(x)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_half_to_float_ieee_soft()
*
*  Function description
*    Convert IEEE half-precision float to float.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Single-precision float.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_half_to_float_ieee_soft(__SEGGER_RTL_U16 x) {
  __SEGGER_RTL_U32 exp;
  __SEGGER_RTL_U32 significand;
  __SEGGER_RTL_U32 sign;
  int              q0;
  //
  // Extract sign, exponent, and significand.
  //
  exp         = (x >> 10) & 0x1F;
  significand = (x & __SEGGER_RTL_U32_C(0x03FF)) << (23-10);
  sign        = (x & __SEGGER_RTL_U32_C(0x8000)) << 16;
  //
  if (__SEGGER_RTL_UNLIKELY(exp == 0x7C00)) {
    //
    // Handle Inf and NaN.
    //
    if (significand == 0) {
      return __SEGGER_RTL_BitcastToF32(K_INF_U32 | sign);
    } else {
      return __SEGGER_RTL_BitcastToF32(K_NAN_U32 | sign);
    }
  } else if (exp == 0) {
    //
    // Handle subnormal.
    //
    q0            = __SEGGER_RTL_CLZ_U32(significand) - 8;
    exp          -= q0;
    significand <<= q0;
  }
  //
  return __SEGGER_RTL_BitcastToF32(((exp + 0x7F-15) << 23) + significand + sign);
}

/*********************************************************************
*
*       __SEGGER_RTL_half_to_double_ieee_soft()
*
*  Function description
*    Convert IEEE half-precision float to double.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Double-precision float.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_half_to_double_ieee_soft(__SEGGER_RTL_U16 x) {
  __SEGGER_RTL_U32 exp;
  __SEGGER_RTL_U32 significand;
  __SEGGER_RTL_U32 sign;
  int              q0;
  //
  // Extract sign, exponent, and significand.
  //
  exp         = (x >> 10) & 0x1F;
  significand = (x & __SEGGER_RTL_U32_C(0x03FF)) << (52-32-10);
  sign        = (x & __SEGGER_RTL_U32_C(0x8000)) << 16;
  //
  if (__SEGGER_RTL_UNLIKELY(exp == 0x7C00)) {
    //
    // Handle Inf and NaN.
    //
    if (significand == 0) {
      return __SEGGER_RTL_BitcastToF64(K_INF_U64 | sign);
    } else {
      return __SEGGER_RTL_BitcastToF64(K_NAN_U64 | sign);
    }
  } else if (exp == 0) {
    //
    // Handle subnormal.
    //
    q0            = __SEGGER_RTL_CLZ_U32(significand) - 11;
    exp          -= q0;
    significand <<= q0;
  }
  //
  return __SEGGER_RTL_BitcastToF64((__SEGGER_RTL_U64)(((exp + 0x3FF-15) << (52-32)) + significand + sign) << 32);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_PolyEvalP()
*
*  Function description
*    Evaluate rational P polynomial, float.
*
*  Additional information
*    Evaluates (...((g * k[n]) + k[n-1]) * g + k[n-2]...) * g
*/
static float __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float32_PolyEvalP(const float *k, float g, int n) {
  float w;
  //
  n -= 2;
  k += n;
  w = SEGGER_FMAF(k[1], g, k[0]);
  while (--n >= 0) {
    w = SEGGER_FMAF(w, g, *--k);
  }
  return SEGGER_MULF(w, g);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_PolyEvalP_3()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_PolyEvalP_3(float g, const float *pCoeff, float P1, float P2, float P3) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  //
  return __SEGGER_RTL_float32_PolyEvalP(pCoeff, g, 3);
#else
  float w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_FMAF(P3, g, P2);
  w = SEGGER_FMAF(w, g, P1);
  w = SEGGER_MULF(w, g);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_PolyEvalP_4()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_PolyEvalP_4(float g, const float *pCoeff, float P1, float P2, float P3, float P4) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  //
  return __SEGGER_RTL_float32_PolyEvalP(pCoeff, g, 4);
#else
  float w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_FMAF(P4, g, P3);
  w = SEGGER_FMAF(w, g, P2);
  w = SEGGER_FMAF(w, g, P1);
  w = SEGGER_MULF(w, g);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_PolyEvalP_5()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_PolyEvalP_5(float g, const float *pCoeff, float P1, float P2, float P3, float P4, float P5) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  __SEGGER_RTL_USE_PARA(P5);
  //
  return __SEGGER_RTL_float32_PolyEvalP(pCoeff, g, 5);
#else
  float w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_FMAF(P5, g, P4);
  w = SEGGER_FMAF(w, g, P3);
  w = SEGGER_FMAF(w, g, P2);
  w = SEGGER_FMAF(w, g, P1);
  w = SEGGER_MULF(w, g);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalP()
*
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_PolyEvalP(const double *k, int n, double g) {
  double w;
  //
  n -= 2;
  k += n;
  w = SEGGER_FMA(k[1], g, k[0]);
  while (--n >= 0) {
    w = SEGGER_FMA(w, g, *--k);
  }
  return SEGGER_MUL(w, g);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalP_2()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_PolyEvalP_2(double g, const double *pCoeff, double P1, double P2) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  //
  return __SEGGER_RTL_float64_PolyEvalP(pCoeff, 2, g);
#else
  double w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_FMA(P2, g, P1);
  w = SEGGER_MUL(w, g);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalP_3()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_PolyEvalP_3(double g, const double *pCoeff, double P1, double P2, double P3) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  //
  return __SEGGER_RTL_float64_PolyEvalP(pCoeff, 3, g);
#else
  double w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_FMA(P3, g, P2);
  w = SEGGER_FMA(w, g, P1);
  w = SEGGER_MUL(w, g);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalP_4()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_PolyEvalP_4(double g, const double *pCoeff, double P1, double P2, double P3, double P4) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  //
  return __SEGGER_RTL_float64_PolyEvalP(pCoeff, 4, g);
#else
  double w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_FMA(P4, g, P3);
  w = SEGGER_FMA(w, g, P2);
  w = SEGGER_FMA(w, g, P1);
  w = SEGGER_MUL(w, g);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalP_5()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_PolyEvalP_5(double g, const double *pCoeff, double P1, double P2, double P3, double P4, double P5) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  __SEGGER_RTL_USE_PARA(P5);
  //
  return __SEGGER_RTL_float64_PolyEvalP(pCoeff, 5, g);
#else
  double w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_FMA(P5, g, P4);
  w = SEGGER_FMA(w, g, P3);
  w = SEGGER_FMA(w, g, P2);
  w = SEGGER_FMA(w, g, P1);
  w = SEGGER_MUL(w, g);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalP_8()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_PolyEvalP_8(double g, const double *pCoeff, double P1, double P2, double P3, double P4, double P5, double P6, double P7, double P8) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  __SEGGER_RTL_USE_PARA(P5);
  __SEGGER_RTL_USE_PARA(P6);
  __SEGGER_RTL_USE_PARA(P7);
  __SEGGER_RTL_USE_PARA(P8);
  //
  return __SEGGER_RTL_float64_PolyEvalP(pCoeff, 8, g);
#else
  double w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_FMA(P8, g, P7);
  w = SEGGER_FMA(w, g, P6);
  w = SEGGER_FMA(w, g, P5);
  w = SEGGER_FMA(w, g, P4);
  w = SEGGER_FMA(w, g, P3);
  w = SEGGER_FMA(w, g, P2);
  w = SEGGER_FMA(w, g, P1);
  w = SEGGER_MUL(w, g);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_PolyEvalQ()
*
*  Function description
*    Evaluate rational Q polynomial, float.
*
*  Additional information
*    Evaluates ((g + k[n]) * g + k[n-1]) * g + k[n-2]...
*/
static float __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float32_PolyEvalQ(const float *k, float g, int n) {
  float w;
  //
  n -= 1;
  w = SEGGER_ADDF(g, k[n]);
  while (--n >= 0) {
    w = SEGGER_FMAF(w, g, k[n]);
  }
  return w;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_PolyEvalQ_3()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_PolyEvalQ_3(float g, const float *pCoeff, float P1, float P2, float P3) {
#if __SEGGER_RTL_FP_HW == 0 || __SEGGER_RTL_OPTIMIZE <= 0
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  //
  return __SEGGER_RTL_float32_PolyEvalQ(pCoeff, g, 3);
#else
  float w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_ADDF(g, P3);
  w = SEGGER_FMAF(w, g, P2);
  w = SEGGER_FMAF(w, g, P1);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_PolyEvalQ_4()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_PolyEvalQ_4(float g, const float *pCoeff, float P1, float P2, float P3, float P4) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  //
  return __SEGGER_RTL_float32_PolyEvalQ(pCoeff, g, 4);
#else
  float w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_ADDF(g, P4);
  w = SEGGER_FMAF(w, g, P3);
  w = SEGGER_FMAF(w, g, P2);
  w = SEGGER_FMAF(w, g, P1);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_PolyEvalQ_5()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_PolyEvalQ_5(float g, const float *pCoeff, float P1, float P2, float P3, float P4, float P5) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  __SEGGER_RTL_USE_PARA(P5);
  //
  return __SEGGER_RTL_float32_PolyEvalQ(pCoeff, g, 5);
#else
  float w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_ADDF(g, P5);
  w = SEGGER_FMAF(w, g, P4);
  w = SEGGER_FMAF(w, g, P3);
  w = SEGGER_FMAF(w, g, P2);
  w = SEGGER_FMAF(w, g, P1);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalQ()
*
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_PolyEvalQ(const double *k, int n, double g) {
  double w;
  //
  k += n;
  w  = g;
  while (--n > 0) {
    w = SEGGER_MUL(SEGGER_ADD(w, *--k), g);
  }
  return SEGGER_ADD(w, k[-1]);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalQ_3()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_PolyEvalQ_3(double g, const double *pCoeff, double P1, double P2, double P3) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  //
  return __SEGGER_RTL_float64_PolyEvalQ(pCoeff, 3, g);
#else
  double w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_ADD(g, P3);
  w = SEGGER_FMA(w, g, P2);
  w = SEGGER_FMA(w, g, P1);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalQ_4()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_PolyEvalQ_4(double g, const double *pCoeff, double P1, double P2, double P3, double P4) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  //
  return __SEGGER_RTL_float64_PolyEvalQ(pCoeff, 4, g);
#else
  double w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_ADD(g, P4);
  w = SEGGER_FMA(w, g, P3);
  w = SEGGER_FMA(w, g, P2);
  w = SEGGER_FMA(w, g, P1);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_PolyEvalQ_5()
*
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_PolyEvalQ_5(double g, const double *pCoeff, double P1, double P2, double P3, double P4, double P5) {
#if (__SEGGER_RTL_FP_HW == 0) || (__SEGGER_RTL_OPTIMIZE <= 0)
  //
  __SEGGER_RTL_USE_PARA(P1);
  __SEGGER_RTL_USE_PARA(P2);
  __SEGGER_RTL_USE_PARA(P3);
  __SEGGER_RTL_USE_PARA(P4);
  __SEGGER_RTL_USE_PARA(P5);
  //
  return __SEGGER_RTL_float64_PolyEvalQ(pCoeff, 5, g);
#else
  double w;
  //
  __SEGGER_RTL_USE_PARA(pCoeff);
  //
  w = SEGGER_ADD(g, P5);
  w = SEGGER_FMA(w, g, P4);
  w = SEGGER_FMA(w, g, P3);
  w = SEGGER_FMA(w, g, P2);
  w = SEGGER_FMA(w, g, P1);
  //
  return w;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_abs_soft()
*
*  Function description
*    Compute absolute value, double.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_abs_soft(__SEGGER_RTL_U32 x) {
  return x & ~FLOAT32_SIGN_MASK;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_abs_inline()
*
*  Function description
*    Compute absolute value, double.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_abs_inline(float x) {
  //
#if defined(__SEGGER_RTL_FLOAT32_ABS)
  //
  return __SEGGER_RTL_FLOAT32_ABS(x);
  //
#else
  //
  // NOTE: It is not possible to use the expression "x < 0 ? -x : x" or something
  // similar as +0 and -0 both compare as equal to 0.  The ISO standard mandates
  // that fabs(-0) = +0 so we really must take care to remove the sign.  You would
  // need to use 'x == 0 ? 0 : x < 0 ? -x : x'.
  //
  return __SEGGER_RTL_BitcastToF32(
           __SEGGER_RTL_float32_abs_soft(
             __SEGGER_RTL_BitcastToU32(x)));
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_abs_soft()
*
*  Function description
*    Compute absolute value, double.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_float64_abs_soft(__SEGGER_RTL_U64 x) {
  return x & ~FLOAT64_SIGN_MASK;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_abs_inline()
*
*  Function description
*    Compute absolute value, double.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_abs_inline(double x) {
  //
#if defined(__SEGGER_RTL_FLOAT64_ABS) && !__SEGGER_RTL_FORCE_SOFT_FLOAT
  return __SEGGER_RTL_FLOAT64_ABS(x);
#else
  //
  // NOTE: It is not possible to use the expression "x < 0 ? -x : x" or something
  // similar as +0 and -0 both compare as equal to 0.  The ISO standard mandates
  // that fabs(-0) = +0 so we really must take care to remove the sign.  You would
  // need to use 'x == 0 ? 0 : x < 0 ? -x : x'.
  //
  return __SEGGER_RTL_BitcastToF64(
           __SEGGER_RTL_float64_abs_soft(
             __SEGGER_RTL_BitcastToU64(x)));
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ldexp_inline()
*
*  Function description
*    Scale by power of two, double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is zero, return x;
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2 ^ n.
*
*  See also
*    scalbn()
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_ldexp_inline(double x, int n) {
  __SEGGER_RTL_U64 u;
  int              exp;
  //
  u   = __SEGGER_RTL_BitcastToU64(x);
  exp = FLOAT64_EXPONENT(u);
  //
  if (__SEGGER_RTL_UNLIKELY(exp == 0x7FF || exp == 0x000)) {
    return x;
  }
  //
  exp += n;
  if (exp >= 0x7FF) {
    u = K_INF_U64 | (u & FLOAT64_SIGN_MASK);
  } else if (exp <= 0) {
    u &= FLOAT64_SIGN_MASK;
  } else {
    u = (u & ~FLOAT64_EXPONENT_MASK) | ((__SEGGER_RTL_U64)exp << FLOAT64_SIGNIFICAND_BITS);
  }
  //
  return __SEGGER_RTL_BitcastToF64(u);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ldexp_outline()
*
*  Function description
*    Scale by power of two, double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is zero, return x;
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2 ^ n.
*
*  See also
*    scalbn()
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_ldexp_outline(double x, int n) {
  return __SEGGER_RTL_float64_ldexp_inline(x, n);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ldexp_inline()
*
*  Function description
*    Scale by power of two, float.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is zero, return x;
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2^n.
*
*  See also
*    scalbnf()
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_ldexp_inline(float x, int n) {
  __SEGGER_RTL_U32 x0;
  int              exp;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  exp = FLOAT32_EXPONENT(x0);
  //
  if (__SEGGER_RTL_UNLIKELY((unsigned)exp-1 >= 0xFE)) {
    return x;
  }
  //
  n += exp;
  if ((unsigned)n-1u < 0xFE) {
    //
    // Normal
    //
    x0 &= ~FLOAT32_EXPONENT_MASK;
    x0 |= (__SEGGER_RTL_U32)n << FLOAT32_SIGNIFICAND_BITS;
  } else if (__SEGGER_RTL_UNLIKELY(n <= 0)) {
    //
    // Underflowed
    //
    x0 &= FLOAT32_SIGN_MASK;
  } else {
    //
    // Overflowed
    //
    x0 = K_INF_U32 | (x0 & FLOAT32_SIGN_MASK);
  }
  //
  return __SEGGER_RTL_BitcastToF32(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ldexp_outline()
*
*  Function description
*    Scale by power of two, float.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is zero, return x;
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2^n.
*
*  See also
*    scalbnf()
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_ldexp_outline(float x, int n) {
  return __SEGGER_RTL_float32_ldexp_inline(x, n);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_frexp_inline()
*
*  Function description
*    Set exponent, float.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_frexp_inline(float x, int *exp) {
  __SEGGER_RTL_U32 x0;
  unsigned         q0;
  //
  // Special case 0.
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  q0 = FLOAT32_EXPONENT(x0);
  //
  // Special case 0, Inf, NaN.
  //
  if (__SEGGER_RTL_UNLIKELY(q0 == 0 || q0 == 0xFF)) {
    *exp = 0;
    return x;
  } else {
    //
    *exp = q0 - 0x7E;
    x0 &= ~__SEGGER_RTL_U32_C(0x7F800000);
    x0 |=  __SEGGER_RTL_U32_C(0x3F000000);
    //
    return __SEGGER_RTL_BitcastToF32(x0);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_frexp_outline()
*
*  Function description
*    Split to significand and exponent, float.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_frexp_outline(float x, int *exp) {
  return __SEGGER_RTL_float32_frexp_inline(x, exp);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_frexp_inline()
*
*  Function description
*    Split to significand and exponent, double.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_frexp_inline(double x, int *exp) {
  __SEGGER_RTL_U64 x0;
  unsigned         q0;
  //
  x0 = __SEGGER_RTL_BitcastToU64(x);
  q0 = FLOAT64_EXPONENT(x0);
  //
  // Special case 0, Inf, NaN.
  //
  if (q0 == 0 || q0 == 0x7FF) {
    *exp = 0;
    return x;
  } else {
    //
    *exp = q0 - 0x3FE;
    x0 &= ~__SEGGER_RTL_U32_C(0x7FF0000000000000);
    x0 |=  __SEGGER_RTL_U32_C(0x3FE0000000000000);
    //
    return __SEGGER_RTL_BitcastToF64(x0);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_frexp_outline()
*
*  Function description
*    Set exponent, double.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_frexp_outline(double x, int *exp) {
  return __SEGGER_RTL_float64_frexp_inline(x, exp);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_logb_inline()
*
*  Function description
*    Radix-indpendent exponent, float.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return -Inf.
*    * If x is infinite, return +Inf.
*    * If x is NaN, return NaN.
*    * Else, return integer part of log[FLTRADIX](x).
*
*  Additional information
*    Calculates the exponent of x, which is the integral part of
*    the FLTRADIX-logarithm of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_logb_inline(float x) {
  int q0;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(x))) {
    if (__SEGGER_RTL_float32_isinf_inline(x)) {
      return K_INF_F32;
    } else if (__SEGGER_RTL_float32_isnan_inline(x)) {
      return K_NAN_F32;
    } else {
      return __SEGGER_RTL_BitcastToF32(K_INF_U32 | FLOAT32_SIGN_MASK);
    }
  }
  //
  q0 = FLOAT32_EXPONENT(__SEGGER_RTL_BitcastToU32(x));
  //
  return SEGGER_I2F(q0 - FLOAT32_EXPONENT_BIAS);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_logb_inline()
*
*  Function description
*    Radix-indpendent exponent, double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return -Inf.
*    * If x is infinite, return +Inf.
*    * If x is NaN, return NaN.
*    * Else, return integer part of log[FLTRADIX](x).
*
*  Additional information
*    Calculates the exponent of x, which is the integral part of
*    the FLTRADIX-logarithm of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_logb_inline(double x) {
  int q0;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    if (__SEGGER_RTL_float64_isinf_inline(x)) {
      return K_INF_F64;
    } else if (__SEGGER_RTL_float64_isnan_inline(x)) {
      return K_NAN_F64;
    } else {
      return __SEGGER_RTL_BitcastToF64(K_INF_U64 | FLOAT64_SIGN_MASK);
    }
  }
  //
  q0 = FLOAT64_EXPONENT(__SEGGER_RTL_BitcastToU64(x));
  //
  return SEGGER_I2D(q0 - FLOAT64_EXPONENT_BIAS);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ilogb_inline()
*
*  Function description
*    Binary logarithm, double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float32_ilogb_inline(float x) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_putative_iszero(x))) {
    return K_FP_ILOGB0;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(x))) {
    return K_FP_ILOGBNAN;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isinf_inline(x))) {
    return K_INT_MAX;
  } else {
    return FLOAT32_EXPONENT(__SEGGER_RTL_BitcastToU32(x)) - FLOAT32_EXPONENT_BIAS;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ilogb_outline()
*
*  Function description
*    Binary logarithm, double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*/
static __SEGGER_RTL_NEVER_INLINE int __SEGGER_RTL_float32_ilogb_outline(float x) {
  return __SEGGER_RTL_float32_ilogb_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ilogb_inline()
*
*  Function description
*    Binary logarithm, double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_float64_ilogb_inline(double x) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_putative_iszero(x))) {
    return K_FP_ILOGB0;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(x))) {
    return K_FP_ILOGBNAN;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isinf_inline(x))) {
    return K_INT_MAX;
  } else {
    return FLOAT64_EXPONENT(__SEGGER_RTL_BitcastToU64(x)) - FLOAT64_EXPONENT_BIAS;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ilogb_outline()
*
*  Function description
*    Binary logarithm, double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*/
static __SEGGER_RTL_NEVER_INLINE int __SEGGER_RTL_float64_ilogb_outline(double x) {
  return __SEGGER_RTL_float64_ilogb_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ceil_inline()
*
*  Function description
*    Compute smallest integer not less than, float.
*
*  Parameters
*    x - Value to compute ceiling of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the smallest integer value not greater than x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_ceil_inline(float x) {
  __SEGGER_RTL_U32 mask;
  __SEGGER_RTL_U32 exact;
  __SEGGER_RTL_U32 x0;
  int              exponent;
  //
  // Get exponent.
  //
  x0       = __SEGGER_RTL_BitcastToU32(x);
  exponent = FLOAT32_EXPONENT(x0);
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(x))) {
    if (exponent == 0) {
      x0 &= FLOAT32_SIGN_MASK;
    }
    return __SEGGER_RTL_BitcastToF32(x0);
  }
  //
  exponent -= 0x7F;
  if (exponent < 0) {
    //
    // 0 < |x| <= 1.0
    //
    if (FLOAT32_SIGN(x0)) {
      return -0.0f;     // x < 0, so round up to zero.
    } else {
      return 1.0f;     // 0 < x, so round up to one.
    }
  } else if (exponent >= 23) {
    //
    // Too large to round up by one unit; addition of a
    // unit is insignificant.
    //
    return x;
  }
  //
  // Bit mask for fractional bits.
  //
  mask = FLOAT32_SIGNIFICAND_MASK >> exponent;
  //
  // Extract fractional bits.
  //
  exact = x0 & mask;
  //
  // Remove fractional bits.
  //
  x0 &= ~mask;
  //
  // Add 1.0 for positive non-integers, ceil(1.1) = 2.
  //
  if (exact && FLOAT32_SIGN(x0) == 0) {
    x0 += __SEGGER_RTL_U32_C(1) << 23 >> exponent;
  }
  //
  return __SEGGER_RTL_BitcastToF32(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ceil_inline()
*
*  Function description
*    Compute smallest integer not less than, double.
*
*  Parameters
*    x - Value to compute ceiling of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the smallest integer value not greater than x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_ceil_inline(double x) {
  __SEGGER_RTL_U64 x0;
  __SEGGER_RTL_U64 mask;
  __SEGGER_RTL_U64 exact;
  int              exponent;
  //
  // Get exponent.
  //
  x0       = __SEGGER_RTL_BitcastToU64(x);
  exponent = FLOAT64_EXPONENT(x0);
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    if (exponent == 0) {
      x0 &= FLOAT64_SIGN_MASK;
    }
    return __SEGGER_RTL_BitcastToF64(x0);
  }
  //
  exponent -= 0x3FF;
  if (exponent < 0) {
    //
    // 0 < |x| <= 1.0
    //
    if (FLOAT64_SIGN(x0)) {
      return 0.0;     // x < 0, so round up to zero.
    } else {
      return 1.0;     // 0 < x, so round up to one.
    }
  } else if (exponent >= 52) {
    //
    // Too large to round up by one unit; addition of a
    // unit is insignificant.
    //
    return __SEGGER_RTL_BitcastToF64(x0);
  }
  //
  // Bit mask for fractional bits.
  //
  mask = FLOAT64_SIGNIFICAND_MASK >> exponent;
  //
  // Extract and remove fractional bits.
  //
  exact = x0 & mask;
  x0   &= ~mask;
  //
  if (exact && FLOAT64_SIGN(x0) == 0) {
    x0 += __SEGGER_RTL_U64_C(1) << 52 >> exponent;
  }
  return __SEGGER_RTL_BitcastToF64(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_floor_inline()
*
*  Function description
*    Compute largest integer not greater than, float.
*
*  Parameters
*    x - Value to floor.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the largest integer value not greater than x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_floor_inline(float x) {
  __SEGGER_RTL_U32 x0;
  __SEGGER_RTL_U32 mask;
  __SEGGER_RTL_U32 exact;
  int              exponent;
  //
  x0       = __SEGGER_RTL_BitcastToU32(x);
  exponent = FLOAT32_EXPONENT(x0);
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(x))) {
    if (exponent == 0) {
      x0 &= FLOAT32_SIGN_MASK;
    }
    return __SEGGER_RTL_BitcastToF32(x0);
  }
  //
  exponent -= 0x7F;
  if (exponent < 0) {
    //
    // 0 < |x| <= 1.0
    //
    if (FLOAT32_SIGN(x0)) {
      return -1.0f;     // x < 0, so round down to -1.
    } else {
      return 0.0f;      // 0 < x, so round down to zero.
    }
  } else if (exponent >= 23) {
    //
    // Too large to round down by one unit; subtraction of a
    // unit is insignificant. Or Inf or NaN.
    //
    return x;
  }
  //
  // Bit mask for fractional bits.
  //
  mask = FLOAT32_SIGNIFICAND_MASK >> exponent;
  //
  // Extract fractional bits.
  //
  exact = x0 & mask;
  //
  // Remove fractional bits.
  //
  x0 &= ~mask;
  //
  // Add 1.0 for negative non-integers, floor(-1.1) = -2.
  //
  if (exact && FLOAT32_SIGN(x0)) {
    x0 += __SEGGER_RTL_U32_C(1) << 23 >> exponent;
  }
  //
  return __SEGGER_RTL_BitcastToF32(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_floor_inline()
*
*  Function description
*    Compute largest integer not greater than, double.
*
*  Parameters
*    x - Value to floor.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the largest integer value not greater than x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_floor_inline(double x) {
  __SEGGER_RTL_U64 mask;
  __SEGGER_RTL_U64 exact;
  __SEGGER_RTL_U64 x0;
  int              exponent;
  //
  x0       = __SEGGER_RTL_BitcastToU64(x);
  exponent = FLOAT64_EXPONENT(x0);
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    if (exponent == 0) {
      x0 &= FLOAT64_SIGN_MASK;
    }
    return __SEGGER_RTL_BitcastToF64(x0);
  }
  //
  exponent -= 0x3FF;
  if (exponent < 0) {
    //
    // 0 < |x| <= 1.0
    //
    if (FLOAT64_SIGN(x0)) {
      return -1.0;     // x < 0, so round up to zero.
    } else {
      return 0.0;     // 0 < x, so round up to one.
    }
  } else if (exponent >= 52) {
    //
    // Too large to round up by one unit; addition of a
    // unit is insignificant. Or x if Inf or NaN.
    //
    return __SEGGER_RTL_BitcastToF64(x0);
  }
  //
  // Bit mask for fractional bits.
  //
  mask = FLOAT64_SIGNIFICAND_MASK >> exponent;
  //
  // Extract fractional bits.
  //
  exact = x0 & mask;
  //
  // Remove fractional bits.
  //
  x0  &= ~mask;
  //
  // Add 1.0 for negative non-integers, floor(-1.1) = -2.
  //
  if (exact && FLOAT64_SIGN(x0)) {
    x0 += __SEGGER_RTL_U64_C(1) << 52 >> exponent;
  }
  return __SEGGER_RTL_BitcastToF64(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_trunc_inline()
*
*  Function description
*    Truncate argument, float.
*
*  Parameters
*    x - Value to truncate.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, x with the fractional part removed.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_trunc_inline(float x) {
  __SEGGER_RTL_U32 x0;
  __SEGGER_RTL_U32 mask;
  int              exponent;
  //
  x0       = __SEGGER_RTL_BitcastToU32(x);
  exponent = FLOAT32_EXPONENT(x0);
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(x))) {
    if (exponent == 0) {
      x0 &= FLOAT32_SIGN_MASK;
    }
    return __SEGGER_RTL_BitcastToF32(x0);
  }
  //
  exponent -= 0x7F;
  if (exponent < 0) {
    //
    // 0 < |x| <= 1.0
    //
    return __SEGGER_RTL_BitcastToF32(FLOAT32_SIGN(x0));
    //
  } else if (exponent >= 23) {
    //
    // Too large to round down by one unit; subtraction of a
    // unit is insignificant. Or Inf or NaN.
    //
    return x;
  }
  //
  // Bit mask for fractional bits.
  //
  mask = FLOAT32_SIGNIFICAND_MASK >> exponent;
  //
  // Remove fractional bits.
  //
  x0 &= ~mask;
  //
  return __SEGGER_RTL_BitcastToF32(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_trunc_inline()
*
*  Function description
*    Truncate argument, double.
*
*  Parameters
*    x - Value to truncate.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, x with the fractional part removed.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_trunc_inline(double x) {
  __SEGGER_RTL_U64 mask;
  __SEGGER_RTL_U64 x0;
  int              exponent;
  //
  x0       = __SEGGER_RTL_BitcastToU64(x);
  exponent = FLOAT64_EXPONENT(x0);
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    if (exponent == 0) {
      x0 &= FLOAT64_SIGN_MASK;
    }
    return __SEGGER_RTL_BitcastToF64(x0);
  }
  //
  exponent -= 0x3FF;
  if (exponent < 0) {
    //
    // 0 < |x| <= 1.0
    //
    return __SEGGER_RTL_BitcastToF64(FLOAT64_SIGN(x0));
    //
  } else if (exponent >= 52) {
    //
    // Too large to round up by one unit; addition of a
    // unit is insignificant. Or x if Inf or NaN.
    //
    return __SEGGER_RTL_BitcastToF64(x0);
  }
  //
  // Bit mask for fractional bits.
  //
  mask = FLOAT64_SIGNIFICAND_MASK >> exponent;
  //
  // Remove fractional bits.
  //
  x0  &= ~mask;
  //
  return __SEGGER_RTL_BitcastToF64(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_rint_inline()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_rint_inline(float x) {
  __SEGGER_RTL_U32 x0;
  __SEGGER_RTL_U32 Mask;
  __SEGGER_RTL_U32 Fraction;
  unsigned         Exponent;
  unsigned         Shift;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  Exponent = FLOAT32_EXPONENT(x0) - FLOAT32_EXPONENT_BIAS;
  //
  // Fast classification...
  //
  if (Exponent < FLOAT32_SIGNIFICAND_BITS) {
    //
    // Finite numbers that require rounding.
    //
    Shift = FLOAT32_SIGNIFICAND_BITS - Exponent;
    Mask  = ~__SEGGER_RTL_U32_C(0) << Shift;
    //
    // Extract fractional part.
    //
    Fraction  = (x0 | FLOAT32_HIDDEN_MASK) & ~(Mask << 1);
    Fraction  = ((Fraction >> Shift) | (Fraction << Exponent << 1));
    Fraction &= FLOAT32_SIGNIFICAND_X_MASK;
    //
    // Clear out lower-order bits and increase if fraction > 0.5.
    //
    x0 &= Mask;
    if (Fraction > FLOAT32_HIDDEN_MASK) {
      x0 += __SEGGER_RTL_U32_C(1) << Shift;
    }
    //
  } else if (Exponent > FLOAT32_EXPONENT_INF - FLOAT32_EXPONENT_BIAS) {
    //
    // |x| < 1, 0 rounds to 0, and others round to +1.0 or -1.0 based on sign.
    //
    x0 = (((__SEGGER_RTL_I32)(__SEGGER_RTL_U32_C(0x7E000000) - (x0 << 1)) >> 31) & __SEGGER_RTL_U32_C(0x3F800000)) | (x0 & FLOAT32_SIGN_MASK);
  }
  //
  return __SEGGER_RTL_BitcastToF32(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_rint_inline()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_rint_inline(double x) {
  __SEGGER_RTL_U64 x0;
  __SEGGER_RTL_U64 Mask;
  __SEGGER_RTL_U64 Fraction;
  unsigned         Exponent;
  unsigned         Shift;
  //
  x0 = __SEGGER_RTL_BitcastToU64(x);
  //
  Exponent = FLOAT64_EXPONENT(x0) - FLOAT64_EXPONENT_BIAS;
  //
  // Fast classification...
  //
  if (Exponent < FLOAT64_SIGNIFICAND_BITS) {
    //
    // Finite numbers that require rounding.
    //
    Shift = FLOAT64_SIGNIFICAND_BITS - Exponent;
    Mask  = ~__SEGGER_RTL_U64_C(0) << Shift;
    //
    // Extract fractional part.
    //
    Fraction  = (x0 | FLOAT64_HIDDEN_MASK) & ~(Mask << 1);
    Fraction  = ((Fraction >> Shift) | (Fraction << Exponent << 1));
    Fraction &= FLOAT64_SIGNIFICAND_X_MASK;
    //
    // Clear out lower-order bits and increase if fraction > 0.5.
    //
    x0 &= Mask;
    if (Fraction > FLOAT64_HIDDEN_MASK) {
      x0 += __SEGGER_RTL_U64_C(1) << Shift;
    }
    //
  } else if (Exponent > FLOAT64_EXPONENT_INF - FLOAT64_EXPONENT_BIAS) {
    //
    // Small, 0 rounds to 0, and others round to 1.0 or 1.0 based on sign.
    //
    x0 = (((__SEGGER_RTL_I64)(__SEGGER_RTL_U64_C(0x7FC0000000000000) - (x0 << 1)) >> 63) & __SEGGER_RTL_U64_C(0x3FF0000000000000)) | (x0 & FLOAT64_SIGN_MASK);
  }
  //
  return __SEGGER_RTL_BitcastToF64(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lrint_inline()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * Integer value nearest x.
*    * If the rounded value is outside the range of the return type,
*      the numeric result is unspecified.
*/
static __SEGGER_RTL_INLINE long __SEGGER_RTL_float32_lrint_inline(float x) {
  return SEGGER_F2L(__SEGGER_RTL_float32_rint_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lrint_inline()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * Integer value nearest x.
*    * If the rounded value is outside the range of the return type,
*      the numeric result is unspecified.
*/
static __SEGGER_RTL_INLINE long __SEGGER_RTL_float64_lrint_inline(double x) {
  return SEGGER_D2L(__SEGGER_RTL_float64_rint_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_llrint_inline()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * Integer value nearest x.
*    * If the rounded value is outside the range of the return type,
*      the numeric result is unspecified.
*/
static __SEGGER_RTL_INLINE long long __SEGGER_RTL_float32_llrint_inline(float x) {
  return SEGGER_F2LL(__SEGGER_RTL_float32_rint_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_llrint_inline()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * Integer value nearest x.
*    * If the rounded value is outside the range of the return type,
*      the numeric result is unspecified.
*/
static __SEGGER_RTL_INLINE long __SEGGER_RTL_float64_llrint_inline(double x) {
  return SEGGER_D2L(__SEGGER_RTL_float64_rint_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_round_inline()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x, rounding away from zero.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_round_inline(float x) {
  __SEGGER_RTL_U32 x0;
  __SEGGER_RTL_U32 Mask;
  int              Exponent;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  Exponent = FLOAT32_EXPONENT(x0) - FLOAT32_EXPONENT_BIAS + 1;
  //
  // Fast classification...
  //
  if (Exponent > 0) {
    //
    // Finite numbers that require rounding.
    //
    if (Exponent <= FLOAT32_SIGNIFICAND_BITS) {
      Mask = FLOAT32_SIGNIFICAND_X_MASK >> Exponent;
      x0 &= ~(Mask >> 1);
      x0 += Mask;
      x0 &= ~Mask;
    }
  } else {
    //
    // |x| < 1, 0 rounds to 0, and others round to +1.0 or -1.0 based on sign.
    //
    x0 &= FLOAT32_SIGN_MASK;
    if (Exponent == 0) {
      x0 |= K_one_U32;
    }
  }
  //
  return __SEGGER_RTL_BitcastToF32(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_round_inline()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x, rounding away from zero.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_round_inline(double x) {
  __SEGGER_RTL_U64 x0;
  __SEGGER_RTL_U64 Mask;
  int              Exponent;
  //
  x0 = __SEGGER_RTL_BitcastToU64(x);
  //
  Exponent = FLOAT64_EXPONENT(x0) - FLOAT64_EXPONENT_BIAS + 1;
  //
  // Fast classification...
  //
  if (Exponent > 0) {
    //
    // Finite numbers that require rounding.
    //
    if (Exponent <= FLOAT64_SIGNIFICAND_BITS) {
      Mask = FLOAT64_SIGNIFICAND_X_MASK >> Exponent;
      x0 &= ~(Mask >> 1);
      x0 += Mask;
      x0 &= ~Mask;
    }
  } else {
    //
    // |x| < 1, 0 rounds to 0, and others round to +1.0 or -1.0 based on sign.
    //
    x0 &= FLOAT64_SIGN_MASK;
    if (Exponent == 0) {
      x0 |= K_one_U64;
    }
  }
  //
  return __SEGGER_RTL_BitcastToF64(x0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lround_inline()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * Integer value nearest x, with ties rounded away from zero.
*    * If the rounded value is outside the range of the return type,
*      the numeric result is unspecified.
*/
static __SEGGER_RTL_INLINE long __SEGGER_RTL_float32_lround_inline(float x) {
  return SEGGER_F2L(__SEGGER_RTL_float32_round_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lround_inline()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * Integer value nearest x, with ties rounded away from zero.
*    * If the rounded value is outside the range of the return type,
*      the numeric result is unspecified.
*/
static __SEGGER_RTL_INLINE long __SEGGER_RTL_float64_lround_inline(double x) {
  return SEGGER_D2L(__SEGGER_RTL_float64_round_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_llround_inline()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * Integer value nearest x, with ties rounded away from zero.
*    * If the rounded value is outside the range of the return type,
*      the numeric result is unspecified.
*/
static __SEGGER_RTL_INLINE long long __SEGGER_RTL_float32_llround_inline(float x) {
  return SEGGER_F2LL(__SEGGER_RTL_float32_round_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_llround_inline()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * Integer value nearest x, with ties rounded away from zero.
*    * If the rounded value is outside the range of the return type,
*      the numeric result is unspecified.
*/
static __SEGGER_RTL_INLINE long __SEGGER_RTL_float64_llround_inline(double x) {
  return SEGGER_D2L(__SEGGER_RTL_float64_round_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_nextafter_inline()
*
*  Function description
*    Next machine-floating value, double.
*
*  Parameters
*    x - Value to step from.
*    y - Director to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_nextafter_inline(float x, float y) {
  __SEGGER_RTL_U32 ix;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(x))) {
    return x;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(y))) {
    return y;
  }
  //
  ix = __SEGGER_RTL_BitcastToU32(x);
  //
  // This is an exactly-zero test rather than a putative test.
  // Generate and accept subnormals.
  //
  if (__SEGGER_RTL_float32_exact_iszero(x)) {
    ix = __SEGGER_RTL_BitcastToU32(y) & FLOAT32_SIGN_MASK;
    if (!__SEGGER_RTL_float32_exact_iszero(y)) {
      ++ix;
    }
  } else if (__SEGGER_RTL_float32_opposite_signs(x, y)) {
    --ix;
  } else if (__SEGGER_RTL_float32_eq_bitwise(x, y)) {
    /* Return x */
  } else if (__SEGGER_RTL_float32_lt_bitwise_unsigned(x, y)) {
    ++ix;
  } else {
    --ix;
  }
  //
  return __SEGGER_RTL_BitcastToF32(ix);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_nextafter_inline()
*
*  Function description
*    Next machine-floating value, double.
*
*  Parameters
*    x - Value to step from.
*    y - Director to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_nextafter_inline(double x, double y) {
  __SEGGER_RTL_U64 ix;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(x))) {
    return x;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(y))) {
    return y;
  }
  //
  ix = __SEGGER_RTL_BitcastToU64(x);
  //
  // This is an exactly-zero test rather than a putative test.
  // Generate and accept subnormals.
  //
  if (__SEGGER_RTL_float64_exact_iszero(x)) {
    ix = __SEGGER_RTL_BitcastToU64(y) & FLOAT64_SIGN_MASK;
    if (!__SEGGER_RTL_float64_exact_iszero(y)) {
      ++ix;
    }
  } else if (__SEGGER_RTL_float64_opposite_signs(x, y)) {
    --ix;
  } else if (__SEGGER_RTL_float64_eq_bitwise(x, y)) {
    /* Return x */
  } else if (__SEGGER_RTL_float64_lt_bitwise_unsigned(x, y)) {
    ++ix;
  } else {
    --ix;
  }
  //
  return __SEGGER_RTL_BitcastToF64(ix);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_nan_inline()
*
*  Function description
*    Parse NaN, float.
*
*  Parameters
*    sText - NaN tag.
*
*  Return value
*    Quiet NaN formed from tag.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_nan_inline(const char *sText) {
  __SEGGER_RTL_U32 x;
  //
  if (sText[0] == '0' && (sText[1] == 'x' || sText[1] == 'X')) {
    sText += 2;
  }
  //
  x = 0;
  for (;;) {
    if ('0' <= *sText && *sText <= '9') {
      x = x * 0x10u + (*sText - '0');
    } else if ('a' <= *sText && *sText <= 'f') {
      x = x * 0x10u + (*sText - 'a') + 10u;
    } else if ('A' <= *sText && *sText <= 'F') {
      x = x * 0x10u + (*sText - 'A') + 10u;
    } else {
      break;
    }
  }
  //
  x &= FLOAT32_SIGNIFICAND_MASK;
  x |= K_NAN_U32;
  //
  return __SEGGER_RTL_BitcastToF32(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_nan_inline()
*
*  Function description
*    Parse NaN, double.
*
*  Parameters
*    sText - NaN tag.
*
*  Return value
*    Quiet NaN formed from tag.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_nan_inline(const char *sText) {
  __SEGGER_RTL_U64 x;
  //
  if (sText[0] == '0' && (sText[1] == 'x' || sText[1] == 'X')) {
    sText += 2;
  }
  //
  x = 0;
  for (;;) {
    if ('0' <= *sText && *sText <= '9') {
      x = x * 0x10u + (*sText - '0');
    } else if ('a' <= *sText && *sText <= 'f') {
      x = x * 0x10u + (*sText - 'a') + 10u;
    } else if ('A' <= *sText && *sText <= 'F') {
      x = x * 0x10u + (*sText - 'A') + 10u;
    } else {
      break;
    }
    ++sText;
  }
  //
  x &= FLOAT64_SIGNIFICAND_MASK;
  x |= K_NAN_U64;
  //
  return __SEGGER_RTL_BitcastToF64(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sqrt_scaled_integer()
*
*  Function description
*    Compute square root, float.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is positive infinity, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    Computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrtf() deviates and always uses IEC 60559 semantics.
*
*  Notes
*    Based on Peter W. Markstein's "Computation of elementary functions
*    on the IBM RISC System/6000 Processor", IBM Journal of Research
*    and Development.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_sqrt_scaled_integer(float x) {
  __SEGGER_RTL_U32 i0, i1, i2;
  __SEGGER_RTL_U64 d0;
  unsigned         q0;
#if !__SEGGER_RTL_SUBNORMALS_READ_AS_ZERO
  unsigned         q1;
#endif
  //
  // Extract combined exponent and sign.
  //
  i0 = __SEGGER_RTL_BitcastToU32(x);
  q0 = (unsigned)(i0 >> 23) - 1;
  //
  if (__SEGGER_RTL_UNLIKELY(q0 >= 0xFE)) {
    //
    // Subnormal, +/-0, Inf, or NaN.
    //
    q0 += 1;
    if (q0 >= 0xFF) {
      //
      // -0, Inf, or NaN.
      //
      i1 = i0 << 1;
      if (i1 == 0x00000000) {                             // Sqrt[-0] = -0
        return x;
      } else if (i1 > __SEGGER_RTL_U32_C(0xFF000000)) {   // Sqrt[NaN] = NaN
        return __SEGGER_RTL_BitcastToF32(i0 | __SEGGER_RTL_U32_C(0x400000));
      } else if (i0 & __SEGGER_RTL_U32_C(0x80000000)) {   // Sqrt[-nonzero] = NaN
        return K_NAN_F32;
      } else {                                            // Sqrt[+Inf] = +Inf
        return x;
      }
    } else {
      //
      // Subnormal or +0.
      //
#if __SEGGER_RTL_SUBNORMALS_READ_AS_ZERO
      return 0;
#else
      if (i0 == 0) {                  // Sqrt[+0] = 0
        return x;
      }
      //
      // Subnormal, so normalize.
      //
      q1   = __SEGGER_RTL_CLZ_U32(i0) - 8;
      q0  -= q1;  // q0 can go negative here...
      i0 <<= q1;
      if (q0 & 1) {
        i0 &= ~__SEGGER_RTL_U32_C(0x800000);
      }
#endif
    }
  }
  //
  // Approximation from first eight bits of significand.
  //
  i0 <<= 8;
  i1 = __SEGGER_RTL_aSqrtData[i0 >> 24];
  i2 = __SEGGER_RTL_aSqrtData[0x100 + (i0 >> 25)]; 
  //
  // Set hidden bit.
  //
  i0 |= __SEGGER_RTL_U32_C(0x80000000);
  //
  // Take care of exponent adjustment.
  //
  i0 >>= (~q0 & 1);
  //
  // Initial approximation and two iterations improves to ~28 bits.  Use a double-wide
  // register to perform the subtraction with carry across the 64-bit product.
  //
  i1 = (i1 << (32-12)) + (__SEGGER_RTL_U32)(__SEGGER_RTL_UMULL_X(i0 - ((i1 * i1) << 16), i2) >> 12);
  d0 = ((__SEGGER_RTL_U64)i0 << 24) - __SEGGER_RTL_UMULL_X(i1, i1);
  i1 += (__SEGGER_RTL_UMULL_HI(__SEGGER_RTL_U64_L(d0), i2) + __SEGGER_RTL_U64_H(d0) * i2) >> 4;
  d0 = ((__SEGGER_RTL_U64)i0 << 24) - __SEGGER_RTL_UMULL_X(i1, i1);
  i1 += (__SEGGER_RTL_UMULL_HI(__SEGGER_RTL_U64_L(d0), i2) + __SEGGER_RTL_U64_H(d0) * i2) >> 4;
  //
  // Truncate root to 24 significand bits (per storage format) plus one bit for rounding.
  //
  i1 = (i1 & ~0x07) | 0x08;
  //
  // Round, pack, and return.
  //
  return __SEGGER_RTL_BitcastToF32(  ((__SEGGER_RTL_U32)((q0 & ~1u) + 0x7E) << 22)
                                   + (i1 >> 4)
                                   + (((__SEGGER_RTL_U64)i0 << 24) >= __SEGGER_RTL_UMULL_X(i1, i1)));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sqrt_newton_fpu()
*
*  Function description
*    Compute square root, float.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is positive infinity, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    Computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrtf() deviates and always uses IEC 60559 semantics. 
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_sqrt_newton_fpu(float x) {
  float y0;
  int   n;
  //
  if (__SEGGER_RTL_OPTIMIZE <= -2 || __SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_float32_putative_iszero(x)) {
      return __SEGGER_RTL_float32_subnormal_flush(x);
    } else if (__SEGGER_RTL_float32_lt_rhs_positive(x, 0)) {
      return K_NAN_F32;
    } else if (__SEGGER_RTL_OPTIMIZE > -2 || !__SEGGER_RTL_float32_isfinite_inline(x)) {
      return x;
    }
  }
  //
  // Get exponent.
  //
  SEGGER_FREXPF(x, &n);
  //
  // Reduce to range [0.5, 1].
  //
  x = SEGGER_LDEXPF(x, -n);
  //
  // Form an initial guess at the root using a linear approximation.
  //
  // Sollya: > p = fpminimax(sqrt(x), 1, [|single...|], [0.5,1], floating, relative); p;
  //         = 0x1.ab52bp-2 + x * 0x1.2e29b8p-1
  //         = 0.4173076152801513671875 + x * 0.5901620388031005859375
  //         > -log2(inf(supnorm(p, sqrt(x), [0.5,1], relative, 2^-20)));
  //         = 7.0647394065776...
  //
  y0 = SEGGER_ADDF(__SEGGER_RTL_FLT_SELECT(0x1.ab52bp-2f,  0.4173076152801513671875f),
                   SEGGER_MULF(__SEGGER_RTL_FLT_SELECT(0x1.2e29b8p-1f, 0.5901620388031005859375f), x));  // 7 bits of precision
  //
  // Newton iterations double the precision at each stage.
  //
  y0 = SEGGER_DIV2F(SEGGER_ADDF(y0, SEGGER_DIVF(x, y0)));  // 14 bits
  y0 = SEGGER_DIV2F(SEGGER_ADDF(y0, SEGGER_DIVF(x, y0)));  // 28 bits
  if (n & 1) {
    y0 = SEGGER_MULF(y0, (float)M_SQRT_1_2);
    ++n;
  }
  //
  // Here we know n > 2, so use an arithmetic shift.
  //
  return SEGGER_LDEXPF(y0, n >> 1);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sqrt_markstein_fpu()
*
*  Function description
*    Compute square root, float.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is positive infinity, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    Computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrtf() deviates and always uses IEC 60559 semantics.
*
*  Notes
*    Based on Peter W. Markstein's "Computation of elementary functions
*    on the IBM RISC System/6000 Processor", IBM Journal of Research
*    and Development.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_sqrt_markstein_fpu(float x) {
  float    g;
  float    y;
  int      n;
  unsigned x0;
  //
  if (__SEGGER_RTL_OPTIMIZE <= -2 || __SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_float32_putative_iszero(x)) {
      return __SEGGER_RTL_float32_subnormal_flush(x);
    } else if (__SEGGER_RTL_float32_lt_rhs_positive(x, 0)) {
      return K_NAN_F32;
    } else if (__SEGGER_RTL_OPTIMIZE > -2 || !__SEGGER_RTL_float32_isfinite_inline(x)) {
      return x;
    }
  }
  //
  // Look up initial guess, g, for the root along with the approximation
  // to the reciprocal 1/2g.
  //
  x0 = (__SEGGER_RTL_BitcastToU32(x) << 8 >> 24);
  g = __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_U32_C(0x3E800000) + ((__SEGGER_RTL_U32)__SEGGER_RTL_aSqrtData[x0] << 16));
  y = __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_U32_C(0x3E800000) + ((__SEGGER_RTL_U32)__SEGGER_RTL_aSqrtData[(x0>>1) + 0x100] << 16));
  //
  // Reduce x to primary range, 0.25 <= x < 1.
  //
  x = SEGGER_FREXPF(x, &n);    // 0.5 <= x < 1
  if (n & 1) {
    x = SEGGER_DIV2F(x);
    ++n;
  }
  //
  // Accuracy from 8 to ~32 bits.
  //
  g = SEGGER_ADDF(g, SEGGER_MULF(SEGGER_SUBF(x, SEGGER_MULF(g, g)), y));
  g = SEGGER_ADDF(g, SEGGER_MULF(SEGGER_SUBF(x, SEGGER_MULF(g, g)), y));
  g = SEGGER_ADDF(g, SEGGER_MULF(SEGGER_SUBF(x, SEGGER_MULF(g, g)), y));
  //
  return SEGGER_LDEXPF(g, n / 2);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sqrt_inline()
*
*  Function description
*    Compute square root, float.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is positive infinity, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    Computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrtf() deviates and always uses IEC 60559 semantics. 
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_sqrt_inline(float x) {
#if defined(__SEGGER_RTL_FLOAT32_SQRT)
  return __SEGGER_RTL_FLOAT32_SQRT(x);
#elif __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_sqrt_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_sqrt_markstein_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sqrt_outline()
*
*  Function description
*    Compute square root, float.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is positive infinity, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    Computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrtf() deviates and always uses IEC 60559 semantics. 
*/
static float __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float32_sqrt_outline(float x) {
  return __SEGGER_RTL_float32_sqrt_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sqrt_newton_fpu()
*
*  Function description
*    Compute square root, double, Newton method.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    Square root of x.
*
*  Additional information
*    This implementation uses Newton iterations and should be fast
*    on cores that have full double floating-point instructions.
*    It is also small on cores that must use floating-point emulation.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sqrt_newton_fpu(double x) {
  double y0;
  int    n;
  int    memn;
  //
  if (__SEGGER_RTL_OPTIMIZE <= -2 || __SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_putative_iszero(x))) {
      return __SEGGER_RTL_float64_subnormal_flush(x);
    } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_lt_rhs_positive(x, 0))) {
      return K_NAN_F64;
    } else if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(x))) {
      return x;
    }
  }
  //
  // Get exponent.
  //
  x = SEGGER_FREXP(x, &memn);
  n = memn;
  //
  // Form an initial guess at the root using a linear approximation.
  //
  y0 = SEGGER_ADD(0.41731, SEGGER_MUL(0.59016, x));  // 7 bits of precision
  //
  // Newton iterations double the precision at each stage.
  //
  y0 = SEGGER_DIV2(SEGGER_ADD(y0, SEGGER_DIV(x, y0))); // 14 bits
  y0 = SEGGER_DIV2(SEGGER_ADD(y0, SEGGER_DIV(x, y0))); // 28 bits
  y0 = SEGGER_SUB(y0, SEGGER_DIV2(SEGGER_SUB(y0, SEGGER_DIV(x, y0)))); // 56 bits
  if (n & 1) {
    y0 = SEGGER_MUL(y0, M_SQRT_1_2);
    ++n;
  }
  //
  return SEGGER_LDEXP(y0, n / 2);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sqrt_kahan_fpu()
*
*  Function description
*    Compute square root, double, Kahan's reciprocal square root method.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    Square root of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sqrt_kahan_fpu(double x) {
  SEGGER_RTL_float64_t xx;
  SEGGER_RTL_float64_t yy;
  __SEGGER_RTL_U32     k;
  double               z;
  //
  if (__SEGGER_RTL_OPTIMIZE <= -2 || __SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_putative_iszero(x))) {
      return __SEGGER_RTL_float64_subnormal_flush(x);
    } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_lt_rhs_positive(x, 0))) {
      return K_NAN_F64;
    } else if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(x))) {
      return x;
    }
  }
  //
  xx.l = __SEGGER_RTL_BitcastToU64(x);
  k    = __SEGGER_RTL_U32_C(0x5FE80000) - (xx.part.hi >> 1);
  yy.part.hi = k - __SEGGER_RTL_kahan_aT2[(k>>14) & 0x3F];    // yy ~ 1/sqrt(x) to 7.8 bits
  yy.part.lo = 0;
  //
  // Apply Reciproot iteration three times to y and multiply the
  // result by x to get an approximation z that matches sqrt(x)
  // to about 1 ulp. To be exact, we will have -1ulp < sqrt(x)-z<1.0625ulp.
  //
  // Rounding mode must be round-to-nearest here.
  //
  z    = SEGGER_MUL(x, SEGGER_MUL(yy.f, yy.f));
  yy.f = SEGGER_MUL(yy.f, SEGGER_SUB(1.5, SEGGER_DIV2(z)));   // almost 15 sig. bits to 1/sqrt(x)
  z    = SEGGER_MUL(x, SEGGER_MUL(yy.f, yy.f));
  yy.f = SEGGER_MUL(yy.f, SEGGER_SUB(__SEGGER_RTL_FLT_SELECT(0x1.7ffffffcp0, 1.499999999068677425384521484375), SEGGER_DIV2(z)));   // about 29 sig. bits to 1/sqrt(x)
  //
  // Special arrangement for better accuracy.
  //
  z = SEGGER_MUL(x, yy.f);                                                              // 29 bits to sqrt(x), with z*y<1
  z = SEGGER_ADD(z, SEGGER_DIV2(SEGGER_MUL(z, (SEGGER_SUB(1, SEGGER_MUL(z, yy.f))))));  // about 1 ulp to sqrt(x)
  //
  return z;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sqrt_markstein_standard()
*
*  Function description
*    Compute square root, double, Markstein method, FPU or FPE.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    Square root of x.
*
*  Additional information
*    This implementation uses Newton iterations and should be fast
*    on cores that have full double floating-point instructions.
*    It is also small on cores that must use floating-point emulation.
*
*  Notes
*    Based on Peter W. Markstein's "Computation of elementary functions
*    on the IBM RISC System/6000 Processor", IBM Journal of Research
*    and Development, with an option to to eliminate refinment of y=1/2g.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sqrt_markstein_standard(double x) {
  double   g;
  double   y;
  int      n;
  unsigned x0;
  //
  if (__SEGGER_RTL_OPTIMIZE <= -2 || __SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_putative_iszero(x))) {
      return __SEGGER_RTL_float64_subnormal_flush(x);
    } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_lt_rhs_positive(x, 0))) {
      return K_NAN_F64;
    } else if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(x))) {
      return x;
    }
  }
  //
  // Look up initial guess, g, for the root along with the approximation
  // to the reciprocal 1/2g.
  //
  x0 = (__SEGGER_RTL_BitcastToU64(x) << 11 >> (24 + 32));
  g = __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_U64_C(0x3FD0000000000000) + ((__SEGGER_RTL_U64)__SEGGER_RTL_aSqrtData[x0] << 45));
  y = __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_U32_C(0x3FD0000000000000) + ((__SEGGER_RTL_U64)__SEGGER_RTL_aSqrtData[(x0>>1) + 0x100] << 45));
  //
  // Reduce x to primary range, 0.25 <= x < 1.
  //
  x = SEGGER_FREXP(x, &n);    // 0.5 <= x < 1
  if (n & 1) {
    x = SEGGER_DIV2(x);
    ++n;
  }
  //
  // Version with refinement of y=1/2g, fewer steps but also more complex.
  //
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);                  // g += (x - g*g) * y;
  y = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(y), g, 0.5), SEGGER_MUL2(y), y);   // y += (0.5 - y*g) * 2*y;
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);                  // g += (x - g*g) * y;
  y = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(y), g, 0.5), SEGGER_MUL2(y), y);   // y += (0.5 - y*g) * 2*y;
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);                  // g += (x - g*g) * y;
  //
  return SEGGER_LDEXP(g, n / 2);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sqrt_markstein_modified()
*
*  Function description
*    Compute square root, double, modified Markstein method.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    Square root of x.
*
*  Additional information
*    This implementation is based on Peter W. Markstein's "Computation of
*    elementary functions on the IBM RISC System/6000 Processor", IBM Journal
*    of Research and Development.
*
*    It eliminates the refinment of y=1/2g, which requires more convergence
*    steps but may reduce the computational load compared with the standard
*    method.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sqrt_markstein_modified(double x) {
  double   g;
  double   y;
  int      n;
  unsigned x0;
  //
  if (__SEGGER_RTL_OPTIMIZE <= -2 || __SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_putative_iszero(x))) {
      return __SEGGER_RTL_float64_subnormal_flush(x);
    } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_lt_rhs_positive(x, 0))) {
      return K_NAN_F64;
    } else if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(x))) {
      return x;
    }
  }
  //
  // Look up initial guess, g, for the root along with the approximation
  // to the reciprocal 1/2g.
  //
  x0 = (__SEGGER_RTL_BitcastToU64(x) << 11 >> (24 + 32));
  g = __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_U64_C(0x3FD0000000000000) + ((__SEGGER_RTL_U64)__SEGGER_RTL_aSqrtData[x0] << 45));
  y = __SEGGER_RTL_BitcastToF64(__SEGGER_RTL_U64_C(0x3FD0000000000000) + ((__SEGGER_RTL_U64)__SEGGER_RTL_aSqrtData[(x0>>1) + 0x100] << 45));
  //
  // Reduce x to primary range, 0.25 <= x < 1.
  //
  x = SEGGER_FREXP(x, &n);    // 0.5 <= x < 1
  if (n & 1) {
    x = SEGGER_DIV2(x);
    ++n;
  }
  //
  // Improve accuracy from 8 to 64 bits, but because we have eliminated
  // the refinement of y=1/2g we rely on the self-correction of Newton's method.
  //
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);  // g += (x - g*g) * y;
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);
  g = SEGGER_FMA(SEGGER_FMA(SEGGER_NEG(g), g, x), y, g);
  //
  return SEGGER_LDEXP(g, n / 2);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sqrt_scaled_integer()
*
*  Function description
*    Compute square root, double, scaled integer.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    Square root of x.
*
*  Additional information
*    This implementation uses Tang-style scaled integers, along with
*    a modified Markstein algorithm, that is fast on cores that lack
*    floating-point instructions but do have support for fast 32x32->64
*    multiplication capability (e.g. Arm and RISC-V).
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sqrt_scaled_integer(double x) {
  __SEGGER_RTL_U64 g;
  __SEGGER_RTL_U64 i0;
  __SEGGER_RTL_U32 y;
  __SEGGER_RTL_U32 g0;
  unsigned         q0;
  unsigned         q1;
  //
  i0 = __SEGGER_RTL_BitcastToU64(x);
  q0 = (unsigned)(i0 >> 52) - 1;
  //
  if (__SEGGER_RTL_UNLIKELY(q0 >= 0x7FE)) {
    //
    // Subnormal, +/-0, Inf, or NaN.
    //
    q0 += 1;
    if (q0 >= 0x7FF) {
      //
      // -0, Inf, or NaN.
      //
      if ((i0 << 1) == 0) {                   // Sqrt[-0] = -0
        return x;
      } else if (__SEGGER_RTL_float64_isnan_soft(i0)) {   // Sqrt[NaN] = NaN
        return __SEGGER_RTL_BitcastToF64(i0);
      } else if (i0 & 0x8000000000000000) {   // Sqrt[-nonzero] = NaN
        return K_NAN_F64;
      } else {                                // Sqrt[+Inf] = +Inf
        return x;
      }
#if 0
    // For the case we do not support subnormals
    } else if (__SEGGER_RTL_float64_putative_iszero(x)) {
      return __SEGGER_RTL_float64_subnormal_flush(x);
#else
    // For the case we do support subnormals
    } else {
      //
      // Subnormal or +0.
      //
      if (i0 == 0) {                          // Sqrt[+0] = +0
        return x;
      }
      //
      // Subnormal, so normalize.
      //
      q0 = __SEGGER_RTL_float64_normalize(&i0, q0);
      if (q0 & 1) {
        i0 &= ~__SEGGER_RTL_U64_C(0x0010000000000000);
      }
#endif
    }
  }
  //
  // Look up initial approximation to the root (g0) along with
  // the approximation to the reciprocal 1/2g (y).
  //
  i0 <<= 11;
  q1   = (unsigned)(i0 >> 56);
  g0   = __SEGGER_RTL_aSqrtData[q1];
  y    = __SEGGER_RTL_aSqrtData[(q1>>1) + 0x100];
  //
  i0  |= __SEGGER_RTL_U64_C(0x8000000000000000);
  if ((q0 & 1) == 0) {
    i0 >>= 1;
  }
  //
  // First step has known bounds on g, i.e. 0x80 <= g <= 0xFF, so use
  // a specialized rather than generalized multiply.
  //
  g  = ((__SEGGER_RTL_U64)g0 << (64-8)) + ((i0 - ((__SEGGER_RTL_U64)(g0 * g0) << (64-16))) >> 8) * y;
  g += ((i0 - __SEGGER_RTL_SquareHi_U64(g)) >> 8) * y;
  g += ((i0 - __SEGGER_RTL_SquareHi_U64(g)) >> 8) * y;
  g += ((i0 - __SEGGER_RTL_SquareHi_U64(g)) >> 8) * y;
  g += ((i0 - __SEGGER_RTL_SquareHi_U64(g)) >> 8) * y;
  g += ((i0 - __SEGGER_RTL_SquareHi_U64(g)) >> 8) * y;
  g += ((i0 - __SEGGER_RTL_SquareHi_U64(g)) >> 8) * y;
  g += ((i0 - __SEGGER_RTL_SquareHi_U64(g)) >> 8) * y;
  //
  g &= ~__SEGGER_RTL_U64_C(0x3FF);
  g |= 0x400u;
  //
  return __SEGGER_RTL_BitcastToF64(((__SEGGER_RTL_U64)((q0 & ~1) + 0x3FE) << 51) +
                                   (g >> 11) +
                                   (i0 > __SEGGER_RTL_SquareHi_U64(g)));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sqrt_inline()
*
*  Function description
*    Compute square root, double.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    sqrt() computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrt() deviates and always uses IEC 60559 semantics. 
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sqrt_inline(double x) {
#if defined(__SEGGER_RTL_FLOAT64_SQRT)
  return __SEGGER_RTL_FLOAT64_SQRT(x);
#elif __SEGGER_RTL_FP_HW < 2 || __SEGGER_RTL_SCALED_INTEGER >= 2
  return __SEGGER_RTL_float64_sqrt_scaled_integer(x);
#else
  return __SEGGER_RTL_float64_sqrt_kahan_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sqrt_outline()
*
*  Function description
*    Compute square root, double.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    sqrt() computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrt() deviates and always uses IEC 60559 semantics. 
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_sqrt_outline(double x) {
  return __SEGGER_RTL_float64_sqrt_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cbrt_scaled_integer()
*
*  Function description
*    Compute cube root, float.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_cbrt_scaled_integer(float x) {
  __SEGGER_RTL_U32 i0, x0, sign, xx;
  __SEGGER_RTL_I32 i1;
  unsigned         q0, q1, q2, LeadingBits;
  //
  // Convert incoming.
  //
  xx = __SEGGER_RTL_BitcastToU32(x);
  //
  // Extract sign.
  //
  sign = xx & FLOAT32_SIGN_MASK;
  //
  // Extract biased exponent and significand.
  //
  x0 = xx << 1;
  q0 = x0 >> 24;
  i0 = xx & FLOAT32_SIGNIFICAND_MASK;
  //
  // Check for subnormal, NaN, or infinite.
  //
  if (__SEGGER_RTL_UNLIKELY(x0 - __SEGGER_RTL_X2(0x00800000) >= __SEGGER_RTL_X2(__SEGGER_RTL_U32_C(0x7F800000) - __SEGGER_RTL_U32_C(0x00800000)))) {
    //
    // cbrt(+-Inf) = +-Inf
    // cbrt(NaN)   = NaN
    //
    if (x0 >= __SEGGER_RTL_U32_C(0xFF000000)) {
      return x;
    }
    if (x0 == 0) {
      return x;
    }
    //
    // Normalize subnormal and recover exponent.
    //
    q0   = __SEGGER_RTL_CLZ_U32(i0) - 8;
    i0 <<= q0;
    q0   = 0x95 + q0;
  } else {
    //
    // Not subnormal, materialize hidden bit and recover exponent.
    //
    i0 = i0 | FLOAT32_HIDDEN_MASK;
    q0 = 0x95 - q0 + 1;
  }
  //
  // Range reduction.  cbrt(x * n^j) = cbrt(x) * cbrt(n^j), j=0..2.
  // Compute exponent/3 to q1, remainder to q2.
  //
  q2  = 23 - q0;
  q1  = (unsigned)(((__SEGGER_RTL_I32)(int)q2 * 0x556) >> 12);  // approximately 1/3, we only need a few bits as q2 is small.
  q2 -= 3*q1;
  if (q2 == 3) {
    ++q1;
    q2 = 0;
  }
  //
  // Get initial approximation from leading six bits of significand where the
  // leading bit is known to be 1, i.e. 1.xxxxx.
  //
  LeadingBits = (i0 >> 18) - 32;
  i0 *= __SEGGER_RTL_cbrt_paras.aInitialApprox[LeadingBits] << 4;
  //
  // NOTE: All inputs are 32-bit signed quantities, esp. i1 is signed.
  //
  i1 = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_I32_C(0xF5770B97));
  i1 = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_I32_C(0x0FCD6E9E) + (i1 >> 3));
  i1 = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_I32_C(0xE38E38E4) + (i1 >> 3));
  i1 = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_I32_C(0x55555555) + (i1 >> 3));
  //
  // Reconstruction.  cbrt(x * n^j) = cbrt(x) * cbrt(n^j), j=0..2.
  //
  i0  = __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_cbrt_paras.aMidRoot[LeadingBits],
                              __SEGGER_RTL_cbrt_paras.aMult[q2]) << 2;
  i0 += __SEGGER_RTL_SMULL_HI(i1, i0) >> 3;
  //
  // Over all values, normalization is either 1 or 2 bits.
  // Normalize.
  //
  q2   = __SEGGER_RTL_CLZ_U32(i0) - 1;  // q2 is either 0 or 1
  i0 <<= q2;
  q1  -= q2;  // Note, not standard normalization, wrap into exponent calculation.
  //
  return __SEGGER_RTL_BitcastToF32(sign + ((__SEGGER_RTL_U32)(0x9D - 31 + q1) << 23) + ((i0 + 64) >> 7));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cbrt_fpu()
*
*  Function description
*    Compute cube root, float.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_cbrt_fpu(float x) {
  float fr;
  float r;
  float sign;
  int   ex;
  int   shx;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(x))) {
    return x;
  }
  //
  sign = x;
  x    = SEGGER_FABSF(x);
  //
  // Compute shx such that (ex - shx) is divisible by 3
  //
  fr = SEGGER_FREXPF(x, &ex);
  shx = ex % 3;
  if (shx > 0) {
    shx -= 3; 
  }
  // 
  // Compute exponent of cube root.
  //
  ex = (ex-shx) / 3;
  fr = SEGGER_LDEXPF(fr, shx);  // 0.125 <= fr < 1.0
  //
  // Compute seed with a quadratic approximation.
  //
  // Sollya:  > fpminimax(x^(1/3), [|0,1,2|], [|single...|], [0.125;1], floating, relative);
  //          0.3812513053417205810546875 + x * (1.072302341461181640625 + x * (-0.4694612026214599609375))
  //
  fr = SEGGER_FMAF(SEGGER_FMAF(-0.46946116f, fr, 1.072302f), fr, 0.3812513f);  // 0.5 <= fr < 1
  r  = SEGGER_LDEXPF(fr, ex); // 6 bits of precision
  //
  // Newton-Raphson iterations double precision at each step.
  //
  for (shx = 0; shx < 3; ++shx) {
    r = SEGGER_FMAF(2.0f/3.0f, r, SEGGER_DIVF(SEGGER_MULF(1.0f/3.0f, x), SEGGER_MULF(r, r)));
  }
  //
  return __SEGGER_RTL_float32_signbit_xor(r, sign);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cbrt()
*
*  Function description
*    Compute cube root, float.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_cbrt(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_cbrt_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_cbrt_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cbrt()
*
*  Function description
*    Compute cube root, double.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_cbrt(double x) {
  double fr, r, sign;
  int    ex, shx;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    return x;
  }
  //
  sign = x;
  x    = SEGGER_FABS(x);
  //
  // Compute shx such that (ex - shx) is divisible by 3
  //
  fr = SEGGER_FREXP(x, &ex);
  shx = ex % 3;
  if (shx > 0) {
    shx -= 3;
  }
  //
  // Compute exponent of cube root.
  //
  ex = (ex-shx) / 3;
  fr = SEGGER_LDEXP(fr, shx);  // 0.125 <= fr < 1.0
  //
  // Compute seed with a quadratic approximation.
  //
  // Sollya:  > fpminimax(x^(1/3), [|0,1,2|], [|single...|], [0.125;1], floating, relative);
  //          0.3812513053417205810546875 + x * (1.072302341461181640625 + x * (-0.4694612026214599609375))
  //
  fr = SEGGER_FMA(SEGGER_FMA(-0.46946116, fr, 1.072302), fr, 0.3812513);  // 0.5 <= fr < 1
  r  = SEGGER_LDEXP(fr, ex); // 6 bits of precision
  //
  // Newton-Raphson iterations double precision at each step.
  //
  for (shx = 0; shx < 4; ++shx) {
    r = SEGGER_FMA(2.0/3.0, r, SEGGER_DIV(SEGGER_MUL(1.0/3.0, x), SEGGER_MUL(r, r))); // 12 to 96 bits
  }
  //
  return __SEGGER_RTL_float64_signbit_xor(r, sign);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_rsqrt_inline()
*
*  Function description
*    Reciprocal square root, float.
*
*  Parameters
*    x - Value to compute reciprocal square root of.
*
*  Return value
*    * If x is +/-zero, return +/-infinity.
*    * If x is positively infinite, return 0.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return reciprocal square root of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_rsqrt_inline(float x) {
#if defined(__SEGGER_RTL_FLOAT32_SQRT)
  //
  return 1.0f / __SEGGER_RTL_FLOAT32_SQRT(x);
  //
#else
  //
  float            xhalf;
  __SEGGER_RTL_U32 ix;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_float32_putative_iszero(x)) {
      return __SEGGER_RTL_float32_signbit_copy(K_INF_F32, x);
    } else if (__SEGGER_RTL_float32_lt_rhs_positive(x, 0)) {
      return K_NAN_F32;
    } else if (__SEGGER_RTL_float32_isinf_inline(x)) {
      return 0;
    } else {
      return x;  // NaN
    }
  }
  //
  xhalf = SEGGER_NEGF(SEGGER_DIV2F(x));
  //
  ix = __SEGGER_RTL_BitcastToU32(x);
  ix = __SEGGER_RTL_U32_C(0x5F375A86) - (ix >> 1);
  x  = __SEGGER_RTL_BitcastToF32(ix);
  //
  // Newton iterations.
  //
  x = SEGGER_MULF(x, SEGGER_FMAF(xhalf, SEGGER_MULF(x, x), 1.5f));
  x = SEGGER_MULF(x, SEGGER_FMAF(xhalf, SEGGER_MULF(x, x), 1.5f));
  x = SEGGER_MULF(x, SEGGER_FMAF(xhalf, SEGGER_MULF(x, x), 1.5f));
  //
  return x;
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_rsqrt_inline()
*
*  Function description
*    Reciprocal square root, double.
*
*  Parameters
*    x - Value to compute reciprocal square root of.
*
*  Return value
*    * If x is +/-zero, return +/-infinity.
*    * If x is positively infinite, return 0.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return reciprocal square root of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_rsqrt_inline(double x) {
#if defined(__SEGGER_RTL_FLOAT64_SQRT)
  //
  return 1.0 / __SEGGER_RTL_FLOAT64_SQRT(x);
  //
#else
  //
  double           xhalf;
  __SEGGER_RTL_U64 ix;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_float64_putative_iszero(x)) {
      return __SEGGER_RTL_float64_signbit_copy(K_INF_F64, x);
    } else if (__SEGGER_RTL_float64_lt_rhs_positive(x, 0)) {
      return K_NAN_F64;
    } else if (__SEGGER_RTL_float64_isinf_inline(x)) {
      return 0;
    } else {
      return x;  // NaN
    }
  }
  //
  xhalf = SEGGER_NEG(SEGGER_DIV2(x));
  //
  ix = __SEGGER_RTL_BitcastToU64(x);
  ix = __SEGGER_RTL_U64_C(0x5FE6EC85E7DE30DA) - (ix >> 1);
  x  = __SEGGER_RTL_BitcastToF64(ix);
  //
  // Newton iterations.
  //
  x = SEGGER_MUL(x, SEGGER_FMA(xhalf, SEGGER_MUL(x, x), 1.5));
  x = SEGGER_MUL(x, SEGGER_FMA(xhalf, SEGGER_MUL(x, x), 1.5));
  x = SEGGER_MUL(x, SEGGER_FMA(xhalf, SEGGER_MUL(x, x), 1.5));
  x = SEGGER_MUL(x, SEGGER_FMA(xhalf, SEGGER_MUL(x, x), 1.5));
  //
  return x;
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_exp_fpu()
*
*  Function description
*    Compute base-e exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_exp_fpu(float x) {
  int   n;
  float g;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(x))) {
    return x;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_gt_rhs_positive(x, M_LN_HUGEF))) {
    return K_INF_F32;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_lt_one_negative(x, -87.336544751f))) {
    return 0;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_lt_rhs_positive(SEGGER_FABSF(x), 1.e-20f))) {
    return 1;
  }
  //
  n = SEGGER_F2I(SEGGER_FMAF(1.44266950408889634074f, x, __SEGGER_RTL_float32_signbit_xor(0.5f, x)));
  g = SEGGER_I2F(n);
  g = SEGGER_FMAF(2.1219444005469058277e-4f, g, SEGGER_FMAF(-0.693359375f, g, x));
  x = SEGGER_MULF(g, g);
  g = SEGGER_MULF(SEGGER_FMAF(SEGGER_FMAF(0.165203300268279130e-4f, x, 0.694360001511792852e-2f), x, 0.249999999999999993e+0f), g);
  x = SEGGER_ADDF(0.5f, SEGGER_DIVF(g, SEGGER_FMAF(SEGGER_FMAF(0.495862884905441294e-3f, x, 0.555538666969001188e-1f), x, SEGGER_SUBF(0.5f, g))));
  //
  return SEGGER_LDEXPF(x, n+1);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_exp_scaled_integer()
*
*  Function description
*    Compute base-e exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Notes
*    This code is based on the [TANG] paper.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_exp_scaled_integer(float x) {
  __SEGGER_RTL_U32 x0;
  __SEGGER_RTL_U32 t;
  __SEGGER_RTL_U32 i0, i1, i2;
  unsigned         q0, q1;
  //
  // Extract exponent to be converted to scale factor.
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  q0 = x0 << 1 >> 24;
  //
  // Exp[x] where x is subnormal or zero is 1.
  //
  if (__SEGGER_RTL_UNLIKELY(q0 == 0)) {
    return 1;
  }
  //
  // Extract i0 as significand.
  //
  i0 = x0 & 0x7FFFFFu;
  //
  // NaN returns itself.
  //
  if (__SEGGER_RTL_UNLIKELY(q0 == 0xFF && i0 != 0)) {
    return x;
  }
  //
  // Handle exceptional cases quickly.
  //
  if ((x0 & __SEGGER_RTL_U32_C(0x80000000)) == 0) {
    //
    // Get rid of overflows quickly.
    //
    if (x0 > K_log_HUGE_U32) {
      return K_INF_F32;
    }
  } else {
    //
    // Get rid of underflows quickly; catches -Inf.
    //
    if (x0 > K_log_SMALL_U32) {
      return 0;
    }
  }
  //
  // Zeros, Infs, and subnormals have been dealt with.  We now have to only
  // consider normals, so materialize hidden bit.
  //
  i0 |= 0x800000;
  //
  // Put i0 into two's complement form for fixed point arithmetic.
  //
  if (x0 & 0x80000000) {
    i0 = 0u - i0;
  }
  //
  if (q0 <= 0x76) {
    //
    // 2b800000 (9.094947E-13) <= x < 3b800000 (0.00390625).
    //
    q0 = 0x76 - q0;
    if (q0 >= 32) {
      return 1.0f;
    }
    //
    // Use approximation Exp[x] ~= 1 + x + (x^2)/2, "x sufficiently small".
    //
    i0  = (__SEGGER_RTL_I32)i0 >> q0;                             // apply scaling
    i0 += (__SEGGER_RTL_I32)__SEGGER_RTL_SMULL_HI(i0, i0) >> 1;   // x = x + (x^2)/2
    i0  = (__SEGGER_RTL_I32)(i0 + (1<<1)) >> 2;                   // round and scale
    i0 += __SEGGER_RTL_U32_C(0x40000000);                         // x = x + 1 last
    q0  = 0;
    //
  } else {
    //
    // Compute adjustment shifts.  
    //
    q0 = q0 - 0x71;
    q1 = 32 - q0;
    //
    // Multiply by 1/Log[2] which is simply Log2[E].  Use Mathematica, BaseForm[N[Log2[E], 12], 16],
    // which is 5c551d94ae, a 39-bit approximation.  We break this into two, a (signed) 31-bit
    // approximation 5c551d95.00 and an 8-bit extension which is 4.ae-5.00 = -0.52.
    //
    __SEGGER_RTL_SMULL(i1, i2, __SEGGER_RTL_U32_C(0x5C551D95), i0);  // i2:i1 contains 62-bit product
    //
    // Convert 62-bit product to Q30 in i1 and round.
    //
    t  = (__SEGGER_RTL_I32)i2 << q0 >> q0;
    i1 = (t << (q0-1)) + (((i1 >> q1) + 1) >> 1);
    //
    // Isolate high order bits of product into i2.
    //
    i2 = (__SEGGER_RTL_I32)(i2 - t) >> q1;
    //
    // Subtract extension product i0 x Log2[E] to provide correction.
    //
    i1 -= (__SEGGER_RTL_I32)(i0 * 0x52) >> (8+1) >> q1;
    //
    // Evaluate polynomial. Coefficients for approximation
    // of (2^x-1)/x calculated using Mathematica:
    // 
    // We use a short Taylor series expanded around zero over a narrow range:
    //   Series[(2^x-1)/x, {x, 0, 4}]
    // =
    //   Log[2] + 1/2 Log[2]^2 x + 1/6 Log[2]^3 x^2 + 1/24 Log[2]^4 x^3 + 1/120 Log[2]^5 x^4
    //
    // Read coefficients directly using:
    //   BaseForm[NumberForm[N[Series[(2^x-1)/x, {x, 0, 4}], 12] * 2^31, ExponentFunction -> (Null &)], 16]
    //
    i0 = __SEGGER_RTL_SMULL_HI(i1, __SEGGER_RTL_I32_C(0x002BB100));
    i0 = __SEGGER_RTL_SMULL_HI(i1, __SEGGER_RTL_I32_C(0x013B2AB7) + ((__SEGGER_RTL_I32)i0 >> 2));
    i0 = __SEGGER_RTL_SMULL_HI(i1, __SEGGER_RTL_I32_C(0x071AC236) + ((__SEGGER_RTL_I32)i0 >> 2));
    i0 = __SEGGER_RTL_SMULL_HI(i1, __SEGGER_RTL_I32_C(0x1EBFBE00) + ((__SEGGER_RTL_I32)i0 >> 2));
    i0 = __SEGGER_RTL_SMULL_HI(i1, __SEGGER_RTL_I32_C(0x58B90BFC) + ((__SEGGER_RTL_I32)i0 >> 2));
    i0 = (__SEGGER_RTL_I32)i0 >> 1;  // Q31
    //
    // Extract 'j' in Tang's paper, leading three bits of i2.  Note that the
    // table is permuted to allow unsigned arithmetic easily.
    //
    q0 = i2 & 7;
    i1 = __SEGGER_RTL_float32_Exp_rot_twojby8[q0];
    i0 = i1 + __SEGGER_RTL_SMULL_HI(i0, i1);  // Multiply leading bits of X to obtain a correction dR; compute R + dR
    //
    // Correct scaling.
    //
    q0 = ((unsigned)i2 + q0) >> 3;
  }
  //
  // ENHANCEMENT: use generic add-normalization macros
  //
  q0 += 0x7E;
  if (i0 < __SEGGER_RTL_U32_C(0x40000000)) {
    i0 <<= 1;
    --q0;
  }
  //
  // Pack result rounding i0.
  //
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(q0, i0, 0));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_exp_inline()
*
*  Function description
*    Compute base-e exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_exp_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_exp_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_exp_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_exp_outline()
*
*  Function description
*    Compute base-e exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_exp_outline(float x) {
  return __SEGGER_RTL_float32_exp_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_exp_fpu()
*
*  Function description
*    Compute base-e exponential, double, using FPU or FPE.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    e^x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_exp_fpu(double x) {
  int    n;
  double g;
  double h;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    if (__SEGGER_RTL_float64_isnan_inline(x)) {
      return x;
    } else if (__SEGGER_RTL_float64_isinf_inline(x)) {
      return __SEGGER_RTL_float64_lt_rhs_positive(x, 0) ? 0 : x;
    }
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_gt_rhs_positive(x, M_LN_HUGE))) {
    return K_INF_F64;
  } else if (__SEGGER_RTL_float64_lt_one_negative(x, -708.39641853226408)) {
    return 0;
  } else if (__SEGGER_RTL_float64_lt_rhs_positive(SEGGER_FABS(x), 1.e-20)) {
    return 1;
  }
  //
  n = SEGGER_D2I(SEGGER_FMA(1.44266950408889634074, x, __SEGGER_RTL_float64_signbit_xor(0.5, x)));
  g = SEGGER_I2D(n);
  g = SEGGER_FMA(2.1219444005469058277e-4, g, SEGGER_FMA(-0.693359375, g, x));
  x = SEGGER_MUL(g, g);
  g = SEGGER_MUL(SEGGER_FMA(SEGGER_FMA(0.165203300268279130e-4, x, 0.694360001511792852e-2), x, 0.249999999999999993e+0), g);
  h = SEGGER_FMA(SEGGER_FMA(0.495862884905441294e-3, x, 0.555538666969001188e-1), x, SEGGER_SUB(0.5, g));
  x = SEGGER_ADD(0.5, SEGGER_DIV(g, h));
  //
  return SEGGER_LDEXP(x, n+1);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_exp_scaled_integer()
*
*  Function description
*    Compute base-e exponential, double, using scaled-integer arithmetic.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    e^x.
*
*  Notes
*    [TANG2]  Table-Lookup Algorithms for Elementary Functions and Their
*             Error Analysis, Proceedings of the 10th Symposium on Computer
*             Arithmetic.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_exp_scaled_integer(double x) {
  __SEGGER_RTL_U64 i0, i1, i2, i3, i4;
  __SEGGER_RTL_U32 h0, h1, h2, h3;
  unsigned         q0, q1;
  //
  // Convert incoming argument, remove sign.
  //
  i0 = __SEGGER_RTL_BitcastToU64(x);
  i1 = i0 & __SEGGER_RTL_U64_C(0x7FFFFFFFFFFFFFFF);
  //
  // Quickly figure if the input lies in the normal input range
  // and, if it does, bypass the exceptional checks.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_U32_C(0x03F6232A) < __SEGGER_RTL_U64_H(i1)-__SEGGER_RTL_U32_C(0x3C900000))) {
    //
    // Exceptional input.
    //
    if (i1 < __SEGGER_RTL_U64_C(0x3C90000000000000)) {
      //
      // Exp[+small] = 1.
      //
      return 1.0;
      //
    } else if (i1 >= K_INF_U64) {
      //
      // Inf or NaN. 
      //
      if (i0 == K_MINUS_INF_U64) {
        //
        // -Inf, return 0
        //
        return 0;
        //
      } else if (i1 == K_INF_U64) {
        //
        // +Inf, return +Inf
        //
        return __SEGGER_RTL_BitcastToF64(i0);
        //
      } else {
        //
        // NaN.
        //
        return __SEGGER_RTL_BitcastToF64(i0 | __SEGGER_RTL_U64_C(0x0008000000000000));
      }
    } else if (i0 & __SEGGER_RTL_U64_C(0x8000000000000000)) {
      //
      // Negative argument, check for underflow.
      //
      if (i1 >= __SEGGER_RTL_U64_C(0x40874910D52D3052)) {
        return 0;
      }
    } else {
      //
      // Positive argument, check for overflow.
      //
      if (i1 > __SEGGER_RTL_U64_C(0x40862E42FEFA39EF)) {
        return K_INF_F64;
      }
    }
  }
  //
  // Remove exponent, set hidden bit, compute scale factor.
  //
  q0 = 0x433 - ((i1 >> 52) & 0x7FF);
  i1 &= __SEGGER_RTL_U64_C(0x000FFFFFFFFFFFFF);
  i1 |= __SEGGER_RTL_U64_C(0x0010000000000000);
  //
  // Convert from signed magnitude to two's complement.
  //
  if (i0 & __SEGGER_RTL_U64_C(0x8000000000000000)) {
    i1 = 0 - i1;
  }
  //
  h0 = __SEGGER_RTL_U64_ROUND(i1);
  //
  // 96x64->96 fractional signed multiply.
  //
  i2  = __SEGGER_RTL_SMULL_X(__SEGGER_RTL_U64_L(i1), K_LOG2_E_H);
  i4  = __SEGGER_RTL_SMULL_X(h0, K_LOG2_E_H);
  i3  = __SEGGER_RTL_SMULL_X(h0, K_LOG2_E_M);
  i4 += (__SEGGER_RTL_I64)i3 >> 32;
  i4 += (__SEGGER_RTL_I64)i2 >> 32;
  //
  i2  = i2 & __SEGGER_RTL_U64_C(0xFFFFFFFF);
  i2 += i3 & __SEGGER_RTL_U64_C(0xFFFFFFFF);
  i2 += __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_U64_L(i1), K_LOG2_E_M);
  i2 += __SEGGER_RTL_SMULL_HI(h0, K_LOG2_E_L);
  //
  i4 += (__SEGGER_RTL_I64)i2 >> 32;
  //
  h2 = i4 >> 32;                             // H
  h1 = i4 & __SEGGER_RTL_U32_C(0xFFFFFFFF);  // M
  //
  h3 = __SEGGER_RTL_SAFE_ASR_I32(h2, q0-42);
  h3 = ((__SEGGER_RTL_I32)h3 >> 1) + (h3 & 1);
  h2 = h2 - __SEGGER_RTL_SAFE_LSL_U32(h3, q0-41);
  //
  // Normalize i4:i2.
  //
  q1 = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_ABS_I32(h2)) - 2;
  i4 = ((__SEGGER_RTL_U64)h2 << 32) + h1; // Reconstruct
  i4 = (i4 << q1) + ((__SEGGER_RTL_U32)i2 >> (32-q1));
  q0 = q0 + q1;
  h1 = __SEGGER_RTL_U64_ROUND(i4);
  //
  q0 -= 64 + 3;
  h2 = q0 + 3;
  if (h2 >= 31) {
    h2 = 31;
  }
  h0 = __SEGGER_RTL_U32_C(0x5761FF94) + (__SEGGER_RTL_SMULL_HI(__SEGGER_RTL_U32_C(0x50C244BE), h1) >> h2);
  h0 = __SEGGER_RTL_U32_C(0x4ECAADBE) + (__SEGGER_RTL_SMULL_HI(h0,                             h1) >> h2);
  // 
  h2 = q0 + 2;
  i0 = __SEGGER_RTL_SMULL_X(h0, h1);
  //
  i0 = __SEGGER_RTL_U64_C(0x71AC235C1282FE2D) + ((__SEGGER_RTL_I64)i0 >> h2);
  i0 = __SEGGER_RTL_SMULL_X(__SEGGER_RTL_U64_ROUND(i0), h1) + __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_U64_L(i0), h1);
  //
  i0 = __SEGGER_RTL_U64_C(0x7AFEF7FE0B163AA2) + ((__SEGGER_RTL_I64)i0 >> h2);
  i0 = __SEGGER_RTL_SMULL_X(__SEGGER_RTL_U64_ROUND(i0), h1) + __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_U64_L(i0), h1);
  //
  i0 = __SEGGER_RTL_U64_C(0x58B90BFBE8E7BCD6) + ((__SEGGER_RTL_I64)i0 >> h2);
  i0 = __SEGGER_RTL_SMULL_X(__SEGGER_RTL_U64_ROUND(i0), h1) + __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_U64_L(i0), h1);
  //
  q0 = q0 - 1;
  h0 = __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_I32_C(0x58B90BFC), __SEGGER_RTL_U64_L(i4));
  i0 += __SEGGER_RTL_SMULL_HI(h0, __SEGGER_RTL_U64_H(i0)) >> q0;
  i0 += (__SEGGER_RTL_I32)h0;
  //
  // Multiply i0 by exponent coefficient and accumulate.
  //
  i1 = __SEGGER_RTL_float64_aExpCoeff[h3 & 63];
  h1 = __SEGGER_RTL_U64_ROUND(i0);
  h0 = __SEGGER_RTL_U64_ROUND(i1);
  i0 = __SEGGER_RTL_SMULL_X (h1, h0)
     + __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_U64_L(i0), h0)
     + __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_U64_L(i1), h1);
  i0 = i1 + ((__SEGGER_RTL_I64)i0 >> q0);
  //
  // Compute number of places to normalize significand.
  // At this point we know q1 will take one of the values 1 or 2
  // as there is always a leading zero bit and at most one
  // additional zero bit from subtraction from 0x4000000000000000
  // of a reduced-range value.
  //
  h3 = (__SEGGER_RTL_I32)h3 >> 6;
  q1 = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_U64_H(i0));
  if (q1 == 2) {
    i0 <<= 1;
    h3  -= 1;
  }
  //
  // Check for negative exponent, which is gradual underflow.
  //
  h3 += 0x3FE;       // Reapply bias
  if (h3 & __SEGGER_RTL_U32_C(0x80000000)) {
    i0 >>= 0u - h3;  // Generate subnormal...
    h3 = 0;          // ...with zero exponent.
  }
  //
  // Round, pack, and return.
  //
  return __SEGGER_RTL_BitcastToF64((((i0 >> 9) + 1) >> 1) +        // Significand
                                   ((__SEGGER_RTL_U64)h3 << 52) +  // Exponent
                                   0);                             // Sign
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_exp_inline()
*
*  Function description
*    Compute base-e exponential, double.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_exp_inline(double x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 2
  return __SEGGER_RTL_float64_exp_scaled_integer(x);
#else
  return __SEGGER_RTL_float64_exp_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_exp_outline()
*
*  Function description
*    Compute base-e exponential, double.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_exp_outline(double x) {
  return __SEGGER_RTL_float64_exp_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_expm1_scaled_integer()
*
*  Function description
*    Compute base-e exponential, modified, double.
*
*  Parameters
*    x - Value to compute exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return base-e exponential of x minus 1 (e**x - 1).
*
*  Notes
*    This code is based on the [TANG] and [EXPM1] papers.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_expm1_scaled_integer(float x) {
  __SEGGER_RTL_U32 x0, x1;
  __SEGGER_RTL_U32 i0, i1, i2, i3;
  unsigned         q0, q1, q2, q3;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Remove sign.
  //
  x1 = x0 << 1;
  //
  // Extract significand and exponent.
  //
  i0 = x0 & __SEGGER_RTL_U32_C(0xFFFFFF);
  q0 = 150 - (x1 >> 24);   // Convert to alignment shift count
  //
  // Materialize hidden bit.
  //
  i0 |= __SEGGER_RTL_U32_C(0x800000);
  //
  // Convert signed-magnitude to two's complement significand.
  //
  if (x0 & FLOAT32_SIGN_MASK) {
    i0 = 0u - i0;
  }
  //
  // Break into approximation classes based on exponent.
  //
  if (__SEGGER_RTL_UNLIKELY((int)q0 <= 16 || (__SEGGER_RTL_I32)x0 >= __SEGGER_RTL_I32_C(0x42B17218) /*Overflow point*/)) {
    //
    // Inf, NaN, and inputs that cause overflow and underflow.
    //
    if (x1 > __SEGGER_RTL_U32_C(0xFF000000)) {
      //
      // NaN input, convert to quiet NaN.
      //
      x0 |= __SEGGER_RTL_U32_C(0x400000);
      //
    } else if ((__SEGGER_RTL_I32)i0 <= 0) {
      //
      // Very small input, Exp[Small]-1 = -1.
      //
      x0 = FLOAT32_SIGN_MASK | __SEGGER_RTL_U32_C(0x3F800000);
      //
    } else {
      //
      // Very large input, set to Inf.
      //
      x0 = K_INF_U32;
    }
    //
    return __SEGGER_RTL_BitcastToF32(x0);
    //
  } else if (q0 >= 53) {
    //
    // 0 <= x <= ~8.42937e-08, Exp[x]-1 = x.
    //
    // ...so return same input.
    //
    return x;
    //
  } else {
    if (q0 > 23) {
      //
      // Limit range of shift count; limiting large shifts in this way
      // has no significance as everything is shifted off in this case.
      //
      q1 = q0 - 4;
      if (q1 > 30) {
        q1 = 30;
      }
      //
      x0 = (((__SEGGER_RTL_I32)i0 >> q1) + 1) >> 1;
      i0 -= x0 << (q1+1);
      //
      // Normalize with leading two bits equal to sign bit.
      //
#if defined(__SEGGER_RTL_CLZ_U32) && !defined(__SEGGER_RTL_CLZ_U32_SYNTHESIZED)
      //
      q1 = __SEGGER_RTL_CLZ_U32(i0 ^ ((__SEGGER_RTL_I32)i0 >> 31)) - 2;
      //
#else
      //
      q1 = 0;
      i1 = i0 ^ ((__SEGGER_RTL_I32)i0 >> 31);
      while ((i1 & __SEGGER_RTL_U32_C(0x20000000)) == 0) {
        i1 <<= 1;
        q1  += 1;
      }
      //
#endif
      //
      q0  += q1;
      i0 <<= q1;
      //
      // Short polynomial approximating Exp[x]-1.
      //
      q1  = q0 - 32;
      i1  = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_I32_C(0x01111BE7));
      i1  = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_I32_C(0x05558DF6) + ((__SEGGER_RTL_I32)i1 >> q1));
      i1  = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_I32_C(0x15555551) + ((__SEGGER_RTL_I32)i1 >> q1));
      i1  = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_I32_C(0x3FFFFFF2) + ((__SEGGER_RTL_I32)i1 >> q1));
      i0 += __SEGGER_RTL_SMULL_HI(i0, i1) >> (q1 - 1);
      //
      x0 += 8;
      i3  = __SEGGER_RTL_float32_Expfm1_exp_j_by_8.Value[x0];
      if (i3 == 0) {
        q2 = q0;
      } else {
        q2 = __SEGGER_RTL_float32_Expfm1_exp_j_by_8.Scale[x0];
        i1 = __SEGGER_RTL_SMULL_HI(i0, i3);
        q3 = q2 - 32;
        if ((int)q3 < 0) {
          i0 = (__SEGGER_RTL_I32)(i1 + ((__SEGGER_RTL_I32)i0 >> (0u-q3))) >> q1;
        } else {
          i0 = (__SEGGER_RTL_I32)(i0 + ((__SEGGER_RTL_I32)i1 >> (0u+q3))) >> (q0 - q2);
        }
        i0 += i3;
      }
    } else {
      //
      // 0.5 <= x <= ...
      //
      q1 = q0 - 4;
      q2 = q0 - 5;
      //
      // Multiply by 1/Log[2] which is simply Log2[E].  Use Mathematica, BaseForm[N[Log2[E], 12], 16],
      // which is 5c551d94ae, a 39-bit approximation.  We break this into two, a (signed) 31-bit
      // approximation 5c551d95.00 and an 8-bit extension which is 4.ae-5.00 = -0.52.
      //
      __SEGGER_RTL_SMULL(i2, i1, __SEGGER_RTL_I32_C(0x5C551D95), i0);  // i2:i1 contains 62-bit product
      //
      // Convert 62-bit product to Q30 in i2 and round.
      //
      i3 = __SEGGER_RTL_SIGN_EXTEND(i1, q2);
      i2 = (i3 << (32 - q1)) + ((__SEGGER_RTL_I32)((i2 >> q2) + 1) >> 1);
      //
      // Subtract extension product i3 x Log2[E] to provide correction.
      //
      i2 -= (__SEGGER_RTL_I32)(((__SEGGER_RTL_I32)i0 >> 4) * 0x52) >> q0;
      //
      // Isolate high order bits of product.
      //
      q1 = (int)(i1 - i3) >> q2;
      //
      i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x002BDEDC));
      i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x013B3101) + ((__SEGGER_RTL_I32)i1 >> 2));
      i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x071AC1F1) + ((__SEGGER_RTL_I32)i1 >> 2));
      i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x1EBFBDFE) + ((__SEGGER_RTL_I32)i1 >> 2));
      i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x58B90BFC) + ((__SEGGER_RTL_I32)i1 >> 2));
      //
      q3  = __SEGGER_RTL_SIGN_EXTEND(q1, 3);  // q3 [-4, +4].
      q2  = (int)(q1 - q3) >> 3;
      //
      i0  = __SEGGER_RTL_float32_Exp_std_2tojby8[q3 + 4];
      i0 += __SEGGER_RTL_SMULL_HI((__SEGGER_RTL_I32)i1 >> 1, i0);
      //
      if ((int)q2 <= 0) {
        q2 = 0u - q2;
        if (q2 > 31) {
          i0 = __SEGGER_RTL_U32_C(0xC0000001);
        } else {
          i0 = __SEGGER_RTL_U32_C(0xC0000000) + ((__SEGGER_RTL_I32)i0 >> q2);
        }
        q2 = 30;
      } else {
        if (q2 < 30) {
          i0 += __SEGGER_RTL_I32_C(0xC0000000) >> q2;
        } else {
          i0 += __SEGGER_RTL_I32_C(0xC0000000) >> 30;
        }
        q2 = 30 - q2;
      }
    }
    //
    // Set i1 = |i0| as IEEE representation is signed magnitude.
    //
    i1 = __SEGGER_RTL_ABS_I32(i0);
    //
    // Normalize and calculate IEEE exponent.
    //
#if defined(__SEGGER_RTL_CLZ_U32) && !defined(__SEGGER_RTL_CLZ_U32_SYNTHESIZED)
    //
    // There is a fast CLZ available, so use it to normalize.
    //
    q1 = __SEGGER_RTL_CLZ_U32(i1);
    i1 <<= q1 - 1;
    q1 = 157 - q2 - q1;
    //
#else
    //
    // By exhaustion, we know that i1 only needs 1, 2, or 3 shifts
    // to normalize.  Use this to do the normalization.
    //
    // NOTE: If the approximation calculations above are ever
    // changed, this may not apply and a rerun of normalization
    // analysis will be required!
    //
    if ((i1 & __SEGGER_RTL_U32_C(0x40000000)) != 0) {
      q1 = 157 - q2 - 1;
    } else if ((i1 & __SEGGER_RTL_U32_C(0x20000000)) != 0) {
      i1 <<= 1;
      q1 = 157 - q2 - 2;
    } else {
      i1 <<= 2;
      q1 = 157 - q2 - 3;
    }
    //
#endif
    //
    // Round, pack, apply sign, and return.
    //
    return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(q1, i1, i0));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_exp10_inline()
*
*  Function description
*    Compute base-10 exponential, double (BSD).
*
*  Parameters
*    x - Value to compute base-10 exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-10 exponential of x.
*
*  Notes
*    Commonly found in Linux, BSD, and GNU C libraries.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_exp10_inline(double x) {
  return SEGGER_EXP(SEGGER_MUL(M_LN10_DBL, x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_exp10_inline()
*
*  Function description
*    Compute base-10 exponential, float (BSD).
*
*  Parameters
*    x - Value to compute base-10 exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-10 exponential of x.
*
*  Notes
*    Commonly found in Linux, BSD, and GNU C libraries.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_exp10_inline(float x) {
  return SEGGER_EXPF(SEGGER_MULF(M_LN10_FLT, x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_log_scaled_integer()
*
*  Function description
*    Compute natural logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_log_scaled_integer(float x) {
  __SEGGER_RTL_U32 x0, x1, x2, x3;
  unsigned         q0, q1, q2;
  unsigned         j;
  //
  x1 = x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Cast out sign.
  //
  x0 <<= 1;
  //
  // Log[+-0] = -Inf.
  //
  if (__SEGGER_RTL_UNLIKELY(x0 == 0)) {
    return __SEGGER_RTL_BitcastToF32(K_MINUS_INF_U32);
  }
  //
  // Special exponent?
  //
  q0 = x0 >> 24;
  if (__SEGGER_RTL_UNLIKELY(q0 == 0xff)) {
    if (__SEGGER_RTL_U32_C(0x7F800000) >= x1) {
      //
      // Log[Inf] is Inf, Log[NaN] is NaN.
      //
      return x;
      //
    } else {
      //
      // Log[-Inf] or NaN with sign bit set.
      //
      return __SEGGER_RTL_BitcastToF32(x1 | __SEGGER_RTL_U32_C(0x400000));
    }
  }
  //
  // Handle negative arguments after special arguments are dealt with.
  //
  if (__SEGGER_RTL_UNLIKELY(x1 & __SEGGER_RTL_U32_C(0x80000000))) {
    //
    // Log[-ve] = NaN.
    //
    return K_NAN_F32;
  }
  //
  // Extract significand.
  //
  x0 = x1 & __SEGGER_RTL_U32_C(0xFFFFFF);
  //
  // Materialize hidden bit for normals.
  //
  if (q0) {
    //
    // Standard normalized number.
    //
    x0 = x0 | __SEGGER_RTL_U32_C(0x800000);
    --q0;
  }
  //
  // Argument = x0/2^q0, however subsume below (*).
  // q0 = 0x95 - q0;
  //
  // We could use an if-else above, but CLZ works faster on processors
  // that have it.
  //
  q1 = __SEGGER_RTL_CLZ_U32(x0);
  x0 = x0 << q1 >> 1;
  //
  // (*) Compute x3 far in advance of when it's needed, but x3 is easily
  // computed right now.   x3 has limited range and could be forced into a
  // 16-bit type, but it's multiplied up later to be 32 bits wide.
  // Logically...  x3 = 31 - q1 - (0x95 - q0), but rearranged:
  //
  x3 = 31 - 0x95 - q1 + q0;
  //
  // Log[2^k * x] = k Log[2] + Log[x * r] - Log[r].
  // For arguments sufficiently far from 1.0 we use
  //    Log[2^k * x] = k Log[2] - Log[x3] - Log[r2] + Log[x * x3 * r2],
  // with x * x3 * r2 very close to 1.
  //
  // Leading five bits of significand for table lookup.  This happens to
  // always be in the range [0, 16] as we extract 0x7F+2 = 0x81 and then the
  // leading 5 bits are 0x10.
  //
  j = (__SEGGER_RTL_I32)((x0 & __SEGGER_RTL_U32_C(0x3F000000)) + __SEGGER_RTL_U32_C(0x02000000)) >> 26;
  if (j > 7) {
    ++x3;
  }
  //
  // Normalize.  Always 1 or more leading zeros returned by clz, but
  // result is aligned correctly when negative.
  //
  x0 = ((x0 >> 7) * __SEGGER_RTL_logf_1_over_1_plus_j_by_16[j]) << 3;
  q0 = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_ABSX_I32(x0));
  x0 <<= q0-1;
  //
  // If x0 is significant in the polynomial, expand the polynomial out.
  //
  if (q0 < 31) {
    //
    // Polynomial that approximates Log[1+x]-x, optimized.
    // Use Mathematica to generate the starting coefficients:
    //    BaseForm[N[ExpandAll[PadeApproximant[Log[1 + x] - x, {x, 0, {6, 0}}]/2], 16], 16]
    //
    q2 = q0 + 1;
    x1 = __SEGGER_RTL_SMULL_HI(x0, __SEGGER_RTL_U32_C(0xEAA2199C));
    x1 = __SEGGER_RTL_SMULL_HI(x0, ((__SEGGER_RTL_I32)x1 >> q2) + __SEGGER_RTL_I32_C(0x19A1A9AA));
    x1 = __SEGGER_RTL_SMULL_HI(x0, ((__SEGGER_RTL_I32)x1 >> q2) - __SEGGER_RTL_I32_C(0x1FFFFEE4));
    x1 = __SEGGER_RTL_SMULL_HI(x0, ((__SEGGER_RTL_I32)x1 >> q2) + __SEGGER_RTL_I32_C(0x2AAAA9E1));
    x1 = __SEGGER_RTL_SMULL_HI(x0, ((__SEGGER_RTL_I32)x1 >> q2) - __SEGGER_RTL_I32_C(0x40000000));
    x1 = __SEGGER_RTL_SMULL_HI(x0, x1);
    //
    // Accumulate Log[1+sig/16].
    //
    if (j & 0xF) {
      x1 = ((__SEGGER_RTL_I32)x1 >> q0);
      q0 = __SEGGER_RTL_logf_log_1_plus_j_by_16.Scale[j];
      q2 = q0 - q2;
      x1 += (x0 & (~0u >> q2));
      q2 = 32 - q2;
      x0 = __SEGGER_RTL_logf_log_1_plus_j_by_16.Value[j] + ((__SEGGER_RTL_I32)x0 >> q2) + ((__SEGGER_RTL_I32)x1 >> q2);
    } else {
      // Special case where Log[1] = 0.
      x0 = ((__SEGGER_RTL_I32)x0 >> 1) + ((__SEGGER_RTL_I32)x1 >> q2);
      q0 += 32;
    }
  } else {
    //
    // If argument is exactly 1.0, Log[1.0] is exactly 0.0.
    //
    if (x3 == 0) {
      return 0;
    }
    q0 += 32;
  }
  //
  if (x3) {
    //
    // Multiply by Log[2] with an additional 8 bits of extended precision.
    // Log[2] is 0x58B90BFB|E8E, but recast this as 0x58B90BFB|E8E = 0x58B90BFC|000 - 0|172.
    //
    q2 = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_ABS_I32(x3));
    __SEGGER_RTL_SMULL(x1, x2, __SEGGER_RTL_U32_C(0x58B90BFC), x3 << (q2 - 2));
    //
    // Compute extended product correction low order bits.
    //
    q2 -= 3;
    x3 *= ((__SEGGER_RTL_I32)(-0x172)>>1);
    x3 += x1 >> (q2-2 - 8);
    //
    // Sum into x3.
    //
    if (q0 < q2+32) {
      //
      // x0 and x3 differ by less than 2^32, so align and sum.
      //
      q1 = q0 - q2;
      x2 += (__SEGGER_RTL_I32)x0 >> q1;
      //
      // Discard high order bits.
      //
      x0 &= ~__SEGGER_RTL_U32_C(0) >> (32-q1);
      //
      // Shift aligning on bit #10.
      //
      if (q0 < 32+10) {
        x3 += x0 << (32+10 - q0);
      } else {
        x3 += (__SEGGER_RTL_I32)x0 >> (q0 - 32 - 10);
      }
    } else {
      x3 += (__SEGGER_RTL_I32)x0 >> (q0 - 32 - 10);
    }
    //
    // Accumulate.
    //
    x0 = x2 + ((__SEGGER_RTL_I32)x3 >> (32 + 10 - q2));
    q0 = q2;
  }
  //
  // Round -ve values up and leave +ve values unchanged.  This
  // also saves the sign of the result.
  //
  x0 = x0 + (x0 >> 31);
  //
  // Normalize, but there's a wrinkle because x0 is a two's complement
  // fraction, so we need to save the sign (above), convert to signed-magnitude,
  // and normalize the significand.
  //
  x1 = __SEGGER_RTL_ABS_I32(x0);
  q1 = __SEGGER_RTL_CLZ_U32(x1);
  x1 = x1 << q1 >> 1;
  //
  // Pack.
  //
  q0 = 0x9D - q0 - q1;
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(q0, x1, x0));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_log_fpu()
*
*  Function description
*    Compute natural logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return -Inf.
*    * If x is +Inf, return +Inf.
*    * ELse, return base-e logarithm of x.
*
*  Notes
*    [C&W]   Chapter 5, ALOG/ALOG10
*    [BEEBE] Chapter 10, Exponential and logarithm
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_log_fpu(float x) {
  float r;
  int   memn;
  int   n;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial_or_negative(x))) {
    if (__SEGGER_RTL_float32_isnan_inline(x)) {
      return x;
    } else if (__SEGGER_RTL_float32_putative_iszero(x)) {
      return __SEGGER_RTL_BitcastToF32(K_MINUS_INF_U32);
    } else if (__SEGGER_RTL_float32_lt_rhs_positive(x, 0)) {
      return K_NAN_F32;
    } else {  // Is +Inf
      return x;
    }
  }
  //
  // Doing this splits the live range for n leading to better code.
  //
  x = SEGGER_FREXPF(x, &memn);
  n = memn;
  //
  if (__SEGGER_RTL_float32_gt_rhs_positive(x, (float)M_SQRT_1_2)) {
#if __SEGGER_RTL_FP_HW > 0
    x = SEGGER_DIVF(SEGGER_SUBF(x, 1), SEGGER_FMAF(0.5f, x, 0.5f));
#else
    x = SEGGER_DIVF(SEGGER_SUBF(x, 1), SEGGER_ADDF(SEGGER_DIV2F(x), 0.5f));
#endif
  } else {
    n -= 1;
    x = SEGGER_SUBF(x, 0.5f);
    x = SEGGER_DIVF(x, SEGGER_FMAF(0.5f, x, 0.5f));
  }
  //
  r = SEGGER_MULF(x, x);
  x = SEGGER_FMAF(x,
                  SEGGER_DIVF(SEGGER_MULF(r, -0.5527074855E+0f),
                              SEGGER_ADDF(r, -0.6632718214E+1f)),
                  x);
  r = SEGGER_I2F(n);
  //
  return SEGGER_FMAF(r, 355.0f/512.0f, SEGGER_FMAF(r, -2.121944400546905827679e-4f, x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_log_inline()
*
*  Function description
*    Compute natural logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_log_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_log_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_log_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_log_outline()
*
*  Function description
*    Compute natural logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x.
*/
static float __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float32_log_outline(float x) {
  return __SEGGER_RTL_float32_log_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_log_inline()
*
*  Function description
*    Compute natural logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_log_inline(double x) {
  double r;
  int    memn;
  int    n;
  //
  if (__SEGGER_RTL_float64_isspecial_or_negative(x)) {
    if (__SEGGER_RTL_float64_isnan_inline(x)) {
      return x;
    } else if (__SEGGER_RTL_float64_putative_iszero(x)) {
      return __SEGGER_RTL_BitcastToF64(FLOAT64_SIGN_MASK | K_INF_U64);
    } else if (__SEGGER_RTL_float64_lt_rhs_positive(x, 0)) {
      return K_NAN_F64;
    } else { // Must be +Inf
      return x;
    }
  }
  //
  x = SEGGER_FREXP(x, &memn);
  n = memn;
  //
  if (__SEGGER_RTL_float64_gt_rhs_positive(x, M_SQRT_1_2)) {
    x = SEGGER_DIV(SEGGER_ADD(x, -1), SEGGER_ADD(SEGGER_DIV2(x), 0.5));
  } else {
    n -= 1;
    x = SEGGER_SUB(x, 0.5);
    x = SEGGER_DIV(x, SEGGER_ADD(SEGGER_DIV2(x), 0.5));
  }
  //
  r = SEGGER_MUL(x, x);
  r = SEGGER_MUL(r,
                 SEGGER_DIV(SEGGER_ADD(__SEGGER_RTL_float64_PolyEvalP_2(r, __SEGGER_RTL_float64_Log.Poly.P, K_LOG_P_DBL), -0.64124943423745581147e2),
                            __SEGGER_RTL_float64_PolyEvalQ_3(r, __SEGGER_RTL_float64_Log.Poly.Q, K_LOG_Q_DBL)));
  x = SEGGER_FMA(x, r, x);
  r = SEGGER_I2D(n);
  //
  return SEGGER_FMA(r, 355.0/512.0, SEGGER_FMA(r, -2.121944400546905827679e-4, x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_log_outline()
*
*  Function description
*    Compute natural logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x.
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_log_outline(double x) {
  return __SEGGER_RTL_float64_log_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_log1p_inline()
*
*  Function description
*    Compute natural logarithm plus one, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*
*  Notes
*    [BEEBE] Chapter 10, "Logarithms near one"
*    [TANG3] Section 2.3, "The LOG1P function"
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_log1p_inline(float x) {
  float w;
  //
  w = SEGGER_ADDF(1, x);
  if (__SEGGER_RTL_float32_eq_bitwise(w, 1)) {
    return x;
  } else {
    return SEGGER_MULF(SEGGER_LOGF(w), SEGGER_DIVF(x, SEGGER_SUBF(w, 1)));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_log1p_outline()
*
*  Function description
*    Compute natural logarithm plus one, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*
*  Notes
*    [BEEBE] Chapter 10, "Logarithms near one"
*    [TANG3] Section 2.3, "The LOG1P function"
*/
static float __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float32_log1p_outline(float x) {
  return __SEGGER_RTL_float32_log1p_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_log1p_inline()
*
*  Function description
*    Compute natural logarithm plus one, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*
*  Notes
*    [BEEBE] Chapter 10, "Logarithms near one"
*    [TANG3] Section 2.3, "The LOG1P function"
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_log1p_inline(double x) {
  double w;
  //
  w = SEGGER_ADD(1, x);
  if (__SEGGER_RTL_float64_eq_finite(w, 1)) {
    return x;
  } else {
    return SEGGER_MUL(SEGGER_LOG(w), SEGGER_DIV(x, SEGGER_SUB(w, 1)));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_log1p_outline()
*
*  Function description
*    Compute natural logarithm plus one, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*
*  Notes
*    [BEEBE] Chapter 10, "Logarithms near one"
*    [TANG3] Section 2.3, "The LOG1P function"
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_log1p_outline(double x) {
  return __SEGGER_RTL_float64_log1p_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_log10_inline()
*
*  Function description
*    Compute common logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_log10_inline(float x) {
  return SEGGER_MULF(SEGGER_LOGF(x), M_INV_LN10_FLT);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_log10_inline()
*
*  Function description
*    Compute common logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_log10_inline(double x) {
  return SEGGER_DIV(SEGGER_LOG(x), M_LN10_DBL);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_log2()
*
*  Function description
*    Compute base-2 logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_log2(float x) {
  return SEGGER_MULF(SEGGER_LOGF(x), M_INV_LN2_FLT);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_log2()
*
*  Function description
*    Compute base-2 logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_log2(double x) {
  return SEGGER_DIV(SEGGER_LOG(x), M_LN2_DBL);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sincos_fpu()
*
*  Parameters
*    x       - Angle in radians to compute sine or cosine of.
*    y       - Phase offset.
*    sign    - Sign of incoming argument.
*    coscase - Flag indicating cosine is required.
*
*  Return value
*    Calculated sine or cosine of angle.
*
*  Notes
*    [C&W] Chapter 8, "SIN/COS".
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_sincos_fpu(float x, float y, float sign, int coscase) {
  float xn;
  float f;
  float g;
  int   n;
  //
  // Range reduction.
  //
  n  = SEGGER_F2I(SEGGER_DIVF(SEGGER_ADDF(y, (float)M_PI_2), (float)M_PI));
  xn = SEGGER_I2F(n);
  if ((n & 1) != 0) {
    sign = -sign;
  }
  if (coscase) {
    xn = SEGGER_SUBF(xn, 0.5f);
  }
  //
  f = SEGGER_ADDF(SEGGER_ADDF(x, SEGGER_MULF(xn, -3.1416015625f)),
                  SEGGER_MULF(xn, 8.908910206761537356617e-6f));
  //
  if (__SEGGER_RTL_float32_ge_finite(SEGGER_FABSF(f), 1.e-10f)) {
    //
    g = SEGGER_MULF(f, f);
    f = SEGGER_FMAF(__SEGGER_RTL_float32_PolyEvalP_5(g,
                                                     __SEGGER_RTL_float32_SinCos.Poly.P,
                                                     K_SINCOS_P_FLT),
                    f,
                    f);
  }
  //
  return __SEGGER_RTL_float32_signbit_xor(f, sign);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_SinReduce()
*
*  Function description
*    Reduce input modulo pi into hexadecant plus sign.
*
*  Return value
*    Output is breakpoint [0, 15] for sine together with sign [bit 5].
*/
static __SEGGER_RTL_INLINE unsigned __SEGGER_RTL_float32_SinReduce(__SEGGER_RTL_U32 *pI, unsigned *pQ) {
  __SEGGER_RTL_U32         d3, d2, d1, d0;  // Triple-precision and Quadruple-precision product register
  __SEGGER_RTL_U32         i0;
  __SEGGER_RTL_U32         x0, x1;
  unsigned                 q0, q1;
  const __SEGGER_RTL_U16 * pPi;
  //
  i0 = *pI;
  q0 = *pQ;
  //
  // Input is i0/2^q0.
  //
  // Reduce input modulo 2pi.
  //
  if ((int)q0 >= 16) {
    //
    // 0x3f000000 (0.5) <= x < 0x43800000 (256), fast reduction case.
    //
    // Multiply i0 (30 bits) by extended 64-bit 1/pi generating a 94-bit
    // product in x0:d1:d2.
    //
    x0 = 0;
    __SEGGER_RTL_SMULL(d2, d1, __SEGGER_RTL_U32_C(0x27220A95), i0);  // 0x517CC1B727220A95 is 1/pi to rounded to 64 bits
    __SEGGER_RTL_SMLAL(d1, x0, __SEGGER_RTL_U32_C(0x517CC1B7), i0);
    //
    // Calculate breakpoint index j from the leading four bits of the
    // remainder after dividing by pi.
    //
    q1 = 32 - (q0 - 4);
    d0 = (__SEGGER_RTL_I32)x0 << q1 >> q1;
    x0 = (__SEGGER_RTL_I32)(x0 - d0) >> (q0 - 4);
    //
    if ((d0 ^ ((__SEGGER_RTL_I32)d0 >> 31)) == 0) {
      //
      // Leading 32 bits are just sign bits, all significance
      // contained in lower 64 bits.  Semi-normalize d0:d1:d2 by
      // shifting 30 bits, drop to (maximum) 62 bits of precision
      // in d0:d1.
      //
      d0 = (d0 << 30) | d1 >> 2;
      d1 = (d1 << 30) | d2 >> 2;
      q0 += 30;
    }
    //
    // Normalize to 31 bits.
    //
    q1 = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_ABS_I32(d0)) - 1;
    i0 = __SEGGER_RTL_LSL_U64_HI(d0, d1, q1);
    q0 = q0 + q1 - 2;
    //
    // Scaled value in i0/2^q0.
    //
  } else {
    //
    // 0x43800000 (256) <= x < 0x7F80000 (Inf), slow path.
    //
    // Use Payne-Hanek reduction algorithm.  See [NG].
    //
    if ((int)q0 <= 0) {
      x0 = (int)(~q0) >> 4;
    } else {
      x0 = 0;
    }
    //
    // x0 now in [-1, 6].
    //
    d1 = d0 = 0;
    pPi = &__SEGGER_RTL_inv_pi[x0+1];
    __SEGGER_RTL_UMULL(d3, d2, i0, pPi[5] | ((__SEGGER_RTL_U32)(pPi[4]) << 16));
    __SEGGER_RTL_UMLAL(d2, d1, i0, pPi[3] | ((__SEGGER_RTL_U32)(pPi[2]) << 16));
    __SEGGER_RTL_UMLAL(d1, d0, i0, pPi[1] | ((__SEGGER_RTL_U32)(pPi[0]) << 16));
    //
    x0 <<= 4;
    x0 = 35 - x0 - q0;
    x1 = (__SEGGER_RTL_I32)x0 >> 5;
    q0 = x0 - (x1 << 5);
    //
    // We need to shift left by q0+1 bits in the following code (*) where
    // q0+1 could possibly be 32.  Because shifting by 32 bits is undefined
    // in the C language for 32-bit quantities, we reduce the shift to a
    // maximum of 31 bits and add a single one-bit shift to compensate.
    //
    q1 = 31 - q0;
    if ((int)q0 < 5) {
      i0 = d1;
      x0 = (d0 << q0 << 1) | (i0 >> q1);  // (*)
      d1 = d2; d2 = d3;
    } else {
      if (x1 != 0) {
        d0 = d1; d1 = d2; d2 = d3;
      }
      i0 = d0;
      x0 = i0 >> q1;
    }
    i0 = (i0 << q0 << 1) | (d1 >> q1);  // (*)
    d1 = (d1 << q0 << 1) | (d2 >> q1);  // (*)
    x0 -= (__SEGGER_RTL_I32)i0 >> 31;
    q0 = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_ABS_I32(i0));
    i0 = i0 << (q0 - 1) | (d1 >> (32 - q0) >> 1);
    q0 = q0 + 33;
    x0 = x0 & 31;
  }
  //
  *pI = i0;
  *pQ = q0;
  //
  return x0;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_SinKernel()
*
*  Function description
*    Compute general sine.
*
*  Parameters
*    x0    - Input angle, hexadecant.
*    i0    - Input angle, integer.
*    q0    - Input angle, scale.
*    pI    - Pointer to object that receives the sine of x0, integer.
*    pQ    - Pointer to object that receives the sine of x0, scale.
*    pSign - Pointer to object that receives the sign of calculated sine.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_SinKernel(__SEGGER_RTL_U32   x0,
                                                               __SEGGER_RTL_U32   i0,
                                                               __SEGGER_RTL_U32   q0,
                                                               __SEGGER_RTL_U32 * pI,
                                                               unsigned         * pQ,
                                                               __SEGGER_RTL_U32 * pSign) {
  __SEGGER_RTL_U32 i1, i2, i3;  // Working registers when calculating and summing factors
  __SEGGER_RTL_U32 q1, q2;      // Working register scale factors (independent of i registers)
  __SEGGER_RTL_U32 Pr, Qr;
  __SEGGER_RTL_U32 Sj_i, Cj_i;
  __SEGGER_RTL_U32 Sj_q, Cj_q;
  const SEGGER_RTL_SIN_COS_PARA *pPara;
  //
  // Approximate sin(r)-r and cos(r)-1 by two polynomials p(r) and q(r).
  // Coefficients found by applying Remez algorithm over the interval
  // [0, 1/16] using odd and even polynomials respectively.
  //
  // Reconstruct sin(x) = sin(c[j] + r)
  //                    = sin(c[j])cos(r) + cos(c[j])sin(r)
  //                   ~= sin(c[j])(q(r)+1) + cos(c[j])(p(r)+r)
  //                   ~= sin(c[j]) + sin(c[j])q(r) + r.cos(c[j]) + cos(c[j])p(r)
  //                   ~= sin(c[j]) + r.cos(c[j]) + sin(c[j])q(r) + cos(c[j])p(r)
  //                   ~= S[j] + r.C[j] + S[j]q(r) + C[j]p(r)
  //
  // c[j] = sin(j pi / 16) for j = 0, ..., 15.
  //
  // Normalize product (with rounding) to 31 bits.
  //
  __SEGGER_RTL_SMULL(i1, i0, i0, __SEGGER_RTL_U32_C(0x6487ED51));
  i0 = (i0<<1) + (((i1 >> 30) + 1) >> 1);
  //
  q2 = q0 - 32;
  i1 = (int)i0 >> q2;
  //
  // Calculate p(r)/r^2 because r^2 is a common factor in p(r) and q(r).
  // Using this, in the general branch, we sum the factors that use
  // the polynomials and then multiply by r^2 incurring one high-order
  // multiplication rather than one for each factor.
  //
  i1 = __SEGGER_RTL_SMULL_HI(i1, i1);
  Pr = __SEGGER_RTL_SMULL_HI(i1, __SEGGER_RTL_U32_C(0x0443E8E7));
  Pr = __SEGGER_RTL_SMULL_HI(i0, Pr - __SEGGER_RTL_U32_C(0x5555550E));
  //
  if ((x0 & 15) == 0) {
    //
    // As sin(r) = 0 and cos(r) = 1, the approximation is simplified:
    //
    //   sin(x) ~= S[j] + r C[j] + S[j]q(r) + C[j]p(r)
    //          ~= 0 + r*1 + 0*q(r) + 1*p(r)
    //          ~= r + p(r)
    //
    __SEGGER_RTL_SMULL(i2, i1, i0, i0);           // r*r, rounded.
    i1 = (i1 << 1) + ((__SEGGER_RTL_I32)((i2 >> 30) + 1) >> 1);
    i1 = __SEGGER_RTL_SMULL_HI(Pr, i1);           // p(r)
    q1 = (q0 << 1) - 62;                          // == (q0-31) + (q0-31)
    i0 = i0 + __SEGGER_RTL_SAFE_ASR_I32(i1, q1);  // r + p(r)
    //
  } else {
    //
    // Calculate q(r)/r^2.
    //
    Qr = __SEGGER_RTL_SMULL_HI(i1, __SEGGER_RTL_U32_C(0x0AA96B54)) - __SEGGER_RTL_U32_C(0x7FFFFF07);
    //
    // Extract breakpoint parameters.
    //
    pPara = &__SEGGER_RTL_float32_SinCosParas[x0 & 15];
    Sj_i = pPara->S_value;
    Sj_q = pPara->S_scale;
    Cj_i = pPara->C_value;
    Cj_q = pPara->C_scale + q0;
    //
    // Reconstruct sin(x) as above.
    //
    i1 = __SEGGER_RTL_SMULL_HI(Pr, Cj_i);      // C[j]p(r)/r^2
    i3 = __SEGGER_RTL_SMULL_HI(Qr, Sj_i);      // S[j]q(r)/r^2
    i3 = i3 + ((__SEGGER_RTL_I32)i1 >> (Cj_q - Sj_q + 1));  // S[j]q(r)/r^2 + C[j]p(r)/r^2
    i2 = __SEGGER_RTL_SMULL_HI(i0, i0);        // Keep significance here...
    i3 = __SEGGER_RTL_SMULL_HI(i3, i2);        // S[j]q(r) + C[j]p(r)...
    q1 = Sj_q + (q2 << 1);
    i2 = __SEGGER_RTL_SMULL_HI(i0, Cj_i);      // r C[j]
    if (i2 == 0) {                             // ==> Cj[i] == 0 and Sj[i] == 1.
      Cj_q = q1;
    }
    i2 = i2 + ((__SEGGER_RTL_I32)i3 >> (q1 - Cj_q));         // r C[j] + S[j]q(r) + C[j]p(r)
    i0 = Sj_i + __SEGGER_RTL_SAFE_ASR_I32(i2, Cj_q - Sj_q);  // S[j] + r C[j] + S[j]q(r) + C[j]p(r)
    q0 = Sj_q;
  }
  //
  i1 = __SEGGER_RTL_ABS_I32(i0);
  //
  // Store sin(x) results.
  //
  *pI    = i1;
  *pQ    = (unsigned)q0;
  *pSign = (x0 << (31-4)) ^ i0;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sin_scaled_integer()
*
*  Function description
*    Calculate sine, float.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*
*  Notes
*    [TANG2]  Table-Lookup Algorithms for Elementary Functions and Their
*             Error Analysis, Proceedings of the 10th Symposium on Computer
*             Arithmetic.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_sin_scaled_integer(float x) {
  __SEGGER_RTL_U32 x0;
  __SEGGER_RTL_U32 i0, i1, i2;  // Working registers when calculating and summing factors
  unsigned         q0, q1;      // Working register scale factors (independent of i registers)
  __SEGGER_RTL_U32 sign;
  __SEGGER_RTL_U32 sin_sign;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Extract biased exponent.
  //
  q0 = FLOAT32_EXPONENT(x0);
  //
  // Sin(-0) = -0.
  // Sin(+0) = +0.
  // Sin(x)  = x for |x| < 0x39800000 (2.44140625e-4).
  //
  if (q0 <= 114) {
    return x;
  }
  //
  // Nonzero x.  Save sign, extract significand, compute
  // scale factor such that x0 = i0/2^q0.
  //
  sign = x0;
  i0   = (x0 & FLOAT32_SIGNIFICAND_MASK) | FLOAT32_HIDDEN_MASK;
  q0   = 150 - q0;
  //
  // Classification of x.
  //
  if (__SEGGER_RTL_UNLIKELY(q0 == 150u - 255u)) {
    //
    // sin(Inf) = NaN.
    // sin(NaN) = NaN.
    //
    return __SEGGER_RTL_BitcastToF32(x0 | __SEGGER_RTL_U32_C(0x400000));
    //  
  } else if ((int)q0 >= 25) {
    //
    // 0x39800000 (2.44140625e-4) <= x <= 0x3f000000 (0.5).
    //
    // Normalize.
    //
    i0 <<= 7;
    q0 +=  7;
    //
    // Truncated polynomial approximation for sin(x) over [0, 0.5].
    //
    i1 = (__SEGGER_RTL_I32)i0 >> (q0 - 32);
    i1 = __SEGGER_RTL_SMULL_HI(i1, i1);
    i2 = __SEGGER_RTL_SMULL_HI(i1, -__SEGGER_RTL_I32_C(0x0019D562));
    i2 = __SEGGER_RTL_SMULL_HI(i1, +__SEGGER_RTL_I32_C(0x04443DDE) + i2);
    i2 = __SEGGER_RTL_SMULL_HI(i1, -__SEGGER_RTL_I32_C(0x55555512) + i2);
    i2 = __SEGGER_RTL_SMULL_HI(i0, i2);
    i0 = i0 + ((__SEGGER_RTL_I32)(i2+1) >> 1);
    //
    // Normalize.
    //
    q1 = __SEGGER_RTL_CLZ_U32(i0);
    i0 <<= (q1-1);
    q0  += q1;
    //
    return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(157 - q0, i0, sign));
    //
  } else {
    //
    // Reduce i0/2^q0 modulo 2*pi with x0 containing the sign of the
    // input and the leading four bits of the remainder which indicate
    // the breakpoint index to use.
    //
    x0 = __SEGGER_RTL_float32_SinReduce(&i0, &q0);
    __SEGGER_RTL_float32_SinKernel(x0, i0, q0, &i1, &q1, &sin_sign);
    q0 = __SEGGER_RTL_CLZ_U32(i1);
    //
    // By exhaustion, no zero is ever generated.  If the approximation
    // changes and this happens, the following will cater for that possibility.
    //
    // ...PACK(i0 == 0 ? ~0u : 157 - (q0+q1), ...
    //
    return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(157 - (q0+q1), i1 << (q0-1), sign ^ sin_sign));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sin_inline()
*
*  Function description
*    Calculate sine, float.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_sin_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_sin_scaled_integer(x);
#else
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(x))) {
    if (!__SEGGER_RTL_float32_isfinite_inline(x)) {
      return K_NAN_F32;  // Sin[NaN] = Inf, Sin[NaN] = NaN.
    } else {  // can only be subnormal or zero, sin(x) = x for small x
      return x;
    }
  }
  if (__SEGGER_RTL_float32_lt_rhs_positive(x, 0)) {
    return __SEGGER_RTL_float32_sincos_fpu(-x, -x, -0.0f, 0);
  } else {
    return __SEGGER_RTL_float32_sincos_fpu(x, x, 0.0f, 0);
  }
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sin_outline()
*
*  Function description
*    Calculate sine, float.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_sin_outline(float x) {
  return __SEGGER_RTL_float32_sin_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sin_inline()
*
*  Function description
*    Calculate sine, double.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sin_inline(double x) {
  int    n;
  double sign;
  double g;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    if (!__SEGGER_RTL_float64_isfinite_inline(x)) {
      return K_NAN_F64;  // Sin[NaN] = Inf, Sin[NaN] = NaN.
    } else {  // can only be subnormal or zero, sin(x) = x for small x
      return x;
    }
  }
  //
  sign = x;
  x    = SEGGER_FABS(x);
  //
  // Range reduction
  //
  n  = SEGGER_D2I(SEGGER_FMA(x, M_INV_PI, 0.5));
  if (n & 1) {
    sign = SEGGER_NEG(sign);
  }
  if (n) {
    x = SEGGER_ADD(SEGGER_SUB(x, SEGGER_MUL(SEGGER_I2D(n), 3.1416015625)),
                   SEGGER_MUL(SEGGER_I2D(n), 8.908910206761537356617e-6));
  }
  //
  // Evaluate polynomial.
  //
  g = SEGGER_MUL(x, x);
  x = SEGGER_FMA(__SEGGER_RTL_float64_PolyEvalP_8(g,
                                                  __SEGGER_RTL_float64_SinCos.Poly.P,
                                                  K_SINCOS_P_DBL),
                 x,
                 x);
  //
  return __SEGGER_RTL_float64_signbit_xor(x, sign);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sin_outline()
*
*  Function description
*    Calculate sine, double.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_sin_outline(double x) {
  return __SEGGER_RTL_float64_sin_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cos_scaled_integer()
*
*  Function description
*    Calculate cosine, float, using scaled-integer arithmetic.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_cos_scaled_integer(float x) {
  __SEGGER_RTL_U32 x0;
  __SEGGER_RTL_U32 i0, i1;  // Working registers when calculating and summing factors
  unsigned         q0, q1;  // Working register scale factors (independent of i registers)
  __SEGGER_RTL_U32 cos_sign;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Extract biased exponent.
  //
  q0 = FLOAT32_EXPONENT(x0);
  //
  // Cos(x) = +1 for |x| < 0x39800000 (2.44140625e-4).
  //
  if (q0 <= 114) {
    return 1.0;
  }
  //
  // Nonzero x.  Save sign, extract significand, compute
  // scale factor such that x0 = i0/2^q0.
  //
  i0 = (x0 & FLOAT32_SIGNIFICAND_MASK) | FLOAT32_HIDDEN_MASK;
  q0 = 150 - q0;
  //
  // Classification of x.
  //
  if (q0 == 150u - 255u) {
    //
    // cos(Inf) = NaN.
    // cos(NaN) = NaN.
    //
    return __SEGGER_RTL_BitcastToF32(x0 | __SEGGER_RTL_U32_C(0x400000));
    //  
  } else if ((int)q0 >= 25) {
    //
    // 0x39800000 (2.44140625e-4) <= |x| <= 0x3f000000 (0.5).
    //
    // Truncated polynomial approximation for cos(x) over [0, 0.5].
    //
    i0 = ((__SEGGER_RTL_I32)i0 << 7) >> (7 + q0 - 32);
    i0 = __SEGGER_RTL_SMULL_HI(i0, i0);
    i1 = __SEGGER_RTL_SMULL_HI(i0, -__SEGGER_RTL_I32_C(0x005A3E8B));
    i1 = __SEGGER_RTL_SMULL_HI(i0, +__SEGGER_RTL_I32_C(0x0AAA8DD5) + i1);
    i1 = __SEGGER_RTL_SMULL_HI(i0, -__SEGGER_RTL_I32_C(0x7FFFFED0) + i1);    // Always -ve
    i1 = __SEGGER_RTL_U32_C(0x40000000) + ((__SEGGER_RTL_I32)(i1+2) >> 2);   // Always two leading zero bits
    //
    return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(0x7D, i1 << 1, 0));
    //
  } else {
    //
    // Reduce i0/2^q0 modulo 2*pi with x0 containing the sign of the
    // input and the leading four bits of the remainder which indicate
    // the breakpoint index to use.
    //
    x0 = (__SEGGER_RTL_float32_SinReduce(&i0, &q0) + 8) & 31;  // Shift by pi/2
    __SEGGER_RTL_float32_SinKernel(x0, i0, q0, &i1, &q1, &cos_sign);
    q0 = __SEGGER_RTL_CLZ_U32(i1);
    //
    // By exhaustion, no zero is ever generated.  If the approximation
    // changes and this happens, the following will cater for that possibility.
    //
    // ...PACK(i0 == 0 ? ~0u : 157 - (q0+q1), ...
    //
    return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(157 - (q0+q1), i1 << (q0-1), cos_sign));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cos_inline()
*
*  Function description
*    Calculate cosine, float.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_cos_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_cos_scaled_integer(x);
#else
  //
  if (!__SEGGER_RTL_float32_isfinite_inline(x)) {
    return K_NAN_F32;
  }
  //
  if (__SEGGER_RTL_float32_lt_rhs_positive(x, 0)) {
    return __SEGGER_RTL_float32_sincos_fpu(SEGGER_NEGF(x), SEGGER_SUBF((float)M_PI_2, x), 1, 1);
  } else {
    return __SEGGER_RTL_float32_sincos_fpu(x, SEGGER_ADDF((float)M_PI_2, x), 1, 1);
  }
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cos_outline()
*
*  Function description
*    Calculate cosine, float.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_cos_outline(float x) {
  return __SEGGER_RTL_float32_cos_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cos_inline()
*
*  Function description
*    Calculate cosine, double.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_cos_inline(double x) {
  double g;
  double xn;
  int    n;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    if (!__SEGGER_RTL_float64_isfinite_inline(x)) {
      return K_NAN_F64;
    } else {  // can only be +zero or subnormal
      return 1;
    }
  }
  //
  x  = SEGGER_FABS(x);
  n  = SEGGER_D2I(SEGGER_FMA(SEGGER_ADD(x, M_PI/2), M_INV_PI, 0.5));
  xn = SEGGER_SUB(SEGGER_I2D(n), 0.5);
  x  = SEGGER_ADD(SEGGER_SUB(x, SEGGER_MUL(xn, 3.1416015625)),
                  SEGGER_MUL(xn, 8.908910206761537356617e-6));
  //
  g = SEGGER_MUL(x, x);
  x = SEGGER_FMA(__SEGGER_RTL_float64_PolyEvalP_8(g,
                                                  __SEGGER_RTL_float64_SinCos.Poly.P,
                                                  K_SINCOS_P_DBL),
                 x,
                 x);
  //
  return (n & 1) ? SEGGER_NEG(x) : x;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cos_outline()
*
*  Function description
*    Calculate cosine, double.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_cos_outline(double x) {
  return __SEGGER_RTL_float64_cos_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tan_scaled_integer()
*
*  Function description
*    Compute tangent, float, scaled integer implementation.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_tan_scaled_integer(float x) {
  __SEGGER_RTL_U32 x0;
  __SEGGER_RTL_U32 i0, i1, i2;      // Working registers when calculating and summing factors
  unsigned         q0, q1;          // Working register scale factors (independent of i registers)
  __SEGGER_RTL_U32 sign;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Extract biased exponent.
  //
  q0 = x0 << 1 >> 24;
  //
  // Tan(-0) = -0.
  // Tan(+0) = +0.
  // Tan(x)  = x for |x| < 0x39800000 (2.44140625e-4).
  //
  if (q0 <= 114) {
    return x;
  }
  //
  // Nonzero x.  Save sign, extract significand, compute
  // scale factor such that x0 = i0/2^q0.
  //
  sign = x0;
  i0   = (x0 & __SEGGER_RTL_U32_C(0xFFFFFF)) | __SEGGER_RTL_U32_C(0x800000);
  q0   = 150 - q0;
  //
  // Classification of x.
  //
  if (__SEGGER_RTL_UNLIKELY((int)q0 == -0x69)) {
    //
    // tan(Inf) = NaN.
    // tan(NaN) = NaN.
    //
    return __SEGGER_RTL_BitcastToF32(x0 | __SEGGER_RTL_U32_C(0x400000));
    //  
  } else if ((int)q0 >= 25) {
    //
    // 0x39800000 (2.44140625e-4) <= |x| <= 0x3f000000 (0.5), fast path.
    //
    // Normalize.
    //
    i0 <<= 7;
    q0 +=  7;
    //
    // Approximation tan(x)/x over [0, 0.5].
    //
    q1 = (q0 - 32) << 1;
    i2 = __SEGGER_RTL_SMULL_HI(i0, i0);                              // i0^2
    i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x0319B19E));
    i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x055ADD78) + ((__SEGGER_RTL_I32)i1 >> q1));
    i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x0DD8E7C1) + ((__SEGGER_RTL_I32)i1 >> q1));
    i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x2221AFA6) + ((__SEGGER_RTL_I32)i1 >> q1));
    i1 = __SEGGER_RTL_SMULL_HI(i2, __SEGGER_RTL_I32_C(0x55555764) + ((__SEGGER_RTL_I32)i1 >> q1));
    i1 = __SEGGER_RTL_SMULL_HI(i1, i0);
    //
    // The following addition could overflow into the high order bit,
    // so conditionally normalize.
    //
    i0 += (__SEGGER_RTL_I32)i1 >> q1;
    q1 = i0 >> 31;
    i0 >>= q1;
    q0  -= q1;
    //
    // Full normalization before packing.
    //
    q1   = __SEGGER_RTL_CLZ_U32(i0);
    i0 <<= q1-1;
    q0  += q1;
    //
    return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(157-q0, i0, sign));
    //
  } else {
    //
    // 0x3f000000 (0.5) <= |x| < Inf, slow path.
    //
    __SEGGER_RTL_U32 sin_i0,   cos_i0,   tan_i0;
    unsigned         sin_q0,   cos_q0,   tan_q0;
    __SEGGER_RTL_U32 sin_sign, cos_sign;
    //
    // Reduce i0/2^q0 modulo 2*pi with x0 containing the sign of the
    // input and the leading four bits of the remainder which indicate
    // the breakpoint index to use.
    //
    x0 = __SEGGER_RTL_float32_SinReduce(&i0, &q0);
    __SEGGER_RTL_float32_SinKernel(x0,          i0, q0, &sin_i0, &sin_q0, &sin_sign);
    __SEGGER_RTL_float32_SinKernel((x0+8) & 31, i0, q0, &cos_i0, &cos_q0, &cos_sign);
    //
    // Normalize divisor so that quotient fits into 32 bits.
    //
    q0       = __SEGGER_RTL_CLZ_U32(cos_i0);
    cos_i0 <<= q0;
    cos_q0  += q0;
    //
    // 32-bit remainder from 64/32 fractional division.
    //
    tan_i0 = __SEGGER_RTL_Div64by32_Moeller(sin_i0, cos_i0) >> 1;
    tan_q0 = sin_q0 - cos_q0 + 32;
    //
    // Normalize.
    //
    q0       = __SEGGER_RTL_CLZ_U32(tan_i0) - 1;
    tan_i0 <<= q0;
    tan_q0  += q0;
    //
    return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(157 - tan_q0, tan_i0, sign ^ sin_sign ^ cos_sign));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tan_fpu()
*
*  Function description
*    Compute tangent, float, FPU or FPE implementation.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_tan_fpu(float x) {
  int   n;
  float f, y, xnum, xden;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(x))) {
    if (!__SEGGER_RTL_float32_isfinite_inline(x)) {
      return K_NAN_F32;
    } else { // must be a zero or subnormal
      return x;  // tan(x) = x for small x.
    }
  }
  //
  y = SEGGER_FABSF(x);
  n = SEGGER_F2I(SEGGER_DIVF(SEGGER_ADDF(SEGGER_MUL2F(y), (float)M_PI_2), (float)M_PI));
  if (__SEGGER_RTL_float32_lt_rhs_positive(x, 0)) {
    n = -n;
  }
  //
  xnum = SEGGER_I2F(n);
  f = SEGGER_FMAF(xnum, 4.454455103380768678308e-6f, SEGGER_FMAF(xnum, -1.57080078125f, x));
  //
  if (__SEGGER_RTL_float32_gt_rhs_positive(SEGGER_FABSF(f), 1.e-10f)) {
    //
    // Larger tangent value.
    //
    y = SEGGER_MULF(f, f);
    xnum = SEGGER_FMAF(__SEGGER_RTL_float32_PolyEvalP_3(y, __SEGGER_RTL_float32_Tan.Poly.P, K_TAN_P_FLT), f, f);
    xden = SEGGER_ADDF(__SEGGER_RTL_float32_PolyEvalP_4(y, __SEGGER_RTL_float32_Tan.Poly.Q, K_TAN_Q_FLT), 1);
  } else {
    //
    // Small tangent value.
    //
    xnum = f;
    xden = 1;
  }
  //
  if (n & 1) {
    return SEGGER_NEGF(SEGGER_DIVF(xden, xnum));
  } else {
    return SEGGER_DIVF(xnum, xden);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tan_inline()
*
*  Function description
*    Compute tangent, float.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_tan_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_tan_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_tan_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tan_outline()
*
*  Function description
*    Compute tangent, float.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_tan_outline(float x) {
  return __SEGGER_RTL_float32_tan_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_tan_inline()
*
*  Function description
*    Compute tangent, double.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_tan_inline(double x) {
  int    n;
  double f, y, xnum, xden;
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x))) {
    if (!__SEGGER_RTL_float64_isfinite_inline(x)) {
      return K_NAN_F64;
    } else {  // 0 or subnormal.
      return x;
    }
  }
  //
  y = SEGGER_FABS(x);
  n = SEGGER_D2I(SEGGER_DIV(SEGGER_ADD(SEGGER_MUL2(y), M_PI_2), M_PI));
  xnum = __SEGGER_RTL_float64_signbit_xor(SEGGER_I2D(n), x);
  f = SEGGER_FMA(xnum, 4.454455103380768678308e-6, SEGGER_FMA(xnum, -1.57080078125, x));
  //
  if (__SEGGER_RTL_float64_gt_rhs_positive(SEGGER_FABS(f), 1.e-10)) {
    y = SEGGER_MUL(f, f);
    xnum = SEGGER_FMA(__SEGGER_RTL_float64_PolyEvalP_3(y, __SEGGER_RTL_float64_Tan.Poly.P, K_TAN_P_DBL), f, f);
    xden = SEGGER_ADD(__SEGGER_RTL_float64_PolyEvalP_4(y, __SEGGER_RTL_float64_Tan.Poly.Q, K_TAN_Q_DBL), 1);
  } else {
    //
    // Small tangent value.
    //
    xnum = f;
    xden = 1;
  }
  //
  if (n & 1) {
    return SEGGER_NEG(SEGGER_DIV(xden, xnum));
  } else {
    return SEGGER_DIV(xnum, xden);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_tan_outline()
*
*  Function description
*    Compute tangent, double.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_tan_outline(double x) {
  return __SEGGER_RTL_float64_tan_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_asinacos_fpu()
*
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_asinacos_fpu(float x, int flag) {
  int   i;
  float y;
  float g;
  //
  y = SEGGER_FABSF(x);
  //
  // Moving this test below taking the absolute value helps code
  // generation as the sign bit is known to be zero now.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(y))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float32_lt_rhs_positive(y, 1.e-10f)) {
    i = flag;
  } else {
    if (__SEGGER_RTL_float32_gt_rhs_positive(y, 0.5f)) {
      if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_gt_rhs_positive(y, 1))) {
        return K_NAN_F32;
      }
      i = 1 - flag;
      g = SEGGER_DIV2F(SEGGER_ADDF(SEGGER_SUBF(0.5f, y), 0.5f));
      y = SEGGER_MULM2F(SEGGER_SQRTF(g));
    } else {
      i = flag;
      g = SEGGER_MULF(y, y);
    }
    y = SEGGER_FMAF(y,
                    SEGGER_DIVF(__SEGGER_RTL_float32_PolyEvalP_5(g, __SEGGER_RTL_float32_ASinACos.Poly.P, K_ASINACOS_P_FLT),
                                __SEGGER_RTL_float32_PolyEvalQ_5(g, __SEGGER_RTL_float32_ASinACos.Poly.Q, K_ASINACOS_Q_FLT)),
                    y);
  }
  //
  if (!flag) {
    y = SEGGER_ADDF(SEGGER_ADDF(__SEGGER_RTL_float32_ASinACos.Quadrant.A[i], y), __SEGGER_RTL_float32_ASinACos.Quadrant.A[i]);
    y = __SEGGER_RTL_float32_signbit_xor(y, x);
  } else if (__SEGGER_RTL_float32_lt0_true(x)) {
    y = SEGGER_ADDF(SEGGER_ADDF(__SEGGER_RTL_float32_ASinACos.Quadrant.B[i], y), __SEGGER_RTL_float32_ASinACos.Quadrant.B[i]);
  } else {
    y = SEGGER_ADDF(SEGGER_SUBF(__SEGGER_RTL_float32_ASinACos.Quadrant.A[i], y), __SEGGER_RTL_float32_ASinACos.Quadrant.A[i]);
  }
  //
  return y;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_asin_inline()
*
*  Function description
*    Compute inverse sine, float.
*
*  Parameters
*    x - Value to compute inverse sine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular sine of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular sine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_asin_inline(float x) {
  return __SEGGER_RTL_float32_asinacos_fpu(x, 0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_acos_scaled_integer()
*
*  Function description
*    Compute inverse cosine, float.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_acos_scaled_integer(float x) {
  __SEGGER_RTL_U32 x0, abs_x0, p;
  __SEGGER_RTL_U32 i0, i1, i2;
  unsigned         q0, q1, q2;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Compute |x0|, but shifted.  
  //
  abs_x0 = x0 << 1;
  //
  // ArcCos[small] = Pi/2.
  //
  if (abs_x0 < __SEGGER_RTL_X2(__SEGGER_RTL_U32_C(0x32900000))) {
    return __SEGGER_RTL_BitcastToF32(K_PiOver2_U32);
  }
  //
  // Handle exceptional cases.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_X2(K_one_U32) <= abs_x0)) {
    if (abs_x0 == __SEGGER_RTL_X2(K_one_U32)) {
      //
      // ArcCos[1] is 0, ArcCos[-1] is Pi by definition.
      //
      return (x0 & FLOAT32_SIGN_MASK) ? __SEGGER_RTL_BitcastToF32(K_Pi_U32) : 0;
      //
    } else if (abs_x0 > __SEGGER_RTL_X2(__SEGGER_RTL_U32_C(0x7F800000))) {
      //
      // ArcCos[NaN] is qNaN..
      //
      return __SEGGER_RTL_BitcastToF32(x0 | __SEGGER_RTL_U32_C(0x400000));
      //
    } else {
      //
      // ArcCos[x], |x| > 1 is sNaN..
      //
      return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_U32_C(0xFFC00000));
    }
  }
  //
  // Scaled integer, i0.  Argument = i0 / 2^q0.  q0 only calculated on one branch.
  //
  i0 = ((x0 & __SEGGER_RTL_U32_C(0xFFFFFF)) | __SEGGER_RTL_U32_C(0x800000)) << 7;
  //
  // Divide into approximation classes.
  //
  if (abs_x0 <= __SEGGER_RTL_X2(__SEGGER_RTL_U32_C(0x3F300000))) {
    //
    // |x| <= 0.6875
    //
    // Approximate ArcCos[x] using a Taylor series.  Unfortunately, the Taylor series
    // converges terribly slowly as x tends to 1 and requires a large number of terms.
    // We do our best to truncate this series in a couple of places.  If you can
    // suffer more than 0.5ulp then there's no need to wring out correctly rounding
    // the least significant bit.
    //
    // We use terms up to and including x^41 in the worst case--how painful is that?
    //
    // Using Mathematica:
    //
    //    Series[ArcCos[x], {x, 0, 41}]
    //
    // is:
    //
    //    Pi/2 - x - x^3/6 - (3x^5)/40 - (5x^7)/112 - (35x^9)/1152 - (63x^11)/2816...
    //         ... - (34461632205 x^41)/11269994184704 + O[x]^42
    //
    // i.e.
    //
    //    Pi/2 - x - 0.1667x^3 - 0.075x^5 - 0.04464x^7 - 0.03038x^9 - 0.02237x^11...
    //         ... - -0.003058x^41 + O[x]^42
    //
    // We need to stretch to x^41 as 0.6875^41 * 0.003058 is 6.5e-10 and 0x00000001 in
    // Q31 format is 2^-31 which is 4.65e-10.  All this to round correctly...
    //
    // The above Taylor expansion is in odd powers of x, so when recasting in Horner
    // form, we make the substitution y=x^2 and rewrite in terms of y.  I hold x^2
    // in i2:q2.  Also, rather than subtracting at each step, we sum the terms and
    // subtract at the end, Pi/2 - (x + x^3/6 + (3x^5)/40...)
    //
    // Scaled integer factor, q0.
    //
    q0 = 0x9D - 30 - (abs_x0 >> 24);
    //
    // Form x^2 here to Q30 in i2:q2.
    //
    i2 = __SEGGER_RTL_U64_L((__SEGGER_RTL_UMULL_X(i0, i0) + (__SEGGER_RTL_U32_C(1)<<30)) >> 31);
    q2 = (q0 << 1) - 3;
    __SEGGER_RTL_NORMALIZE_1UP(i2, q2);
    //
    // Truncate high-order polynomial as much as we can.
    //
    if (q2 >= 32) {
      //
      // x < 1.676381e-8 (0x32900000)
      //
      // x is sufficiently small so that ArcSin[x] ~= x,
      // hence ArcCos[x] ~= Pi/2 - x.  In this case there is
      // nothing to do so we keep i0/q0 as x.
      //
    } else {
      //
      if (q0 <= 2) {  // 0.25 (0x3e800000) <= |x|...
        if (q0 < 2) {
          //
          // Wring out the last bit of precision for rounding...
          //
          p = __SEGGER_RTL_U32_C(0x349CBDB2);
          p = __SEGGER_RTL_U32_C(0x36A6B7FD) +  __SEGGER_RTL_UMULL_HI(i2, p);
          p = __SEGGER_RTL_U32_C(0x3A8EF970) +  __SEGGER_RTL_UMULL_HI(i2, p);
          p = __SEGGER_RTL_U32_C(0x3F9A4F33) +  __SEGGER_RTL_UMULL_HI(i2, p);         // scale down, next coeff would be 0x4xxxxxxxx, too big...
          p = __SEGGER_RTL_U32_C(0x22BDCCC8) + (__SEGGER_RTL_UMULL_HI(i2, p) >> 1);
          p = __SEGGER_RTL_U32_C(0x262D020D) +  __SEGGER_RTL_UMULL_HI(i2, p);
          p = __SEGGER_RTL_U32_C(0x2A374442) +  __SEGGER_RTL_UMULL_HI(i2, p);
          p = __SEGGER_RTL_U32_C(0x2F05B99A) +  __SEGGER_RTL_UMULL_HI(i2, p);
          p = __SEGGER_RTL_U32_C(0x34D0C7AC) +  __SEGGER_RTL_UMULL_HI(i2, p);
          p = __SEGGER_RTL_U32_C(0x3BE77A7E) +  __SEGGER_RTL_UMULL_HI(i2, p);
          p = __SEGGER_RTL_U32_C(0x225DE79E) + (__SEGGER_RTL_UMULL_HI(i2, p) >> 1);   // scale down, as above
        } else {
          // q0 == 2
          p = __SEGGER_RTL_U32_C(0x225DE79E);
        }
        //
        p = __SEGGER_RTL_UMULL_HI(i2, p);
        p = __SEGGER_RTL_UMULL_HI(i2, __SEGGER_RTL_U32_C(0x27FBCA1A) + (p >> q2));
        p = __SEGGER_RTL_UMULL_HI(i2, __SEGGER_RTL_U32_C(0x2F50F0F0) + (p >> q2));
        p = __SEGGER_RTL_UMULL_HI(i2, __SEGGER_RTL_U32_C(0x39333333) + (p >> q2));
        p = __SEGGER_RTL_UMULL_HI(i2, __SEGGER_RTL_U32_C(0x2389D89D) + (p >> q2 >> 1));  // scale down
        p =                           __SEGGER_RTL_U32_C(0x2DD1745D) + (p >> q2);
        //
      } else {
        p = __SEGGER_RTL_U32_C(0x2DD1745D);
      }
      //
      p = __SEGGER_RTL_UMULL_HI(i2, p);
      p = __SEGGER_RTL_UMULL_HI(i2, __SEGGER_RTL_U32_C(0x3E38E38E) + (p >> q2));
      p = __SEGGER_RTL_UMULL_HI(i2, __SEGGER_RTL_U32_C(0x2DB6DB6D) + (p >> q2 >> 1));      // each coefficient is > 0x40...
      p = __SEGGER_RTL_UMULL_HI(i2, __SEGGER_RTL_U32_C(0x26666666) + (p >> q2 >> 1));
      p = __SEGGER_RTL_UMULL_HI(i2, __SEGGER_RTL_U32_C(0x2AAAAAAA) + (p >> q2 >> 1));      // p is x^2/6 + 3x^4/40 + ...
      //
      // Polynomial above is the parenthesized part with a leading x^2 term.
      // Form whole of the parenthesized part by multiplying polynomial
      // through by x (held in i0) and adding x (again, in i0).
      //
      i0 += __SEGGER_RTL_UMULL_HI(i0, p) >> q2;                         // x + x[x^2/6 + 3x^4/40 + ...] = x + x^3/6 + 3x^5/40 + ...
      //
      // Ensure normalized.
      //
      __SEGGER_RTL_NORMALIZE_1DN(i0, q0);
    }
    //
    // Check sign of argument.
    //
    if ((__SEGGER_RTL_I32)x0 >= 0) {
      //
      // ArcCos[x] = Pi/2 - (x + x^3/6 + 3x^5/40 + ...), x > 0
      //
      i0 = K_Pi_2Q29 - (i0 >> q0);
      //
#if defined(__SEGGER_RTL_CLZ_U32) && !defined(__SEGGER_RTL_CLZ_U32_SYNTHESIZED)
      q1   = __SEGGER_RTL_CLZ_U32(i0) - 1;
      i0 <<= q1;
      q0   = 30 + q1;
#else
      //
      // Testing by exhaustion reveals there are only ever one or two leading zeros
      // in the 32-bit result, so we can pun and use regular add normalization.
      //
      q0 = 30;
      __SEGGER_RTL_NORMALIZE_1UP(i0, q0);
#endif
      //
    } else {
      //
      // Use the identity ArcCos[x] = Pi - ArcCos[|x|], x < 0.
      //
      // Substituting the Taylor expansion above we have:
      //   ArcCos[x]  =  Pi - (Pi/2 - (x + x^3/6 + 3x^5/40 + ...))
      //              =  Pi - Pi/2 + ( x + x^3/6 + 3x^5/40 + ...)
      //              =  Pi/2 + x + x^3/6 + 3x^5/40 + ...
      //
      // But we have already calculated x + x^3/6... in i0/2^q0, so all we need to do
      // now is add this to Pi/2 and normalize.
      //
      i0 = K_Pi_2Q29 + (i0 >> q0);
      q0 = 30;
      __SEGGER_RTL_NORMALIZE_1DN(i0, q0);
    }
    //
    // Pack, round, and return.
    //
    return __SEGGER_RTL_BitcastToF32(((__SEGGER_RTL_U32)(0x9C - q0) << 23) + ((i0 + (1<<6)) >> 7));
    //
  } else if (abs_x0 < __SEGGER_RTL_X2(K_Sqrt_0v75_U32)) {
    //
    // 0.6875 < |x| < Sqrt[1-0.5^2]
    //
    // We process the intervals [0x3f300000, 0x3f3fffff], [0x3f400000, 0x3f4fffff],
    // and [0x3f500000, 0x3f5db3d8] using a Taylor series expanded about the midpoint
    // of the interval.  These intervals are [0.6875, 0.75), [0.75, 0.8125), and
    // [0.8125, 0.866025].  See below for details.
    //
    // We use the identity ArcCos[x] = Pi/2 - ArcSin[x] and compute ArcSin[x].
    //
    // Extract leading bits that determine appropriate interval.
    //
    q0 = i0 >> 27;
    //
    // Shift off leading bits to place i0 at the lower bound of the interval,
    // then shift to to midpoint of interval.
    //
    i0 = (i0 << 5) + __SEGGER_RTL_U32_C(0x80000000);
    //
    // Switch on required interval.  q0 can be only 11, 12, or 13.
    //
    if (q0 == 11) {
      //
      // [11/16, 12/16) is [0.6875, 0.75)
      //
      // Expand Taylor series for ArcSin[x] around midpoint of interval, i.e. (11/16 + 12/16)/2
      // which is 23/32 or 0.71875.
      //
      // Mathematica generates the coefficients:
      //
      //    BaseForm[N[Normal[Series[ArcSin[x], {x, 0.71875, 8}]] /. -0.71875 + x -> y, 10], 16]
      //
      // The coefficients are normalized and the intermediate value shifted
      // when put into Horner form.
      //
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x3E5EBF37));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2B23C258) + ((__SEGGER_RTL_I32)p >> 3));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x3DAEF9E3) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2E2F2B1C) + ((__SEGGER_RTL_I32)p >> 3));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x24E94C6D) + ((__SEGGER_RTL_I32)p >> 3));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x215F5057) + ((__SEGGER_RTL_I32)p >> 3));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2237833A) + ((__SEGGER_RTL_I32)p >> 3));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2E067F20) + ((__SEGGER_RTL_I32)p >> 4));
      i0 =                          __SEGGER_RTL_U32_C(0x66A806EB) + ((__SEGGER_RTL_I32)p >> 2);
      q0 = 1;
      //
    } else if (q0 == 12) {
      //
      // [12/16, 13/16) is [0.75, 0.8125)
      //
      // Expand Taylor series for ArcSin[x] around midpoint of interval, 25/32, using Mathematica.
      //
      //    BaseForm[N[Normal[Series[ArcSin[x], {x, 0.78125, 8}]] /. -0.78125 + x -> y, 10], 16]
      //
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x33669D21));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x37507EB9) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x3D891027) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x23D75B64) + ((__SEGGER_RTL_I32)p >> 3));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2CA58F69) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x3E7C3377) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x3364818C) + ((__SEGGER_RTL_I32)p >> 3));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x33439D62) + ((__SEGGER_RTL_I32)p >> 4));
      i0 =                          __SEGGER_RTL_U32_C(0x72C5F212) + ((__SEGGER_RTL_I32)p >> 2);
      q0 = 1;
      //
    } else { // q0 == 13
      //
      // [13/16, Sqrt[3/4]) is [0.8125, ~0.866025)
      //
      // Expand Taylor series for ArcSin[x] around midpoint of "whole" interval rather than
      // the truncated interval with a reduced upper bound (where we use a different approximation).
      // The "whole" interval is [13/16, 14/16) and the midpoint is 27/32 which is 0.84375.
      //
      //    BaseForm[N[Normal[Series[ArcSin[x], {x, 0.84375, 8}]] /. -0.84375 + x -> y, 10], 16]
      //
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x281DA51D));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x3DAF1D88) + ((__SEGGER_RTL_I32)p >> 1));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x31079BDA) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x28CF9D7A) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x245B89A3) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x24465B68) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2BA75668) + ((__SEGGER_RTL_I32)p >> 2));
      p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x3B9E9D1F) + ((__SEGGER_RTL_I32)p >> 3));
      i0 =                          __SEGGER_RTL_U32_C(0x40455647) + ((__SEGGER_RTL_I32)p >> 3);
      q0 = 0;
    }
    //
    // Ensure correctly-normalized result.
    //
    __SEGGER_RTL_NORMALIZE_1UP(i0, q0);
    //
    // ArcCos[x] = Pi/2 - ArcSin[|x|],  x >= 0
    // ArcCos[x] = Pi/2 + ArcSin[|x|],  x <= 0
    //
    if ((__SEGGER_RTL_I32)x0 >= 0) {
      i0 = (K_Pi_2Q29 - (i0 >> q0)) << 1;
      q0 = 0x9C - 30 - 1;
    } else {
      i0 = (K_Pi_2Q29 + (i0 >> q0)) >> 1;
      q0 = 0x9C - 30 + 1;
    }
    //
    return __SEGGER_RTL_BitcastToF32(((__SEGGER_RTL_U32)q0 << 23) + ((i0 + (1<<6)) >> 7));
    //
  } else {
    //
    // Sqrt[1-0.5^2] <= x <= 1.
    //
    // Using a standard Taylor series is going to be too slow to converge, so
    // use a trigonometric identity to reduce the range of the argument to
    // something that converges relatively quickly in a Taylor series.
    //
    // We have Sqrt[3/4] <= x <= 1.
    // 
    // The following are standard identities for ArcCos:
    //
    // ArcCos[x] = Pi/2 + ArcCos[Sqrt[1-x^2]], x <= 0
    // ArcCos[x] = Pi/2 - ArcCos[Sqrt[1-x^2]], x >= 0
    //
    // Hence, setting y = Sqrt[1-x^2] we have 0 <= y <= 0.5.
    //
    // We can compute ArcCos[y] using a short Taylor polynomial and
    // substituting back into the above gives us ArcCos[x] by a
    // simple addition.
    //
    // Compute 1-x^2 by factoring to (1-x)(1+x) which is more accurate.
    //
    i0 >>= 1;
    //
    // Calculate i1 = 1-i0.
    //
    i1 = __SEGGER_RTL_U32_C(0x40000000) - i0;
    q1 = __SEGGER_RTL_CLZ_U32(i1);
    i1 <<= q1-1;
    //
    // i0 = 1 + x
    //
    i0 += __SEGGER_RTL_U32_C(0x40000000);
    //
    // (1-x)(1+x) == 1-x^2, and round.
    //
    i1 = __SEGGER_RTL_U64_L((__SEGGER_RTL_UMULL_X(i0, i1) + (__SEGGER_RTL_U32_C(1)<<30)) >> 31);
    __SEGGER_RTL_NORMALIZE_1UP(i1, q1);
    //
    // We compute Sqrt[x] by using a reciprocal approximation, 1/Sqrt[x], and
    // multiply through by x, i.e. 1/Sqrt[x] * x = x/Sqrt[x] = Sqrt[x].  We
    // only need an approximation to Sqrt[x] anyway, we don't need it correctly
    // rounded--we're plugging it into a Taylor series--but we do need it
    // accurate to 30 bits.
    //
    // This code is based on Apple Technical Report No. 95: Computing the
    // Inverse Square Root, but modified for fixed-point scaled integers.
    // http://www.worldserver.com/turk/computergraphics/InverseSqrt.pdf
    //
    // Compute 1/Sqrt[1-x^2].  We do this by an initial approximation good for
    // eight bits and then refining three times using Newton iterations to
    // extend to 32 bits.
    //
    // We wish to find the zero of f(x) = 1/(x^2)-S.  Differentiating and
    // substituting into the Newton method we have the well-known formula:
    //
    //   y_n+1 = y_n/2 * (3 - x * y_n^2).
    //
    // This is nice because there are no divisions to perform, unlike Sqrt[x]
    // under Newton, and the operations are very simple in integer-based
    // fixed-point arithmetic
    //
    // NOTE: Potentially an extra register but condition free and fewer instructions
    // on ARM/Thumb/Thumb-2...
    //
    // Initial square root approximation from leading eight bits of input (1+x)(1-x).
    // As this is a fraction, we add a one in front of the binary point.
    //
    q0 = q1 & 1;
    i2 = __SEGGER_RTL_rsqrt_approx_bits[(i1 >> 23) - 0x80 + (q0 << 7)] + 0x100;  // ENHANCEMENT: could permute lookup table?
    i0 = i1 << q0;
    //
    // i1 is normalized hence i1>>30 is 1.
    //
    q0 = (30 - 28) - q1;
    q0 = 30 - ((int)q0 >> 1);
    q1 = (q0 << 1) - 31 - 32;
    //
    // Use Newton-Raphson iterations on initial approximation as described above.
    //
    p = i2 * (i0 >> 8);
    i0 -= 1;
    i1  = i0 - __SEGGER_RTL_UMULL_HI(p, p);
    i1  = p + __SEGGER_RTL_U64_L(__SEGGER_RTL_UMULL_X(i2, i1) >> 9);
    i1 += (i2 * (i0 - __SEGGER_RTL_UMULL_HI(i1, i1))) >> 9;
    i1 += (i2 * (i0 - __SEGGER_RTL_UMULL_HI(i1, i1))) >> 9;
    i1 += (i2 * (i0 - __SEGGER_RTL_UMULL_HI(i1, i1))) >> 9;
    i1  = (i1 >> 1) + 1;
    //
    // Now proceed to Taylor series.  Square argument.
    //
    i0 = __SEGGER_RTL_U64_L((__SEGGER_RTL_SMULL_X(i1, i1) + (__SEGGER_RTL_U32_C(1)<<30)) >> 31);
    //
    // Standard Taylor expansion as above.
    //
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x225DE79E));
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x27FBCA1A) + (p >> q1));
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2F50F0F0) + (p >> q1));
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x39333333) + (p >> q1)) >> 1;
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2389D89D) + (p >> q1));
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2DD1745D) + (p >> q1));
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x3E38E38E) + (p >> q1)) >> 1;
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2DB6DB6D) + (p >> q1)) >> 1;
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x26666666) + (p >> q1)) >> 1;
    p = __SEGGER_RTL_SMULL_HI(i0, __SEGGER_RTL_U32_C(0x2AAAAAAA) + (p >> q1));
    p = __SEGGER_RTL_SMULL_HI(i1, p);
    //
    // Normalize.
    //
    i0 = i1 + ((__SEGGER_RTL_I32)p >> q1);
    __SEGGER_RTL_NORMALIZE_1DN(i0, q0);
    //
    // ArcCos[-x] = Pi - ArcCos[x].
    //
    if ((__SEGGER_RTL_I32)x0 <= 0) {
      i0 = K_Pi_2Q29 - (i0 >> (q0-29));
      q0 = 29;
    }
    //
    return __SEGGER_RTL_BitcastToF32(((__SEGGER_RTL_U32)(0x9C - q0) << 23) + ((i0 + (1<<6)) >> 7));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_acos_fpu()
*
*  Function description
*    Compute inverse cosine, float.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_acos_fpu(float x) {
  return __SEGGER_RTL_float32_asinacos_fpu(x, 1);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_acos_inline()
*
*  Function description
*    Compute inverse cosine, float.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_acos_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_acos_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_acos_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_asinacos_fpu()
*
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_asinacos_fpu(double x, int cos_case) {
  int    i;
  double y;
  double g;
  //
  y = SEGGER_FABS(x);
  //
  // Moving this test below taking the absolute value helps code
  // generation as the sign bit is known to be zero now.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(y))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float64_lt_rhs_positive(y, 1.e-10)) {
    i = cos_case;
  } else {
    if (__SEGGER_RTL_float64_gt_rhs_positive(y, 0.5)) {
      if (__SEGGER_RTL_float64_gt_rhs_positive(y, 1)) {
        return K_NAN_F64;
      }
      //
      i = 1 - cos_case;
      g = SEGGER_DIV2(SEGGER_ADD(SEGGER_SUB(0.5, y), 0.5));
      y = SEGGER_MULM2(SEGGER_SQRT(g));
    } else {
      i = cos_case;
      g = SEGGER_MUL(y, y);
    }
    y = SEGGER_FMA(y,
                   SEGGER_DIV(__SEGGER_RTL_float64_PolyEvalP_5(g, __SEGGER_RTL_float64_ASinACos.Poly.P, K_ASINACOS_P_DBL),
                              __SEGGER_RTL_float64_PolyEvalQ_5(g, __SEGGER_RTL_float64_ASinACos.Poly.Q, K_ASINACOS_Q_DBL)),
                   y);
  }
  if (!cos_case) {
    y = SEGGER_ADD(SEGGER_ADD(__SEGGER_RTL_float64_ASinACos.Quadrant.A[i], y), __SEGGER_RTL_float64_ASinACos.Quadrant.A[i]);
    y = __SEGGER_RTL_float64_signbit_xor(y, x);
  } else if (__SEGGER_RTL_float64_lt_rhs_positive(x, 0)) {
    y = SEGGER_ADD(SEGGER_ADD(__SEGGER_RTL_float64_ASinACos.Quadrant.B[i], y), __SEGGER_RTL_float64_ASinACos.Quadrant.B[i]);
  } else {
    y = SEGGER_ADD(SEGGER_SUB(__SEGGER_RTL_float64_ASinACos.Quadrant.A[i], y), __SEGGER_RTL_float64_ASinACos.Quadrant.A[i]);
  }
  return y;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_asin_inline()
*
*  Function description
*    Compute inverse sine, double.
*
*  Parameters
*    x - Value to compute inverse sine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular sine of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular sine of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_asin_inline(double x) {
  return __SEGGER_RTL_float64_asinacos_fpu(x, 0);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_acos_inline()
*
*  Function description
*    Compute inverse cosine, double.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_acos_inline(double x) {
  return __SEGGER_RTL_float64_asinacos_fpu(x, 1);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_atan_inline()
*
*  Function description
*    Compute inverse tangent, float.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_atan_inline(float x) {
  int   n;
  float f;
  float g;
  //
  f = SEGGER_FABSF(x);
  //
  // Moving this test below taking the absolute value helps code
  // generation as the sign bit is known to be zero now.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(f))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float32_gt_rhs_positive(f, 1)) {
    f = SEGGER_DIVF(1, f);
    n = 2;
  } else {
    n = 0;
  }
  //
  if (__SEGGER_RTL_float32_gt_rhs_positive(f, 0.26794919243112270647f)) {
    f = SEGGER_DIVF(
          SEGGER_ADDF(SEGGER_ADDF(SEGGER_ADDF(SEGGER_MULF(0.73205080756887729353f, f), -0.5f), -0.5f), f),
          SEGGER_ADDF(1.73205080756887729353f, f));
    ++n;
  }
  //
  if (__SEGGER_RTL_float32_gt_rhs_positive(SEGGER_FABSF(f), 1.e-10f)) {
    g = SEGGER_MULF(f, f);
    f = SEGGER_ADDF(f, SEGGER_MULF(f,  SEGGER_DIVF(__SEGGER_RTL_float32_PolyEvalP_4(g, __SEGGER_RTL_float32_ATan.Poly.P, K_ATAN_P_FLT),
                                                   __SEGGER_RTL_float32_PolyEvalQ_4(g, __SEGGER_RTL_float32_ATan.Poly.Q, K_ATAN_Q_FLT))));
  }
  if (n > 1) {
    f = SEGGER_NEGF(f);
  }
  if (n) {
    f = SEGGER_ADDF(f, __SEGGER_RTL_float32_ATan.Quadrant.A[n]);
  }
  return __SEGGER_RTL_float32_signbit_xor(f, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_atan_outline()
*
*  Function description
*    Compute inverse tangent, float.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*/
static float __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float32_atan_outline(float x) {
  return __SEGGER_RTL_float32_atan_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_atan_inline()
*
*  Function description
*    Compute inverse tangent, double.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*
*  Notes
*    [C&W] Chapter 11, "ATAN/ATAN2".
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_atan_inline(double x) {
  int    n;
  double f;
  double g;
  //
  f = SEGGER_FABS(x);
  //
  // Moving this test below taking the absolute value helps code
  // generation as the sign bit is known to be zero now.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(f))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float64_gt_rhs_positive(f, 1)) {
    f = SEGGER_DIV(1, f);
    n = 2;
  } else {
    n = 0;
  }
  //
  if (__SEGGER_RTL_float64_gt_rhs_positive(f, 2-M_SQRT_3)) {
    f = SEGGER_DIV(SEGGER_ADD(SEGGER_ADD(SEGGER_ADD(SEGGER_MUL(M_SQRT_3-1, f), -0.5), -0.5), f),
                   SEGGER_ADD(M_SQRT_3, f));
    ++n;
  }
  //
  if (__SEGGER_RTL_float64_gt_rhs_positive(SEGGER_FABS(f), 1.e-10)) {
    g = SEGGER_MUL(f, f);
    f = SEGGER_ADD(f,
                   SEGGER_MUL(f,
                              SEGGER_DIV(__SEGGER_RTL_float64_PolyEvalP_4(g, __SEGGER_RTL_float64_ATan.Poly.P, K_ATAN_P_DBL),
                                         __SEGGER_RTL_float64_PolyEvalQ_4(g, __SEGGER_RTL_float64_ATan.Poly.Q, K_ATAN_Q_DBL))));
  }
  if (n > 1) {
    f = SEGGER_NEG(f);
  }
  if (n) {
    f = SEGGER_ADD(f, __SEGGER_RTL_float64_ATan.Quadrant.A[n]);
  }
  return __SEGGER_RTL_float64_signbit_xor(f, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_atan_outline()
*
*  Function description
*    Compute inverse tangent, double.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_atan_outline(double x) {
  return __SEGGER_RTL_float64_atan_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_atan2_inline()
*
*  Function description
*    Compute inverse tangent, with quadrant, float.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_atan2_inline(float y, float x) {
  float mx;
  float my;
  //
  mx = SEGGER_FABSF(x);
  my = SEGGER_FABSF(y);
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(mx) || __SEGGER_RTL_float32_isspecial(my))) {
    //
    if (__SEGGER_RTL_float32_isnan_inline(x)) {
      return x;
    } else if  (__SEGGER_RTL_float32_isnan_inline(my)) {
      return y;
    } else if (__SEGGER_RTL_float32_isinf_inline(my)) {
      if (__SEGGER_RTL_float32_isinf_inline(x)) {
        return __SEGGER_RTL_float32_signbit_xor(__SEGGER_RTL_float32_lt0_nonzero_finite(x) ? (float)M_3_PI_4 : (float)M_PI_4, y);
      } else {
        return __SEGGER_RTL_float32_signbit_xor((float)M_PI_2, y);
      }
    } else if (__SEGGER_RTL_float32_putative_iszero(y) || __SEGGER_RTL_float32_isinf_inline(x)) {
      return __SEGGER_RTL_float32_signbit_xor(__SEGGER_RTL_float32_signbit_inline(x) ? (float)M_PI : 0, y);
    } else if (__SEGGER_RTL_float32_putative_iszero(x)) {
      return __SEGGER_RTL_float32_signbit_xor((float)M_PI_2, y);
    }
  }
  //
  if (SEGGER_LTF(mx, my) && SEGGER_LTF(SEGGER_FABSF(SEGGER_DIVF(x, y)), 1.0e-20f)) {
    return __SEGGER_RTL_float32_signbit_xor((float)M_PI_2, y);
  }
  //
  y = SEGGER_ATANF(SEGGER_DIVF(y, x));
  if (__SEGGER_RTL_float32_lt_rhs_positive(x, 0)) {
    y = SEGGER_ADDF(y, __SEGGER_RTL_float32_lt_rhs_positive(y, 0) ? +(float)M_PI : -(float)M_PI);
  }
  //
  return y;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_atan2_outline()
*
*  Function description
*    Compute inverse tangent, with quadrant, float.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_atan2_outline(float y, float x) {
  return __SEGGER_RTL_float32_atan2_inline(y, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_atan2_inline()
*
*  Function description
*    Compute inverse tangent, with quadrant, double.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_atan2_inline(double y, double x) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x) || __SEGGER_RTL_float64_isspecial(y))) {
    //
    if (__SEGGER_RTL_float64_isnan_inline(x)) {
      return x;
    } else if  (__SEGGER_RTL_float64_isnan_inline(y)) {
      return y;
    } else if (__SEGGER_RTL_float64_isinf_inline(y)) {
      if (__SEGGER_RTL_float64_isinf_inline(x)) {
        return __SEGGER_RTL_float64_signbit_xor(SEGGER_LT0(x) ? M_3_PI_4 : M_PI_4, y);
      } else {
        return __SEGGER_RTL_float64_signbit_xor(M_PI_2, y);
      }
    } else if (SEGGER_EQ0(y) || __SEGGER_RTL_float64_isinf_inline(x)) {
      return __SEGGER_RTL_float64_signbit_xor(__SEGGER_RTL_float64_signbit_inline(x) ? M_PI : 0, y);
    } else if (SEGGER_EQ0(x)) {
      return __SEGGER_RTL_float64_signbit_xor(M_PI_2, y);
    }
  }
  //
  if (SEGGER_LT(SEGGER_FABS(x), SEGGER_FABS(y)) && SEGGER_LT(SEGGER_FABS(SEGGER_DIV(x, y)), 1.0e-20)) {
    return __SEGGER_RTL_float64_signbit_xor(M_PI_2, y);
  }
  //
  y = SEGGER_ATAN(SEGGER_DIV(y, x));
  if (SEGGER_LT0(x)) {
    y = SEGGER_ADD(y, SEGGER_GT0(y) ? -M_PI : +M_PI);
  }
  return y;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_atan2_outline()
*
*  Function description
*    Compute inverse tangent, with quadrant, double.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_atan2_outline(double y, double x) {
  return __SEGGER_RTL_float64_atan2_inline(y, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sinh_fpu()
*
*  Function description
*    Compute hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*
*  Notes
*    [C&W] Chapter 12, SINH/COSH.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_sinh_fpu(float x) {
  float z;
  //
  z = SEGGER_FABSF(x);
  //
  if (__SEGGER_RTL_float32_isspecial(z)) {
    return x;
  }
  //
  if (__SEGGER_RTL_float32_gt_rhs_positive(z, 1)) {
    if (__SEGGER_RTL_float32_gt_rhs_positive(z, M_LN_HUGEF)) {
      return K_INF_F32;
    }
    z = SEGGER_EXPF(z);
    z = SEGGER_SUBF(SEGGER_DIV2F(z), SEGGER_DIVF(0.5f, z));
    return __SEGGER_RTL_float32_signbit_xor(z, x);
  } else if (__SEGGER_RTL_float32_lt_rhs_positive(z, 1.0e-10f)) {
    return x;
  } else {
    z = SEGGER_MULF(x, x);
    return SEGGER_FMAF(SEGGER_FMAF(SEGGER_FMAF(2.03721912945e-4f, z, 8.33028376239e-3f), z, 1.66667160211e-1f),
                       SEGGER_MULF(z, x),
                       x);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sinh_scaled_integer()
*
*  Function description
*    Compute hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_sinh_scaled_integer(float x) {
  //
  __SEGGER_RTL_U32 x0, x1, x2, x3;
  __SEGGER_RTL_U32 xsq;
  __SEGGER_RTL_U32 sign;
  __SEGGER_RTL_U32 t;
  unsigned         q0, q1;
  unsigned         b;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Extract sign of argument.
  //
  sign = x0 & __SEGGER_RTL_U32_C(0x80000000);
  //
  // We can dispense with the normalities of checking for zero q0.  In this
  // case, we materialize the hidden bit and adjust q0 because we can fall
  // straight into the Sinh[x] == x with a "negative" over-compensated q0.
  //
  // Extract exponent to q0.
  //
  q0 = 0x96 - (x0 << 1 >> 24);
  //
  // Dispense with special cases early.
  //
  if ((int)q0 > 16) {
    //
    // Sinh[x] == x, 0 <= |x| <= 0x39000000 (1.2207031e-4).
    //
    if ((int)q0 >= 36) {
      return x;
    }
    //
    // Sinh[x] overflows when |x| > 0x42B2D4FC (89.415985).
    //
    if ((x0 ^ sign) > __SEGGER_RTL_U32_C(0x42B2D4FC)) {
      return __SEGGER_RTL_BitcastToF32(K_INF_U32 | sign);
    }
  } else {
    //
    // sNaN -> qNaN
    //
    if ((x0 << 1) > __SEGGER_RTL_U32_C(0xFF000000)) {
      x0 |= __SEGGER_RTL_U32_C(0x400000);
      return __SEGGER_RTL_BitcastToF32(x0);
    }
    //
    // Inf.
    //
    return __SEGGER_RTL_BitcastToF32(K_INF_U32 | sign);
  }
  //
  // Extract significand and materialize hidden bit.  We now have input = x0/2^q0.
  //
  x0 &= __SEGGER_RTL_U32_C(0xFFFFFF);
  x0 |= __SEGGER_RTL_U32_C(0x800000);
  //
  // Breakpoint at 0.5.
  //
  if (q0 < 25) {
    //
    // |x| >= 0x3F000000 (0.5)
    //
    // Multiply by Log2[E], approximation is generated by: BaseForm[N[Log2[E], 12]*4, 16]
    // which is 5C551D94AE, a 39-bit approximation.  We break this into two, a (signed) 31-bit
    // approximation 5c551d95.00 and an 8-bit extension which is 4.ae-5.00 = -0.52.
    //
    // Multiply by 1/Log[2] which is simply Log2[E].  Use Mathematica, BaseForm[N[Log2[E], 12], 16],
    // which is 5C551D94AE, a 39-bit approximation.  We break this into two, a (signed) 31-bit
    // approximation 5C551D95.00 and an 8-bit extension which is 4.AE-5.00 = -0.52.
    //
    __SEGGER_RTL_SMULL(x1, x2, __SEGGER_RTL_U32_C(0x5C551D95), x0);
    q1 = q0 - 5;
    t = __SEGGER_RTL_SIGN_EXTEND(x2, q1);
    //
    // Subtract extension product to provide correction.
    //
    x2 -= t;
    x1 = (t << (31 - q1)) + ((__SEGGER_RTL_I32)(1 + (x1 >> q1)) >> 1) - ((((__SEGGER_RTL_I32)x0 >> 4) * 0x52) >> q0);
    //
    // Extract bottom three bits of product, 'j'.
    //
    q1 = (__SEGGER_RTL_I32)x2 >> q1;
    b = __SEGGER_RTL_SIGN_EXTEND(q1, 3);
    q1 -= b;
    //
    // Evaluate exponential; see expf.
    //
    x1 = (__SEGGER_RTL_U32)(((__SEGGER_RTL_SMULL_X(__SEGGER_RTL_U32_C(0x58B90BFC), x1) >> 30) + 1) >> 1);
    xsq = __SEGGER_RTL_SMULL_HI(x1, x1);
    //
    x2 = __SEGGER_RTL_SMULL_HI(xsq, __SEGGER_RTL_U32_C(0x011114AD));
    x2 = __SEGGER_RTL_SMULL_HI(xsq, __SEGGER_RTL_U32_C(0x15555555) + ((__SEGGER_RTL_I32)x2 >> 4));
    x2 = __SEGGER_RTL_SMULL_HI(x1, x2);
    //
    x3 = __SEGGER_RTL_SMULL_HI(xsq, __SEGGER_RTL_U32_C(0x05557555));
    x3 = __SEGGER_RTL_SMULL_HI(xsq, __SEGGER_RTL_U32_C(0x3FFFFFFB) + ((__SEGGER_RTL_I32)x3 >> 4));
    //
    t = __SEGGER_RTL_float32_Exp_std_2tojby8[b + 4];
    x0 = __SEGGER_RTL_SMULL_HI(x1 + ((__SEGGER_RTL_I32)(x3 + ((__SEGGER_RTL_I32)x2 >> 2)) >> 1), t);
    x0 = t + ((__SEGGER_RTL_I32)x0 >> 2);
    //
    // Combine e^x with e^(-x).  If the difference between the exponents
    // is more than 32 then e^(-x) is insignificant and we don't need
    // to compute it.
    //
    q0 = (__SEGGER_RTL_I32)q1 >> 3;
    if (q0 < 16) {
      //
      // Compute e^-x.
      //
      t   = __SEGGER_RTL_float32_Exp_std_2tojby8[4 - b];
      x3 -= (__SEGGER_RTL_I32)x2 >> 2;
      t  += (__SEGGER_RTL_I32)__SEGGER_RTL_SMULL_HI(t, (x3 >> 1) - x1) >> 2;
      //
      // Combine: compute e^x - e^(-x)
      //
      x0 -= (__SEGGER_RTL_I32)t >> q0 >> q0;
    }
    //
    // At most one bit to normalize; can do this faster with clz support.
    //
    if (x0 & __SEGGER_RTL_U32_C(0x40000000)) {
      q0 += 0x9D - 32;
    } else {
      q0 += 0x9C - 32;
      x0 <<= 1;
    }
  } else {
    //
    // 0 <= x < 0x3e800000 (0.5)
    //
    // Value is small and a short Taylor approximation is good enough.
    //
    // Convert to Q31 format
    //
    x0 <<= 7;
    q0  += 7-32;                      // Q31
    //
    // Taylor approximation for Sinh[x]: x + x^3/6 + x^5/120 + x^7/5040 + O[x]^9
    // Recast using Horner's rule for evaluation.
    // Use Mathematica:
    //    BaseForm[NumberForm[N[Series[Sinh[x], {x, 0, 8}]*2, 8], ExponentFunction -> (Null &)], 16]
    // and read coefficients directly.
    //
    xsq = (__SEGGER_RTL_I32)x0 >> q0;
    xsq = __SEGGER_RTL_SMULL_HI(xsq, xsq);                                  // y = x^2
    x1  = __SEGGER_RTL_SMULL_HI(xsq,      __SEGGER_RTL_U32_C(0x001A01A0));  // 1/5040*y
    x1  = __SEGGER_RTL_SMULL_HI(xsq, x1 + __SEGGER_RTL_U32_C(0x04444444));  // (1/5040*y + 1/120)y
    x1  = __SEGGER_RTL_SMULL_HI(x0,  x1 + __SEGGER_RTL_U32_C(0x55555555));  // (((1/5040*y + 1/120)y + 1/6)x
    x1  = __SEGGER_RTL_SMULL_HI(x0,  x1);                                   // (((1/5040*y + 1/120)y + 1/6)x^2  (use precise Q31 x)
    x1  = __SEGGER_RTL_SMULL_HI(x0,  x1);                                   // (((1/5040*y + 1/120)y + 1/6)x^3 == (((1/5040*x^2 + 1/120)x^2 + 1/6)x^3 in Q31  (use precise Q31 x)
    //
    // x + (((1/5040*y + 1/120)y + 1/6)x^3 == (((1/5040*x^2 + 1/120)x^2 + 1/6)x^3 in Q32
    //
    x0 += (__SEGGER_RTL_I32)x1 >> q0 >> q0 >> 1;
    //
    // This is fast (hopefully) because x0 is computed immediately before and sets the condition codes.
    //
    if ((__SEGGER_RTL_I32)x0 < 0) {
      q0 = 0x9D-32 - q0;
      x0 >>= 1;
    } else {
      q0 = 0x9C-32 - q0;
    }
  }
  //
  // Pack result rounding x0.
  //
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_PACK(q0, x0, sign));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sinh_inline()
*
*  Function description
*    Compute hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_sinh_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_sinh_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_sinh_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_sinh_outline()
*
*  Function description
*    Compute hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_sinh_outline(float x) {
  return __SEGGER_RTL_float32_sinh_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sinh_inline()
*
*  Function description
*    Compute hyperbolic sine, double.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_sinh_inline(double x) {
  double y;
  //
  y = SEGGER_FABS(x);
  //
  // Zero or Inf/NaN arguments return themselves.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(y))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float64_gt_rhs_positive(y, 1)) {
    if (__SEGGER_RTL_float64_gt_rhs_positive(y, M_LN_HUGE)) {
      return K_INF_F64;
    }
    y = SEGGER_EXP(SEGGER_ADD(y, -0.69316101074218750000));
    if (__SEGGER_RTL_float64_lt_rhs_positive(y, 1.0e10)) {
      y = SEGGER_ADD(y, SEGGER_DIV(-0.24999308500451499336, y));
    }
    y = SEGGER_FMA(0.13830277879601902638e-4, y, y);
    return __SEGGER_RTL_float64_signbit_xor(y, x);
  } else if (__SEGGER_RTL_float64_lt_rhs_positive(y, 1.0e-10)) {
    return x;
  } else {
    y = SEGGER_MUL(x, x);
    return SEGGER_FMA(x,
                      SEGGER_DIV(__SEGGER_RTL_float64_PolyEvalP_4(y, __SEGGER_RTL_float64_Sinh.P, K_SINH_P_DBL),
                                 __SEGGER_RTL_float64_PolyEvalQ_3(y, __SEGGER_RTL_float64_Sinh.Q, K_SINH_Q_DBL)),
                      x);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_sinh_outline()
*
*  Function description
*    Compute hyperbolic sine, double.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_sinh_outline(double x) {
  return __SEGGER_RTL_float64_sinh_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cosh_scaled_integer()
*
*  Function description
*    Compute hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic cosine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_cosh_scaled_integer(float x) {
  __SEGGER_RTL_U32 x0, x1, x2, x3;
  __SEGGER_RTL_U32 xsq;
  __SEGGER_RTL_I32 i1;
  unsigned         q0, q1;
  unsigned         t, b;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Cosh[x] is an even function, so we don't need to preserve the sign bit
  // of x anywhere, unlike Sinh[x].
  //
  // We can dispense with the normalities of checking for zero q0.  In this
  // case, we materialize the hidden bit and adjust q0 because we can fall
  // straight into the Cosh[x] == 1 case with a "negative" over-compensated q0.
  //
  // Extract scale factor from exponent.
  //
  q0 = 0x96 - (x0 << 1 >> 24);
  //
  // Dispense with special cases early.
  //
  if (__SEGGER_RTL_UNLIKELY((int)q0 < 16)) {
    //
    // We have a large normal which overflows, or Inf which also overflows, or NaN.
    // The only case to distinguish is NaN which converts sNaN to qNaN.
    //
    if ((x0 << 1) <= __SEGGER_RTL_U32_C(0xFF000000)) {
      return K_INF_F32;
    } else {
      return __SEGGER_RTL_BitcastToF32(x0 | __SEGGER_RTL_U32_C(0x400000));
    }
  } else if (__SEGGER_RTL_UNLIKELY((x0 & __SEGGER_RTL_U32_C(0x7FFFFFFF)) > __SEGGER_RTL_U32_C(0x42B2D4FC))) {
    //
    // Cosh[x] overflows here...
    //
    return K_INF_F32;
  }
  //
  // Extract significand and materialize hidden bit.
  //
  x0 &= __SEGGER_RTL_U32_C(0xFFFFFF);
  x0 |= __SEGGER_RTL_U32_C(0x800000);
  //
  // Cosh[x] -> 1 for small x.
  //
  if (q0 > 24) {
    //
    // Cosh[x] == 1 for sufficiently small x.
    //
    if (q0 >= 36) {
      return 1.0f;
    }
    //
    // Value is small and a short Taylor approximation is good enough.
    //
    // 25 <= q0 <= 38, hence 0 <= q0-25 <= 13
    //
    x0 <<= 7;
    q0  += 7-32;                  // Q31
    //
    // Taylor approximation for Cosh[x]-1:  x^2/2 + x^4/24 + x^6/720 + x^8/40320 + O[x]^9
    // Setting y = x^2 we have y/2 + y^2/24 + y^3/720.  Recast using Horner's rule for evaluation.
    //
    // Use Mathematica:
    //
    //    BaseForm[NumberForm[N[Series[Cosh[x]-1, {x, 0, 8}]*8, 8], ExponentFunction -> (Null &)], 16]
    //
    // and read coefficients directly.
    //
    x2 = x0 >> q0;                                                          // Adjust for polynomial in x^2
    x2 = __SEGGER_RTL_SMULL_HI(x2, x2);                                     // y = x^2
    i1 = __SEGGER_RTL_SMULL_HI(x2,      __SEGGER_RTL_U32_C(0x0000D00D));    // 1/40320*y
    i1 = __SEGGER_RTL_SMULL_HI(x2, i1 + __SEGGER_RTL_U32_C(0x002D82D8));    // (1/40320*y + 1/720)*y
    i1 = __SEGGER_RTL_SMULL_HI(x2, i1 + __SEGGER_RTL_U32_C(0x05555555));    // ((1/40320*y + 1/720)*y + 1/24)*y...
    i1 = __SEGGER_RTL_SMULL_HI(x0, i1 + __SEGGER_RTL_U32_C(0x40000000));    // (((1/40320*y + 1/720)*y + 1/24)*y + 1/2)x    (use precise Q31 x)
    i1 = __SEGGER_RTL_SMULL_HI(x0, i1) >> q0 >> q0;                         // (((1/40320*y + 1/720)*y + 1/24)*y + 1/2)x^2  (use precise Q31 x)
    //
    // We have approximated Cosh[x]-1 above, so add one and that's an approximation for Cosh[x].
    //
    return __SEGGER_RTL_BitcastToF32(K_one_U32 + ((i1 + (1<<7)) >> 8));
    //
  } else {
    //
    // Multiply by Log2[E], approximation is generated by: BaseForm[N[Log2[E], 12]*4, 16]
    // which is 5c551d94ae, a 39-bit approximation.  We break this into two, a (signed) 31-bit
    // approximation 5c551d95.00 and an 8-bit extension which is 4.ae-5.00 = -0.52.
    //
    // Multiply by 1/Log[2] which is simply Log2[E].  Use Mathematica, BaseForm[N[Log2[E], 12], 16],
    // which is 5c551d94ae, a 39-bit approximation.  We break this into two, a (signed) 31-bit
    // approximation 5c551d95.00 and an 8-bit extension which is 4.ae-5.00 = -0.52.
    //
    __SEGGER_RTL_SMULL(x1, x2, __SEGGER_RTL_U32_C(0x5C551D95), x0);
    q1 = q0 - 5;
    //
    t = __SEGGER_RTL_SIGN_EXTEND(x2, q1);
    //
    // Subtract extension product to provide correction.
    //
    x2 -= t;
    x1 = (t << (31 - q1)) + ((__SEGGER_RTL_I32)(1 + (x1 >> q1)) >> 1) - ((((__SEGGER_RTL_I32)x0 >> 4) * 0x52) >> q0);
    //
    // Extract bottom three bits of product, 'j'.
    //
    q1 = (__SEGGER_RTL_I32)x2 >> q1;
    b = __SEGGER_RTL_SIGN_EXTEND(q1, 3);
    q1 -= b;
    //
    // Evaluate exponential; see expf.
    //
    x1 = (__SEGGER_RTL_U32)(((__SEGGER_RTL_SMULL_X(__SEGGER_RTL_U32_C(0x58B90BFC), x1) >> 30) + 1) >> 1);
    xsq = __SEGGER_RTL_SMULL_HI(x1, x1);
    //
    x2 = __SEGGER_RTL_SMULL_HI(xsq, __SEGGER_RTL_I32_C(0x011114AD));
    x2 = __SEGGER_RTL_SMULL_HI(xsq, __SEGGER_RTL_I32_C(0x15555555) + ((__SEGGER_RTL_I32)x2 >> 4));
    x2 = __SEGGER_RTL_SMULL_HI(x1, x2);
    //
    x3 = __SEGGER_RTL_SMULL_HI(xsq, __SEGGER_RTL_I32_C(0x05557555));
    x3 = __SEGGER_RTL_SMULL_HI(xsq, __SEGGER_RTL_I32_C(0x3FFFFFFB) + ((__SEGGER_RTL_I32)x3 >> 4));
    //
    t = __SEGGER_RTL_float32_Exp_std_2tojby8[b + 4];
    x0 = __SEGGER_RTL_SMULL_HI(x1 + ((__SEGGER_RTL_I32)(x3 + ((__SEGGER_RTL_I32)x2 >> 2)) >> 1), t);
    x0 = t + ((__SEGGER_RTL_I32)x0 >> 2);
    //
    // Combine e^x with e^(-x).  If the difference between the exponents
    // is more than 32 then e^(-x) is insignificant and we don't need
    // to compute it.
    //
    q0 = (__SEGGER_RTL_I32)q1 >> 3;
    if (q0 < 16) {
      //
      // Compute e^-x
      //
      t = __SEGGER_RTL_float32_Exp_std_2tojby8[4-b];
      x3 -= (__SEGGER_RTL_I32)x2 >> 2;
      t += (__SEGGER_RTL_I32)__SEGGER_RTL_SMULL_HI(t, (x3 >> 1) - x1) >> 2;
      //
      // Combine: compute e^x + e^(-x)
      //
      x0 += t >> q0 >> q0;
    }
    //
    // Normalize.
    //
    q0 += 0x9D - 32;
#if defined(__SEGGER_RTL_CLZ_U32) && !defined(__SEGGER_RTL_CLZ_U32_SYNTHESIZED)
    q1 = __SEGGER_RTL_CLZ_U32(x0 << 1);
    x0 <<= q1;
    q0  -= q1;
#else
    if ((x0 & __SEGGER_RTL_U32_C(0x40000000)) == 0) {
      x0 <<= 1;
      q0  -= 1;
    }
#endif
    //
    // Pack result rounding x0.
    //
    return __SEGGER_RTL_BitcastToF32(((__SEGGER_RTL_U32)q0 << 23) + ((x0 + (1<<6)) >> 7));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cosh_fpu()
*
*  Function description
*    Compute hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_cosh_fpu(float x) {
  //
  // We don't need any special-case handling here and just let
  // Inf, NaN, and zero trundle through.
  //
  x = SEGGER_FABSF(x);
  //
  if (__SEGGER_RTL_float32_gt_rhs_positive(x, 1)) {
    x = SEGGER_EXPF(SEGGER_ADDF(x, -0.6931610107421875f));
    if (__SEGGER_RTL_float32_lt_rhs_positive(x, 1.0e10f)) {
      x = SEGGER_ADDF(x, SEGGER_DIVF(0.24999308500451499336f, x));
    }
    return SEGGER_FMAF(0.13830277879601902638e-4f, x, x);
  } else {
    x = SEGGER_EXPF(x);
    return SEGGER_DIV2F(SEGGER_ADDF(x, SEGGER_DIVF(1, x)));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cosh_inline()
*
*  Function description
*    Compute hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_cosh_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_cosh_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_cosh_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cosh_outline()
*
*  Function description
*    Compute hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_cosh_outline(float x) {
  return __SEGGER_RTL_float32_cosh_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cosh_inline()
*
*  Function description
*    Compute hyperbolic cosine, double.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_cosh_inline(double x) {
  //
  // We don't need any special-case handling here and just let
  // Inf, NaN, and zero trundle through.
  //
  x = SEGGER_FABS(x);
  //
  if (__SEGGER_RTL_float64_gt_rhs_positive(x, 1)) {
    x = SEGGER_EXP(SEGGER_ADD(x, -0.6931610107421875));
    if (__SEGGER_RTL_float64_lt_rhs_positive(x, 1.0e10)) {
      x = SEGGER_ADD(x, SEGGER_DIV(0.24999308500451499336, x));
    }
    return SEGGER_FMA(0.13830277879601902638e-4, x, x);
  } else {
    x = SEGGER_EXP(x);
    return SEGGER_DIV2(SEGGER_ADD(x, SEGGER_DIV(1, x)));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cosh_outline()
*
*  Function description
*    Compute hyperbolic cosine, double.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_cosh_outline(double x) {
  return __SEGGER_RTL_float64_cosh_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tanh_fpu()
*
*  Function description
*    Compute hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_tanh_fpu(float x) {
  float y;
  float sign;
  //
  sign = x;
  x    = SEGGER_FABSF(x);
  //
  // Moving this test below taking the absolute value helps code
  // generation as the sign bit is known to be zero now.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(x))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float32_gt_rhs_positive(x, 27)) {
    x = 1;
  } else if (__SEGGER_RTL_float32_gt_rhs_positive(x, 0.549306144334054846f)) {
    x = SEGGER_EXPF(SEGGER_MUL2F(x));
    x = SEGGER_SUBF(1, SEGGER_DIVF(2, SEGGER_ADDF(x, 1)));
  } else if (__SEGGER_RTL_float32_gt_rhs_positive(x, 1.0e-10f)) {
    y = SEGGER_MULF(x, x);
    x = SEGGER_FMAF(x,
                    SEGGER_DIVF(__SEGGER_RTL_float32_PolyEvalP_3(y, __SEGGER_RTL_float32_Tanh.Poly.P, K_TANH_P_FLT),
                               __SEGGER_RTL_float32_PolyEvalQ_3(y, __SEGGER_RTL_float32_Tanh.Poly.Q, K_TANH_Q_FLT)),
                    x);
  }
  //
  return __SEGGER_RTL_float32_signbit_xor(x, sign);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tanh_scaled_integer()
*
*  Function description
*    Compute hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_tanh_scaled_integer(float x) {
  __SEGGER_RTL_U32 sign;
  __SEGGER_RTL_U32 x0, x1, x2, x3;
  unsigned         q0, q1, q2;
  //
  x0 = __SEGGER_RTL_BitcastToU32(x);
  //
  // Preserve operand sign
  //
  sign = x0 & __SEGGER_RTL_U32_C(0x80000000);
  //
  // Tanh[NaN] is qNaN.
  //
  if ((x0 << 1) > __SEGGER_RTL_U32_C(0xFF000000)) {
    return __SEGGER_RTL_BitcastToF32(x0 | __SEGGER_RTL_U32_C(0x400000));
  }
  //
  // Tanh[x] with |x| > ~+/-9.01091 is +/-1.
  //
  if ((x0 << 1) >= __SEGGER_RTL_X2(__SEGGER_RTL_U32_C(0x41102CB4))) {
    return __SEGGER_RTL_BitcastToF32(sign | K_one_U32);
  }
  //
  // Extract scale factor from exponent.
  //
  q0 = 0x96 - (x0 << 1 >> 24);
  //
  // Take care of small x.
  //
  if (q0 >= 39) {
    //
    // Tanh[0] or subnormal is zero because we make it so.
    //
    if ((__SEGGER_RTL_I32)q0 == 0x96) {
      return __SEGGER_RTL_BitcastToF32(sign);
    }
    //
    // Tanh[x] ~= x, x sufficiently small.
    //
    return x;
  }
  //
  // Extract significand and set hidden bit.
  //
  x0 &= __SEGGER_RTL_U32_C(0xFFFFFF);
  x0 |= __SEGGER_RTL_U32_C(0x800000);
  //
  if (q0 < 25) {
    //
    // 0.5 <= |x| <~ 9.01091
    //
    // Multiply by 1/Log[2] which is simply Log2[E].  Use Mathematica, BaseForm[N[Log2[E], 12], 16],
    // which is 5c551d94ae, a 39-bit approximation.  We break this into two, a (signed) 31-bit
    // approximation 5c551d95.00 and an 8-bit extension which is 4.ae-5.00 = -0.52.
    //
    x0 = -(__SEGGER_RTL_I32)x0;
    q2 = q0 - 6;
    __SEGGER_RTL_SMULL(x0, x1, __SEGGER_RTL_I32_C(0x5C551D95), x0);
    x0 = (x1 << (32-q2) >> 1) + ((__SEGGER_RTL_I32)((x0 >> q2) + 1) >> 1) - ((((__SEGGER_RTL_I32)x0 >> 3) * 0x52) >> q0);
    //
    // Use low 3 bits as breakpoints.
    //
    q2 = x1 >> q2;
    q0 = (__SEGGER_RTL_I32)q2 << 29 >> 29;  // break on midpoint
    q2 = q0 - q2;
    q2 = (q2 >> 3) & 31;  // q2 is [1, 26].
    //
    // Compute e^x.
    //
    x1 = __SEGGER_RTL_SMULL_HI(x0, __SEGGER_RTL_I32_C(0x002BDEDC));
    x1 = __SEGGER_RTL_SMULL_HI(x0, __SEGGER_RTL_I32_C(0x013B3101) + ((__SEGGER_RTL_I32)x1 >> 2));
    x1 = __SEGGER_RTL_SMULL_HI(x0, __SEGGER_RTL_I32_C(0x071AC1F1) + ((__SEGGER_RTL_I32)x1 >> 2));
    x1 = __SEGGER_RTL_SMULL_HI(x0, __SEGGER_RTL_I32_C(0x1EBFBDFE) + ((__SEGGER_RTL_I32)x1 >> 2));
    x1 = __SEGGER_RTL_SMULL_HI(x0, __SEGGER_RTL_I32_C(0x58B90BFC) + ((__SEGGER_RTL_I32)x1 >> 2));
    x0 = __SEGGER_RTL_float32_Exp_std_2tojby8[q0 + 4];  // ENHANCEMENT: could rotate table as in exp() and modify q2 accordingly!
    x0 = __SEGGER_RTL_SMULL_HI((__SEGGER_RTL_I32)x1 >> 1, x0) + x0;
    x0 >>= q2;
    //
    // x0 is now e^2x.  Compute Tanh[x] = (e^2x-1) / (e^2x+1)
    //
    // 1 + e^2x, already normalized.
    //
    x1 = __SEGGER_RTL_U32_C(0x40000000) + x0;
    //
    // 1 - e^2x, normalize.
    //
    x0   = __SEGGER_RTL_U32_C(0x40000000) - x0;
    q0   = 1;
    x0 <<= 1;
    //
    // We now compute 1/(1-e^2x) * (1+e^2x).
    //
    // Use leading six bits of divisor to estimate leading 8 bits of reciprocal.
    //
    x3 = (int)(__SEGGER_RTL_I8)__SEGGER_RTL_float32_tanh_Reciprocal[(x1 >> 24) & 63];
    //
    // Do division, based on Tang's single-precision division.
    // ENHANCEMENT: could negate reciprocal table and change all the signs.
    //
    x1 *= x3;
    x2 = __SEGGER_RTL_SMULL_HI(x1, x1);
    x2 = __SEGGER_RTL_SMULL_HI(__SEGGER_RTL_SMULL_HI(x3 << 24, x1),
                               __SEGGER_RTL_I32_C(0x40000001) + ((__SEGGER_RTL_I32)x1 >> 6) + ((__SEGGER_RTL_I32)x2 >> 10) + ((__SEGGER_RTL_I32)x1 >> 31));
    x0 = __SEGGER_RTL_SMULL_HI(((1-(__SEGGER_RTL_I32)x2) >> 1) - (x3 << 25), x0);
    //
    // Normalize.
    //
#if defined(__SEGGER_RTL_CLZ_U32)
    //
    // There is a fast CLZ available, so use it to normalize.
    //
    q1   = __SEGGER_RTL_CLZ_U32(x0) - 1;
    q0  += q1;
    x0 <<= q1;
    //
#else
    //
    // By exhaustion, we know that x0 only needs 1 or 2 shifts
    // to normalize.  Use this to do the normalization.
    //
    // NOTE: If the approximation calculations above are ever
    // changed, this may not apply and a rerun of normalization
    // analysis will be required!
    //
    if ((x0 & __SEGGER_RTL_U32_C(0x40000000)) == 0) {
      x0 <<= 1;
      q0  += 1;
      if ((x0 & __SEGGER_RTL_U32_C(0x40000000)) == 0) {
        x0 <<= 1;
        q0  += 1;
      }
    }
#endif
    //
    // Pack, round, and return.
    //
    return __SEGGER_RTL_BitcastToF32(sign + ((__SEGGER_RTL_U32)(0x9d - q0 - 30) << 23) + ((x0 + (1<<6)) >> 7));
    //
  } else {
    //
    // 0 <= |x| < 0.5
    //
    // Normalize such that msb is bit 30.
    //
    x0 <<= 7;
    q0  += 7;
    //
    // Approximate Tanh[x] as an optimized Taylor polynomial.
    //
    //   BaseForm[N[Normal[Series[Tanh[x], {x, 0, 12}]], 10], 16]
    //
    // Polynomial in x^2, we use i2:q1 as x0:q0^2.
    //
    q1 = 2*q0 - 64;
    x2 = __SEGGER_RTL_SMULL_HI(x0, x0);
    x1 = __SEGGER_RTL_SMULL_HI(x2, -__SEGGER_RTL_I32_C(0x01B402EA));
    x1 = __SEGGER_RTL_SMULL_HI(x2,  __SEGGER_RTL_I32_C(0x057401DA) + ((__SEGGER_RTL_I32)x1 >> q1));
    x1 = __SEGGER_RTL_SMULL_HI(x2, -__SEGGER_RTL_I32_C(0x0DCC626C) + ((__SEGGER_RTL_I32)x1 >> q1));
    x1 = __SEGGER_RTL_SMULL_HI(x2,  __SEGGER_RTL_I32_C(0x2221E539) + ((__SEGGER_RTL_I32)x1 >> q1));
    x1 = __SEGGER_RTL_SMULL_HI(x2, -__SEGGER_RTL_I32_C(0x55555447) + ((__SEGGER_RTL_I32)x1 >> q1));
    //
    // x + x(ax^2 + bx^4 + cx^6...) = x + ax^3 + bx^5 + cx^7...
    //
    x0 += (__SEGGER_RTL_I32)__SEGGER_RTL_SMULL_HI(x0, x1) >> q1;
    //
    // Normalize.
    //
#if defined(__SEGGER_RTL_CLZ_U32) && !defined(__SEGGER_RTL_CLZ_U32_SYNTHESIZED)
    //
    // There is a fast CLZ available, so use it to normalize.
    //
    q1 = __SEGGER_RTL_CLZ_U32(x0) - 1;
    q0  += q1;
    x0 <<= q1;
    //
#else
    //
    // By exhaustion, we know that x0 only needs 1 or 2 shifts
    // to normalize.  Use this to do the normalization.
    //
    // NOTE: If the approximation calculations above are ever
    // changed, this may not apply and a rerun of normalization
    // analysis will be required!
    //
    if ((x0 & __SEGGER_RTL_U32_C(0x40000000)) == 0) {
      x0 <<= 1;
      q0  += 1;
      if ((x0 & __SEGGER_RTL_U32_C(0x40000000)) == 0) {
        x0 <<= 1;
        q0  += 1;
      }
    }
#endif
    //
    // Pack, round, and return.
    //
    return __SEGGER_RTL_BitcastToF32(sign + ((__SEGGER_RTL_U32)(0x9d - q0 - 1) << 23) + ((x0 + (1<<6)) >> 7));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tanh_inline()
*
*  Function description
*    Compute hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_tanh_inline(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_tanh_scaled_integer(x);
#else
  return __SEGGER_RTL_float32_tanh_fpu(x);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tanh_outline()
*
*  Function description
*    Compute hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_tanh_outline(float x) {
  return __SEGGER_RTL_float32_tanh_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_tanh_inline()
*
*  Function description
*    Compute hyperbolic tangent, double.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_tanh_inline(double x) {
  double y;
  double sign;
  //
  sign = x;
  x    = SEGGER_FABS(x);
  //
  // Moving this test below taking the absolute value helps code
  // generation as the sign bit is known to be zero now.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(x))) {
    return x;
  } else if (__SEGGER_RTL_float64_ge_rhs_positive(x, 27)) {
    x = 1;
  } else if (__SEGGER_RTL_float64_gt_rhs_positive(x, 0.549306144334054846)) {
    x = SEGGER_EXP(SEGGER_MUL2(x));
    x = SEGGER_SUB(1, SEGGER_DIV(2, SEGGER_ADD(x, 1)));
  } else if (__SEGGER_RTL_float64_gt_rhs_positive(x, 1.0e-10)) {
    y = SEGGER_MUL(x, x);
    x = SEGGER_FMA(x,
                   SEGGER_DIV(__SEGGER_RTL_float64_PolyEvalP_3(y, __SEGGER_RTL_float64_Tanh.Poly.P, K_TANH_P_DBL),
                              __SEGGER_RTL_float64_PolyEvalQ_3(y, __SEGGER_RTL_float64_Tanh.Poly.Q, K_TANH_Q_DBL)),
                   x);
  }
  //
  return __SEGGER_RTL_float64_signbit_xor(x, sign);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_tanh_outline()
*
*  Function description
*    Compute hyperbolic tangent, double.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_tanh_outline(double x) {
  return __SEGGER_RTL_float64_tanh_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_asinh_inline()
*
*  Function description
*    Compute inverse hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sine of.
*
*  Additional information
*    Calculates the inverse hyperbolic sine of x. 

*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic sine of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_asinh_inline(float x) {
  float t;
  float w;
  //
  t = SEGGER_FABSF(x);
  //
  if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float32_isfinite_inline(t))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float32_lt_rhs_positive(t, __SEGGER_RTL_FLT_SELECT(0x1p-28f, 3.7252902984619140625e-9f))) {
    return x;
  } else if (__SEGGER_RTL_float32_gt_rhs_positive(t, __SEGGER_RTL_FLT_SELECT(0x1p+28f, 268435456.0f))) {
    w = SEGGER_ADDF(SEGGER_LOGF(t), M_LN2_FLT);
  } else if (__SEGGER_RTL_float32_gt_rhs_positive(t, 2.0f)) {
    w = SEGGER_LOGF(SEGGER_ADDF(SEGGER_MUL2F(t), SEGGER_DIVF(1, SEGGER_ADDF(SEGGER_SQRTF(SEGGER_FMAF(x, x, 1)), t))));
  } else {
    w = SEGGER_MULF(x, x);
    w = SEGGER_LOG1PF(SEGGER_ADDF(t, SEGGER_DIVF(w, SEGGER_ADDF(1, SEGGER_SQRTF(SEGGER_ADDF(1, w))))));
  }
  //
  return __SEGGER_RTL_float32_signbit_xor(w, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_asinh_inline()
*
*  Function description
*    Compute inverse hyperbolic sine, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sine of.
*
*  Additional information
*    Calculates the inverse hyperbolic sine of x. 

*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic sine of x.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_asinh_inline(double x) {
  double t;
  double w;
  //
  t = SEGGER_FABS(x);
  //
  if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(t))) {
    return x;
  } else if (__SEGGER_RTL_float64_lt_rhs_positive(t, __SEGGER_RTL_FLT_SELECT(0x1p-28f, 3.7252902984619140625e-9f))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float64_ge_rhs_positive(t, __SEGGER_RTL_FLT_SELECT(0x1p28f, 268435456))) {
    w = SEGGER_ADD(SEGGER_LOG(t), M_LN2_DBL);
  } else if (__SEGGER_RTL_float64_ge_rhs_positive(t, 2)) {
    w = SEGGER_LOG(SEGGER_ADD(SEGGER_MUL2(t), SEGGER_DIV(1, SEGGER_ADD(SEGGER_SQRT(SEGGER_FMA(x, x, 1)), t))));
  } else {
    w = SEGGER_MUL(x, x);
    w = SEGGER_LOG1P(SEGGER_ADD(t, SEGGER_DIV(w, SEGGER_ADD(1, SEGGER_SQRT(SEGGER_ADD(1, w))))));
  }
  //
  w = __SEGGER_RTL_float64_signbit_xor(w, x);
  //
  return w;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_acosh_inline()
*
*  Function description
*    Compute inverse hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Additional information
*    Calculates the non-negative inverse hyperbolic cosine of x. 

*  Return value
*    * If x < 1, return NaN.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic cosine of x.
*
*  Notes
*    acosh(x) = ln(x + sqrt(x*x - 1));  x >= 1
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_acosh_inline(float x) {       
  //
  // Don't need any special NaN handling here.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_lt_rhs_positive(x, 1))) {
    return K_NAN_F32;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_gt_rhs_positive(x, 1500))) { // Know x is positive non-NaN >= 1.
    return SEGGER_ADDF(SEGGER_LOGF(x), M_LN2_FLT);  // acosh(big) = log(2x) = log(2) + log(x)
  } else if (__SEGGER_RTL_float32_gt_rhs_positive(x, 2)) {
    return SEGGER_LOGF(SEGGER_FMSF(2, x, SEGGER_DIVF(1, SEGGER_ADDF(x, SEGGER_SQRTF(SEGGER_FMAF(x, x, -1))))));
  } else {
    x = SEGGER_ADDF(x, -1);
    return SEGGER_LOG1PF(SEGGER_ADDF(x, SEGGER_SQRTF(SEGGER_FMAF(2, x, SEGGER_MULF(x, x)))));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_acosh_inline()
*
*  Function description
*    Compute inverse hyperbolic cosine, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Additional information
*    Calculates the non-negative inverse hyperbolic cosine of x. 

*  Return value
*    * If x < 1, return NaN.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic cosine of x.
*
*  Notes
*    acosh(x) = ln(x + sqrt(x*x - 1));  x >= 1
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_acosh_inline(double x) {
  //
  // Don't need any special NaN handling here.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_lt_rhs_positive(x, 1))) {
    return K_NAN_F64;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_ge_rhs_positive(x, __SEGGER_RTL_FLT_SELECT(0x1p28f, 268435456)))) {
    return SEGGER_ADD(SEGGER_LOG(x), M_LN2_DBL);      // acosh(big) = log(2x) = log(2) + log(x)
  } else if (__SEGGER_RTL_float64_ge_rhs_positive(x, 2)) {
    return SEGGER_LOG(SEGGER_FMS(2, x, SEGGER_DIV(1, SEGGER_ADD(x, SEGGER_SQRT(SEGGER_FMA(x, x, -1))))));
  } else {
    x = SEGGER_ADD(x, -1);
    return SEGGER_LOG1P(SEGGER_ADD(x, SEGGER_SQRT(SEGGER_FMA(2, x, SEGGER_MUL(x, x)))));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_atanh_inline()
*
*  Function description
*    Compute inverse hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Additional information
*    Calculates the non-negative inverse hyperbolic tangent of x. 

*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * If x = +/-1, return +/-infinity.
*    * Else, return inverse hyperbolic tangent of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_atanh_inline(float x) {
  float t;
  float mx;
  //
  mx = __SEGGER_RTL_float32_abs_inline(x);
  //
  // Moving this test below taking the absolute value helps code
  // generation as the sign bit is known to be zero now.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(mx))) {
    return x;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_ge_finite(mx, 1))) {
    if (__SEGGER_RTL_float32_eq_bitwise(mx, 1)) {
      return __SEGGER_RTL_float32_signbit_xor(K_INF_F32, x);
    } else {
      return K_NAN_F32;
    }
  }
  //
  if (__SEGGER_RTL_float32_lt_rhs_positive(mx, __SEGGER_RTL_FLT_SELECT(0x1p-28f, 3.7252902984619140625e-9f))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float32_lt_rhs_positive(mx, 0.5f)) {
    t = SEGGER_MUL2F(mx);
    t = SEGGER_DIV2F(SEGGER_LOG1PF(SEGGER_ADDF(t, SEGGER_DIVF(SEGGER_MULF(t, mx), SEGGER_SUBF(1, mx)))));
  } else {
    t = SEGGER_DIV2F(SEGGER_LOG1PF(SEGGER_DIVF(SEGGER_MUL2F(mx), SEGGER_SUBF(1, mx))));
  }
  //
  return __SEGGER_RTL_float32_signbit_xor(t, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_atanh_inline()
*
*  Function description
*    Compute inverse hyperbolic tangent, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Additional information
*    Calculates the non-negative inverse hyperbolic tangent of x. 

*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * If x = +/-1, return +/-infinity.
*    * Else, return inverse hyperbolic tangent of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_atanh_inline(double x) {
  double mx;
  double t;
  //
  // Get magnitude of x.
  //
  mx = __SEGGER_RTL_float64_abs_inline(x);
  //
  // Moving this test below taking the absolute value helps code
  // generation as the sign bit is known to be zero now.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(mx))) {
    return x;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_ge_finite(mx, 1))) {
    if (__SEGGER_RTL_float64_eq_finite(mx, 1)) {
      return __SEGGER_RTL_float64_signbit_xor(K_INF_F64, x);
    } else {
      return K_NAN_F64;
    }
  } else if (__SEGGER_RTL_float64_lt_rhs_positive(mx, __SEGGER_RTL_FLT_SELECT(0x1p-28f, 3.7252902984619140625e-9))) {
    return x;
  }
  //
  if (__SEGGER_RTL_float64_lt_rhs_positive(mx, 0.5)) {
    t = SEGGER_MUL2(mx);
    t = SEGGER_DIV2(SEGGER_LOG1P(SEGGER_ADD(t, SEGGER_DIV(SEGGER_MUL(t, mx), SEGGER_SUB(1, mx)))));
  } else {
    t = SEGGER_DIV2(SEGGER_LOG1P(SEGGER_DIV(SEGGER_MUL2(mx), SEGGER_SUB(1, mx))));
  }
  //
  return __SEGGER_RTL_float64_signbit_xor(t, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fmax_soft_inline()
*
*  Function description
*    Compute maximum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_fmax_soft_inline(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_soft(x))) {
    return y;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_soft(y))) {
    return x;
  } else if (__SEGGER_RTL_float32_signbit_soft(x | y)) {
    return (x < y) ? x : y;
  } else {
    return (x >= y) ? x : y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fmax_fpu_inline()
*
*  Function description
*    Compute maximum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_fmax_fpu_inline(float x, float y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(x))) {
    return y;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(y))) {
    return x;
  } else if (SEGGER_GTF(x, y)) {
    return x;
  } else {
    return y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fmax_inline()
*
*  Function description
*    Compute maximum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_fmax_inline(float x, float y) {
#if __SEGGER_RTL_FP_HW == 0
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_float32_fmax_soft_inline(__SEGGER_RTL_BitcastToU32(x), __SEGGER_RTL_BitcastToU32(y)));
#else
  return __SEGGER_RTL_float32_fmax_fpu_inline(x, y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_fmax_inline()
*
*  Function description
*    Compute maximum, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_fmax_inline(double x, double y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(x))) {
    return y;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(y))) {
    return x;
  } else if (SEGGER_GT(x, y)) {
    return x;
  } else {
    return y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fmin_fpu_inline()
*
*  Function description
*    Compute minimum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_fmin_fpu_inline(float x, float y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(x))) {
    return y;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(y))) {
    return x;
  } else if (SEGGER_LTF(x, y)) {
    return x;
  } else {
    return y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fmin_soft_inline()
*
*  Function description
*    Compute minimum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*/
static __SEGGER_RTL_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_float32_fmin_soft_inline(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_soft(x))) {
    return y;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_soft(y))) {
    return x;
  } else if (__SEGGER_RTL_float32_signbit_soft(x | y)) {
    return (x >= y) ? x : y;
  } else {
    return (x < y) ? x : y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fmin_inline()
*
*  Function description
*    Compute minimum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_fmin_inline(float x, float y) {
#if __SEGGER_RTL_FP_HW == 0
  return __SEGGER_RTL_BitcastToF32(__SEGGER_RTL_float32_fmin_soft_inline(__SEGGER_RTL_BitcastToU32(x), __SEGGER_RTL_BitcastToU32(y)));
#else
  return __SEGGER_RTL_float32_fmin_fpu_inline(x, y);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_fmin_inline()
*
*  Function description
*    Compute minimum, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_fmin_inline(double x, double y) {
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(x))) {
    return y;
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(y))) {
    return x;
  } else if (SEGGER_LT(x, y)) {
    return x;
  } else {
    return y;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fdim_inline()
*
*  Function description
*    Positive difference, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x > y, x-y.
*    * Else, +0.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_fdim_inline(float x, float y) {
  if (SEGGER_GTF(x, y)) {
    return SEGGER_SUBF(x, y);
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_fdim_inline()
*
*  Function description
*    Positive difference, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x > y, x-y.
*    * Else, +0.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_fdim_inline(double x, double y) {
  if (SEGGER_GT(x, y)) {
    return SEGGER_SUB(x, y);
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_modf_inline()
*
*  Function description
*    Separate integer and fractional parts, float.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modff() returns the signed
*    fractional part of x.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_modf_inline(float x, float *iptr) {
  __SEGGER_RTL_U32 x0;
  int              exponent;
  //
  x0       = __SEGGER_RTL_BitcastToU32(x);
  exponent = FLOAT32_EXPONENT(x0);
  //
  exponent -= 0x7F;
  if (exponent < 0) {
    *iptr = 0.0f;
    return x;
  } else if (exponent >= 23) {
    *iptr = x;
    return __SEGGER_RTL_BitcastToF32(x0 & FLOAT32_SIGN_MASK);  // 0 with sign of x
  }
  //
  // Remove fractional bits from x.
  //
  x0 &= ~(FLOAT32_SIGNIFICAND_MASK >> exponent);
  *iptr = __SEGGER_RTL_BitcastToF32(x0);
  //
  // Return fractional value.
  //
  return SEGGER_SUBF(x, *iptr);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_modf_outline()
*
*  Function description
*    Separate integer and fractional parts, float.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modff() returns the signed
*    fractional part of x.
*/
static float __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float32_modf_outline(float x, float *iptr) {
  return __SEGGER_RTL_float32_modf_inline(x, iptr);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_modf_inline()
*
*  Function description
*    Separate integer and fractional parts, double.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modf() returns the signed
*    fractional part of x.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_modf_inline(double x, double *iptr) {
  __SEGGER_RTL_U64 x0;
  int              exponent;
  //
  x0       = __SEGGER_RTL_BitcastToU64(x);
  exponent = FLOAT64_EXPONENT(x0);
  //
  exponent -= 0x3FF;
  if (exponent < 0) {
    *iptr = 0;
    return x;
  } else if (exponent >= 52) {
    *iptr = x;
    return __SEGGER_RTL_BitcastToF64(x0 & FLOAT64_SIGN_MASK);  // 0 with sign of x
  }
  //
  // Remove fractional bits from x.
  //
  x0 &= ~(FLOAT64_SIGNIFICAND_MASK >> exponent);
  *iptr = __SEGGER_RTL_BitcastToF64(x0);
  //
  // Return fractional value.
  //
  return SEGGER_SUB(x, *iptr);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_modf_outline()
*
*  Function description
*    Separate integer and fractional parts, double.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modf() returns the signed
*    fractional part of x.
*/
static double __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_modf_outline(double x, double *iptr) {
  return __SEGGER_RTL_float64_modf_inline(x, iptr);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_fmod_inline()
*
*  Function description
*    Compute remainder after division, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_fmod_inline(float x, float y) {
  __SEGGER_RTL_U32 i;
  __SEGGER_RTL_U32 x_i0, y_i0;
  int              x_q0, y_q0;
  __SEGGER_RTL_U32 x_sign;
  //
  // Convert incoming arguments.
  //
  x_i0   = __SEGGER_RTL_BitcastToU32(x);
  y_i0   = __SEGGER_RTL_BitcastToU32(y);
  x_sign = x_i0 & FLOAT32_SIGN_MASK;
  //
  // Deal with special cases out of line.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial_or_negative(x) ||
                            __SEGGER_RTL_float32_isspecial_or_negative(y))) {
    //
    // (NaN, x) and (x, NaN) is NaN.
    //
    if (__SEGGER_RTL_float32_isnan_inline(x)) {
      return x;
    } else if (__SEGGER_RTL_float32_isnan_inline(y)) {
      return y;
    }
    //
    // (+-0, y) is x for y != 0.
    //
    if ((x_i0 << 1) == 0 && (y_i0 << 1) != 0) {
      return x;
    }
    //
    // (Inf, x) is NaN.
    //
    if (__SEGGER_RTL_float32_isinf_inline(x)) {
      return K_NAN_F32;
    }
    //
    // (x, Inf) is x.
    //
    if (__SEGGER_RTL_float32_isinf_inline(y)) {
      return x;
    }
    //
    // (x, 0) = NaN.
    //
    if ((y_i0 << 1) == 0) {
      return K_NAN_F32;
    }
    //
    // y could be negative in this case as negative operands are caught above,
    // so remove sign.
    //
    y_i0 &= ~FLOAT32_SIGN_MASK;
  }
  //
  // x and y are known to be non-zero and finite with y > 0.
  //
  // If |x| <  |y|, return x mod |y|, easily calculated.
  // If |x| == |y|, return 0 with x's sign.
  //
  x_i0 ^= x_sign;
  if (__SEGGER_RTL_UNLIKELY(x_i0 <= y_i0)) {
    if (x_i0 == y_i0) {
      return __SEGGER_RTL_BitcastToF32(x_sign);
    } else {
      return x;
    }
  }
  //
  // Normalize x.
  //
  x_q0 = FLOAT32_EXPONENT(x_i0);
  if (x_q0 != 0) {
    x_i0 &= FLOAT32_SIGNIFICAND_MASK;
    x_i0 |= FLOAT32_HIDDEN_MASK;
  } else {
    x_q0 = __SEGGER_RTL_float32_normalize(&x_i0, 0);
  }
  //
  // Normalize y.
  //
  y_q0 = FLOAT32_EXPONENT(y_i0);
  if (y_q0 != 0) {
    y_i0 &= FLOAT32_SIGNIFICAND_MASK;
    y_i0 |= FLOAT32_HIDDEN_MASK;
  } else {
    y_q0 = __SEGGER_RTL_float32_normalize(&y_i0, 0);
  }
  //
  // Compute x mod y with early out if y divides x.
  //
  while (x_q0 > y_q0) {
    i = x_i0 - y_i0;
    if ((i & __SEGGER_RTL_U32_C(0x80000000)) == 0) {
      if (i == 0) {
        return __SEGGER_RTL_BitcastToF32(x_sign);
      }
      x_i0 = i;
    }
    x_i0 <<= 1;
    x_q0  -= 1;
  }
  //
  i = x_i0 - y_i0;
  if ((i & __SEGGER_RTL_U32_C(0x80000000)) == 0) {
    if (i == 0) {
      return __SEGGER_RTL_BitcastToF32(x_sign);
    }
    x_i0 = i;
  }
  //
  // Normalize.
  //
  x_q0 = __SEGGER_RTL_float32_normalize(&x_i0, x_q0);
  //
  // Pack and return.
  //
  --x_q0;
  if (__SEGGER_RTL_UNLIKELY(x_q0 < 0)) {
    //
    // Subnormal result.
    //
    x_i0 >>= -x_q0;
    x_q0   = 0;
  }
  //
  // Pack remainder.
  //
  return __SEGGER_RTL_BitcastToF32(((__SEGGER_RTL_U32)x_q0 << 23) 
                                   + x_i0
                                   + x_sign);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_fmod_inline()
*
*  Function description
*    Compute remainder after division, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_fmod_inline(double x, double y) {
  //
#if __SEGGER_RTL_FP_HW >= 2
  //
  double xmag;
  double ymag;
  double ymagX;
  double r;
  int    n;
  int    nr;
  int    ny;
  //
  if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(x) ||
                            __SEGGER_RTL_float64_isnan_inline(y) ||
                            __SEGGER_RTL_float64_putative_iszero(y))) {
    return K_NAN_F64;
  }
  //
  xmag = __SEGGER_RTL_float64_abs_inline(x);
  ymag = __SEGGER_RTL_float64_abs_inline(y);
  //
  if (SEGGER_LT(xmag, ymag)) {
    return x;
  } else if (SEGGER_EQ(xmag, ymag)) {
    return __SEGGER_RTL_float64_signbit_copy(0.0, x);
  }
  //
  r = xmag;
  __SEGGER_RTL_float64_frexp_inline(ymag, &ny);
  //
  while (SEGGER_GE(r, ymag)) {
    __SEGGER_RTL_float64_frexp_inline(r, &nr);
    n = nr - ny;
    ymagX = __SEGGER_RTL_float64_ldexp_inline(ymag, n);
    if (SEGGER_GT(ymagX, r)) {
      ymagX = __SEGGER_RTL_float64_ldexp_inline(ymag, n-1);
    }
    r = SEGGER_SUB(r, ymagX);
  }
  return SEGGER_LT0(x) ? SEGGER_NEG(r) : r;
  //
#else
  //
  __SEGGER_RTL_U64 i;
  __SEGGER_RTL_U64 x_sign;
  __SEGGER_RTL_U64 x_i0, y_i0;
  int              x_q0, y_q0;
  //
  //
  // Convert incoming arguments.
  //
  x_i0   = __SEGGER_RTL_BitcastToU64(x);
  y_i0   = __SEGGER_RTL_BitcastToU64(y);
  x_sign = x_i0 & FLOAT64_SIGN_MASK;
  //
  // Deal with special cases out of line.
  //
  if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(x) ||
                            __SEGGER_RTL_float64_isnan_inline(y) ||
                            __SEGGER_RTL_float64_putative_iszero(y))) {
    return K_NAN_F64;
  }
  //
  y_i0 &= ~FLOAT64_SIGN_MASK;
  //
  // x and y are known to be non-zero and finite with y > 0.
  //
  // If |x| <  |y|, return x mod |y|, easily calculated.
  // If |x| == |y|, return 0 with x's sign.
  //
  x_i0 ^= x_sign;
  if (__SEGGER_RTL_UNLIKELY(x_i0 <= y_i0)) {
    if (x_i0 == y_i0) {
      return __SEGGER_RTL_BitcastToF64(x_sign);
    } else {
      return x;
    }
  }
  //
  // Normalize x.
  //
  x_q0 = FLOAT64_EXPONENT(x_i0);
  if (x_q0 != 0) {
    x_i0 &= FLOAT64_SIGNIFICAND_MASK;
    x_i0 |= FLOAT64_HIDDEN_MASK;
  } else {
    x_q0 = __SEGGER_RTL_float64_normalize(&x_i0, 0);
  }
  //
  // Normalize y.
  //
  y_q0 = FLOAT64_EXPONENT(y_i0);
  if (y_q0 != 0) {
    y_i0 &= FLOAT64_SIGNIFICAND_MASK;
    y_i0 |= FLOAT64_HIDDEN_MASK;
  } else {
    y_q0 = __SEGGER_RTL_float64_normalize(&y_i0, 0);
  }
  //
  // Compute x mod y with early out if y divides x.
  //
  while (x_q0 > y_q0) {
    i = x_i0 - y_i0;
    if ((i & __SEGGER_RTL_U64_C(0x8000000000000000)) == 0) {
      if (i == 0) {
        return __SEGGER_RTL_BitcastToF64(x_sign);
      }
      x_i0 = i;
    }
    x_i0 <<= 1;
    x_q0  -= 1;
  }
  //
  i = x_i0 - y_i0;
  if ((i & __SEGGER_RTL_U64_C(0x8000000000000000)) == 0) {
    if (i == 0) {
      return __SEGGER_RTL_BitcastToF64(x_sign);
    }
    x_i0 = i;
  }
  //
  // Profiling reveals it's faster to normalize without CLZ
  // support on RV32.
  //
  while ((x_i0 & __SEGGER_RTL_U64_C(0x0010000000000000)) == 0) {
    x_i0 <<= 1;
    x_q0  -= 1;
  }
  //
  // Pack and return.
  //
  --x_q0;
  if (__SEGGER_RTL_UNLIKELY(x_q0 < 0)) {
    //
    // Subnormal result.
    //
    x_i0 >>= -x_q0;
    x_q0   = 0;
  }
  //
  return __SEGGER_RTL_BitcastToF64(((__SEGGER_RTL_U64)x_q0 << 52) 
                                   + x_i0
                                   + x_sign);
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_remainder_inline()
*
*  Function description
*    Compute remainder after division, double.
*
*  Parameters
*    x   - Value #1.
*    y   - Value #2.
*    quo - Pointer to the object that receives the quotient of x
*          divided by y, modulo 2^(INT_MAX+1)
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result is in the range [-|y/2|, +|y/2|].  In the case that
*    x-iy = 1/2, we require that i be even.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_remainder_inline(float x, float y, int *quo) {
  float    xmag;
  float    ymag;
  float    ymagX;
  float    r;
  int      n;
  int      nr;
  int      ny;
  unsigned parity;
  int      quotient;
  //
  if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float32_isfinite_inline(x) ||
                            __SEGGER_RTL_float32_isnan_inline(y) ||
                            __SEGGER_RTL_float32_putative_iszero(y))) {
    *quo = 0;
    return K_NAN_F32;
  }
  //
  xmag = SEGGER_FABSF(x);
  ymag = SEGGER_FABSF(y);
  //
  if (SEGGER_LTF(xmag, SEGGER_DIV2F(ymag))) {
    *quo = 0;
    return x;
  } else if (SEGGER_EQF(xmag, ymag)) {
    *quo = 1;
    return __SEGGER_RTL_float32_signbit_copy(0.0, x);
  }
  //
  r = xmag;
  SEGGER_FREXPF(ymag, &ny);
  quotient = 0;
  parity   = 0;
  //
  while (SEGGER_GEF(r, ymag)) {
    SEGGER_FREXPF(r, &nr);
    n = nr - ny;
    ymagX = SEGGER_LDEXPF(ymag, n);
    if (SEGGER_GTF(ymagX, r)) {
      --n;
      ymagX = SEGGER_LDEXPF(ymag, n);
    }
    if (n == 0) {
      parity = ~parity;
    }
    quotient += __SEGGER_RTL_SAFE_LSL_U32(1, n);
    r = SEGGER_SUBF(r, ymagX);
  }
  //
  if (SEGGER_GTF(r, SEGGER_DIV2F(ymag))) {
    r = SEGGER_SUBF(r, ymag);
    parity    = ~parity;
    quotient += 1;
  }
  if (parity != 0 && (SEGGER_EQF(r, SEGGER_DIV2F(ymag)))) {
    r = SEGGER_NEGF(r);
  }
  //
  *quo = quotient;
  //
  return SEGGER_LT0F(x) ? SEGGER_NEGF(r) : r;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_remainder_inline()
*
*  Function description
*    Compute remainder after division, double.
*
*  Parameters
*    x   - Value #1.
*    y   - Value #2.
*    quo - Pointer to the object that receives the quotient of x
*          divided by y, modulo 2^(INT_MAX+1)
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result is in the range [-|y/2|, +|y/2|].  In the case that
*    x-iy = 1/2, we require that i be even.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_float64_remainder_inline(double x, double y, int *quo) {
  double   xmag;
  double   ymag;
  double   ymagX;
  double   r;
  int      n;
  int      nr;
  int      ny;
  unsigned parity;
  int      quotient;
  //
  if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(x) ||
                            __SEGGER_RTL_float64_isnan_inline(y) ||
                            __SEGGER_RTL_float64_putative_iszero(y))) {
    *quo = 0;
    return K_NAN_F64;
  }
  //
  xmag = SEGGER_FABS(x);
  ymag = SEGGER_FABS(y);
  //
  if (SEGGER_LT(xmag, SEGGER_DIV2(ymag))) {
    *quo = 0;
    return x;
  } else if (SEGGER_EQ(xmag, ymag)) {
    *quo = 1;
    return __SEGGER_RTL_float64_signbit_copy(0.0, x);
  }
  //
  r = xmag;
  SEGGER_FREXP(ymag, &ny);
  parity   = 0;
  quotient = 0;
  //
  while (SEGGER_GE(r, ymag)) {
    SEGGER_FREXP(r, &nr);
    n = nr - ny;
    ymagX = SEGGER_LDEXP(ymag, n);
    if (SEGGER_GT(ymagX, r)) {
      --n;
      ymagX = SEGGER_LDEXP(ymag, n);
    }
    if (n == 0) {
      parity = ~parity;
    }
    quotient += __SEGGER_RTL_SAFE_LSL_U32(1, n);
    r = SEGGER_SUB(r, ymagX);
  }
  //
  if (SEGGER_GT(r, SEGGER_DIV2(ymag))) {
    r = SEGGER_SUB(r, ymag);
    parity    = ~parity;
    quotient += 1;
  }
  if (parity != 0 && (SEGGER_EQ(r, SEGGER_DIV2(ymag)))) {
    r = SEGGER_NEG(r);
  }
  //
  *quo = quotient;
  //
  return SEGGER_LT0(x) ? SEGGER_NEG(r) : r;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_hypot_inline()
*
*  Function description
*    Compute magnitude of complex, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrt(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_hypot_inline(float x, float y) {
  if (__SEGGER_RTL_float32_isinf_inline(x) || __SEGGER_RTL_float32_isinf_inline(y)) {
    return K_INF_F32;
  } else {
    return SEGGER_SQRTF(SEGGER_ADDF(SEGGER_MULF(x, x), SEGGER_MULF(y, y)));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_hypot_outline()
*
*  Function description
*    Compute magnitude of complex, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrt(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_hypot_outline(float x, float y) {
  return __SEGGER_RTL_float32_hypot_inline(x, y);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_hypot_inline()
*
*  Function description
*    Compute magnitude of complex, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrt(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_hypot_inline(double x, double y) {
  if (__SEGGER_RTL_float64_isinf_inline(x) || __SEGGER_RTL_float64_isinf_inline(y)) {
    return K_INF_F64;
  }
  return SEGGER_SQRT(SEGGER_ADD(SEGGER_MUL(x, x), SEGGER_MUL(y, y)));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_hypot_outline()
*
*  Function description
*    Compute magnitude of complex, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrt(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_hypot_outline(double x, double y) {
  return __SEGGER_RTL_float64_hypot_inline(x, y);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_pow_inline()
*
*  Function description
*    Raise to power, float.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_float32_pow_inline(float x, float y) {
  int   iy;
  int   reciprocate;
  float r;
  //
  // +1 ** Any == +1.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_eq_bitwise(x, 1))) {
    return x;
  }
  //
  // x ** +0 == +1, even if x is a NaN.
  // x ** -0 == +1, even if x is a NaN.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_putative_iszero(y))) {
    return 1;
  }
  //
  // Handling of Inf, NaN, and zero inputs for either operand.
  // This is so messy.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isspecial(x) || __SEGGER_RTL_float32_isspecial(y))) {
    //
    // pow(x, NaN) == NaN.
    // pow(NaN, y) == NaN.
    //
    if (__SEGGER_RTL_float32_isnan_inline(x)) {
      return x;
    } else if (__SEGGER_RTL_float32_isnan_inline(y)) {
      return y;
    }
    //
    if (__SEGGER_RTL_float32_putative_iszero(x)) {
      //
      // -0 ** -Inf == +Inf
      // +0 ** -Inf == +Inf
      //
      // Check for exponent as positive integer.  Also handles the case for y = Inf
      // where y is not considered an integer by by way of the two conversions.
      //
      iy = SEGGER_F2I(y);
      if (__SEGGER_RTL_float32_eq_bitwise(y, SEGGER_I2F(iy))) {
        r = __SEGGER_RTL_float32_lt_rhs_positive(y, 0)
              ? K_INF_F32
              : 0;
        //
        return (iy & 1) ? __SEGGER_RTL_float32_signbit_xor(r, x) : r;
      }
    } else if (__SEGGER_RTL_BitcastToU32(x) == K_INF_U32) {
      //
      // +Inf ** -ve = +0
      // +Inf ** +ve = +Inf
      //
      return __SEGGER_RTL_float32_lt_rhs_positive(y, 0)
               ? 0
               : K_INF_F32;
      //
    } else if (__SEGGER_RTL_BitcastToU32(x) == (K_INF_U32 | FLOAT32_SIGN_MASK)) {
      //
      r = __SEGGER_RTL_float32_lt_rhs_positive(y, 0) ? 0 : K_INF_F32;
      iy = SEGGER_F2I(y);
      if (__SEGGER_RTL_float32_eq_bitwise(y, SEGGER_I2F(iy))) {
        if (iy & 1) {
          r = __SEGGER_RTL_float32_signbit_xor(r, x);
        }
      }
      return r;
      //
    } else if (__SEGGER_RTL_BitcastToU32(y) == (K_INF_U32 | FLOAT32_SIGN_MASK)) {
      //
      // -1 ** -Inf = +1.
      //  x ** -Inf = +Inf, |x| < 1.
      //  x ** -Inf = 0,    |x| >= 1.
      //
      if (__SEGGER_RTL_float32_eq_bitwise(x, -1)) {
        return +1;
      } else {
        return __SEGGER_RTL_float32_lt_rhs_positive(SEGGER_FABSF(x), 1)
                 ? K_INF_F32
                 : 0;
      }
    } else {  // x^+Inf is the only remaining case.
      //
      // x ** +Inf == +1  where   x == +1  ... this case already handled on entry by +1 ** Any = +1.
      //
      // Left with:
      //
      // x ** +Inf == +0    where  |x| <  1
      // x ** +Inf == +1    where   x == -1
      // x ** +Inf == -+Inf where  |x| >  1
      //
      if (__SEGGER_RTL_float32_lt_rhs_positive(SEGGER_FABSF(x), 1)) {
        return 0;
      } if (__SEGGER_RTL_float32_eq_bitwise(x, -1)) {
        return +1;
      } else {
        return K_INF_F32;
      }
    }
  }
  //
  // General case.
  //
  iy = SEGGER_F2I(y);
  if (__SEGGER_RTL_float32_eq_bitwise(y, SEGGER_I2F(iy))) {
    //
    // Binary expansion.
    //
    reciprocate = 0;
    if (iy < 0) {
      reciprocate = 1;
      iy = -iy;
    }
    y = 1;
    while (iy) {
      if (iy & 1) {
        y = SEGGER_MULF(y, x);
      }
      iy >>= 1;
      if (iy) {
        x = SEGGER_MULF(x, x);
      }
    }
    //
    return reciprocate ? SEGGER_DIVF(1, y) : y;
  } else if (SEGGER_LT0F(x)) {
    return K_NAN_F32;
  } else {
    return SEGGER_EXPF(SEGGER_MULF(y, SEGGER_LOGF(SEGGER_FABSF(x))));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_pow_outline()
*
*  Function description
*    Raise to power, float.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*/
static __SEGGER_RTL_NEVER_INLINE float __SEGGER_RTL_float32_pow_outline(float x, float y) {
  return __SEGGER_RTL_float32_pow_inline(x, y);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_pow_inline()
*
*  Function description
*    Raise to power, double.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_pow_inline(double x, double y) {
  int    iy;
  int    reciprocate;
  double r;
  //
  // +1 ** Any == +1.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_eq_bitwise(x, 1))) {
    return x;
  }
  //
  // x ** +0 == +1, even if x is a NaN.
  // x ** -0 == +1, even if x is a NaN.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_putative_iszero(y))) {
    return 1;
  }
  //
  // Handling of Inf, NaN, and zero inputs for either operand.
  // This is so messy.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isspecial(x) || __SEGGER_RTL_float64_isspecial(y))) {
    //
    // pow(x, NaN) == NaN.
    // pow(NaN, y) == NaN.
    //
    if (__SEGGER_RTL_float64_isnan_inline(x)) {
      return x;
    } else if (__SEGGER_RTL_float64_isnan_inline(y)) {
      return y;
    }
    //
    if (__SEGGER_RTL_float64_putative_iszero(x)) {
      //
      // -0 ** -Inf == +Inf
      // +0 ** -Inf == +Inf
      //
      // Check for exponent as positive integer.  Also handles the case for y = Inf
      // where y is not considered an integer by by way of the two conversions.
      //
      iy = SEGGER_D2I(y);
      if (__SEGGER_RTL_float64_eq_bitwise(y, SEGGER_I2D(iy))) {
        r = __SEGGER_RTL_float64_lt_rhs_positive(y, 0)
              ? K_INF_F64
              : 0;
        //
        return (iy & 1) ? __SEGGER_RTL_float64_signbit_xor(r, x) : r;
      }
    } else if (__SEGGER_RTL_BitcastToU64(x) == K_INF_U64) {
      //
      // +Inf ** -ve = +0
      // +Inf ** +ve = +Inf
      //
      return __SEGGER_RTL_float64_lt_rhs_positive(y, 0)
               ? 0
               : K_INF_F64;
      //
    } else if (__SEGGER_RTL_BitcastToU64(x) == (K_INF_U64 | FLOAT64_SIGN_MASK)) {
      //
      r = __SEGGER_RTL_float64_lt_rhs_positive(y, 0) ? 0 : K_INF_F64;
      iy = SEGGER_D2I(y);
      if (__SEGGER_RTL_float64_eq_bitwise(y, SEGGER_I2D(iy))) {
        if (iy & 1) {
          r = __SEGGER_RTL_float64_signbit_xor(r, x);
        }
      }
      return r;
    } else if (__SEGGER_RTL_BitcastToU64(y) == (K_INF_U64 | FLOAT64_SIGN_MASK)) {
      //
      // -1 ** -Inf = +1.
      // -1 ** -Inf = -1.
      //  x ** -Inf = +Inf, |x| < 1.
      //  x ** -Inf = 0,    |x| >= 1.
      //
      if (__SEGGER_RTL_float64_eq_bitwise(x, -1)) {
        return +1;
      } else {
        return __SEGGER_RTL_float64_lt_rhs_positive(SEGGER_FABS(x), 1)
                 ? K_INF_F64
                 : 0;
      }
    } else {  // x^+Inf is the only remaining case.
      //
      // x ** +Inf == +1  where   x == +1  ... this case already handled on entry by +1 ** Any = +1.
      //
      // Left with:
      //
      // x ** +Inf == +0    where  |x| <  1
      // x ** +Inf == +1    where   x == -1
      // x ** +Inf == -+Inf where  |x| >  1
      //
      if (__SEGGER_RTL_float64_lt_rhs_positive(SEGGER_FABS(x), 1)) {
        return 0;
      } if (__SEGGER_RTL_float64_eq_bitwise(x, -1)) {
        return +1;
      } else {
        return K_INF_F64;
      }
    }
  }
  //
  // General case.
  //
  iy = SEGGER_D2I(y);
  if (__SEGGER_RTL_float64_eq_bitwise(y, SEGGER_I2D(iy)) && (unsigned)iy != 0x80000000u) {
    //
    // Binary expansion.
    //
    reciprocate = 0;
    if (iy < 0) {
      reciprocate = 1;
      iy = -iy;
    }
    y = 1;
    while (iy) {
      if (iy & 1) {
        y = SEGGER_MUL(y, x);
      }
      iy = (unsigned)iy >> 1;
      if (iy) {
        x = SEGGER_MUL(x, x);
      }
    }
    //
    return reciprocate ? SEGGER_DIV(1, y) : y;
  } else if (SEGGER_LT0(x)) {
    return K_NAN_F64;
  } else {
    return SEGGER_EXP(SEGGER_MUL(y, SEGGER_LOG(SEGGER_FABS(x))));
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_pow_outline()
*
*  Function description
*    Raise to power, double.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*/
static __SEGGER_RTL_NEVER_INLINE double __SEGGER_RTL_float64_pow_outline(double x, double y) {
  return __SEGGER_RTL_float64_pow_inline(x, y);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_tgamma_inline()
*
*  Function description
*    Gamma function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Gamma(x).
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_tgamma_inline(float x) {
  float v;
  //
  v = SEGGER_MULF(12, x) - SEGGER_DIVF(1, SEGGER_MULF(10, x));
  v = SEGGER_ADDF(x, SEGGER_DIVF(1, v));
  v = SEGGER_DIVF(v, (float)M_E);
  v = SEGGER_POWF(v, x);
  v = SEGGER_MULF(v, SEGGER_SQRTF(SEGGER_DIVF((float)(2*M_PI), x)));
  //
  return v;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_tgamma_inline()
*
*  Function description
*    Gamma function, double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Gamma(x).
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_tgamma_inline(double x) {
  double v;
  //
  v = SEGGER_MUL(12, x) - SEGGER_DIV(1, SEGGER_MUL(10, x));
  v = SEGGER_ADD(x, SEGGER_DIV(1, v));
  v = SEGGER_DIV(v, M_E);
  v = SEGGER_POW(v, x);
  v = SEGGER_MUL(v, SEGGER_SQRT(SEGGER_DIV(2*M_PI, x)));
  //
  return v;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_lgamma_inline()
*
*  Function description
*    Log-Gamma function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    log(gamma(x)).
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_lgamma_inline(float x) {
  float v;
  //
  v = SEGGER_MULF(12, x) - SEGGER_DIVF(1, SEGGER_MULF(10, x));
  v = SEGGER_ADDF(x, SEGGER_DIVF(1, v));
  v = SEGGER_SUBF(SEGGER_LOGF(v), 1);
  v = SEGGER_MULF(v, x);
  v = SEGGER_ADDF(v, SEGGER_DIV2F((float)M_LOG_2PI - SEGGER_LOGF(x)));
  //
  return v;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_lgamma_inline()
*
*  Function description
*    Log-Gamma function, double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    log(gamma(x)).
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_lgamma_inline(double x) {
  double v;
  //
  v = SEGGER_MUL(12, x) - SEGGER_DIV(1, SEGGER_MUL(10, x));
  v = SEGGER_ADD(x, SEGGER_DIV(1, v));
  v = SEGGER_SUB(SEGGER_LOG(v), 1);
  v = SEGGER_MUL(v, x);
  v = SEGGER_ADD(v, SEGGER_DIV2((float)M_LOG_2PI - SEGGER_LOG(x)));
  //
  return v;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_erf_inline()
*
*  Function description
*    Error function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erf(x).
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_erf_inline(float x) {
  const float a = 0.147f;
  float ax2;
  float x2;
  float v;
  //
  x2  = SEGGER_MULF(x, x);
  ax2 = SEGGER_MULF(a, x2);
  v   = SEGGER_DIVF(SEGGER_ADDF((float)(4/M_PI), ax2), SEGGER_ADDF(1, ax2));
  v   = SEGGER_NEGF(SEGGER_MULF(x2, v));
  v   = SEGGER_SQRTF(SEGGER_SUBF(1, SEGGER_EXPF(v)));
  //
  return __SEGGER_RTL_float32_signbit_xor(v, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_erf_inline()
*
*  Function description
*    Error function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erf(x).
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_erf_inline(double x) {
  const double a = 0.147;
  double ax2;
  double x2;
  double v;
  //
  x2  = SEGGER_MUL(x, x);
  ax2 = SEGGER_MUL(a, x2);
  v   = SEGGER_DIV(SEGGER_ADD(4/M_PI, ax2), SEGGER_ADD(1, ax2));
  v   = SEGGER_NEG(SEGGER_MUL(x2, v));
  v   = SEGGER_SQRT(SEGGER_SUB(1, SEGGER_EXP(v)));
  //
  return __SEGGER_RTL_float64_signbit_xor(v, x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_erfc_inline()
*
*  Function description
*    Complementary error function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erfc(x).
*/
static __SEGGER_RTL_INLINE float __SEGGER_RTL_float32_erfc_inline(float x) {
  return SEGGER_DIVF(
           SEGGER_MULF(SEGGER_SUBF(1, SEGGER_EXPF(SEGGER_MULF(-1.98f, x))),
                       SEGGER_EXPF(SEGGER_NEGF(SEGGER_MULF(x, x)))),
           SEGGER_MULF((float)(1.135 * M_SQRT_PI), x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_erfc_inline()
*
*  Function description
*    Complementary error function, double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erfc(x).
*/
static __SEGGER_RTL_INLINE double __SEGGER_RTL_float64_erfc_inline(double x) {
  return SEGGER_DIV(
           SEGGER_MUL(SEGGER_SUB(1, SEGGER_EXP(SEGGER_MUL(-1.98, x))),
                       SEGGER_EXP(SEGGER_NEG(SEGGER_MUL(x, x)))),
           SEGGER_MUL(1.135 * M_SQRT_PI, x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cmplx_inline()
*
*  Function description
*    Construct, float complex.
*
*  Parameters
*    re - Real part of complex.
*    im - Imaginary part of complex.
*
*  Return value
*    The complex value re + im.I.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_float32_cmplx_inline(float re, float im) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.part.Re = re;
  z.u.part.Im = im;
  //
  return z.u.value;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cmplx_inline()
*
*  Function description
*    Construct, double complex.
*
*  Parameters
*    re - Real part of complex.
*    im - Imaginary part of complex.
*
*  Return value
*    The complex value re + im.I.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_float64_cmplx_inline(double re, double im) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.part.Re = re;
  z.u.part.Im = im;
  //
  return z.u.value;
}

/*********************************************************************
*
*       __SEGGER_RTL_ldouble_cmplx_inline()
*
*  Function description
*    Construct, long double complex.
*
*  Parameters
*    re - Real part of complex.
*    im - Imaginary part of complex.
*
*  Return value
*    The complex value re + im.I.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_ldouble_cmplx_inline(double re, double im) {
  __SEGGER_RTL_LDOUBLE_COMPLEX z;
  //
  z.u.part.Re = re;
  z.u.part.Im = im;
  //
  return z.u.value;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cadd()
*
*  Function description
*    Add, float complex.
*
*  Parameters
*    pX - Pointer to summand #1.
*    pY - Pointer to summand #2.
*
*  Additional information
*    Sets the object pointed to by pResult to the complex sum of
*    the objects pointed to by pX and pY.
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_cadd(__SEGGER_RTL_FLOAT32_COMPLEX *pX, const __SEGGER_RTL_FLOAT32_COMPLEX *pY) {
  pX->u.part.Re = SEGGER_ADDF(pX->u.part.Re, pY->u.part.Re);
  pX->u.part.Im = SEGGER_ADDF(pX->u.part.Im, pY->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cadd()
*
*  Function description
*    Add, double complex.
*
*  Parameters
*    pX - Pointer to summand #1.
*    pY - Pointer to summand #2.
*
*  Additional information
*    Sets the object pointed to by pResult to the complex sum of
*    the objects pointed to by pX and pY.
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_cadd(__SEGGER_RTL_FLOAT64_COMPLEX *pX, const __SEGGER_RTL_FLOAT64_COMPLEX *pY) {
  pX->u.part.Re = SEGGER_ADD(pX->u.part.Re, pY->u.part.Re);
  pX->u.part.Im = SEGGER_ADD(pX->u.part.Im, pY->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_csub()
*
*  Function description
*    Subtract, double complex.
*
*  Parameters
*    pX - Pointer to minuend.
*    pY - Pointer to subtrahend.
*
*  Additional information
*    Sets the object pointed to by pResult to the complex difference
*    between the objects pointed to by pX and pY.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_csub(__SEGGER_RTL_FLOAT32_COMPLEX *pX, const __SEGGER_RTL_FLOAT32_COMPLEX *pY) {
  pX->u.part.Re = SEGGER_SUBF(pX->u.part.Re, pY->u.part.Re);
  pX->u.part.Im = SEGGER_SUBF(pX->u.part.Im, pY->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_csub()
*
*  Function description
*    Subtract, double complex.
*
*  Parameters
*    pX - Pointer to minuend.
*    pY - Pointer to subtrahend.
*
*  Additional information
*    Sets the object pointed to by pResult to the complex difference
*    between the objects pointed to by pX and pY.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_csub(__SEGGER_RTL_FLOAT64_COMPLEX *pX, const __SEGGER_RTL_FLOAT64_COMPLEX *pY) {
  pX->u.part.Re = SEGGER_SUB(pX->u.part.Re, pY->u.part.Re);
  pX->u.part.Im = SEGGER_SUB(pX->u.part.Im, pY->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cmul()
*
*  Function description
*    Multiply, double complex.
*
*  Parameters
*    pX - Pointer to multiplicand and product.
*    pY - Pointer to multiplier.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cmul(__SEGGER_RTL_FLOAT32_COMPLEX *pX, const __SEGGER_RTL_FLOAT32_COMPLEX *pY) {
  float a;
  float b;
  float c;
  float d;
  float Zre;
  float Zim;
  float ac;
  float bd;
  float ad;
  float bc;
  int   Recalc;
  //
  a = pX->u.part.Re;
  b = pX->u.part.Im;
  c = pY->u.part.Re;
  d = pY->u.part.Im;
  //
  ac = SEGGER_MULF(a, c);
  bd = SEGGER_MULF(b, d);
  ad = SEGGER_MULF(a, d);
  bc = SEGGER_MULF(b, c);
  //
  Zre = SEGGER_SUBF(ac, bd);
  Zim = SEGGER_ADDF(ad, bc);
  //
  if (__SEGGER_RTL_float32_isnan_inline(Zre) && __SEGGER_RTL_float32_isnan_inline(Zim)) {
    //
    Recalc = 0;
    //
    if (__SEGGER_RTL_float32_isinf_inline(a) || __SEGGER_RTL_float32_isinf_inline(b)) {
      a = __SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_float32_isinf_inline(a) ? 1.0f : 0.0f, a);
      b = __SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_float32_isinf_inline(b) ? 1.0f : 0.0f, b);
      if (__SEGGER_RTL_float32_isnan_inline(c)) { c = __SEGGER_RTL_float32_signbit_copy(0, c); }
      if (__SEGGER_RTL_float32_isnan_inline(d)) { d = __SEGGER_RTL_float32_signbit_copy(0, d); }
      //
      Recalc = 1;
    }
    //
    if (__SEGGER_RTL_float32_isinf_inline(c) || __SEGGER_RTL_float32_isinf_inline(d)) {
      c = __SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_float32_isinf_inline(c) ? 1.0f : 0.0f, c);
      d = __SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_float32_isinf_inline(d) ? 1.0f : 0.0f, d);
      if (__SEGGER_RTL_float32_isnan_inline(a)) { a = __SEGGER_RTL_float32_signbit_copy(0, a); }
      if (__SEGGER_RTL_float32_isnan_inline(b)) { b = __SEGGER_RTL_float32_signbit_copy(0, b); }
      //
      Recalc = 1;
    }
    //
    if (!Recalc && (__SEGGER_RTL_float32_isinf_inline(ac) || __SEGGER_RTL_float32_isinf_inline(bd) || __SEGGER_RTL_float32_isinf_inline(ad) || __SEGGER_RTL_float32_isinf_inline(bc))) {
      if (__SEGGER_RTL_float32_isnan_inline(a)) { a = __SEGGER_RTL_float32_signbit_copy(0.0f, a); }
      if (__SEGGER_RTL_float32_isnan_inline(b)) { b = __SEGGER_RTL_float32_signbit_copy(0.0f, b); }
      if (__SEGGER_RTL_float32_isnan_inline(c)) { c = __SEGGER_RTL_float32_signbit_copy(0.0f, c); }
      if (__SEGGER_RTL_float32_isnan_inline(d)) { d = __SEGGER_RTL_float32_signbit_copy(0.0f, d); }
      //
      Recalc = 1;
    }
    //
    if (Recalc) {
      Zre = SEGGER_MULF(K_INF_F32, SEGGER_SUBF(SEGGER_MULF(a, c), SEGGER_MULF(b, d)));
      Zim = SEGGER_MULF(K_INF_F32, SEGGER_ADDF(SEGGER_MULF(a, d), SEGGER_MULF(b, c)));
    }
  }
  //
  pX->u.part.Re = Zre;
  pX->u.part.Im = Zim;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cmul()
*
*  Function description
*    Multiply, double complex.
*
*  Parameters
*    pX - Pointer to multiplicand and product.
*    pY - Pointer to multiplier.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cmul(__SEGGER_RTL_FLOAT64_COMPLEX *pX, const __SEGGER_RTL_FLOAT64_COMPLEX *pY) {
  double a;
  double b;
  double c;
  double d;
  double Zre;
  double Zim;
  double ac;
  double bd;
  double ad;
  double bc;
  int    Recalc;
  //
  a = pX->u.part.Re;
  b = pX->u.part.Im;
  c = pY->u.part.Re;
  d = pY->u.part.Im;
  //
  ac = SEGGER_MUL(a, c);
  bd = SEGGER_MUL(b, d);
  ad = SEGGER_MUL(a, d);
  bc = SEGGER_MUL(b, c);
  //
  Zre = SEGGER_SUB(ac, bd);
  Zim = SEGGER_ADD(ad, bc);
  //
  if (__SEGGER_RTL_float64_isnan_inline(Zre) && __SEGGER_RTL_float64_isnan_inline(Zim)) {
    //
    Recalc = 0;
    //
    if (__SEGGER_RTL_float64_isinf_inline(a) || __SEGGER_RTL_float64_isinf_inline(b)) {
      a = __SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_float64_isinf_inline(a) ? 1.0 : 0.0, a);
      b = __SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_float64_isinf_inline(b) ? 1.0 : 0.0, b);
      if (__SEGGER_RTL_float64_isnan_inline(c)) { c = __SEGGER_RTL_float64_signbit_copy(0, c); }
      if (__SEGGER_RTL_float64_isnan_inline(d)) { d = __SEGGER_RTL_float64_signbit_copy(0, d); }
      //
      Recalc = 1;
    }
    //
    if (__SEGGER_RTL_float64_isinf_inline(c) || __SEGGER_RTL_float64_isinf_inline(d)) {
      c = __SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_float64_isinf_inline(c) ? 1.0 : 0.0, c);
      d = __SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_float64_isinf_inline(d) ? 1.0 : 0.0, d);
      if (__SEGGER_RTL_float64_isnan_inline(a)) { a = __SEGGER_RTL_float64_signbit_copy(0, a); }
      if (__SEGGER_RTL_float64_isnan_inline(b)) { b = __SEGGER_RTL_float64_signbit_copy(0, b); }
      //
      Recalc = 1;
    }
    //
    if (!Recalc && (__SEGGER_RTL_float64_isinf_inline(ac) || __SEGGER_RTL_float64_isinf_inline(bd) || __SEGGER_RTL_float64_isinf_inline(ad) || __SEGGER_RTL_float64_isinf_inline(bc))) {
      if (__SEGGER_RTL_float64_isnan_inline(a)) { a = __SEGGER_RTL_float64_signbit_copy(0.0, a); }
      if (__SEGGER_RTL_float64_isnan_inline(b)) { b = __SEGGER_RTL_float64_signbit_copy(0.0, b); }
      if (__SEGGER_RTL_float64_isnan_inline(c)) { c = __SEGGER_RTL_float64_signbit_copy(0.0, c); }
      if (__SEGGER_RTL_float64_isnan_inline(d)) { d = __SEGGER_RTL_float64_signbit_copy(0.0, d); }
      //
      Recalc = 1;
    }
    //
    if (Recalc) {
      Zre = SEGGER_MUL(K_INF_F64, SEGGER_SUB(SEGGER_MUL(a, c), SEGGER_MUL(b, d)));
      Zim = SEGGER_MUL(K_INF_F64, SEGGER_ADD(SEGGER_MUL(a, d), SEGGER_MUL(b, c)));
    }
  }
  //
  pX->u.part.Re = Zre;
  pX->u.part.Im = Zim;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cdiv()
*
*  Function description
*    Divide, float complex.
*
*  Parameters
*    pX - Pointer to dividend and quotient.
*    pY - Pointer to divisor.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cdiv(__SEGGER_RTL_FLOAT32_COMPLEX *pX, const __SEGGER_RTL_FLOAT32_COMPLEX *pY) {
  float a;
  float b;
  float c;
  float d;
  float denom;
  float x;
  float y;
  int   ilogbw;
  //
  a = pX->u.part.Re;
  b = pX->u.part.Im;
  c = pY->u.part.Re;
  d = pY->u.part.Im;
  //
  ilogbw = SEGGER_ILOGBF(__SEGGER_RTL_float32_fmax_inline(SEGGER_FABSF(c), SEGGER_FABSF(d)));
  if (0 <= ilogbw && ilogbw < 1024) {
    c = SEGGER_LDEXPF(c, -ilogbw - 1);
    d = SEGGER_LDEXPF(d, -ilogbw - 1);
  } else {
    ilogbw = 0;
  }
  //
  denom = SEGGER_ADDF(SEGGER_MULF(c, c), SEGGER_MULF(d, d));
  x = SEGGER_LDEXPF(SEGGER_DIVF(SEGGER_ADDF(SEGGER_MULF(a, c), SEGGER_MULF(b, d)), denom), -ilogbw - 1);
  y = SEGGER_LDEXPF(SEGGER_DIVF(SEGGER_SUBF(SEGGER_MULF(b, c), SEGGER_MULF(a, d)), denom), -ilogbw - 1);
  //
  // Recover infinities and zeros that computed as NaN+iNaN;
  // the only cases are nonzero/zero, infinite/finite, and finite/infinite, ...
  //
  if (__SEGGER_RTL_float32_isnan_inline(x) && __SEGGER_RTL_float32_isnan_inline(y)) {
    if (SEGGER_EQ0F(denom) && (!__SEGGER_RTL_float32_isnan_inline(a) || !__SEGGER_RTL_float32_isnan_inline(b))) {
      x = SEGGER_MULF(__SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_INFINITY, c), a);
      y = SEGGER_MULF(__SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_INFINITY, c), b);
    } else if ((__SEGGER_RTL_float32_isinf_inline(a) || __SEGGER_RTL_float32_isinf_inline(b)) && __SEGGER_RTL_float32_isfinite_inline(c) && __SEGGER_RTL_float32_isfinite_inline(d)) {
      a = __SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_float32_isinf_inline(a) ? 1.0f : 0.0f, a);
      b = __SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_float32_isinf_inline(b) ? 1.0f : 0.0f, b);
      x = SEGGER_MULF(__SEGGER_RTL_INFINITY, SEGGER_ADDF(SEGGER_MULF(a, c), SEGGER_MULF(b, d)));
      y = SEGGER_MULF(__SEGGER_RTL_INFINITY, SEGGER_SUBF(SEGGER_MULF(b, c), SEGGER_MULF(a, d)));
    } else if ((ilogbw == K_INT_MAX) && __SEGGER_RTL_float32_isfinite_inline(a) && __SEGGER_RTL_float32_isfinite_inline(b)) {
      c = __SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_float32_isinf_inline(c) ? 1.0f : 0.0f, c);
      d = __SEGGER_RTL_float32_signbit_copy(__SEGGER_RTL_float32_isinf_inline(d) ? 1.0f : 0.0f, d);
      x = SEGGER_MULF(0.0f, SEGGER_MULF(a, c) + SEGGER_MULF(b, d));
      y = SEGGER_MULF(0.0f, SEGGER_MULF(b, c) - SEGGER_MULF(a, d));
    }
  }
  //
  pX->u.part.Re = x;
  pX->u.part.Im = y;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cdiv_inline()
*
*  Function description
*    Divide, double complex.
*
*  Parameters
*    pX - Pointer to dividend and quotient.
*    pY - Pointer to divisor.
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_cdiv_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX, const __SEGGER_RTL_FLOAT64_COMPLEX *pY) {
  double a;
  double b;
  double c;
  double d;
  double denom;
  double x;
  double y;
  int    ilogbw;
  //
  a = pX->u.part.Re;
  b = pX->u.part.Im;
  c = pY->u.part.Re;
  d = pY->u.part.Im;
  //
  ilogbw = SEGGER_ILOGB(__SEGGER_RTL_float64_fmax_inline(SEGGER_FABS(c), SEGGER_FABS(d)));
  if (0 <= ilogbw && ilogbw < 1024) {
    c = SEGGER_LDEXP(c, -ilogbw - 1);
    d = SEGGER_LDEXP(d, -ilogbw - 1);
  } else {
    ilogbw = 0;
  }
  //
  denom = SEGGER_ADD(SEGGER_MUL(c, c), SEGGER_MUL(d, d));
  x = SEGGER_LDEXP(SEGGER_DIV(SEGGER_ADD(SEGGER_MUL(a, c), SEGGER_MUL(b, d)), denom), -ilogbw - 1);
  y = SEGGER_LDEXP(SEGGER_DIV(SEGGER_SUB(SEGGER_MUL(b, c), SEGGER_MUL(a, d)), denom), -ilogbw - 1);
  //
  // Recover infinities and zeros that computed as NaN+iNaN;
  // the only cases are nonzero/zero, infinite/finite, and finite/infinite, ...
  //
  if (__SEGGER_RTL_float64_isnan_inline(x) && __SEGGER_RTL_float64_isnan_inline(y)) {
    if (SEGGER_EQ0(denom) && (!__SEGGER_RTL_float64_isnan_inline(a) || !__SEGGER_RTL_float64_isnan_inline(b))) {
      x = SEGGER_MUL(__SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_INFINITY, c), a);
      y = SEGGER_MUL(__SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_INFINITY, c), b);
    } else if ((__SEGGER_RTL_float64_isinf_inline(a) || __SEGGER_RTL_float64_isinf_inline(b)) && __SEGGER_RTL_float64_isfinite_inline(c) && __SEGGER_RTL_float64_isfinite_inline(d)) {
      a = __SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_float64_isinf_inline(a) ? 1.0 : 0.0, a);
      b = __SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_float64_isinf_inline(b) ? 1.0 : 0.0, b);
      x = SEGGER_MUL(__SEGGER_RTL_INFINITY, SEGGER_ADD(SEGGER_MUL(a, c), SEGGER_MUL(b, d)));
      y = SEGGER_MUL(__SEGGER_RTL_INFINITY, SEGGER_SUB(SEGGER_MUL(b, c), SEGGER_MUL(a, d)));
    } else if ((ilogbw == K_INT_MAX) && __SEGGER_RTL_float64_isfinite_inline(a) && __SEGGER_RTL_float64_isfinite_inline(b)) {
      c = __SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_float64_isinf_inline(c) ? 1.0 : 0.0, c);
      d = __SEGGER_RTL_float64_signbit_copy(__SEGGER_RTL_float64_isinf_inline(d) ? 1.0 : 0.0, d);
      x = SEGGER_MUL(0.0, SEGGER_MUL(a, c) + SEGGER_MUL(b, d));
      y = SEGGER_MUL(0.0, SEGGER_MUL(b, c) - SEGGER_MUL(a, d));
    }
  }
  //
  pX->u.part.Re = x;
  pX->u.part.Im = y;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cdiv_outline()
*
*  Function description
*    Divide, double complex.
*
*  Parameters
*    pX - Pointer to dividend and quotient.
*    pY - Pointer to divisor.
*/
static void __SEGGER_RTL_NEVER_INLINE __SEGGER_RTL_float64_cdiv_outline(__SEGGER_RTL_FLOAT64_COMPLEX *pX, const __SEGGER_RTL_FLOAT64_COMPLEX *pY) {
  __SEGGER_RTL_float64_cdiv_inline(pX, pY);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cdiv2()
*
*  Function description
*    Divide by two, double complex.
*
*  Parameters
*    pX - Pointer to object to halve.
*
*  Additional information
*    Divides the complex object pointed to by pX by two.  The
*    real and imaginary components are both halved.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cdiv2(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  pX->u.part.Re = SEGGER_DIV2F(pX->u.part.Re);
  pX->u.part.Im = SEGGER_DIV2F(pX->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cdiv2()
*
*  Function description
*    Divide by two, double complex.
*
*  Parameters
*    pX - Pointer to object to halve.
*
*  Additional information
*    Divides the complex object pointed to by pX by two.  The
*    real and imaginary components are both halved.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cdiv2(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  pX->u.part.Re = SEGGER_DIV2(pX->u.part.Re);
  pX->u.part.Im = SEGGER_DIV2(pX->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cmuli()
*
*  Function description
*    Multiply by i, float complex.
*
*  Parameters
*    pX - Pointer to object to multiply by i.
*
*  Additional information
*    Multiplication by i is achieved by constructing a new value
*    with the imaginary part negated and then real and imaginary
*    parts swapped.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cmuli(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  float t;
  //
  t = pX->u.part.Re;
  pX->u.part.Re = pX->u.part.Im;
  pX->u.part.Im = SEGGER_NEGF(t);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cmuli()
*
*  Function description
*    Multiply by i, double complex.
*
*  Parameters
*    pX - Pointer to object to multiply by i.
*
*  Additional information
*    Multiplication by i is achieved by constructing a new value
*    with the imaginary part negated and then real and imaginary
*    parts swapped.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cmuli(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  double t;
  //
  t = pX->u.part.Re;
  pX->u.part.Re = pX->u.part.Im;
  pX->u.part.Im = SEGGER_NEG(t);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cdivi()
*
*  Function description
*    Divide by i, double complex.
*
*  Parameters
*    pX - Pointer to object to divide by i.
*
*  Additional information
*    Division by i is achieved by constructing a new value
*    with the imaginary part negated and then real and imaginary
*    parts swapped.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cdivi(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  float t;
  //
  t = pX->u.part.Re;
  pX->u.part.Re = SEGGER_NEGF(pX->u.part.Im);
  pX->u.part.Im = t;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cdivi()
*
*  Function description
*    Divide by i, double complex.
*
*  Parameters
*    pX - Pointer to object to divide by i.
*
*  Additional information
*    Division by i is achieved by constructing a new value
*    with the imaginary part negated and then real and imaginary
*    parts swapped.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cdivi(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  double t;
  //
  t = pX->u.part.Re;
  pX->u.part.Re = SEGGER_NEG(pX->u.part.Im);
  pX->u.part.Im = t;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cneg()
*
*  Function description
*    Negate, double complex.
*
*  Parameters
*    pX - Pointer to object to negate.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cneg(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  pX->u.part.Re = SEGGER_NEGF(pX->u.part.Re);
  pX->u.part.Im = SEGGER_NEGF(pX->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cneg()
*
*  Function description
*    Negate, double complex.
*
*  Parameters
*    pX - Pointer to object to negate.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cneg(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  pX->u.part.Re = SEGGER_NEG(pX->u.part.Re);
  pX->u.part.Im = SEGGER_NEG(pX->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ccnj()
*
*  Function description
*    Conjugate, double complex.
*
*  Parameters
*    pX - Pointer to object to conjugate.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_ccnj(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  pX->u.part.Im = SEGGER_NEGF(pX->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ccnj()
*
*  Function description
*    Conjugate, double complex.
*
*  Parameters
*    pX - Pointer to object to conjugate.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_ccnj(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  pX->u.part.Im = SEGGER_NEG(pX->u.part.Im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cproj()
*
*  Function description
*    Project point, float complex.
*
*  Parameters
*    pX - Pointer to value to project.
*
*  Additional information
*    Project the object pointed to by pX onto the Riemann sphere.
*
*    z projects to z, except that all complex infinities (even those with
*    one infinite part and one NaN part) project to positive infinity on
*    the real axis. If z has an infinite part, then cproj(z) is be
*    equivalent to:
*
*    * INFINITY + I * copysign(0.0, cimag(z))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cproj(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  if (__SEGGER_RTL_float32_isinf_inline(pX->u.part.Re) || __SEGGER_RTL_float32_isinf_inline(pX->u.part.Im)) {
    pX->u.part.Re = K_INF_F32;
    pX->u.part.Im = __SEGGER_RTL_float32_signbit_copy(0.0f, pX->u.part.Im);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cproj()
*
*  Function description
*    Project point, double complex.
*
*  Parameters
*    pX - Pointer to value to project.
*
*  Additional information
*    Project the object pointed to by pX onto the Riemann sphere.
*
*    z projects to z, except that all complex infinities (even those with
*    one infinite part and one NaN part) project to positive infinity on
*    the real axis. If z has an infinite part, then cproj(z) is be
*    equivalent to:
*
*    * INFINITY + I * copysign(0.0, cimag(z))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cproj(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  if (__SEGGER_RTL_float64_isinf_inline(pX->u.part.Re) || __SEGGER_RTL_float64_isinf_inline(pX->u.part.Im)) {
    pX->u.part.Re = K_INF_F64;
    pX->u.part.Im = __SEGGER_RTL_float64_signbit_copy(0.0, pX->u.part.Im);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_csqrt()
*
*  Function description
*    Square root, double complex.
*
*  Parameters
*    pX - Pointer to object to compute complex square root of.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_csqrt(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  float Re;
  float Im;
  float r;
  float s;
  //
  Re = SEGGER_FABSF(pX->u.part.Re);
  Im = SEGGER_FABSF(pX->u.part.Im);
  //
  if (SEGGER_GTF(Re, Im)) {
    r  = SEGGER_DIVF(Im, Re);
    s  = SEGGER_ADDF(SEGGER_SQRTF(SEGGER_ADDF(1, SEGGER_MULF(r, r))), 1);
    Re = SEGGER_SQRTF(SEGGER_LDEXPF(SEGGER_MULF(Re, s), -1));
    Im = SEGGER_SQRTF(SEGGER_DIVF(SEGGER_MULF(Im, r), SEGGER_LDEXPF(s, 1)));
  } else if (SEGGER_EQ0F(Im)) {
    Re = SEGGER_SQRTF(Re);
  } else {
    r  = SEGGER_DIVF(Re, Im);
    s  = SEGGER_ADDF(SEGGER_SQRTF(SEGGER_ADDF(1, SEGGER_MULF(r, r))), r);
    Re = SEGGER_SQRTF(SEGGER_LDEXPF(SEGGER_MULF(Im, s), -1));
    Im = SEGGER_SQRTF(SEGGER_DIVF(Im, SEGGER_LDEXPF(s, 1)));
  }
  //
  if (__SEGGER_RTL_float32_signbit_inline(pX->u.part.Re)) {
    r  = Re;
    Re = Im;
    Im = r;
  }
  if (__SEGGER_RTL_float32_signbit_inline(pX->u.part.Im)) {
    Im = SEGGER_NEGF(Im);
  }
  //
  // Take care of exceptions.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(Im))) {
    //
    if (__SEGGER_RTL_float32_isinf_inline(pX->u.part.Im)) {
      //
      // csqrt(x   +/- Inf.i) = +Inf +/- Inf.i
      // csqrt(NaN +/- Inf.i) = +Inf +/- Inf.i
      //
      Re = __SEGGER_RTL_BitcastToF32(K_INF_U32);
      Im = pX->u.part.Im;
      //
    } else if (__SEGGER_RTL_float32_isposinf_inline(pX->u.part.Re)) {
      //
      // csqrt(+Inf + NaN.i) = +/-Inf.i + NaN
      //
      Re = __SEGGER_RTL_BitcastToF32(K_INF_U32);  // Also pX->u.part.Re would do.
      //
    } else if (__SEGGER_RTL_float32_isneginf_inline(pX->u.part.Re)) {
      //
      // csqrt(-Inf + NaN.i) = NaN + +/-Inf.i
      //
      Im = __SEGGER_RTL_BitcastToF32(K_INF_U32);  // Also pX->u.part.Re would do.
      //
    } else if (__SEGGER_RTL_float32_isnan_inline(pX->u.part.Re)) {
      //
      // csqrt(NaN + 0.i) = NaN + NaN.i for the case where input has real part -NaN
      //
      Re = __SEGGER_RTL_BitcastToF32(K_NAN_U32);
      Im = __SEGGER_RTL_BitcastToF32(K_NAN_U32);
    }
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(pX->u.part.Re))) {
    //
    // csqrt(NaN + 0.i) = NaN + NaN.i for the case where input has real part +NaN
    //
    Im = __SEGGER_RTL_BitcastToF32(K_NAN_U32);
  }
  //
  pX->u.part.Re = Re;
  pX->u.part.Im = Im;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_csqrt()
*
*  Function description
*    Square root, double complex.
*
*  Parameters
*    pX - Pointer to object to compute complex square root of.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_csqrt(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  double Re;
  double Im;
  double r;
  double s;
  //
  Re = SEGGER_FABS(pX->u.part.Re);
  Im = SEGGER_FABS(pX->u.part.Im);
  //
  if (SEGGER_GT(Re, Im)) {
    r  = SEGGER_DIV(Im, Re);
    s  = SEGGER_ADD(SEGGER_SQRT(SEGGER_ADD(1, SEGGER_MUL(r, r))), 1);
    Re = SEGGER_SQRT(SEGGER_LDEXP(SEGGER_MUL(Re, s), -1));
    Im = SEGGER_SQRT(SEGGER_DIV(SEGGER_MUL(Im, r), SEGGER_LDEXP(s, 1)));
  } else if (SEGGER_EQ0(Im)) {
    Re = SEGGER_SQRT(Re);
  } else {
    r  = SEGGER_DIV(Re, Im);
    s  = SEGGER_ADD(SEGGER_SQRT(SEGGER_ADD(1, SEGGER_MUL(r, r))), r);
    Re = SEGGER_SQRT(SEGGER_LDEXP(SEGGER_MUL(Im, s), -1));
    Im = SEGGER_SQRT(SEGGER_DIV(Im, SEGGER_LDEXP(s, 1)));
  }
  //
  if (__SEGGER_RTL_float64_signbit_inline(pX->u.part.Re)) {
    r  = Re;
    Re = Im;
    Im = r;
  }
  if (__SEGGER_RTL_float64_signbit_inline(pX->u.part.Im)) {
    Im = SEGGER_NEG(Im);
  }
  //
  // Take care of exceptions.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(Im))) {
    //
    if (__SEGGER_RTL_float64_isinf_inline(pX->u.part.Im)) {
      //
      // csqrt(x   +/- Inf.i) = +Inf +/- Inf.i
      // csqrt(NaN +/- Inf.i) = +Inf +/- Inf.i
      //
      Re = __SEGGER_RTL_BitcastToF64(K_INF_U64);
      Im = pX->u.part.Im;
      //
    } else if (__SEGGER_RTL_float64_isposinf_inline(pX->u.part.Re)) {
      //
      // csqrt(+Inf + NaN.i) = +/-Inf.i + NaN
      //
      Re = __SEGGER_RTL_BitcastToF64(K_INF_U64);  // Also pX->u.part.Re would do.
      //
    } else if (__SEGGER_RTL_float64_isneginf_inline(pX->u.part.Re)) {
      //
      // csqrt(-Inf + NaN.i) = NaN + +/-Inf.i
      //
      Im = __SEGGER_RTL_BitcastToF64(K_INF_U64);  // Also pX->u.part.Re would do.
      //
    } else if (__SEGGER_RTL_float64_isnan_inline(pX->u.part.Re)) {
      //
      // csqrt(NaN + 0.i) = NaN + NaN.i for the case where input has real part -NaN
      //
      Re = __SEGGER_RTL_BitcastToF64(K_NAN_U64);
      Im = __SEGGER_RTL_BitcastToF64(K_NAN_U64);
    }
  } else if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(pX->u.part.Re))) {
    //
    // csqrt(NaN + 0.i) = NaN + NaN.i for the case where input has real part +NaN
    //
    Im = __SEGGER_RTL_BitcastToF64(K_NAN_U64);
  }
  //
  pX->u.part.Re = Re;
  pX->u.part.Im = Im;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_clog_inline()
*
*  Function description
*    Compute natural logarithm, float complex.
*
*  Parameters
*    pX - Pointer to value to compute logarithm of.
*
*  Additional information
*    The natural logarithm of x is calculated according to the following table:
*
*    +-------------------+-------------------------------------------+
*    | Argument          | clog(Argument)                            |
*    +-------------------+-------------------------------------------+
*    | -0 + 0.i          | -Inf + Pi.i                               |
*    | +0 + 0.i          | -Inf + 0.i                                |
*    | a + Inf.i         | +Inf + Pi/2.i, for finite a               |
*    | a + NaN.i         | NaN + NaN.i, for finite a                 |
*    | -Inf + b.i        | +Inf + Pi.i, for finite positive b        |
*    | +Inf + b.i        | +Inf + 0.i, for finite positive b         |
*    | -Inf + Inf.i      | +Inf + 3Pi/4.i                            |
*    | +Inf + Inf.i      | +Inf + Pi/4.i                             |
*    | +/-Inf + NaN.i    | +Inf + NaN.i                              |
*    | NaN + b.i         | NaN + NaN.i, for finite b                 |
*    | NaN + Inf.i       | +Inf + NaN.i                              |
*    | NaN + NaN.i       | NaN + NaN.i                               |
*    +-------------------+-------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * clog(conj(z)) = conj(clog(z)).
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_clog_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  float mag;
  float arg;
  //
  mag = SEGGER_LOGF(SEGGER_HYPOTF(pX->u.part.Re, pX->u.part.Im));
  arg = SEGGER_ATAN2F(pX->u.part.Im, pX->u.part.Re);
  //
  pX->u.part.Re = mag;
  pX->u.part.Im = arg;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_clog_inline()
*
*  Function description
*    Compute natural logarithm, double complex.
*
*  Parameters
*    pX - Pointer to value to compute logarithm of.
*
*  Additional information
*    See __SEGGER_RTL_float32_clog_inline().
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_clog_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  double mag;
  double arg;
  //
  mag = SEGGER_LOG(SEGGER_HYPOT(pX->u.part.Re, pX->u.part.Im));
  arg = SEGGER_ATAN2(pX->u.part.Im, pX->u.part.Re);
  //
  pX->u.part.Re = mag;
  pX->u.part.Im = arg;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_clog()
*
*  Function description
*    Compute natural logarithm, double complex.
*
*  Parameters
*    pX - Pointer to value to compute logarithm of.
*
*  Additional information
*    See __SEGGER_RTL_float32_clog_inline().
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_clog(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_float32_clog_inline(pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_clog()
*
*  Function description
*    Compute natural logarithm, double complex.
*
*  Parameters
*    pX - Pointer to value to compute logarithm of.
*
*  Additional information
*    See __SEGGER_RTL_float64_clog_inline().
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_clog(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_float64_clog_inline(pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cexp_inline()
*
*  Function description
*    Compute base-e exponential, double complex.
*
*  Parameters
*    pX - Pointer to complex argument.
*
*  Additional information
*    The base-e exponential of x+i.y is calculated according to the following
*    table:
*
*    +-------------------+-----------------------------------------------+
*    | Argument          | cexp(Argument)                                |
*    +-------------------+-----------------------------------------------+
*    | -/-0 + 0.i        | +1 + 0.i                                      |
*    | a + Inf.i         | NaN + NaN.i, for finite a                     |
*    | a + NaN.i         | NaN + NaN.i, for finite a                     |
*    | +Inf + 0.i        | +Inf + 0.i, for finite positive b             |
*    | -Inf + b.i        | +0 cis(b) for finite b                        |
*    | +Inf + b.i        | +Inf cis(b) for finite nonzero b              |
*    | -Inf + Inf.i      | +/-Inf + +/-0.i, signs unspecified            |
*    | +Inf + Inf.i      | +/-Inf + i.NaN, sign of real part unspecified |
*    | -Inf + NaN.i      | +/-0 + +/-0.i, signs unspecified              |
*    | +Inf + NaN.i      | +/-Inf + NaN.i, sign of real part unspecified |
*    | NaN + 0.i         | NaN + 0.i                                     |
*    | NaN + b.i         | NaN + NaN.i, for nonzero b                    |
*    | NaN + NaN.i       | NaN + NaN.i                                   |
*    +-------------------+-----------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality
*
*    *  cexp(conj(z)) = conj(cexp(z)).
*
*  Notes
*    exp(a + b.i) = exp(a)cos(b) + i.exp(a)sin(b)
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_cexp_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT32_COMPLEX Result;
  float                        eX;
  //
  // General case.
  //
  eX               = SEGGER_EXPF(pX->u.part.Re);
  Result.u.part.Re = SEGGER_MULF(eX, SEGGER_COSF(pX->u.part.Im));
  Result.u.part.Im = SEGGER_MULF(eX, SEGGER_SINF(pX->u.part.Im));
  //
  // If something went wrong and a NaN is generated, deal with the exceptional case.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(Result.u.part.Im))) {
    //
    if (__SEGGER_RTL_BitcastToU32_inline(pX->u.part.Re) == K_MINUS_INF_U32 && !__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im)) {
      //
      // cexp(-Inf + Inf.i) = -Inf -0.i
      // cexp(0 + NaN.i) = NaN + +/-0.i
      //
      Result.u.part.Re = 0;
      Result.u.part.Im = 0;
      //
    } else if (__SEGGER_RTL_BitcastToU32_inline(pX->u.part.Re) == K_INF_U32 && !__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im)) {
      //
      // cexp(+Inf + Inf.i) = +/-Inf + NaN.i
      // cexp(+Inf + NaN.i) = +/-Inf + NaN.i
      //
      Result.u.part.Re = __SEGGER_RTL_BitcastToF32(K_INF_U32);
      //
    } else if (!__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Re) && SEGGER_EQ0F(pX->u.part.Im)) {
      //
      // cexp(+Inf + 0.i) = +Inf + 0.i
      // cexp( NaN + 0.i) =  NaN + 0.i
      //
      Result = *pX;
    }
  }
  //
  *pX = Result;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cexp_inline()
*
*  Function description
*    Compute base-e exponential, double complex.
*
*  Parameters
*    pX - Pointer to complex argument.
*
*  Additional information
*    See __SEGGER_RTL_float32_cexp_inline().
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_cexp_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT64_COMPLEX Result;
  double                       eX;
  //
  // General case.
  //
  eX               = SEGGER_EXP(pX->u.part.Re);
  Result.u.part.Re = SEGGER_MUL(eX, SEGGER_COS(pX->u.part.Im));
  Result.u.part.Im = SEGGER_MUL(eX, SEGGER_SIN(pX->u.part.Im));
  //
  // If something went wrong and a NaN is generated, deal with the exceptional case.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(Result.u.part.Im))) {
    //
    if (__SEGGER_RTL_BitcastToU64(pX->u.part.Re) == K_MINUS_INF_U64 && !__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im)) {
      //
      // cexp(-Inf + Inf.i) = -Inf -0.i
      // cexp(0 + NaN.i) = NaN + +/-0.i
      //
      Result.u.part.Re = 0;
      Result.u.part.Im = 0;
      //
    } else if (__SEGGER_RTL_BitcastToU64(pX->u.part.Re) == K_INF_U64 && !__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im)) {
      //
      // cexp(+Inf + Inf.i) = +/-Inf + NaN.i
      // cexp(+Inf + NaN.i) = +/-Inf + NaN.i
      //
      Result.u.part.Re = __SEGGER_RTL_BitcastToF64(K_INF_U64);
      //
    } else if (!__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Re) && SEGGER_EQ0(pX->u.part.Im)) {
      //
      // cexp(+Inf + 0.i) = +Inf + 0.i
      // cexp( NaN + 0.i) =  NaN + 0.i
      //
      Result = *pX;
    }
  }
  //
  *pX = Result;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cexp()
*
*  Function description
*    Compute base-e exponential, double complex.
*
*  Parameters
*    pX - Pointer to complex argument.
*
*  Additional information
*    See __SEGGER_RTL_float32_cexp_inline().
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cexp(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_float32_cexp_inline(pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cexp()
*
*  Function description
*    Compute base-e exponential, double complex.
*
*  Parameters
*    pX - Pointer to complex argument.
*
*  Additional information
*    See __SEGGER_RTL_float64_cexp_inline().
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cexp(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_float64_cexp_inline(pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cpow()
*
*  Function description
*    Power, float complex.
*
*  Parameters
*    pX - Pointer to base and result.
*    pY - Pointer to power.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cpow(__SEGGER_RTL_FLOAT32_COMPLEX *pX, const __SEGGER_RTL_FLOAT32_COMPLEX *pY) {
  __SEGGER_RTL_float32_clog(pX);
  __SEGGER_RTL_float32_cmul(pX, pY);
  __SEGGER_RTL_float32_cexp(pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cpow()
*
*  Function description
*    Power, double complex.
*
*  Parameters
*    pX - Pointer to base and result.
*    pY - Pointer to power.
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cpow(__SEGGER_RTL_FLOAT64_COMPLEX *pX, const __SEGGER_RTL_FLOAT64_COMPLEX *pY) {
  __SEGGER_RTL_float64_clog(pX);
  __SEGGER_RTL_float64_cmul(pX, pY);
  __SEGGER_RTL_float64_cexp(pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_csinh_inline()
*
*  Function description
*    Compute complex hyperbolic sine, float .
*
*  Parameters
*    pX - Pointer to value to compute complex hyperbolic sine of.
*
*  Notes
*    csinh(z) == (cexp(z) - cexp(-z)) / 2
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_csinh_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT32_COMPLEX eMx;
  //
  if (SEGGER_EQ0F(pX->u.part.Re)) {
    //
    // Specialize real case for better accuracy.
    //
    pX->u.part.Im = SEGGER_SINF(pX->u.part.Im);
    //
  } else {
    //
    // General case.
    //
    eMx = *pX;
    __SEGGER_RTL_float32_cneg (&eMx);          // -z
    __SEGGER_RTL_float32_cexp (&eMx);          // cexp(-z)
    __SEGGER_RTL_float32_cexp (pX);            // cexp(z)
    __SEGGER_RTL_float32_csub (pX, &eMx);      // cexp(z) - cexp(-z)
    __SEGGER_RTL_float32_cdiv2(pX);            // (cexp(z) - cexp(-z)) / 2
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_csinh_inline()
*
*  Function description
*    Compute complex hyperbolic sine, double.
*
*  Parameters
*    pX - Pointer to value to compute complex hyperbolic sine of.
*
*  Notes
*    csinh(z) == (cexp(z) - cexp(-z)) / 2
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_csinh_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT64_COMPLEX eMx;
  //
  if (SEGGER_EQ0(pX->u.part.Re)) {
    //
    // Specialize real case for better accuracy.
    //
    pX->u.part.Im = SEGGER_SIN(pX->u.part.Im);
    //
  } else {
    //
    // General case.
    //
    eMx = *pX;
    __SEGGER_RTL_float64_cneg (&eMx);          // -z
    __SEGGER_RTL_float64_cexp (&eMx);          // cexp(-z)
    __SEGGER_RTL_float64_cexp (pX);            // cexp(z)
    __SEGGER_RTL_float64_csub (pX, &eMx);      // cexp(z) - cexp(-z)
    __SEGGER_RTL_float64_cdiv2(pX);            // (cexp(z) - cexp(-z)) / 2
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ccosh_inline()
*
*  Function description
*    Compute complex hyperbolic cosine, float.
*
*  Parameters
*    pX - Pointer to value to compute complex hyperbolic cosine of.
*
*  Notes
*    ccosh(z) == (cexp(z) + cexp(-z)) / 2
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_ccosh_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT32_COMPLEX eMx;
  __SEGGER_RTL_FLOAT32_COMPLEX ePx;
  int                          Conjugate;
  //
  if (__SEGGER_RTL_float32_signbit_inline(pX->u.part.Re)) {
    __SEGGER_RTL_float32_cneg(pX);
  }
  Conjugate = __SEGGER_RTL_float32_signbit_inline(pX->u.part.Im);
  if (Conjugate) {
    __SEGGER_RTL_float32_ccnj(pX);
  }
  //
  eMx = *pX;
  ePx = *pX;
  //
  __SEGGER_RTL_float32_cneg (&eMx);          // -z
  __SEGGER_RTL_float32_cexp (&eMx);          // cexp(-z)
  __SEGGER_RTL_float32_cexp (&ePx);          // cexp(z)
  __SEGGER_RTL_float32_cadd (&ePx, &eMx);    // cexp(z) + cexp(-z)
  __SEGGER_RTL_float32_cdiv2(&ePx);          // (cexp(z) + cexp(-z)) / 2
  //
  // If something went wrong and a NaN is generated, deal with the exceptional case.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(ePx.u.part.Re))) {
    //
    if (!__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im)) {
      //
      if (SEGGER_EQ0F(pX->u.part.Re)) {
        //
        // cexp(0 + Inf.i) = NaN + +/-0.i
        // cexp(0 + NaN.i) = NaN + +/-0.i
        //
        ePx.u.part.Im = 0;
        //
      } else if (__SEGGER_RTL_float32_isinf_inline(pX->u.part.Re)) {
        //
        // cexp(+/-Inf + NaN.i) = +Inf + NaN.i
        //
        ePx.u.part.Re = __SEGGER_RTL_BitcastToF32(K_INF_U32);
      }
    } else if (__SEGGER_RTL_float32_isnan_inline(pX->u.part.Re)) {
      //
      if (SEGGER_EQ0F(pX->u.part.Im)) {
        //
        // cexp(NaN + 0.i) = NaN + +/-0.i
        //
        ePx.u.part.Im = 0;
      }
    }
  }
  //
  if (Conjugate) {
    __SEGGER_RTL_float32_ccnj(&ePx);
  }
  //
  *pX = ePx;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ccosh_inline()
*
*  Function description
*    Compute complex hyperbolic cosine, double.
*
*  Parameters
*    pX - Pointer to value to compute complex hyperbolic cosine of.
*
*  Notes
*    ccosh(z) == (cexp(z) + cexp(-z)) / 2
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_ccosh_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT64_COMPLEX eMx;
  __SEGGER_RTL_FLOAT64_COMPLEX ePx;
  int                          Conjugate;
  //
  if (__SEGGER_RTL_float64_signbit_inline(pX->u.part.Re)) {
    __SEGGER_RTL_float64_cneg(pX);
  }
  Conjugate = __SEGGER_RTL_float64_signbit_inline(pX->u.part.Im);
  if (Conjugate) {
    __SEGGER_RTL_float64_ccnj(pX);
  }
  //
  eMx = *pX;
  ePx = *pX;
  //
  __SEGGER_RTL_float64_cneg (&eMx);          // -z
  __SEGGER_RTL_float64_cexp (&eMx);          // cexp(-z)
  __SEGGER_RTL_float64_cexp (&ePx);          // cexp(z)
  __SEGGER_RTL_float64_cadd (&ePx, &eMx);    // cexp(z) + cexp(-z)
  __SEGGER_RTL_float64_cdiv2(&ePx);          // (cexp(z) + cexp(-z)) / 2
  //
  // If something went wrong and a NaN is generated, deal with the exceptional case.
  //
  if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(ePx.u.part.Re))) {
    //
    if (!__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im)) {
      //
      if (SEGGER_EQ0(pX->u.part.Re)) {
        //
        // cexp(0 + Inf.i) = NaN + +/-0.i
        // cexp(0 + NaN.i) = NaN + +/-0.i
        //
        ePx.u.part.Im = 0;
        //
      } else if (__SEGGER_RTL_float64_isinf_inline(pX->u.part.Re)) {
        //
        // cexp(+/-Inf + NaN.i) = +Inf + NaN.i
        //
        ePx.u.part.Re = __SEGGER_RTL_BitcastToF64(K_INF_U64);
      }
    } else if (__SEGGER_RTL_float64_isnan_inline(pX->u.part.Re)) {
      //
      if (SEGGER_EQ0(pX->u.part.Im)) {
        //
        // cexp(NaN + 0.i) = NaN + +/-0.i
        //
        ePx.u.part.Im = 0;
      }
    }
  }
  //
  if (Conjugate) {
    __SEGGER_RTL_float64_ccnj(&ePx);
  }
  //
  *pX = ePx;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ctanh_inline()
*
*  Function description
*    Compute complex hyperbolic tangent, float.
*
*  Parameters
*    pX - Pointer to value to compute complex hyperbolic tangent of.
*
*  Notes
*    ctanh(z) = exp(z) - exp(-z)) / (exp(z) + exp(-z))
*             = [sinh(2*a) / (cosh(2*a) + cos(2*b)) + i.sin(2*b) / (cosh(2*a)+cos(2*b))], where z = a + b.i
*             = [sinh(2*a) + i.sin(2*b)] / [cosh(2*a) + cos(2*b)]
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_ctanh_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT32_COMPLEX Result;
  float                        s;
  //
  if (SEGGER_EQ0F(pX->u.part.Re)) {
    //
    // Specialize imaginary case for better accuracy.
    //
    pX->u.part.Im = SEGGER_TANF(pX->u.part.Im);
    //
    // Deal with exceptional cases.
    //
    if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(pX->u.part.Im))) {
      //
      // ctanh(0 + Inf.i) = NaN + NaN.i
      // ctanh(0 + NaN.i) = NaN + NaN.i
      //
      pX->u.part.Re = pX->u.part.Im;
    }
    //
  } else if (SEGGER_EQ0F(pX->u.part.Im)) {
    //
    // Specialize real case for better accuracy.
    //
    pX->u.part.Re = SEGGER_TANHF(pX->u.part.Re);
    //
  } else {
    //
    // General case.
    //
    Result.u.part.Re = SEGGER_MUL2F(pX->u.part.Re);
    Result.u.part.Im = SEGGER_MUL2F(pX->u.part.Im);
    //
    s = SEGGER_ADDF(SEGGER_COSHF(Result.u.part.Re), SEGGER_COSF(Result.u.part.Im));
    //
    Result.u.part.Re = SEGGER_DIVF(SEGGER_SINHF(Result.u.part.Re), s);
    Result.u.part.Im = SEGGER_DIVF(SEGGER_SINF (Result.u.part.Im), s);
    //
    // If something went wrong and a NaN is generated, deal with the exceptional case.
    //
    if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float32_isnan_inline(Result.u.part.Re))) {
      //
      if (__SEGGER_RTL_float32_isinf_inline(pX->u.part.Re)) {
        //
        // ctanh(Inf + Inf.i) = 1 + +/-0.i
        // ctanh(Inf + NaN.i) = 1 + +/-0.i
        //
        Result.u.part.Re = __SEGGER_RTL_float32_signbit_xor(1, pX->u.part.Re);
        if (__SEGGER_RTL_float32_isnan_inline(Result.u.part.Im)) {
          Result.u.part.Im = 0;
        }
      }
    }
    //
    *pX = Result;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ctanh_inline()
*
*  Function description
*    Compute complex hyperbolic tangent, double.
*
*  Parameters
*    pX - Pointer to value to compute complex hyperbolic tangent of.
*
*  Notes
*    ctanh(z) = exp(z) - exp(-z)) / (exp(z) + exp(-z))
*             = [sinh(2*a) / (cosh(2*a) + cos(2*b)) + i.sin(2*b) / (cosh(2*a)+cos(2*b))], where z = a + b.i
*             = [sinh(2*a) + i.sin(2*b)] / [cosh(2*a) + cos(2*b)]
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_ctanh_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT64_COMPLEX Result;
  double                       s;
  //
  if (SEGGER_EQ0(pX->u.part.Re)) {
    //
    // Specialize imaginary case for better accuracy.
    //
    pX->u.part.Im = SEGGER_TAN(pX->u.part.Im);
    //
    // Deal with exceptional cases.
    //
    if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(pX->u.part.Im))) {
      //
      // ctanh(0 + Inf.i) = NaN + NaN.i
      // ctanh(0 + NaN.i) = NaN + NaN.i
      //
      pX->u.part.Re = pX->u.part.Im;
    }
    //
  } else if (SEGGER_EQ0(pX->u.part.Im)) {
    //
    // Specialize real case for better accuracy.
    //
    pX->u.part.Re = SEGGER_TANH(pX->u.part.Re);
    //
  } else {
    //
    // General case.
    //
    Result.u.part.Re = SEGGER_MUL2(pX->u.part.Re);
    Result.u.part.Im = SEGGER_MUL2(pX->u.part.Im);
    //
    s = SEGGER_ADD(SEGGER_COSH(Result.u.part.Re), SEGGER_COS(Result.u.part.Im));
    //
    Result.u.part.Re = SEGGER_DIV(SEGGER_SINH(Result.u.part.Re), s);
    Result.u.part.Im = SEGGER_DIV(SEGGER_SIN (Result.u.part.Im), s);
    //
    // If something went wrong and a NaN is generated, deal with the exceptional case.
    //
    if (__SEGGER_RTL_UNLIKELY(__SEGGER_RTL_float64_isnan_inline(Result.u.part.Re))) {
      //
      if (__SEGGER_RTL_float64_isinf_inline(pX->u.part.Re)) {
        //
        // ctanh(Inf + Inf.i) = 1 + +/-0.i
        // ctanh(Inf + NaN.i) = 1 + +/-0.i
        //
        Result.u.part.Re = __SEGGER_RTL_float64_signbit_xor(1, pX->u.part.Re);
        if (__SEGGER_RTL_float64_isnan_inline(Result.u.part.Im)) {
          Result.u.part.Im = 0;
        }
      }
    }
    //
    *pX = Result;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_csin_inline()
*
*  Function description
*    Compute complex sine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    csin(z) = -i csinh(i.z)
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_csin_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_float32_cmuli       (pX);
  __SEGGER_RTL_float32_csinh_inline(pX);
  __SEGGER_RTL_float32_cdivi       (pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_csin_inline()
*
*  Function description
*    Compute complex sine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    csin(z) = -i csinh(i.z)
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_csin_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_float64_cmuli       (pX);
  __SEGGER_RTL_float64_csinh_inline(pX);
  __SEGGER_RTL_float64_cdivi       (pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ccos_inline()
*
*  Function description
*    Compute complex cosine, float.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    ccos(z) = ccosh(i.z)
*            = (cexp(i.z) + cexp(-i.z)) / 2
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_ccos_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_float32_cmuli       (pX);
  __SEGGER_RTL_float32_ccosh_inline(pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ccos_inline()
*
*  Function description
*    Compute complex cosine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    ccos(z) = ccosh(i.z)
*            = (cexp(i.z) + cexp(-i.z)) / 2
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_ccos_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_float64_cmuli       (pX);
  __SEGGER_RTL_float64_ccosh_inline(pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_ctan_inline()
*
*  Function description
*    Compute complex tangent, float.
*
*  Parameters
*    pX - Pointer to value to compute complex tangent of.
*
*  Notes
*    ctan(z) = -i.ctanh(i.z)
*            = i.[exp(-i.z) - exp(i.z)) / (exp(-i.z) + exp(i.z)]
*            = [sin(2*a) - i.sinh(-2*b)] / (cosh(-2*b) + cos(2*a))
*            = [sin(2*a) - i.sinh(-2*b)] / (cosh(2*b) + cos(2*a))      as cosh is even
*            = [sin(2*a) + i.sinh(2*b)]  / (cosh(2*b) + cos(2*a))      as sinh is odd
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float32_ctan_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_float32_cmuli       (pX);
  __SEGGER_RTL_float32_ctanh_inline(pX);
  __SEGGER_RTL_float32_cdivi       (pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_ctan_inline()
*
*  Function description
*    Compute complex tangent, double.
*
*  Parameters
*    pX - Pointer to value to compute complex tangent of.
*
*  Notes
*    ctan(z) = -i.ctanh(i.z)
*            = i.[exp(-i.z) - exp(i.z)) / (exp(-i.z) + exp(i.z)]
*            = [sin(2*a) - i.sinh(-2*b)] / (cosh(-2*b) + cos(2*a))
*            = [sin(2*a) - i.sinh(-2*b)] / (cosh(2*b) + cos(2*a))      as cosh is even
*            = [sin(2*a) + i.sinh(2*b)]  / (cosh(2*b) + cos(2*a))      as sinh is odd
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_float64_ctan_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_float64_cmuli       (pX);
  __SEGGER_RTL_float64_ctanh_inline(pX);
  __SEGGER_RTL_float64_cdivi       (pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_casinh()
*
*  Function description
*    Compute inverse hperbolic sine, float.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    casinh(z) = clog(z + csqrt(1 + z^2))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_casinh(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT32_COMPLEX Result;
  __SEGGER_RTL_FLOAT32_COMPLEX Z;
  int                          ReSign;
  int                          ImSign;
  //
  Z = *pX;
  //
  ReSign = __SEGGER_RTL_float32_signbit_inline(Z.u.part.Re);
  if (ReSign) {
    __SEGGER_RTL_float32_cneg(&Z);
  }
  ImSign = __SEGGER_RTL_float32_signbit_inline(Z.u.part.Im);
  if (ImSign) {
    __SEGGER_RTL_float32_ccnj(&Z);
  }
  //
  Result = Z;
  __SEGGER_RTL_float32_cmul(&Result, &Result);
  Result.u.part.Re = SEGGER_ADDF(Result.u.part.Re, 1.0f);
  __SEGGER_RTL_float32_csqrt(&Result);
  __SEGGER_RTL_float32_cadd(&Result, &Z);
  __SEGGER_RTL_float32_clog(&Result);
  //
  if (ImSign) {
    __SEGGER_RTL_float32_ccnj(&Result);
  }
  if (ReSign) {
    __SEGGER_RTL_float32_cneg(&Result);
  }
  //
  // Deal with exceptions.
  //
  if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float32_isfinite_inline(Result.u.part.Re))) {
    //
    if (__SEGGER_RTL_float32_isfinite_inline(Z.u.part.Re) && __SEGGER_RTL_float32_isinf_inline(Z.u.part.Im)) {
      Result.u.part.Im = __SEGGER_RTL_float32_signbit_xor((float)M_PI_2, Result.u.part.Im);
    } else if (__SEGGER_RTL_float32_isnan_inline(Z.u.part.Re) && SEGGER_EQ0F(Z.u.part.Im)) {
      Result.u.part.Im = pX->u.part.Im;
    } else if (__SEGGER_RTL_float32_isinf_inline(Z.u.part.Re) && __SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im)) {
      Result.u.part.Im = __SEGGER_RTL_float32_signbit_copy(0.0f, pX->u.part.Im);
    }
  }
  //
  *pX = Result;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_casinh()
*
*  Function description
*    Compute inverse hperbolic sine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    casinh(z) = clog(z + csqrt(1 + z^2))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_casinh(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT64_COMPLEX Result;
  __SEGGER_RTL_FLOAT64_COMPLEX Z;
  int                          ReSign;
  int                          ImSign;
  //
  Z = *pX;
  //
  ReSign = __SEGGER_RTL_float64_signbit_inline(Z.u.part.Re);
  if (ReSign) {
    __SEGGER_RTL_float64_cneg(&Z);
  }
  ImSign = __SEGGER_RTL_float64_signbit_inline(Z.u.part.Im);
  if (ImSign) {
    __SEGGER_RTL_float64_ccnj(&Z);
  }
  //
  Result = Z;
  __SEGGER_RTL_float64_cmul(&Result, &Result);
  Result.u.part.Re = SEGGER_ADD(Result.u.part.Re, 1);
  __SEGGER_RTL_float64_csqrt(&Result);
  __SEGGER_RTL_float64_cadd(&Result, &Z);
  __SEGGER_RTL_float64_clog(&Result);
  //
  if (ImSign) {
    __SEGGER_RTL_float64_ccnj(&Result);
  }
  if (ReSign) {
    __SEGGER_RTL_float64_cneg(&Result);
  }
  //
  // Deal with exceptions.
  //
  if (__SEGGER_RTL_UNLIKELY(!__SEGGER_RTL_float64_isfinite_inline(Result.u.part.Re))) {
    //
    if (__SEGGER_RTL_float64_isfinite_inline(Z.u.part.Re) && __SEGGER_RTL_float64_isinf_inline(Z.u.part.Im)) {
      Result.u.part.Im = __SEGGER_RTL_float64_signbit_xor(M_PI_2, Result.u.part.Im);
    } else if (__SEGGER_RTL_float64_isnan_inline(Z.u.part.Re) && SEGGER_EQ0(Z.u.part.Im)) {
      Result.u.part.Im = pX->u.part.Im;
    } else if (__SEGGER_RTL_float64_isinf_inline(Z.u.part.Re) && __SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im)) {
      Result.u.part.Im = __SEGGER_RTL_float64_signbit_copy(0, pX->u.part.Im);
    }
  }
  //
  *pX = Result;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cacosh_inline()
*
*  Function description
*    Compute inverse hperbolic cosine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    cacosh(z) = clog(z + csqrt(z+1)*csqrt(z-1)))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cacosh_inline(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT32_COMPLEX T1;
  __SEGGER_RTL_FLOAT32_COMPLEX T2;
  int                          sign;
  //
  sign          = __SEGGER_RTL_float32_signbit_inline(pX->u.part.Im);
  pX->u.part.Im = SEGGER_FABSF(pX->u.part.Im);
  //
  T1 = *pX;
  T1.u.part.Re = SEGGER_ADDF(T1.u.part.Re, 1);
  __SEGGER_RTL_float32_csqrt(&T1);
  //
  T2 = *pX;
  T2.u.part.Re = SEGGER_SUBF(T2.u.part.Re, 1);
  __SEGGER_RTL_float32_csqrt(&T2);
  //
  __SEGGER_RTL_float32_cmul(&T1, &T2);
  __SEGGER_RTL_float32_cadd(&T1, pX);
  __SEGGER_RTL_float32_clog(&T1);
  //
  // Handle exceptions.
  //
  if (__SEGGER_RTL_BitcastToU32(T1.u.part.Re) == K_INF_U32) {
    //
    if (__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Re)) {
      //
      // cacosh(x + Inf.i) = +Inf + 1/2.Pi.i.
      //
      T1.u.part.Im = (float)M_PI_2;
      //
    } else if (__SEGGER_RTL_BitcastToU32(pX->u.part.Re) == K_INF_U32) {
      //
      // cacosh(+Inf + y.i)   = +Inf + 0.i for positive-signed finite y
      // cacosh(+Inf + Inf.i) = +Inf + 1/4.Pi.i
      //
      if (__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im)) {
        T1.u.part.Im = 0;
      } else if (__SEGGER_RTL_float32_isinf_inline(pX->u.part.Im)) {
        T1.u.part.Im = (float)M_PI_4;
      }
      //
    } else if (__SEGGER_RTL_BitcastToU32(pX->u.part.Re) == K_MINUS_INF_U32) {
      //
      // cacosh(-Inf + y.i)   = +Inf + Pi.i for positive-signed finite y
      // cacosh(-Inf + Inf.i) = +Inf + 3/4.Pi.i
      //
      if (__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im)) {
        T1.u.part.Im = (float)M_PI;
      } else if (__SEGGER_RTL_float32_isinf_inline(pX->u.part.Im)) {
        T1.u.part.Im = (float)(0.75 * M_PI);
      }
    }
  }
  //
  if (sign) {
    T1.u.part.Im = SEGGER_NEGF(T1.u.part.Im);
  }
  //
  *pX = T1;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cacosh_inline()
*
*  Function description
*    Compute inverse hperbolic cosine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    cacosh(z) = clog(z + csqrt(z+1)*csqrt(z-1)))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cacosh_inline(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT64_COMPLEX T1;
  __SEGGER_RTL_FLOAT64_COMPLEX T2;
  int                          sign;
  //
  sign          = __SEGGER_RTL_float64_signbit_inline(pX->u.part.Im);
  pX->u.part.Im = SEGGER_FABS(pX->u.part.Im);
  //
  T1 = *pX;
  T1.u.part.Re = SEGGER_ADD(T1.u.part.Re, 1);
  __SEGGER_RTL_float64_csqrt(&T1);
  //
  T2 = *pX;
  T2.u.part.Re = SEGGER_SUB(T2.u.part.Re, 1);
  __SEGGER_RTL_float64_csqrt(&T2);
  //
  __SEGGER_RTL_float64_cmul(&T1, &T2);
  __SEGGER_RTL_float64_cadd(&T1, pX);
  __SEGGER_RTL_float64_clog(&T1);
  //
  // Handle exceptions.
  //
  if (__SEGGER_RTL_BitcastToU64(T1.u.part.Re) == K_INF_U64) {
    //
    if (__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Re)) {
      //
      // cacosh(x + Inf.i) = +Inf + 1/2.Pi.i.
      //
      T1.u.part.Im = M_PI_2;
      //
    } else if (__SEGGER_RTL_BitcastToU64(pX->u.part.Re) == K_INF_U64) {
      //
      // cacosh(+Inf + y.i)   = +Inf + 0.i for positive-signed finite y
      // cacosh(+Inf + Inf.i) = +Inf + 1/4.Pi.i
      //
      if (__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im)) {
        T1.u.part.Im = 0;
      } else if (__SEGGER_RTL_float64_isinf_inline(pX->u.part.Im)) {
        T1.u.part.Im = M_PI_4;
      }
      //
    } else if (__SEGGER_RTL_BitcastToU64(pX->u.part.Re) == K_MINUS_INF_U64) {
      //
      // cacosh(-Inf + y.i)   = +Inf + Pi.i for positive-signed finite y
      // cacosh(-Inf + Inf.i) = +Inf + 3/4.Pi.i
      //
      if (__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im)) {
        T1.u.part.Im = M_PI;
      } else if (__SEGGER_RTL_float64_isinf_inline(pX->u.part.Im)) {
        T1.u.part.Im = 3 * M_PI / 4;
      }
    }
  }
  //
  if (sign) {
    T1.u.part.Im = SEGGER_NEG(T1.u.part.Im);
  }
  //
  *pX = T1;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_catanh()
*
*  Function description
*    Compute inverse hperbolic tangent, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Additional information
*    Calculates the non-negative inverse tangent of Re + i.Im.
*
*  Notes
*    atanh(z) = 1/2 * clog((1+z) / (1-z))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_catanh(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT32_COMPLEX tp;
  __SEGGER_RTL_FLOAT32_COMPLEX tm;
  int                          ReSign;
  int                          ImSign;
  int                          Special;
  //
  ReSign = __SEGGER_RTL_float32_signbit_inline(pX->u.part.Re);
  if (ReSign) {
    __SEGGER_RTL_float32_cneg(pX);
  }
  ImSign = __SEGGER_RTL_float32_signbit_inline(pX->u.part.Im);
  if (ImSign) {
    __SEGGER_RTL_float32_ccnj(pX);
  }
  //
  // Handle exceptions.  Do this before general case because not all
  // exceptional inputs generate an exceptional output.
  //
  Special = 0;
  if ((!__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Re) || !__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im))) {
    //
    if (__SEGGER_RTL_float32_isinf_inline(pX->u.part.Re)) {
      //
      // catanh(Inf + y.i)   = +0 + 1/2.Pi.i
      // catanh(Inf + Inf.i) = +0 + 1/2.Pi.i
      // catanh(Inf + NaN.i) = +0 + NaN.i
      //
      tp.u.part.Re = 0;
      tp.u.part.Im = __SEGGER_RTL_float32_isnan_inline(pX->u.part.Im) ? pX->u.part.Im : (float)M_PI_2;
      Special = 1;
      //
    } else if (__SEGGER_RTL_float32_isnan_inline(pX->u.part.Re) && __SEGGER_RTL_float32_isinf_inline(pX->u.part.Im)) {
      //
      // catanh(NaN + Inf.i) = +/-0 + 1/2.Pi.i
      //
      tp.u.part.Re = 0;
      tp.u.part.Im = (float)M_PI_2;
      Special = 1;
      //
    } else if (__SEGGER_RTL_float32_isinf_inline(pX->u.part.Im)) {
      //
      // catanh(NaN + Inf.i) = +/-0 + 1/2.Pi.i
      //
      tp.u.part.Re = 0;
      tp.u.part.Im = (float)M_PI_2;
      Special = 1;
      //
    } else if (SEGGER_EQ0F(pX->u.part.Re)) {
      //
      // catanh(+0 + 0.i)   = +0 + 0i
      // catanh(+0 + NaN.i) = +0 + NaNi
      //
      tp.u.part.Re = 0;
      tp.u.part.Im = pX->u.part.Im;
      Special = 1;
      //
    }
  } else if (SEGGER_EQF(pX->u.part.Re, 1) && SEGGER_EQ0F(pX->u.part.Im)) {
    //
    // catanh(+1 + 0.i) = +Inf + 0i
    //
    tp.u.part.Re = __SEGGER_RTL_BitcastToF32(K_INF_U32);
    tp.u.part.Im = 0;
    Special = 1;
  }
  //
  if (!Special) {
    //
    // General case.
    //
    tp.u.part.Re = SEGGER_ADDF(1, pX->u.part.Re);
    tp.u.part.Im = pX->u.part.Im;
    //
    tm.u.part.Re = SEGGER_SUBF(1, pX->u.part.Re);
    tm.u.part.Im = SEGGER_NEGF(pX->u.part.Im);
    //
    __SEGGER_RTL_float32_cdiv(&tp, &tm);  // (1+z) / (1-z)
    __SEGGER_RTL_float32_clog(&tp);       // clog((1+z) / (1-z))
    __SEGGER_RTL_float32_cdiv2(&tp);      // 1/2 * clog((1+z) / (1-z))
  }
  //
  if (ReSign) {
    __SEGGER_RTL_float32_cneg(&tp);
  }
  if (ImSign) {
    __SEGGER_RTL_float32_ccnj(&tp);
  }
  //                                                                                       
  *pX = tp;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_catanh()
*
*  Function description
*    Compute inverse hperbolic tangent, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Additional information
*    Calculates the non-negative inverse tangent of Re + i.Im.
*
*  Notes
*    atanh(z) = 1/2 * clog((1+z) / (1-z))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_catanh(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT64_COMPLEX tp;
  __SEGGER_RTL_FLOAT64_COMPLEX tm;
  int                          ReSign;
  int                          ImSign;
  int                          Special;
  //
  ReSign = __SEGGER_RTL_float64_signbit_inline(pX->u.part.Re);
  if (ReSign) {
    __SEGGER_RTL_float64_cneg(pX);
  }
  ImSign = __SEGGER_RTL_float64_signbit_inline(pX->u.part.Im);
  if (ImSign) {
    __SEGGER_RTL_float64_ccnj(pX);
  }
  //
  // Handle exceptions.  Do this before general case because not all
  // exceptional inputs generate an exceptional output.
  //
  Special = 0;
  if ((!__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Re) || !__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im))) {
    //
    if (__SEGGER_RTL_float64_isinf_inline(pX->u.part.Re)) {
      //
      // catanh(Inf + y.i)   = +0 + 1/2.Pi.i
      // catanh(Inf + Inf.i) = +0 + 1/2.Pi.i
      // catanh(Inf + NaN.i) = +0 + NaN.i
      //
      tp.u.part.Re = 0;
      tp.u.part.Im = __SEGGER_RTL_float64_isnan_inline(pX->u.part.Im) ? pX->u.part.Im : M_PI_2;
      Special = 1;
      //
    } else if (__SEGGER_RTL_float64_isnan_inline(pX->u.part.Re) && __SEGGER_RTL_float64_isinf_inline(pX->u.part.Im)) {
      //
      // catanh(NaN + Inf.i) = +/-0 + 1/2.Pi.i
      //
      tp.u.part.Re = 0;
      tp.u.part.Im = M_PI_2;
      Special = 1;
      //
    } else if (__SEGGER_RTL_float64_isinf_inline(pX->u.part.Im)) {
      //
      // catanh(NaN + Inf.i) = +/-0 + 1/2.Pi.i
      //
      tp.u.part.Re = 0;
      tp.u.part.Im = M_PI_2;
      Special = 1;
      //
    } else if (SEGGER_EQ0(pX->u.part.Re)) {
      //
      // catanh(+0 + 0.i)   = +0 + 0i
      // catanh(+0 + NaN.i) = +0 + NaNi
      //
      tp.u.part.Re = 0;
      tp.u.part.Im = pX->u.part.Im;
      Special = 1;
      //
    }
  } else if (SEGGER_EQ(pX->u.part.Re, 1) && SEGGER_EQ0(pX->u.part.Im)) {
    //
    // catanh(+1 + 0.i) = +Inf + 0i
    //
    tp.u.part.Re = __SEGGER_RTL_BitcastToF64(K_INF_U64);
    tp.u.part.Im = 0;
    Special = 1;
  }
  //
  if (!Special) {
    //
    // General case.
    //
    tp.u.part.Re = SEGGER_ADD(1, pX->u.part.Re);
    tp.u.part.Im = pX->u.part.Im;
    //
    tm.u.part.Re = SEGGER_SUB(1, pX->u.part.Re);
    tm.u.part.Im = SEGGER_NEG(pX->u.part.Im);
    //
    __SEGGER_RTL_float64_cdiv_outline(&tp, &tm);  // (1+z) / (1-z)
    __SEGGER_RTL_float64_clog(&tp);               // clog((1+z) / (1-z))
    __SEGGER_RTL_float64_cdiv2(&tp);              // 1/2 * clog((1+z) / (1-z))
  }
  //
  if (ReSign) {
    __SEGGER_RTL_float64_cneg(&tp);
  }
  if (ImSign) {
    __SEGGER_RTL_float64_ccnj(&tp);
  }
  //                                                                                       
  *pX = tp;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_casin()
*
*  Function description
*    Compute inverse sine, float.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    casin(z) = -i casinh(i.z)
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_casin(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_float32_cmuli (pX);
  __SEGGER_RTL_float32_casinh(pX);
  __SEGGER_RTL_float32_cdivi (pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_casin()
*
*  Function description
*    Compute inverse sine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    casin(z) = -i casinh(i.z)
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_casin(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_float64_cmuli (pX);
  __SEGGER_RTL_float64_casinh(pX);
  __SEGGER_RTL_float64_cdivi (pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cacos()
*
*  Function description
*    Compute inverse complex cosine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    cacos(z) = Pi/2 + i.log(z.i + csqrt(1-z*z))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_cacos(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT32_COMPLEX iz;
  __SEGGER_RTL_FLOAT32_COMPLEX Result;
  //
  // General case.
  //
  iz = *pX;
  __SEGGER_RTL_float32_cmuli(&iz);
  Result.u.part.Re = SEGGER_SUBF(1, SEGGER_MULF(SEGGER_SUBF(pX->u.part.Re, pX->u.part.Im), SEGGER_ADDF(pX->u.part.Re, pX->u.part.Im)));
  Result.u.part.Im = SEGGER_MULM2F(SEGGER_MULF(pX->u.part.Re, pX->u.part.Im));
  __SEGGER_RTL_float32_csqrt(&Result);
  __SEGGER_RTL_float32_cadd (&Result, &iz);
  __SEGGER_RTL_float32_clog (&Result);
  __SEGGER_RTL_float32_cmuli(&Result);
  Result.u.part.Re = SEGGER_ADDF((float)M_PI_2, Result.u.part.Re);
  //
  // Deal with exceptions.
  //
  if (!__SEGGER_RTL_float32_isfinite_inline(Result.u.part.Im)) {
    //
    if (__SEGGER_RTL_float32_isposinf_inline(pX->u.part.Re)) {
      //
      if (__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im)) {
        //
        // cacos(+Inf.i + x) = +0 - Inf.i
        // cacos(+Inf.i - x) = +0 + Inf.i
        //
        Result.u.part.Re = 0;
        Result.u.part.Im = __SEGGER_RTL_float32_signbit_xor(__SEGGER_RTL_BitcastToF32(K_MINUS_INF_U32), pX->u.part.Im);
        //
      } else if (__SEGGER_RTL_float32_isnan_inline(pX->u.part.Im)) {
        //
        // cacos(+Inf + NaN.i) = NaN +/-Inf.i, sign of result Inf is unspecified
        //
        Result.u.part.Im = __SEGGER_RTL_BitcastToF32(K_MINUS_INF_U32);  // We use -Inf elsewhere, so reuse that constant.
        //
      } else if (__SEGGER_RTL_float32_isneginf_inline(pX->u.part.Im)) {
        //
        // cacos(Inf - Inf.i) = Pi/4 + Inf.i
        //
        Result.u.part.Re = (float)M_PI_4;
        Result.u.part.Im = __SEGGER_RTL_BitcastToF32(K_INF_U32);
      }
      //
    } else if (__SEGGER_RTL_float32_isneginf_inline(pX->u.part.Re)) {
      //
      if (__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Im)) {
        //
        // cacos(-Inf.i + x) = +Pi - Inf.i
        // cacos(-Inf.i - x) = +Pi + Inf.i
        //
        Result.u.part.Re = (float)M_PI;
        Result.u.part.Im = __SEGGER_RTL_float32_signbit_xor(__SEGGER_RTL_BitcastToF32(K_MINUS_INF_U32), pX->u.part.Im);
        //
      } else if (__SEGGER_RTL_float32_isnan_inline(pX->u.part.Im)) {
        //
        // cacos(-Inf + NaN.i) = NaN +/-Inf.i, sign of result Inf is unspecified
        //
        Result.u.part.Im = __SEGGER_RTL_BitcastToF32(K_MINUS_INF_U32);  // We use -Inf elsewhere, so reuse that constant.
        //
      } else if (__SEGGER_RTL_float32_isneginf_inline(pX->u.part.Im)) {
        //
        // cacos(-Inf + Inf.i) = 3/4.Pi + Inf.i
        //
        Result.u.part.Re = (float)M_3_PI_4;
        Result.u.part.Im = __SEGGER_RTL_BitcastToF32(K_INF_U32);
      }
      //
    } else if (__SEGGER_RTL_float32_isnan_inline(pX->u.part.Re)) {
      //
      if (__SEGGER_RTL_float32_isnan_inline(pX->u.part.Re) && __SEGGER_RTL_float32_isinf_inline(pX->u.part.Im)) {
        //
        // cacos(NaN + +/-Inf.i) = NaN -/+Inf.i, sign of result Inf is inverted.
        //
        Result.u.part.Im = SEGGER_NEGF(pX->u.part.Im);
      }
      //
    } else if (__SEGGER_RTL_float32_isfinite_inline(pX->u.part.Re) && __SEGGER_RTL_float32_isinf_inline(pX->u.part.Im)) {
      //
      // cacos(x + Inf.i) = Pi/2 - Inf.i.
      // cacos(x - Inf.i) = Pi/2 + Inf.i.
      //
      Result.u.part.Re = (float)M_PI_2;
      Result.u.part.Im = SEGGER_NEGF(pX->u.part.Im);
      //
    } else if (SEGGER_EQ0F(pX->u.part.Re) && __SEGGER_RTL_float32_isnan_inline(pX->u.part.Im)) {
      //
      // cacos(+/-0 + NaN.i) = Pi/2 + NaN.i.
      //
      Result.u.part.Re = (float)M_PI_2;
    }
  }
  //
  *pX = Result;
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cacos()
*
*  Function description
*    Compute inverse complex cosine, double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Notes
*    cacos(z) = Pi/2 + i.log(z.i + csqrt(1-z*z))
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_cacos(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_FLOAT64_COMPLEX iz;
  __SEGGER_RTL_FLOAT64_COMPLEX Result;
  //
  // General case.
  //
  iz = *pX;
  __SEGGER_RTL_float64_cmuli(&iz);
  Result.u.part.Re = SEGGER_SUB(1, SEGGER_MUL(SEGGER_SUB(pX->u.part.Re, pX->u.part.Im), SEGGER_ADD(pX->u.part.Re, pX->u.part.Im)));
  Result.u.part.Im = SEGGER_MULM2(SEGGER_MUL(pX->u.part.Re, pX->u.part.Im));
  __SEGGER_RTL_float64_csqrt(&Result);
  __SEGGER_RTL_float64_cadd (&Result, &iz);
  __SEGGER_RTL_float64_clog (&Result);
  __SEGGER_RTL_float64_cmuli(&Result);
  Result.u.part.Re = SEGGER_ADD(M_PI_2, Result.u.part.Re);
  //
  // Deal with exceptions.
  //
  if (!__SEGGER_RTL_float64_isfinite_inline(Result.u.part.Im)) {
    //
    if (__SEGGER_RTL_float64_isposinf_inline(pX->u.part.Re)) {
      //
      if (__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im)) {
        //
        // cacos(+Inf.i + x) = +0 - Inf.i
        // cacos(+Inf.i - x) = +0 + Inf.i
        //
        Result.u.part.Re = 0;
        Result.u.part.Im = __SEGGER_RTL_float64_signbit_xor(__SEGGER_RTL_BitcastToF64(K_MINUS_INF_U64), pX->u.part.Im);
        //
      } else if (__SEGGER_RTL_float64_isnan_inline(pX->u.part.Im)) {
        //
        // cacos(+Inf + NaN.i) = NaN +/-Inf.i, sign of result Inf is unspecified
        //
        Result.u.part.Im = __SEGGER_RTL_BitcastToF64(K_MINUS_INF_U64);  // We use -Inf elsewhere, so reuse that constant.
        //
      } else if (__SEGGER_RTL_float64_isneginf_inline(pX->u.part.Im)) {
        //
        // cacos(Inf - Inf.i) = Pi/4 + Inf.i
        //
        Result.u.part.Re = M_PI_4;
        Result.u.part.Im = __SEGGER_RTL_BitcastToF64(K_INF_U64);
      }
      //
    } else if (__SEGGER_RTL_float64_isneginf_inline(pX->u.part.Re)) {
      //
      if (__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Im)) {
        //
        // cacos(-Inf.i + x) = +Pi - Inf.i
        // cacos(-Inf.i - x) = +Pi + Inf.i
        //
        Result.u.part.Re = M_PI;
        Result.u.part.Im = __SEGGER_RTL_float64_signbit_xor(__SEGGER_RTL_BitcastToF64(K_MINUS_INF_U64), pX->u.part.Im);
        //
      } else if (__SEGGER_RTL_float64_isnan_inline(pX->u.part.Im)) {
        //
        // cacos(-Inf + NaN.i) = NaN +/-Inf.i, sign of result Inf is unspecified
        //
        Result.u.part.Im = __SEGGER_RTL_BitcastToF64(K_MINUS_INF_U64);  // We use -Inf elsewhere, so reuse that constant.
        //
      } else if (__SEGGER_RTL_float64_isneginf_inline(pX->u.part.Im)) {
        //
        // cacos(-Inf + Inf.i) = 3/4.Pi + Inf.i
        //
        Result.u.part.Re = M_3_PI_4;
        Result.u.part.Im = __SEGGER_RTL_BitcastToF64(K_INF_U64);
      }
      //
    } else if (__SEGGER_RTL_float64_isnan_inline(pX->u.part.Re)) {
      //
      if (__SEGGER_RTL_float64_isnan_inline(pX->u.part.Re) && __SEGGER_RTL_float64_isinf_inline(pX->u.part.Im)) {
        //
        // cacos(NaN + +/-Inf.i) = NaN -/+Inf.i, sign of result Inf is inverted.
        //
        Result.u.part.Im = SEGGER_NEG(pX->u.part.Im);
      }
      //
    } else if (__SEGGER_RTL_float64_isfinite_inline(pX->u.part.Re) && __SEGGER_RTL_float64_isinf_inline(pX->u.part.Im)) {
      //
      // cacos(x + Inf.i) = Pi/2 - Inf.i.
      // cacos(x - Inf.i) = Pi/2 + Inf.i.
      //
      Result.u.part.Re = M_PI_2;
      Result.u.part.Im = SEGGER_NEG(pX->u.part.Im);
      //
    } else if (SEGGER_EQ0(pX->u.part.Re) && __SEGGER_RTL_float64_isnan_inline(pX->u.part.Im)) {
      //
      // cacos(+/-0 + NaN.i) = Pi/2 + NaN.i.
      //
      Result.u.part.Re = M_PI_2;
    }
  }
  //
  *pX = Result;
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_catan()
*
*  Function description
*    Compute inverse tangent, complex float.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Additional information
*    Calculates the non-negative inverse tangent of Re + i.Im.

*  Notes
*    atan(z) = -i catanh(i.z)
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float32_catan(__SEGGER_RTL_FLOAT32_COMPLEX *pX) {
  __SEGGER_RTL_float32_cmuli (pX);
  __SEGGER_RTL_float32_catanh(pX);
  __SEGGER_RTL_float32_cdivi (pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_catan()
*
*  Function description
*    Compute inverse tangent, complex double.
*
*  Parameters
*    pX - Pointer to argument.
*
*  Additional information
*    Calculates the non-negative inverse tangent of Re + i.Im.

*  Notes
*    atan(z) = -i catanh(i.z)
*/
static __SEGGER_RTL_INLINE void __SEGGER_RTL_float64_catan(__SEGGER_RTL_FLOAT64_COMPLEX *pX) {
  __SEGGER_RTL_float64_cmuli (pX);
  __SEGGER_RTL_float64_catanh(pX);
  __SEGGER_RTL_float64_cdivi (pX);
}

/*********************************************************************
*
*       __SEGGER_RTL_complex_double_to_ldouble()
*
*  Function description
*    Convert double complex to long double complex.
*
*  Parameters
*    x - Value to convert.
*
*  Return value
*    The converted value.
*/
static __SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_complex_double_to_ldouble(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  __SEGGER_RTL_LDOUBLE_COMPLEX zl;
  //
  z.u.value     = x;
  zl.u.part.Re  = __SEGGER_RTL_double_to_ldouble(z.u.part.Re);
  zl.u.part.Im  = __SEGGER_RTL_double_to_ldouble(z.u.part.Im);
  //
  return zl.u.value;
}

/*********************************************************************
*
*       __SEGGER_RTL_complex_ldouble_to_double()
*
*  Function description
*    Convert long double complex to double complex.
*
*  Parameters
*    x - Value to convert.
*
*  Return value
*    The converted value.
*/
static __SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_complex_ldouble_to_double(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  __SEGGER_RTL_LDOUBLE_COMPLEX zl;
  //
  zl.u.value  = x;
  z.u.part.Re  = __SEGGER_RTL_ldouble_to_double(zl.u.part.Re);
  z.u.part.Im  = __SEGGER_RTL_ldouble_to_double(zl.u.part.Im);
  //
  return z.u.value;
}

/*********************************************************************
*
*       Public code (GCC API functions)
*
**********************************************************************
*/

#if (__SEGGER_RTL_INCLUDE_GNU_API == 1) || (FORCE_GNU_API)

/*********************************************************************
*
*       __ltsf2()
*
*  Function description
*    Less than, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return < 0 if both operands are non-NaN and a < b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __ltsf2(float x, float y) {
  return __SEGGER_RTL_float32_lt(x, y) ? -1 : 0;
}

/*********************************************************************
*
*       __ltdf2()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return < 0 if both operands are non-NaN and a < b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __ltdf2(double x, double y) {
  return __SEGGER_RTL_float64_lt(x, y) ? -1 : 0;
}

/*********************************************************************
*
*       __lesf2()
*
*  Function description
*    Less than or equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return <= 0 if both operands are non-NaN and a < b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __lesf2(float x, float y) {
  return __SEGGER_RTL_float32_le(x, y) ? 0 : 1;
}

/*********************************************************************
*
*       __ledf2()
*
*  Function description
*    Less than or equal, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return <= 0 if both operands are non-NaN and a < b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __ledf2(double x, double y) {
  return __SEGGER_RTL_float64_le(x, y) ? 0 : 1;
}

/*********************************************************************
*
*       __gtsf2()
*
*  Function description
*    Greater than, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return > 0 if both operands are non-NaN and a > b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __gtsf2(float x, float y) {
  return __SEGGER_RTL_float32_gt(x, y) ? 1 : 0;
}

/*********************************************************************
*
*       __gtdf2()
*
*  Function description
*    Greater than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return > 0 if both operands are non-NaN and a > b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __gtdf2(double x, double y) {
  return __SEGGER_RTL_float64_gt(x, y) ? 1 : 0;
}

/*********************************************************************
*
*       __gesf2()
*
*  Function description
*    Greater than or equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return >= 0 if both operands are non-NaN and a >= b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __gesf2(float x, float y) {
  return __SEGGER_RTL_float32_ge(x, y) ? 0 : -1;
}

/*********************************************************************
*
*       __gedf2()
*
*  Function description
*    Greater than or equal, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return >= 0 if both operands are non-NaN and a >= b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __gedf2(double x, double y) {
  return __SEGGER_RTL_float64_ge(x, y) ? 0 : -1;
}

/*********************************************************************
*
*       __eqsf2()
*
*  Function description
*    Equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return == 0 if both operands are non-NaN and a == b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __eqsf2(float x, float y) {
  return __SEGGER_RTL_float32_eq(x, y) ? 0 : 1;
}

/*********************************************************************
*
*       __eqdf2()
*
*  Function description
*    Equal, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return == 0 if both operands are non-NaN and a == b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __eqdf2(double x, double y) {
  return __SEGGER_RTL_float64_eq(x, y) ? 0 : 1;
}

/*********************************************************************
*
*       __nesf2()
*
*  Function description
*    Not equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return == 0 if both operands are non-NaN and a == b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __nesf2(float x, float y) {
  return __SEGGER_RTL_float32_ne(x, y) ? 1 : 0;
}

/*********************************************************************
*
*       __nedf2()
*
*  Function description
*    Not equal, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return == 0 if both operands are non-NaN and a == b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __nedf2(double x, double y) {
  return __SEGGER_RTL_float64_ne(x, y) ? 1 : 0;
}

/*********************************************************************
*
*       __unordsf2()
*
*  Function description
*    Unordered operand query, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return nonzero if comparison between operands is unordered.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __unordsf2(float x, float y) {
  return __SEGGER_RTL_float32_isnan_inline(x) || __SEGGER_RTL_float32_isnan_inline(y);
}

/*********************************************************************
*
*       __unorddf2()
*
*  Function description
*    Unordered operand query, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return nonzero if comparison between operands is unordered.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __unorddf2(double x, double y) {
  return __SEGGER_RTL_float64_isnan_inline(x) || __SEGGER_RTL_float64_isnan_inline(y);
}

/*********************************************************************
*
*       __addsf3()
*
*  Function description
*    Add, float.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __addsf3(float x, float y) {
  return __SEGGER_RTL_float32_add(x, y);
}

/*********************************************************************
*
*       __adddf3()
*
*  Function description
*    Add, double.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __adddf3(double x, double y) {
  return __SEGGER_RTL_float64_add(x, y);
}

/*********************************************************************
*
*       __subsf3()
*
*  Function description
*    Subtract, float.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __subsf3(float x, float y) {
  return __SEGGER_RTL_float32_sub(x, y);
}

/*********************************************************************
*
*       __subdf3()
*
*  Function description
*    Subtract, double.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __subdf3(double x, double y) {
  return __SEGGER_RTL_float64_sub(x, y);
}

/*********************************************************************
*
*       __mulsf3()
*
*  Function description
*    Multiply, float.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __mulsf3(float x, float y) {
  return __SEGGER_RTL_float32_mul(x, y);
}

/*********************************************************************
*
*       __muldf3()
*
*  Function description
*    Multiply, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __muldf3(double x, double y) {
  return __SEGGER_RTL_float64_mul(x, y);
}

/*********************************************************************
*
*       __divsf3()
*
*  Function description
*    Divide, float.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __divsf3(float x, float y) {
  return __SEGGER_RTL_float32_div(x, y);
}

/*********************************************************************
*
*       __divdf3()
*
*  Function description
*    Divide, double.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __divdf3(double x, double y) {
  return __SEGGER_RTL_float64_div(x, y);
}

/*********************************************************************
*
*       __fixsfsi()
*
*  Function description
*    Convert float to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __fixsfsi(float x) {
  return __SEGGER_RTL_float_to_int32(x);
}

/*********************************************************************
*
*       __fixdfsi()
*
*  Function description
*    Convert double to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __fixdfsi(double x) {
  return __SEGGER_RTL_double_to_int32(x);
}

/*********************************************************************
*
*       __fixsfdi()
*
*  Function description
*    Convert float to long long.
*
*  Parameters
*    f - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    The RV32 compiler converts a float to a 64-bit integer
*    by calling runtime support to handle it.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API __fixsfdi(float f) {
  return __SEGGER_RTL_float_to_int64(f);
}

/*********************************************************************
*
*       __fixdfdi()
*
*  Function description
*    Convert double to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    RV32 always calls runtime for double to int64 conversion.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API __fixdfdi(double x) {
  return __SEGGER_RTL_double_to_int64(x);
}

/*********************************************************************
*
*       __fixunsdfdi()
*
*  Function description
*    Convert double to unsigned long long.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __fixunsdfdi(double x) {
  return __SEGGER_RTL_double_to_uint64(x);
}

/*********************************************************************
*
*       __fixunssfsi()
*
*  Function description
*    Convert float to unsigned.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
unsigned __SEGGER_RTL_PUBLIC_API __fixunssfsi(float x) {
  return __SEGGER_RTL_float_to_uint32(x);
}

/*********************************************************************
*
*       __fixunsdfsi()
*
*  Function description
*    Convert double to unsigned.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
unsigned __SEGGER_RTL_PUBLIC_API __fixunsdfsi(double x) {
  return __SEGGER_RTL_double_to_uint32(x);
}

/*********************************************************************
*
*       __floatsisf()
*
*  Function description
*    Convert int to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __floatsisf(__SEGGER_RTL_I32 x) {
  return __SEGGER_RTL_int32_to_float(x);
}

/*********************************************************************
*
*       __floatsidf()
*
*  Function description
*    Convert int to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __floatsidf(__SEGGER_RTL_I32 x) {
  return __SEGGER_RTL_int32_to_double(x);
}

/*********************************************************************
*
*       __floatunsisf()
*
*  Function description
*    Convert unsigned to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __floatunsisf(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_uint32_to_float(x);
}

/*********************************************************************
*
*       __fixunssfdi()
*
*  Function description
*    Convert float to unsigned long long.
*
*  Parameters
*    f - Float value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __fixunssfdi(float f) {
  return __SEGGER_RTL_float_to_uint64(f);
}

/*********************************************************************
*
*       __floatunsidf()
*
*  Function description
*    Convert unsigned to double.
*
*  Parameters
*    x - Unsigned value to convert.
*
*  Return value
*    Double value.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __floatunsidf(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_uint32_to_double(x);
}

/*********************************************************************
*
*       __floatdisf()
*
*  Function description
*    Convert long long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __floatdisf(__SEGGER_RTL_I64 x) {
  return __SEGGER_RTL_int64_to_float(x);
}

/*********************************************************************
*
*       __floatdidf()
*
*  Function description
*    Convert long long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __floatdidf(__SEGGER_RTL_I64 x) {
  return __SEGGER_RTL_int64_to_double(x);
}

/*********************************************************************
*
*       __floatundisf()
*
*  Function description
*    Convert unsigned long long to float.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Float value.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __floatundisf(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_uint64_to_float(x);
}

/*********************************************************************
*
*       __floatundidf()
*
*  Function description
*    Convert unsigned long long to double.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Double value.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __floatundidf(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_uint64_to_double(x);
}

/*********************************************************************
*
*       __truncdfsf2()
*
*  Function description
*    Truncate double to float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Float value.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __truncdfsf2(double x) {
  return __SEGGER_RTL_double_to_float(x);
}

/*********************************************************************
*
*       __extendsfdf2()
*
*  Function description
*    Extend float to double.
*
*  Parameters
*    x - Float value to extend.
*
*  Return value
*    Double value.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __extendsfdf2(float x) {
  return __SEGGER_RTL_float_to_double(x);
}

#endif

// All half-precision conversions default to C implementation.

#if __SEGGER_RTL_INCLUDE_GNU_FP16_API > 0

/*********************************************************************
*
*       __SEGGER_RTL_half_to_float_ieee()
*
*  Function description
*    Convert IEEE half-precision float to float.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Single-precision float.
*/
static __SEGGER_RTL_ALWAYS_INLINE float __SEGGER_RTL_half_to_float_ieee(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_half_to_float_ieee_soft(__SEGGER_RTL_BitcastToU16_inline(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_half_to_double_ieee()
*
*  Function description
*    Convert IEEE half-precision double to double.
*
*  Parameters
*    x - Half-precision double.
*
*  Return value
*    Double-precision double.
*/
static __SEGGER_RTL_ALWAYS_INLINE double __SEGGER_RTL_half_to_double_ieee(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_half_to_double_ieee_soft(__SEGGER_RTL_BitcastToU16_inline(x));
}

/*********************************************************************
*
*       __gnu_f2h_ieee()
*
*  Function description
*    Truncate float to IEEE half-precision float.
*
*  Parameters
*    x - Float value to truncate.
*
*  Return value
*    Float value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __gnu_f2h_ieee(float x) {
  return __SEGGER_RTL_float_to_half_ieee_soft(__SEGGER_RTL_BitcastToU32(x));
}

/*********************************************************************
*
*       __gnu_d2h_ieee()
*
*  Function description
*    Truncate double to IEEE half-precision float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Half-precision value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __gnu_d2h_ieee(double x) {
  return __SEGGER_RTL_double_to_half_ieee_soft(__SEGGER_RTL_BitcastToU64(x));
}

/*********************************************************************
*
*       __gnu_h2f_ieee()
*
*  Function description
*    Convert IEEE half-precision float to float.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Single-precision float.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __gnu_h2f_ieee(__SEGGER_RTL_U16 x) {
  return __SEGGER_RTL_half_to_float_ieee_soft(x);
}

/*********************************************************************
*
*       __gnu_h2d_ieee()
*
*  Function description
*    Convert IEEE half-precision float to double.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Double-precision float.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __gnu_h2d_ieee(__SEGGER_RTL_U16 x) {
  return __SEGGER_RTL_half_to_double_ieee_soft(x);
}

/*********************************************************************
*
*       __truncsfhf2()
*
*  Function description
*    Truncate float to IEEE half-precision float.
*
*  Parameters
*    x - Float value to truncate.
*
*  Return value
*    Float value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT16 __SEGGER_RTL_PUBLIC_API __truncsfhf2(float x) {
  return __SEGGER_RTL_BitcastToF16_inline(
           __SEGGER_RTL_float_to_half_ieee_soft(
             __SEGGER_RTL_BitcastToU32(x)));
}

/*********************************************************************
*
*       __truncdfhf2()
*
*  Function description
*    Truncate double to IEEE half-precision float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Half-precision value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT16 __SEGGER_RTL_PUBLIC_API __truncdfhf2(double x) {
  return __SEGGER_RTL_BitcastToF16_inline(
           __SEGGER_RTL_double_to_half_ieee_soft(
             __SEGGER_RTL_BitcastToU64(x)));
}

/*********************************************************************
*
*       __trunctfhf2()
*
*  Function description
*    Truncate long double to IEEE half-precision float.
*
*  Parameters
*    x - Long-double value to truncate.
*
*  Return value
*    Half-precision value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT16 __SEGGER_RTL_PUBLIC_API __trunctfhf2(long double x) {
  return __SEGGER_RTL_BitcastToF16_inline(
           __SEGGER_RTL_double_to_half_ieee(
             __SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       __extendhfsf2()
*
*  Function description
*    Convert IEEE half-precision float to float.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Single-precision float.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __extendhfsf2(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_half_to_float_ieee(x);
}

/*********************************************************************
*
*       __extendhfdf2()
*
*  Function description
*    Convert IEEE half-precision float to double.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Double-precision float.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __extendhfdf2(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_half_to_double_ieee(x);
}

/*********************************************************************
*
*       __extendhftf2()
*
*  Function description
*    Convert IEEE half-precision float to long double.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Long-double float.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __extendhftf2(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_double_to_ldouble(__SEGGER_RTL_half_to_double_ieee(x));
}

/*********************************************************************
*
*       __fixhfsi()
*
*  Function description
*    Convert half-precision float to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I32 __SEGGER_RTL_PUBLIC_API __fixhfsi(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_float_to_int32(__SEGGER_RTL_half_to_float_ieee(x));
}

/*********************************************************************
*
*       __fixunshfsi()
*
*  Function description
*    Convert half-precision float to unsigned.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
unsigned __SEGGER_RTL_PUBLIC_API __fixunshfsi(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_float_to_uint32(__SEGGER_RTL_half_to_float_ieee(x));
}

/*********************************************************************
*
*       __fixhfdi()
*
*  Function description
*    Convert half-precision float to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API __fixhfdi(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_float_to_int32(__SEGGER_RTL_half_to_float_ieee(x));
}

/*********************************************************************
*
*       __fixunshfdi()
*
*  Function description
*    Convert half-precision float to unsigned.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API __fixunshfdi(__SEGGER_RTL_FLOAT16 x) {
  return __SEGGER_RTL_float_to_uint32(__SEGGER_RTL_half_to_float_ieee(x));
}

/*********************************************************************
*
*       __floatsihf()
*
*  Function description
*    Convert int to half-precision float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT16 __SEGGER_RTL_PUBLIC_API __floatsihf(__SEGGER_RTL_I32 x) {
  return __SEGGER_RTL_BitcastToF16_inline(
           __SEGGER_RTL_float_to_half_ieee(
             __SEGGER_RTL_int32_to_float(x)));
}

/*********************************************************************
*
*       __floatunsihf()
*
*  Function description
*    Convert unsigned to half-precision float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT16 __SEGGER_RTL_PUBLIC_API __floatunsihf(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_BitcastToF16_inline(
           __SEGGER_RTL_float_to_half_ieee(
             __SEGGER_RTL_uint32_to_float(x)));
}

/*********************************************************************
*
*       __floatdihf()
*
*  Function description
*    Convert long long to half-precision float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT16 __SEGGER_RTL_PUBLIC_API __floatdihf(__SEGGER_RTL_I64 x) {
  return __SEGGER_RTL_BitcastToF16_inline(
           __SEGGER_RTL_float_to_half_ieee(
             __SEGGER_RTL_int64_to_float(x)));
}

/*********************************************************************
*
*       __floatundihf()
*
*  Function description
*    Convert unsigned long long to half-precision float.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Float value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT16 __SEGGER_RTL_PUBLIC_API __floatundihf(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_BitcastToF16_inline(
           __SEGGER_RTL_float_to_half_ieee(
             __SEGGER_RTL_uint64_to_float(x)));
}

/*********************************************************************
*
*       __eqhf2()
*
*  Function description
*    Equal, half-precision float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return == 0 if both operands are non-NaN and a == b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __eqhf2(__SEGGER_RTL_FLOAT16 x, __SEGGER_RTL_FLOAT16 y) {
  return __SEGGER_RTL_float32_eq(x, y) ? 0 : 1;
}

/*********************************************************************
*
*       __nehf2()
*
*  Function description
*    Not equal, half-precision float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return == 0 if both operands are non-NaN and a == b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __nehf2(__SEGGER_RTL_FLOAT16 x, __SEGGER_RTL_FLOAT16 y) {
  return __SEGGER_RTL_float32_ne(x, y) ? 1 : 0;
}

/*********************************************************************
*
*       __lehf2()
*
*  Function description
*    Less than or equal, half-precision float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return <= 0 if both operands are non-NaN and a < b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __lehf2(__SEGGER_RTL_FLOAT16 x, __SEGGER_RTL_FLOAT16 y) {
  return __SEGGER_RTL_float32_le(x, y) ? 0 : 1;
}

/*********************************************************************
*
*       __lthf2()
*
*  Function description
*    Less than, half-precision float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return < 0 if both operands are non-NaN and a < b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __lthf2(__SEGGER_RTL_FLOAT16 x, __SEGGER_RTL_FLOAT16 y) {
  return __SEGGER_RTL_float32_lt(x, y) ? -1 : 0;
}

/*********************************************************************
*
*       __gthf2()
*
*  Function description
*    Greater than, half-precision float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return > 0 if both operands are non-NaN and a > b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __gthf2(__SEGGER_RTL_FLOAT16 x, __SEGGER_RTL_FLOAT16 y) {
  return __SEGGER_RTL_float32_gt(x, y) ? 1 : 0;
}

/*********************************************************************
*
*       __gehf2()
*
*  Function description
*    Greater than or equal, half-precision float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return >= 0 if both operands are non-NaN and a >= b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __gehf2(__SEGGER_RTL_FLOAT16 x, __SEGGER_RTL_FLOAT16 y) {
  return __SEGGER_RTL_float32_ge(x, y) ? 0 : -1;
}

#endif

#if __SEGGER_RTL_INCLUDE_GNU_API > 0

/*********************************************************************
*
*       __unordtf2()
*
*  Function description
*    Unordered operand query, long double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return nonzero if comparison between operands is unordered.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __unordtf2(long double x, long double y) {
  return __SEGGER_RTL_float64_isnan_inline(__SEGGER_RTL_ldouble_to_double(x)) ||
         __SEGGER_RTL_float64_isnan_inline(__SEGGER_RTL_ldouble_to_double(y));
}

/*********************************************************************
*
*       __addtf3()
*
*  Function description
*    Add, long double.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __addtf3(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(
           SEGGER_ADD(
             __SEGGER_RTL_ldouble_to_double(x),
             __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       __subtf3()
*
*  Function description
*    Subtract, long double.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __subtf3(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(
           SEGGER_SUB(
             __SEGGER_RTL_ldouble_to_double(x),
             __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       __multf3()
*
*  Function description
*    Multiply, long double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __multf3(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(
           SEGGER_MUL(
             __SEGGER_RTL_ldouble_to_double(x),
             __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       __divtf3()
*
*  Function description
*    Divide, long double.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __divtf3(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(
           SEGGER_DIV(
             __SEGGER_RTL_ldouble_to_double(x),
             __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       __floatsitf()
*
*  Function description
*    Convert int to long double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __floatsitf(__SEGGER_RTL_I32 x) {
  return __SEGGER_RTL_double_to_ldouble(__SEGGER_RTL_int32_to_double(x));
}

/*********************************************************************
*
*       __floatditf()
*
*  Function description
*    Convert long long to long double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __floatditf(__SEGGER_RTL_I64 x) {
  return __SEGGER_RTL_double_to_ldouble(__SEGGER_RTL_int64_to_double(x));
}

/*********************************************************************
*
*       __floatunsitf()
*
*  Function description
*    Convert unsigned to long double.
*
*  Parameters
*    x - Unsigned value to convert.
*
*  Return value
*    Long double value.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __floatunsitf(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_double_to_ldouble(__SEGGER_RTL_uint32_to_double(x));
}

/*********************************************************************
*
*       __floatunditf()
*
*  Function description
*    Convert unsigned long long to long double.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Long double value.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __floatunditf(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_double_to_ldouble(__SEGGER_RTL_uint64_to_double(x));
}

/*********************************************************************
*
*       __fixtfsi()
*
*  Function description
*    Convert long double to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I32 __SEGGER_RTL_PUBLIC_API __fixtfsi(long double x) {
  return __SEGGER_RTL_double_to_int32(__SEGGER_RTL_ldouble_to_double(x));
}

/*********************************************************************
*
*       __fixtfdi()
*
*  Function description
*    Convert long double to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API __fixtfdi(long double x) {
  return __SEGGER_RTL_double_to_int64(__SEGGER_RTL_ldouble_to_double(x));
}

/*********************************************************************
*
*       __eqtf2()
*
*  Function description
*    Equal, long double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return == 0 if both operands are non-NaN and a == b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __eqtf2(long double x, long double y) {
  double dx;
  double dy;
  //
  dx = __SEGGER_RTL_ldouble_to_double(x);
  dy = __SEGGER_RTL_ldouble_to_double(y);
  //
  return dx == dy ? 0 : 1;
}

/*********************************************************************
*
*       __netf2()
*
*  Function description
*    Not equal, long double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return == 0 if both operands are non-NaN and a == b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __netf2(long double x, long double y) {
  double dx;
  double dy;
  //
  dx = __SEGGER_RTL_ldouble_to_double(x);
  dy = __SEGGER_RTL_ldouble_to_double(y);
  //
  return dx != dy ? 1 : 0;
}

/*********************************************************************
*
*       __lttf2()
*
*  Function description
*    Less than, long double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return < 0 if both operands are non-NaN and a < b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __lttf2(long double x, long double y) {
  double dx;
  double dy;
  //
  dx = __SEGGER_RTL_ldouble_to_double(x);
  dy = __SEGGER_RTL_ldouble_to_double(y);
  //
  return dx < dy ? -1 : 0;
}

/*********************************************************************
*
*       __letf2()
*
*  Function description
*    Less than or equal, long double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return <= 0 if both operands are non-NaN and a <= b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __letf2(long double x, long double y) {
  double dx;
  double dy;
  //
  dx = __SEGGER_RTL_ldouble_to_double(x);
  dy = __SEGGER_RTL_ldouble_to_double(y);
  //
  return dx <= dy ? -1 : 1;
}

/*********************************************************************
*
*       __gttf2()
*
*  Function description
*    Greater than, long double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return > 0 if both operands are non-NaN and a > b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __gttf2(long double x, long double y) {
  double dx;
  double dy;
  //
  dx = __SEGGER_RTL_ldouble_to_double(x);
  dy = __SEGGER_RTL_ldouble_to_double(y);
  //
  return dx > dy ? 1 : 0;
}

/*********************************************************************
*
*       __getf2()
*
*  Function description
*    Greater than or equal, long double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    Return >= 0 if both operands are non-NaN and a >= b (GNU
*    three-way boolean).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __getf2(long double x, long double y) {
  double dx;
  double dy;
  //
  dx = __SEGGER_RTL_ldouble_to_double(x);
  dy = __SEGGER_RTL_ldouble_to_double(y);
  //
  return dx >= dy ? 0 : -1;
}

/*********************************************************************
*
*       __extendsftf2()
*
*  Function description
*    Extend float to long double.
*
*  Parameters
*    x - Float value to extend.
*
*  Return value
*    Double value.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __extendsftf2(float x) {
  return __SEGGER_RTL_double_to_ldouble(__SEGGER_RTL_float_to_double(x));
}

/*********************************************************************
*
*       __extenddftf2()
*
*  Function description
*    Extend double to long double.
*
*  Parameters
*    x - Double value to extend.
*
*  Return value
*    Long double value.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API __extenddftf2(double x) {
  return __SEGGER_RTL_double_to_ldouble(x);
}

/*********************************************************************
*
*       __fixunstfsi()
*
*  Function description
*    Convert long double to int.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __fixunstfsi(long double x) {
  return (int)__SEGGER_RTL_ldouble_to_double(x);
}

/*********************************************************************
*
*       __fixunstfdi()
*
*  Function description
*    Convert long double to unsigned long long.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __fixunstfdi(long double x) {
  return __SEGGER_RTL_double_to_uint64(__SEGGER_RTL_ldouble_to_double(x));
}

/*********************************************************************
*
*       __trunctfdf2()
*
*  Function description
*    Truncate long double to double.
*
*  Parameters
*    x - Long double value to truncate.
*
*  Return value
*    Double value.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API __trunctfdf2(long double x) {
  return __SEGGER_RTL_ldouble_to_double(x);
}

/*********************************************************************
*
*       __trunctfsf2()
*
*  Function description
*    Truncate long double to float.
*
*  Parameters
*    x - Long double value to truncate.
*
*  Return value
*    Float value.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API __trunctfsf2(long double x) {
  return (float)__SEGGER_RTL_ldouble_to_double(x);
}

#endif

/*********************************************************************
*
*       __mulsc3()
*
*  Function description
*    Multiply, float complex.
*
*  Parameters
*    a - Real part of multiplicand.
*    b - Imaginary part of multiplicand.
*    c - Real part of multiplier.
*    d - Imaginary part of multiplier.
*
*  Return value
*    Product.
*
*  References
*    [C99] Section G.5.1, Multiplicative operators
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API __mulsc3(float a, float b, float c, float d) {
  __SEGGER_RTL_FLOAT32_COMPLEX x;
  __SEGGER_RTL_FLOAT32_COMPLEX y;
  //
  x.u.part.Re = a;
  x.u.part.Im = b;
  y.u.part.Re = c;
  y.u.part.Im = d;
  //
  __SEGGER_RTL_float32_cmul(&x, &y);
  //
  return x.u.value;
}

/*********************************************************************
*
*       __muldc3()
*
*  Function description
*    Multiply, double complex.
*
*  Parameters
*    a - Real part of multiplicand.
*    b - Imaginary part of multiplicand.
*    c - Real part of multiplier.
*    d - Imaginary part of multiplier.
*
*  Return value
*    Product.
*
*  References
*    [C99] Section G.5.1, Multiplicative operators
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API __muldc3(double a, double b, double c, double d) {
  __SEGGER_RTL_FLOAT64_COMPLEX x;
  __SEGGER_RTL_FLOAT64_COMPLEX y;
  //
  x.u.part.Re = a;
  x.u.part.Im = b;
  y.u.part.Re = c;
  y.u.part.Im = d;
  //
  __SEGGER_RTL_float64_cmul(&x, &y);
  //
  return x.u.value;
}

/*********************************************************************
*
*       __multc3()
*
*  Function description
*    Multiply, long double complex.
*
*  Parameters
*    a - Real part of multiplicand.
*    b - Imaginary part of multiplicand.
*    c - Real part of multiplier.
*    d - Imaginary part of multiplier.
*
*  Return value
*    Product.
*
*  References
*    [C99] Section G.5.1, Multiplicative operators
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API __multc3(long double a, long double b, long double c, long double d) {
  __SEGGER_RTL_FLOAT64_COMPLEX x;
  __SEGGER_RTL_FLOAT64_COMPLEX y;
  //
  x.u.part.Re = __SEGGER_RTL_ldouble_to_double(a);
  x.u.part.Im = __SEGGER_RTL_ldouble_to_double(b);
  y.u.part.Re = __SEGGER_RTL_ldouble_to_double(c);
  y.u.part.Im = __SEGGER_RTL_ldouble_to_double(d);
  //
  __SEGGER_RTL_float64_cmul(&x, &y);
  //
  return __SEGGER_RTL_float64_cmplx_inline(
           __SEGGER_RTL_double_to_ldouble(x.u.part.Re),
           __SEGGER_RTL_double_to_ldouble(x.u.part.Im));
}

/*********************************************************************
*
*       __divsc3()
*
*  Function description
*    Divide, float complex.
*
*  Parameters
*    a - Real part of dividend.
*    b - Imaginary part of dividend.
*    c - Real part of divisor.
*    d - Imaginary part of divisor.
*
*  Return value
*    Quotient.
*
*  References
*    [C99] Section G.5.1, Multiplicative operators
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API __divsc3(float a, float b, float c, float d) {
  __SEGGER_RTL_FLOAT32_COMPLEX x;
  __SEGGER_RTL_FLOAT32_COMPLEX y;
  //
  x.u.part.Re = a;
  x.u.part.Im = b;
  y.u.part.Re = c;
  y.u.part.Im = d;
  //
  __SEGGER_RTL_float32_cdiv(&x, &y);
  //
  return x.u.value;
}

/*********************************************************************
*
*       __divdc3()
*
*  Function description
*    Divide, double complex.
*
*  Parameters
*    a - Real part of dividend.
*    b - Imaginary part of dividend.
*    c - Real part of divisor.
*    d - Imaginary part of divisor.
*
*  Return value
*    Quotient.
*
*  References
*    [C99] Section G.5.1, Multiplicative operators
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API __divdc3(double a, double b, double c, double d) {
  __SEGGER_RTL_FLOAT64_COMPLEX x;
  __SEGGER_RTL_FLOAT64_COMPLEX y;
  //
  x.u.part.Re = a;
  x.u.part.Im = b;
  y.u.part.Re = c;
  y.u.part.Im = d;
  //
  __SEGGER_RTL_float64_cdiv_inline(&x, &y);
  //
  return x.u.value;
}

/*********************************************************************
*
*       __divtc3()
*
*  Function description
*    Divide, long double complex.
*
*  Parameters
*    a - Real part of dividend.
*    b - Imaginary part of dividend.
*    c - Real part of divisor.
*    d - Imaginary part of divisor.
*
*  Return value
*    Quotient.
*
*  References
*    [C99] Section G.5.1, Multiplicative operators
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API __divtc3(long double a, long double b, long double c, long double d) {
  __SEGGER_RTL_FLOAT64_COMPLEX x;
  __SEGGER_RTL_FLOAT64_COMPLEX y;
  //
  x.u.part.Re = __SEGGER_RTL_ldouble_to_double(a);
  x.u.part.Im = __SEGGER_RTL_ldouble_to_double(b);
  y.u.part.Re = __SEGGER_RTL_ldouble_to_double(c);
  y.u.part.Im = __SEGGER_RTL_ldouble_to_double(d);
  //
  __SEGGER_RTL_float64_cdiv_inline(&x, &y);
  //
  return __SEGGER_RTL_float64_cmplx_inline(
           __SEGGER_RTL_double_to_ldouble(x.u.part.Re),
           __SEGGER_RTL_double_to_ldouble(x.u.part.Im));
}

/*********************************************************************
*
*       Public code - SEGGER side-by-side functions
*
**********************************************************************
*/

#if __SEGGER_RTL_INCLUDE_SEGGER_API

/*********************************************************************
*
*       SEGGER_addf()
*
*  Function description
*    Add, float.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_addf(float x, float y) {
  return __SEGGER_RTL_float32_add(x, y);
}

/*********************************************************************
*
*       SEGGER_add()
*
*  Function description
*    Add, double.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_add(double x, double y) {
   return __SEGGER_RTL_float64_add(x, y);
}

/*********************************************************************
*
*       SEGGER_subf()
*
*  Function description
*    Subtract, float.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_subf(float x, float y) {
  return __SEGGER_RTL_float32_sub(x, y);
}

/*********************************************************************
*
*       SEGGER_sub()
*
*  Function description
*    Subtract, double.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_sub(double x, double y) {
  return __SEGGER_RTL_float64_sub(x, y);
}

/*********************************************************************
*
*       SEGGER_mulf()
*
*  Function description
*    Multiply, float.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_mulf(float x, float y) {
  return __SEGGER_RTL_float32_mul(x, y);
}

/*********************************************************************
*
*       SEGGER_mul()
*
*  Function description
*    Multiply, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_mul(double x, double y) {
  return __SEGGER_RTL_float64_mul(x, y);
}

/*********************************************************************
*
*       SEGGER_divf()
*
*  Function description
*    Divide, float.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_divf(float x, float y) {
  return __SEGGER_RTL_float32_div(x, y);
}

/*********************************************************************
*
*       SEGGER_div()
*
*  Function description
*    Divide, double.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_div(double x, double y) {
  return __SEGGER_RTL_float64_div(x, y);
}

/*********************************************************************
*
*       SEGGER_negf()
*
*  Function description
*    Negate, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Negated argument.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_negf(float x) {
  return __SEGGER_RTL_float32_signbit_xor(x, -0.0f);
}

/*********************************************************************
*
*       SEGGER_neg()
*
*  Function description
*    Negate, double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Negated argument.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_neg(double x) {
  return __SEGGER_RTL_float64_signbit_xor(x, -0.0);
}

/*********************************************************************
*
*       SEGGER_eqf()
*
*  Function description
*    Equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not equal to y.
*    1 -- x is equal to y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_eqf(float x, float y) {
  return __SEGGER_RTL_float32_eq(x, y);
}

/*********************************************************************
*
*       SEGGER_eq()
*
*  Function description
*    Equal, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not equal to y.
*    1 -- x is equal to y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_eq(double x, double y) {
  return __SEGGER_RTL_float64_eq(x, y);
}

/*********************************************************************
*
*       SEGGER_ne()
*
*  Function description
*    Not equal, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is equal to y.
*    1 -- x is not equal to y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_ne(double x, double y) {
  return __SEGGER_RTL_float64_ne(x, y);
}

/*********************************************************************
*
*       SEGGER_nef()
*
*  Function description
*    Not equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not equal to y.
*    1 -- x is equal to y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_nef(float x, float y) {
  return __SEGGER_RTL_float32_ne(x, y);
}

/*********************************************************************
*
*       SEGGER_ltf()
*
*  Function description
*    Less than, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not less than y.
*    1 -- x is less than y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_ltf(float x, float y) {
  return __SEGGER_RTL_float32_lt(x, y);
}

/*********************************************************************
*
*       SEGGER_lt()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not less than y.
*    1 -- x is less than y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_lt(double x, double y) {
  return __SEGGER_RTL_float64_lt(x, y);
}

/*********************************************************************
*
*       SEGGER_lef()
*
*  Function description
*    Less than or equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not less than or equal to y.
*    1 -- x is less than or equal to y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_lef(float x, float y) {
  return __SEGGER_RTL_float32_le(x, y);
}

/*********************************************************************
*
*       SEGGER_le()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not less than or equal to y.
*    1 -- x is less than or equal to y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_le(double x, double y) {
  return __SEGGER_RTL_float64_le(x, y);
}

/*********************************************************************
*
*       SEGGER_gtf()
*
*  Function description
*    Less than, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not greater than y.
*    1 -- x is greater than y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_gtf(float x, float y) {
  return __SEGGER_RTL_float32_gt(x, y);
}

/*********************************************************************
*
*       SEGGER_gt()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not greater than y.
*    1 -- x is greater than y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_gt(double x, double y) {
  return __SEGGER_RTL_float64_gt(x, y);
}

/*********************************************************************
*
*       SEGGER_gef()
*
*  Function description
*    Less than, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not greater than or equal to y.
*    1 -- x is greater than or equal to y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_gef(float x, float y) {
  return __SEGGER_RTL_float32_ge(x, y);
}

/*********************************************************************
*
*       SEGGER_ge()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    0 -- x is not greater than or equal to y.
*    1 -- x is greater than or equal to y.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_ge(double x, double y) {
  return __SEGGER_RTL_float64_ge(x, y);
}

/*********************************************************************
*
*       SEGGER_isnanf()
*
*  Function description
*    Not-a-number query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Is not a NaN.
*    != 0 - Is a NaN.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_isnanf(float x) {
  return __SEGGER_RTL_float32_isnan_inline(x);
}

/*********************************************************************
*
*       SEGGER_isinff()
*
*  Function description
*    Infinity query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not infinite.
*    != 0 - Is infinite.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_isinff(float x) {
  return __SEGGER_RTL_float32_isinf_inline(x);
}

/*********************************************************************
*
*       SEGGER_isfinitef()
*
*  Function description
*    Finite query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not finite.
*    != 0 - Is finite.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_isfinitef(float x) {
  return __SEGGER_RTL_float32_isfinite_inline(x);
}

/*********************************************************************
*
*       SEGGER_isnormalf()
*
*  Function description
*    Normal query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not normal.
*    != 0 - Is normal.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_isnormalf(float x) {
  return __SEGGER_RTL_float32_isnormal_inline(x);
}

/*********************************************************************
*
*       SEGGER_classifyf()
*
*  Function description
*    Classify, float.
*
*  Parameters
*    x - Value to classify.
*
*  Return value
*    SEGGER_FP_ZERO      -- Value is +0 or -0.
*    SEGGER_FP_NAN       -- Value is not-a-number.
*    SEGGER_FP_INFINITE  -- Value is +Inf or -Inf.
*    SEGGER_FP_NORMAL    -- Value is a normal and finite.
*    SEGGER_FP_SUBNORMAL -- Value is a subnormal and finite.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_classifyf(float x) {
  if (__SEGGER_RTL_float32_iszero_inline(x)) {
    return __SEGGER_RTL_FP_ZERO;
  } else if (__SEGGER_RTL_float32_isnan_inline(x)) {
    return __SEGGER_RTL_FP_NAN;
  } else if (__SEGGER_RTL_float32_isinf_inline(x)) {
    return __SEGGER_RTL_FP_INFINITE;
  } else if (__SEGGER_RTL_float32_isnormal_inline(x)) {
    return __SEGGER_RTL_FP_NORMAL;
  } else {
    return __SEGGER_RTL_FP_SUBNORMAL;
  }
}

/*********************************************************************
*
*       SEGGER_isnan()
*
*  Function description
*    Not-a-number query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Is not a NaN.
*    != 0 - Is a NaN.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_isnan(double x) {
  return __SEGGER_RTL_float64_isnan_inline(x);
}

/*********************************************************************
*
*       SEGGER_isinf()
*
*  Function description
*    Infinity query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Not infinite.
*    != 0 - Is infinite.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_isinf(double x) {
  return __SEGGER_RTL_float64_isinf_inline(x);
}

/*********************************************************************
*
*       SEGGER_isfinite()
*
*  Function description
*    Finite query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Not finite.
*    != 0 - Is finite.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_isfinite(double x) {
  return __SEGGER_RTL_float64_isfinite_inline(x);
}

/*********************************************************************
*
*       SEGGER_isnormal()
*
*  Function description
*    Normal query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Not normal.
*    != 0 - Is normal.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_isnormal(double x) {
  return __SEGGER_RTL_float64_isnormal_inline(x);
}

/*********************************************************************
*
*       SEGGER_classify()
*
*  Function description
*    Classify, double.
*
*  Parameters
*    x - Value to classify.
*
*  Return value
*    SEGGER_FP_ZERO      -- Value is +0 or -0.
*    SEGGER_FP_NAN       -- Value is not-a-number.
*    SEGGER_FP_INFINITE  -- Value is +Inf or -Inf.
*    SEGGER_FP_NORMAL    -- Value is a normal and finite.
*    SEGGER_FP_SUBNORMAL -- Value is a subnormal and finite.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_classify(double x) {
  if (__SEGGER_RTL_float64_iszero_inline(x)) {
    return __SEGGER_RTL_FP_ZERO;
  } else if (__SEGGER_RTL_float64_isnan_inline(x)) {
    return __SEGGER_RTL_FP_NAN;
  } else if (__SEGGER_RTL_float64_isinf_inline(x)) {
    return __SEGGER_RTL_FP_INFINITE;
  } else if (__SEGGER_RTL_float64_isnormal_inline(x)) {
    return __SEGGER_RTL_FP_NORMAL;
  } else {
    return __SEGGER_RTL_FP_SUBNORMAL;
  }
}

/*********************************************************************
*
*       SEGGER_signbitf()
*
*  Function description
*    Extract sign bit, float.
*
*  Parameters
*    x - Value to extract from as float.
*
*  Return value
*    == 0 - Sign bit is zero (positive).
*    == 1 - Sign bit is one (negative).
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_signbitf(float x) {
  return __SEGGER_RTL_float32_signbit_soft(__SEGGER_RTL_BitcastToU32(x));
}

/*********************************************************************
*
*       SEGGER_signbit()
*
*  Function description
*    Extract sign bit, double.
*
*  Parameters
*    x - Value to extract from as double.
*
*  Return value
*    == 0 - Sign bit is zero (positive).
*    == 1 - Sign bit is one (negative).
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_signbit(double x) {
  return __SEGGER_RTL_float64_signbit_soft(__SEGGER_RTL_BitcastToU64(x));
}

/*********************************************************************
*
*       SEGGER_copysignf()
*
*  Function description
*    Copy sign, float.
*
*  Parameters
*    x - Floating value to inject sign into.
*    y - Floating value carrying the sign to inject.
*
*  Return value
*    x with the sign of y.
*
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_copysignf(float x, float y) {
  return __SEGGER_RTL_float32_signbit_copy(x, y);
}

/*********************************************************************
*
*       SEGGER_copysign()
*
*  Function description
*    Copy sign, double.
*
*  Parameters
*    x - Floating value to inject sign into.
*    y - Floating value carrying the sign to inject.
*
*  Return value
*    x with the sign of y.
*
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_copysign(double x, double y) {
  return __SEGGER_RTL_float64_signbit_copy(x, y);
}

/*********************************************************************
*
*       SEGGER_float_to_int32()
*
*  Function description
*    Convert float to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
__SEGGER_RTL_I32 __SEGGER_RTL_PUBLIC_API SEGGER_float_to_int32(float x) {
  return __SEGGER_RTL_float_to_int32(x);
}

/*********************************************************************
*
*       SEGGER_double_to_int32()
*
*  Function description
*    Convert double to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
__SEGGER_RTL_I32 __SEGGER_RTL_PUBLIC_API SEGGER_double_to_int32(double x) {
  return __SEGGER_RTL_double_to_int32(x);
}

/*********************************************************************
*
*       SEGGER_float_to_uint32()
*
*  Function description
*    Convert float to __SEGGER_RTL_U32.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API SEGGER_float_to_uint32(float x) {
  return __SEGGER_RTL_float_to_uint32(x);
}

/*********************************************************************
*
*       SEGGER_double_to_uint32()
*
*  Function description
*    Convert double to unsigned.
*
*  Parameters
*    x - Float value to convert.
*
*  Return value
*    Integerized value.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API SEGGER_double_to_uint32(double x) {
  return __SEGGER_RTL_double_to_uint32(x);
}

/*********************************************************************
*
*       SEGGER_float_to_int64()
*
*  Function description
*    Convert float to long long.
*
*  Parameters
*    f - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    The RV32 compiler converts a float to a 64-bit integer
*    by calling runtime support to handle it.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API SEGGER_float_to_int64(float f) {
  return __SEGGER_RTL_float_to_int64(f);
}

/*********************************************************************
*
*       SEGGER_double_to_int64()
*
*  Function description
*    Convert double to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    RV32 always calls runtime for double to int64 conversion.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API SEGGER_double_to_int64(double x) {
  return __SEGGER_RTL_double_to_int64(x);
}

/*********************************************************************
*
*       SEGGER_float_to_uint64()
*
*  Function description
*    Convert float to __SEGGER_RTL_U64.
*
*  Parameters
*    f - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API SEGGER_float_to_uint64(float f) {
  return __SEGGER_RTL_float_to_uint64(f);
}

/*********************************************************************
*
*       SEGGER_double_to_uint64()
*
*  Function description
*    Convert double to __SEGGER_RTL_U64.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    RV32 always calls runtime for double to int64 conversion.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API SEGGER_double_to_uint64(double x) {
  return __SEGGER_RTL_double_to_uint64(x);
}

/*********************************************************************
*
*       SEGGER_int32_to_float()
*
*  Function description
*    Convert int to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_int32_to_float(__SEGGER_RTL_I32 x) {
  return __SEGGER_RTL_int32_to_float(x);
}

/*********************************************************************
*
*       SEGGER_int32_to_double()
*
*  Function description
*    Convert int to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_int32_to_double(__SEGGER_RTL_I32 x) {
  return __SEGGER_RTL_int32_to_double(x);
}

/*********************************************************************
*
*       SEGGER_uint32_to_float()
*
*  Function description
*    Convert unsigned to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_uint32_to_float(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_uint32_to_float(x);
}

/*********************************************************************
*
*       SEGGER_uint32_to_double()
*
*  Function description
*    Convert unsigned to double.
*
*  Parameters
*    x - Unsigned value to convert.
*
*  Return value
*    Double value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_uint32_to_double(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_uint32_to_double(x);
}

/*********************************************************************
*
*       SEGGER_int64_to_float()
*
*  Function description
*    Convert long long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_int64_to_float(__SEGGER_RTL_I64 x) {
  return __SEGGER_RTL_int64_to_float(x);
}

/*********************************************************************
*
*       SEGGER_int64_to_double()
*
*  Function description
*    Convert long long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_int64_to_double(__SEGGER_RTL_I64 x) {
  return __SEGGER_RTL_int64_to_double(x);
}

/*********************************************************************
*
*       SEGGER_uint64_to_float()
*
*  Function description
*    Convert unsigned long long to float.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Float value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_uint64_to_float(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_uint64_to_float(x);
}

/*********************************************************************
*
*       SEGGER_uint64_to_double()
*
*  Function description
*    Convert unsigned long long to double.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    Double value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_uint64_to_double(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_uint64_to_double(x);
}

/*********************************************************************
*
*       SEGGER_float_to_double()
*
*  Function description
*    Extend float to double.
*
*  Parameters
*    x - Float value to extend.
*
*  Return value
*    Double value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_float_to_double(float x) {
  return __SEGGER_RTL_float_to_double(x);
}

/*********************************************************************
*
*       SEGGER_double_to_float()
*
*  Function description
*    Truncate double to float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Float value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_double_to_float(double x) {
  return __SEGGER_RTL_double_to_float(x);
}

/*********************************************************************
*
*       SEGGER_float_to_int()
*
*  Function description
*    Convert float to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_float_to_int(float x) {
  return sizeof(int) <= 4 ? (int)SEGGER_float_to_int32(x)
                          : (int)SEGGER_float_to_int64(x);
}

/*********************************************************************
*
*       SEGGER_int_to_float()
*
*  Function description
*    Convert float to int.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_int_to_float(int x) {
  return sizeof(int) <= 4 ? __SEGGER_RTL_int32_to_float((__SEGGER_RTL_I32)x)
                          : __SEGGER_RTL_int64_to_float((__SEGGER_RTL_I64)x);
}

/*********************************************************************
*
*       SEGGER_double_to_int()
*
*  Function description
*    Convert double to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_double_to_int(double x) {
  return sizeof(int) <= 4 ? (int)__SEGGER_RTL_double_to_int32(x)
                          : (int)__SEGGER_RTL_double_to_int64(x);
}

/*********************************************************************
*
*       SEGGER_int_to_double()
*
*  Function description
*    Convert double to int.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_int_to_double(int x) {
  return sizeof(int) <= 4 ? __SEGGER_RTL_int32_to_double((__SEGGER_RTL_I32)x)
                          : __SEGGER_RTL_int64_to_double((__SEGGER_RTL_I64)x);
}

/*********************************************************************
*
*       SEGGER_float_to_uint()
*
*  Function description
*    Convert float to unsigned.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
unsigned __SEGGER_RTL_PUBLIC_API SEGGER_float_to_uint(float x) {
  return sizeof(unsigned) <= 4 ? (unsigned)__SEGGER_RTL_float_to_uint32(x)
                               : (unsigned)__SEGGER_RTL_float_to_uint64(x);
}

/*********************************************************************
*
*       SEGGER_uint_to_float()
*
*  Function description
*    Convert unsigned to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_uint_to_float(unsigned x) {
  return sizeof(unsigned) <= 4 ? __SEGGER_RTL_uint32_to_float((__SEGGER_RTL_U32)x)
                               : __SEGGER_RTL_uint64_to_float((__SEGGER_RTL_U64)x);
}

/*********************************************************************
*
*       SEGGER_double_to_uint()
*
*  Function description
*    Convert double to unsigned.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
unsigned __SEGGER_RTL_PUBLIC_API SEGGER_double_to_uint(double x) {
  return sizeof(unsigned) <= 4 ? (unsigned)__SEGGER_RTL_double_to_uint32(x)
                               : (unsigned)__SEGGER_RTL_double_to_uint64(x);
}

/*********************************************************************
*
*       SEGGER_uint_to_double()
*
*  Function description
*    Convert unsigned to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_uint_to_double(unsigned x) {
  return sizeof(unsigned) <= 4 ? __SEGGER_RTL_uint32_to_double((__SEGGER_RTL_U32)x)
                               : __SEGGER_RTL_uint64_to_double((__SEGGER_RTL_U64)x);
}

/*********************************************************************
*
*       SEGGER_float_to_long()
*
*  Function description
*    Convert float to long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
long __SEGGER_RTL_PUBLIC_API SEGGER_float_to_long(float x) {
  return sizeof(long) <= 4 ? (long)__SEGGER_RTL_float_to_int32(x)
                           : (long)__SEGGER_RTL_float_to_int64(x);
}

/*********************************************************************
*
*       SEGGER_long_to_float()
*
*  Function description
*    Convert long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_long_to_float(long x) {
  return sizeof(long) <= 4 ? __SEGGER_RTL_int32_to_float((__SEGGER_RTL_I32)x)
                           : __SEGGER_RTL_int64_to_float((__SEGGER_RTL_I64)x);
}

/*********************************************************************
*
*       SEGGER_double_to_long()
*
*  Function description
*    Convert double to long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
long __SEGGER_RTL_PUBLIC_API SEGGER_double_to_long(double x) {
  return sizeof(long) <= 4 ? (long)__SEGGER_RTL_double_to_int32(x)
                           : (long)__SEGGER_RTL_double_to_int64(x);
}

/*********************************************************************
*
*       SEGGER_long_to_double()
*
*  Function description
*    Convert long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_long_to_double(long x) {
  return sizeof(long) <= 4 ? __SEGGER_RTL_int32_to_double((__SEGGER_RTL_I32)x)
                           : __SEGGER_RTL_int64_to_double((__SEGGER_RTL_I64)x);
}

/*********************************************************************
*
*       SEGGER_float_to_ulong()
*
*  Function description
*    Convert float to unsigned long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
unsigned long __SEGGER_RTL_PUBLIC_API SEGGER_float_to_ulong(float x) {
  return sizeof(unsigned long) <= 4 ? (unsigned long)__SEGGER_RTL_float_to_uint32(x)
                                    : (unsigned long)__SEGGER_RTL_float_to_uint64(x);
}

/*********************************************************************
*
*       SEGGER_ulong_to_float()
*
*  Function description
*    Convert unsigned long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_ulong_to_float(unsigned long x) {
  return sizeof(long) <= 4 ? __SEGGER_RTL_uint32_to_float((__SEGGER_RTL_U32)x)
                           : __SEGGER_RTL_uint64_to_float((__SEGGER_RTL_U64)x);
}

/*********************************************************************
*
*       SEGGER_double_to_ulong()
*
*  Function description
*    Convert double to unsigned long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
unsigned long __SEGGER_RTL_PUBLIC_API SEGGER_double_to_ulong(double x) {
  return sizeof(unsigned long) <= 4 ? (unsigned long)__SEGGER_RTL_double_to_uint32(x)
                                    : (unsigned long)__SEGGER_RTL_double_to_uint64(x);
}

/*********************************************************************
*
*       SEGGER_ulong_to_double()
*
*  Function description
*    Convert unsigned long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_ulong_to_double(unsigned long x) {
  return sizeof(unsigned long) <= 4 ? __SEGGER_RTL_uint32_to_double((__SEGGER_RTL_U32)x)
                                    : __SEGGER_RTL_uint64_to_double((__SEGGER_RTL_U64)x);
}

/*********************************************************************
*
*       SEGGER_float_to_llong()
*
*  Function description
*    Convert float to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
long long __SEGGER_RTL_PUBLIC_API SEGGER_float_to_llong(float x) {
  return __SEGGER_RTL_float_to_int64(x);
}

/*********************************************************************
*
*       SEGGER_llong_to_float()
*
*  Function description
*    Convert long long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_llong_to_float(long long x) {
  return __SEGGER_RTL_int64_to_float(x);
}

/*********************************************************************
*
*       SEGGER_double_to_llong()
*
*  Function description
*    Convert double to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
long long __SEGGER_RTL_PUBLIC_API SEGGER_double_to_llong(double x) {
  return __SEGGER_RTL_double_to_int64(x);
}

/*********************************************************************
*
*       SEGGER_llong_to_double()
*
*  Function description
*    Convert long long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_llong_to_double(long long x) {
  return __SEGGER_RTL_int64_to_double(x);
}

/*********************************************************************
*
*       SEGGER_float_to_ullong()
*
*  Function description
*    Convert float to unsigned long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
unsigned long long __SEGGER_RTL_PUBLIC_API SEGGER_float_to_ullong(float x) {
  return __SEGGER_RTL_float_to_uint64(x);
}

/*********************************************************************
*
*       SEGGER_ullong_to_float()
*
*  Function description
*    Convert unsigned long long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_ullong_to_float(unsigned long long x) {
  return __SEGGER_RTL_uint64_to_float(x);
}

/*********************************************************************
*
*       SEGGER_double_to_ullong()
*
*  Function description
*    Convert double to unsigned long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*/
unsigned long long __SEGGER_RTL_PUBLIC_API SEGGER_double_to_ullong(double x) {
  return __SEGGER_RTL_double_to_uint64(x);
}

/*********************************************************************
*
*       SEGGER_ullong_to_double()
*
*  Function description
*    Convert unsigned long long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_ullong_to_double(unsigned long long x) {
  return __SEGGER_RTL_uint64_to_double(x);
}

/*********************************************************************
*
*       SEGGER_modff()
*
*  Function description
*    Separate integer and fractional parts, float.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modff() returns the signed
*    fractional part of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_modff(float x, float *iptr) {
  return __SEGGER_RTL_float32_modf_inline(x, iptr);
}

/*********************************************************************
*
*       SEGGER_modf()
*
*  Function description
*    Separate integer and fractional parts, double.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modf() returns the signed
*    fractional part of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_modf(double x, double *iptr) {
  return __SEGGER_RTL_float64_modf_inline(x, iptr);
}

/*********************************************************************
*
*       SEGGER_fabs()
*
*  Function description
*    Compute absolute value, double.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_fabs(double x) {
  return __SEGGER_RTL_float64_abs_inline(x);
}

/*********************************************************************
*
*       SEGGER_fabsf()
*
*  Function description
*    Compute absolute value, float.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_fabsf(float x) {
  return __SEGGER_RTL_float32_abs_inline(x);
}

/*********************************************************************
*
*       SEGGER_fmaf()
*
*  Function description
*    Compute fused multiply-add, float.
*
*  Parameters
*    x - Multiplier.
*    y - Multiplicand.
*    z - Summand.
*
*  Return value
*    Return (x * y) + z.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_fmaf(float x, float y, float z) {
  return __SEGGER_RTL_float32_fma_inline(x, y, z);
}

/*********************************************************************
*
*       SEGGER_fma()
*
*  Function description
*    Compute fused multiply-add, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*    z - Summand.
*
*  Return value
*    Return (x * y) + z.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_fma(double x, double y, double z) {
  return __SEGGER_RTL_float64_fma_inline(x, y, z);
}

/*********************************************************************
*
*       SEGGER_fmsf()
*
*  Function description
*    Compute fused multiply-subtract, float.
*
*  Parameters
*    x - Multiplier.
*    y - Multiplicand.
*    z - Summand.
*
*  Return value
*    Return (x * y) - z.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_fmsf(float x, float y, float z) {
  return __SEGGER_RTL_float32_fma_inline(x, y, SEGGER_NEGF(z));
}

/*********************************************************************
*
*       SEGGER_fms()
*
*  Function description
*    Compute fused multiply-subtract, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*    z - Summand.
*
*  Return value
*    Return (x * y) - z.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_fms(double x, double y, double z) {
  return __SEGGER_RTL_float64_fma_inline(x, y, SEGGER_NEG(z));
}

/*********************************************************************
*
*       SEGGER_ldexpf()
*
*  Function description
*    Scale by power of two, float.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is zero, return x;
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2^n.
*
*  See also
*    scalbnf()
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_ldexpf(float x, int n) {
  return __SEGGER_RTL_float32_ldexp_inline(x, n);
}

/*********************************************************************
*
*       SEGGER_ldexp()
*
*  Function description
*    Scale by power of two, double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is zero, return x;
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2 ^ n.
*
*  See also
*    scalbn()
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_ldexp(double x, int n) {
  return __SEGGER_RTL_float64_ldexp_inline(x, n);
}

/*********************************************************************
*
*       SEGGER_frexpf()
*
*  Function description
*    Split to significand and exponent, float.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_frexpf(float x, int *exp) {
  return __SEGGER_RTL_float32_frexp_inline(x, exp);
}

/*********************************************************************
*
*       SEGGER_frexp()
*
*  Function description
*    Split to significand and exponent, double.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_frexp(double x, int *exp) {
  return __SEGGER_RTL_float64_frexp_inline(x, exp);
}

/*********************************************************************
*
*       SEGGER_ilogbf()
*
*  Function description
*    Radix-independent exponent, float.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_ilogbf(float x) {
  return __SEGGER_RTL_float32_ilogb_inline(x);
}

/*********************************************************************
*
*       SEGGER_ilogb()
*
*  Function description
*    Radix-independent exponent, double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*/
int __SEGGER_RTL_PUBLIC_API SEGGER_ilogb(double x) {
  return __SEGGER_RTL_float64_ilogb_inline(x);
}

/*********************************************************************
*
*       SEGGER_ceilf()
*
*  Function description
*    Compute smallest integer not less than, float.
*
*  Parameters
*    x - Value to compute ceiling of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the smallest integer value not greater than x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_ceilf(float x) {
  return __SEGGER_RTL_float32_ceil_inline(x);
}

/*********************************************************************
*
*       SEGGER_ceil()
*
*  Function description
*    Compute smallest integer not less than, double.
*
*  Parameters
*    x - Value to compute ceiling of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the smallest integer value not greater than x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_ceil(double x) {
  return __SEGGER_RTL_float64_ceil_inline(x);
}

/*********************************************************************
*
*       SEGGER_floorf()
*
*  Function description
*    Compute largest integer not greater than, float.
*
*  Parameters
*    x - Value to floor.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the largest integer value not greater than x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_floorf(float x) {
  return __SEGGER_RTL_float32_floor_inline(x);
}

/*********************************************************************
*
*       SEGGER_floor()
*
*  Function description
*    Compute largest integer not greater than, double.
*
*  Parameters
*    x - Value to floor.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the largest integer value not greater than x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_floor(double x) {
  return __SEGGER_RTL_float64_floor_inline(x);
}

/*********************************************************************
*
*       SEGGER_sinf()
*
*  Function description
*    Calculate sine, float.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_sinf(float x) {
  return __SEGGER_RTL_float32_sin_inline(x);
}

/*********************************************************************
*
*       SEGGER_sin()
*
*  Function description
*    Calculate sine, double.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_sin(double x) {
  return __SEGGER_RTL_float64_sin_inline(x);
}

/*********************************************************************
*
*       SEGGER_cosf()
*
*  Function description
*    Calculate cosine, float.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_cosf(float x) {
  return __SEGGER_RTL_float32_cos_inline(x);
}

/*********************************************************************
*
*       SEGGER_cos()
*
*  Function description
*    Calculate cosine, double.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_cos(double x) {
  return __SEGGER_RTL_float64_cos_inline(x);
}

/*********************************************************************
*
*       SEGGER_tanf()
*
*  Function description
*    Compute tangent, float.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_tanf(float x) {
  return __SEGGER_RTL_float32_tan_inline(x);
}

/*********************************************************************
*
*       SEGGER_tan()
*
*  Function description
*    Compute tangent, double.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_tan(double x) {
  return __SEGGER_RTL_float64_tan_inline(x);
}

/*********************************************************************
*
*       SEGGER_asinf()
*
*  Function description
*    Compute inverse sine, float.
*
*  Parameters
*    x - Value to compute inverse sine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular sine of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular sine of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_asinf(float x) {
  return __SEGGER_RTL_float32_asin_inline(x);
}

/*********************************************************************
*
*       SEGGER_asin()
*
*  Function description
*    Compute inverse sine, double.
*
*  Parameters
*    x - Value to compute inverse sine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular sine of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular sine of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_asin(double x) {
  return __SEGGER_RTL_float64_asin_inline(x);
}

/*********************************************************************
*
*       SEGGER_acosf()
*
*  Function description
*    Compute inverse cosine, float.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_acosf(float x) {
  return __SEGGER_RTL_float32_acos_inline(x);
}

/*********************************************************************
*
*       SEGGER_acos()
*
*  Function description
*    Compute inverse cosine, double.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_acos(double x) {
  return __SEGGER_RTL_float64_acos_inline(x);
}

/*********************************************************************
*
*       SEGGER_atanf()
*
*  Function description
*    Compute inverse tangent, float.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_atanf(float x) {
  return __SEGGER_RTL_float32_atan_inline(x);
}

/*********************************************************************
*
*       SEGGER_atan()
*
*  Function description
*    Compute inverse tangent, double.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_atan(double x) {
  return __SEGGER_RTL_float64_atan_inline(x);
}

/*********************************************************************
*
*       SEGGER_atan2f()
*
*  Function description
*    Compute inverse tangent, with quadrant, float.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_atan2f(float y, float x) {
  return __SEGGER_RTL_float32_atan2_inline(y, x);
}

/*********************************************************************
*
*       SEGGER_atan2()
*
*  Function description
*    Compute inverse tangent, with quadrant, double.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_atan2(double y, double x) {
  return __SEGGER_RTL_float64_atan2_inline(y, x);
}

/*********************************************************************
*
*       SEGGER_sqrtf()
*
*  Function description
*    Compute square root, float.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    sqrt() computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrt() deviates and always uses IEC 60559 semantics. 
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_sqrtf(float x) {
  return __SEGGER_RTL_float32_sqrt_inline(x);
}

/*********************************************************************
*
*       SEGGER_sqrt()
*
*  Function description
*    Compute square root, double.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    sqrt() computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrt() deviates and always uses IEC 60559 semantics. 
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_sqrt(double x) {
  return __SEGGER_RTL_float64_sqrt_inline(x);
}

/*********************************************************************
*
*       SEGGER_cbrtf()
*
*  Function description
*    Compute cube root, float.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_cbrtf(float x) {
  return __SEGGER_RTL_float32_cbrt(x);
}

/*********************************************************************
*
*       SEGGER_cbrt()
*
*  Function description
*    Compute cube root, double.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_cbrt(double x) {
  return __SEGGER_RTL_float64_cbrt(x);
}

/*********************************************************************
*
*       SEGGER_rsqrtf()
*
*  Function description
*    Compute reciprocal square root, float.
*
*  Parameters
*    x - Value to compute reciprocal square root of.
*
*  Return value
*    * If x is +/-zero, return +/-infinity.
*    * If x is positively infinite, return 0.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return reciprocal square root of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_rsqrtf(float x) {
  return __SEGGER_RTL_float32_rsqrt_inline(x);
}

/*********************************************************************
*
*       SEGGER_rsqrt()
*
*  Function description
*    Compute reciprocal square root, double.
*
*  Parameters
*    x - Value to compute reciprocal square root of.
*
*  Return value
*    * If x is +/-zero, return +/-infinity.
*    * If x is positively infinite, return 0.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return reciprocal square root of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_rsqrt(double x) {
  return __SEGGER_RTL_float64_rsqrt_inline(x);
}

/*********************************************************************
*
*       SEGGER_fmodf()
*
*  Function description
*    Compute remainder after division, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_fmodf(float x, float y) {
  return __SEGGER_RTL_float32_fmod_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_fmod()
*
*  Function description
*    Compute remainder after division, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_fmod(double x, double y) {
  return __SEGGER_RTL_float64_fmod_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_fmaxf()
*
*  Function description
*    Compute maximum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_fmaxf(float x, float y) {
  return __SEGGER_RTL_float32_fmax_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_fmax()
*
*  Function description
*    Compute maximum, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_fmax(double x, double y) {
  return __SEGGER_RTL_float64_fmax_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_fminf()
*
*  Function description
*    Compute minimum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_fminf(float x, float y) {
  return __SEGGER_RTL_float32_fmin_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_fmin()
*
*  Function description
*    Compute minimum, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_fmin(double x, double y) {
  return __SEGGER_RTL_float64_fmin_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_fdimf()
*
*  Function description
*    Positive difference, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x > y, x-y.
*    * Else, +0.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_fdimf(float x, float y) {
  return __SEGGER_RTL_float32_fdim_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_fdim()
*
*  Function description
*    Positive difference, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x > y, x-y.
*    * Else, +0.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_fdim(double x, double y) {
  return __SEGGER_RTL_float64_fdim_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_hypotf()
*
*  Function description
*    Compute magnitude of complex, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrt(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_hypotf(float x, float y) {
  return __SEGGER_RTL_float32_hypot_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_hypot()
*
*  Function description
*    Compute magnitude of complex, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrt(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_hypot(double x, double y) {
  return __SEGGER_RTL_float64_hypot_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_logf()
*
*  Function description
*    Compute natural logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_logf(float x) {
  return __SEGGER_RTL_float32_log_inline(x);
}

/*********************************************************************
*
*       SEGGER_log()
*
*  Function description
*    Compute natural logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_log(double x) {
  return __SEGGER_RTL_float64_log_inline(x);
}
  
/*********************************************************************
*
*       SEGGER_log1pf()
*
*  Function description
*    Compute natural logarithm plus one, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_log1pf(float x) {
  return __SEGGER_RTL_float32_log1p_inline(x);
}

/*********************************************************************
*
*       SEGGER_log1p()
*
*  Function description
*    Compute natural logarithm plus one, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_log1p(double x) {
  return __SEGGER_RTL_float64_log1p_inline(x);
}

/*********************************************************************
*
*       SEGGER_log2f()
*
*  Function description
*    Compute base-2 logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_log2f(float x) {
  return __SEGGER_RTL_float32_log2(x);
}

/*********************************************************************
*
*       SEGGER_log2()
*
*  Function description
*    Compute base-2 logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_log2(double x) {
  return __SEGGER_RTL_float64_log2(x);
}

/*********************************************************************
*
*       SEGGER_log10f()
*
*  Function description
*    Compute common logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_log10f(float x) {
  return __SEGGER_RTL_float32_log10_inline(x);
}

/*********************************************************************
*
*       SEGGER_log10()
*
*  Function description
*    Compute common logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_log10(double x) {
  return __SEGGER_RTL_float64_log10_inline(x);
}

/*********************************************************************
*
*       SEGGER_expf()
*
*  Function description
*    Compute base-e exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_expf(float x) {
  return __SEGGER_RTL_float32_exp_inline(x);
}

/*********************************************************************
*
*       SEGGER_exp()
*
*  Function description
*    Compute base-e exponential, double.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_exp(double x) {
  return __SEGGER_RTL_float64_exp_inline(x);
}

/*********************************************************************
*
*       SEGGER_exp2f()
*
*  Function description
*    Compute base-2 exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_exp2f(float x) {
  return __SEGGER_RTL_float32_pow_inline(2.0f, x);
}

/*********************************************************************
*
*       SEGGER_exp2()
*
*  Function description
*    Compute base-2 exponential, double.
*
*  Parameters
*    x - Value to compute base-2 exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_exp2(double x) {
  return __SEGGER_RTL_float64_pow_inline(2.0, x);
}

/*********************************************************************
*
*       SEGGER_exp10f()
*
*  Function description
*    Compute base-10 exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_exp10f(float x) {
  return __SEGGER_RTL_float32_exp10_inline(x);
}

/*********************************************************************
*
*       SEGGER_exp10()
*
*  Function description
*    Compute base-10 exponential, double.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_exp10(double x) {
  return __SEGGER_RTL_float64_exp10_inline(x);
}

/*********************************************************************
*
*       SEGGER_powf()
*
*  Function description
*    Raise to power, float.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_powf(float x, float y) {
  return __SEGGER_RTL_float32_pow_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_pow()
*
*  Function description
*    Raise to power, double.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_pow(double x, double y) {
  return __SEGGER_RTL_float64_pow_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_sinhf()
*
*  Function description
*    Compute hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_sinhf(float x) {
  return __SEGGER_RTL_float32_sinh_inline(x);
}

/*********************************************************************
*
*       SEGGER_sinh()
*
*  Function description
*    Compute hyperbolic sine, double.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_sinh(double x) {
  return __SEGGER_RTL_float64_sinh_inline(x);
}

/*********************************************************************
*
*       SEGGER_coshf()
*
*  Function description
*    Compute hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_coshf(float x) {
  return __SEGGER_RTL_float32_cosh_inline(x);
}

/*********************************************************************
*
*       SEGGER_cosh()
*
*  Function description
*    Compute hyperbolic cosine, double.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_cosh(double x) {
  return __SEGGER_RTL_float64_cosh_inline(x);
}

/*********************************************************************
*
*       SEGGER_tanhf()
*
*  Function description
*    Compute hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_tanhf(float x) {
  return __SEGGER_RTL_float32_tanh_inline(x);
}

/*********************************************************************
*
*       SEGGER_tanh()
*
*  Function description
*    Compute hyperbolic tangent, double.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_tanh(double x) {
  return __SEGGER_RTL_float64_tanh_inline(x);
}

/*********************************************************************
*
*       SEGGER_asinhf()
*
*  Function description
*    Compute inverse hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sine of.
*
*  Additional information
*    Calculates the inverse hyperbolic sine of x. 

*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic sine of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_asinhf(float x) {
  return __SEGGER_RTL_float32_asinh_inline(x);
}

/*********************************************************************
*
*       SEGGER_asinh()
*
*  Function description
*    Compute inverse hyperbolic sine, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sine of.
*
*  Additional information
*    Calculates the inverse hyperbolic sine of x. 

*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic sine of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_asinh(double x) {
  return __SEGGER_RTL_float64_asinh_inline(x);
}

/*********************************************************************
*
*       SEGGER_acoshf()
*
*  Function description
*    Compute inverse hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Additional information
*    Calculates the non-negative inverse hyperbolic cosine of x. 

*  Return value
*    * If x < 1, return NaN.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic cosine of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_acoshf(float x) {
  return __SEGGER_RTL_float32_acosh_inline(x);
}

/*********************************************************************
*
*       SEGGER_acosh()
*
*  Function description
*    Compute inverse hyperbolic cosine, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Additional information
*    Calculates the non-negative inverse hyperbolic cosine of x. 

*  Return value
*    * If x < 1, return NaN.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic cosine of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_acosh(double x) {
  return __SEGGER_RTL_float64_acosh_inline(x);
}

/*********************************************************************
*
*       SEGGER_atanhf()
*
*  Function description
*    Compute inverse hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Additional information
*    Calculates the non-negative inverse hyperbolic tangent of x. 

*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * If x = +/-1, return +/-infinity.
*    * Else, return inverse hyperbolic tangent of x.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_atanhf(float x) {
  return __SEGGER_RTL_float32_atanh_inline(x);
}

/*********************************************************************
*
*       SEGGER_atanh()
*
*  Function description
*    Compute inverse hyperbolic tangent, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Additional information
*    Calculates the non-negative inverse hyperbolic tangent of x. 

*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * If x = +/-1, return +/-infinity.
*    * Else, return inverse hyperbolic tangent of x.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_atanh(double x) {
  return __SEGGER_RTL_float64_atanh_inline(x);
}

/*********************************************************************
*
*       SEGGER_nextafterf()
*
*  Function description
*    Next machine-floating value, float.
*
*  Parameters
*    x - Value to step from.
*    y - Director to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*/
float __SEGGER_RTL_PUBLIC_API SEGGER_nextafterf(float x, float y) {
  return __SEGGER_RTL_float32_nextafter_inline(x, y);
}

/*********************************************************************
*
*       SEGGER_nextafter()
*
*  Function description
*    Next machine-floating value, double.
*
*  Parameters
*    x - Value to step from.
*    y - Director to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*/
double __SEGGER_RTL_PUBLIC_API SEGGER_nextafter(double x, double y) {
  return __SEGGER_RTL_float64_nextafter_inline(x, y);
}

#endif

#if __SEGGER_RTL_INCLUDE_AEABI_API == 1

/*********************************************************************
*
*       Public code (Arm AEABI API functions)
*
**********************************************************************
*/

/*********************************************************************
*
*       __aeabi_fadd()
*
*  Function description
*    Add, float.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_fadd(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_BitcastToU32(
           __SEGGER_RTL_float32_add(
             __SEGGER_RTL_BitcastToF32(x),
             __SEGGER_RTL_BitcastToF32(y)));
}

/*********************************************************************
*
*       __aeabi_dadd()
*
*  Function description
*    Add, double.
*
*  Parameters
*    x - Augend.
*    y - Addend.
*
*  Return value
*    Sum.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_dadd(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_BitcastToU64(
           __SEGGER_RTL_float64_add(
             __SEGGER_RTL_BitcastToF64(x),
             __SEGGER_RTL_BitcastToF64(y)));
}

/*********************************************************************
*
*       __aeabi_fsub()
*
*  Function description
*    Subtract, float.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_fsub(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_BitcastToU32(
           __SEGGER_RTL_float32_sub(
             __SEGGER_RTL_BitcastToF32(x),
             __SEGGER_RTL_BitcastToF32(y)));
}

/*********************************************************************
*
*       __aeabi_dsub()
*
*  Function description
*    Subtract, double.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_dsub(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_BitcastToU64(
           __SEGGER_RTL_float64_sub(
             __SEGGER_RTL_BitcastToF64(x),
             __SEGGER_RTL_BitcastToF64(y)));
}

/*********************************************************************
*
*       __aeabi_frsub()
*
*  Function description
*    Reverse subtract, float.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __aeabi_frsub(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_BitcastToU32(
           __SEGGER_RTL_float32_sub(
             __SEGGER_RTL_BitcastToF32(y),
             __SEGGER_RTL_BitcastToF32(x)));
}

/*********************************************************************
*
*       __aeabi_drsub()
*
*  Function description
*    Reverse subtract, double.
*
*  Parameters
*    x - Minuend.
*    y - Subtrahend.
*
*  Return value
*    Difference.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_drsub(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_BitcastToU64(
           __SEGGER_RTL_float64_sub(
             __SEGGER_RTL_BitcastToF64(y),
             __SEGGER_RTL_BitcastToF64(x)));
}

/*********************************************************************
*
*       __aeabi_fmul()
*
*  Function description
*    Multiply, float.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_fmul(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_BitcastToU32(
           __SEGGER_RTL_float32_mul(
             __SEGGER_RTL_BitcastToF32(x),
             __SEGGER_RTL_BitcastToF32(y)));
}

/*********************************************************************
*
*       __aeabi_dmul()
*
*  Function description
*    Multiply, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*
*  Return value
*    Product.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_dmul(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_BitcastToU64(
           __SEGGER_RTL_float64_mul(
             __SEGGER_RTL_BitcastToF64(x),
             __SEGGER_RTL_BitcastToF64(y)));
}

/*********************************************************************
*
*       __aeabi_fdiv()
*
*  Function description
*    Divide, float.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_fdiv(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_BitcastToU32(
           __SEGGER_RTL_float32_div(
             __SEGGER_RTL_BitcastToF32(x),
             __SEGGER_RTL_BitcastToF32(y)));
}

/*********************************************************************
*
*       __aeabi_ddiv()
*
*  Function description
*    Divide, double.
*
*  Parameters
*    x - Dividend.
*    y - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_ddiv(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_BitcastToU64(
           __SEGGER_RTL_float64_div(
             __SEGGER_RTL_BitcastToF64(x),
             __SEGGER_RTL_BitcastToF64(y)));
}

/*********************************************************************
*
*       __aeabi_fcmpeq()
*
*  Function description
*    Equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not equal to y.
*    == 1 - x is equal to y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_fcmpeq(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_float32_eq(
           __SEGGER_RTL_BitcastToF32(x),
           __SEGGER_RTL_BitcastToF32(y));
}

/*********************************************************************
*
*       __aeabi_dcmpeq()
*
*  Function description
*    Equal, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not equal to y.
*    == 1 - x is equal to y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_dcmpeq(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_float64_eq(
           __SEGGER_RTL_BitcastToF64(x),
           __SEGGER_RTL_BitcastToF64(y));
}

/*********************************************************************
*
*       __aeabi_fcmplt()
*
*  Function description
*    Less than, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not less than y.
*    == 1 - x is less than y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_fcmplt(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_float32_lt(
           __SEGGER_RTL_BitcastToF32(x),
           __SEGGER_RTL_BitcastToF32(y));
}

/*********************************************************************
*
*       __aeabi_dcmplt()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not less than y.
*    == 1 - x is less than y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_dcmplt(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_float64_lt(
           __SEGGER_RTL_BitcastToF64(x),
           __SEGGER_RTL_BitcastToF64(y));
}

/*********************************************************************
*
*       __aeabi_fcmple()
*
*  Function description
*    Less than or equal, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not less than or equal to y.
*    == 1 - x is less than or equal to y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_fcmple(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_float32_le(
           __SEGGER_RTL_BitcastToF32(x),
           __SEGGER_RTL_BitcastToF32(y));
}

/*********************************************************************
*
*       __aeabi_dcmple()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not less than or equal to y.
*    == 1 - x is less than or equal to y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_dcmple(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_float64_le(
           __SEGGER_RTL_BitcastToF64(x),
           __SEGGER_RTL_BitcastToF64(y));
}

/*********************************************************************
*
*       __aeabi_fcmpgt()
*
*  Function description
*    Less than, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not greater than y.
*    == 1 - x is greater than y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_fcmpgt(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_float32_gt(
           __SEGGER_RTL_BitcastToF32(x),
           __SEGGER_RTL_BitcastToF32(y));
}

/*********************************************************************
*
*       __aeabi_dcmpgt()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not greater than y.
*    == 1 - x is greater than y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_dcmpgt(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_float64_gt(
           __SEGGER_RTL_BitcastToF64(x),
           __SEGGER_RTL_BitcastToF64(y));
}

/*********************************************************************
*
*       __aeabi_fcmpge()
*
*  Function description
*    Less than, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not greater than or equal to y.
*    == 1 - x is greater than or equal to y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_fcmpge(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_float32_ge(
           __SEGGER_RTL_BitcastToF32(x),
           __SEGGER_RTL_BitcastToF32(y));
}

/*********************************************************************
*
*       __aeabi_dcmpge()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    == 0 - x is not greater than or equal to y.
*    == 1 - x is greater than or equal to y.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_dcmpge(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_float64_ge(
           __SEGGER_RTL_BitcastToF64(x),
           __SEGGER_RTL_BitcastToF64(y));
}

/*********************************************************************
*
*       __aeabi_fcmpun()
*
*  Function description
*    Unordered, float.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    One if x or y are NaN; else zero.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_fcmpun(__SEGGER_RTL_U32 x, __SEGGER_RTL_U32 y) {
  return __SEGGER_RTL_float32_isnan_soft(x) || __SEGGER_RTL_float32_isnan_soft(y);
}

/*********************************************************************
*
*       __aeabi_dcmpun()
*
*  Function description
*    Unordered, double.
*
*  Parameters
*    x - Left-hand operand.
*    y - Right-hand operand.
*
*  Return value
*    One if x or y are NaN; else zero.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_dcmpun(__SEGGER_RTL_U64 x, __SEGGER_RTL_U64 y) {
  return __SEGGER_RTL_float64_isnan_soft(x) || __SEGGER_RTL_float64_isnan_soft(y);
}

/*********************************************************************
*
*       __aeabi_f2iz()
*
*  Function description
*    Convert float to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I32 __SEGGER_RTL_PUBLIC_API __aeabi_f2iz(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_float_to_int32(__SEGGER_RTL_BitcastToF32(x));
}

/*********************************************************************
*
*       __aeabi_d2iz()
*
*  Function description
*    Convert double to int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I32 __SEGGER_RTL_PUBLIC_API __aeabi_d2iz(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_double_to_int32(__SEGGER_RTL_BitcastToF64(x));
}

/*********************************************************************
*
*       __aeabi_f2uiz()
*
*  Function description
*    Convert float to unsigned int.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_f2uiz(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_float_to_uint32(__SEGGER_RTL_BitcastToF32(x));
}

/*********************************************************************
*
*       __aeabi_d2uiz()
*
*  Function description
*    Convert double to unsigned.
*
*  Parameters
*    x - Double value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_d2uiz(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_double_to_uint32(__SEGGER_RTL_BitcastToF64(x));
}

/*********************************************************************
*
*       __aeabi_f2lz()
*
*  Function description
*    Convert float to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    The RV32 compiler converts a __SEGGER_RTL_U32 to a 64-bit integer
*    by calling runtime support to handle it.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API __aeabi_f2lz(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_float_to_int64(__SEGGER_RTL_BitcastToF32(x));
}

/*********************************************************************
*
*       __aeabi_d2lz()
*
*  Function description
*    Convert double to long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Notes
*    RV32 always calls runtime for __SEGGER_RTL_U64 to int64 conversion.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_I64 __SEGGER_RTL_PUBLIC_API __aeabi_d2lz(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_double_to_int64(__SEGGER_RTL_BitcastToF64(x));
}

/*********************************************************************
*
*       __aeabi_f2ulz()
*
*  Function description
*    Convert float to unsigned long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_f2ulz(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_float_to_uint64(__SEGGER_RTL_BitcastToF32(x));
}

/*********************************************************************
*
*       __aeabi_d2ulz()
*
*  Function description
*    Convert double to unsigned long long.
*
*  Parameters
*    x - Floating value to convert.
*
*  Return value
*    Integerized value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_d2ulz(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_double_to_uint64(__SEGGER_RTL_BitcastToF64(x));
}

/*********************************************************************
*
*       __aeabi_i2f()
*
*  Function description
*    Convert int to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_i2f(__SEGGER_RTL_I32 x) {
  return __SEGGER_RTL_BitcastToU32(__SEGGER_RTL_int32_to_float(x));
}

/*********************************************************************
*
*       __aeabi_i2d()
*
*  Function description
*    Convert int to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_i2d(__SEGGER_RTL_I32 x) {
  return __SEGGER_RTL_BitcastToU64(__SEGGER_RTL_int32_to_double(x));
}

/*********************************************************************
*
*       __aeabi_ui2f()
*
*  Function description
*    Convert unsigned to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_ui2f(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_BitcastToU32(__SEGGER_RTL_uint32_to_float(x));
}

/*********************************************************************
*
*       __aeabi_ui2d()
*
*  Function description
*    Convert unsigned to double.
*
*  Parameters
*    x - Unsigned value to convert.
*
*  Return value
*    __SEGGER_RTL_U64 value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_ui2d(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_BitcastToU64(__SEGGER_RTL_uint32_to_double(x));
}

/*********************************************************************
*
*       __aeabi_l2f()
*
*  Function description
*    Convert long long to float.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_l2f(__SEGGER_RTL_I64 x) {
  return __SEGGER_RTL_BitcastToU32(__SEGGER_RTL_int64_to_float(x));
}

/*********************************************************************
*
*       __aeabi_l2d()
*
*  Function description
*    Convert long long to double.
*
*  Parameters
*    x - Integer value to convert.
*
*  Return value
*    Floating value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_l2d(__SEGGER_RTL_I64 x) {
  return __SEGGER_RTL_BitcastToU64(__SEGGER_RTL_int64_to_double(x));
}

/*********************************************************************
*
*       __aeabi_ul2f()
*
*  Function description
*    Convert unsigned long long to float.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    __SEGGER_RTL_U32 value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_ul2f(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_BitcastToU32(__SEGGER_RTL_uint64_to_float(x));
}

/*********************************************************************
*
*       __aeabi_ul2d()
*
*  Function description
*    Convert unsigned long long to double.
*
*  Parameters
*    x - Unsigned long long value to convert.
*
*  Return value
*    __SEGGER_RTL_U64 value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_ul2d(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_BitcastToU64(__SEGGER_RTL_uint64_to_double(x));
}

/*********************************************************************
*
*       __aeabi_f2d()
*
*  Function description
*    Extend float to double.
*
*  Parameters
*    x - Floating value to extend.
*
*  Return value
*    __SEGGER_RTL_U64  value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_f2d(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_BitcastToU64(
           __SEGGER_RTL_float_to_double(
             __SEGGER_RTL_BitcastToF32(x)));
}

/*********************************************************************
*
*       __aeabi_d2f()
*
*  Function description
*    Truncate double to float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Float value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_d2f(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_BitcastToU32(
           __SEGGER_RTL_double_to_float(
             __SEGGER_RTL_BitcastToF64(x)));
}

#endif

#if __SEGGER_RTL_INCLUDE_AEABI_API > 0

/*********************************************************************
*
*       __aeabi_f2h()
*
*  Function description
*    Truncate float to IEEE half-precision float.
*
*  Parameters
*    x - Float value to truncate.
*
*  Return value
*    Float value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __aeabi_f2h(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_float_to_half_ieee_soft(x);
}

/*********************************************************************
*
*       __aeabi_d2h()
*
*  Function description
*    Truncate double to IEEE half-precision float.
*
*  Parameters
*    x - Double value to truncate.
*
*  Return value
*    Half-precision value.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __aeabi_d2h(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_double_to_half_ieee_soft(x);
}

/*********************************************************************
*
*       __aeabi_h2f()
*
*  Function description
*    Convert IEEE half-precision float to float.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Single-precision float.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __aeabi_h2f(__SEGGER_RTL_U16 x) {
  return __SEGGER_RTL_BitcastToU32(__SEGGER_RTL_half_to_float_ieee_soft(x));
}

/*********************************************************************
*
*       __aeabi_h2d()
*
*  Function description
*    Convert IEEE half-precision float to double.
*
*  Parameters
*    x - Half-precision float.
*
*  Return value
*    Double-precision float.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __aeabi_h2d(__SEGGER_RTL_U16 x) {
  return __SEGGER_RTL_BitcastToU64(__SEGGER_RTL_half_to_double_ieee_soft(x));
}

#endif

/*********************************************************************
*
*       Public code (C API functions)
*
**********************************************************************
*/

#if __SEGGER_RTL_INCLUDE_C_API

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_float32_isnan()
*
*  Function description
*    Not-a-number query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Is not a NaN.
*    != 0 - Is a NaN.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float32_isnan(float x) {
  return __SEGGER_RTL_float32_isnan_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isinf()
*
*  Function description
*    Infinity query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not infinite.
*    != 0 - Is infinite.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float32_isinf(float x) {
  return __SEGGER_RTL_float32_isinf_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isfinite()
*
*  Function description
*    Finite query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not finite.
*    != 0 - Is finite.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float32_isfinite(float x) {
  return __SEGGER_RTL_float32_isfinite_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_isnormal()
*
*  Function description
*    Normal query, float.
*
*  Parameters
*    x - Value to test as float.
*
*  Return value
*    == 0 - Not normal.
*    != 0 - Is normal.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float32_isnormal(float x) {
  return __SEGGER_RTL_float32_isnormal_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_signbit()
*
*  Function description
*    Extract sign bit, float.
*
*  Parameters
*    x - Value to extract from as float.
*
*  Return value
*    == 0 - Sign bit is zero (positive).
*    == 1 - Sign bit is one (negative).
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float32_signbit(float x) {
  return __SEGGER_RTL_float32_signbit_soft(__SEGGER_RTL_BitcastToU32(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_classify()
*
*  Function description
*    Classify, float.
*
*  Parameters
*    x - Value to classify.
*
*  Return value
*    __SEGGER_RTL_FP_ZERO      -- Value is +0 or -0.
*    __SEGGER_RTL_FP_NAN       -- Value is not-a-number.
*    __SEGGER_RTL_FP_INFINITE  -- Value is +Inf or -Inf.
*    __SEGGER_RTL_FP_NORMAL    -- Value is a normal and finite.
*    __SEGGER_RTL_FP_SUBNORMAL -- Value is a subnormal and finite.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float32_classify(float x) {
  if (__SEGGER_RTL_float32_iszero_inline(x)) {
    return __SEGGER_RTL_FP_ZERO;
  } else if (__SEGGER_RTL_float32_isnan_inline(x)) {
    return __SEGGER_RTL_FP_NAN;
  } else if (__SEGGER_RTL_float32_isinf_inline(x)) {
    return __SEGGER_RTL_FP_INFINITE;
  } else if (__SEGGER_RTL_float32_isnormal_inline(x)) {
    return __SEGGER_RTL_FP_NORMAL;
  } else {
    return __SEGGER_RTL_FP_SUBNORMAL;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isnan()
*
*  Function description
*    Not-a-number query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Is not a NaN.
*    != 0 - Is a NaN.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float64_isnan(double x) {
  return __SEGGER_RTL_float64_isnan_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isinf()
*
*  Function description
*    Infinity query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Not infinite.
*    != 0 - Is infinite.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float64_isinf(double x) {
  return __SEGGER_RTL_float64_isinf_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isfinite()
*
*  Function description
*    Finite query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Not finite.
*    != 0 - Is finite.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float64_isfinite(double x) {
  return __SEGGER_RTL_float64_isfinite_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_isnormal()
*
*  Function description
*    Normal query, double.
*
*  Parameters
*    x - Value to test as double.
*
*  Return value
*    == 0 - Not normal.
*    != 0 - Is normal.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float64_isnormal(double x) {
  return __SEGGER_RTL_float64_isnormal_inline(x);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_signbit()
*
*  Function description
*    Extract sign bit, double.
*
*  Parameters
*    x - Value to extract from as double.
*
*  Return value
*    == 0 - Sign bit is zero (positive).
*    == 1 - Sign bit is one (negative).
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float64_signbit(double x) {
  return __SEGGER_RTL_float64_signbit_soft(__SEGGER_RTL_BitcastToU64(x));
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_classify()
*
*  Function description
*    Classify, double.
*
*  Parameters
*    x - Value to classify.
*
*  Return value
*    __SEGGER_RTL_FP_ZERO      -- Value is +0 or -0.
*    __SEGGER_RTL_FP_NAN       -- Value is not-a-number.
*    __SEGGER_RTL_FP_INFINITE  -- Value is +Inf or -Inf.
*    __SEGGER_RTL_FP_NORMAL    -- Value is a normal and finite.
*    __SEGGER_RTL_FP_SUBNORMAL -- Value is a subnormal and finite.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float64_classify(double x) {
  if (__SEGGER_RTL_float64_iszero_inline(x)) {
    return __SEGGER_RTL_FP_ZERO;
  } else if (__SEGGER_RTL_float64_isnan_inline(x)) {
    return __SEGGER_RTL_FP_NAN;
  } else if (__SEGGER_RTL_float64_isinf_inline(x)) {
    return __SEGGER_RTL_FP_INFINITE;
  } else if (__SEGGER_RTL_float64_isnormal_inline(x)) {
    return __SEGGER_RTL_FP_NORMAL;
  } else {
    return __SEGGER_RTL_FP_SUBNORMAL;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_float32_cmplx()
*
*  Function description
*    Construct, float complex.
*
*  Parameters
*    re - Real part of complex.
*    im - Imaginary part of complex.
*
*  Return value
*    The complex value re + im.I.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_float32_cmplx(float re, float im) {
  return __SEGGER_RTL_float32_cmplx_inline(re, im);
}

/*********************************************************************
*
*       __SEGGER_RTL_float64_cmplx()
*
*  Function description
*    Construct, double complex.
*
*  Parameters
*    re - Real part of complex.
*    im - Imaginary part of complex.
*
*  Return value
*    The complex value re + im.I.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_float64_cmplx(double re, double im) {
  return __SEGGER_RTL_float64_cmplx_inline(re, im);
}

/*********************************************************************
*
*       __SEGGER_RTL_ldouble_cmplx()
*
*  Function description
*    Construct, double complex.
*
*  Parameters
*    re - Real part of complex.
*    im - Imaginary part of complex.
*
*  Return value
*    The complex value re + im.I.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_ldouble_cmplx(long double re, long double im) {
  return __SEGGER_RTL_ldouble_cmplx_inline(re, im);
}

/*********************************************************************
*
*       copysignf()
*
*  Function description
*    Copy sign, float.
*
*  Parameters
*    x - Floating value to inject sign into.
*    y - Floating value carrying the sign to inject.
*
*  Return value
*    x with the sign of y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API copysignf(float x, float y) {
  return __SEGGER_RTL_float32_signbit_copy(x, y);
}

/*********************************************************************
*
*       copysign()
*
*  Function description
*    Copy sign, double.
*
*  Parameters
*    x - Floating value to inject sign into.
*    y - Floating value carrying the sign to inject.
*
*  Return value
*    x with the sign of y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API copysign(double x, double y) {
  return __SEGGER_RTL_float64_signbit_copy(x, y);
}

/*********************************************************************
*
*       copysignl()
*
*  Function description
*    Copy sign, long double.
*
*  Parameters
*    x - Floating value to inject sign into.
*    y - Floating value carrying the sign to inject.
*
*  Return value
*    x with the sign of y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API copysignl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(copysign(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       ldexpf()
*
*  Function description
*    Scale by power of two, float.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is zero, return x;
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2^n.
*
*  See also
*    scalbnf()
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API ldexpf(float x, int n) {
  return __SEGGER_RTL_float32_ldexp_inline(x, n);
}

/*********************************************************************
*
*       ldexp()
*
*  Function description
*    Scale by power of two, double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is +/-0, return x;
*    * If x is +/-Inf, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2 ^ n.
*
*  See also
*    scalbn()
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API ldexp(double x, int n) {
  return __SEGGER_RTL_float64_ldexp_inline(x, n);
}

/*********************************************************************
*
*       ldexpl()
*
*  Function description
*    Scale by power of two, long double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of two to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of two.
*
*  Return value
*    * If x is +/-0, return x;
*    * If x is +/-Inf, return x.
*    * If x is NaN, return x.
*    * Else, return x * 2 ^ n.
*
*  See also
*    scalbnl()
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API ldexpl(long double x, int n) {
  return __SEGGER_RTL_double_to_ldouble(ldexp(__SEGGER_RTL_ldouble_to_double(x), n));
}

/*********************************************************************
*
*       logbf()
*
*  Function description
*    Radix-indpendent exponent, float.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return -Inf.
*    * If x is infinite, return +Inf.
*    * If x is NaN, return NaN.
*    * Else, return integer part of log[FLTRADIX](x).
*
*  Additional information
*    Calculates the exponent of x, which is the integral part of
*    the FLTRADIX-logarithm of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API logbf(float x) {
  return __SEGGER_RTL_float32_logb_inline(x);
}

/*********************************************************************
*
*       logb()
*
*  Function description
*    Radix-indpendent exponent, double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return -Inf.
*    * If x is infinite, return +Inf.
*    * If x is NaN, return NaN.
*    * Else, return integer part of log[FLTRADIX](x).
*
*  Additional information
*    Calculates the exponent of x, which is the integral part of
*    the FLTRADIX-logarithm of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API logb(double x) {
  return __SEGGER_RTL_float64_logb_inline(x);
}

/*********************************************************************
*
*       logbl()
*
*  Function description
*    Radix-indpendent exponent, long double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return -Inf.
*    * If x is infinite, return +Inf.
*    * If x is NaN, return NaN.
*    * Else, return integer part of log[FLTRADIX](x).
*
*  Additional information
*    Calculates the exponent of x, which is the integral part of
*    the FLTRADIX-logarithm of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API logbl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(logb(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       ilogbf()
*
*  Function description
*    Radix-independent exponent, float.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API ilogbf(float x) {
  return __SEGGER_RTL_float32_ilogb_inline(x);
}

/*********************************************************************
*
*       ilogb()
*
*  Function description
*    Radix-independent exponent, double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API ilogb(double x) {
  return __SEGGER_RTL_float64_ilogb_inline(x);
}

/*********************************************************************
*
*       ilogbl()
*
*  Function description
*    Radix-independent exponent, long double.
*
*  Parameters
*    x - Floating value to operate on.
*
*  Return value
*    * If x is zero, return FP_ILOGB0.
*    * If x is NaN, return FP_ILOGBNAN.
*    * If x is infinite, return MAX_INT.
*    * Else, return integer part of log[FLTRADIX](x).
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API ilogbl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(logb(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       scalbnf()
*
*  Function description
*    Scale, float.
*
*  Parameters
*    x - Value to scale.
*    n - Power of FLT_RADIX to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of FLT_RADIX.
*
*    As floating-point arithmetic conforms to IEC 60559, FLT_RADIX 
*    is 2 and scalbnf() is (in this implementation) identical to ldexpf().
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * FLT_RADIX ^ n.
*
*  See also
*    ldexpf()
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API scalbnf(float x, int n) {
  return __SEGGER_RTL_float32_ldexp_inline(x, n);
}

/*********************************************************************
*
*       scalbn()
*
*  Function description
*    Scale, double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of DBL_RADIX to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of DBL_RADIX.
*
*    As floating-point arithmetic conforms to IEC 60559, DBL_RADIX 
*    is 2 and scalbn() is (in this implementation) identical to ldexp().
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * DBL_RADIX ^ n.
*
*  See also
*    ldexp()
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API scalbn(double x, int n) {
  return __SEGGER_RTL_float64_ldexp_inline(x, n);
}

/*********************************************************************
*
*       scalbnl()
*
*  Function description
*    Scale, long double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of LDBL_RADIX to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of LDBL_RADIX.
*
*    As floating-point arithmetic conforms to IEC 60559, LDBL_RADIX 
*    is 2 and scalbnl() is (in this implementation) identical to ldexpl().
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * LDBL_RADIX ^ n.
*
*  See also
*    ldexpl()
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API scalbnl(long double x, int n) {
  return __SEGGER_RTL_double_to_ldouble(scalbn(__SEGGER_RTL_ldouble_to_double(x), n));
}

/*********************************************************************
*
*       scalblnf()
*
*  Function description
*    Scale, float.
*
*  Parameters
*    x - Value to scale.
*    n - Power of FLT_RADIX to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of FLT_RADIX.
*
*    As floating-point arithmetic conforms to IEC 60559, FLT_RADIX 
*    is 2 and scalbnf() is (in this implementation) identical to ldexpf().
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * FLT_RADIX ^ n.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API scalblnf(float x, long n) {
  return __SEGGER_RTL_float32_ldexp_inline(x, (int)n);
}

/*********************************************************************
*
*       scalbln()
*
*  Function description
*    Scale, double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of DBL_RADIX to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of DBL_RADIX.
*
*    As floating-point arithmetic conforms to IEC 60559, DBL_RADIX 
*    is 2 and scalbln() is (in this implementation) identical to ldexp().
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * DBL_RADIX ^ n.
*
*  See also
*    ldexp()
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API scalbln(double x, long n) {
  return __SEGGER_RTL_float64_ldexp_inline(x, (int)n);
}

/*********************************************************************
*
*       scalblnl()
*
*  Function description
*    Scale, long double.
*
*  Parameters
*    x - Value to scale.
*    n - Power of LDBL_RADIX to scale by.
*
*  Additional information
*    Multiplies a floating-point number by an integral power
*    of LDBL_RADIX.
*
*    As floating-point arithmetic conforms to IEC 60559, LDBL_RADIX 
*    is 2 and scalblnl() is (in this implementation) identical to ldexpl().
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x * LDBL_RADIX ^ n.
*
*  See also
*    ldexpl()
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API scalblnl(long double x, long n) {
  return __SEGGER_RTL_double_to_ldouble(scalbln(__SEGGER_RTL_ldouble_to_double(x), n));
}

/*********************************************************************
*
*       frexpf()
*
*  Function description
*    Split to significand and exponent, float.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API frexpf(float x, int *exp) {
  return __SEGGER_RTL_float32_frexp_inline(x, exp);
}

/*********************************************************************
*
*       frexp()
*
*  Function description
*    Split to significand and exponent, double.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API frexp(double x, int *exp) {
  return __SEGGER_RTL_float64_frexp_inline(x, exp);
}

/*********************************************************************
*
*       frexpl()
*
*  Function description
*    Split to significand and exponent, long double.
*
*  Parameters
*    x   - Floating value to operate on.
*    exp - Pointer to integer receiving the power-of-two exponent of x.
*
*  Return value
*    * If x is zero, infinite or NaN, return x and store zero into
*      the integer pointed to by exp.
*    * Else, return the value f, such that f has a magnitude in the
*      interval [0.5, 1) and x equals f * pow(2, *exp)
*
*  Additional information
*    Breaks a floating-point number into a normalized fraction
*    and an integral power of two.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API frexpl(long double x, int *exp) {
  return __SEGGER_RTL_double_to_ldouble(frexp(__SEGGER_RTL_ldouble_to_double(x), exp));
}

/*********************************************************************
*
*       modff()
*
*  Function description
*    Separate integer and fractional parts, float.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modff() returns the signed
*    fractional part of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API modff(float x, float *iptr) {
  return __SEGGER_RTL_float32_modf_inline(x, iptr);
}

/*********************************************************************
*
*       modf()
*
*  Function description
*    Separate integer and fractional parts, double.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modf() returns the signed
*    fractional part of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API modf(double x, double *iptr) {
  return __SEGGER_RTL_float64_modf_inline(x, iptr);
}

/*********************************************************************
*
*       modfl()
*
*  Function description
*    Separate integer and fractional parts, long double.
*
*  Parameters
*    x    - Value to separate.
*    iptr - Pointer to object that receives the integral part of x.
*
*  Return value
*    The signed fractional part of x.
*
*  Additional information
*    Breaks x into integral and fractional parts, each of which has
*    the same type and sign as x.
*
*    The integral part (in floating-point format) is stored in the
*    object pointed to by iptr and modf() returns the signed
*    fractional part of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API modfl(long double x, long double *iptr) {
  double i;
  double r;
  //
  r = modf(__SEGGER_RTL_ldouble_to_double(x), &i);
  *iptr = __SEGGER_RTL_double_to_ldouble(i);
  //
  return __SEGGER_RTL_double_to_ldouble(r);
}

/*********************************************************************
*
*       fabsf()
*
*  Function description
*    Compute absolute value, float.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API fabsf(float x) {
  return __SEGGER_RTL_float32_abs_inline(x);
}

/*********************************************************************
*
*       fabs()
*
*  Function description
*    Compute absolute value, double.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API fabs(double x) {
  return __SEGGER_RTL_float64_abs_inline(x);
}

/*********************************************************************
*
*       fabsl()
*
*  Function description
*    Compute absolute value, long double.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, absolute value of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API fabsl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(fabs(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       fmaf()
*
*  Function description
*    Compute fused multiply-add, float.
*
*  Parameters
*    x - Multiplier.
*    y - Multiplicand.
*    z - Summand.
*
*  Return value
*    Return (x * y) + z.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API fmaf(float x, float y, float z) {
  return __SEGGER_RTL_float32_fma_inline(x, y, z);
}

/*********************************************************************
*
*       fma()
*
*  Function description
*    Compute fused multiply-add, double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*    z - Summand.
*
*  Return value
*    Return (x * y) + z.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API fma(double x, double y, double z) {
  return __SEGGER_RTL_float64_fma_inline(x, y, z);
}

/*********************************************************************
*
*       fmal()
*
*  Function description
*    Compute fused multiply-add, long double.
*
*  Parameters
*    x - Multiplicand.
*    y - Multiplier.
*    z - Summand.
*
*  Return value
*    Return (x * y) + z.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API fmal(long double x, long double y, long double z) {
  return __SEGGER_RTL_double_to_ldouble(fma(__SEGGER_RTL_ldouble_to_double(x),
                                            __SEGGER_RTL_ldouble_to_double(y),
                                            __SEGGER_RTL_ldouble_to_double(z)));
}

/*********************************************************************
*
*       fmodf()
*
*  Function description
*    Compute remainder after division, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API fmodf(float x, float y) {
  return __SEGGER_RTL_float32_fmod_inline(x, y);
}

/*********************************************************************
*
*       fmod()
*
*  Function description
*    Compute remainder after division, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API fmod(double x, double y) {
  return __SEGGER_RTL_float64_fmod_inline(x, y);
}

/*********************************************************************
*
*       fmodl()
*
*  Function description
*    Compute remainder after division, long double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API fmodl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(fmod(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       remainderf()
*
*  Function description
*    Compute remainder after division, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API remainderf(float x, float y) {
  int q;
  //
  return __SEGGER_RTL_float32_remainder_inline(x, y, &q);
}

/*********************************************************************
*
*       remainder()
*
*  Function description
*    Compute remainder after division, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API remainder(double x, double y) {
  int q;
  //
  return __SEGGER_RTL_float64_remainder_inline(x, y, &q);
}

/*********************************************************************
*
*       remainderl()
*
*  Function description
*    Compute remainder after division, long double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API remainderl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(remainder(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       remquof()
*
*  Function description
*    Compute remainder after division, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*    quo - Pointer to object that receives the integer part of x
*          divided by y.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API remquof(float x, float y, int *quo) {
  return __SEGGER_RTL_float32_remainder_inline(x, y, quo);
}

/*********************************************************************
*
*       remquo()
*
*  Function description
*    Compute remainder after division, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*    quo - Pointer to object that receives the integer part of x
*          divided by y.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API remquo(double x, double y, int *quo) {
  return __SEGGER_RTL_float64_remainder_inline(x, y, quo);
}

/*********************************************************************
*
*       remquol()
*
*  Function description
*    Compute remainder after division, long double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*    quo - Pointer to object that receives the integer part of x
*          divided by y.
*
*  Return value
*    * If x is NaN, return NaN.
*    * If x is zero and y is nonzero, return x.
*    * If x is infinite, return NaN.
*    * If x is finite and y is infinite, return x.
*    * If y is NaN, return NaN.
*    * If y is zero, return NaN.
*    * Else, return remainder of x divided by y.
*
*  Additional information
*    Computes the floating-point remainder of x divided by y, i.e.
*    the value x - i*y for some integer i such that, if y is nonzero,
*    the result has the same sign as x and magnitude less than the
*    magnitude of y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API remquol(long double x, long double y, int *quo) {
  return __SEGGER_RTL_double_to_ldouble(remquo(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y), quo));
}

/*********************************************************************
*
*       fmaxf()
*
*  Function description
*    Compute maximum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API fmaxf(float x, float y) {
  return __SEGGER_RTL_float32_fmax_inline(x, y);
}

/*********************************************************************
*
*       fmax()
*
*  Function description
*    Compute maximum, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API fmax(double x, double y) {
  return __SEGGER_RTL_float64_fmax_inline(x, y);
}

/*********************************************************************
*
*       fmaxl()
*
*  Function description
*    Compute maximum, long double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return maximum of x and y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API fmaxl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(fmax(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       fminf()
*
*  Function description
*    Compute minimum, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API fminf(float x, float y) {
  return __SEGGER_RTL_float32_fmin_inline(x, y);
}

/*********************************************************************
*
*       fmin()
*
*  Function description
*    Compute minimum, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API fmin(double x, double y) {
  return __SEGGER_RTL_float64_fmin_inline(x, y);
}

/*********************************************************************
*
*       fminl()
*
*  Function description
*    Compute minimum, long double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x is NaN, return y.
*    * If y is NaN, return x.
*    * Else, return minimum of x and y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API fminl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(fmin(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       fdimf()
*
*  Function description
*    Positive difference, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x > y, x-y.
*    * Else, +0.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API fdimf(float x, float y) {
  return __SEGGER_RTL_float32_fdim_inline(x, y);
}

/*********************************************************************
*
*       fdim()
*
*  Function description
*    Positive difference, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x > y, x-y.
*    * Else, +0.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API fdim(double x, double y) {
  return __SEGGER_RTL_float64_fdim_inline(x, y);
}

/*********************************************************************
*
*       fdiml()
*
*  Function description
*    Positive difference, long double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x > y, x-y.
*    * Else, +0.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API fdiml(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(fdim(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       hypotf()
*
*  Function description
*    Compute magnitude of complex, float.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrt(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API hypotf(float x, float y) {
  return __SEGGER_RTL_float32_hypot_inline(x, y);
}

/*********************************************************************
*
*       hypot()
*
*  Function description
*    Compute magnitude of complex, double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrt(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API hypot(double x, double y) {
  return __SEGGER_RTL_float64_hypot_inline(x, y);
}

/*********************************************************************
*
*       hypotl()
*
*  Function description
*    Compute magnitude of complex, long double.
*
*  Parameters
*    x - Value #1.
*    y - Value #2.
*
*  Return value
*    * If x or y are infinite, return infinity.
*    * If x or y is NaN, return NaN. 
*    * Else, return sqrtl(x*x + y*y).
*
*  Additional information
*    Computes the square root of the sum of the squares of x and y
*    without undue overflow or underflow. If x and y are the lengths
*    of the sides of a right-angled triangle, then this computes the
*    length of the hypotenuse.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API hypotl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(hypot(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       ceilf()
*
*  Function description
*    Compute smallest integer not less than, float.
*
*  Parameters
*    x - Value to compute ceiling of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the smallest integer value not greater than x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API ceilf(float x) {
  return __SEGGER_RTL_float32_ceil_inline(x);
}

/*********************************************************************
*
*       ceil()
*
*  Function description
*    Compute smallest integer not less than, double.
*
*  Parameters
*    x - Value to compute ceiling of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the smallest integer value not greater than x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API ceil(double x) {
  return __SEGGER_RTL_float64_ceil_inline(x);
}

/*********************************************************************
*
*       ceill()
*
*  Function description
*    Compute smallest integer not less than, long double.
*
*  Parameters
*    x - Value to compute ceiling of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the smallest integer value not greater than x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API ceill(long double x) {
  return __SEGGER_RTL_double_to_ldouble(ceil(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       floorf()
*
*  Function description
*    Compute largest integer not greater than, float.
*
*  Parameters
*    x - Value to floor.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the largest integer value not greater than x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API floorf(float x) {
  return __SEGGER_RTL_float32_floor_inline(x);
}

/*********************************************************************
*
*       floor()
*
*  Function description
*    Compute largest integer not greater than, double.
*
*  Parameters
*    x - Value to floor.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the largest integer value not greater than x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API floor(double x) {
  return __SEGGER_RTL_float64_floor_inline(x);
}

/*********************************************************************
*
*       floorl()
*
*  Function description
*    Compute largest integer not greater than, long double.
*
*  Parameters
*    x - Value to floor.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the largest integer value not greater than x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API floorl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(floor(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       nearbyintf()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API nearbyintf(float x) {
  return __SEGGER_RTL_float32_rint_inline(x);
}

/*********************************************************************
*
*       nearbyint()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API nearbyint(double x) {
  return __SEGGER_RTL_float64_rint_inline(x);
}

/*********************************************************************
*
*       nearbyintl()
*
*  Function description
*    Round to nearest integer, long double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API nearbyintl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(nearbyint(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       truncf()
*
*  Function description
*    Truncate to integer, float.
*
*  Parameters
*    x - Value to truncate.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x with fractional part removed.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API truncf(float x) {
  return __SEGGER_RTL_float32_trunc_inline(x);
}

/*********************************************************************
*
*       trunc()
*
*  Function description
*    Truncate to integer, double.
*
*  Parameters
*    x - Value to truncate.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x with fractional part removed.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API trunc(double x) {
  return __SEGGER_RTL_float64_trunc_inline(x);
}

/*********************************************************************
*
*       truncl()
*
*  Function description
*    Truncate to integer, long double.
*
*  Parameters
*    x - Value to truncate.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return x with fractional part removed.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API truncl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(trunc(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       rintf()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API rintf(float x) {
  return __SEGGER_RTL_float32_rint_inline(x);
}

/*********************************************************************
*
*       rint()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API rint(double x) {
  return __SEGGER_RTL_float64_rint_inline(x);
}

/*********************************************************************
*
*       rintl()
*
*  Function description
*    Round to nearest integer, long double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API rintl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(rint(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       lrintf()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long __SEGGER_RTL_PUBLIC_API lrintf(float x) {
  return __SEGGER_RTL_float32_lrint_inline(x);
}

/*********************************************************************
*
*       lrint()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long __SEGGER_RTL_PUBLIC_API lrint(double x) {
  return __SEGGER_RTL_float64_lrint_inline(x);
}

/*********************************************************************
*
*       lrintl()
*
*  Function description
*    Round to nearest integer, long double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long __SEGGER_RTL_PUBLIC_API lrintl(long double x) {
  return lrint(__SEGGER_RTL_ldouble_to_double(x));
}

/*********************************************************************
*
*       llrintf()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long long __SEGGER_RTL_PUBLIC_API llrintf(float x) {
  return __SEGGER_RTL_float32_llrint_inline(x);
}

/*********************************************************************
*
*       llrint()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long long __SEGGER_RTL_PUBLIC_API llrint(double x) {
  return __SEGGER_RTL_float64_llrint_inline(x);
}

/*********************************************************************
*
*       llrintl()
*
*  Function description
*    Round to nearest integer, long double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long long __SEGGER_RTL_PUBLIC_API llrintl(long double x) {
  return llrint(__SEGGER_RTL_ldouble_to_double(x));
}

/*********************************************************************
*
*       roundf()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x, ties away from zero.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API roundf(float x) {
  return __SEGGER_RTL_float32_round_inline(x);
}

/*********************************************************************
*
*       round()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x, ties away from zero.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API round(double x) {
  return __SEGGER_RTL_float64_round_inline(x);
}

/*********************************************************************
*
*       roundl()
*
*  Function description
*    Round to nearest integer, long double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x, ties away from zero.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API roundl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(round(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       lroundf()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long __SEGGER_RTL_PUBLIC_API lroundf(float x) {
  return __SEGGER_RTL_float32_lround_inline(x);
}

/*********************************************************************
*
*       lround()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long __SEGGER_RTL_PUBLIC_API lround(double x) {
  return __SEGGER_RTL_float64_lround_inline(x);
}

/*********************************************************************
*
*       lroundl()
*
*  Function description
*    Round to nearest integer, long double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long __SEGGER_RTL_PUBLIC_API lroundl(long double x) {
  return lround(__SEGGER_RTL_ldouble_to_double(x));
}

/*********************************************************************
*
*       llroundf()
*
*  Function description
*    Round to nearest integer, float.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long long __SEGGER_RTL_PUBLIC_API llroundf(float x) {
  return __SEGGER_RTL_float32_llround_inline(x);
}

/*********************************************************************
*
*       llround()
*
*  Function description
*    Round to nearest integer, double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long long __SEGGER_RTL_PUBLIC_API llround(double x) {
  return __SEGGER_RTL_float64_llround_inline(x);
}

/*********************************************************************
*
*       llroundl()
*
*  Function description
*    Round to nearest integer, long double.
*
*  Parameters
*    x - Value to compute nearest integer of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return the nearest integer value to x.
*
*  Thread safety
*    Safe.
*/
long long __SEGGER_RTL_PUBLIC_API llroundl(long double x) {
  return llround(__SEGGER_RTL_ldouble_to_double(x));
}

/*********************************************************************
*
*       nextafterf()
*
*  Function description
*    Next machine-floating value, float.
*
*  Parameters
*    x - Value to step from.
*    y - Director to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API nextafterf(float x, float y) {
  return __SEGGER_RTL_float32_nextafter_inline(x, y);
}

/*********************************************************************
*
*       nextafter()
*
*  Function description
*    Next machine-floating value, double.
*
*  Parameters
*    x - Value to step from.
*    y - Director to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API nextafter(double x, double y) {
  return __SEGGER_RTL_float64_nextafter_inline(x, y);
}

/*********************************************************************
*
*       nextafterl()
*
*  Function description
*    Next machine-floating value, long double.
*
*  Parameters
*    x - Value to step from.
*    y - Director to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API nextafterl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(nextafter(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       nexttowardf()
*
*  Function description
*    Next machine-floating value, float.
*
*  Parameters
*    x - Value to step from.
*    y - Direction to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API nexttowardf(float x, long double y) {
  return __SEGGER_RTL_float32_nextafter_inline(x, SEGGER_LD2F(y));
}

/*********************************************************************
*
*       nexttoward()
*
*  Function description
*    Next machine-floating value, double.
*
*  Parameters
*    x - Value to step from.
*    y - Direction to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API nexttoward(double x, long double y) {
  return __SEGGER_RTL_float64_nextafter_inline(x, SEGGER_LD2D(y));
}

/*********************************************************************
*
*       nexttowardl()
*
*  Function description
*    Next machine-floating value, long double.
*
*  Parameters
*    x - Value to step from.
*    y - Direction to step in.
*
*  Return value
*    Next machine-floating value after x in direction of y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API nexttowardl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(nexttoward(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       nanf()
*
*  Function description
*    Parse NaN, float.
*
*  Parameters
*    tag - NaN tag.
*
*  Return value
*    Quiet NaN formed from tag.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API nanf(const char *tag) {
  return __SEGGER_RTL_float32_nan_inline(tag);
}

/*********************************************************************
*
*       nan()
*
*  Function description
*    Parse NaN, double.
*
*  Parameters
*    tag - NaN tag.
*
*  Return value
*    Quiet NaN formed from tag.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API nan(const char *tag) {
  return __SEGGER_RTL_float64_nan_inline(tag);
}

/*********************************************************************
*
*       nanl()
*
*  Function description
*    Parse NaN, long double.
*
*  Parameters
*    tag - NaN tag.
*
*  Return value
*    Quiet NaN formed from tag.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API nanl(const char *tag) {
  return __SEGGER_RTL_double_to_ldouble(nan(tag));
}

/*********************************************************************
*
*       sinf()
*
*  Function description
*    Calculate sine, float.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API sinf(float x) {
  return __SEGGER_RTL_float32_sin_inline(x);
}

/*********************************************************************
*
*       sin()
*
*  Function description
*    Calculate sine, double.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API sin(double x) {
  return __SEGGER_RTL_float64_sin_inline(x);
}

/*********************************************************************
*
*       sinl()
*
*  Function description
*    Calculate sine, long double.
*
*  Parameters
*    x - Angle to compute sine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular sine of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API sinl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(sin(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       cosf()
*
*  Function description
*    Calculate cosine, float.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API cosf(float x) {
  return __SEGGER_RTL_float32_cos_inline(x);
}

/*********************************************************************
*
*       cos()
*
*  Function description
*    Calculate cosine, double.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API cos(double x) {
  return __SEGGER_RTL_float64_cos_inline(x);
}

/*********************************************************************
*
*       cosl()
*
*  Function description
*    Calculate cosine, long double.
*
*  Parameters
*    x - Angle to compute cosine of, radians.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return NaN.
*    * Else, return circular cosine of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API cosl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(cos(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       tanf()
*
*  Function description
*    Compute tangent, float.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API tanf(float x) {
  return __SEGGER_RTL_float32_tan_inline(x);
}

/*********************************************************************
*
*       tan()
*
*  Function description
*    Compute tangent, double.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API tan(double x) {
  return __SEGGER_RTL_float64_tan_inline(x);
}

/*********************************************************************
*
*       tanl()
*
*  Function description
*    Compute tangent, long double.
*
*  Parameters
*    x - Angle to compute tangent of, radians.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return NaN.
*    * If x is NaN, return x.
*    * Else, return tangent of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API tanl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(tan(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       sincosf()
*
*  Function description
*    Calculate sine and cosine, float.
*
*  Parameters
*    x    - Angle to compute sine and cosine of, radians.
*    pSin - Pointer to object that receives the sine of x.
*    pCos - Pointer to object that receives the cosine of x.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API sincosf(float x, float *pSin, float *pCos) {
  *pSin = __SEGGER_RTL_float32_sin_inline(x);
  *pCos = __SEGGER_RTL_float32_cos_inline(x);
}

/*********************************************************************
*
*       sincos()
*
*  Function description
*    Calculate sine and cosine, double.
*
*  Parameters
*    x    - Angle to compute sine and cosine of, radians.
*    pSin - Pointer to object that receives the sine of x.
*    pCos - Pointer to object that receives the cosine of x.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API sincos(double x, double *pSin, double *pCos) {
  *pSin = __SEGGER_RTL_float64_sin_inline(x);
  *pCos = __SEGGER_RTL_float64_cos_inline(x);
}

/*********************************************************************
*
*       sincosl()
*
*  Function description
*    Calculate sine and cosine, long double.
*
*  Parameters
*    x    - Angle to compute sine and cosine of, radians.
*    pSin - Pointer to object that receives the sine of x.
*    pCos - Pointer to object that receives the cosine of x.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API sincosl(long double x, long double *pSin, long double *pCos) {
  *pSin = __SEGGER_RTL_double_to_ldouble(__SEGGER_RTL_float64_sin_inline(__SEGGER_RTL_ldouble_to_double(x)));
  *pCos = __SEGGER_RTL_double_to_ldouble(__SEGGER_RTL_float64_cos_inline(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       asinf()
*
*  Function description
*    Compute inverse sine, float.
*
*  Parameters
*    x - Value to compute inverse sine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular sine of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular sine of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API asinf(float x) {
  return __SEGGER_RTL_float32_asin_inline(x);
}

/*********************************************************************
*
*       asin()
*
*  Function description
*    Compute inverse sine, double.
*
*  Parameters
*    x - Value to compute inverse sine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular sine of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular sine of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API asin(double x) {
  return __SEGGER_RTL_float64_asin_inline(x);
}

/*********************************************************************
*
*       asinl()
*
*  Function description
*    Compute inverse sine, long double.
*
*  Parameters
*    x - Value to compute inverse sine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular sine of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular sine of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API asinl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(asin(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       acosf()
*
*  Function description
*    Compute inverse cosine, float.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API acosf(float x) {
  return __SEGGER_RTL_float32_acos_inline(x);
}

/*********************************************************************
*
*       acos()
*
*  Function description
*    Compute inverse cosine, double.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API acos(double x) {
  return __SEGGER_RTL_float64_acos_inline(x);
}

/*********************************************************************
*
*       acosl()
*
*  Function description
*    Compute inverse cosine, long double.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    circular cosine of x.  The principal value lies in the interval
*    [0, Pi] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * Else, return inverse circular cosine of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API acosl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(acos(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       atanf()
*
*  Function description
*    Compute inverse tangent, float.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API atanf(float x) {
  return __SEGGER_RTL_float32_atan_inline(x);
}

/*********************************************************************
*
*       atan()
*
*  Function description
*    Compute inverse tangent, double.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API atan(double x) {
  return __SEGGER_RTL_float64_atan_inline(x);
}

/*********************************************************************
*
*       atanl()
*
*  Function description
*    Compute inverse tangent, long double.
*
*  Parameters
*    x - Value to compute inverse tangent of.
*
*  Additional information
*    Calculates the principal value, in radians, of the inverse
*    tangent of x.  The principal value lies in the interval
*    [-Pi/2, Pi/2] radians. 
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return inverse tangent of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API atanl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(atan(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       atan2f()
*
*  Function description
*    Compute inverse tangent, with quadrant, float.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API atan2f(float y, float x) {
  return __SEGGER_RTL_float32_atan2_inline(y, x);
}
  
/*********************************************************************
*
*       atan2()
*
*  Function description
*    Compute inverse tangent, with quadrant, double.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API atan2(double y, double x) {
  return __SEGGER_RTL_float64_atan2_inline(y, x);
}

/*********************************************************************
*
*       atan2l()
*
*  Function description
*    Compute inverse tangent, with quadrant, long double.
*
*  Parameters
*    y - Rise value of angle.
*    x - Run value of angle.
*
*  Additional information
*    This calculates the value, in radians, of the inverse tangent 
*    of y divided by x using the signs of x and y to compute the quadrant
*    of the return value. The principal value lies in the interval 
*    [-Pi, +Pi] radians. 
*
*  Return value
*    Inverse tangent of y/x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API atan2l(long double y, long double x) {
  return __SEGGER_RTL_double_to_ldouble(atan2(__SEGGER_RTL_ldouble_to_double(y), __SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       sqrtf()
*
*  Function description
*    Compute square root, float.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    sqrt() computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrt() deviates and always uses IEC 60559 semantics. 
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API sqrtf(float x) {
  return __SEGGER_RTL_float32_sqrt_inline(x);
}

/*********************************************************************
*
*       sqrt()
*
*  Function description
*    Compute square root, double.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    sqrt() computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrt() deviates and always uses IEC 60559 semantics. 
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API sqrt(double x) {
  return __SEGGER_RTL_float64_sqrt_inline(x);
}

/*********************************************************************
*
*       sqrtl()
*
*  Function description
*    Compute square root, long double.
*
*  Parameters
*    x - Value to compute square root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return square root of x.
*
*  Additional information
*    sqrtl() computes the nonnegative square root of x.  C90 and C99
*    require that a domain error occurs if the argument is less than 
*    zero, sqrtl() deviates and always uses IEC 60559 semantics. 
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API sqrtl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(sqrt(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       cbrtf()
*
*  Function description
*    Compute cube root, float.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API cbrtf(float x) {
  return __SEGGER_RTL_float32_cbrt(x);
}

/*********************************************************************
*
*       cbrt()
*
*  Function description
*    Compute cube root, double.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API cbrt(double x) {
  return __SEGGER_RTL_float64_cbrt(x);
}

/*********************************************************************
*
*       cbrtl()
*
*  Function description
*    Compute cube root, long double.
*
*  Parameters
*    x - Value to compute cube root of.
*
*  Return value
*    * If x is zero, return x.
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return cube root of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API cbrtl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(cbrt(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       rsqrtf()
*
*  Function description
*    Compute reciprocal square root, float.
*
*  Parameters
*    x - Value to compute reciprocal square root of.
*
*  Return value
*    * If x is +/-zero, return +/-infinity.
*    * If x is positively infinite, return 0.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return reciprocal square root of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API rsqrtf(float x) {
  return __SEGGER_RTL_float32_rsqrt_inline(x);
}

/*********************************************************************
*
*       rsqrt()
*
*  Function description
*    Compute reciprocal square root, double.
*
*  Parameters
*    x - Value to compute reciprocal square root of.
*
*  Return value
*    * If x is +/-zero, return +/-infinity.
*    * If x is positively infinite, return 0.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return reciprocal square root of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API rsqrt(double x) {
  return __SEGGER_RTL_float64_rsqrt_inline(x);
}

/*********************************************************************
*
*       rsqrtl()
*
*  Function description
*    Compute reciprocal square root, long double.
*
*  Parameters
*    x - Value to compute reciprocal square root of.
*
*  Return value
*    * If x is +/-zero, return +/-infinity.
*    * If x is positively infinite, return 0.
*    * If x is NaN, return x.
*    * If x < 0, return NaN.
*    * Else, return reciprocal square root of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API rsqrtl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(rsqrt(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       logf()
*
*  Function description
*    Compute natural logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API logf(float x) {
  return __SEGGER_RTL_float32_log_inline(x);
}

/*********************************************************************
*
*       log()
*
*  Function description
*    Compute natural logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return -Inf.
*    * If x is +Inf, return +Inf.
*    * ELse, return base-e logarithm of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API log(double x) {
  return __SEGGER_RTL_float64_log_inline(x);
}

/*********************************************************************
*
*       logl()
*
*  Function description
*    Compute natural logarithm, long double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return -Inf.
*    * If x is +Inf, return +Inf.
*    * ELse, return base-e logarithm of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API logl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(log(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*      log1pf()
*
*  Function description
*    Compute natural logarithm plus one, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API log1pf(float x) {
  return __SEGGER_RTL_float32_log1p_inline(x);
}

/*********************************************************************
*
*      log1p()
*
*  Function description
*    Compute natural logarithm plus one, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API log1p(double x) {
  return __SEGGER_RTL_float64_log1p_inline(x);
}

/*********************************************************************
*
*      log1pl()
*
*  Function description
*    Compute natural logarithm plus one, long double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-e logarithm of x+1.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API log1pl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(log1p(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       log2f()
*
*  Function description
*    Compute base-2 logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API log2f(float x) {
  return __SEGGER_RTL_float32_log2(x);
}

/*********************************************************************
*
*       log2()
*
*  Function description
*    Compute base-2 logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API log2(double x) {
  return __SEGGER_RTL_float64_log2(x);
}

/*********************************************************************
*
*       log2l()
*
*  Function description
*    Compute base-2 logarithm, long double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API log2l(long double x) {
  return __SEGGER_RTL_double_to_ldouble(log2(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       log10f()
*
*  Function description
*    Compute common logarithm, float.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API log10f(float x) {
  return __SEGGER_RTL_float32_log10_inline(x);
}

/*********************************************************************
*
*       log10()
*
*  Function description
*    Compute common logarithm, double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API log10(double x) {
  return __SEGGER_RTL_float64_log10_inline(x);
}

/*********************************************************************
*
*       log10l()
*
*  Function description
*    Compute common logarithm, long double.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    * If x = NaN, return x.
*    * If x < 0, return NaN.
*    * If x = 0, return negative infinity.
*    * If x is positively infinite, return infinity.
*    * ELse, return base-10 logarithm of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API log10l(long double x) {
  return __SEGGER_RTL_double_to_ldouble(log10(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       expf()
*
*  Function description
*    Compute base-e exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API expf(float x) {
  return __SEGGER_RTL_float32_exp_inline(x);
}

/*********************************************************************
*
*       exp()
*
*  Function description
*    Compute base-e exponential, double.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API exp(double x) {
  return __SEGGER_RTL_float64_exp_inline(x);
}

/*********************************************************************
*
*       expl()
*
*  Function description
*    Compute base-e exponential, long double.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API expl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(exp(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       exp2f()
*
*  Function description
*    Compute base-2 exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API exp2f(float x) {
  return __SEGGER_RTL_float32_pow_inline(2.0f, x);
}

/*********************************************************************
*
*       exp2()
*
*  Function description
*    Compute base-2 exponential, double.
*
*  Parameters
*    x - Value to compute base-2 exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API exp2(double x) {
  return __SEGGER_RTL_float64_pow_inline(2.0, x);
}

/*********************************************************************
*
*       exp2l()
*
*  Function description
*    Compute base-2 exponential, long double.
*
*  Parameters
*    x - Value to compute base-2 exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API exp2l(long double x) {
  return __SEGGER_RTL_double_to_ldouble(exp2(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       exp10f()
*
*  Function description
*    Compute base-10 exponential, float.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API exp10f(float x) {
  return __SEGGER_RTL_float32_exp10_inline(x);
}

/*********************************************************************
*
*       exp10()
*
*  Function description
*    Compute base-10 exponential, double.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API exp10(double x) {
  return __SEGGER_RTL_float64_exp10_inline(x);
}

/*********************************************************************
*
*       exp10l()
*
*  Function description
*    Compute base-10 exponential, long double.
*
*  Parameters
*    x - Value to compute base-e exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is positively infinite, return x.
*    * If x is negatively infinite, return 0.
*    * Else, return base-e exponential of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API exp10l(long double x) {
  return __SEGGER_RTL_double_to_ldouble(exp10(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       expm1f()
*
*  Function description
*    Compute base-e exponential, modified, float.
*
*  Parameters
*    x - Value to compute exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return base-e exponential of x minus 1 (e**x - 1).
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API expm1f(float x) {
#if __SEGGER_RTL_SCALED_INTEGER >= 1
  return __SEGGER_RTL_float32_expm1_scaled_integer(x);
#else
  return SEGGER_SUBF(SEGGER_EXPF(x), 1);
#endif
}

/*********************************************************************
*
*       expm1()
*
*  Function description
*    Compute base-e exponential, modified, double.
*
*  Parameters
*    x - Value to compute exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return base-e exponential of x minus 1 (e**x - 1).
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API expm1(double x) {
  return SEGGER_SUB(SEGGER_EXP(x), 1);
}

/*********************************************************************
*
*       expm1l()
*
*  Function description
*    Compute base-e exponential, modified, long double.
*
*  Parameters
*    x - Value to compute exponential of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return base-e exponential of x minus 1 (e**x - 1).
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API expm1l(long double x) {
  return __SEGGER_RTL_double_to_ldouble(expm1(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       powf()
*
*  Function description
*    Raise to power, float.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API powf(float x, float y) {
  return __SEGGER_RTL_float32_pow_inline(x, y);
}

/*********************************************************************
*
*       pow()
*
*  Function description
*    Raise to power, double.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API pow(double x, double y) {
  return __SEGGER_RTL_float64_pow_inline(x, y);
}

/*********************************************************************
*
*       powl()
*
*  Function description
*    Raise to power, long double.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power y.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API powl(long double x, long double y) {
  return __SEGGER_RTL_double_to_ldouble(pow(__SEGGER_RTL_ldouble_to_double(x), __SEGGER_RTL_ldouble_to_double(y)));
}

/*********************************************************************
*
*       tgammaf()
*
*  Function description
*    Gamma function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    gamma(x).
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API tgammaf(float x) {
  return __SEGGER_RTL_float32_tgamma_inline(x);
}

/*********************************************************************
*
*       tgamma()
*
*  Function description
*    Gamma function, double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    gamma(x).
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API tgamma(double x) {
  return __SEGGER_RTL_float64_tgamma_inline(x);
}

/*********************************************************************
*
*       tgammal()
*
*  Function description
*    Gamma function, long double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    gamma(x).
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API tgammal(long double x) {
  return __SEGGER_RTL_double_to_ldouble(tgamma(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       lgammaf()
*
*  Function description
*    Log-Gamma function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    log(gamma(x)).
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API lgammaf(float x) {
  return __SEGGER_RTL_float32_lgamma_inline(x);
}

/*********************************************************************
*
*       lgamma()
*
*  Function description
*    Log-Gamma function, double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    log(gamma(x)).
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API lgamma(double x) {
  return __SEGGER_RTL_float64_lgamma_inline(x);
}

/*********************************************************************
*
*       lgammal()
*
*  Function description
*    Log-Gamma function, long double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    log(gamma(x)).
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API lgammal(long double x) {
  return __SEGGER_RTL_double_to_ldouble(lgamma(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       erff()
*
*  Function description
*    Error function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erf(x).
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API erff(float x) {
  return __SEGGER_RTL_float32_erf_inline(x);
}

/*********************************************************************
*
*       erf()
*
*  Function description
*    Error function, double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erf(x).
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API erf(double x) {
  return __SEGGER_RTL_float64_erf_inline(x);
}

/*********************************************************************
*
*       erfl()
*
*  Function description
*    Error function, long double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erf(x).
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API erfl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(erf(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       erfcf()
*
*  Function description
*    Complementary error function, float.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erfc(x).
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API erfcf(float x) {
  return __SEGGER_RTL_float32_erfc_inline(x);
}

/*********************************************************************
*
*       erfc()
*
*  Function description
*    Complementary error function, double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erfc(x).
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API erfc(double x) {
  return __SEGGER_RTL_float64_erfc_inline(x);
}

/*********************************************************************
*
*       erfcl()
*
*  Function description
*    Complementary error function, long double.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    erfc(x).
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API erfcl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(erfc(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       sinhf()
*
*  Function description
*    Compute hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API sinhf(float x) {
  return __SEGGER_RTL_float32_sinh_inline(x);
}

/*********************************************************************
*
*       sinh()
*
*  Function description
*    Compute hyperbolic sine, double.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API sinh(double x) {
  return __SEGGER_RTL_float64_sinh_inline(x);
}

/*********************************************************************
*
*       sinhl()
*
*  Function description
*    Compute hyperbolic sine, long double.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return x.
*    * Else, return hyperbolic sine of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API sinhl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(sinh(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       coshf()
*
*  Function description
*    Compute hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API coshf(float x) {
  return __SEGGER_RTL_float32_cosh_inline(x);
}

/*********************************************************************
*
*       cosh()
*
*  Function description
*    Compute hyperbolic cosine, double.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API cosh(double x) {
  return __SEGGER_RTL_float64_cosh_inline(x);
}
  
/*********************************************************************
*
*       coshl()
*
*  Function description
*    Compute hyperbolic cosine, long double.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    * If x is NaN, return x.
*    * If x is infinite, return +Inf.
*    * Else, return hyperbolic cosine of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API coshl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(cosh(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       tanhf()
*
*  Function description
*    Compute hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API tanhf(float x) {
  return __SEGGER_RTL_float32_tanh_inline(x);
}

/*********************************************************************
*
*       tanh()
*
*  Function description
*    Compute hyperbolic tangent, double.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API tanh(double x) {
  return __SEGGER_RTL_float64_tanh_inline(x);
}

/*********************************************************************
*
*       tanhl()
*
*  Function description
*    Compute hyperbolic tangent, long double.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * Else, return hyperbolic tangent of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API tanhl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(tanh(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       asinhf()
*
*  Function description
*    Compute inverse hyperbolic sine, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sine of.
*
*  Additional information
*    Calculates the inverse hyperbolic sine of x. 
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic sine of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API asinhf(float x) {
  return __SEGGER_RTL_float32_asinh_inline(x);
}

/*********************************************************************
*
*       asinh()
*
*  Function description
*    Compute inverse hyperbolic sine, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sine of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic sine of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API asinh(double x) {
  return __SEGGER_RTL_float64_asinh_inline(x);
}

/*********************************************************************
*
*       asinhl()
*
*  Function description
*    Compute inverse hyperbolic sine, long double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sine of.
*
*  Return value
*    * If x is infinite, return x.
*    * If x is NaN, return x.
*    * Else, return inverse hyperbolic sine of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API asinhl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(asinh(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       acoshf()
*
*  Function description
*    Compute inverse hyperbolic cosine, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Return value
*    * If x < 1, return NaN.
*    * If x is NaN, return x.
*    * Else, return non-negative inverse hyperbolic cosine of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API acoshf(float x) {
  return __SEGGER_RTL_float32_acosh_inline(x);
}

/*********************************************************************
*
*       acosh()
*
*  Function description
*    Compute inverse hyperbolic cosine, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Return value
*    * If x < 1, return NaN.
*    * If x is NaN, return x.
*    * Else, return non-negative inverse hyperbolic cosine of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API acosh(double x) {
  return __SEGGER_RTL_float64_acosh_inline(x);
}

/*********************************************************************
*
*       acoshl()
*
*  Function description
*    Compute inverse hyperbolic cosine, long double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Return value
*    * If x < 1, return NaN.
*    * If x is NaN, return x.
*    * Else, return non-negative inverse hyperbolic cosine of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API acoshl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(acosh(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       atanhf()
*
*  Function description
*    Compute inverse hyperbolic tangent, float.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * If x = +/-1, return +/-infinity.
*    * Else, return non-negative inverse hyperbolic tangent of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API atanhf(float x) {
  return __SEGGER_RTL_float32_atanh_inline(x);
}

/*********************************************************************
*
*       atanh()
*
*  Function description
*    Compute inverse hyperbolic tangent, double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * If x = +/-1, return +/-infinity.
*    * Else, return non-negative inverse hyperbolic tangent of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API atanh(double x) {
  return __SEGGER_RTL_float64_atanh_inline(x);
}

/*********************************************************************
*
*       atanhl()
*
*  Function description
*    Compute inverse hyperbolic tangent, long double.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Return value
*    * If x is NaN, return x.
*    * If |x| > 1, return NaN.
*    * If x = +/-1, return +/-infinity.
*    * Else, return non-negative inverse hyperbolic tangent of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API atanhl(long double x) {
  return __SEGGER_RTL_double_to_ldouble(atanh(__SEGGER_RTL_ldouble_to_double(x)));
}

/*********************************************************************
*
*       crealf()
*
*  Function description
*    Real part, float complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    The real part of the complex value.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API crealf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  //
  return z.u.part.Re;
}

/*********************************************************************
*
*       creal()
*
*  Function description
*    Real part, double complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    The real part of the complex value.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API creal(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  //
  return z.u.part.Re;
}

/*********************************************************************
*
*       creall()
*
*  Function description
*    Real part, long double complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    The real part of the complex value.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API creall(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  __SEGGER_RTL_LDOUBLE_COMPLEX z;
  //
  z.u.value = x;
  //
  return z.u.part.Re;
}

/*********************************************************************
*
*       cimagf()
*
*  Function description
*    Imaginary part, float complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    The imaginary part of the complex value.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API cimagf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  //
  return z.u.part.Im;
}

/*********************************************************************
*
*       cimag()
*
*  Function description
*    Imaginary part, double complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    The imaginary part of the complex value.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API cimag(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  //
  return z.u.part.Im;
}

/*********************************************************************
*
*       cimagl()
*
*  Function description
*    Imaginary part, long double complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    The imaginary part of the complex value.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API cimagl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  __SEGGER_RTL_LDOUBLE_COMPLEX z;
  //
  z.u.value = x;
  //
  return z.u.part.Im;
}

/*********************************************************************
*
*       cabsf()
*
*  Function description
*    Compute magnitude, float complex.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    The magnitude of x, |x|.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API cabsf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  return SEGGER_HYPOTF(crealf(x), cimagf(x));
}

/*********************************************************************
*
*       cabs()
*
*  Function description
*    Compute magnitude, double complex.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    The magnitude of x, |x|.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API cabs(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  return SEGGER_HYPOT(creal(x), cimag(x));
}

/*********************************************************************
*
*       cabsl()
*
*  Function description
*    Compute magnitude, long double complex.
*
*  Parameters
*    x - Value to compute magnitude of.
*
*  Return value
*    The magnitude of x, |x|.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API cabsl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_double_to_ldouble(cabs(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       cargf()
*
*  Function description
*    Compute phase, float complex.
*
*  Parameters
*    x - Value to compute phase of.
*
*  Return value
*    The phase of x.
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API cargf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  return SEGGER_ATAN2F(cimagf(x), crealf(x));
}

/*********************************************************************
*
*       carg()
*
*  Function description
*    Compute phase, double complex.
*
*  Parameters
*    x - Value to compute phase of.
*
*  Return value
*    The phase of x.
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API carg(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  return SEGGER_ATAN2(cimag(x), creal(x));
}

/*********************************************************************
*
*       cargl()
*
*  Function description
*    Compute phase, long double complex.
*
*  Parameters
*    x - Value to compute phase of.
*
*  Return value
*    The phase of x.
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API cargl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_double_to_ldouble(carg(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       cprojf()
*
*  Function description
*    Project, float complex.
*
*  Parameters
*    x - Value to project.
*
*  Return value
*    The projection of x to the Reimann sphere.
*
*  Additional information
*    x projects to x, except that all complex infinities (even those with
*    one infinite part and one NaN part) project to positive infinity on
*    the real axis. If x has an infinite part, then cproj(x) is be
*    equivalent to:
*
*    * INFINITY + I * copysign(0.0, cimag(x))
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API cprojf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_cproj(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       cproj()
*
*  Function description
*    Project, double complex.
*
*  Parameters
*    x - Value to project.
*
*  Return value
*    The projection of x to the Reimann sphere.
*
*  Additional information
*    x projects to x, except that all complex infinities (even those with
*    one infinite part and one NaN part) project to positive infinity on
*    the real axis. If x has an infinite part, then cproj(x) is be
*    equivalent to:
*
*    * INFINITY + I * copysign(0.0, cimag(x))
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API cproj(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_cproj(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       cprojl()
*
*  Function description
*    Project, long double complex.
*
*  Parameters
*    x - Value to project.
*
*  Return value
*    The projection of x to the Reimann sphere.
*
*  Additional information
*    x projects to x, except that all complex infinities (even those with
*    one infinite part and one NaN part) project to positive infinity on
*    the real axis. If x has an infinite part, then cproj(x) is be
*    equivalent to:
*
*    * INFINITY + I * copysignl(0.0, cimagl(x))
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API cprojl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(cproj(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       conjf()
*
*  Function description
*    Conjugate, float complex.
*
*  Parameters
*    x - Value to conjugate.
*
*  Return value
*    The complex conjugate of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API conjf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_ccnj(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       conj()
*
*  Function description
*    Conjugate, double complex.
*
*  Parameters
*    x - Value to conjugate.
*
*  Return value
*    The complex conjugate of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API conj(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_ccnj(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       conjl()
*
*  Function description
*    Conjugate, long double complex.
*
*  Parameters
*    x - Value to conjugate.
*
*  Return value
*    The complex conjugate of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API conjl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(conj(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       csqrtf()
*
*  Function description
*    Square root, float complex.
*
*  Parameters
*    x - Value to compute squate root of.
*
*  Return value
*    The square root of x according to the following table:
*
*    +--------------+-----------------------------------------------------+
*    | Argument     | csqrt(Argument)                                     |
*    +--------------+-----------------------------------------------------+
*    | +/-0 + 0.i   | +0 + 0i                                             |
*    | a + Inf.i    | +Inf + Inf.i, for all a                             |
*    | a + NaN.i    | +NaN + NaN.i, for finite a                          |
*    | -Inf + b.i   | +0 + Inf.i for finite positive-signed b             |
*    | +Inf + b.i   | +Inf + 0.i, for finite positive-signed b            |
*    | +Inf + Inf.i | +Inf + Pi/4.i                                       |
*    | -Inf + NaN.i | +NaN + +/Inf.i, sign of imaginary part unspecified  |
*    | +Inf + NaN.i | +Inf + NaN.i                                        |
*    | NaN + b.i    | NaN + NaN.i, for finite b                           |
*    | NaN + Inf.i  | +Inf + Inf.i                                        |
*    | NaN + NaN.i  | NaN + NaN.i                                         |
*    +--------------+-----------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * csqrt(conj(z)) = conj(csqrt(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API csqrtf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_csqrt(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       csqrt()
*
*  Function description
*    Square root, double complex.
*
*  Parameters
*    x - Value to compute squate root of.
*
*  Return value
*    The square root of x according to the following table:
*
*    +--------------+-----------------------------------------------------+
*    | Argument     | csqrt(Argument)                                     |
*    +--------------+-----------------------------------------------------+
*    | +/-0 + 0.i   | +0 + 0i                                             |
*    | a + Inf.i    | +Inf + Inf.i, for all a                             |
*    | a + NaN.i    | +NaN + NaN.i, for finite a                          |
*    | -Inf + b.i   | +0 + Inf.i for finite positive-signed b             |
*    | +Inf + b.i   | +Inf + 0.i, for finite positive-signed b            |
*    | +Inf + Inf.i | +Inf + Pi/4.i                                       |
*    | -Inf + NaN.i | +NaN + +/Inf.i, sign of imaginary part unspecified  |
*    | +Inf + NaN.i | +Inf + NaN.i                                        |
*    | NaN + b.i    | NaN + NaN.i, for finite b                           |
*    | NaN + Inf.i  | +Inf + Inf.i                                        |
*    | NaN + NaN.i  | NaN + NaN.i                                         |
*    +--------------+-----------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * csqrt(conj(z)) = conj(csqrt(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API csqrt(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_csqrt(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       csqrtl()
*
*  Function description
*    Square root, long double complex.
*
*  Parameters
*    x - Value to compute squate root of.
*
*  Return value
*    The square root of x according to the following table:
*
*    +--------------+-----------------------------------------------------+
*    | Argument     | csqrt(Argument)                                     |
*    +--------------+-----------------------------------------------------+
*    | +/-0 + 0.i   | +0 + 0i                                             |
*    | a + Inf.i    | +Inf + Inf.i, for all a                             |
*    | a + NaN.i    | +NaN + NaN.i, for finite a                          |
*    | -Inf + b.i   | +0 + Inf.i for finite positive-signed b             |
*    | +Inf + b.i   | +Inf + 0.i, for finite positive-signed b            |
*    | +Inf + Inf.i | +Inf + Pi/4.i                                       |
*    | -Inf + NaN.i | +NaN + +/Inf.i, sign of imaginary part unspecified  |
*    | +Inf + NaN.i | +Inf + NaN.i                                        |
*    | NaN + b.i    | NaN + NaN.i, for finite b                           |
*    | NaN + Inf.i  | +Inf + Inf.i                                        |
*    | NaN + NaN.i  | NaN + NaN.i                                         |
*    +--------------+-----------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * csqrt(conj(z)) = conj(csqrt(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API csqrtl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(csqrt(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       clogf()
*
*  Function description
*    Compute natural logarithm, float complex.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    The natural logarithm of x according to the following table:
*
*    +-------------------+------------------------------------+
*    | Argument          | clog(Argument)                     |
*    +-------------------+------------------------------------+
*    | -0 + 0.i          | -Inf + Pi.i                        |
*    | +0 + 0.i          | -Inf + 0.i                         |
*    | a + Inf.i         | +Inf + Pi/2.i, for finite a        |
*    | a + NaN.i         | NaN + NaN.i, for finite a          |
*    | -Inf + b.i        | +Inf + Pi.i, for finite positive b |
*    | +Inf + b.i        | +Inf + 0.i, for finite positive b  |
*    | -Inf + Inf.i      | +Inf + 3Pi/4.i                     |
*    | +Inf + Inf.i      | +Inf + Pi/4.i                      |
*    | +/-Inf + NaN.i    | +Inf + NaN.i                       |
*    | NaN + b.i         | NaN + NaN.i, for finite b          |
*    | NaN + Inf.i       | +Inf + NaN.i                       |
*    | NaN + NaN.i       | NaN + NaN.i                        |
*    +-------------------+------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * clog(conj(z)) = conj(clog(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API clogf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_clog_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       clog()
*
*  Function description
*    Compute natural logarithm, double complex.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    The natural logarithm of x according to the following table:
*
*    +-------------------+------------------------------------+
*    | Argument          | clog(Argument)                     |
*    +-------------------+------------------------------------+
*    | -0 + 0.i          | -Inf + Pi.i                        |
*    | +0 + 0.i          | -Inf + 0.i                         |
*    | a + Inf.i         | +Inf + Pi/2.i, for finite a        |
*    | a + NaN.i         | NaN + NaN.i, for finite a          |
*    | -Inf + b.i        | +Inf + Pi.i, for finite positive b |
*    | +Inf + b.i        | +Inf + 0.i, for finite positive b  |
*    | -Inf + Inf.i      | +Inf + 3Pi/4.i                     |
*    | +Inf + Inf.i      | +Inf + Pi/4.i                      |
*    | +/-Inf + NaN.i    | +Inf + NaN.i                       |
*    | NaN + b.i         | NaN + NaN.i, for finite b          |
*    | NaN + Inf.i       | +Inf + NaN.i                       |
*    | NaN + NaN.i       | NaN + NaN.i                        |
*    +-------------------+------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * clog(conj(z)) = conj(clog(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API clog(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_clog_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       clogl()
*
*  Function description
*    Compute natural logarithm, long double complex.
*
*  Parameters
*    x - Value to compute logarithm of.
*
*  Return value
*    The natural logarithm of x according to the following table:
*
*    +-------------------+------------------------------------+
*    | Argument          | clog(Argument)                     |
*    +-------------------+------------------------------------+
*    | -0 + 0.i          | -Inf + Pi.i                        |
*    | +0 + 0.i          | -Inf + 0.i                         |
*    | a + Inf.i         | +Inf + Pi/2.i, for finite a        |
*    | a + NaN.i         | NaN + NaN.i, for finite a          |
*    | -Inf + b.i        | +Inf + Pi.i, for finite positive b |
*    | +Inf + b.i        | +Inf + 0.i, for finite positive b  |
*    | -Inf + Inf.i      | +Inf + 3Pi/4.i                     |
*    | +Inf + Inf.i      | +Inf + Pi/4.i                      |
*    | +/-Inf + NaN.i    | +Inf + NaN.i                       |
*    | NaN + b.i         | NaN + NaN.i, for finite b          |
*    | NaN + Inf.i       | +Inf + NaN.i                       |
*    | NaN + NaN.i       | NaN + NaN.i                        |
*    +-------------------+------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * clog(conj(z)) = conj(clog(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API clogl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(clog(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       cexpf()
*
*  Function description
*    Compute base-e exponential, float complex.
*
*  Parameters
*    x - Value to compute exponential of.
*
*  Return value
*    The base-e exponential of x=a+b.i according to the following table:
*
*    +--------------+-----------------------------------------------+
*    | Argument     | cexp(Argument)                                |
*    +--------------+-----------------------------------------------+
*    | -/-0 + 0.i   | +1 + 0.i                                      |
*    | a + Inf.i    | NaN + NaN.i, for finite a                     |
*    | a + NaN.i    | NaN + NaN.i, for finite a                     |
*    | +Inf + 0.i   | +Inf + 0.i, for finite positive b             |
*    | -Inf + b.i   | +0 cis(b) for finite b                        |
*    | +Inf + b.i   | +Inf cis(b) for finite nonzero b              |
*    | -Inf + Inf.i | +/-Inf + +/-0.i, signs unspecified            |
*    | +Inf + Inf.i | +/-Inf + i.NaN, sign of real part unspecified |
*    | -Inf + NaN.i | +/-0 + +/-0.i, signs unspecified              |
*    | +Inf + NaN.i | +/-Inf + NaN.i, sign of real part unspecified |
*    | NaN + 0.i    | NaN + 0.i                                     |
*    | NaN + b.i    | NaN + NaN.i, for nonzero b                    |
*    | NaN + NaN.i  | NaN + NaN.i                                   |
*    +--------------+-----------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality
*
*    * cexp(conj(x)) = conj(cexp(x)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API cexpf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_cexp(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       cexp()
*
*  Function description
*    Compute base-e exponential, double complex.
*
*  Parameters
*    x - Value to compute exponential of.
*
*  Return value
*    The base-e exponential of x=a+b.i according to the following table:
*
*    +--------------+-----------------------------------------------+
*    | Argument     | cexp(Argument)                                |
*    +--------------+-----------------------------------------------+
*    | -/-0 + 0.i   | +1 + 0.i                                      |
*    | a + Inf.i    | NaN + NaN.i, for finite a                     |
*    | a + NaN.i    | NaN + NaN.i, for finite a                     |
*    | +Inf + 0.i   | +Inf + 0.i, for finite positive b             |
*    | -Inf + b.i   | +0 cis(b) for finite b                        |
*    | +Inf + b.i   | +Inf cis(b) for finite nonzero b              |
*    | -Inf + Inf.i | +/-Inf + +/-0.i, signs unspecified            |
*    | +Inf + Inf.i | +/-Inf + i.NaN, sign of real part unspecified |
*    | -Inf + NaN.i | +/-0 + +/-0.i, signs unspecified              |
*    | +Inf + NaN.i | +/-Inf + NaN.i, sign of real part unspecified |
*    | NaN + 0.i    | NaN + 0.i                                     |
*    | NaN + b.i    | NaN + NaN.i, for nonzero b                    |
*    | NaN + NaN.i  | NaN + NaN.i                                   |
*    +--------------+-----------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality
*
*    * cexp(conj(x)) = conj(cexp(x)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API cexp(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_cexp(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       cexpl()
*
*  Function description
*    Compute base-e exponential, long double complex.
*
*  Parameters
*    x - Value to compute exponential of.
*
*  Return value
*    The base-e exponential of x=a+b.i according to the following table:
*
*    +--------------+-----------------------------------------------+
*    | Argument     | cexp(Argument)                                |
*    +--------------+-----------------------------------------------+
*    | -/-0 + 0.i   | +1 + 0.i                                      |
*    | a + Inf.i    | NaN + NaN.i, for finite a                     |
*    | a + NaN.i    | NaN + NaN.i, for finite a                     |
*    | +Inf + 0.i   | +Inf + 0.i, for finite positive b             |
*    | -Inf + b.i   | +0 cis(b) for finite b                        |
*    | +Inf + b.i   | +Inf cis(b) for finite nonzero b              |
*    | -Inf + Inf.i | +/-Inf + +/-0.i, signs unspecified            |
*    | +Inf + Inf.i | +/-Inf + i.NaN, sign of real part unspecified |
*    | -Inf + NaN.i | +/-0 + +/-0.i, signs unspecified              |
*    | +Inf + NaN.i | +/-Inf + NaN.i, sign of real part unspecified |
*    | NaN + 0.i    | NaN + 0.i                                     |
*    | NaN + b.i    | NaN + NaN.i, for nonzero b                    |
*    | NaN + NaN.i  | NaN + NaN.i                                   |
*    +--------------+-----------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality
*
*    * cexp(conj(x)) = conj(cexp(x)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API cexpl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(cexp(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       cpowf()
*
*  Function description
*    Power, float complex.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power of y.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API cpowf(__SEGGER_RTL_FLOAT32_C_COMPLEX x, __SEGGER_RTL_FLOAT32_C_COMPLEX y) {
  __SEGGER_RTL_FLOAT32_COMPLEX a;
  __SEGGER_RTL_FLOAT32_COMPLEX b;
  //
  a.u.value = x;
  b.u.value = y;
  __SEGGER_RTL_float32_cpow(&a, &b);
  //
  return a.u.value;
}

/*********************************************************************
*
*       cpow()
*
*  Function description
*    Power, double complex.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power of y.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API cpow(__SEGGER_RTL_FLOAT64_C_COMPLEX x, __SEGGER_RTL_FLOAT64_C_COMPLEX y) {
  __SEGGER_RTL_FLOAT64_COMPLEX a;
  __SEGGER_RTL_FLOAT64_COMPLEX b;
  //
  a.u.value = x;
  b.u.value = y;
  __SEGGER_RTL_float64_cpow(&a, &b);
  //
  return a.u.value;
}

/*********************************************************************
*
*       cpowl()
*
*  Function description
*    Power, long double complex.
*
*  Parameters
*    x - Base.
*    y - Power.
*
*  Return value
*    Return x raised to the power of y.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API cpowl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x, __SEGGER_RTL_LDOUBLE_C_COMPLEX y) {
  return __SEGGER_RTL_complex_double_to_ldouble(
           cpow(__SEGGER_RTL_complex_ldouble_to_double(x),
                __SEGGER_RTL_complex_ldouble_to_double(y)));
}

/*********************************************************************
*
*       csinf()
*
*  Function description
*    Compute sine, float complex.
*
*  Parameters
*    x - Value to compute sine of.
*
*  Return value
*    The sine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API csinf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_csin_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       csin()
*
*  Function description
*    Compute sine, double complex.
*
*  Parameters
*    x - Value to compute sine of.
*
*  Return value
*    The sine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API csin(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_csin_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       csinl()
*
*  Function description
*    Compute sine, long double complex.
*
*  Parameters
*    x - Value to compute sine of.
*
*  Return value
*    The sine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API csinl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(csin(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       ccosf()
*
*  Function description
*    Compute cosine, float complex.
*
*  Parameters
*    x - Value to compute cosine of.
*
*  Return value
*    The cosine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API ccosf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_ccos_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       ccos()
*
*  Function description
*    Compute cosine, double complex.
*
*  Parameters
*    x - Value to compute cosine of.
*
*  Return value
*    The cosine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API ccos(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_ccos_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       ccosl()
*
*  Function description
*    Compute cosine, long double complex.
*
*  Parameters
*    x - Value to compute cosine of.
*
*  Return value
*    The cosine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API ccosl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(ccos(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       ctanf()
*
*  Function description
*    Compute tangent, float complex.
*
*  Parameters
*    x - Value to compute tangent of.
*
*  Return value
*    The tangent of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API ctanf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_ctan_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       ctan()
*
*  Function description
*    Compute tangent, double complex.
*
*  Parameters
*    x - Value to compute tangent of.
*
*  Return value
*    The tangent of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API ctan(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_ctan_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       ctanl()
*
*  Function description
*    Compute tangent, long double complex.
*
*  Parameters
*    x - Value to compute tangent of.
*
*  Return value
*    The tangent of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API ctanl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(ctan(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       csinhf()
*
*  Function description
*    Compute hyperbolic sine, float complex.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    The hyperbolic sine of x according to the following table:
*
*    +--------------+---------------------------------------------------+
*    | Argument     | csinh(Argument)                                   |
*    +--------------+---------------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                          |
*    | +0 + Inf.i   | +/-0 + NaN.i, sign of real part unspecified       |
*    | +0 + NaN.i   | +/-0 + NaN.i, sign of real part unspecified       |
*    | a + Inf.i    | NaN + NaN.i, for positive finite a                |
*    | a + NaN.i    | NaN + NaN.i, for finite nonzero a                 |
*    | +Inf + 0.i   | +Inf + 0.i                                        |
*    | +Inf + b.i   | +Inf x cos(b) + +Inf x sin(b).i for positive finite b |
*    | +Inf + Inf.i | +/-Inf + NaN.i, sign of real part unspecified     |
*    | +Inf + NaN.i | +/-Inf + NaN.i, sign of real part unspecified     |
*    | NaN + 0.i    | NaN + 0.i                                         |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                    |
*    | NaN + NaN.i  | NaN + NaN.i                                       |
*    +--------------+---------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * csinh(conj(z)) = conj(csinh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * csinh(-z) = -csinh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API csinhf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_csinh_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       csinh()
*
*  Function description
*    Compute hyperbolic sine, double complex.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    The hyperbolic sine of x according to the following table:
*
*    +--------------+---------------------------------------------------+
*    | Argument     | csinh(Argument)                                   |
*    +--------------+---------------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                          |
*    | +0 + Inf.i   | +/-0 + NaN.i, sign of real part unspecified       |
*    | +0 + NaN.i   | +/-0 + NaN.i, sign of real part unspecified       |
*    | a + Inf.i    | NaN + NaN.i, for positive finite a                |
*    | a + NaN.i    | NaN + NaN.i, for finite nonzero a                 |
*    | +Inf + 0.i   | +Inf + 0.i                                        |
*    | +Inf + b.i   | +Inf x cos(b) + +Inf x sin(b).i for positive finite b |
*    | +Inf + Inf.i | +/-Inf + NaN.i, sign of real part unspecified     |
*    | +Inf + NaN.i | +/-Inf + NaN.i, sign of real part unspecified     |
*    | NaN + 0.i    | NaN + 0.i                                         |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                    |
*    | NaN + NaN.i  | NaN + NaN.i                                       |
*    +--------------+---------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * csinh(conj(z)) = conj(csinh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * csinh(-z) = -csinh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API csinh(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_csinh_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       csinhl()
*
*  Function description
*    Compute hyperbolic sine, long double complex.
*
*  Parameters
*    x - Value to compute hyperbolic sine of.
*
*  Return value
*    The hyperbolic sine of x according to the following table:
*
*    +--------------+---------------------------------------------------+
*    | Argument     | csinh(Argument)                                   |
*    +--------------+---------------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                          |
*    | +0 + Inf.i   | +/-0 + NaN.i, sign of real part unspecified       |
*    | +0 + NaN.i   | +/-0 + NaN.i, sign of real part unspecified       |
*    | a + Inf.i    | NaN + NaN.i, for positive finite a                |
*    | a + NaN.i    | NaN + NaN.i, for finite nonzero a                 |
*    | +Inf + 0.i   | +Inf + 0.i                                        |
*    | +Inf + b.i   | +Inf x cos(b) + +Inf x sin(b).i for positive finite b |
*    | +Inf + Inf.i | +/-Inf + NaN.i, sign of real part unspecified     |
*    | +Inf + NaN.i | +/-Inf + NaN.i, sign of real part unspecified     |
*    | NaN + 0.i    | NaN + 0.i                                         |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                    |
*    | NaN + NaN.i  | NaN + NaN.i                                       |
*    +--------------+---------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * csinh(conj(z)) = conj(csinh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * csinh(-z) = -csinh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API csinhl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(csinh(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       ccoshf()
*
*  Function description
*    Compute hyperbolic cosine, float complex.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    The hyperbolic cosine of x according to the following table:
*
*    +--------------+--------------------------------------------------+
*    | Argument     | ccosh(Argument)                                  |
*    +--------------+--------------------------------------------------+
*    | +0 + 0.i     | +1 + 0.i                                         |
*    | +0 + Inf.i   | NaN + +/-0.i, sign of imaginary part unspecified |
*    | +0 + NaN.i   | NaN + +/-0.i, sign of imaginary part unspecified |
*    | a + Inf.i    | NaN + NaN.i, for finite nonzero a                |
*    | a + NaN.i    | NaN + NaN.i, for finite nonzero a                |
*    | +Inf + 0.i   | +Inf + 0.i                                       |
*    | +Inf + b.i   | +Inf x cos(b) + Inf x sin(b).i for finite nonzero b  |
*    | +Inf + Inf.i | +Inf + NaN.i                                     |
*    | +Inf + NaN.i | +Inf + NaN.i                                     |
*    | NaN + 0.i    | NaN + +/-0.i, sign of imaginary part unspecified |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                   |
*    | NaN + NaN.i  | NaN + NaN.i                                      |
*    +--------------+--------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * ccosh(conj(z)) = conj(ccosh(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API ccoshf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_ccosh_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       ccosh()
*
*  Function description
*    Compute hyperbolic cosine, double complex.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    The hyperbolic cosine of x according to the following table:
*
*    +--------------+--------------------------------------------------+
*    | Argument     | ccosh(Argument)                                  |
*    +--------------+--------------------------------------------------+
*    | +0 + 0.i     | +1 + 0.i                                         |
*    | +0 + Inf.i   | NaN + +/-0.i, sign of imaginary part unspecified |
*    | +0 + NaN.i   | NaN + +/-0.i, sign of imaginary part unspecified |
*    | a + Inf.i    | NaN + NaN.i, for finite nonzero a                |
*    | a + NaN.i    | NaN + NaN.i, for finite nonzero a                |
*    | +Inf + 0.i   | +Inf + 0.i                                       |
*    | +Inf + b.i   | +Inf x cos(b) + Inf x sin(b).i for finite nonzero b  |
*    | +Inf + Inf.i | +Inf + NaN.i                                     |
*    | +Inf + NaN.i | +Inf + NaN.i                                     |
*    | NaN + 0.i    | NaN + +/-0.i, sign of imaginary part unspecified |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                   |
*    | NaN + NaN.i  | NaN + NaN.i                                      |
*    +--------------+--------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * ccosh(conj(z)) = conj(ccosh(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API ccosh(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_ccosh_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       ccoshl()
*
*  Function description
*    Compute hyperbolic cosine, long double complex.
*
*  Parameters
*    x - Value to compute hyperbolic cosine of.
*
*  Return value
*    The hyperbolic cosine of x according to the following table:
*
*    +--------------+--------------------------------------------------+
*    | Argument     | ccosh(Argument)                                  |
*    +--------------+--------------------------------------------------+
*    | +0 + 0.i     | +1 + 0.i                                         |
*    | +0 + Inf.i   | NaN + +/-0.i, sign of imaginary part unspecified |
*    | +0 + NaN.i   | NaN + +/-0.i, sign of imaginary part unspecified |
*    | a + Inf.i    | NaN + NaN.i, for finite nonzero a                |
*    | a + NaN.i    | NaN + NaN.i, for finite nonzero a                |
*    | +Inf + 0.i   | +Inf + 0.i                                       |
*    | +Inf + b.i   | +Inf x cos(b) + Inf x sin(b).i for finite nonzero b  |
*    | +Inf + Inf.i | +Inf + NaN.i                                     |
*    | +Inf + NaN.i | +Inf + NaN.i                                     |
*    | NaN + 0.i    | NaN + +/-0.i, sign of imaginary part unspecified |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                   |
*    | NaN + NaN.i  | NaN + NaN.i                                      |
*    +--------------+--------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * ccosh(conj(z)) = conj(ccosh(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API ccoshl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(ccosh(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       ctanhf()
*
*  Function description
*    Compute hyperbolic tangent, float complex.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    The hyperbolic tangent of x according to the following table:
*
*    +--------------+-------------------------------------------------+
*    | Argument     | ctanh(Argument)                                 |
*    +--------------+-------------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                        |
*    | a + Inf.i    | NaN + NaN.i, for finite a                       |
*    | a + NaN.i    | NaN + NaN.i, for finite a                       |
*    | +Inf + b.i   | +1 + sin(2b) x 0.i for positive-signed finite b   |
*    | +Inf + Inf.i | +1 + +/-0.i, sign of imaginary part unspecified |
*    | +Inf + NaN.i | +1 + +/-0.i, sign of imaginary part unspecified |
*    | NaN + 0.i    | NaN + 0.i                                       |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                  |
*    | NaN + NaN.i  | NaN + NaN.i                                     |
*    +--------------+-------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * ctanhf(conj(z)) = conj(ctanhf(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * ctanhf(-z) = -ctanhf(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API ctanhf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_ctanh_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       ctanh()
*
*  Function description
*    Compute hyperbolic tangent, double complex.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    The hyperbolic tangent of x according to the following table:
*
*    +--------------+-------------------------------------------------+
*    | Argument     | ctanh(Argument)                                 |
*    +--------------+-------------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                        |
*    | a + Inf.i    | NaN + NaN.i, for finite a                       |
*    | a + NaN.i    | NaN + NaN.i, for finite a                       |
*    | +Inf + b.i   | +1 + sin(2b) x 0.i for positive-signed finite b   |
*    | +Inf + Inf.i | +1 + +/-0.i, sign of imaginary part unspecified |
*    | +Inf + NaN.i | +1 + +/-0.i, sign of imaginary part unspecified |
*    | NaN + 0.i    | NaN + 0.i                                       |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                  |
*    | NaN + NaN.i  | NaN + NaN.i                                     |
*    +--------------+-------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * ctanh(conj(z)) = conj(ctanh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * ctanh(-z) = -ctanh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API ctanh(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_ctanh_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       ctanhl()
*
*  Function description
*    Compute hyperbolic tangent, long double complex.
*
*  Parameters
*    x - Value to compute hyperbolic tangent of.
*
*  Return value
*    The hyperbolic tangent of x according to the following table:
*
*    +--------------+-------------------------------------------------+
*    | Argument     | ctanh(Argument)                                 |
*    +--------------+-------------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                        |
*    | a + Inf.i    | NaN + NaN.i, for finite a                       |
*    | a + NaN.i    | NaN + NaN.i, for finite a                       |
*    | +Inf + b.i   | +1 + sin(2b) x 0.i for positive-signed finite b   |
*    | +Inf + Inf.i | +1 + +/-0.i, sign of imaginary part unspecified |
*    | +Inf + NaN.i | +1 + +/-0.i, sign of imaginary part unspecified |
*    | NaN + 0.i    | NaN + 0.i                                       |
*    | NaN + b.i    | NaN + NaN.i, for all nonzero b                  |
*    | NaN + NaN.i  | NaN + NaN.i                                     |
*    +--------------+-------------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * ctanh(conj(z)) = conj(ctanh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * ctanh(-z) = -ctanh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API ctanhl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(ctanh(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       cacosf()
*
*  Function description
*    Compute inverse cosine, float complex.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Return value
*    The inverse cosine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API cacosf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_cacos(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       cacos()
*
*  Function description
*    Compute inverse cosine, double complex.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Return value
*    The inverse cosine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API cacos(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_cacos(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       cacosl()
*
*  Function description
*    Compute inverse cosine, long double complex.
*
*  Parameters
*    x - Value to compute inverse cosine of.
*
*  Return value
*    The inverse cosine of x.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API cacosl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(cacos(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       casinf()
*
*  Function description
*    Compute inverse sine, float complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Inverse sine of x.
*
*  Notes
*    casin(z) = -i casinh(i.z)
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API casinf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_casin(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       casin()
*
*  Function description
*    Compute inverse sine, double complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Inverse sine of x.
*
*  Notes
*    casin(z) = -i casinh(i.z)
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API casin(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_casin(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       casinl()
*
*  Function description
*    Compute inverse sine, long double complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Inverse sine of x.
*
*  Notes
*    casinl(z) = -i casinhl(i.z)
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API casinl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(casin(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       catanf()
*
*  Function description
*    Compute inverse tangent, float complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Inverse tangent of x.
*
*  Notes
*    catan(z) = -i catanh(i.z)
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API catanf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_catan(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       catan()
*
*  Function description
*    Compute inverse tangent, double complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Inverse tangent of x.
*
*  Notes
*    catan(z) = -i catanh(i.z)
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API catan(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_catan(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       catanl()
*
*  Function description
*    Compute inverse tangent, long double complex.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Inverse tangent of x.
*
*  Notes
*    catanl(z) = -i catanhl(i.z)
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API catanl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(catan(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       casinhf()
*
*  Function description
*    Compute inverse hyperbolic sine, float  complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sineof.
*
*  Return value
*    The inverse hyperbolic sine of x according to the following table:
*
*    +--------------+-----------------------------------------------+
*    | Argument     | casinh(Argument)                              |
*    +--------------+-----------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                      |
*    | +0 + Inf.i   | +Inf + Pi/2.i                                 |
*    | a + NaN.i    | NaN + NaN.i                                   |
*    | +Inf + b.i   | +Inf + 0.i, for positive-signed b             |
*    | +Inf + Inf.i | +Pi + 0.i                                     |
*    | +Inf + NaN.i | +Inf + NaN.i                                  |
*    | NaN + 0.i    | NaN + 0.i                                     |
*    | NaN + b.i    | NaN + NaN.i, for finite nonzero b             |
*    | NaN + Inf.i  | +/-Inf + NaN.i, sign of real part unspecified |
*    | NaN + NaN.i  | NaN + NaN.i                                   |
*    +--------------+-----------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * casinh(conj(z)) = conj(casinh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * casinh(-z) = -casinh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API casinhf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_casinh(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       casinh()
*
*  Function description
*    Compute inverse hyperbolic sine, double complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sineof.
*
*  Return value
*    The inverse hyperbolic sine of x according to the following table:
*
*    +--------------+-----------------------------------------------+
*    | Argument     | casinh(Argument)                              |
*    +--------------+-----------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                      |
*    | +0 + Inf.i   | +Inf + Pi/2.i                                 |
*    | a + NaN.i    | NaN + NaN.i                                   |
*    | +Inf + b.i   | +Inf + 0.i, for positive-signed b             |
*    | +Inf + Inf.i | +Pi + 0.i                                     |
*    | +Inf + NaN.i | +Inf + NaN.i                                  |
*    | NaN + 0.i    | NaN + 0.i                                     |
*    | NaN + b.i    | NaN + NaN.i, for finite nonzero b             |
*    | NaN + Inf.i  | +/-Inf + NaN.i, sign of real part unspecified |
*    | NaN + NaN.i  | NaN + NaN.i                                   |
*    +--------------+-----------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * casinh(conj(z)) = conj(casinh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * casinh(-z) = -casinh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API casinh(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_casinh(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       casinhl()
*
*  Function description
*    Compute inverse hyperbolic sine, long double complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic sineof.
*
*  Return value
*    The inverse hyperbolic sine of x according to the following table:
*
*    +--------------+-----------------------------------------------+
*    | Argument     | casinh(Argument)                              |
*    +--------------+-----------------------------------------------+
*    | +0 + 0.i     | +0 + 0.i                                      |
*    | +0 + Inf.i   | +Inf + Pi/2.i                                 |
*    | a + NaN.i    | NaN + NaN.i                                   |
*    | +Inf + b.i   | +Inf + 0.i, for positive-signed b             |
*    | +Inf + Inf.i | +Pi + 0.i                                     |
*    | +Inf + NaN.i | +Inf + NaN.i                                  |
*    | NaN + 0.i    | NaN + 0.i                                     |
*    | NaN + b.i    | NaN + NaN.i, for finite nonzero b             |
*    | NaN + Inf.i  | +/-Inf + NaN.i, sign of real part unspecified |
*    | NaN + NaN.i  | NaN + NaN.i                                   |
*    +--------------+-----------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * casinh(conj(z)) = conj(casinh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * casinh(-z) = -casinh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API casinhl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(casinh(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       cacoshf()
*
*  Function description
*    Compute inverse hyperbolic cosine, float complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Return value
*    The inverse hyperbolic cosine of x according to the following table:
*
*    +----------------+-------------------------------------------+
*    | Argument       | cacosh(Argument)                          |
*    +----------------+-------------------------------------------+
*    | +/-0 + 0.i     | +0 + 0.i                                  |
*    | a + Inf.i      | +Inf + Pi/2.i, for finite a               |
*    | a + NaN.i      | NaN + NaN.i, for finite a                 |
*    | -Inf + b.i     | +Inf + Pi.i, for positive-signed finite b |
*    | +Inf + b.i     | +Inf + 0.i, for positive-signed finite b  | 
*    | -Inf + Inf.i   | +/-Inf + 3Pi/4.i                          |
*    | +Inf + Inf.i   | +/-Inf + Pi/4.i                           |
*    | +/-Inf + NaN.i | +Inf + NaN.i                              |
*    | NaN + b.i      | NaN + NaN.i, for finite b                 |
*    | NaN + Inf.i    | +Inf + NaN.i                              |
*    | NaN + NaN.i    | NaN + NaN.i                               |
*    +----------------+-------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * cacosh(conj(z)) = conj(cacosh(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API cacoshf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_cacosh_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       cacosh()
*
*  Function description
*    Compute inverse hyperbolic cosine, double complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Return value
*    The inverse hyperbolic cosine of x according to the following table:
*
*    +----------------+-------------------------------------------+
*    | Argument       | cacosh(Argument)                          |
*    +----------------+-------------------------------------------+
*    | +/-0 + 0.i     | +0 + 0.i                                  |
*    | a + Inf.i      | +Inf + Pi/2.i, for finite a               |
*    | a + NaN.i      | NaN + NaN.i, for finite a                 |
*    | -Inf + b.i     | +Inf + Pi.i, for positive-signed finite b |
*    | +Inf + b.i     | +Inf + 0.i, for positive-signed finite b  | 
*    | -Inf + Inf.i   | +/-Inf + 3Pi/4.i                          |
*    | +Inf + Inf.i   | +/-Inf + Pi/4.i                           |
*    | +/-Inf + NaN.i | +Inf + NaN.i                              |
*    | NaN + b.i      | NaN + NaN.i, for finite b                 |
*    | NaN + Inf.i    | +Inf + NaN.i                              |
*    | NaN + NaN.i    | NaN + NaN.i                               |
*    +----------------+-------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * cacosh(conj(z)) = conj(cacosh(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API cacosh(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_cacosh_inline(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       cacoshl()
*
*  Function description
*    Compute inverse hyperbolic cosine, long double complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic cosine of.
*
*  Return value
*    The inverse hyperbolic cosine of x according to the following table:
*
*    +----------------+-------------------------------------------+
*    | Argument       | cacosh(Argument)                          |
*    +----------------+-------------------------------------------+
*    | +/-0 + 0.i     | +0 + 0.i                                  |
*    | a + Inf.i      | +Inf + Pi/2.i, for finite a               |
*    | a + NaN.i      | NaN + NaN.i, for finite a                 |
*    | -Inf + b.i     | +Inf + Pi.i, for positive-signed finite b |
*    | +Inf + b.i     | +Inf + 0.i, for positive-signed finite b  | 
*    | -Inf + Inf.i   | +/-Inf + 3Pi/4.i                          |
*    | +Inf + Inf.i   | +/-Inf + Pi/4.i                           |
*    | +/-Inf + NaN.i | +Inf + NaN.i                              |
*    | NaN + b.i      | NaN + NaN.i, for finite b                 |
*    | NaN + Inf.i    | +Inf + NaN.i                              |
*    | NaN + NaN.i    | NaN + NaN.i                               |
*    +----------------+-------------------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * cacosh(conj(z)) = conj(cacosh(z)).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API cacoshl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(cacosh(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

/*********************************************************************
*
*       catanhf()
*
*  Function description
*    Compute inverse hyperbolic tangent, float complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Return value
*    The inverse hyperbolic tangent of x according to the following table:
*
*    +--------------+-----------------------------------+
*    | Argument     | catanh(Argument)                  |
*    +--------------+-----------------------------------+
*    | +0 + 0.i     | +0 + 0.i                          |
*    | +0 + NaN.i   | +0 + NaN.i                        |
*    | +1 + 0.i     | +Inf + 0.i                        |
*    | a + Inf.i    | +0 + Pi/2.i for positive-signed a |
*    | a + NaN.i    | NaN + NaN.i, for nonzero finite a |
*    | +Inf + b.i   | +0 + Pi/2.i for positive-signed b |
*    | +Inf + Inf.i | +0 + Pi/2.i                       |
*    | +Inf + NaN.i | +0 + NaN.i                        |
*    | NaN + b.i    | NaN + NaN.i, for finite b         |
*    | NaN + NaN.i  | NaN + NaN.i                       |
*    +--------------+-----------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * catanh(conj(z)) = conj(catanh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * catanh(-z) = -catanh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT32_C_COMPLEX __SEGGER_RTL_PUBLIC_API catanhf(__SEGGER_RTL_FLOAT32_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT32_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float32_catanh(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       catanh()
*
*  Function description
*    Compute inverse hyperbolic tangent, double complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Return value
*    The inverse hyperbolic tangent of x according to the following table:
*
*    +--------------+-----------------------------------+
*    | Argument     | catanh(Argument)                  |
*    +--------------+-----------------------------------+
*    | +0 + 0.i     | +0 + 0.i                          |
*    | +0 + NaN.i   | +0 + NaN.i                        |
*    | +1 + 0.i     | +Inf + 0.i                        |
*    | a + Inf.i    | +0 + Pi/2.i for positive-signed a |
*    | a + NaN.i    | NaN + NaN.i, for nonzero finite a |
*    | +Inf + b.i   | +0 + Pi/2.i for positive-signed b |
*    | +Inf + Inf.i | +0 + Pi/2.i                       |
*    | +Inf + NaN.i | +0 + NaN.i                        |
*    | NaN + b.i    | NaN + NaN.i, for finite b         |
*    | NaN + NaN.i  | NaN + NaN.i                       |
*    +--------------+-----------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * catanh(conj(z)) = conj(catanh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * catanh(-z) = -catanh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_FLOAT64_C_COMPLEX __SEGGER_RTL_PUBLIC_API catanh(__SEGGER_RTL_FLOAT64_C_COMPLEX x) {
  __SEGGER_RTL_FLOAT64_COMPLEX z;
  //
  z.u.value = x;
  __SEGGER_RTL_float64_catanh(&z);
  //
  return z.u.value;
}

/*********************************************************************
*
*       catanhl()
*
*  Function description
*    Compute inverse hyperbolic tangent, long double complex.
*
*  Parameters
*    x - Value to compute inverse hyperbolic tangent of.
*
*  Return value
*    The inverse hyperbolic tangent of x according to the following table:
*
*    +--------------+-----------------------------------+
*    | Argument     | catanh(Argument)                  |
*    +--------------+-----------------------------------+
*    | +0 + 0.i     | +0 + 0.i                          |
*    | +0 + NaN.i   | +0 + NaN.i                        |
*    | +1 + 0.i     | +Inf + 0.i                        |
*    | a + Inf.i    | +0 + Pi/2.i for positive-signed a |
*    | a + NaN.i    | NaN + NaN.i, for nonzero finite a |
*    | +Inf + b.i   | +0 + Pi/2.i for positive-signed b |
*    | +Inf + Inf.i | +0 + Pi/2.i                       |
*    | +Inf + NaN.i | +0 + NaN.i                        |
*    | NaN + b.i    | NaN + NaN.i, for finite b         |
*    | NaN + NaN.i  | NaN + NaN.i                       |
*    +--------------+-----------------------------------+
*
*    For arguments with a negative imaginary component, use the equality:
*
*    * catanh(conj(z)) = conj(catanh(z)).
*
*    For arguments with a negative real component, use the equality:
*
*    * catanh(-z) = -catanh(z).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_LDOUBLE_C_COMPLEX __SEGGER_RTL_PUBLIC_API catanhl(__SEGGER_RTL_LDOUBLE_C_COMPLEX x) {
  return __SEGGER_RTL_complex_double_to_ldouble(catanh(__SEGGER_RTL_complex_ldouble_to_double(x)));
}

#endif

/*************************** End of file ****************************/
