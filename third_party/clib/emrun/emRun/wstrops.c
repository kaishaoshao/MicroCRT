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

#include "wchar.h"
#include "wctype.h"
#include "stdlib.h"
#include "string.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       wcsstr()
*
*  Function description
*    Find string within string, forward.
*
*  Parameters
*    s1 - Pointer to wide string to search.
*    s2 - Pointer to wide string to search for.
*
*  Return value
*    Returns a pointer to the located wide string, or a null pointer if 
*    the wide string is not found. If s2 points to a wide string with zero
*    length, wcsstr() returns s1.
*
*  Additional information
*    Locates the first occurrence in the wide string pointed to by s1
*    of the sequence of wide characters (excluding the terminating null
*    wide character) in the wide string pointed to by s2.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcsstr(const wchar_t *s1, const wchar_t *s2) {
  const wchar_t *p;
  //
  if (*s2 == 0) {
    return (wchar_t *)s1;  // cast away const
  }
  //
  for (p = s1; *p; ++p) {
    const wchar_t *ps1 = p;
    const wchar_t *ps2 = s2;
    //
    while (*ps1 && *ps2 && *ps1 == *ps2) {
      ++ps1;
      ++ps2;
    }
    //
    if (*ps2 == 0) {
      return (wchar_t *)p;  // cast away const
    }
  }
  //
  return 0;
}

/*********************************************************************
*
*       wcsspn()
*
*  Function description
*    Compute size of string prefixed by a set of characters.
*
*  Parameters
*    s1 - Pointer to zero-terminated wide string to search.
*    s2 - Pointer to zero-terminated acceptable-set wide string.
*
*  Return value
*    Returns the length of the wide string pointed to by s1 which
*    consists entirely of wide characters from the wide string
*    pointed to by s2
*
*  Additional information
*    Computes the length of the maximum initial segment of the wide string 
*    pointed to by s1 which consists entirely of wide characters from
*    the string pointed to by s2.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcsspn(const wchar_t *s1, const wchar_t *s2) {
  const wchar_t *p;
  //
  p = s1;
  while (*p && wcschr(s2, *p)) {
    p++;
  }
  return p - s1;
}

/*********************************************************************
*
*       wcssep()
*
*  Function description
*    Break string into tokens (BSD).
*
*  Parameters
*    stringp - Pointer to pointer to zero-terminated wide string.
*    delim   - Pointer to delimiter set wide string.
*
*  Return value
*    See below.
*
*  Additional information
*    Locates, in the wide string referenced by *stringp, the first
*    occurrence of any wide character in the wide string delim (or the
*    terminating null character) and replaces it with a null
*    wide character.  The location of the next wide character after the
*    delimiter wide character (or NULL, if the end of the wide string was
*    reached) is stored in *stringp. The original value
*    of *stringp is returned. 
*
*    An empty field (that is, a wide character in the string delim occurs as the
*    first character of *stringp) can be detected by comparing the location
*    referenced by the returned pointer to the null wide character.
*
*    If *stringp is initially null, wcssep() returns null.
*
*  Notes
*    Commonly found in Linux and BSD C libraries.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcssep(wchar_t **stringp, const wchar_t *delim) {
  wchar_t *res;
  //
  // Degenerate simple cases.
  //
  if (stringp == 0 || *stringp == 0 || **stringp == 0) {
    return 0;
  }
  //
  // Remember start of token.
  //
  res = *stringp;
  while (**stringp && !wcschr(delim, **stringp)) {
    ++(*stringp);
  }
  //
  // Fix delimiter.
  //
  if (**stringp) {
    **stringp = 0;
    ++(*stringp);
  }
  //
  // Return start of token.
  //
  return res;
}

/*********************************************************************
*
*       wcscat()
*
*  Function description
*    Concatenate strings.
*
*  Parameters
*    s1 - Zero-terminated wide string to append to.
*    s2 - Zero-terminated wide string to append.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Appends a copy of the wide string pointed to by s2 (including 
*    the terminating null wide character) to the end of the wide string
*    pointed to by s1.  The initial character of s2 overwrites the null
*    wide character at the end of s1. The behavior of wcscat() is undefined
*    if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcscat(wchar_t *s1, const wchar_t *s2) {
  wchar_t *p = s1;
  //
  // Find end of dst.
  //
  while (*p)
    ++p;
  //
  // Append src.
  //
  while ((*p++ = *s2++))
    ;
  //
  return s1;
}

/*********************************************************************
*
*       wcschr()
*
*  Function description
*    Find character within string, forward.
*
*  Parameters
*    s - Wide string to search.
*    c - Wide character to search for.
*
*  Return value
*    Returns a pointer to the located wide character, or a null pointer 
*    if c does not occur in the wide string.
*
*  Additional information
*    Locates the first occurrence of c in the wide string pointed to
*    by s. The terminating wide null character is considered to be
*    part of the string.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcschr(const wchar_t *s, wchar_t c) {
  while (*s && *s != c) {
    s++;
  }
  //
  return *s == c ? (wchar_t *)s : 0;
}

/*********************************************************************
*
*       wcscmp()
*
*  Function description
*    Compare strings.
*
*  Parameters
*    s1 - Pointer to wide string #1.
*    s2 - Pointer to wide string #2.
*
*  Return value
*    Returns an integer greater than, equal to, or less than zero, 
*    if the null-terminated wide string pointed to by s1 is greater than, 
*    equal to, or less than the null-terminated wide string pointed to
*    by s2.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API wcscmp(const wchar_t *s1, const wchar_t *s2) {
  int ret;
  //
  while ((ret = *s1 - *s2) == 0 && *s2) {
    ++s1;
    ++s2;
  }
  return ret;
}

/*********************************************************************
*
*       wcscoll()
*
*  Function description
*    Collate strings.
*
*  Parameters
*    s1 - Pointer to wide string #1.
*    s2 - Pointer to wide string #2.
*
*  Return value
*    Returns an integer greater than, equal to, or less than zero, 
*    if the null-terminated wide string pointed to by s1 is greater than, 
*    equal to, or less than the null-terminated wide string pointed to
*    by s2.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API wcscoll(const wchar_t *s1, const wchar_t *s2) {
  return wcscmp(s1, s2);
}

/*********************************************************************
*
*       wcsxfrm()
*
*  Function description
*    Transform strings.
*
*  Parameters
*    s1 - Pointer to destination array.
*    s2 - Pointer to source string.
*    n  - Maximum number of characters in the destination array.
*
*  Return value
*    Returns the length of the transformed string.  If the value
*    returned is n or more, the contents of the array pointed to
*    by s1 are undefined.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcsxfrm(wchar_t *s1, const wchar_t *s2, size_t n) {
  wcsncpy(s1, s2, n);
  if (n) {
    s1[n-1] = L'\0';
  }
  return n;
}

/*********************************************************************
*
*       wcscpy()
*
*  Function description
*    Copy string.
*
*  Parameters
*    s1 - Pointer to wide string to copy to.
*    s2 - Pointer to wide string to copy.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Copies the wide string pointed to by s2 (including the terminating 
*    null wide character) into the array pointed to by s1. The behavior of
*    wcscpy() is undefined if copying takes place between objects that
*    overlap.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcscpy(wchar_t *s1, const wchar_t *s2) {
  wchar_t *p = s1;
  //
  // Copy src to dst.
  //
  while ((*p++ = *s2++) != 0) {
    // pass
  }
  //
  // Return original dst.
  //
  return s1;
}

/*********************************************************************
*
*       wcscspn()
*
*  Function description
*    Compute size of string not prefixed by a set of characters.
*
*  Parameters
*    s1 - Pointer to wide string to search.
*    s2 - Pointer to wide string containing characters to skip.
*
*  Return value
*    Returns the length of the segment of wide string s1 prefixed
*    by wide characters from s2.
*
*  Additional information
*    Computes the length of the maximum initial segment of the wide string 
*    pointed to by s1 which consists entirely of wide characters not from
*    the wide string pointed to by s2.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcscspn(const wchar_t *s1, const wchar_t *s2) {
  const wchar_t *p;
  //
  p = s1;
  while (*p && wcschr(s2, *p) == 0) {
    ++p;
  }
  return p - s1;
}

/*********************************************************************
*
*       wcsdup()
*
*  Function description
*    Duplicate string (POSIX.1).
*
*  Parameters
*    s - Pointer to wide string to duplicate.
*
*  Return value
*    Returns a pointer to the new wide string or a null pointer if the
*    new wide string cannot be created.  The returned pointer can be passed
*    to free().
*
*  Additional information
*    Duplicates the wide string pointed to by s by using malloc() to
*    allocate memory for a copy of s and then copies s, including the
*    terminating null, to that memory
*
*  Notes
*    Conforms to POSIX.1-2008 and SC22 TR 24731-2.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcsdup(const wchar_t *s) {
  wchar_t *result;
  //
  result = (wchar_t *)malloc((wcslen(s) + 1) * sizeof(wchar_t));
  if (result) {
    wcscpy(result, s);
  }
  return result;
}

/*********************************************************************
*
*       wcsndup()
*
*  Function description
*    Duplicate string, limit length (GNU).
*
*  Parameters
*    s - Pointer to wide string to duplicate.
*    n - Maximum number of wide characters to duplicate.
*
*  Return value
*    Returns a pointer to the new wide string or a null pointer if the
*    new wide string cannot be created.  The returned pointer can be
*    passed to free().
*
*  Additional information
*    Duplicates at most n wide characters from the the string pointed to
*    by s by using malloc() to allocate memory for a copy of s.
*    
*    If the length of string pointed to by s is greater than n wide characters,
*    only n wide characters will be duplicated.  If n is greater than the length of
*    the wide string pointed to by s, all characters in the string are copied into the
*    allocated array including the terminating null character.
*
*  Notes
*    This is a GNU extension.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcsndup(const wchar_t *s, size_t n) {
  size_t    max;
  wchar_t * result;
  //
  max = wcsnlen(s, n);
  result = (wchar_t *)malloc(max + 1);
  if (result) {
    (memcpy)(result, s, max);
    result[max] = 0;
  }
  return result;
}

/*********************************************************************
*
*       wcslen()
*
*  Function description
*    Calculate length of string.
*
*  Parameters
*    s - Pointer to zero-terminated wide string.
*
*  Return value
*    Returns the length of the wide string pointed to by s, that 
*    is the number of wide characters that precede the terminating
*    wide null character.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcslen(const wchar_t *s) {
  const wchar_t *sos;
  //
  // Find end of string.
  //
  sos = s;
  while (*s) {
    ++s;
  }
  //
  // Compute and return length.
  //
  return s-sos;
}

/*********************************************************************
*
*       wcsnstr()
*
*  Function description
*    Find string within string, forward, limit length (BSD).
*
*  Parameters
*    s1 - Pointer to wide string to search.
*    s2 - Pointer to wide string to search for.
*    n  - Maximum number of characters to search for.
*
*  Return value
*    Returns a pointer to the located wide string, or a null pointer if 
*    the wide string is not found. If s2 points to a wide string with zero
*    length, wcsnstr() returns s1.
*
*  Additional information
*    Searches at most n wide characters to locate the first occurrence
*    in the wide string pointed to by s1 of the sequence of wide characters
*    (excluding the terminating wide null character) in the string pointed
*    to by s2.
*
*  Notes
*    Commonly found in Linux and BSD C libraries.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcsnstr(const wchar_t *s1, const wchar_t *s2, size_t n) {
  size_t snippetLen;
  //
  // Handle degenerate case quickly.
  //
  snippetLen = wcslen(s2);
  if (snippetLen == 0) {
    return (wchar_t *)s1;  // cast away const
  }
  //
  // Iterate over string.
  //
  while (n >= snippetLen) {
    //
    // Find next potential match.
    //
    wchar_t *fnd = wcsnchr(s1, n, s2[0]);
    if (fnd == 0) {
      return 0;
    }
    //
    // Did we match here?
    //
    if (wcsncmp(s2, fnd, snippetLen) == 0) {
      return fnd;
    }
    //
    // No match, step over the gap.
    //
    n -= (fnd - s1) + 1;
    s1 = fnd + 1;
  }
  //
  // No matches at all...
  //
  return 0;
}

/*********************************************************************
*
*       wcspbrk()
*
*  Function description
*    Find first occurrence of characters within string.
*
*  Parameters
*    s1 - Pointer to wide string to search.
*    s2 - Pointer to wide string to search for.
*
*  Return value
*    Returns a pointer to the first wide character, or a null pointer if
*    no wide character from s2 occurs in s1.
*
*  Additional information
*    Locates the first occurrence in the wide string pointed to by s1
*    of any wide character from the string pointed to by s2.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcspbrk(const wchar_t *s1, const wchar_t *s2) {
  const wchar_t *p;
  //
  p = s1;
  while (*p && wcschr(s2, *p) == 0) {
    ++p;
  }
  return *p ? (wchar_t *)p : 0;
}

/*********************************************************************
*
*       wcsrchr()
*
*  Function description
*    Find character within string, reverse.
*
*  Parameters
*    s - Pointer to wide string to search.
*    c - Wide character to search for.
*
*  Return value
*    Returns a pointer to the located wide character, or a null pointer 
*    if c does not occur in the string.
*
*  Additional information
*    Locates the last occurrence of c in the wide string pointed
*    to by s. The terminating wide null character is considered to
*    be part of the string.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcsrchr(const wchar_t *s, wchar_t c) {
  const wchar_t *s0;
  //
  // Save source.
  //
  s0 = s;
  //
  // Find end of string.
  //
  while (*s++) {
    // pass
  }
  //
  // Search towards front.
  //
  while (--s != s0 && *s != c) {
    // pass
  }
  //
  // If found, return pointer to character.
  //
  return *s == c ? (wchar_t *)s : 0;
}

/*********************************************************************
*
*       wcsncat()
*
*  Function description
*    Concatenate strings, limit length.
*
*  Parameters
*    s1 - Wide string to append to.
*    s2 - Wide string to append.
*    n  - Maximum number of wide characters in s1.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Appends not more than n wide characters from the array pointed 
*    to by s2 to the end of the wide string pointed to by s1. A null wide character 
*    in s1 and wide characters that follow it are not appended. The initial wide character 
*    of s2 overwrites the null wide character at the end of s1. A terminating 
*    wide null character is always appended to the result. The behavior of wcsncat()
*    is undefined if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcsncat(wchar_t *s1, const wchar_t *s2, size_t n) {
  wchar_t *p = s1;
  //
  // Find end of destination.
  //
  while (*s1)
    s1++;
  //
  // Append up to count characters.
  //
  while (n--)
    if ((*s1++ = *s2++) == 0)
      return p;
  //
  // Appended exactly count characters, terminate string.
  //
  *s1 = 0;
  return p;
}

/*********************************************************************
*
*       wcsnchr()
*
*  Function description
*    Find character within string, forward, limit length.
*
*  Parameters
*    s - Pointer to wide string to search.
*    n - Number of wide characters to search.
*    c - Wide character to search for.
*
*  Return value
*    Returns a pointer to the located wide character, or a null pointer 
*    if c does not occur in the string.
*
*  Additional information
*    Searches not more than n wide characters to locate the first occurrence
*    of c in the wide string pointed to by s.  The terminating wide null
*    character is considered to be part of the wide string.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcsnchr(const wchar_t *s, size_t n, wchar_t c) {
  while (n > 0 && *s && *s != c) {
    ++s;
  }
  //
  return n > 0 && *s == c ? (wchar_t *)s : 0;
}

/*********************************************************************
*
*       wcsncmp()
*
*  Function description
*    Compare strings, limit length.
*
*  Parameters
*    s1 - Pointer to wide string #1.
*    s2 - Pointer to wide string #2.
*    n  - Maximum number of wide characters to compare.
*
*  Return value
*    Returns an integer greater than, equal to, or less than zero, 
*    if the possibly null-terminated array pointed to by s1 is greater than, 
*    equal to, or less than the possibly null-terminated array pointed to by s2.
*
*  Additional information
*    Compares not more than n wide characters from the array pointed 
*    to by s1 to the array pointed to by s2. Wide characters that follow 
*    a null wide character are not compared.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n) {
  //
  // Handle degenerate case.
  //
  if (n == 0) {
    return 0;
  }
  //
  // Find common prefix until strings run out.
  //
  while (--n && *s1 && *s1 == *s2) {
    ++s1;
    ++s2;
  }
  //
  // Return where strings differ taking care of potential underflows.
  //
  if (sizeof(wchar_t) == sizeof(int)) {
    if (*s1 < *s2) {
      return -1;
    } else if (*s1 == *s2) {
      return 0;
    } else {
      return +1;
    }
  } else {
    return *s1 - *s2;
  }
}

/*********************************************************************
*
*       wcsncpy()
*
*  Function description
*    Copy string, limit length.
*
*  Parameters
*    s1 - Pointer to wide string to copy to.
*    s2 - Pointer to wide string to copy.
*    n  - Maximum number of wide characters to copy.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Copies not more than n wide characters from the array pointed 
*    to by s2 to the array pointed to by s1. Wide characters that follow 
*    a null wide character in s2 are not copied. The behavior of wcsncpy()
*    is undefined if copying takes place between objects that overlap. If the array 
*    pointed to by s2 is a wide string that is shorter than n wide characters, 
*    null wide characters are appended to the copy in the array pointed to by s1, 
*    until n characters in all have been written.
*
*  Notes
*    No wide null character is implicitly appended to the end of s1, so s1 will only be
*    terminated by a wide null character if the length of the wide string pointed to by s2 is
*    less than n.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcsncpy(wchar_t *s1, const wchar_t *s2, size_t n) {
  wchar_t *s0;
  //
  // Save original destination.
  //
  s0 = s1;
  //
  // Copy string until count exhausted or trailing zero.
  while (n && (*s1++ = *s2++)) {
    --n;
  }
  //
  // Pad with zeroes to write exactly n characters.
  //
  if (n) {
    while (--n) {
      *s1++ = 0;
    }
  }
  //
  // Return original destination.
  //
  return s0;
}

/*********************************************************************
*
*       wcsnlen()
*
*  Function description
*    Calculate length of string, limit length (POSIX.1).
*
*  Parameters
*    s - Pointer to wide string.
*    n - Maximum number of wide characters to examine.
*
*  Return value
*    Returns the length of the wide string pointed to by s, up
*    to a maximum of n wide characters.  wcsnlen() only examines
*    the first n wide characters of the string s.
*
*  Notes
*    Conforms to POSIX.1-2008.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcsnlen(const wchar_t *s, size_t n) {
  const wchar_t *sos;
  //
  // Find end of string.
  //
  sos = s;
  while (*s && n > 0) {
    ++s;
    --n;
  }
  //
  // Compute and return length.
  //
  return s-sos;
}

/*********************************************************************
*
*       wmemccpy()
*
*  Function description
*    Copy memory, specify terminator (POSIX.1).
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    c  - Character that terminates copy.
*    n  - Maximum number of characters to copy.
*
*  Return value
*    Returns a pointer to the wide character immediately following c
*    in s1, or NULL if c was not found in the first n wide characters
*    of s2.
*
*  Additional information
*    Copies at most n wide characters from the object pointed to by
*    s2 into the object pointed to by s1. The copying stops as soon
*    as n wide characters are copied or the wide character c is copied
*    into the destination object pointed to by s1.
*    
*    The behavior of wmemccpy() is undefined if copying takes place
*    between objects that overlap.
*
*  Notes
*    Conforms to POSIX.1-2008.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wmemccpy(wchar_t *s1, const wchar_t *s2, wchar_t c, size_t n) {
  while (n && (*s1++ = *s2++) != c) {
    --n;
  }
  //
  return n ? s1 : 0;
}

/*********************************************************************
*
*       wmemchr()
*
*  Function description
*    Find character in memory, forward.
*
*  Parameters
*    s - Pointer to object to search.
*    c - Wide character to search for.
*    n - Number of wide characters in object to search.
*
*  Return value
*     == NULL - c does not occur in the object.
*     != NULL - Pointer to the located wide character.
*
*  Additional information
*    Locates the first occurrence of c in the initial n wide characters
*    of the object pointed to by s.  Unlike wcschr(), wmemchr() does
*    not terminate a search when a null wide character is found in the
*    object pointed to by s.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wmemchr(const wchar_t *s, wchar_t c, size_t n) {
  while (n && *s != c) {
    ++s;
    --n;
  }
  //
  return n ? (wchar_t *)s : 0;
}

/*********************************************************************
*
*       wmemcmp()
*
*  Function description
*    Compare memory.
*
*  Parameters
*    s1 - Pointer to object #1.
*    s2 - Pointer to object #2.
*    n  - Number of wide characters to compare.
*
*  Return value
*     <  0 - s1 is less than s2.
*     == 0 - s1 is equal to s2.
*     >  0 - s1 is greater than to s2.
*
*  Additional information
*    Compares the first n wide characters of the object pointed 
*    to by s1 to the first n wide characters of the object pointed to by 
*    s2. wmemcmp() returns an integer greater than, equal to, or less 
*    than zero as the object pointed to by s1 is greater than, equal to,
*    or less than the object pointed to by s2.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n) {
  if (n == 0) {
    return 0;
  }
  //
  while (--n && *s1 == *s2) {
    ++s1;
    ++s2;
  }
  //
  return *s1 - *s2;
}

/*********************************************************************
*
*       wmemcpy()
*
*  Function description
*    Copy memory.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of wide characters to copy.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Copies n wide characters from the object pointed to by s2
*    into the object pointed to by s1. The behavior of wmemcpy() is
*    undefined if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wmemcpy(wchar_t *s1, const wchar_t *s2, size_t n) {
  wchar_t *p = s1;
  //
  while (n--) {
    *s1++ = *s2++;
  }
  //
  return p;
}

/*********************************************************************
*
*       wmemmove()
*
*  Function description
*    Copy memory, tolerate overlaps.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of wide characters to copy.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Copies n wide characters from the object pointed to by s2
*    into the object pointed to by s1 ensuring that if s1 and s2
*    overlap, the copy works correctly. Copying takes place as if
*    the n wide characters  from the object pointed to by s2 are
*    first copied into a temporary array of n wide characters that
*    does not overlap the objects pointed to by s1 and s2, and
*    then the n wide characters from the temporary array are
*    copied into the object pointed to by s1.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wmemmove(wchar_t *s1, const wchar_t *s2, size_t n) {
  wchar_t *ret;
  //
  ret = s1;
  //
  if (s2 < s1) {
    for (s2 += n, s1 += n; n; --n) {
      *--s1 = *--s2;
    }
  } else if (s2 != s1) {
    while (n--) {
      *s1++ = *s2++;
    }
  }
  //
  return ret;
}

/*********************************************************************
*
*       wmempcpy()
*
*  Function description
*    Copy memory (GNU).
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of wide characters to copy.
*
*  Return value
*    Returns a pointer to the wide character immediately following
*    the final wide character written into s1.
*
*  Additional information
*    Copies n wide characters from the object pointed to by s2
*    into the object pointed to by s1. The behavior of wmempcpy() is
*    undefined if copying takes place between objects that overlap.
*
*  Notes
*    This is an extension found in GNU libc.
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wmempcpy(wchar_t *s1, const wchar_t *s2, size_t n) {
  wmemcpy(s1, s2, n);
  return s1 + n;
}

/*********************************************************************
*
*       wmemset()
*
*  Function description
*    Set memory to wide character.
*
*  Parameters
*    s - Pointer to destination object.
*    c - Wide character to copy.
*    n - Length of destination object in wide characters.
*
*  Return value
*    Returns s.
*
*  Additional information
*    Copies the value of c into each of the first n
*    wide characters of the object pointed to by s. 
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wmemset(wchar_t *s, wchar_t c, size_t n) {
  wchar_t *ret;
  //
  ret = s;
  //
  // Copy bytes over memory.
  //
  while (n--) {
    *s++ = c;
  }
  //
  // Return original start address.
  //
  return ret;
}

/*********************************************************************
*
*       wcstok()
*
*  Function description
*    Break string into tokens.
*
*  Parameters
*    s1  - Pointer to zero-terminated wide string to parse.
*    s2  - Pointer to zero-terminated set of separators.
*    ptr - Pointer to object that maintains parse state.
*
*  Return value
*    NULL if no further tokens else a pointer to the next token.
*
*  Additional information
*    A sequence of calls to wcstok() breaks the wide string pointed to by s1 
*    into a sequence of tokens, each of which is delimited by a wide character from the 
*    wide string pointed to by s2. The first call in the sequence has a non-null 
*    first argument; subsequent calls in the sequence have a null first argument. 
*    The separator wide string pointed to by s2 may be different from call to call. 
*    
*    The first call in the sequence searches the wide string pointed to by s1
*    for the wide first character that is not contained in the current separator wide string 
*    pointed to by s2. If no such wide character is found, then there are no tokens 
*    in the string pointed to by s1 and wcstok() returns a null pointer. 
*    If such a wide character is found, it is the start of the first token.
*    
*    wcstok() then searches from there for a wide character that is contained in 
*    the current separator wide string. If no such wide character is found, the current token 
*    extends to the end of the wide string pointed to by s1, and subsequent searches 
*    for a token will return a null pointer. If such a wide character is found, it is 
*    overwritten by a null wide character, which terminates the current token. wcstok()
*    saves a pointer to the following wide character, from which the next search for a 
*    token will start.
*    
*    Each subsequent call, with a null pointer as the value of the first argument, 
*    starts searching from the saved pointer and behaves as described above.
*    
*  See also
*    wcssep().
*
*  Thread safety
*    Safe.
*/
wchar_t * __SEGGER_RTL_PUBLIC_API wcstok(wchar_t *s1, const wchar_t *s2, wchar_t **ptr) {
  wchar_t *start;
  //
  // Null input starts scanning from end of last token.
  //
  if (s1 == 0) {
    s1 = *ptr;
  }
  //
  // Skip leading separators.
  //
  while (*s1 && wcschr(s2, *s1)) {
    s1++;
  }
  //
  // If end of string, no further tokens.
  //
  if (*s1 == 0) {
    return 0;
  }
  //
  // Save start of token.
  //
  start = s1;
  //
  // Find end of token.
  //
  while (*s1 && wcschr(s2, *s1) == 0) {
    s1++;
  }
  //
  if (*s1) {
    //
    // Break string.
    //
    *s1 = 0;
    //
    // Prime for next scan.
    //
    *ptr = s1+1;
  } else {
    *ptr = s1;
  }
  //
  // Return start of token.
  //
  return start;
}

/*********************************************************************
*
*       wcslcpy()
*
*  Function description
*    Copy string, limit length, always zero terminate (BSD).
*
*  Parameters
*    s1 - Pointer to wide string to copy to.
*    s2 - Pointer to wide string to copy.
*    n  - Maximum number of wide characters, including terminating
*         null, in s1.
*
*  Return value
*    Returns the number of wide characters it tried to copy, which is
*    the length of the wide string s2 or n, whichever is smaller.
*
*  Additional information
*    Copies up to n-1 wide characters from the wide string pointed to by
*    s2 into the array pointed to by s1 and always terminates
*    the result with a null character.
*    
*    The behavior of strlcpy() is undefined if copying takes place between
*    objects that overlap.
*
*  Notes
*    Commonly found in BSD libraries and contrasts with wcsncpy()
*    in that the resulting string is always terminated with a null
*    wide character.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcslcpy(wchar_t *s1, const wchar_t *s2, size_t n) {
  const wchar_t *s = s2;
  size_t siz = n;
  //
  // Copy as many wide characters as will fit.
  //
  if (siz) {
    while (--siz) {
      if ((*s1++ = *s++) == 0) {
        break;
      }
    }
  }
  //
  // Not enough room in dst, add null terminator and traverse rest of source.
  //
  if (siz == 0) {
    if (n) {
      *s1 = 0;
    }
    while (*s++) {
      // pass
    }
  }
  //
  // Count does not include terminating null.
  //
  return s - s2 - 1;
}

/*********************************************************************
*
*       wcslcat()
*
*  Function description
*    Concatenate strings, limit length, always zero terminate (BSD).
*
*  Parameters
*    s1 - Pointer to wide string to append to.
*    s2 - Pointer to wide string to append.
*    n  - Maximum number of characters, including terminating
*         wide null, in s1.
*
*  Return value
*    Returns the number of wide characters it tried to copy, which is
*    the sum of the lengths of the wide strings s1 and s2 or n,
*    whichever is smaller.
*
*  Additional information
*    Appends no more than n-strlen(s1}-1 wide characters pointed to by s2 into
*    the array pointed to by s1 and always terminates the result with a
*    wide null character if n is greater than zero.  Both the wide strings s1 and
*    s2 must be terminated with a wide null character on entry to wcslcat()
*    and a character position for the terminating wide null should be included
*    in n.
*
*    The behavior of wcslcat() is undefined if copying takes place between
*    objects that overlap.
*
*  Notes
*    Commonly found in BSD libraries.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API wcslcat(char *s1, const char *s2, size_t n) {
  char *d = s1;
  const char *s = s2;
  size_t siz = n;
  size_t dlen;
  //
  // Find the end of dst and adjust bytes left but don't go past end.
  //
  while (siz-- && *d) {
    ++d;
  }
  dlen = d - s1;
  siz = n - dlen;
  //
  // If already filled, return how many we tried to copy now.
  //
  if (siz == 0) {
    return (strlen)(s) + dlen;
  }
  //
  while (*s) {
    if (siz != 1) {
      *d++ = *s;
      siz--;
    }
    s++;
  }
  *d = 0;
  //
  // Count does not include terminating null.
  //
  return s - s2 + dlen;
}

/*********************************************************************
*
*       wcscasecmp()
*
*  Function description
*    Compare strings, ignore case (POSIX.1).
*
*  Parameters
*    s1 - Pointer to wide string #1.
*    s2 - Pointer to wide string #2.
*
*  Return value
*     <  0 - s1 is less than s2.
*     == 0 - s1 is equal to s2.
*     >  0 - s1 is greater than to s2.
*
*  Additional information
*    Compares the wide string pointed to by s1 to the wide string pointed 
*    to by s2 ignoring differences in case.
*
*    wcscasecmp() returns an integer greater than, equal to, or
*    less than zero if the wide string pointed to by s1 is greater
*    than, equal to, or less than the wide string pointed to by s2.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API wcscasecmp(const char *s1, const char *s2) {
  while (towlower(*s1) == towlower(*s2) && *s2) {
    ++s1;
    ++s2;
  }
  //
  if (towlower(*s1) < towlower(*s2)) {
    return -1;
  } else if (towlower(*s1) > towlower(*s2)) {
    return +1;
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       wcsncasecmp()
*
*  Function description
*    Compare strings, ignore case, limit length (POSIX.1).
*
*  Parameters
*    s1 - Pointer to wide string #1.
*    s2 - Pointer to wide string #2.
*    n  - Maximum number of wide characters to compare.
*
*  Return value
*     <  0 - s1 is less than s2.
*     == 0 - s1 is equal to s2.
*     >  0 - s1 is greater than to s2.
*
*  Additional information
*    Compares not more than n wide characters from the array pointed 
*    to by s1 to the array pointed to by s2 ignoring differences in case.
*    Characters that follow a wide null character are not compared.
*    
*    strncasecmp() returns an integer greater than, equal to, or less than zero, 
*    if the possibly null-terminated array pointed to by s1 is greater than, 
*    equal to, or less than the possibly null-terminated array pointed to by s2.
*
*  Notes
*    Conforms to POSIX.1-2017.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API wcsncasecmp(const char *s1, const char *s2, size_t n) {
  //
  // Handle degenerate case.
  //
  if (n == 0) {
    return 0;
  }
  //
  // Find common prefix until strings run out.
  //
  while (--n && *s1 && towlower(*s1) == towlower(*s2)) {
    ++s1;
    ++s2;
  }
  //
  if (towlower(*s1) < towlower(*s2)) {
    return -1;
  } else if (towlower(*s1) > towlower(*s2)) {
    return +1;
  } else {
    return 0;
  }
}

/*************************** End of file ****************************/
