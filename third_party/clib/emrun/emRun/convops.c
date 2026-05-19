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
#include "ctype.h"
#include "stdlib.h"
#include "stddef.h"
#include "errno.h"
#include "math.h"
#include "limits.h"
#include "float.h"

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
*       __SEGGER_RTL_xtoa()
*
*  Function description
*    Convert to string, int.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to object that receives the converted number.
*    radix - Radix for conversion, 10 for decimal, 16 for hexadecimal.
*    neg   - Flag, value is negative.
*
*  Return value
*    Returns the pointer buf.
*/
static char * __SEGGER_RTL_xtoa(unsigned val, char *buf, unsigned radix, int neg) {
  char *p, *q;
  //
  p = buf;
  if (neg) {
    *p++ = '-';
    val = ~val + 1;
  }
  q = p;
  //
  // Need at least one digit in the buffer.
  //
  do {
    unsigned digit = (unsigned)(val % radix);
    val /= radix;
    *p++ = digit > 9 ? (digit - 10 + 'a') : (digit + '0');
  } while (val > 0);
  //
  // Reverse order.
  //
  *p-- = '\0';
  do {
    char temp = *p;
    *p-- = *q;
    *q++ = temp;
  } while (q < p);
  //
  return buf;
}

/*********************************************************************
*
*       __SEGGER_RTL_xltoa()
*
*  Function description
*    Convert to string, long.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to object that receives the converted number.
*    radix - Radix for conversion, 10 for decimal, 16 for hexadecimal.
*    neg   - Flag, value is negative.
*
*  Return value
*    Returns the pointer buf.
*/
static char * __SEGGER_RTL_xltoa(unsigned long val, char *buf, unsigned radix, int neg) {
  char *p, *q;
  //
  p = buf;
  if (neg) {
    *p++ = '-';
    val = ~val + 1;
  }
  q = p;
  //
  // Need at least one digit in the buffer.
  //
  do {
    unsigned digit = (unsigned)(val % radix);
    val /= radix;
    *p++ = digit > 9 ? (digit - 10 + 'a') : (digit + '0');
  } 
  while (val > 0);
  //
  // Reverse order.
  //
  *p-- = '\0';
  do {
    char temp = *p;
    *p-- = *q;
    *q++ = temp;
  } while (q < p);
  //
  return buf;
}

/*********************************************************************
*
*       __SEGGER_RTL_xlltoa()
*
*  Function description
*    Convert to string, long long.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to object that receives the converted number.
*    radix - Radix for conversion, 10 for decimal, 16 for hexadecimal.
*    neg   - Flag, value is negative.
*
*  Return value
*    Returns the pointer buf.
*/
static char * __SEGGER_RTL_xlltoa(unsigned long long val, char *buf, unsigned radix, int neg) {
  char *p, *q;
  //
  p = buf;
  if (neg) {
    *p++ = '-';
    val = ~val + 1;
  }
  q = p;
  //
  // Need at least one digit in the buffer.
  //
  do {
    unsigned digit = (unsigned)(val % radix);
    val /= radix;
    *p++ = digit > 9 ? (digit - 10 + 'a') : (digit + '0');
  } while (val > 0);
  //
  // Reverse order.
  //
  *p-- = '\0';
  do {
    char temp = *p;
    *p-- = *q;
    *q++ = temp;
  } while (q < p);
  //
  return buf;
}

/*********************************************************************
*
*       __SEGGER_RTL_strtoul()
*
*  Function description
*    Convert to number, long.
*
*  Parameters
*    nsptr  - Pointer to string to convert from.
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
*/
static unsigned long __SEGGER_RTL_strtoul(const char *nsptr, char **endptr, int base) {
  const unsigned char *nptr = (const unsigned char *)nsptr;
  int c, ok, overflowed, digit;
  unsigned long dhigh, dlow;
  //
  // Not successful yet, nor overflown.
  //
  ok = 0;
  overflowed = 0;
  //
  // Skip initial spaces.
  //
  while ((c = *nptr++) != 0 && (isspace)(c)) {
    // pass
  }
  //
  // Leading zero needs to be treated as, perhaps, octal or hex.
  //
  if (c == '0') {
    //
    // Say this is ok for now.
    //
    ok = 1;
    c = *nptr++;
    if (c == 'x' || c == 'X') {
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
  while ((digit = __SEGGER_RTL_digit(c, base)) >= 0) {
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
    *endptr = ok ? (char *)nptr-1 : (char *)nsptr;
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
*       __SEGGER_RTL_strtoull()
*
*  Function description
*    Convert to number, long long.
*
*  Parameters
*    nsptr  - Pointer to string to convert from.
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
*/
static unsigned long long __SEGGER_RTL_strtoull(const char *nsptr, char **endptr, int base) {
  const unsigned char *nptr = (const unsigned char *)nsptr;
  int c, ok, overflowed, digit;
  unsigned long long dhigh, dlow;
  //
  // Not successful yet, nor overflown.
  //
  ok = 0;
  overflowed = 0;
  //
  // Skip initial spaces.
  //
  while ((c = *nptr++) != 0 && (isspace)(c)) {
    // pass
  }
  //
  // Leading zero needs to be treated as, perhaps, octal or hex.
  //
  if (c == '0') {
    //
    // Say this is ok for now.
    //
    ok = 1;
    c = *nptr++;
    if (c == 'x' || c == 'X') {
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
  while ((digit = __SEGGER_RTL_digit(c, base)) >= 0) {
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
    *endptr = ok ? (char *)nptr-1 : (char *)nsptr;
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
*       atof()
*
*  Function description
*    Convert to number, double.
*
*  Parameters
*    nptr - Pointer to string to convert from.
*
*  Return value
*    Returns the converted value, if any.  If the value of the
*    result cannot be represented, the behavior is undefined.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to an double representation.
*    
*    atof() does not affect the value of errno on an error.
*    
*  Notes
*    Except for the behavior on error, atof() is equivalent to
*    (int)strtod(nptr, NULL).
*
*  Thread safety
*    Safe.
*
*  See also
*    strtod()
*/
double __SEGGER_RTL_PUBLIC_API atof(const char *nptr) {
  double   val  = 0;
  int      exp  = 0;
  unsigned sign = 0;
  //
  while ((isspace)(*nptr)) {
    ++nptr;
  }
  //
  if (*nptr == '-') {
    ++nptr;
    sign = 1;
  } else if (*nptr == '+') {
    ++nptr;
  }
  //
  while ((isdigit)(*nptr)) {
    val = val * 10 + (*nptr++ - '0');
  }
  //
  if (*nptr == '.') {
    while ((isdigit)(*++nptr)) {
      val = val * 10 + (*nptr - '0');
      --exp;
    }
  }
  //
  if (*nptr == 'e' || *nptr == 'E') {
    exp += atoi(++nptr);
  }
  //
  if (val != 0) {
    //
    // Limit exponent.
    //
    if (exp > 2*LDBL_MAX_10_EXP) {
      exp = 2*LDBL_MAX_10_EXP;
    }
    if (exp < -2*LDBL_MAX_10_EXP) {
      exp = -2*LDBL_MAX_10_EXP;
    }
    while (exp < 0) {
      val /= 10;
      ++exp;
    }
    while (exp > 0) {
      val *= 10;
      --exp;
    }
  }
  //
  return sign ? -val : val;
}

/*********************************************************************
*
*       atoi()
*
*  Function description
*    Convert to number, int.
*
*  Parameters
*    nptr - Pointer to string to convert from.
*
*  Return value
*    Returns the converted value, if any.  If the value of the
*    result cannot be represented, the behavior is undefined.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to an int representation.
*    
*    atoi() does not affect the value of errno on an error.
*    
*  Notes
*    Except for the behavior on error, atoi() is equivalent to
*    (int)strtol(nptr, NULL, 10).
*
*  Thread safety
*    Safe.
*
*  See also
*    strtol()
*/
int __SEGGER_RTL_PUBLIC_API atoi(const char *nptr) {
  //
  // Standard requires we not change errno.
  //
  int save_errno = errno;
  int res = (int)strtol(nptr, (char **)NULL, 10);
  errno = save_errno;
  return res;
}

/*********************************************************************
*
*       atol()
*
*  Function description
*    Convert to number, long.
*
*  Parameters
*    nptr - Pointer to string to convert from.
*
*  Return value
*    Returns the converted value, if any.  If the value of the
*    result cannot be represented, the behavior is undefined.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to a long representation.
*    
*    atol() does not affect the value of errno on an error.
*    
*  Notes
*    Except for the behavior on error, atol() is equivalent to
*    strtol(nptr, NULL, 10).
*
*  Thread safety
*    Safe.
*
*  See also
*    strtol()
*/
long int __SEGGER_RTL_PUBLIC_API atol(const char *nptr) {
  //
  // Standard requires we not change errno.
  //
  int save_errno = errno;
  long int res = strtol(nptr, (char **)NULL, 10);
  errno = save_errno;
  return res;
}

/*********************************************************************
*
*       atoll()
*
*  Function description
*    Convert to number, long long.
*
*  Parameters
*    nptr - Pointer to string to convert from.
*
*  Return value
*    Returns the converted value, if any.  If the value of the
*    result cannot be represented, the behavior is undefined.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to a long-long representation.
*    
*    atoll() does not affect the value of errno on an error.
*    
*  Notes
*    Except for the behavior on error, atoll() is equivalent to
*    strtoll(nptr, NULL, 10).
*
*  Thread safety
*    Safe.
*
*  See also
*    strtoll()
*/
long long int __SEGGER_RTL_PUBLIC_API atoll(const char *nptr) {
  //
  // Standard requires we not change errno.
  //
  int save_errno = errno;
  long long int res = strtoll(nptr, (char **)NULL, 10);
  errno = save_errno;
  return res;
}

/*********************************************************************
*
*       strtof()
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
*    First, strtof() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling a floating-point constant, and a final string
*    of one or more unrecognized characters, including the terminating null character
*    of the input string. strtof() then attempts to convert the subject sequence
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
*  Thread safety
*    Safe.
*
*  See also
*    strtod()
*/
float __SEGGER_RTL_PUBLIC_API strtof(const char *nptr, char **endptr) {
  const char *p = nptr;
  int ch, x = 0;
  float l = 0.0;
  int flag = 0;
  //
  while ((isspace)(ch = *p++)) {
    // pass
  }
  //
  switch (ch) {
  case '-':
    flag |= FLAG_NUMNEG;
    //FALLTHRU
  case '+':
    ch = *p++;
  }
  //
  for (;;) {
    if (ch == '.' && (flag & FLAG_DOTSEEN) == 0) {
      flag |= FLAG_DOTSEEN;
    } else if ((isdigit)(ch)) {
      flag |= FLAG_NUMOK;
      l = l*10 + (ch - '0');
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
  if ((ch == 'e' || ch == 'E') && (flag & FLAG_NUMOK)) {
    int x2 = 0;
    flag &= ~FLAG_NUMOK;
    switch (ch = *p++) {
    case '-':
      flag |= FLAG_EXPNEG;
      //FALLTHRU
    case '+':
      ch = *p++;
    }
    while ((isdigit)(ch)) {
      flag |= FLAG_NUMOK;
      x2 = (x2 > 1000) ? 10000 : 10*x2 + (ch - '0');
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
      *endptr = (char *)p-1;
    }
    return l;
  } else {
    if (endptr) {
      *endptr = (char *)nptr;
    }
    return 0;
  }
}

/*********************************************************************
*
*       strtod()
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
*    First, strtod() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling a floating-point constant, and a final string
*    of one or more unrecognized characters, including the terminating null character
*    of the input string. strtod() then attempts to convert the subject sequence
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
*  Thread safety
*    Safe.
*
*  See also
*    strtof()
*/
double __SEGGER_RTL_PUBLIC_API strtod(const char *nptr, char **endptr) {
  const char *p = nptr;
  int ch, x = 0;
  double l = 0.0;
  int flag = 0;
  //
  while ((isspace)(ch = *p++)) {
    // pass
  }
  //
  switch (ch) {
  case '-':
    flag |= FLAG_NUMNEG;
    //FALLTHRU
  case '+':
    ch = *p++;
  }
  //
  for (;;) {
    if (ch == '.' && (flag & FLAG_DOTSEEN) == 0) {
      flag |= FLAG_DOTSEEN;
    } else if ((isdigit)(ch)) {
      flag |= FLAG_NUMOK;
      l = l*10 + (ch - '0');
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
  if ((ch == 'e' || ch == 'E') && (flag & FLAG_NUMOK)) {
    int x2 = 0;
    flag &= ~FLAG_NUMOK;
    switch (ch = *p++) {
    case '-':
      flag |= FLAG_EXPNEG;
      //FALLTHRU
    case '+':
      ch = *p++;
    }
    while ((isdigit)(ch)) {
      flag |= FLAG_NUMOK;
      x2 = (x2 > 1000) ? 10000 : 10*x2 + (ch - '0');
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
      *endptr = (char *)p-1;
    }
    return l;
  } else {
    if (endptr) {
      *endptr = (char *)nptr;
    }
    return 0;
  }
}

/*********************************************************************
*
*       strtold()
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
*    First, strtold() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling a floating-point constant, and a final string
*    of one or more unrecognized characters, including the terminating null character
*    of the input string. strtod() then attempts to convert the subject sequence
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
*  Thread safety
*    Safe.
*
*  See also
*    strtod()
*/
long double __SEGGER_RTL_PUBLIC_API strtold(const char *nptr, char **endptr) {
  return (long double)(strtod(nptr, endptr));
}

/*********************************************************************
*
*       strtol()
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
*    First, strtol() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling an integer represented in some radix determined
*    by the value of base, and a final string of one or more unrecognized
*    characters, including the terminating null character of the input string.
*    strtol() then attempts to convert the subject sequence to an integer, and
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
long __SEGGER_RTL_PUBLIC_API strtol(const char *nptr, char **endptr, int base) {
  const unsigned char *unptr = (const unsigned char *)nptr;
  char *endp;
  unsigned long ud;
  int neg;
  int c;
  //
  while ((c = *unptr++) != 0 && (isspace)(c)) {
    // pass
  }
  neg = 0;
  switch (c) {
  case '-':
    neg = 1;
  case '+':
    break;
  default:
    --unptr;
    break;
  }
  //
  ud = __SEGGER_RTL_strtoul((char *)unptr, &endp, base);
  if (endptr) {
    *endptr = endp == (char *)unptr ? (char *)nptr : endp;
  }
  if (neg) {
    if (ud == (unsigned long)LONG_MAX+1) {
      return LONG_MIN;
    } else if (ud <= LONG_MAX) {
      return -(long)ud;
    } else {
      errno = ERANGE;
      return LONG_MIN;
    }
  } else {
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
*       strtoll()
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
*    First, strtoll() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling an integer represented in some radix determined
*    by the value of base, and a final string of one or more unrecognized
*    characters, including the terminating null character of the input string.
*    strtoll() then attempts to convert the subject sequence to an integer, and
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
long long __SEGGER_RTL_PUBLIC_API strtoll(const char *nptr, char **endptr, int base) {
  const unsigned char *unptr = (const unsigned char *)nptr;
  char *endp;
  unsigned long long ud;

  int neg  = 0, c;
  while ((c = *unptr++) != 0 && (isspace)(c))
    ;
  switch (c) {
  case '-':
    neg = 1;
  case '+':
    break;
  default:
    --unptr;
    break;
  }
  //
  ud = __SEGGER_RTL_strtoull((char *)unptr, &endp, base);
  if (endptr) {
    *endptr = endp == (char *)unptr ? (char *)nptr : endp;
  }
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
*       strtoul()
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
*    strtoul() returns the converted value, if any. If no conversion could
*    be performed, zero is returned. If the correct value is outside the range of
*    representable values, ULONG_MAX is and the value of the macro ERANGE is
*    stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to a long int representation.
*    
*    First, strtoul() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling an integer represented in some radix determined
*    by the value of base, and a final string of one or more unrecognized
*    characters, including the terminating null character of the input string. strtoul()
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
unsigned long __SEGGER_RTL_PUBLIC_API strtoul(const char *nptr, char **endptr, int base) {
  const unsigned char *unptr = (const unsigned char *)nptr;
  unsigned long ud;
  char *endp;
  int neg = 0, c;
  int saved_errno = errno;
  //
  while ((c = *unptr++) != 0 && (isspace)(c)) {
    // pass
  }
  switch (c) {
  case '-':
    neg = 1;
  case '+':
    break;
  default:
    --unptr;
    break;
  }
  //
  errno = 0;
  ud = __SEGGER_RTL_strtoul((char *)unptr, &endp, base);
  if (endptr) {
    *endptr = endp == (char *)unptr ? (char *)nptr : endp;
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
*       strtoull()
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
*    strtoull() returns the converted value, if any. If no conversion could
*    be performed, zero is returned. If the correct value is outside the range of
*    representable values, ULLONG_MAX is and the value of the macro ERANGE is
*    stored in errno.
*
*  Additional information
*    Converts the initial portion of the string pointed to by nptr
*    to a long int representation.
*    
*    First, strtoull() decomposes the input string into three parts: an initial,
*    possibly empty, sequence of white-space characters, as specified by isspace(),
*    a subject sequence resembling an integer represented in some radix determined
*    by the value of base, and a final string of one or more unrecognized
*    characters, including the terminating null character of the input string. strtoull()
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
unsigned long long __SEGGER_RTL_PUBLIC_API strtoull(const char *nptr, char **endptr, int base) {
  const unsigned char *unptr = (const unsigned char *)nptr;
  unsigned long long ud;
  char *endp;
  int neg = 0, c;
  int errno_saved = errno;
  //
  while ((c = *unptr++) != 0 && (isspace)(c)) {
    // pass
  }
  //
  switch (c) {
  case '-':
    neg = 1;
  case '+':
    break;
  default:
    --unptr;
    break;
  }
  //
  errno = 0;
  ud = __SEGGER_RTL_strtoull((char *)unptr, &endp, base);
  if (endptr) {
    *endptr = endp == (char *)unptr ? (char *)nptr : endp;
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

/*********************************************************************
*
*       itoa()
*
*  Function description
*    Convert to string, int.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to array of characters that receives the string.
*    radix - Number base to use for conversion, 2 to 36.
*
*  Return value
*    Returns buf.
*
*  Additional information
*    Converts val to a string in base radix and places the result in
*    buf which must be large enough to hold the output. If radix is
*    greater than 36, the result is undefined.
*
*    If val is negative and radix is 10, the string has a leading
*    minus sign (-); for all other values of radix, value is considered
*    unsigned and never has a leading minus sign.
*
*  Notes
*    This is a non-standard function. Even though this function is
*    commonly used by compilers on other platforms, there is no
*    guarantee that this function will behave the same on all
*    platforms, in all cases.
*
*  Thread safety
*    Safe.
*
*  See also
*    ltoa(), lltoa(), utoa(), ultoa(), ulltoa()
*/
char * __SEGGER_RTL_PUBLIC_API itoa(int val, char *buf, int radix) {
  if (radix == 10 && val < 0) {
    return __SEGGER_RTL_xtoa((unsigned)val, buf, radix, 1);
  } else {
    return __SEGGER_RTL_xtoa((unsigned)val, buf, radix, 0);
  }
}

/*********************************************************************
*
*       ltoa()
*
*  Function description
*    Convert to string, long.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to array of characters that receives the string.
*    radix - Number base to use for conversion, 2 to 36.
*
*  Return value
*    Returns buf.
*
*  Additional information
*    Converts val to a string in base radix and places the result in
*    buf which must be large enough to hold the output. If radix is
*    greater than 36, the result is undefined.
*
*    If val is negative and radix is 10, the string has a leading
*    minus sign (-); for all other values of radix, value is considered
*    unsigned and never has a leading minus sign.
*
*  Notes
*    This is a non-standard function. Even though this function is
*    commonly used by compilers on other platforms, there is no
*    guarantee that this function will behave the same on all
*    platforms, in all cases.
*
*  Thread safety
*    Safe.
*
*  See also
*    itoa(), lltoa(), utoa(), ultoa(), ulltoa()
*/
char * __SEGGER_RTL_PUBLIC_API ltoa(long val, char *buf, int radix) {
  return __SEGGER_RTL_xltoa((unsigned long)val, buf, radix, (radix == 10 && val < 0));
}

/*********************************************************************
*
*       lltoa()
*
*  Function description
*    Convert to string, long long.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to array of characters that receives the string.
*    radix - Number base to use for conversion, 2 to 36.
*
*  Return value
*    Returns buf.
*
*  Additional information
*    Converts val to a string in base radix and places the result in
*    buf which must be large enough to hold the output. If radix is
*    greater than 36, the result is undefined.
*
*    If val is negative and radix is 10, the string has a leading
*    minus sign (-); for all other values of radix, value is considered
*    unsigned and never has a leading minus sign.
*
*  Notes
*    This is a non-standard function. Even though this function is
*    commonly used by compilers on other platforms, there is no
*    guarantee that this function will behave the same on all
*    platforms, in all cases.
*
*  Thread safety
*    Safe.
*
*  See also
*    itoa(), ltoa(), utoa(), ultoa(), ulltoa()
*/
char * __SEGGER_RTL_PUBLIC_API lltoa(long long val, char *buf, int radix) {
  return __SEGGER_RTL_xlltoa((unsigned long long)val, buf, radix, (radix == 10 && val < 0));
}

/*********************************************************************
*
*       utoa()
*
*  Function description
*    Convert to string, unsigned.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to array of characters that receives the string.
*    radix - Number base to use for conversion, 2 to 36.
*
*  Return value
*    Returns buf.
*
*  Additional information
*    Converts val to a string in base radix and places the result in
*    buf which must be large enough to hold the output. If radix is
*    greater than 36, the result is undefined.
*
*  Notes
*    This is a non-standard function. Even though this function is
*    commonly used by compilers on other platforms, there is no
*    guarantee that this function will behave the same on all
*    platforms, in all cases.
*
*  Thread safety
*    Safe.
*
*  See also
*    itoa(), ltoa(), lltoa(), ultoa(), ulltoa()
*/
char * __SEGGER_RTL_PUBLIC_API utoa(unsigned val, char *buf, int radix) {
  return __SEGGER_RTL_xtoa(val, buf, radix, 0);
}

/*********************************************************************
*
*       ultoa()
*
*  Function description
*    Convert to string, unsigned long.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to array of characters that receives the string.
*    radix - Number base to use for conversion, 2 to 36.
*
*  Return value
*    Returns buf.
*
*  Additional information
*    Converts val to a string in base radix and places the result in
*    buf which must be large enough to hold the output. If radix is
*    greater than 36, the result is undefined.
*
*  Notes
*    This is a non-standard function. Even though this function is
*    commonly used by compilers on other platforms, there is no
*    guarantee that this function will behave the same on all
*    platforms, in all cases.
*
*  Thread safety
*    Safe.
*
*  See also
*    itoa(), ltoa(), lltoa(), ulltoa(), utoa()
*/
char * __SEGGER_RTL_PUBLIC_API ultoa(unsigned long val, char *buf, int radix) {
  return __SEGGER_RTL_xltoa(val, buf, radix, 0);
}

/*********************************************************************
*
*       ulltoa()
*
*  Function description
*    Convert to string, unsigned long long.
*
*  Parameters
*    val   - Value to convert.
*    buf   - Pointer to array of characters that receives the string.
*    radix - Number base to use for conversion, 2 to 36.
*
*  Return value
*    Returns buf.
*
*  Additional information
*    Converts val to a string in base radix and places the result in
*    buf which must be large enough to hold the output. If radix is
*    greater than 36, the result is undefined.
*
*  See also
*    itoa(), ltoa(), lltoa(), ultoa(), utoa()
*
*  Thread safety
*    Safe.
*
*  Notes
*    This is a non-standard function. Even though this function is
*    commonly used by compilers on other platforms, there is no
*    guarantee that this function will behave the same on all
*    platforms, in all cases.
*/
char * __SEGGER_RTL_PUBLIC_API ulltoa(unsigned long long val, char *buf, int radix) {
  return __SEGGER_RTL_xlltoa(val, buf, radix, 0);
}

/*************************** End of file ****************************/
