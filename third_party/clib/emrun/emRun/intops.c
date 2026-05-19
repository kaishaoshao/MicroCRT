/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

References:
    [WARREN]  - Hacker's Delight, Henry Warren.
    [MOELLER] - Improved division by invariant integers, Niels Moeller & Torbjorn Granlund.
    [AOCP]    - Seminumerical algorithms, Donald Knuth.
    [TANG]    - An Overview of Floating-Point Support and Math Library on the Intel XScale Architecture, Ping Tak Peter Tang.

*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Int.h"
#include "__SEGGER_RTL_Inlines.h"
#include "stdlib.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define __SEGGER_RTL_CNEG_I32(X, Y)   (((X) ^ ((__SEGGER_RTL_I32)(Y) >> 31)) - ((__SEGGER_RTL_I32)(Y) >> 31))
#define __SEGGER_RTL_CNEG_I64(X, Y)   (((X) ^ ((__SEGGER_RTL_I64)(Y) >> 63)) - ((__SEGGER_RTL_I64)(Y) >> 63))
#define __SEGGER_RTL_CNEG_I128(X, Y)  (((X) ^ ((__SEGGER_RTL_I128)(Y) >> 127)) - ((__SEGGER_RTL_I128)(Y) >> 127))
#define __SEGGER_RTL_ABS_I32(X)       __SEGGER_RTL_CNEG_I32(X, X)
#define __SEGGER_RTL_ABS_I64(X)       __SEGGER_RTL_CNEG_I64(X, X)
#define __SEGGER_RTL_ABS_I128(X)      __SEGGER_RTL_CNEG_I128(X, X)

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

static __SEGGER_RTL_STATE_THREAD unsigned long __SEGGER_RTL_rand_next = 1;  // srand(1) is required if srand() not called.

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// 6-bit-in, 8-bit-out reciprocal LUT, 2^14/(01bbbbbb).
static const __SEGGER_RTL_U8 __SEGGER_RTL_inverse_lut[] = {
  0xFC, 0xF8, 0xF4, 0xF0, 0xED, 0xEA, 0xE6, 0xE3,
  0xE0, 0xDD, 0xDA, 0xD7, 0xD4, 0xD2, 0xCF, 0xCC,
  0xCA, 0xC7, 0xC5, 0xC3, 0xC0, 0xBE, 0xBC, 0xBA,
  0xB8, 0xB6, 0xB4, 0xB2, 0xB0, 0xAE, 0xAC, 0xAA,
  0xA8, 0xA7, 0xA5, 0xA3, 0xA2, 0xA0, 0x9F, 0x9D,
  0x9C, 0x9A, 0x99, 0x97, 0x96, 0x94, 0x93, 0x92,
  0x90, 0x8F, 0x8E, 0x8D, 0x8C, 0x8A, 0x89, 0x88,
  0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80,
};

// Generate table using following code:
//
//  for (unsigned r = 512; r < 1024; ++r) {
//    printf("0x%04X, ", ((1uL << 24) - (1uL << 14) + (1uL << 9)) / r);
//    if ((r & 7) == 7) {
//      printf("\n");
//    }
//  }
//
static const __SEGGER_RTL_U16 __SEGGER_RTL_Moeller_inverse_lut[] = {
  0x7FE1, 0x7FA1, 0x7F61, 0x7F22, 0x7EE3, 0x7EA4, 0x7E65, 0x7E27,
  0x7DE9, 0x7DAB, 0x7D6D, 0x7D30, 0x7CF3, 0x7CB6, 0x7C79, 0x7C3D,
  0x7C00, 0x7BC4, 0x7B89, 0x7B4D, 0x7B12, 0x7AD7, 0x7A9C, 0x7A61,
  0x7A27, 0x79EC, 0x79B2, 0x7979, 0x793F, 0x7906, 0x78CC, 0x7894,
  0x785B, 0x7822, 0x77EA, 0x77B2, 0x777A, 0x7742, 0x770B, 0x76D3,
  0x769C, 0x7665, 0x762F, 0x75F8, 0x75C2, 0x758C, 0x7556, 0x7520,
  0x74EA, 0x74B5, 0x7480, 0x744B, 0x7416, 0x73E2, 0x73AD, 0x7379,
  0x7345, 0x7311, 0x72DD, 0x72AA, 0x7277, 0x7243, 0x7210, 0x71DE,
  0x71AB, 0x7179, 0x7146, 0x7114, 0x70E2, 0x70B1, 0x707F, 0x704E,
  0x701C, 0x6FEB, 0x6FBA, 0x6F8A, 0x6F59, 0x6F29, 0x6EF9, 0x6EC8,
  0x6E99, 0x6E69, 0x6E39, 0x6E0A, 0x6DDB, 0x6DAB, 0x6D7D, 0x6D4E,
  0x6D1F, 0x6CF1, 0x6CC2, 0x6C94, 0x6C66, 0x6C38, 0x6C0A, 0x6BDD,
  0x6BB0, 0x6B82, 0x6B55, 0x6B28, 0x6AFB, 0x6ACF, 0x6AA2, 0x6A76,
  0x6A49, 0x6A1D, 0x69F1, 0x69C6, 0x699A, 0x696E, 0x6943, 0x6918,
  0x68ED, 0x68C2, 0x6897, 0x686C, 0x6842, 0x6817, 0x67ED, 0x67C3,
  0x6799, 0x676F, 0x6745, 0x671B, 0x66F2, 0x66C8, 0x669F, 0x6676,
  0x664D, 0x6624, 0x65FC, 0x65D3, 0x65AA, 0x6582, 0x655A, 0x6532,
  0x650A, 0x64E2, 0x64BA, 0x6493, 0x646B, 0x6444, 0x641C, 0x63F5,
  0x63CE, 0x63A7, 0x6381, 0x635A, 0x6333, 0x630D, 0x62E7, 0x62C1,
  0x629A, 0x6275, 0x624F, 0x6229, 0x6203, 0x61DE, 0x61B8, 0x6193,
  0x616E, 0x6149, 0x6124, 0x60FF, 0x60DA, 0x60B6, 0x6091, 0x606D,
  0x6049, 0x6024, 0x6000, 0x5FDC, 0x5FB8, 0x5F95, 0x5F71, 0x5F4D,
  0x5F2A, 0x5F07, 0x5EE3, 0x5EC0, 0x5E9D, 0x5E7A, 0x5E57, 0x5E35,
  0x5E12, 0x5DEF, 0x5DCD, 0x5DAB, 0x5D88, 0x5D66, 0x5D44, 0x5D22,
  0x5D00, 0x5CDE, 0x5CBD, 0x5C9B, 0x5C7A, 0x5C58, 0x5C37, 0x5C16,
  0x5BF5, 0x5BD4, 0x5BB3, 0x5B92, 0x5B71, 0x5B51, 0x5B30, 0x5B10,
  0x5AEF, 0x5ACF, 0x5AAF, 0x5A8F, 0x5A6F, 0x5A4F, 0x5A2F, 0x5A0F,
  0x59EF, 0x59D0, 0x59B0, 0x5991, 0x5972, 0x5952, 0x5933, 0x5914,
  0x58F5, 0x58D6, 0x58B7, 0x5899, 0x587A, 0x585B, 0x583D, 0x581F,
  0x5800, 0x57E2, 0x57C4, 0x57A6, 0x5788, 0x576A, 0x574C, 0x572E,
  0x5711, 0x56F3, 0x56D5, 0x56B8, 0x569B, 0x567D, 0x5660, 0x5643,
  0x5626, 0x5609, 0x55EC, 0x55CF, 0x55B2, 0x5596, 0x5579, 0x555D,
  0x5540, 0x5524, 0x5507, 0x54EB, 0x54CF, 0x54B3, 0x5497, 0x547B,
  0x545F, 0x5443, 0x5428, 0x540C, 0x53F0, 0x53D5, 0x53B9, 0x539E,
  0x5383, 0x5368, 0x534C, 0x5331, 0x5316, 0x52FB, 0x52E0, 0x52C6,
  0x52AB, 0x5290, 0x5276, 0x525B, 0x5240, 0x5226, 0x520C, 0x51F1,
  0x51D7, 0x51BD, 0x51A3, 0x5189, 0x516F, 0x5155, 0x513B, 0x5121,
  0x5108, 0x50EE, 0x50D5, 0x50BB, 0x50A2, 0x5088, 0x506F, 0x5056,
  0x503C, 0x5023, 0x500A, 0x4FF1, 0x4FD8, 0x4FBF, 0x4FA6, 0x4F8E,
  0x4F75, 0x4F5C, 0x4F44, 0x4F2B, 0x4F13, 0x4EFA, 0x4EE2, 0x4ECA,
  0x4EB1, 0x4E99, 0x4E81, 0x4E69, 0x4E51, 0x4E39, 0x4E21, 0x4E09,
  0x4DF1, 0x4DDA, 0x4DC2, 0x4DAA, 0x4D93, 0x4D7B, 0x4D64, 0x4D4D,
  0x4D35, 0x4D1E, 0x4D07, 0x4CF0, 0x4CD8, 0x4CC1, 0x4CAA, 0x4C93,
  0x4C7D, 0x4C66, 0x4C4F, 0x4C38, 0x4C21, 0x4C0B, 0x4BF4, 0x4BDE,
  0x4BC7, 0x4BB1, 0x4B9A, 0x4B84, 0x4B6E, 0x4B58, 0x4B41, 0x4B2B,
  0x4B15, 0x4AFF, 0x4AE9, 0x4AD3, 0x4ABD, 0x4AA8, 0x4A92, 0x4A7C,
  0x4A66, 0x4A51, 0x4A3B, 0x4A26, 0x4A10, 0x49FB, 0x49E5, 0x49D0,
  0x49BB, 0x49A6, 0x4990, 0x497B, 0x4966, 0x4951, 0x493C, 0x4927,
  0x4912, 0x48FE, 0x48E9, 0x48D4, 0x48BF, 0x48AB, 0x4896, 0x4881,
  0x486D, 0x4858, 0x4844, 0x482F, 0x481B, 0x4807, 0x47F3, 0x47DE,
  0x47CA, 0x47B6, 0x47A2, 0x478E, 0x477A, 0x4766, 0x4752, 0x473E,
  0x472A, 0x4717, 0x4703, 0x46EF, 0x46DB, 0x46C8, 0x46B4, 0x46A1,
  0x468D, 0x467A, 0x4666, 0x4653, 0x4640, 0x462C, 0x4619, 0x4606,
  0x45F3, 0x45E0, 0x45CD, 0x45BA, 0x45A7, 0x4594, 0x4581, 0x456E,
  0x455B, 0x4548, 0x4536, 0x4523, 0x4510, 0x44FE, 0x44EB, 0x44D8,
  0x44C6, 0x44B3, 0x44A1, 0x448F, 0x447C, 0x446A, 0x4458, 0x4445,
  0x4433, 0x4421, 0x440F, 0x43FD, 0x43EB, 0x43D9, 0x43C7, 0x43B5,
  0x43A3, 0x4391, 0x437F, 0x436D, 0x435C, 0x434A, 0x4338, 0x4327,
  0x4315, 0x4303, 0x42F2, 0x42E0, 0x42CF, 0x42BD, 0x42AC, 0x429B,
  0x4289, 0x4278, 0x4267, 0x4256, 0x4244, 0x4233, 0x4222, 0x4211,
  0x4200, 0x41EF, 0x41DE, 0x41CD, 0x41BC, 0x41AB, 0x419A, 0x418A,
  0x4179, 0x4168, 0x4157, 0x4147, 0x4136, 0x4125, 0x4115, 0x4104,
  0x40F4, 0x40E3, 0x40D3, 0x40C2, 0x40B2, 0x40A2, 0x4091, 0x4081,
  0x4071, 0x4061, 0x4050, 0x4040, 0x4030, 0x4020, 0x4010, 0x4000,
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_PCT_U32_inline()
*
*  Function description
*    Population count, 32-bit integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Count of number of one bits in x.
*
*  Reference
*    Hacker's Delight, section 5.1.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_PCT_U32_inline(__SEGGER_RTL_U32 x) {
  x = x - ((x >> 1) & __SEGGER_RTL_U32_C(0x55555555));
  x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
  x = (x + (x >> 4)) & 0x0F0F0F0F;
  x = (x + (x >> 16));
  return (x + (x >> 8)) & 0x3F;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_SmallDivisor()
*
*  Function description
*    Calculate quotient with simple algorithms.
*
*  Parameters
*    u - Dividend.
*    v - Divisor, 0 <= v <= 4.
*
*  Return value
*     floor(u / v)
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32_SmallDivisor(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  switch (v) {
  default: return 0;
  case 1:  return u;
  case 2:  return u >> 1;
  case 3:  return __SEGGER_RTL_UMULL_HI(u, 0xAAAAAAABu) >> 1;
  case 4:  return u >> 2;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_IsSmallQuotient()
*
*  Function description
*    Can quotient be calculated using truncated division by clockwork?
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    == 0 - No
*    != 0 - Yes.
*
*  Additional information
*    This will answer whether the quotient fits into eight bits.
*/
static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_Div_U32_IsSmallQuotient(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  return v > (u >> 7);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_SmallQuotient()
*
*  Function description
*    Calculate quotient using truncated division by clockwork.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    floor(u / v)
*
*  Additional information
*    Use __SEGGER_RTL_Div_U32_IsSmallQuotient() to test applicability
*    before calling this function.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32_SmallQuotient(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  __SEGGER_RTL_U32 q;
  //
  q = 0;
  //
  // Quickly cull larger quotient calculations.
  //
  if (v <= u >> 2) {
    if (v <= u >> 7) { u -= v << 7; q += 1u << 7; }
    if (v <= u >> 6) { u -= v << 6; q += 1u << 6; }
    if (v <= u >> 5) { u -= v << 5; q += 1u << 5; }
    if (v <= u >> 4) { u -= v << 4; q += 1u << 4; }
    if (v <= u >> 3) { u -= v << 3; q += 1u << 3; }
    if (v <= u >> 2) { u -= v << 2; q += 1u << 2; }
  }
  if (v <= u >> 1)   { u -= v << 1; q += 1u << 1; }
  if (v <= u >> 0)   {              q += 1u << 0; }
  //
  return q;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Tang()
*
*  Function description
*    Calculate quotient using Tang algorithm.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v)
*
*  Additional information
*    See [TANG].
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32_Tang(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  __SEGGER_RTL_U32 q;
  __SEGGER_RTL_U32 r;
  __SEGGER_RTL_U32 b;
  __SEGGER_RTL_U64 a;
  int              n;
  //
  // Look up reciprocal estimate.
  //
  n = __SEGGER_RTL_CLZ_U32(v);
  b = __SEGGER_RTL_inverse_lut[(v << n >> 25) - 64] << (n - 7);
  //
  // R = 2^32 - B*B[r] = -(B*B[r]) with B[r] from LUT.
  //
  r = 0u - (v * b);
  //
  // A[j]h = HIGH (R*A[j-1]h + A[j-1]l)
  // A[j]l = LOW  (R*A[j-1]h + A[j-1]l)  ... five times
  //
  a = __SEGGER_RTL_UMULL_X(b, u);                                             q  = __SEGGER_RTL_U64_H(a);
  a = __SEGGER_RTL_UMULL_X(r, __SEGGER_RTL_U64_H(a)) + __SEGGER_RTL_U64_L(a); q += __SEGGER_RTL_U64_H(a);
  a = __SEGGER_RTL_UMULL_X(r, __SEGGER_RTL_U64_H(a)) + __SEGGER_RTL_U64_L(a); q += __SEGGER_RTL_U64_H(a);
  a = __SEGGER_RTL_UMULL_X(r, __SEGGER_RTL_U64_H(a)) + __SEGGER_RTL_U64_L(a); q += __SEGGER_RTL_U64_H(a);
  a = __SEGGER_RTL_UMULL_X(r, __SEGGER_RTL_U64_H(a)) + __SEGGER_RTL_U64_L(a); q += __SEGGER_RTL_U64_H(a);
  a = __SEGGER_RTL_UMULL_X(r, __SEGGER_RTL_U64_H(a)) + __SEGGER_RTL_U64_L(a); q += __SEGGER_RTL_U64_H(a);
  q += __SEGGER_RTL_U64_H(__SEGGER_RTL_U64_L(a) + r);
  //
  return q;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Tang_Special()
*
*  Function description
*    Calculate quotient using Tang, with special cases.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v)
*
*  Additional information
*    See [MOELLER].
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32_Tang_Special(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  if (v <= 4) {
    return __SEGGER_RTL_Div_U32_SmallDivisor(u, v);
  } else if (__SEGGER_RTL_Div_U32_IsSmallQuotient(u, v)) {
    return __SEGGER_RTL_Div_U32_SmallQuotient(u, v);
  } else {
    return __SEGGER_RTL_Div_U32_Tang(u, v);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_CalcReciprocal()
*
*  Function description
*    Calculate reciprocal word for Moeller-Grunland division.
*
*  Parameters
*    d - Most-significant 32 bits of divisor.
*
*  Return value
*     (2^32-1) / d.
*
*  Additional information
*    See [MOELLER].
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32_CalcReciprocal(__SEGGER_RTL_U32 d) {
  __SEGGER_RTL_U32 d0, d10, d21, d31;
  __SEGGER_RTL_U32 v0, v1, v2;
  __SEGGER_RTL_U32 p0, p1;
  __SEGGER_RTL_U32 e;
  //
  d0  = d & 1;                                                 // d0  := d mod 2
  d10 = d >> 22;                                               // d10 := floor(2^-22.d)
  d21 = (d >> 11) + 1;                                         // d21 := floor(s^-11.d) + 1
  d31 = (d >> 1) + d0;                                         // d31 := ceil(d/2)
#if ((defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)) && (__SEGGER_RTL_OPTIMIZE < 0)
  (void)__SEGGER_RTL_Moeller_inverse_lut;
  v0  = ((1uL << 24) - (1uL << 14) + (1uL << 9)) / d10;        // v0  := floor((2^24-2^14+2^9) / d10), dynamic calculation
#else
  v0 = __SEGGER_RTL_Moeller_inverse_lut[d10 - 0x200];          // v0  := floor((2^24-2^14+2^9) / d10), static calculation
#endif
  v1  = (v0 << 4) - __SEGGER_RTL_UMULL_HI(v0 * v0, d21) - 1;   // v1  := 2^4.v0 - floor(2^-32.v0^2.d21)
  e   = (d0 ? v1>>1 : 0) - (v1 * d31);                         // e   := 2^48 - v1.d31 + floor(v1/2).d0
  v2  = (v1 << 15) + (__SEGGER_RTL_UMULL_HI(v1, e) >> 1);      // v2  := 2^15.v1 + floor(2^33.v1.e)
  p0 = d; p1 = d;                                              // v3  := (v2 - floor(2^-32(v2 + 2^32 + 1)d)) mod 2^32
  __SEGGER_RTL_UMLAL(p0, p1, v2, d);
  return v2 - p1;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_U32_Moeller()
*
*  Function description
*    64-bit by 32-bit division.
*
*  Parameters
*    u - Dividend.
*    d - Divisor, normalized.
*
*  Return value
*     floor(u / d).
*
*  Additional information
*    See [MOELLER] Algorithm 4.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U64_U32_Moeller(__SEGGER_RTL_U64 u, __SEGGER_RTL_U32 d) {
  __SEGGER_RTL_U32 u1, u0;  // High and low parts of dividend
  __SEGGER_RTL_U32 q1, q0;  // High and low parts of quotient
  __SEGGER_RTL_U32 r;       // Remainder
  __SEGGER_RTL_U32 v;       // Reciprocal of d
  __SEGGER_RTL_U64 q;       // Quotient estimate
  //
  u1 = __SEGGER_RTL_U64_H(u);
  u0 = __SEGGER_RTL_U64_L(u);
  //
  v  = __SEGGER_RTL_Div_U32_CalcReciprocal(d);
  q  = __SEGGER_RTL_UMULL_X(v, u1) + u;
  q1 = __SEGGER_RTL_U64_H(q) + 1;
  q0 = __SEGGER_RTL_U64_L(q);
  //
  r  = u0 - q1 * d;
  if (r > q0) {
    q1 -= 1;
    r  += d;
  }
  if (r >= d) {
    q1 += 1;
  }
  return q1;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Moeller()
*
*  Function description
*    Calculate quotient using Moeller-Grunland algorithm.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v)
*
*  Additional information
*    See [MOELLER].
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32_Moeller(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  unsigned n;
  //
  n = __SEGGER_RTL_CLZ_U32(v);
  return __SEGGER_RTL_Div_U64_U32_Moeller((__SEGGER_RTL_U64)u << n, v << n);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Moeller_Special()
*
*  Function description
*    Calculate quotient using Moeller-Grunland algorithm, with special cases.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v)
*
*  Additional information
*    See [MOELLER].
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32_Moeller_Special(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  if (v <= 4) {
    return __SEGGER_RTL_Div_U32_SmallDivisor(u, v);
  } else if (__SEGGER_RTL_Div_U32_IsSmallQuotient(u, v)) {
    return __SEGGER_RTL_Div_U32_SmallQuotient(u, v);
  } else {
    return __SEGGER_RTL_Div_U32_Moeller(u, v);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Clockwork()
*
*  Function description
*    Calculate quotient using division by clockwork.
*
*  Parameters
*    u   - Dividend.
*    v   - Divisor.
*    rem - Pointer to object that receives the remainder.
*
*  Return value
*    floor(u / v)
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32_Clockwork(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v, __SEGGER_RTL_U32 *rem) {
  __SEGGER_RTL_U32 quot;
  //
  if (v == 0) {
    u    = 0;
    quot = 0;
  } else {
    __SEGGER_RTL_U32 qbit;
    //
    quot = 0;
    qbit = 1;
    //
#if !defined(__SEGGER_RTL_CLZ_U32_SYNTHESIZED)
    {
      unsigned n;
      n      = __SEGGER_RTL_CLZ_U32(v);
      v    <<= n;
      qbit <<= n;
    }
#else
    while ((v & __SEGGER_RTL_U32_C(0x80000000)) == 0) {
      v    <<= 1;
      qbit <<= 1;
    }
#endif
    //
    while (qbit) {
      if (v <= u) {
        u    -= v;
        quot += qbit;
      }
      v    >>= 1;
      qbit >>= 1;
    }
  }
  //
  if (rem) {
    *rem = u;
  }
  //
  return quot;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_3()
*
*  Function description
*    Divide 64-bit unsigned by 3.
*
*  Parameters
*    u - Dividend.
*
*  Return value
*     floor(u / 3)
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div_U64_3(__SEGGER_RTL_U64 u) {
  //
#if defined(__SEGGER_RTL_U128)
  //
  // Use 128-bit arithmetic with multiply-high magic.
  //
  return (__SEGGER_RTL_U64)((__SEGGER_RTL_U128)u * __SEGGER_RTL_U64_C(0xAAAAAAAAAAAAAAAB) >> 65);  // 1/3
  //
#else
  //
  // Simulate with 64-bit arithmetic.
  //
  __SEGGER_RTL_U32 u0l, u0h, u1h, u1l;
  __SEGGER_RTL_U64 q;
  //
  __SEGGER_RTL_UMULL(u0l, u0h, 0x55555555, __SEGGER_RTL_U64_L(u));
  __SEGGER_RTL_UMULL(u1l, u1h, 0x55555555, __SEGGER_RTL_U64_H(u));
  //
  u0h += u1l;
  u1h += u0h < u1l;
  q    = ((__SEGGER_RTL_U64)u1h << 32) + u0h;
  u0l += u0h; u1h += u0l < u0h; q += u1h;
  u0l += u1h; q   += u0l < u1h;
  u0l += 1;   q   += u0l == 0;
  //
  return q;
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_TinyDivisor()
*
*  Function description
*    Calculate quotient with simple algorithms.
*
*  Parameters
*    u - Dividend.
*    v - Divisor, 0 <= v <= 4.
*
*  Return value
*     floor(u / v)
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div_U64_TinyDivisor(__SEGGER_RTL_U64 u, __SEGGER_RTL_U32 v) {
  //
  switch (v) {
  default: return 0;
  case 1:  return u;
  case 2:  return u >> 1;
  case 3:  return __SEGGER_RTL_Div_U64_3(u);
  case 4:  return u >> 2;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_Clockwork()
*
*  Function description
*    Calculate quotient using division by clockwork.
*
*  Parameters
*    u   - Dividend.
*    v   - Divisor.
*    rem - Pointer to object that receives the remainder.
*
*  Return value
*    floor(u / v)
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div_U64_Clockwork(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v, __SEGGER_RTL_U64 *rem) {
  __SEGGER_RTL_U64 quot;
  __SEGGER_RTL_U64 qbit;
  //
  if (v == 0) {
    u    = 0;
    quot = 0;
  } else {
    //
    quot = 0;
    qbit = 1;
    //
#if !defined(__SEGGER_RTL_CLZ_U64_SYNTHESIZED)
    {
      unsigned n;
      n      = __SEGGER_RTL_CLZ_U64(v);
      v    <<= n;
      qbit <<= n;
    }
#else
    if (v <= __SEGGER_RTL_U64_C(0xFFFFFFFF)) {
      v    <<= 32;
      qbit <<= 32;
    }
    while ((v & __SEGGER_RTL_U64_C(0x8000000000000000)) == 0) {
      v    <<= 1;
      qbit <<= 1;
    }
#endif
    //
    while (qbit) {
      if (v <= u) {
        u    -= v;
        quot += qbit;
      }
      v    >>= 1;
      qbit >>= 1;
    }
  }
  //
  if (rem) {
    *rem = u;
  }
  //
  return quot;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32()
*
*  Function description
*    Divide, unsigned 32-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    floor(u / v).
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Div_U32(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  //
#if (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
  //
  return u / v;
  //
#elif __SEGGER_RTL_OPTIMIZE >= 0 && defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  //
  return __SEGGER_RTL_Div_U32_Moeller_Special(u, v);
  //
#else
  //
  return __SEGGER_RTL_Div_U32_Clockwork(u, v, NULL);
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_Mod_U32()
*
*  Function description
*    Modulus, unsigned 32-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    u % v.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Mod_U32(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  //
#if (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
  //
  return u % v;
  //
#elif __SEGGER_RTL_OPTIMIZE >= 0 && defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  //
  __SEGGER_RTL_U32 q;
  //
  q = __SEGGER_RTL_Div_U32_Moeller_Special(u, v);
  return u - q*v;
  //
#else
  //
  __SEGGER_RTL_U32 rem;
  //
  __SEGGER_RTL_Div_U32_Clockwork(u, v, &rem);
  return rem;
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_U16()
*
*  Function description
*    Divide, unsigned 64-bit integer by 16-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    floor(u / v).
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div_U64_U16(__SEGGER_RTL_U64 u, __SEGGER_RTL_U32 v) {
  __SEGGER_RTL_U32 q0, q1, q2;
  __SEGGER_RTL_U32 u1, u0;
  __SEGGER_RTL_U32 r;
  //
  // Extract high and low parts of dividend.
  //
  u1 = __SEGGER_RTL_U64_H(u);
  u0 = __SEGGER_RTL_U64_L(u);
  //
  // Schoolbook division.
  //
  r  = u1;
  __SEGGER_RTL_DIVMOD_U32(q2, r, r, v);         // Generate first quotient digit, 16 bits and remainder, 16 bits.
  r  = (r << 16) + (u0 >> 16);                  // Shift and bring in next dividend digit.
  __SEGGER_RTL_DIVMOD_U32(q1, r, r, v);         // Generate second quotient digit, 16 bits and remainder, 16 bits.
  r  = (r << 16) + (u0 & 0xFFFFu);              // Shift and bring in next dividend digit.
  q0 = r / v;                                   // Generate third quotient digit, 16 bits.
  //
  // Combine quotient digits.
  //
  return q2 * __SEGGER_RTL_U64_C(0x100000000) +
         q1 * 0x10000u +
         q0;
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_U24()
*
*  Function description
*    Divide, unsigned 64-bit integer by 24-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    floor(u / v).
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div_U64_U24(__SEGGER_RTL_U64 u, __SEGGER_RTL_U32 v) {
  __SEGGER_RTL_U32 q0, q1, q2, q3, q4;
  __SEGGER_RTL_U32 u0, u1;
  __SEGGER_RTL_U32 r;
  //
  // Extract high and low parts of dividend.
  //
  u1 = __SEGGER_RTL_U64_H(u);
  u0 = __SEGGER_RTL_U64_L(u);
  //
  // Schoolbook division.
  //
  r  = u1;
  __SEGGER_RTL_DIVMOD_U32(q4, r, r, v);         // Generate first quotient digit, 8 bits and remainder, 24 bits.
  r  = (r << 8) + (u0 >> 24);                   // Shift and bring in next dividend digit.
  __SEGGER_RTL_DIVMOD_U32(q3, r, r, v);         // Generate second quotient digit, 8 bits and remainder, 24 bits.
  r  = (r << 8) + ((u0 >> 16) & 0xFFu);         // Shift and bring in next dividend digit.
  __SEGGER_RTL_DIVMOD_U32(q2, r, r, v);         // Generate third quotient digit, 8 bits and remainder, 24 bits.
  r  = (r << 8) + ((u0 >> 8) & 0xFFu);          // Shift and bring in next dividend digit.
  __SEGGER_RTL_DIVMOD_U32(q1, r, r, v);         // Generate fourth quotient digit, 8 bits and remainder, 24 bits.
  r  = (r << 8) + (u0 & 0xFFu);                 // Shift and bring in next dividend digit.
  q0 = r / v;                                   // Generate fifth quotient digit, 8 bits.
  //
  // Combine quotient digits.
  //
  return __SEGGER_RTL_U64_MK(q4, q3 * 0x1000000u +
                                 q2 *   0x10000u +
                                 q1 *     0x100u +
                                 q0);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_Moeller()
*
*  Function description
*    64-bit by 64-bit division.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v).
*
*  Additional information
*    This is a modification of the algorithm presented by [WARREN],
*    which itself is a specialization of Algorithm D in [AOCP].
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div_U64_Moeller(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  __SEGGER_RTL_U32 q0, q1;
  __SEGGER_RTL_U32 u0, u1;
  __SEGGER_RTL_U32 v0;
  unsigned         n;
  //
  if (__SEGGER_RTL_U64_H(v) == 0) {
    //
    // v < 2**32.
    //
    if (__SEGGER_RTL_U64_H(u) == 0) {
      //
      // u < 2**32.
      //
      return __SEGGER_RTL_Div_U32(__SEGGER_RTL_U64_L(u), __SEGGER_RTL_U64_L(v));
      //
    } else if (__SEGGER_RTL_U64_H(u) < __SEGGER_RTL_U64_L(v)) {
      //
      // u/v cannot overflow, one division required.
      //
      v0 = __SEGGER_RTL_U64_L(v);
      n  = __SEGGER_RTL_CLZ_U32(v0);
      //
      return __SEGGER_RTL_Div_U64_U32_Moeller(u << n, v0 << n);
      //
    } else {
      //
      // If u/v would overflow, break into two halves.
      //
      v0 = __SEGGER_RTL_U64_L(v);
      u1 = __SEGGER_RTL_U64_H(u);
      u0 = __SEGGER_RTL_U64_L(u);
      //
      // Calculate quotient digits.
      //
      __SEGGER_RTL_DIVMOD_U32(q1, u1, u1, v0);  // same as q1 = u1 / v0; u1 = u1 - q1*v0; but avoids compiler using slow REMU on RISC-V
      n  = __SEGGER_RTL_CLZ_U32(v0);
      q0 = __SEGGER_RTL_Div_U64_U32_Moeller(__SEGGER_RTL_U64_MK(u1, u0) << n, v0 << n);
      //
      return __SEGGER_RTL_U64_MK(q1, q0);
    }
  } else {
    //
    // v >= 2**32.  Normalize divisor and ensure no overflow.
    //
    n  = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_U64_H(v));
    q0 = __SEGGER_RTL_Div_U64_U32_Moeller(u >> 1, __SEGGER_RTL_U64_H(v << n));
    //
    // Undo normalization and division of u by 2.
    //
    q0 >>= 31 - n;
    if (q0 != 0) {          // Make q0 correct or
      q0 -= 1;              // too small by 1.
    }
    //
    // Apply division theorem.
    //
    if ((u - q0*v) >= v) {
      q0 += 1;              // Now q0 is correct.
    }
    //
    return q0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_Moeller_Special()
*
*  Function description
*    64-bit by 64-bit division, special-cased.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v).
*
*  Additional information
*    Special cases for divison of smaller operands.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div_U64_Moeller_Special(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  //
  if (__SEGGER_RTL_U64_H(v) == 0) {
    //
    // Avoid recursion in required 32-bit division when the base integer
    // type is 64 bits wide by checking __SEGGER_RTL_TYPESET != 64.
    //
    if (__SEGGER_RTL_U64_H(u) == 0 && __SEGGER_RTL_TYPESET != 64) {
      //
      // Both operands less than 2^32, divide using division of 32-bit operands.
      //
      return __SEGGER_RTL_U64_L(u) / __SEGGER_RTL_U64_L(v);
      //
    } else if (v <= 0x00FFFFFF) {
      //
      // Divisor is small, dividend is big.
      //
      if (v <= 4) {
        return __SEGGER_RTL_Div_U64_TinyDivisor(u, __SEGGER_RTL_U64_L(v));
      } else if (v <= 0xFFFF) {
        return __SEGGER_RTL_Div_U64_U16(u, __SEGGER_RTL_U64_L(v));
      } else {
        return __SEGGER_RTL_Div_U64_U24(u, __SEGGER_RTL_U64_L(v));
      }
    }
  }
  //
  // Prefer Moeller algorithm for 32-bit processors optimizing for balanced and speed.
  //
  return __SEGGER_RTL_Div_U64_Moeller(u, v);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64()
*
*  Function description
*    Divide, unsigned 64-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    floor(u / v).
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Div_U64(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  //
#if (__SEGGER_RTL_TYPESET == 64) && (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
  //
  // Division is expected to be natively coded inline by the compiler.
  //
  return u / v;
  //
#elif (__SEGGER_RTL_TYPESET < 64) &&  __SEGGER_RTL_OPTIMIZE >= 0 && defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  //
  // Division will be synthesized by multiplication by reciprocal.
  //
  return __SEGGER_RTL_Div_U64_Moeller_Special(u, v);
  //
#elif (__SEGGER_RTL_OPTIMIZE >= 0) && (__SEGGER_RTL_TYPESET == 32)
  //
  // 32-bit target, balanced or fast: divide into two flows.
  //
  if (u == (__SEGGER_RTL_U32)u && v == (__SEGGER_RTL_U32)v) {
    return __SEGGER_RTL_Div_U32_Clockwork((__SEGGER_RTL_U32)u, (__SEGGER_RTL_U32)v, NULL);
  } else {
    return __SEGGER_RTL_Div_U64_Clockwork(u, v, NULL);
  }
  //
#else
  //
  return __SEGGER_RTL_Div_U64_Clockwork(u, v, NULL);
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_Mod_U64()
*
*  Function description
*    Remainder, unsigned 64-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Remainder of u / v.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U64 __SEGGER_RTL_Mod_U64(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  __SEGGER_RTL_U64 rem;
  //
#if (__SEGGER_RTL_TYPESET == 64) && (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
  //
  // Remainder is expected to be natively coded inline by the compiler.
  //
  rem = u % v;
  //
#elif __SEGGER_RTL_OPTIMIZE >= 0 && defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  //
  // Division will be synthesized by multiplication by reciprocal.
  //
  rem = u - v * __SEGGER_RTL_Div_U64(u, v);
  //
#elif (__SEGGER_RTL_OPTIMIZE >= 0) && (__SEGGER_RTL_TYPESET == 32)
  //
  // 32-bit target, balanced or fast: divide into two flows.
  //
  if (u == (__SEGGER_RTL_U32)u && v == (__SEGGER_RTL_U32)v) {
    __SEGGER_RTL_U32 hrem;
    //
    __SEGGER_RTL_Div_U32_Clockwork((__SEGGER_RTL_U32)u, (__SEGGER_RTL_U32)v, &hrem);
    rem = hrem;
  } else {
    __SEGGER_RTL_Div_U64_Clockwork(u, v, &rem);
  }
#else
  //
  __SEGGER_RTL_Div_U64_Clockwork(u, v, &rem);
  //
#endif
  //
  return rem;
}

#if defined(__SEGGER_RTL_U128)

/*********************************************************************
*
*       __SEGGER_RTL_Div_U128_Clockwork()
*
*  Function description
*    Calculate quotient using division by clockwork.
*
*  Parameters
*    u   - Dividend.
*    v   - Divisor.
*    rem - Pointer to object that receives the remainder.
*
*  Return value
*    floor(u / v)
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U128 __SEGGER_RTL_Div_U128_Clockwork(__SEGGER_RTL_U128 u, __SEGGER_RTL_U128 v, __SEGGER_RTL_U128 *rem) {
  __SEGGER_RTL_U128 quot;
  __SEGGER_RTL_U128 qbit;
  //
  if (v == 0) {
    u    = 0;
    quot = 0;
  } else {
    //
    quot = 0;
    qbit = 1;
    //
    while ((__SEGGER_RTL_I128)v >= 0) {
      v    <<= 1;
      qbit <<= 1;
    }
    //
    while (qbit) {
      if (v <= u) {
        u    -= v;
        quot += qbit;
      }
      v    >>= 1;
      qbit >>= 1;
    }
  }
  //
  if (rem) {
    *rem = u;
  }
  //
  return quot;
}

#endif

/*********************************************************************
*
*       __SEGGER_RTL_Mul_U32_Clockwork()
*
*  Function description
*    Multiply, signed 32-bit integer.
*
*  Parameters
*    a - Multiplier.
*    b - Multiplicand.
*
*  Return value
*    Product.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_U32 __SEGGER_RTL_Mul_U32_Clockwork(__SEGGER_RTL_U32 a, __SEGGER_RTL_U32 b) {
  __SEGGER_RTL_U32 Product;
  //
  if (a > b) {
    Product = 0;
    do {
      if (b & 1) {
        Product += a;
      }
      a <<= 1;
      b >>= 1;
    } while (b);
  } else {
    Product = 0;
    do {
      if (a & 1) {
        Product += b;
      }
      b <<= 1;
      a >>= 1;
    } while (a);
  }
  return Product;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#if __SEGGER_RTL_INCLUDE_GNU_API

/*********************************************************************
*
*       __ashldi3()
*
*  Function description
*    Shift left, 64-bit integer.
*
*  Parameters
*    x - Value to shift.
*    n - Number of places to shift left.
*
*  Return value
*    x shifted by n bits.
*
*  Thread safety
*    Safe.
*/
int64_t __SEGGER_RTL_PUBLIC_API __ashldi3(int64_t x, int n) {
  n &= 63;
  while (n) {
    x <<= 1;
    --n;
  }
  return x;
}

/*********************************************************************
*
*       __ashrdi3()
*
*  Function description
*    Shift right, 64-bit integer.
*
*  Parameters
*    x - Value to shift.
*    n - Number of places to shift right.
*
*  Return value
*    x shifted by n bits.
*
*  Thread safety
*    Safe.
*/
int64_t __SEGGER_RTL_PUBLIC_API __ashrdi3(int64_t x, int n) {
  n &= 63;
  while (n) {
    x >>= 1;
    --n;
  }
  return x;
}

/*********************************************************************
*
*       __lshrdi3()
*
*  Function description
*    Shift right, 64-bit unsign.
*
*  Parameters
*    x - Value to shift.
*    n - Number of places to shift right.
*
*  Return value
*    x shifted by n bits.
*
*  Thread safety
*    Safe.
*/
uint64_t __SEGGER_RTL_PUBLIC_API __lshrdi3(uint64_t x, int n) {
  n &= 63;
  while (n) {
    x >>= 1;
    --n;
  }
  return x;
}

/*********************************************************************
*
*       __mulsi3()
*
*  Function description
*    Multiply, signed 32-bit integer.
*
*  Parameters
*    a - Multiplier.
*    b - Multiplicand.
*
*  Return value
*    Product.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __mulsi3(__SEGGER_RTL_U32 a, __SEGGER_RTL_U32 b) {
#if defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  return a * b;
#else
  return __SEGGER_RTL_Mul_U32_Clockwork(a, b);
#endif
}

/*********************************************************************
*
*       __udivmodsi4()
*
*  Function description
*    Divide with remainder, unsigned 32-bit integer.
*
*  Parameters
*    u   - Divivdend.
*    v   - Divisor.
*    rem - Pointer to object that receives the remainder.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __udivmodsi4(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v, __SEGGER_RTL_U32 *rem) {
  return __SEGGER_RTL_Div_U32_Clockwork(u, v, rem);
}

/*********************************************************************
*
*       __muldi3()
*
*  Function description
*    Multiply, signed 64-bit integer.
*
*  Parameters
*    a - Multiplier.
*    b - Multiplicand.
*
*  Return value
*    Product.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __muldi3(__SEGGER_RTL_U64 a, __SEGGER_RTL_U64 b) {
  //
#if (__SEGGER_RTL_TYPESET == 64) && defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  //
  return a * b;
  //
  //
#elif !defined(__SEGGER_RTL_UMULL_SYNTHESIZED)
  //
  __SEGGER_RTL_U32 h;
  __SEGGER_RTL_U32 l;
  //
  __SEGGER_RTL_UMULL(l, h, __SEGGER_RTL_U64_L(a), __SEGGER_RTL_U64_L(b));
  h += __SEGGER_RTL_U64_H(a) * __SEGGER_RTL_U64_L(b);
  h += __SEGGER_RTL_U64_H(b) * __SEGGER_RTL_U64_L(a);
  //
  return ((__SEGGER_RTL_U64)h << 32) + l;
  //
#elif __SEGGER_RTL_OPTIMIZE >= 0
  //
  __SEGGER_RTL_U64 Product;
  __SEGGER_RTL_U32 x;
  __SEGGER_RTL_U64 y;
  //
  x = __SEGGER_RTL_U64_L(a);
  y = __SEGGER_RTL_U64_L(b);
  //
  if (x > y) {
    __SEGGER_RTL_U32 t;
    t = x;
    x = y;
    y = t;
  }
  //
  Product = 0;
  //
  while (x) {
    if (x & 1) {
      Product += y;
    }
    x >>= 1;
    y <<= 1;
  }
  //
  Product += (__SEGGER_RTL_U64)__SEGGER_RTL_Mul_U32_Clockwork(__SEGGER_RTL_U64_H(a), __SEGGER_RTL_U64_L(b)) << 32;
  Product += (__SEGGER_RTL_U64)__SEGGER_RTL_Mul_U32_Clockwork(__SEGGER_RTL_U64_H(b), __SEGGER_RTL_U64_L(a)) << 32;
  //
  return Product;
  //
#else
  //
  __SEGGER_RTL_U64 Product;
  //
  Product = 0;
  while (a) {
    if (a & 1) {
      Product += b;
    }
    a >>= 1;
    b <<= 1;
  }
  //
  return Product;
  //
#endif
}

#if defined(__SEGGER_RTL_U128)

/*********************************************************************
*
*       __multi3()
*
*  Function description
*    Multiply, signed 128-bit integer.
*
*  Parameters
*    a - Multiplier.
*    b - Multiplicand.
*
*  Return value
*    Product.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __multi3(__SEGGER_RTL_U128 a, __SEGGER_RTL_U128 b) {
  __SEGGER_RTL_U128 Product;
  //
  Product = 0;
  while (a) {
    if (a & 1) {
      Product += b;
    }
    a >>= 1;
    b <<= 1;
  }
  //
  return Product;
}

#endif

/*********************************************************************
*
*       __udivsi3()
*
*  Function description
*    Divide, unsigned 32-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __udivsi3(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  return __SEGGER_RTL_Div_U32(u, v);
}

/*********************************************************************
*
*       __umodsi3()
*
*  Function description
*    Remainder after divide, unsigned 32-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Remainder.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __umodsi3(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  return __SEGGER_RTL_Mod_U32(u, v);
}

/*********************************************************************
*
*       __udivmoddi4()
*
*  Function description
*    Divide with remainder, unsigned 64-bit integer.
*
*  Parameters
*    u   - Dividend.
*    v   - Divisor.
*    rem - Pointer to object that receives the remainder.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __udivmoddi4(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v, __SEGGER_RTL_U64 *rem) {
  return __SEGGER_RTL_Div_U64_Clockwork(u, v, rem);
}

/*********************************************************************
*
*       __udivdi3()
*
*  Function description
*    Divide, unsigned 64-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    floor(u / v).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __udivdi3(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  return __SEGGER_RTL_Div_U64(u, v);
}

/*********************************************************************
*
*       __umoddi3()
*
*  Function description
*    Remainder after divide, unsigned 64-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Remainder.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __umoddi3(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  return __SEGGER_RTL_Mod_U64(u, v);
}

/*********************************************************************
*
*       __divdi3()
*
*  Function description
*    Divide, signed 64-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
int64_t __SEGGER_RTL_PUBLIC_API __divdi3(int64_t u, int64_t v) {
  //
#if (__SEGGER_RTL_TYPESET == 64) && (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
  //
  return u / v;
  //
#else
  //
  int64_t q;
  //
  q = __SEGGER_RTL_Div_U64(__SEGGER_RTL_ABS_I64(u), __SEGGER_RTL_ABS_I64(v));
  //
  return __SEGGER_RTL_CNEG_I64(q, u ^ v);
  //
#endif
}

/*********************************************************************
*
*       __moddi3()
*
*  Function description
*    Remainder after divide, signed 64-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Remainder.
*
*  Thread safety
*    Safe.
*/
int64_t __SEGGER_RTL_PUBLIC_API __moddi3(int64_t u, int64_t v) {
  //
#if (__SEGGER_RTL_TYPESET == 64) && (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
  //
  return u % v;
  //
#else
  //
  __SEGGER_RTL_U64 rem;
  //
  v = __SEGGER_RTL_ABS_I64(v);
  rem = __SEGGER_RTL_Mod_U64(__SEGGER_RTL_ABS_I64(u), v);
  //
  return __SEGGER_RTL_CNEG_I64(rem, u);
  //
#endif
}

/*********************************************************************
*
*       __divsi3()
*
*  Function description
*    Divide, signed 32-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
int32_t __SEGGER_RTL_PUBLIC_API __divsi3(int32_t u, int32_t v) {
  int32_t q;
  //
#if (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
  //
  q = u / v;
  //
#else
  //
  q = __SEGGER_RTL_Div_U32(__SEGGER_RTL_ABS_I32(u), __SEGGER_RTL_ABS_I32(v));
  q = __SEGGER_RTL_CNEG_I32(q, u ^ v);
  //
#endif
  //
  return q;
}

/*********************************************************************
*
*       __modsi3()
*
*  Function description
*    Remainder after divide, signed 32-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Remainder.
*
*  Thread safety
*    Safe.
*/
int32_t __SEGGER_RTL_PUBLIC_API __modsi3(int32_t u, int32_t v) {
  __SEGGER_RTL_U32 rem;
  //
#if (defined(__SEGGER_RTL_CORE_HAS_DIV) && __SEGGER_RTL_CORE_HAS_DIV) || (defined(__SEGGER_RTL_CORE_HAS_IDIV) && __SEGGER_RTL_CORE_HAS_IDIV)
  //
  rem = u % v;
  //
#elif defined(__SEGGER_RTL_CORE_HAS_MUL_MULH) && __SEGGER_RTL_CORE_HAS_MUL_MULH
  //
  v = __SEGGER_RTL_ABS_I32(v);
  rem = __SEGGER_RTL_ABS_I32(u) - v * __SEGGER_RTL_Div_U32(__SEGGER_RTL_ABS_I32(u), v);
  rem = __SEGGER_RTL_CNEG_I32(rem, u);
  //
#else
  //
  __SEGGER_RTL_Div_U32_Clockwork(__SEGGER_RTL_ABS_I32(u), __SEGGER_RTL_ABS_I32(v), &rem);
  rem = __SEGGER_RTL_CNEG_I32(rem, u);
  //
#endif
  //
  return rem;
}

#if defined(__SEGGER_RTL_U128)

/*********************************************************************
*
*       __udivti3()
*
*  Function description
*    Divide, unsigned 128-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    floor(u / v).
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __udivti3(__SEGGER_RTL_U128 u, __SEGGER_RTL_U128 v) {
  return __SEGGER_RTL_Div_U128_Clockwork(u, v, NULL);
}

/*********************************************************************
*
*       __umodti3()
*
*  Function description
*    Remainder after divide, unsigned 128-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Remainder.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __umodti3(__SEGGER_RTL_U128 u, __SEGGER_RTL_U128 v) {
  __SEGGER_RTL_U128 r;
  //
  __SEGGER_RTL_Div_U128_Clockwork(u, v, &r);
  return r;
}

/*********************************************************************
*
*       __divti3()
*
*  Function description
*    Divide, signed 128-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Quotient.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __divti3(__SEGGER_RTL_U128 u, __SEGGER_RTL_U128 v) {
  __SEGGER_RTL_U128 q;
  //
  q = __SEGGER_RTL_Div_U128_Clockwork(__SEGGER_RTL_ABS_I128(u), __SEGGER_RTL_ABS_I128(v), NULL);
  //
  return __SEGGER_RTL_CNEG_I128(q, u ^ v);
}

/*********************************************************************
*
*       __modti3()
*
*  Function description
*    Remainder after divide, signed 128-bit integer.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    Remainder.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __modti3(__SEGGER_RTL_U128 u, __SEGGER_RTL_U128 v) {
  __SEGGER_RTL_U128 rem;
  //
  __SEGGER_RTL_Div_U128_Clockwork(__SEGGER_RTL_ABS_I128(u), __SEGGER_RTL_ABS_I128(v), &rem);
  //
  return __SEGGER_RTL_CNEG_I128(rem, u);
}

#endif

#endif

/*********************************************************************
*
*       __clzsi2()
*
*  Function description
*    Count leading zeros, 32-bit integer.
*
*  Parameters
*    x - Argument; x must not be zero.
*
*  Return value
*    Number of leading zeros in x.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __clzsi2(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_CLZ_U32(x);
}

/*********************************************************************
*
*       __clzdi2()
*
*  Function description
*    Count leading zeros, 64-bit integer.
*
*  Parameters
*    x - Argument; x must not be zero.
*
*  Return value
*    Number of leading zeros in x.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __clzdi2(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_CLZ_U64(x);
}

/*********************************************************************
*
*       __ffssi2()
*
*  Function description
*    Find first set bit, integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    == 0 - No bit set in x.
*    != 0 - Position of first bit set in x, with 1 being bit 0,
*           2 being bit 1, and so on.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __ffssi2(__SEGGER_RTL_I32 x) {
  if (x == 0) {
    return 0;
  } else {
#if defined(__SEGGER_RTL_BITREV_U32)
    return __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_BITREV_U32(x)) + 1;
#else
    int n;
    //
    n = 1;
    while ((x & 1) == 0) {
      x >>= 1;
      ++n;
    }
    //
    return n;
#endif
  }
}

/*********************************************************************
*
*       __ffsdi2()
*
*  Function description
*    Find first set bit, long long.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    == 0 - No bit set in x.
*    != 0 - Position of first bit set in x, with 1 being bit 0,
*           2 being bit 1, and so on.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __ffsdi2(__SEGGER_RTL_I64 x) {
  if (x == 0) {
    return 0;
  } else {
#if defined(__SEGGER_RTL_BITREV_U64)
    return __SEGGER_RTL_CLZ_U64(__SEGGER_RTL_BITREV_U64(x)) + 1;
#else
    int n;
    //
    n = 1;
    while ((x & 1) == 0) {
      x >>= 1;
      ++n;
    }
    //
    return n;
#endif
  }
}

/*********************************************************************
*
*       ffs()
*
*  Function description
*    Find first set bit, integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    == 0 - No bit set in x.
*    != 0 - Position of first bit set in x, with 1 being bit 0,
*           2 being bit 1, and so on.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API ffs(int x) {
  if (x == 0) {
    return 0;
  } else {
#if (__SEGGER_RTL_SIZEOF_INT == 2) || (__SEGGER_RTL_SIZEOF_INT == 4) && defined(__SEGGER_RTL_BITREV_U32)
    return __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_BITREV_U32(x)) + 1;
#elif (__SEGGER_RTL_SIZEOF_INT == 8) && defined(__SEGGER_RTL_BITREV_U64)
    return __SEGGER_RTL_CLZ_U64(__SEGGER_RTL_BITREV_U64(x)) + 1;
#else
    int n;
    //
    n = 1;
    while ((x & 1) == 0) {
      x >>= 1;
      ++n;
    }
    //
    return n;
#endif
  }
}

/*********************************************************************
*
*       ffsl()
*
*  Function description
*    Find first set bit, long.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    == 0 - No bit set in x.
*    != 0 - Position of first bit set in x, with 1 being bit 0,
*           2 being bit 1, and so on.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API ffsl(long x) {
  if (x == 0) {
    return 0;
  } else {
#if (__SEGGER_RTL_SIZEOF_LONG == 4) && defined(__SEGGER_RTL_BITREV_U32)
    return __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_BITREV_U32(x)) + 1;
#elif (__SEGGER_RTL_SIZEOF_INT == 8) && defined(__SEGGER_RTL_BITREV_U64)
    return __SEGGER_RTL_CLZ_U64(__SEGGER_RTL_BITREV_U64(x)) + 1;
#else
    int n;
    //
    n = 1;
    while ((x & 1) == 0) {
      x >>= 1;
      ++n;
    }
    //
    return n;
#endif
  }
}

/*********************************************************************
*
*       ffsll()
*
*  Function description
*    Find first set bit, long long.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    == 0 - No bit set in x.
*    != 0 - Position of first bit set in x, with 1 being bit 0,
*           2 being bit 1, and so on.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API ffsll(long long x) {
  if (x == 0) {
    return 0;
  } else {
#if defined(__SEGGER_RTL_BITREV_U64)
    return __SEGGER_RTL_CLZ_U64(__SEGGER_RTL_BITREV_U64(x)) + 1;
#else
    int n;
    //
    n = 1;
    while ((x & 1) == 0) {
      x >>= 1;
      ++n;
    }
    //
    return n;
#endif
  }
}

/*********************************************************************
*
*       __ctzsi2()
*
*  Function description
*    Count trailing zeros, 32-bit integer.
*
*  Parameters
*    x - Argument; x must not be zero.
*
*  Return value
*    Number of trailing zeros in x.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __ctzsi2(unsigned x) {
  //
#if defined(__SEGGER_RTL_BITREV_U32)
  //
  return __clzsi2(__SEGGER_RTL_BITREV_U32(x));
  //
#else
  //
  unsigned n;
  //
  if (x == 0) {
    return 32;
  }
  //
  n = 0;
  //
  if ((x & 0xFFFFu) == 0) {
    n  += 16;
    x >>= 16;
  }
  if ((x & 0xFFu) == 0) {
    n  += 8;
    x >>= 8;
  }
  if ((x & 0xFu) == 0) {
    n  += 4;
    x >>= 4;
  }
  if ((x & 0x3u) == 0) {
    n  += 2;
    x >>= 2;
  }
  //
  return n + (~x & 1);
  //
#endif
}

/*********************************************************************
*
*       __ctzti2()
*
*  Function description
*    Count trailing zeros, 64-bit integer.
*
*  Parameters
*    x - Argument; x must not be zero.
*
*  Return value
*    Number of trailing zeros in x.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __ctzti2(unsigned long long x) {
  if (x & 0xFFFFFFFFuL) {
    return __ctzsi2((uint32_t)x);
  } else {
    return 32 + __ctzsi2((uint32_t)(x >> 32));
  }
}

/*********************************************************************
*
*       __ctzti2()
*
*  Function description
*    Count trailing zeros, long integer.
*
*  Parameters
*    x - Argument; x must not be zero.
*
*  Return value
*    Number of trailing zeros in x.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __ctzdi2(unsigned long x) {
  if (sizeof(unsigned long) == sizeof(unsigned int)) {
    return __ctzsi2(x);
  } else {
    return __ctzti2(x);
  }
}

/*********************************************************************
*
*       __bswapsi2()
*
*  Function description
*    Swap byte order, 32-bit integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Byte-swapped value.
*
*  Thread safety
*    Safe.
*/
uint32_t __SEGGER_RTL_PUBLIC_API __bswapsi2(uint32_t x) {
  return (((x & 0xFF000000) >> 24) |
          ((x & 0x00FF0000) >>  8) |
          ((x & 0x0000FF00) <<  8) |
          ((x & 0x000000FF) << 24));
}

/*********************************************************************
*
*       __bswapdi2()
*
*  Function description
*    Swap byte order, 64-bit integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Byte-swapped value.
*
*  Thread safety
*    Safe.
*/
uint64_t __SEGGER_RTL_PUBLIC_API __bswapdi2(uint64_t x) {
  return (((x & 0xfF00000000000000uLL) >> 56) |
          ((x & 0x00FF000000000000uLL) >> 40) |
          ((x & 0x0000FF0000000000uLL) >> 24) |
          ((x & 0x000000FF00000000uLL) >>  8) |
          ((x & 0x00000000FF000000uLL) <<  8) |
          ((x & 0x0000000000FF0000uLL) << 24) |
          ((x & 0x000000000000FF00uLL) << 40) |
          ((x & 0x00000000000000FFuLL) << 56));
}

/*********************************************************************
*
*       __popcountsi2()
*
*  Function description
*    Population count, 32-bit integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Count of number of one bits in x.
*
*  Reference
*    Hacker's Delight, section 5.1.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __popcountsi2(__SEGGER_RTL_U32 x) {
  return __SEGGER_RTL_PCT_U32_inline(x);
}

/*********************************************************************
*
*       __popcountdi2()
*
*  Function description
*    Population count, 64-bit integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    Count of number of one bits in x.
*
*  Reference
*    Hacker's Delight, section 5.1.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __popcountdi2(__SEGGER_RTL_U64 x) {
  return __SEGGER_RTL_PCT_U32_inline(__SEGGER_RTL_U64_H(x)) +
         __SEGGER_RTL_PCT_U32_inline(__SEGGER_RTL_U64_L(x));
}

/*********************************************************************
*
*       __paritysi2()
*
*  Function description
*    Parity, 32-bit integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    == 1 - number of one bits in x is odd.
*    == 0 - number of one bits in x is even.
*
*  Reference
*    Hacker's Delight, section 5.2.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __paritysi2(__SEGGER_RTL_U32 x) {
  x ^= x >> 16;
  x ^= x >> 8;
  x ^= x >> 4;
  return (0x6996 >> (x & 0xF)) & 1;
}

/*********************************************************************
*
*       __paritydi2()
*
*  Function description
*    Parity, 64-bit integer.
*
*  Parameters
*    x - Argument.
*
*  Return value
*    == 1 - number of one bits in x is odd.
*    == 0 - number of one bits in x is even.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API __paritydi2(__SEGGER_RTL_U64 x) {
  __SEGGER_RTL_U32 a;
  //
  a = __SEGGER_RTL_U64_H(x) ^ __SEGGER_RTL_U64_L(x);
  a ^= a >> 16;
  a ^= a >> 8;
  a ^= a >> 4;
  return (0x6996 >> (a & 0xF)) & 1;
}

/*********************************************************************
*
*       abs()
*
*  Function description
*    Calculate absolute value, int.
*
*  Parameters
*    Value - Integer value.
*
*  Return value
*    The absolute value of the integer argument Value.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API abs(int Value) {
  return Value >= 0 ? Value : -Value;
}

/*********************************************************************
*
*       labs()
*
*  Function description
*    Calculate absolute value, long.
*
*  Parameters
*    Value - Long integer value.
*
*  Return value
*    The absolute value of the long integer argument Value.
*
*  Thread safety
*    Safe.
*/
long int __SEGGER_RTL_PUBLIC_API labs(long int Value) {
  return Value >= 0 ? Value : -Value;
}

/*********************************************************************
*
*       llabs()
*
*  Function description
*    Calculate absolute value, long long.
*
*  Parameters
*    Value - Long long integer value.
*
*  Return value
*    The absolute value of the long long integer argument Value.
*
*  Thread safety
*    Safe.
*/
long long int __SEGGER_RTL_PUBLIC_API llabs(long long int Value) {
  return Value >= 0 ? Value : -Value;
}

/*********************************************************************
*
*       div()
*
*  Function description
*    Divide returning quotient and remainder, int.
*
*  Parameters
*    Numer - Numerator.
*    Denom - Demoninator.
*
*  Additional information
*    This computes Numer divided by Denom and Numer modulo Denom
*    in a single operation.
*
*  Return value
*    Returns a structure of type div_t comprising both the
*    quotient and the remainder. The structures contain the members
*    quot (the quotient) and rem (the remainder), each of which has
*    the same type as the arguments Numer and Denom. If either part
*    of the result cannot be represented, the behavior is undefined.
*
*  Thread safety
*    Safe.
*
*  See also
*    div_t
*/
div_t __SEGGER_RTL_PUBLIC_API div(int Numer, int Denom) {
  div_t Result;
  //
  Result.quot = Numer / Denom;
  Result.rem  = Numer % Denom;
  //
  // Some processors use symmetric division.  Detect that here and
  // fix up for floored division.
  //
  if (Numer < 0 && Result.rem > 0) {
    Result.quot += 1;
    Result.rem  -= Denom;
  }
  //
  return Result;
}

/*********************************************************************
*
*       ldiv()
*
*  Function description
*    Divide returning quotient and remainder, long.
*
*  Parameters
*    Numer - Numerator.
*    Denom - Demoninator.
*
*  Additional information
*    This computes Numer divided by Denom and Numer modulo Denom
*    in a single operation.
*
*  Return value
*    Returns a structure of type ldiv_t comprising both the
*    quotient and the remainder. The structures contain the members
*    quot (the quotient) and rem (the remainder), each of which has
*    the same type as the arguments Numer and Denom. If either part
*    of the result cannot be represented, the behavior is undefined.
*
*  Thread safety
*    Safe.
*
*  See also
*    ldiv_t
*/
ldiv_t __SEGGER_RTL_PUBLIC_API ldiv(long Numer, long Denom) {
  ldiv_t Result;
  //
  Result.quot = Numer / Denom;
  Result.rem  = Numer % Denom;
  //
  // Some processors use symmetric division.  Detect that here and
  // fix up for floored division.
  //
  if (Numer < 0 && Result.rem > 0) {
    Result.quot += 1;
    Result.rem  -= Denom;
  }
  //
  return Result;
}

/*********************************************************************
*
*       lldiv()
*
*  Function description
*    Divide returning quotient and remainder, long long.
*
*  Parameters
*    Numer - Numerator.
*    Denom - Demoninator.
*
*  Additional information
*    This computes Numer divided by Denom and Numer modulo Denom
*    in a single operation.
*
*  Return value
*    Returns a structure of type lldiv_t comprising both the
*    quotient and the remainder. The structures contain the members
*    quot (the quotient) and rem (the remainder), each of which has
*    the same type as the arguments Numer and Denom. If either part
*    of the result cannot be represented, the behavior is undefined.
*
*  Thread safety
*    Safe.
*
*  See also
*    lldiv_t
*/
lldiv_t __SEGGER_RTL_PUBLIC_API lldiv(long long Numer, long long Denom) {
  lldiv_t Result;
  //
  Result.quot = Numer / Denom;
  Result.rem  = Numer % Denom;
  //
  // Some processors use symmetric division.  Detect that here and
  // fix up for floored division.
  //
  if (Numer < 0 && Result.rem > 0) {
    Result.quot += 1;
    Result.rem  -= Denom;
  }
  //
  return Result;
}

/*********************************************************************
*
*       imaxdiv()
*
*  Function description
*    Divide returning quotient and remainder, intmax_t.
*
*  Parameters
*    Numer - Numerator.
*    Denom - Demoninator.
*
*  Additional information
*    This computes Numer divided by Denom and Numer modulo Denom
*    in a single operation.
*
*  Return value
*    Returns a structure of type imaxdiv_t comprising both the
*    quotient and the remainder. The structures contain the members
*    quot (the quotient) and rem (the remainder), each of which has
*    the same type as the arguments Numer and Denom. If either part
*    of the result cannot be represented, the behavior is undefined.
*
*  Thread safety
*    Safe.
*/
imaxdiv_t __SEGGER_RTL_PUBLIC_API imaxdiv(intmax_t Numer, intmax_t Denom) {
  imaxdiv_t Result;
  //
  Result.quot = Numer / Denom;
  Result.rem  = Numer % Denom;
  //
  // Some processors use symmetric division.  Detect that here and
  // fix up for floored division.
  //
  if (Numer < 0 && Result.rem > 0) {
    Result.quot += 1;
    Result.rem  -= Denom;
  }
  //
  return Result;
}

/*********************************************************************
*
*       rand()
*
*  Function description
*    Return next random number in sequence.
*
*  Additional information
*    This computes a sequence of pseudo-random integers in the
*    range 0 to RAND_MAX.
*
*  Return value
*    Returns the computed pseudo-random integer.
*
*  Thread safety
*    Safe [if configured].
*
*  See also
*    srand()
*/
int __SEGGER_RTL_PUBLIC_API rand(void) {
  //
  // Use the ANSI version.
  //
  __SEGGER_RTL_rand_next = __SEGGER_RTL_rand_next * 1103515245 + 12345;
  //
  return (int)((__SEGGER_RTL_rand_next >> 16) % 32768);
}

/*********************************************************************
*
*       srand()
*
*  Function description
*    Set seed of random number sequence.
*
*  Parameters
*    s - New seed value for pseudo-random sequence.
*
*  Additional information
*    This uses the argument Seed as a seed for a new sequence of
*    pseudo-random numbers to be returned by subsequent calls to rand().
*    If srand() is called with the same seed value, the same sequence
*    of pseudo-random numbers is generated.
*
*    If rand() is called before any calls to srand() have been made, a
*    sequence is generated as if srand() is first called with a seed
*    value of 1.
*
*  Thread safety
*    Safe [if configured].
*
*  See also
*    rand()
*/
void __SEGGER_RTL_PUBLIC_API srand(unsigned s) {
  __SEGGER_RTL_rand_next = s;
}

#if __SEGGER_RTL_INCLUDE_BENCHMARKING

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Moeller_Standalone()
*
*  Function description
*    Calculate quotient using Moeller-Grunland algorithm.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v)
*
*  Additional information
*    See [MOELLER].
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_Div_U32_Moeller_Standalone(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  return __SEGGER_RTL_Div_U32_Moeller(u, v);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Tang_Standalone()
*
*  Function description
*    Calculate quotient using Tang algorithm.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v)
*
*  Additional information
*    See [TANG].
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_Div_U32_Tang_Standalone(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  return __SEGGER_RTL_Div_U32_Tang(u, v);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Clockwork_Standalone()
*
*  Function description
*    Calculate quotient using division by clockwork.
*
*  Parameters
*    u   - Dividend.
*    v   - Divisor.
*
*  Return value
*    floor(u / v)
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_Div_U32_Clockwork_Standalone(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  return __SEGGER_RTL_Div_U32_Clockwork(u, v, NULL);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Moeller_Special_Standalone()
*
*  Function description
*    Calculate quotient using Moeller-Grunland algorithm, with special cases.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v)
*
*  Additional information
*    See [MOELLER].
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_Div_U32_Moeller_Special_Standalone(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  return __SEGGER_RTL_Div_U32_Moeller_Special(u, v);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U32_Tang_Special_Standalone()
*
*  Function description
*    Calculate quotient using Tang algorithm, with special cases.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v)
*
*  Additional information
*    See [TANG].
*/
__SEGGER_RTL_U32 __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_Div_U32_Tang_Special_Standalone(__SEGGER_RTL_U32 u, __SEGGER_RTL_U32 v) {
  return __SEGGER_RTL_Div_U32_Tang_Special(u, v);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_Moeller_Standalone()
*
*  Function description
*    64-bit by 64-bit division, Moeller algorithm.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v).
*
*  Additional information
*    This is a modification of the algorithm presented by [WARREN],
*    which itself is a specialization of Algorithm D in [AOCP].
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_Div_U64_Moeller_Standalone(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  return __SEGGER_RTL_Div_U64_Moeller(u, v);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_Moeller_Special_Standalone()
*
*  Function description
*    64-bit by 64-bit division, Moeller algorithm, special-cased.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*     floor(u / v).
*
*  Additional information
*    Special cases for divison of smaller operands.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_Div_U64_Moeller_Special_Standalone(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  return __SEGGER_RTL_Div_U64_Moeller_Special(u, v);
}

/*********************************************************************
*
*       __SEGGER_RTL_Div_U64_Clockwork_Standalone()
*
*  Function description
*    Calculate quotient using division by clockwork.
*
*  Parameters
*    u - Dividend.
*    v - Divisor.
*
*  Return value
*    floor(u / v)
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_Div_U64_Clockwork_Standalone(__SEGGER_RTL_U64 u, __SEGGER_RTL_U64 v) {
  return __SEGGER_RTL_Div_U64_Clockwork(u, v, NULL);
}

#endif

/*************************** End of file ****************************/
