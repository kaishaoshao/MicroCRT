/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "ctype.h"
#include "wctype.h"
#include "stdlib.h"

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

// 10^(2^i)
static const double __SEGGER_RTL_aPower2[] = {
  1.e+001,
  1.e+002,
  1.e+004,
  1.e+008,
  1.e+016,
  1.e+032,
  1.e+064,
  1.e+128,
  1.e+256
};

// 10^(2^i)
static const float __SEGGER_RTL_aPower2f[] = {
  1.e+001f,
  1.e+002f,
  1.e+004f,
  1.e+008f,
  1.e+016f,
  1.e+032f,
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_pow10()
*
*/
double __SEGGER_RTL_pow10(int e) {
  const double *p;
  double t;
  char   recip;

  recip = 0;
  if (e < 0) {
    e = -e;
    recip = 1;
  }
  t = 1;
  p = __SEGGER_RTL_aPower2;
  while (e > 0) {
    if (e & 1) {
      t *= *p;
    }
    e >>= 1;
    ++p;
  }
  //
  return recip ? 1.0 / t : t;
}

/*********************************************************************
*
*       __SEGGER_RTL_pow10f()
*
*/
float __SEGGER_RTL_pow10f(int e) {
  const float *p;
  float t;
  int   recip;
  //
  recip = 0;
  if (e < 0) {
    e = -e;
    recip = 1;
  }
  t = 1;
  p = __SEGGER_RTL_aPower2f;
  while (e > 0) {
    if (e & 1) {
      t *= *p;
    }
    e >>= 1;
    ++p;
  }
  //
  return recip ? 1.0f / t : t;
}

/*********************************************************************
*
*       __SEGGER_RTL_digit()
*
*/
int __SEGGER_RTL_digit(int ch, int radix) {
  if ((isdigit)(ch)) {
    ch -= '0';
  } else if ((islower)(ch)) {
    ch -= 'a' - 10;
  } else if ((isupper)(ch)) {
    ch -= 'A' - 10;
  } else {
    ch = -1;
  }
  //
  return ch < radix ? ch : -1;
}

/*********************************************************************
*
*       __SEGGER_RTL_wdigit()
*
*/
int __SEGGER_RTL_wdigit(wchar_t ch, int radix) {
  if ((iswdigit)(ch)) {
    ch -= L'0';
  } else if ((iswlower)(ch)) {
    ch -= L'a' - 10;
  } else if ((iswupper)(ch)) {
    ch -= L'A' - 10;
  } else {
    return -1;
  }
  //
  return (int)ch < radix ? ch : -1;
}

/*************************** End of file ****************************/
