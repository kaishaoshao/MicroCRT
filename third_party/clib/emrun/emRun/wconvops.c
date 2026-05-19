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

#include "__SEGGER_RTL_Int.h"
#include "wctype.h"
#include "stdlib.h"
#include "stddef.h"
#include "errno.h"
#include "math.h"
#include "limits.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define FLAG_DOTSEEN      (1<<0)
#define FLAG_NUMOK        (1<<1)
#define FLAG_NUMNEG       (1<<2)
#define FLAG_EXPNEG       (1<<3)

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_wcstoul()
*
*  Function description
*    Convert to number, long.
*
*  Parameters
*    nsptr  - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted wide character.
*    base   - Radix to use for conversion, 2 to 36.
*
*  Return value
*    Returns the converted value, if any. If no conversion could be
*    performed, zero is returned. If the correct value is outside
*    the range of representable values, LONG_MIN or LONG_MAX is
*    returned according to the sign of the value, if any, and the
*    value of the macro ERANGE is stored in errno.
*/
static unsigned long __SEGGER_RTL_wcstoul(const wchar_t *nsptr, wchar_t **endptr, int base) {
  const wchar_t *nptr = nsptr;
  wchar_t c;
  int ok, overflowed, digit;
  unsigned long dhigh, dlow;
  //
  // Not successful yet, nor overflown.
  //
  ok = 0;
  overflowed = 0;
  //
  // Skip initial spaces.
  //
  while ((c = *nptr++) != 0 && (iswspace)(c)) {
    // pass
  }
  //
  // Leading zero needs to be treated as, perhaps, octal or hex.
  //
  if (c == L'0') {
    //
    // Say this is ok for now.
    //
    ok = 1;
    c = *nptr++;
    if (c == L'x' || c == L'X') {
      //
      // 0x is only a valid prefix if no base is given, or hex is given.
      //
      if (base == 0 || base == 16) {
        //
        // Need more than 0x to start off a hex number.
        //
        ok = 0;
        base = 16;
        c = *nptr++;
      }
    } else if (base == 0) {
      base = 8;
    }
  }
  //
  // If no base specified by the caller, and none given by the
  // user, default to decimal.
  //
  if (base == 0) {
    base = 10;
  }
  //
  // Convert digit stream until we hit a non-digit in base.
  //
  dhigh = 0;
  dlow = 0;
  while ((digit = __SEGGER_RTL_wdigit(c, base)) >= 0) {
    //
    // Have seen digits, so syntax is ok.
    //
    ok = 1;
    //
#if __SEGGER_RTL_SIZEOF_LONG == 4
    dlow = base * dlow + digit;
    dhigh = base * dhigh + (dlow >> 16);
    dlow &= 0xFFFFu;
    if (dhigh >= 0x10000u) {
      overflowed = 1;
    }
#elif __SEGGER_RTL_SIZEOF_LONG == 8
    dlow = base * dlow + digit;
    dhigh = base * dhigh + (dlow >> 32);
    dlow &= 0xFFFFFFFFuL;
    if (dhigh >= 0x100000000uL) {
      overflowed = 1;
    }
#else
  #error case not covered
#endif
    c = *nptr++;
  }
  //
  if (endptr) {
    *endptr = ok ? (wchar_t *)nptr-1 : (wchar_t *)nsptr;
  }
  //
  if (overflowed) {
    errno = ERANGE;
    return ULONG_MAX;
  } else {
#if __SEGGER_RTL_SIZEOF_LONG == 4
    return (dhigh << 16) | dlow;
#else
    return (dhigh << 32) | dlow;
#endif
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_wcstoull()
*
*  Function description
*    Convert to number, long long.
*
*  Parameters
*    nsptr  - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted wide character.
*    base   - Radix to use for conversion, 2 to 36.
*
*  Return value
*    Returns the converted value, if any. If no conversion could be
*    performed, zero is returned. If the correct value is outside
*    the range of representable values, LONG_MIN or LONG_MAX is
*    returned according to the sign of the value, if any, and the
*    value of the macro ERANGE is stored in errno.
*/
static unsigned long long __SEGGER_RTL_wcstoull(const wchar_t *nsptr, wchar_t **endptr, int base) {
  const wchar_t *nptr = nsptr;
  wchar_t c;
  int ok, overflowed, digit;
  unsigned long long dhigh, dlow;
  //
  // Not successful yet, nor overflown.
  //
  ok = 0;
  overflowed = 0;
  //
  // Skip initial spaces.
  //
  while ((c = *nptr++) != 0 && (iswspace)(c)) {
    // pass
  }
  //
  // Leading zero needs to be treated as, perhaps, octal or hex.
  //
  if (c == L'0') {
    //
    // Say this is ok for now.
    //
    ok = 1;
    c = *nptr++;
    if (c == L'x' || c == L'X') {
      //
      // 0x is only a valid prefix if no base is given, or hex is given.
      //
      if (base == 0 || base == 16) {
        //
        // Need more than 0x to start off a hex number.
        //
        ok = 0;
        base = 16;
        c = *nptr++;
      }
    } else if (base == 0) {
      base = 8;
    }
  }
  //
  // If no base specified by the caller, and none given by the
  // user, default to decimal.
  //
  if (base == 0) {
    base = 10;
  }
  //
  // Convert digit stream until we hit a non-digit in base.
  //
  dhigh = 0;
  dlow = 0;
  while ((digit = __SEGGER_RTL_wdigit(c, base)) >= 0) {
    //
    // Have seen digits, so syntax is ok.
    //
    ok = 1;
    //
    dlow = base * dlow + digit;
    dhigh = base * dhigh + (dlow >> 32);
    dlow &= 0xFFFFFFFFuLL;
    if (dhigh >= 0x100000000uLL) {
      overflowed = 1;
    }
    c = *nptr++;
  }
  //
  if (endptr) {
    *endptr = ok ? (wchar_t *)nptr-1 : (wchar_t *)nsptr;
  }
  //
  if (overflowed) {
    errno = ERANGE;
    return ULLONG_MAX;
  } else {
    return (dhigh << 32) | dlow;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       wcstof()
*
*  Function description
*    Convert to number, float.
*
*  Parameters
*    nptr   - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted character.
*
*  Return value
*    Returns the converted value, if any. If no conversion could be
*    performed, zero is returned. If the correct value is outside the
*    range of representable values, HUGE_VALF is returned according
*    to the sign of the value, if any, and the value of the macro ERANGE
*    is stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to float representation.
*    
*    First, wcstof() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling a floating-point constant, and a final string
*    of one or more unrecognized characters, including the terminating null character
*    of the input string. wcstof() then attempts to convert the subject sequence
*    to a floating-point number, and return the result.
*    
*    The subject sequence is defined as the longest initial subsequence of the input
*    string, starting with the first non-white-space character, that is of the expected
*    form. The subject sequence contains no characters if the input string is empty
*    or consists entirely of white space, or if the first non-white-space character
*    is other than a sign or a permissible letter or digit.
*    
*    The expected form of the subject sequence is an optional plus or minus sign
*    followed by a nonempty sequence of decimal digits optionally containing a
*    decimal-point character, then an optional exponent part.
*    
*    If the subject sequence begins with a minus sign, the value resulting from
*    the conversion is negated.  A pointer to the final string is stored in the
*    object pointed to by endptr, provided that endptr is not a null pointer.
*    
*    If the subject sequence is empty or does not have the expected form, no conversion
*    is performed, the value of nptr is stored in the object pointed to by
*    endptr, provided that endptr is not a null pointer.
*
*  See also
*    wcstod()
*
*  Thread safety
*    Safe.
*/
float __SEGGER_RTL_PUBLIC_API wcstof(const wchar_t *nptr, wchar_t **endptr) {
  const wchar_t *p = nptr;
  wchar_t ch;
  int x = 0;
  float l = 0.0;
  int flag = 0;
  //
  while ((iswspace)(ch = *p++)) {
    // pass
  }
  //
  switch (ch) {
  case L'-':
    flag |= FLAG_NUMNEG;
    //FALLTHRU
  case L'+':
    ch = *p++;
  }
  //
  for (;;) {
    if (ch == L'.' && (flag & FLAG_DOTSEEN) == 0) {
      flag |= FLAG_DOTSEEN;
    } else if ((iswdigit)(ch)) {
      flag |= FLAG_NUMOK;
      l = l*10 + (ch - L'0');
      if (flag & FLAG_DOTSEEN) {
        --x;
      }
    } else {
      break;
    }
    ch = *p++;
  }
  //
  // Unread the 'e' in (e.g.) "+.e" as this isn't valid.
  //
  if ((ch == L'e' || ch == L'E') && (flag & FLAG_NUMOK)) {
    int x2 = 0;
    flag &= ~FLAG_NUMOK;
    switch (ch = *p++) {
    case L'-':
      flag |= FLAG_EXPNEG;
      //FALLTHRU
    case L'+':
      ch = *p++;
    }
    while ((iswdigit)(ch)) {
      flag |= FLAG_NUMOK;
      x2 = (x2 > 1000) ? 10000 : 10*x2 + (ch - L'0');
      ch = *p++;
    }
    if (flag & FLAG_EXPNEG) {
      x -= x2;
    } else {
      x += x2;
    }
  }
  //
  // Scale exponent.
  //
  while (x > 0) {
    l *= 10;
    --x;
  }
  while (x < 0) {
    l /= 10;
    ++x;
  }
  //
  if (isinf(l)) {
    errno = ERANGE;
    l = HUGE_VALF;
  }
  if (flag & FLAG_NUMNEG) {
    l = -l;
  }
  if (flag & FLAG_NUMOK) {
    if (endptr) {
      *endptr = (wchar_t *)p-1;
    }
    return l;
  } else {
    if (endptr) {
      *endptr = (wchar_t *)nptr;
    }
    return 0;
  }
}

/*********************************************************************
*
*       wcstod()
*
*  Function description
*    Convert to number, double.
*
*  Parameters
*    nptr   - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted character.
*
*  Return value
*    Returns the converted value, if any. If no conversion could be
*    performed, zero is returned. If the correct value is outside the
*    range of representable values, HUGE_VAL is returned according
*    to the sign of the value, if any, and the value of the macro ERANGE
*    is stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to double representation.
*    
*    First, wcstod() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling a floating-point constant, and a final string
*    of one or more unrecognized characters, including the terminating null character
*    of the input string. wcstod() then attempts to convert the subject sequence
*    to a floating-point number, and return the result.
*    
*    The subject sequence is defined as the longest initial subsequence of the input
*    string, starting with the first non-white-space character, that is of the expected
*    form. The subject sequence contains no characters if the input string is empty
*    or consists entirely of white space, or if the first non-white-space character
*    is other than a sign or a permissible letter or digit.
*    
*    The expected form of the subject sequence is an optional plus or minus sign
*    followed by a nonempty sequence of decimal digits optionally containing a
*    decimal-point character, then an optional exponent part.
*    
*    If the subject sequence begins with a minus sign, the value resulting from
*    the conversion is negated.  A pointer to the final string is stored in the
*    object pointed to by endptr, provided that endptr is not a null pointer.
*    
*    If the subject sequence is empty or does not have the expected form, no conversion
*    is performed, the value of nptr is stored in the object pointed to by
*    endptr, provided that endptr is not a null pointer.
*
*  See also
*    wcstof()
*
*  Thread safety
*    Safe.
*/
double __SEGGER_RTL_PUBLIC_API wcstod(const wchar_t *nptr, wchar_t **endptr) {
  const wchar_t *p = nptr;
  wchar_t ch;
  int x = 0;
  double l = 0.0;
  int flag = 0;
  //
  while ((iswspace)(ch = *p++)) {
    // pass
  }
  //
  switch (ch) {
  case L'-':
    flag |= FLAG_NUMNEG;
    //FALLTHRU
  case L'+':
    ch = *p++;
  }
  //
  for (;;) {
    if (ch == L'.' && (flag & FLAG_DOTSEEN) == 0) {
      flag |= FLAG_DOTSEEN;
    } else if ((iswdigit)(ch)) {
      flag |= FLAG_NUMOK;
      l = l*10 + (ch - L'0');
      if (flag & FLAG_DOTSEEN) {
        --x;
      }
    } else {
      break;
    }
    ch = *p++;
  }
  //
  // Unread the 'e' in (e.g.) "+.e" as this isn't valid.
  //
  if ((ch == L'e' || ch == L'E') && (flag & FLAG_NUMOK)) {
    int x2 = 0;
    flag &= ~FLAG_NUMOK;
    switch (ch = *p++) {
    case L'-':
      flag |= FLAG_EXPNEG;
      //FALLTHRU
    case L'+':
      ch = *p++;
    }
    while ((iswdigit)(ch)) {
      flag |= FLAG_NUMOK;
      x2 = (x2 > 1000) ? 10000 : 10*x2 + (ch - L'0');
      ch = *p++;
    }
    if (flag & FLAG_EXPNEG) {
      x -= x2;
    } else {
      x += x2;
    }
  }
  //
  // Scale exponent.
  //
  while (x > 0) {
    l *= 10;
    --x;
  }
  while (x < 0) {
    l /= 10;
    ++x;
  }
  //
  if (isinf(l)) {
    errno = ERANGE;
    l = HUGE_VAL;
  }
  if (flag & FLAG_NUMNEG) {
    l = -l;
  }
  //
  if (flag & FLAG_NUMOK) {
    if (endptr) {
      *endptr = (wchar_t *)p-1;
    }
    return l;
  } else {
    if (endptr) {
      *endptr = (wchar_t *)nptr;
    }
    return 0;
  }
}

/*********************************************************************
*
*       wcstold()
*
*  Function description
*    Convert to number, long double.
*
*  Parameters
*    nptr   - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted character.
*
*  Return value
*    Returns the converted value, if any. If no conversion could be
*    performed, zero is returned. If the correct value is outside the
*    range of representable values, HUGE_VAL is returned according
*    to the sign of the value, if any, and the value of the macro ERANGE
*    is stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to long double representation.
*    
*    First, wcstold() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling a floating-point constant, and a final string
*    of one or more unrecognized characters, including the terminating null character
*    of the input string. wcstod() then attempts to convert the subject sequence
*    to a floating-point number, and return the result.
*    
*    The subject sequence is defined as the longest initial subsequence of the input
*    string, starting with the first non-white-space character, that is of the expected
*    form. The subject sequence contains no characters if the input string is empty
*    or consists entirely of white space, or if the first non-white-space character
*    is other than a sign or a permissible letter or digit.
*    
*    The expected form of the subject sequence is an optional plus or minus sign
*    followed by a nonempty sequence of decimal digits optionally containing a
*    decimal-point character, then an optional exponent part.
*    
*    If the subject sequence begins with a minus sign, the value resulting from
*    the conversion is negated.  A pointer to the final string is stored in the
*    object pointed to by endptr, provided that endptr is not a null pointer.
*    
*    If the subject sequence is empty or does not have the expected form, no conversion
*    is performed, the value of nptr is stored in the object pointed to by
*    endptr, provided that endptr is not a null pointer.
*
*  See also
*    wcstod()
*
*  Thread safety
*    Safe.
*/
long double __SEGGER_RTL_PUBLIC_API wcstold(const wchar_t *nptr, wchar_t **endptr) {
  return (long double)(wcstod(nptr, endptr));
}

/*********************************************************************
*
*       wcstol()
*
*  Function description
*    Convert to number, long.
*
*  Parameters
*    nptr   - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted character.
*    base   - Radix to use for conversion, 2 to 36.
*
*  Return value
*    Returns the converted value, if any. If no conversion could be
*    performed, zero is returned. If the correct value is outside
*    the range of representable values, LONG_MIN or LONG_MAX is
*    returned according to the sign of the value, if any, and the
*    value of the macro ERANGE is stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to a long representation.
*    
*    First, wcstol() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling an integer represented in some radix determined
*    by the value of base, and a final string of one or more unrecognized
*    characters, including the terminating null character of the input string.
*    wcstol() then attempts to convert the subject sequence to an integer, and
*    return the result.
*    
*    When converting, no integer suffix (such as U, L, UL, LL, ULL) is allowed.
*    
*    If the value of base is zero, the expected form of the subject sequence
*    is an optional plus or minus sign followed by an integer constant.
*    
*    If the value of base is between 2 and 36 (inclusive), the expected form
*    of the subject sequence is an optional plus or minus sign followed by a sequence
*    of letters and digits representing an integer with the radix specified by base.
*    The letters from a (or A) through z (or Z) represent the values 10 through 35;
*    only letters and digits whose ascribed values are less than that of base
*    are permitted.
*    
*    If the value of base is 16, the characters "0x" or "0X" may optionally
*    precede the sequence of letters and digits, following the optional sign.
*    
*    The subject sequence is defined as the longest initial subsequence of the input
*    string, starting with the first non-white-space character, that is of the expected
*    form. The subject sequence contains no characters if the input string is empty
*    or consists entirely of white space, or if the first non-white-space character
*    is other than a sign or a permissible letter or digit.
*    
*    If the subject sequence has the expected form and the value of base
*    is zero, the sequence of characters starting with the first digit is interpreted
*    as an integer constant. If the subject sequence has the expected form and the
*    value of base is between 2 and 36, it is used as the base for conversion.
*    
*    If the subject sequence begins with a minus sign, the value resulting from
*    the conversion is negated.
*    
*    A pointer to the final string is stored in the object pointed to by endptr,
*    provided that endptr is not a null pointer.
*    
*    If the subject sequence is empty or does not have the expected form, no conversion
*    is performed, the value of nptr is stored in the object pointed to by
*    endptr, provided that endptr is not a null pointer.
*
*  Thread safety
*    Safe.
*/
long __SEGGER_RTL_PUBLIC_API wcstol(const wchar_t *nptr, wchar_t **endptr, int base) {
  const wchar_t *unptr = nptr;
  wchar_t *endp;
  unsigned long ud;
  int neg  = 0;
  wchar_t c;
  //
  while ((c = *unptr++) != 0 && (iswspace)(c)) {
    // pass
  }
  switch (c) {
  case L'-':
    neg = 1;
  case L'+':
    break;
  default:
    --unptr;
    break;
  }
  //
  ud = __SEGGER_RTL_wcstoul((wchar_t *)unptr, &endp, base);
  if (endptr)
    *endptr = endp == (wchar_t *)unptr ? (wchar_t *)nptr : endp;
  if (neg)
    if (ud == (unsigned long)LONG_MAX+1) {
      return LONG_MIN;
    } else if (ud <= LONG_MAX) {
      return -(long)ud;
    } else {
      errno = ERANGE;
      return LONG_MIN;
    }
  else {
    if (ud <= LONG_MAX) {
      return (long)ud;
    } else {
      errno = ERANGE;
      return LONG_MAX;
    }
  }
}

/*********************************************************************
*
*       wcstoll()
*
*  Function description
*    Convert to number, long long.
*
*  Parameters
*    nptr   - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted character.
*    base   - Radix to use for conversion, 2 to 36.
*
*  Return value
*    Returns the converted value, if any. If no conversion could be
*    performed, zero is returned. If the correct value is outside
*    the range of representable values, LLONG_MIN or LLONG_MAX is
*    returned according to the sign of the value, if any, and the
*    value of the macro ERANGE is stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to a long representation.
*    
*    First, wcstoll() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling an integer represented in some radix determined
*    by the value of base, and a final string of one or more unrecognized
*    characters, including the terminating null character of the input string.
*    wcstoll() then attempts to convert the subject sequence to an integer, and
*    return the result.
*    
*    When converting, no integer suffix (such as U, L, UL, LL, ULL) is allowed.
*    
*    If the value of base is zero, the expected form of the subject sequence
*    is an optional plus or minus sign followed by an integer constant.
*    
*    If the value of base is between 2 and 36 (inclusive), the expected form
*    of the subject sequence is an optional plus or minus sign followed by a sequence
*    of letters and digits representing an integer with the radix specified by base.
*    The letters from a (or A) through z (or Z) represent the values 10 through 35;
*    only letters and digits whose ascribed values are less than that of base
*    are permitted.
*    
*    If the value of base is 16, the characters "0x" or "0X" may optionally
*    precede the sequence of letters and digits, following the optional sign.
*    
*    The subject sequence is defined as the longest initial subsequence of the input
*    string, starting with the first non-white-space character, that is of the expected
*    form. The subject sequence contains no characters if the input string is empty
*    or consists entirely of white space, or if the first non-white-space character
*    is other than a sign or a permissible letter or digit.
*    
*    If the subject sequence has the expected form and the value of base
*    is zero, the sequence of characters starting with the first digit is interpreted
*    as an integer constant. If the subject sequence has the expected form and the
*    value of base is between 2 and 36, it is used as the base for conversion.
*    
*    If the subject sequence begins with a minus sign, the value resulting from
*    the conversion is negated.
*    
*    A pointer to the final string is stored in the object pointed to by endptr,
*    provided that endptr is not a null pointer.
*    
*    If the subject sequence is empty or does not have the expected form, no conversion
*    is performed, the value of nptr is stored in the object pointed to by
*    endptr, provided that endptr is not a null pointer.
*
*  Thread safety
*    Safe.
*/
long long __SEGGER_RTL_PUBLIC_API wcstoll(const wchar_t *nptr, wchar_t **endptr, int base) {
  const wchar_t *unptr = nptr;
  wchar_t *endp;
  unsigned long long ud;

  int neg  = 0, c;
  while ((c = *unptr++) != 0 && (iswspace)(c))
    ;
  switch (c) {
  case L'-':
    neg = 1;
  case L'+':
    break;
  default:
    --unptr;
    break;
  }
  //
  ud = __SEGGER_RTL_wcstoull((wchar_t *)unptr, &endp, base);
  if (endptr)
    *endptr = endp == (wchar_t *)unptr ? (wchar_t *)nptr : endp;
  if (neg) {
    if (ud == (unsigned long long)LLONG_MAX+1) {
      return LLONG_MIN;
    } else if (ud <= LLONG_MAX) {
      return -(long long)ud;
    } else {
      errno = ERANGE;
      return LLONG_MIN;
    }
  } else {
    if (ud <= LLONG_MAX) {
      return (long long)ud;
    } else {
      errno = ERANGE;
      return LLONG_MAX;
    }
  }
}

/*********************************************************************
*
*       wcstoul()
*
*  Function description
*    Convert to number, unsigned long.
*
*  Parameters
*    nptr   - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted character.
*    base   - Radix to use for conversion, 2 to 36.
*
*  Return value
*    wcstoul() returns the converted value, if any. If no conversion could
*    be performed, zero is returned. If the correct value is outside the range of
*    representable values, ULONG_MAX is and the value of the macro ERANGE is
*    stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to a long int representation.
*    
*    First, wcstoul() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling an integer represented in some radix determined
*    by the value of base, and a final string of one or more unrecognized
*    characters, including the terminating null character of the input string. wcstoul()
*    then attempts to convert the subject sequence to an integer, and return the
*    result.
*    
*    When converting, no integer suffix (such as U, L, UL, LL, ULL) is allowed.
*    
*    If the value of base is zero, the expected form of the subject sequence
*    is an optional plus or minus sign followed by an integer constant.
*    
*    If the value of base is between 2 and 36 (inclusive), the expected form
*    of the subject sequence is an optional plus or minus sign followed by a sequence
*    of letters and digits representing an integer with the radix specified by base.
*    The letters from a (or A) through z (or Z) represent the values 10 through 35;
*    only letters and digits whose ascribed values are less than that of base
*    are permitted.
*    
*    If the value of base is 16, the characters "0x" or "0X" may optionally
*    precede the sequence of letters and digits, following the optional sign.
*    
*    The subject sequence is defined as the longest initial subsequence of the input
*    string, starting with the first non-white-space character, that is of the expected
*    form. The subject sequence contains no characters if the input string is empty
*    or consists entirely of white space, or if the first non-white-space character
*    is other than a sign or a permissible letter or digit.
*    
*    If the subject sequence has the expected form and the value of base
*    is zero, the sequence of characters starting with the first digit is interpreted
*    as an integer constant. If the subject sequence has the expected form and the
*    value of base is between 2 and 36, it is used as the base for conversion.
*    
*    If the subject sequence begins with a minus sign, the value resulting from
*    the conversion is negated.
*    
*    A pointer to the final string is stored in the object pointed to by endptr,
*    provided that endptr is not a null pointer.
*    
*    If the subject sequence is empty or does not have the expected form, no conversion
*    is performed, the value of nptr is stored in the object pointed to by
*    endptr, provided that endptr is not a null pointer.
*
*  Thread safety
*    Safe.
*/
unsigned long __SEGGER_RTL_PUBLIC_API wcstoul(const wchar_t *nptr, wchar_t **endptr, int base) {
  const wchar_t *unptr = nptr;
  unsigned long ud;
  wchar_t *endp;
  int neg = 0;
  wchar_t c;
  int saved_errno = errno;
  //
  while ((c = *unptr++) != 0 && (iswspace)(c)) {
    // pass
  }
  switch (c) {
  case L'-':
    neg = 1;
  case L'+':
    break;
  default:
    --unptr;
    break;
  }
  //
  errno = 0;
  ud = __SEGGER_RTL_wcstoul((wchar_t *)unptr, &endp, base);
  if (endptr) {
    *endptr = endp == (wchar_t *)unptr ? (wchar_t *)nptr : endp;
  }
  if (errno == ERANGE) {
    return ud;
  }
  errno = saved_errno;
  if (neg) {
    if (ud == (unsigned)LONG_MAX+1) {
      return LONG_MIN;
    } else if (ud <= LONG_MAX) {
      return -(long)ud;
    } else {
      errno = ERANGE;
      return ULONG_MAX;
    }
  } else {
    return ud;
  }
}

/*********************************************************************
*
*       wcstoull()
*
*  Function description
*    Convert to number, unsigned long long.
*
*  Parameters
*    nptr   - Pointer to string to convert from.
*    endptr - If nonnull, a pointer to object that receives
*             the pointer to the first unconverted character.
*    base   - Radix to use for conversion, 2 to 36.
*
*  Return value
*    wcstoull() returns the converted value, if any. If no conversion could
*    be performed, zero is returned. If the correct value is outside the range of
*    representable values, ULLONG_MAX is and the value of the macro ERANGE is
*    stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to a long int representation.
*    
*    First, wcstoull() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling an integer represented in some radix determined
*    by the value of base, and a final string of one or more unrecognized
*    characters, including the terminating null character of the input string. wcstoull()
*    then attempts to convert the subject sequence to an integer, and return the
*    result.
*    
*    When converting, no integer suffix (such as U, L, UL, LL, ULL) is allowed.
*    
*    If the value of base is zero, the expected form of the subject sequence
*    is an optional plus or minus sign followed by an integer constant.
*    
*    If the value of base is between 2 and 36 (inclusive), the expected form
*    of the subject sequence is an optional plus or minus sign followed by a sequence
*    of letters and digits representing an integer with the radix specified by base.
*    The letters from a (or A) through z (or Z) represent the values 10 through 35;
*    only letters and digits whose ascribed values are less than that of base
*    are permitted.
*    
*    If the value of base is 16, the characters "0x" or "0X" may optionally
*    precede the sequence of letters and digits, following the optional sign.
*    
*    The subject sequence is defined as the longest initial subsequence of the input
*    string, starting with the first non-white-space character, that is of the expected
*    form. The subject sequence contains no characters if the input string is empty
*    or consists entirely of white space, or if the first non-white-space character
*    is other than a sign or a permissible letter or digit.
*    
*    If the subject sequence has the expected form and the value of base
*    is zero, the sequence of characters starting with the first digit is interpreted
*    as an integer constant. If the subject sequence has the expected form and the
*    value of base is between 2 and 36, it is used as the base for conversion.
*    
*    If the subject sequence begins with a minus sign, the value resulting from
*    the conversion is negated.
*    
*    A pointer to the final string is stored in the object pointed to by endptr,
*    provided that endptr is not a null pointer.
*    
*    If the subject sequence is empty or does not have the expected form, no conversion
*    is performed, the value of nptr is stored in the object pointed to by
*    endptr, provided that endptr is not a null pointer.
*
*  Thread safety
*    Safe.
*/
unsigned long long __SEGGER_RTL_PUBLIC_API wcstoull(const wchar_t *nptr, wchar_t **endptr, int base) {
  const wchar_t *unptr = nptr;
  unsigned long long ud;
  wchar_t *endp;
  int neg = 0;
  wchar_t c;
  int errno_saved = errno;
  //
  while ((c = *unptr++) != 0 && (iswspace)(c)) {
    // pass
  }
  //
  switch (c) {
  case L'-':
    neg = 1;
  case L'+':
    break;
  default:
    --unptr;
    break;
  }
  //
  errno = 0;
  ud = __SEGGER_RTL_wcstoull((wchar_t *)unptr, &endp, base);
  if (endptr) {
    *endptr = endp == (wchar_t *)unptr ? (wchar_t *)nptr : endp;
  }
  if (errno == ERANGE) {
    return ud;
  }
  errno = errno_saved;
  if (neg) {
    if (ud == (unsigned long long)LLONG_MAX+1) {
      return (unsigned long long)LLONG_MIN;
    } else if (ud <= LLONG_MAX) {
      return -(long long)ud;
    } else {
      errno = ERANGE;
      return ULLONG_MAX;
    }
  } else {
    return ud;
  }
}

/*************************** End of file ****************************/
