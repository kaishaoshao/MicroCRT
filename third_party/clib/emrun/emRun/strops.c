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
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

typedef __SEGGER_RTL_UNALIGNED_ATTR __SEGGER_RTL_WORD __SEGGER_RTL_UNALIGNED_WORD;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static __SEGGER_RTL_STATE_THREAD char *__SEGGER_RTL_strtok_state;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

static __SEGGER_RTL_ALWAYS_INLINE int __SEGGER_RTL_Overlaps(const void *s1, const void *s2, size_t n) {
  intptr_t i1;
  intptr_t i2;
  //
  if (n == 0) {
    return 0;
  }
  //
  i1 = (intptr_t)s1;
  i2 = (intptr_t)s2;
  if (i1 < i2) {
    return i1+(intptr_t)n <= i2;
  } else {
    return i2+(intptr_t)n <= i1;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_memcpy_forward()
*
*  Function description
*    Copy memory, forward direction, lower memory to higher memory addresses.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of memcpy() is undefined
*    if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_memcpy_forward(void *s1, const void *s2, size_t n) {
  //
#if __SEGGER_RTL_OPTIMIZE >= 0
  //
  char                    * b1 = s1;
  const char              * b2 = s2;
  __SEGGER_RTL_WORD         Word;
  __SEGGER_RTL_WORD       * w1;
  const __SEGGER_RTL_WORD * w2;
  //
  // Can we ever be equally aligned?
  //
  if (__SEGGER_RTL_ALIGN_REM(b1) == __SEGGER_RTL_ALIGN_REM(b2)) {
    //
    // Copy by bytes until aligned or terminated.
    //
    while (__SEGGER_RTL_ALIGN_REM(b1) != 0 && n > 0) {
      *b1++ = *b2++;
      --n;
    }
    //
    // Ready to process a word at a time.
    //
    w1 = (      __SEGGER_RTL_WORD *)b1;
    w2 = (const __SEGGER_RTL_WORD *)b2;
    //
    // Copy words.
    //
#if __SEGGER_RTL_OPTIMIZE >= 2
    while (n >= 4*sizeof(Word)) {
      *w1++ = *w2++;
      *w1++ = *w2++;
      *w1++ = *w2++;
      *w1++ = *w2++;
      n -= 4*sizeof(Word);
    }
#endif
    while (n >= sizeof(Word)) {
      *w1++ = *w2++;
      n -= sizeof(Word);
    }
    //
    b1 = (char *)w1;
    b2 = (const char *)w2;
    //
    while (n--) {
      *b1++ = *b2++;
    }
  } else {
    //
    // Bring source to alignment.
    //
    while (n > 0 && __SEGGER_RTL_ALIGN_REM(b2) != 0) {
      *b1++ = *b2++;
      --n;
    }
    //
    // Ready to process a word at a time as b2 aligned.
    //
    w2 = (const __SEGGER_RTL_WORD *)b2;
    //
    // Copy words that have nonzero bytes.
    //
#if __SEGGER_RTL_OPTIMIZE >= 2
    while (n >= 4*sizeof(Word)) {
      __SEGGER_RTL_WR_WORD(b1, *w2++); b1 += sizeof(Word);
      __SEGGER_RTL_WR_WORD(b1, *w2++); b1 += sizeof(Word);
      __SEGGER_RTL_WR_WORD(b1, *w2++); b1 += sizeof(Word);
      __SEGGER_RTL_WR_WORD(b1, *w2++); b1 += sizeof(Word);
      n -= 4*sizeof(Word);
    }
#endif
    while (n >= sizeof(Word)) {
      __SEGGER_RTL_WR_WORD(b1, *w2++); b1 += sizeof(Word);
      n -= sizeof(Word);
    }
    //
    b2 = (const char *)w2;
    //
    while (n--) {
      *b1++ = *b2++;
    }
  }
  //
#else
  //
  unsigned char       *b1 = s1;
  const unsigned char *b2 = s2;
  //
  while (n--) {
    *b1++ = *b2++;
  }
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_memcpy_backward()
*
*  Function description
*    Copy memory, backward direction, higher memory to lower memory addresses.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Return value
*    Returns a pointer to the destination object.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of memcpy() is undefined
*    if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_memcpy_backward(void *s1, const void *s2, size_t n) {
  //
#if __SEGGER_RTL_OPTIMIZE >= 0
  //
  char                    * b1 = s1;
  const char              * b2 = s2;
  __SEGGER_RTL_WORD         Word;
  __SEGGER_RTL_WORD       * w1;
  const __SEGGER_RTL_WORD * w2;
  //
  // Can we ever be equally aligned?
  //
  if (__SEGGER_RTL_ALIGN_REM(b1) == __SEGGER_RTL_ALIGN_REM(b2)) {
    //
    // Copy by bytes until aligned or terminated.
    //
    while (__SEGGER_RTL_ALIGN_REM(b1) != 0 && n > 0) {
      *--b1 = *--b2;
      --n;
    }
    //
    // Ready to process a word at a time.
    //
    w1 = (      __SEGGER_RTL_WORD *)b1;
    w2 = (const __SEGGER_RTL_WORD *)b2;
    //
    // Copy words.
    //
#if __SEGGER_RTL_OPTIMIZE >= 2
    while (n >= 4*sizeof(Word)) {
      *--w1 = *--w2;
      *--w1 = *--w2;
      *--w1 = *--w2;
      *--w1 = *--w2;
      n -= 4*sizeof(Word);
    }
#endif
    while (n >= sizeof(Word)) {
      *--w1 = *--w2;
      n -= sizeof(Word);
    }
    //
    b1 = (char *)w1;
    b2 = (const char *)w2;
    //
    while (n--) {
      *--b1 = *--b2;
    }
  } else {
    //
    // Bring source to alignment.
    //
    while (n > 0 && __SEGGER_RTL_ALIGN_REM(b2) != 0) {
      *--b1 = *--b2;
      --n;
    }
    //
    // Ready to process a word at a time as b2 aligned.
    //
    w2 = (const __SEGGER_RTL_WORD *)b2;
    //
    // Copy words that have nonzero bytes.
    //
#if __SEGGER_RTL_OPTIMIZE >= 2
    while (n >= 4*sizeof(Word)) {
      b1 -= sizeof(Word); __SEGGER_RTL_WR_WORD(b1, *--w2);
      b1 -= sizeof(Word); __SEGGER_RTL_WR_WORD(b1, *--w2);
      b1 -= sizeof(Word); __SEGGER_RTL_WR_WORD(b1, *--w2);
      b1 -= sizeof(Word); __SEGGER_RTL_WR_WORD(b1, *--w2);
      n  -= 4*sizeof(Word);
    }
#endif
    while (n >= sizeof(Word)) {
      b1 -= sizeof(Word); __SEGGER_RTL_WR_WORD(b1, *--w2);
      n  -= sizeof(Word);
    }
    //
    b2 = (const char *)w2;
    //
    while (n--) {
      *--b1 = *--b2;
    }
  }
  //
#else
  //
  unsigned char       *b1 = s1;
  const unsigned char *b2 = s2;
  //
  while (n--) {
    *--b1 = *--b2;
  }
  //
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_memcpy_inline()
*
*  Function description
*    Copy memory.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Return value
*    Returns a pointer to the destination object.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of memcpy() is undefined
*    if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_NO_BUILTIN void * __SEGGER_RTL_memcpy_inline(void *s1, const void *s2, size_t n) {
  __SEGGER_RTL_memcpy_forward(s1, s2, n);
  return s1;
}

/*********************************************************************
*
*       __SEGGER_RTL_memmove_inline()
*
*  Function description
*    Copy memory, tolerate overlaps.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1 ensuring that if s1 and s2 overlap,
*    the copy works correctly. Copying takes place as if the n
*    characters from the object pointed to by s2 are first copied
*    into a temporary array of n characters that does not overlap
*    the objects pointed to by s1 and s2, and then the n characters
*    from the temporary array are copied into the object pointed to
*    by s1.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_NO_BUILTIN void * __SEGGER_RTL_memmove_inline(void *s1, const void *s2, size_t n) {
  if (s1 < s2) {
    __SEGGER_RTL_memcpy_forward(s1, s2, n);
  } else if (s1 > s2) {
    __SEGGER_RTL_memcpy_backward((char *)s1 + n, (const char *)s2 + n, n);
  }
  return s1;
}

/*********************************************************************
*
*       __SEGGER_RTL_memset_inline()
*
*  Function description
*    Set memory to character.
*
*  Parameters
*    s - Pointer to destination object.
*    c - Character to copy.
*    n - Length of destination object in characters.
*
*  Additional information
*    Copies the value of c (converted to an unsigned char) 
*    into each of the first n characters of the object pointed to by s.
*/
static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_NO_BUILTIN void * __SEGGER_RTL_memset_inline(void *s, int c, size_t n) {
  //
#if __SEGGER_RTL_OPTIMIZE >= 0
  //
  char                        * uc;
  __SEGGER_RTL_UNALIGNED_WORD * w1;
  __SEGGER_RTL_WORD             Word;
  unsigned                      Lead;
  //
  // Dispose of zero-length case quickly.
  //
  if (n == 0) {
    return s;
  }
  //
  uc   = (char *)s;
  Word = __SEGGER_RTL_BYTE_PATTERN((unsigned char)c);
  //
  // Write leading bytes until alignment.
  //
  if (__SEGGER_RTL_ALIGN_REM(uc) != 0) {
    Lead = sizeof(Word) - __SEGGER_RTL_ALIGN_REM(uc);
    if (Lead > n) {
      Lead = n;
    }
    __SEGGER_RTL_WR_PARTIAL_WORD(uc, Word, Lead);
    uc += Lead;
    n  -= Lead;
  }
  //
  // Write aligned words.
  //
  w1 = (__SEGGER_RTL_UNALIGNED_WORD *)uc;
#if __SEGGER_RTL_OPTIMIZE >= 2
  while (n >= 4*sizeof(Word)) {
    *w1++ = Word;
    *w1++ = Word;
    *w1++ = Word;
    *w1++ = Word;
    n    -= 4*sizeof(Word);
  }
#endif  
  while (n >= sizeof(Word)) {
    *w1++ = Word;
    n    -= sizeof(Word);
  }
  //
  // Write any trailing bytes.
  //
  if (n > 0) {
    __SEGGER_RTL_WR_PARTIAL_WORD((char *)w1, Word, n);
  }
  //
#else
  //
  unsigned char *ucdst = s;
  //
  // Copy bytes over memory.
  //
  while (n--) {
    *ucdst++ = (unsigned char)c;
  }
  //
#endif
  //
  // Return original start address.
  //
  return s;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       memmove()
*
*  Function description
*    Copy memory, tolerate overlaps.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Return value
*     Returns the value of s1.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1 ensuring that if s1 and s2 overlap, the
*    copy works correctly. Copying takes place as if the n characters 
*    from the object pointed to by s2 are first copied into a temporary
*    array of n characters that does not overlap the objects pointed to
*    by s1  and s2, and then the n characters from the temporary array
*    are copied into the object pointed to by s1.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API memmove(void *s1, const void *s2, size_t n) {
  return __SEGGER_RTL_memmove_inline(s1, s2, n);
}

/*********************************************************************
*
*       memmove_s()
*
*  Function description
*    Copy memory, tolerate overlaps, safe (C11).
*
*  Parameters
*    s1    - Pointer to destination object.
*    s1max - Size of destination object in characters.
*    s2    - Pointer to source object.
*    n     - Number of characters to copy.
*
*  Runtime constraints
*    * Neither s1 nor s2 shall be a null pointer.
*    * Neither s1max nor n shall be greater than RSIZE_MAX.
*    * n shall not be greater than s1max.
*
*    If there is a runtime-constraint violation, memmove_s() stores
*    zeros in the first s1max characters of the object pointed to by
*    s1, if s1 is not a null pointer and s1max is not greater than RSIZE_MAX.
*
*  Return value
*    == 0 if runtime constraints are not violated.
*    != 0 if runtime constraints are violated.
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <string.h> to access this function.
*
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1 ensuring that if s1 and s2 overlap, the
*    copy works correctly. Copying takes place as if the n characters 
*    from the object pointed to by s2 are first copied into a temporary
*    array of n characters that does not overlap the objects pointed to
*    by s1  and s2, and then the n characters from the temporary array
*    are copied into the object pointed to by s1.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*
*  Thread safety
*    Safe.
*/
errno_t __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_NO_BUILTIN memmove_s(void *s1, size_t s1max, const void *s2, size_t n) {
  const char *msg;
  //
  // Enforce constraints.
  //
  if      (s1 == NULL)        { msg = "memmove_s(s1, s1max, s2, n) with s1=NULL";           }
  else if (s2 == NULL)        { msg = "memmove_s(s1, s1max, s2, n) with s2=NULL";           }
  else if (s1max > RSIZE_MAX) { msg = "memmove_s(s1, s1max, s2, n) with s1max > RSIZE_MAX"; }
  else if (n > s1max)         { msg = "memmove_s(s1, s1max, s2, n) with n > s1max";         }
  else                        { msg = NULL;                                                 }
  //
  if (msg == NULL) {
    (memmove)(s1, s2, n);
    return 0;
  } else {
    if (s1 != NULL && s1max <= RSIZE_MAX) {
      (memset)(s1, 0, s1max);
    }
    __SEGGER_RTL_constraint_violation(msg, NULL, EINVAL);
    return EINVAL;
  }
}

/*********************************************************************
*
*       memcmp()
*
*  Function description
*    Compare memory.
*
*  Parameters
*    s1 - Pointer to object #1.
*    s2 - Pointer to object #2.
*    n  - Number of characters to compare.
*
*  Return value
*     <  0 - s1 is less than s2.
*     == 0 - s1 is equal to s2.
*     >  0 - s1 is greater than to s2.
*
*  Additional information
*    Compares the first n characters of the object pointed to by s1 to the
*    first n characters of the object pointed to by s2. memcmp() returns an
*    integer greater than, equal to, or less than zero as the object pointed
*    to by s1 is greater than, equal to, or less than the object pointed to
*    by s2.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char * u1;
  const unsigned char * u2;
#if __SEGGER_RTL_OPTIMIZE >= 0
  unsigned              DiffIndex;
#endif
  //
  u1 = (const unsigned char *)s1;
  u2 = (const unsigned char *)s2;
  //
#if __SEGGER_RTL_OPTIMIZE >= 0
  //
  const __SEGGER_RTL_WORD * w1;
  const __SEGGER_RTL_WORD * w2;
  //
  // Can we ever both be aligned?
  //
  if (__SEGGER_RTL_ALIGN_REM(u1) == __SEGGER_RTL_ALIGN_REM(u2)) {
    //
    // Force alignment of both in parallel.
    //
    for (;;) {
      if (__SEGGER_RTL_ALIGN_REM(u1) == 0) { break;            }
      if (n == 0)                          { return 0;         }
      if (*u1 != *u2)                      { return *u1 - *u2; }
      ++u1;
      ++u2;
      --n;
    }
    //
    w1 = (const __SEGGER_RTL_WORD *)u1;
    w2 = (const __SEGGER_RTL_WORD *)u2;
    //
#if __SEGGER_RTL_OPTIMIZE >= 2
    //
    while (n >= 4*sizeof(__SEGGER_RTL_WORD)) {
      if (__SEGGER_RTL_UNLIKELY(w1[0] != w2[0])) {                                                     break; }
      if (__SEGGER_RTL_UNLIKELY(w1[1] != w2[1])) { w1 += 1; w2 += 1; n -= 1*sizeof(__SEGGER_RTL_WORD); break; }
      if (__SEGGER_RTL_UNLIKELY(w1[2] != w2[2])) { w1 += 2; w2 += 2; n -= 2*sizeof(__SEGGER_RTL_WORD); break; }
      if (__SEGGER_RTL_UNLIKELY(w1[3] != w2[3])) { w1 += 3; w2 += 3; n -= 3*sizeof(__SEGGER_RTL_WORD); break; }
      w1 += 4;
      w2 += 4;
      n  -= 4*sizeof(__SEGGER_RTL_WORD);
    }
    //
#endif
    //
    while (n >= sizeof(__SEGGER_RTL_WORD) && *w1 == *w2) {
      ++w1;
      ++w2;
      n -= sizeof(__SEGGER_RTL_WORD);
    }
    //
    //
    // Either w1 and w2 point to a word where there is a difference.
    // The byte at the first difference determines the ordering.
    //
    if (n == 0) {
      return 0;
    }
    //
    DiffIndex  = __SEGGER_RTL_DIFF_INDEX(*w1, *w2);
    if (DiffIndex >= n) {
      return 0;
    }
    return __SEGGER_RTL_DIFF_BYTE(*w1, DiffIndex) - __SEGGER_RTL_DIFF_BYTE(*w2, DiffIndex);
    //
  } else {
    //
    // Choose to bring u1 to alignment.
    //
    for (;;) {
      if (__SEGGER_RTL_ALIGN_REM(u1) == 0) { break;            }
      if (n == 0)                          { return 0;         }
      if (*u1 != *u2)                      { return *u1 - *u2; }
      ++u1;
      ++u2;
      --n;
    }
    //
    // u1 aligned but u2 not, proceed by word compares.
    //
    w1 = (const __SEGGER_RTL_WORD *)u1;
    //
    while (n >= sizeof(__SEGGER_RTL_WORD) && *w1 == __SEGGER_RTL_RD_WORD(u2)) {
      w1 += 1;
      u2 += sizeof(__SEGGER_RTL_WORD);
      n  -= sizeof(__SEGGER_RTL_WORD);
    }
    //
    if (n == 0) {
      return 0;
    }
    //
    u1 = (const unsigned char *)w1;
  }
  //
  while (--n && *u1 == *u2) {
    ++u1;
    ++u2;
  }
  //
  // Return where strings differ.
  //
  return *u1 - *u2;
  //
#else
  //
  // Handle degenerate case.
  //
  if (n == 0) {
    return 0;
  }
  //
  // Find common prefix until strings run out.
  //
  while (--n && *u1 == *u2) {
    ++u1;
    ++u2;
  }
  //
  // Return where strings differ.
  //
  return *u1 - *u2;
  //
#endif
}

/*********************************************************************
*
*       memlcp()
*
*  Function description
*    Compare memory, longest common prefix.
*
*  Parameters
*    s1 - Pointer to object #1.
*    s2 - Pointer to object #2.
*    n  - Number of characters to compare.
*
*  Return value
*     The length of the longest common prefix of object #1 and object #2,
*     i.e. the number of leading characters of both objects that compare
*     identical.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API memlcp(const void *s1, const void *s2, size_t n) {
  const unsigned char * u1;
  const unsigned char * u2;
  const unsigned char * origin;
  //
  origin = (const unsigned char *)s1;
  u1     = (const unsigned char *)s1;
  u2     = (const unsigned char *)s2;
  //
#if __SEGGER_RTL_OPTIMIZE >= 0
  //
  const __SEGGER_RTL_WORD * w1;
  const __SEGGER_RTL_WORD * w2;
  //
  // Can we ever both be aligned?
  //
  if (__SEGGER_RTL_ALIGN_REM(u1) == __SEGGER_RTL_ALIGN_REM(u2)) {
    //
    // Force alignment of both in parallel.
    //
    while (__SEGGER_RTL_ALIGN_REM(u1) != 0 && n > 0 && *u1 == *u2) {
      ++u1;
      ++u2;
      --n;
    }
    //
    if (n == 0) {
      return u1 - origin;
    }
    //
    w1 = (const __SEGGER_RTL_WORD *)u1;
    w2 = (const __SEGGER_RTL_WORD *)u2;
    //
    for (;;) {
      if (n < sizeof(__SEGGER_RTL_WORD) || *w1 != *w2) { break; } else { ++w1; ++w2; n -= sizeof(__SEGGER_RTL_WORD); }
#if __SEGGER_RTL_OPTIMIZE >= 2
      if (n < sizeof(__SEGGER_RTL_WORD) || *w1 != *w2) { break; } else { ++w1; ++w2; n -= sizeof(__SEGGER_RTL_WORD); }
      if (n < sizeof(__SEGGER_RTL_WORD) || *w1 != *w2) { break; } else { ++w1; ++w2; n -= sizeof(__SEGGER_RTL_WORD); }
      if (n < sizeof(__SEGGER_RTL_WORD) || *w1 != *w2) { break; } else { ++w1; ++w2; n -= sizeof(__SEGGER_RTL_WORD); }
#endif
    }
    //
    u1 = (const unsigned char *)w1;
    u2 = (const unsigned char *)w2;
    //
    if (n == 0) {
      return u1 - origin;
    }
    //
  } else {
    //
    // Choose to bring u1 to alignment.
    //
    while (__SEGGER_RTL_ALIGN_REM(u1) != 0 && n > 0 && *u1 == *u2) {
      ++u1;
      ++u2;
      --n;
    }
    //
    if (n == 0) {
      return u1 - origin;
    }
    //
    // u1 aligned but u2 not, proceed by word compares.
    //
    w1 = (const __SEGGER_RTL_WORD *)u1;
    //
    while (n >= sizeof(__SEGGER_RTL_WORD) && *w1 == __SEGGER_RTL_RD_WORD(u2)) {
      ++w1;
      u2 += sizeof(__SEGGER_RTL_WORD);
      n  -= sizeof(__SEGGER_RTL_WORD);
    }
    //
    u1 = (const unsigned char *)w1;
    //
    if (n == 0) {
      return u1 - origin;
    }
  }
  //
  while (--n && *u1 == *u2) {
    ++u1;
    ++u2;
  }
  //
  // Return where strings differ.
  //
  return u1 - origin;
  //
#else
  //
  // Handle degenerate case.
  //
  if (n == 0) {
    return 0;
  }
  //
  // Find common prefix until strings run out.
  //
  while (--n && *u1 == *u2) {
    ++u1;
    ++u2;
  }
  //
  // Return where strings differ.
  //
  return u1 - origin;
  //
#endif
}

/*********************************************************************
*
*       memchr()
*
*  Function description
*    Find character in memory, forward.
*
*  Parameters
*    s - Pointer to object to search.
*    c - Character to search for.
*    n - Number of characters in object to search.
*
*  Return value
*     == NULL - c does not occur in the object.
*     != NULL - Pointer to the located character.
*
*  Additional information
*    Locates the first occurrence of c (converted to an unsigned char)
*    in the initial n characters (each interpreted as unsigned char)
*    of the object pointed to by s.  Unlike strchr(), memchr() does
*    not terminate a search when a null character is found in the object 
*    pointed to by s.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API memchr(const void *s, int c, size_t n) {
  const unsigned char * s0;
  //
  c &= 0xFF;
  //
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  // Acceleration possible with optimized zero-byte check.
  //
  const __SEGGER_RTL_WORD * pWord;
  __SEGGER_RTL_WORD         Word;
  __SEGGER_RTL_WORD         Pattern;
  unsigned                  Index;
  //
  // Dispatch simple cases and avoid one-time setup costs.
  //
  if (n == 0) {
    return NULL;
  }
  //
  pWord   = __SEGGER_RTL_ALIGN_PTR(s);               // Pointer to natural alignment
  n      += __SEGGER_RTL_ALIGN_REM(s);               // Account for pointer alignment
  Word    = __SEGGER_RTL_FILL_HEAD(s, *pWord, c^1);  // Fill bytes before 's' with non-matching values.
  Pattern = __SEGGER_RTL_BYTE_PATTERN(c);
  //
  // Iterate over all words that have no matching bytes.
  //
#if __SEGGER_RTL_OPTIMIZE >= 2
  while (n >= 4 * sizeof(Word)) {
    if (__SEGGER_RTL_LIKELY(__SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0)) { Word = *++pWord; } else {                      break; }
    if (__SEGGER_RTL_LIKELY(__SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0)) { Word = *++pWord; } else { n -= 1*sizeof(Word); break; }
    if (__SEGGER_RTL_LIKELY(__SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0)) { Word = *++pWord; } else { n -= 2*sizeof(Word); break; }
    if (__SEGGER_RTL_LIKELY(__SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0)) { Word = *++pWord; } else { n -= 3*sizeof(Word); break; }
    n -= 4*sizeof(Word);
  }
#endif
  for (;;) {
    if (__SEGGER_RTL_LIKELY(n >= sizeof(Word) && __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0)) { Word = *++pWord; n -= sizeof(Word); } else { break; }
  }
  //
  // Find index of any matching byte within the word, even bytes beyond the
  // specified range.
  //
  Index = __SEGGER_RTL_ZBYTE_INDEX(Word ^ Pattern);
  //
  // If found within the specified range, return appropriate pointer, else null.
  //
  return __SEGGER_RTL_LIKELY(Index < n) ? (char *)pWord + Index : NULL;
  //
#endif
  //
  s0 = s;
#if __SEGGER_RTL_OPTIMIZE >= 0
  while (n >= 4) {
    if (s0[0] == c) return (char *)s0;
    if (s0[1] == c) return (char *)s0+1;
    if (s0[2] == c) return (char *)s0+2;
    if (s0[3] == c) return (char *)s0+3;
    s0 += 4;
    n  -= 4;
  }
#endif
  while (n && *s0 != c) {
    ++s0;
    --n;
  }
  //
  return n ? (void *)s0 : NULL;
}

/*********************************************************************
*
*       memrchr()
*
*  Function description
*    Find character in memory, reverse (BSD).
*
*  Parameters
*    s - Pointer to object to search.
*    c - Character to search for.
*    n - Number of characters in object to search.
*
*  Return value
*    Returns a pointer to the located character, or a null pointer 
*    if c does not occur in the octet string.
*
*  Additional information
*    Locates the last occurrence of c (converted to a char) 
*    in the octet string pointed to by s.
*
*  Conformance
*    Commonly found in Linux and BSD C libraries.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API memrchr(const void *s, int c, size_t n) {
  const char *us;
  //  
  // Search towards front.
  //
  us = s;
  us += n;
  while (--us != s && *us != (char)c) {
    // pass
  }
  //
  // If found, return pointer to character.
  //
  return *us == (char)c ? (char *)us : 0;
}

/*********************************************************************
*
*       memcpy()
*
*  Function description
*    Copy memory.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Return value
*    Returns a pointer to the destination object.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of memcpy() is undefined
*    if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_NO_BUILTIN memcpy(void *s1, const void *s2, size_t n) {
  __SEGGER_RTL_memcpy_forward(s1, s2, n);
  return s1;
}

/*********************************************************************
*
*       memcpy_s()
*
*  Function description
*    Copy memory, safe (C11).
*
*  Parameters
*    s1    - Pointer to destination object.
*    s1max - Size of destination object in characters.
*    s2    - Pointer to source object.
*    n     - Number of characters to copy.
*
*  Runtime constraints
*    * Neither s1 nor s2 shall be a null pointer.
*    * Neither s1max nor n shall be greater than RSIZE_MAX.
*    * n shall not be greater than s1max.
*    * Copying shall not take place between objects that overlap.
*
*    If there is a runtime-constraint violation, memcpy_s() stores
*    zeros in the first s1max characters of the object pointed to by
*    s1, if s1 is not a null pointer and s1max is not greater than RSIZE_MAX.
*
*  Return value
*    == 0 if runtime constraints are not violated.
*    != 0 if runtime constraints are violated.
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <string.h> to access this function.
*
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*
*  Thread safety
*    Safe.
*/
errno_t __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_NO_BUILTIN memcpy_s(void *s1, size_t s1max, const void *s2, size_t n) {
  const char *msg;
  //
  // Enforce constraints.
  //
  if      (s1 == NULL)                       { msg = "memcpy_s(s1, s1max, s2, n) with s1=NULL";           }
  else if (s2 == NULL)                       { msg = "memcpy_s(s1, s1max, s2, n) with s2=NULL";           }
  else if (s1max > RSIZE_MAX)                { msg = "memcpy_s(s1, s1max, s2, n) with s1max > RSIZE_MAX"; }
  else if (n > RSIZE_MAX)                    { msg = "memcpy_s(s1, s1max, s2, n) with n > RSIZE_MAX";     }
  else if (n > s1max)                        { msg = "memcpy_s(s1, s1max, s2, n) with n > s1max";         }
  else if (__SEGGER_RTL_Overlaps(s1, s2, n)) { msg = "memcpy_s(s1, s1max, s2, n) with s1 overlapping s2"; }
  else                                       { msg = NULL;                                                }
  //
  if (msg == NULL) {
    (memcpy)(s1, s2, n);
    return 0;
  } else {
    if (s1 != NULL && s1max <= RSIZE_MAX) {
      (memset)(s1, 0, s1max);
    }
    __SEGGER_RTL_constraint_violation(msg, NULL, EINVAL);
    return EINVAL;
  }
}

/*********************************************************************
*
*       mempcpy()
*
*  Function description
*    Copy memory (GNU).
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Return value
*    Returns a pointer to the character immediately following the
*    final character written into s1.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of mempcpy() is undefined
*    if copying takes place between objects that overlap.
*
*  Conformance
*    This is an extension found in GNU libc.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API mempcpy(void *s1, const void *s2, size_t n) {
  return (char *)(memcpy)(s1, s2, n) + n;
}

/*********************************************************************
*
*       memmem()
*
*  Function description
*    Find memory in memory, forward (BSD).
*
*  Parameters
*    s1 - Pointer to object to search.
*    n1 - Number of characters to search in s1.
*    s2 - Pointer to object to search for.
*    n2 - Number of characters to search from s2.
*
*  Return value
*     == NULL - (s2, n2) does not occur in (s1, n1).
*     != NULL - Pointer to the first occurrence of (s2, n2) in (s1, n1).
*
*  Additional information
*    Locates the first occurrence of the octet string s2 of length n2
*    in the octet string s1 of length n1.
*
*  Conformance
*    Commonly found in Linux and BSD C libraries.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API memmem(const void *s1, size_t n1, const void *s2, size_t n2) {
  const char * begin;
  const char * extent;
  int          c;
  //
  // Degenerate cases.
  //
  if (n2 == 0) {
    return (void *)s1;  // cast away const
  } else if (n1 < n2) {
    return 0;
  }
  //
  extent = (const char *)s1 + n1 - n2;
  c = *(const char *)s2;
  //
  for (begin = s1; begin <= extent; ++begin) {
    if (*begin == c && memcmp(begin, s2, n2) == 0) {
      return (void *)begin;  // cast away const
    }
  }
  //
  return 0;
}

/*********************************************************************
*
*       memccpy()
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
*    Returns a pointer to the character immediately following c in
*    s1, or NULL if c was not found in the first n characters of s2.
*
*  Additional information
*    Copies at most n characters from the object pointed to
*    by s2 into the object pointed to by s1. The copying stops as soon as
*    n characters are copied or the character c is copied into the
*    destination object pointed to by s1.
*    
*    The behavior of memccpy() is undefined if copying takes place
*    between objects that overlap.
*
*  Conformance
*    POSIX.1-2008.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API memccpy(void *s1, const void *s2, int c, size_t n) {
  unsigned char       *ucdst;
  const unsigned char *ucsrc;
  //
  ucdst = s1;
  ucsrc = s2;
  while (n && (*ucdst++ = *ucsrc++) != (unsigned char)c) {
    --n;
  }
  //
  return n ? ucdst : 0;
}

/*********************************************************************
*
*       memset()
*
*  Function description
*    Set memory to character.
*
*  Parameters
*    s - Pointer to destination object.
*    c - Character to write.
*    n - Number of characters to write in destination object.
*
*  Return value
*    Returns s.
*
*  Additional information
*    Copies the value of c (converted to an unsigned char) into each
*    of the first n characters of the object pointed to by s.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_NO_BUILTIN memset(void *s, int c, size_t n) {
  __SEGGER_RTL_memset_inline(s, c, n);
  return s;
}

/*********************************************************************
*
*       memset_s()
*
*  Function description
*    Set memory to character, safe (C11).
*
*  Parameters
*    s    - Pointer to destination object.
*    smax - Size of destination object in characters.
*    c    - Character to write.
*    n    - Number of characters to write in destination object.
*
*  Runtime constraints
*    * s shall not be a null pointer.
*    * Neither smax nor n shall be greater than RSIZE_MAX.
*    * n shall not be greater than s1max.
*
*    If there is a runtime-constraint violation, memset_s() stores
*    c (converted to an unsigned char) into each of the first smax
*    characters of the object pointed to by s, if s is not a null
*    pointer and smax is not greater than RSIZE_MAX.
*
*  Return value
*    == 0 if runtime constraints are not violated.
*    != 0 if runtime constraints are violated.
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <string.h> to access this function.
*
*    Copies the value of c (converted to an unsigned char) into each
*    of the first n characters of the object pointed to by s.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*
*  Thread safety
*    Safe.
*/
errno_t __SEGGER_RTL_PUBLIC_API memset_s(void *s, size_t smax, int c, size_t n) {
  char *msg;
  //
  // Enforce constraints.
  //
  if      (s == NULL)        { msg = "memset(s, smax, c, n) with s=NULL";           }
  else if (smax > RSIZE_MAX) { msg = "memset(s, smax, c, n) with smax > RSIZE_MAX"; }
  else if (n > RSIZE_MAX)    { msg = "memset(s, smax, c, n) with n > RSIZE_MAX";    }
  else if (n > smax)         { msg = "memset(s, smax, c, n) with n > smax";         }
  else                       { msg = NULL;                                          }
  //
  if (msg == NULL) {
    (memset)(s, c, n);
    return 0;
  } else {
    if (s != NULL && smax <= RSIZE_MAX) {
      (memset)(s, c, smax);
    }
    __SEGGER_RTL_constraint_violation(msg, NULL, EINVAL);
    return EINVAL;
  }
}

/*********************************************************************
*
*       strcasecmp()
*
*  Function description
*    Compare strings, ignore case (POSIX.1).
*
*  Parameters
*    s1 - Pointer to string #1.
*    s2 - Pointer to string #2.
*
*  Return value
*     <  0 - s1 is less than s2.
*     == 0 - s1 is equal to s2.
*     >  0 - s1 is greater than to s2.
*
*  Additional information
*    Compares the string pointed to by s1 to the string pointed 
*    to by s2 ignoring differences in case.
*
*    strcasecmp() returns an integer greater than, equal to, or
*    less than zero if the string pointed to by s1 is greater
*    than, equal to, or less than the string pointed to by s2.
*
*  Conformance
*    POSIX.1-2008.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API strcasecmp(const char *s1, const char *s2) {
  int ret;
  //
  while ((ret = tolower(*s1) - tolower(*s2)) == 0 && *s2) {
    ++s1;
    ++s2;
  }
  //
  return ret;
}

/*********************************************************************
*
*       strncasecmp()
*
*  Function description
*    Compare strings, ignore case, limit length (POSIX.1).
*
*  Parameters
*    s1 - Pointer to string #1.
*    s2 - Pointer to string #2.
*    n  - Maximum number of characters to compare.
*
*  Return value
*     <  0 - s1 is less than s2.
*     == 0 - s1 is equal to s2.
*     >  0 - s1 is greater than to s2.
*
*  Additional information
*    Compares not more than n characters from the array pointed 
*    to by s1 to the array pointed to by s2 ignoring differences in case.
*    Characters that follow a null character are not compared.
*    
*    strncasecmp() returns an integer greater than, equal to, or less than zero, 
*    if the possibly null-terminated array pointed to by s1 is greater than, 
*    equal to, or less than the possibly null-terminated array pointed to by s2.
*
*  Conformance
*    POSIX.1-2008.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API strncasecmp(const char *s1, const char *s2, size_t n) {
  //
  // Handle degenerate case.
  //
  if (n == 0) {
    return 0;
  }
  //
  // Find common prefix until strings run out.
  //
  while (--n && *s1 && tolower(*s1) == tolower(*s2)) {
    ++s1;
    ++s2;
  }
  //
  // Return where strings differ.
  //
  return tolower(*s1) - tolower(*s2);
}

/*********************************************************************
*
*       strcasestr()
*
*  Function description
*    Find string within string, forward, ignore case (BSD).
*
*  Parameters
*    s1 - String to search.
*    s2 - String to search for.
*
*  Return value
*    Returns a pointer to the located string, or a null pointer if 
*    the string is not found. If s2 points to a string with zero length,
*    returns s1.
*
*  Additional information
*    Locates the first occurrence in the string pointed to by s1
*    of the sequence of characters (excluding the terminating null character) in 
*    the string pointed to by s2 without regard to character case.
*
*  Conformance
*    This extension is commonly found in Linux and BSD C libraries.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strcasestr(const char *s1, const char *s2) {
  const char *p = s1;
  //
  if (*s2 == 0) {
    return (char *)p;  // cast away const
  }
  //
  while (*p) {
    const char *xs1 = p;
    const char *xs2 = s2;
    //
    while (*xs1 && *xs2 && toupper(*xs1) == toupper(*xs2)) {
      ++xs1, ++xs2;
    }
    //
    if (*xs2 == 0) {
      return (char *)p;  // cast away const
    }
    ++p;
  }
  //
  return 0;
}

/*********************************************************************
*
*       strncasestr()
*
*  Function description
*    Find string within string, forward, ignore case, limit length (BSD).
*
*  Parameters
*    s1 - String to search.
*    s2 - String to search for.
*    n  - Maximum number of characters to compare in s2.
*
*  Return value
*    Returns a pointer to the located string, or a null pointer if 
*    the string is not found. If s2 points to a string with zero length,
*    returns s1.
*
*  Additional information
*   Searches at most n characters to locate the first occurrence
*   in the string pointed to by s1 of the sequence of characters (excluding the
*   terminating null character) in the string pointed to by s2 without regard
*   to character case.
*
*  Conformance
*    This extension is commonly found in Linux and BSD C libraries.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strncasestr(const char *s1, const char *s2, size_t n) {
  char   c, sc;
  size_t len;
  //
  c = tolower(*s2++);
  if (c) {
    len = (strlen)(s2);
    do {
      do {
        if (n-- < 1 || (sc = *s1++) == '\0') {
          return 0;
        }
      } while (tolower(sc) != c);
      if (len > n) {
        return 0;
      }
    } while (strncasecmp(s1, s2, len) != 0);
    --s1;
  }
  //
  return (char *)s1;
}

/*********************************************************************
*
*       strchr()
*
*  Function description
*    Find character within string, forward.
*
*  Parameters
*    s - String to search.
*    c - Character to search for.
*
*  Return value
*    Returns a pointer to the located character, or a null pointer 
*    if c does not occur in the string.
*
*  Additional information
*    Locates the first occurrence of c (converted to a char)
*    in the string pointed to by s. The terminating null character 
*    is considered to be part of the string.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strchr(const char *s, int c) {
  //
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  // Acceleration possible with optimized zero-byte check.
  //
  const __SEGGER_RTL_WORD * pWord;
  __SEGGER_RTL_WORD         Word;
  __SEGGER_RTL_WORD         Pattern;
  unsigned                  ZeroIndex;
  unsigned                  MatchIndex;
  //
  c      &= 0xFF;
  pWord   = __SEGGER_RTL_ALIGN_PTR(s);               // Pointer to natural alignment
  Word    = __SEGGER_RTL_FILL_HEAD(s, *pWord, c^1);  // Fill bytes before 's' with non-matching values.
  Pattern = __SEGGER_RTL_BYTE_PATTERN(c);
  //
  // Iterate over all words that do not have zero bytes.
  //
  for (;;) {
    if ((__SEGGER_RTL_ZBYTE_CHECK(Word) | __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern)) != 0) break; else Word = *++pWord;
#if __SEGGER_RTL_OPTIMIZE >= 2
    if ((__SEGGER_RTL_ZBYTE_CHECK(Word) | __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern)) != 0) break; else Word = *++pWord;
    if ((__SEGGER_RTL_ZBYTE_CHECK(Word) | __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern)) != 0) break; else Word = *++pWord;
    if ((__SEGGER_RTL_ZBYTE_CHECK(Word) | __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern)) != 0) break; else Word = *++pWord;
#endif
  }
  //
  ZeroIndex  = __SEGGER_RTL_ZBYTE_INDEX(Word);
  MatchIndex = __SEGGER_RTL_ZBYTE_INDEX(Word ^ Pattern);
  //
  if (c == 0) {
    return (char *)pWord + ZeroIndex;
  } else if (MatchIndex < ZeroIndex) {
    return (char *)pWord + MatchIndex;
  } else {
    return NULL;
  }
  //
#else
  //
  while (*s && *s != (char)c) {
    ++s;
  }
  //
  return *s == (char)c ? (char *)s : NULL;
  //
#endif
}

/*********************************************************************
*
*       strcpy()
*
*  Function description
*    Copy string.
*
*  Parameters
*    s1 - String to copy to.
*    s2 - String to copy.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Copies the string pointed to by s2 (including the terminating 
*    null character) into the array pointed to by s1. The behavior of strcpy()
*    is undefined if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strcpy(char *s1, const char *s2) {
  //
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  char                    * res = s1;
  __SEGGER_RTL_WORD         Word;
  __SEGGER_RTL_WORD       * w1;
  const __SEGGER_RTL_WORD * w2;
  unsigned                  n;
  //
  // Can we ever be equally aligned?
  //
  if (__SEGGER_RTL_ALIGN_REM(s1) == __SEGGER_RTL_ALIGN_REM(s2)) {
    //
    // Copy by bytes until aligned or terminated.
    //
    while (__SEGGER_RTL_ALIGN_REM(s1) != 0) {
      if ((*s1++ = *s2++) == 0) {
        return res;
      }
    }
    //
    // Ready to process a word at a time.
    //
    w1 = (      __SEGGER_RTL_WORD *)s1;
    w2 = (const __SEGGER_RTL_WORD *)s2;
    //
    // Copy words that have nonzero bytes.
    //
#if __SEGGER_RTL_OPTIMIZE >= 2
    for (;;) {
      if (__SEGGER_RTL_ZBYTE_CHECK(*w2) == 0) { *w1++ = *w2++; } else { break; }
      if (__SEGGER_RTL_ZBYTE_CHECK(*w2) == 0) { *w1++ = *w2++; } else { break; }
      if (__SEGGER_RTL_ZBYTE_CHECK(*w2) == 0) { *w1++ = *w2++; } else { break; }
      if (__SEGGER_RTL_ZBYTE_CHECK(*w2) == 0) { *w1++ = *w2++; } else { break; }
    }
#else
    while (__SEGGER_RTL_ZBYTE_CHECK(*w2) == 0) {
      *w1++ = *w2++;
    }
#endif
    //
    // Terminating word as it has a zero byte; find its index and prepare
    // to copy.
    //
    n = __SEGGER_RTL_ZBYTE_INDEX(*w2) + 1; // Number of nonzero bytes plus terminating zero byte.
    //
    // Store partial-word tail.
    //
    __SEGGER_RTL_WR_PARTIAL_WORD((char *)w1, *w2, n);
    //
  } else {
    //
    // Bring source to alignment; early exit if terminated.
    //
    while (__SEGGER_RTL_ALIGN_REM(s2) != 0) {
      if ((*s1++ = *s2++) == 0) {
        return res;
      }
    }
    //
    // Ready to process a word at a time as s2 aligned.
    //
    w2 = (const __SEGGER_RTL_WORD *)s2;
    //
    // Copy words that have nonzero bytes.
    //
    while (__SEGGER_RTL_ZBYTE_CHECK(*w2) == 0) {
      __SEGGER_RTL_WR_WORD(s1, *w2++);
      s1 += sizeof(Word);
    }
    //
    // Terminating word as it has a zero byte; find its index and prepare
    // to copy.
    //
    n = __SEGGER_RTL_ZBYTE_INDEX(*w2) + 1; // Number of nonzero bytes plus terminating zero byte.
    //
    // Store partial-word tail.
    //
    __SEGGER_RTL_WR_PARTIAL_WORD(s1, *w2, n);
  }
  //
  return res;
  //
#elif __SEGGER_RTL_OPTIMIZE < 0
  //
  return (memcpy)(s1, s2, (strlen)(s2)+1);
  //
#else
  //
  char *res = s1;
  //
  while ((*s1++ = *s2++) != 0) {
    /* Pass */
  }
  //
  return res;
  //
#endif
}

/*********************************************************************
*
*       strcpy_s()
*
*  Function description
*    Copy string, safe (C11).
*
*  Parameters
*    s1    - Pointer to destination object.
*    s1max - Size of destination object in characters.
*    s2    - Pointer to source object.
*
*  Runtime constraints
*    * Neither s1 nor s2 shall be a null pointer.
*    * s1max shall not be greater than RSIZE_MAX.
*    * s1max shall be greater than strnlen_s(s2, s1max).
*    * Copying shall not take place between objects that overlap.
*
*    If there is a runtime-constraint violation, strcpy_s() sets
*    s1[0] to zero if s1 is nonnull and s1max is not greater
*    than RSIZE_MAX.
*
*  Return value
*    == 0 if runtime constraints are not violated.
*    != 0 if runtime constraints are violated.
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <string.h> to access this function.
*
*    Copies the string pointed to by s2 (including the terminating 
*    null character) into the array pointed to by s1.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*
*  Thread safety
*    Safe.
*/
errno_t __SEGGER_RTL_PUBLIC_API strcpy_s(char *s1, size_t s1max, const char *s2) {
  const char *msg;
  size_t      s2len;
  //
  // Enforce constraints.
  //
  s2len = strnlen_s(s2, s1max);  // Returns 0 if s2 is null.
  //
  if      (s1 == NULL)                           { msg = "strcpy_s(s1, s1max, s2) with s1=NULL";                       }
  else if (s2 == NULL)                           { msg = "strcpy_s(s1, s1max, s2) with s2=NULL";                       }
  else if (s1max > RSIZE_MAX)                    { msg = "strcpy_s(s1, s1max, s2) with s1max > RSIZE_MAX";             }
  else if (s1max <= s2len)                       { msg = "strcpy_s(s1, s1max, s2) with s1max <= strnlen_s(s2, s1max)"; }
  else if (__SEGGER_RTL_Overlaps(s1, s2, s2len)) { msg = "strcpy_s(s1, s1max, s2) with s1 overlapping s2";             }
  else                                           { msg = NULL;                                                         }
  //
  if (msg == NULL) {
    (strcpy)(s1, s2);
    return 0;
  } else {
    if (s1 != NULL && 0 < s1max && s1max <= RSIZE_MAX) {
      s1[0] = 0;
    }
    __SEGGER_RTL_constraint_violation(msg, NULL, EINVAL);
    return EINVAL;
  }
}

/*********************************************************************
*
*       stpcpy()
*
*  Function description
*    Copy string, return end.
*
*  Parameters
*    s1 - String to copy to.
*    s2 - String to copy.
*
*  Return value
*    A pointer to the end of the string s1, i.e. the terminating null
*    byte of the string s1, after s2 is copied to it.
*
*  Additional information
*    Copies the string pointed to by s2 (including the terminating 
*    null character) into the array pointed to by s1. The behavior of stpcpy()
*    is undefined if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API stpcpy(char *s1, const char *s2) {
  while ((*s1++ = *s2++) != 0) {
    /* Null*/
  }
  //
  return s1 - 1;
}

/*********************************************************************
*
*       strlen()
*
*  Function description
*    Calculate length of string.
*
*  Parameters
*    s - Pointer to zero-terminated string.
*
*  Return value
*    Returns the length of the string pointed to by s, that 
*    is the number of characters that precede the terminating
*    null character.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API strlen(const char *s) {
  //
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  // Acceleration possible with optimized zero-byte check.
  //
  const char              * s0;
  const __SEGGER_RTL_WORD * pWord;
  __SEGGER_RTL_WORD         Word;
  //
  // Dispatch simple cases and avoid one-time setup costs.
  //
  if (*s == 0) {
    return 0;
  }
  //
  s0    = s;
  pWord = __SEGGER_RTL_ALIGN_PTR(s);                // Pointer to natural alignment
  Word  = __SEGGER_RTL_FILL_HEAD(s, *pWord, 0xFF);  // Fill bytes before 's' with nonzero values.
  //
  // Iterate over all words that do not have zero bytes.
  //
  for (;;) {
    if (__SEGGER_RTL_LIKELY(__SEGGER_RTL_ZBYTE_CHECK(Word) == 0)) Word = *++pWord; else break;
#if __SEGGER_RTL_OPTIMIZE >= 2
    if (__SEGGER_RTL_LIKELY(__SEGGER_RTL_ZBYTE_CHECK(Word) == 0)) Word = *++pWord; else break;
    if (__SEGGER_RTL_LIKELY(__SEGGER_RTL_ZBYTE_CHECK(Word) == 0)) Word = *++pWord; else break;
    if (__SEGGER_RTL_LIKELY(__SEGGER_RTL_ZBYTE_CHECK(Word) == 0)) Word = *++pWord; else break;
#endif
  }
  //
  // Advance over first byte that is zero within the current word.
  //
  s = (char *)pWord + __SEGGER_RTL_ZBYTE_INDEX(Word);
  //
#else
  //
  // Acceleration not possible, but loop unrolling is always possible.
  //
  const char * s0 = s;
  //
#if __SEGGER_RTL_OPTIMIZE >= 0
  for (;;) {
    if (*s++ == 0) break;
    if (*s++ == 0) break;
    if (*s++ == 0) break;
    if (*s++ == 0) break;
  }
  --s;
#else
  while (*s) {
    ++s;
  }
#endif
#endif
  //
  return s - s0;
}

/*********************************************************************
*
*       strcat()
*
*  Function description
*    Concatenate strings.
*
*  Parameters
*    s1 - Zero-terminated string to append to.
*    s2 - Zero-terminated string to append.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Appends a copy of the string pointed to by s2 (including 
*    the terminating null character) to the end of the string pointed to by s1.
*    The initial character of s2 overwrites the null character at the end 
*    of s1. The behavior of strcat() is undefined if copying takes place 
*    between objects that overlap.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strcat(char *s1, const char *s2) {
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  (strcpy)(s1 + (strlen)(s1), s2);
  return s1;
  //
#else
  //
  char *res = s1;
  //
  while (*s1) {
    ++s1;
  }
  //
  // Append src.
  //
  while ((*s1++ = *s2++) != 0) {
    // pass
  }
  //
  return res;
  //
#endif
}

/*********************************************************************
*
*       strcmp()
*
*  Function description
*    Compare strings.
*
*  Parameters
*    s1 - Pointer to string #1.
*    s2 - Pointer to string #2.
*
*  Return value
*    Returns an integer greater than, equal to, or less than zero, 
*    if the null-terminated array pointed to by s1 is greater than, 
*    equal to, or less than the null-terminated array pointed to by s2.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API strcmp(const char *s1, const char *s2) {
  int ret;
  //
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  const __SEGGER_RTL_WORD *w1;
  const __SEGGER_RTL_WORD *w2;
  //
  // Can both be aligned?
  //
  if (__SEGGER_RTL_ALIGN_REM(s1) == __SEGGER_RTL_ALIGN_REM(s2)) {
    //
    int DiffIndex;
    int NullIndex;
    //
    // Force alignment.
    //
    while (__SEGGER_RTL_ALIGN_REM(s1) != 0) {
      if (*s1 != *s2 || *s1 == 0) {
        return (unsigned char)*s1 - (unsigned char)*s2;
      }
      ++s1;
      ++s2;
    }
    //
    w1 = (const __SEGGER_RTL_WORD *)s1;
    w2 = (const __SEGGER_RTL_WORD *)s2;
    //
    for (;;) {
      if (__SEGGER_RTL_UNLIKELY(*w1 != *w2 || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0)) { break; } else { ++w1; ++w2; }
#if __SEGGER_RTL_OPTIMIZE >= 2
      if (__SEGGER_RTL_UNLIKELY(*w1 != *w2 || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0)) { break; } else { ++w1; ++w2; }
      if (__SEGGER_RTL_UNLIKELY(*w1 != *w2 || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0)) { break; } else { ++w1; ++w2; }
      if (__SEGGER_RTL_UNLIKELY(*w1 != *w2 || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0)) { break; } else { ++w1; ++w2; }
#endif
    }
    //
    // Either w1, w2, or both point to a word where there is a difference or there is a terminating zero.
    //
    DiffIndex = __SEGGER_RTL_DIFF_INDEX(*w1, *w2);
    NullIndex = __SEGGER_RTL_ZBYTE_INDEX(*w1);
    //
    // If the terminating zero byte precedes the non-match, the strings are the same to the
    // terminating byte.
    //
    if (NullIndex < DiffIndex) {
      return 0;
    }
    //
    // The byte at the first difference determines the ordering.
    //
    return __SEGGER_RTL_DIFF_BYTE(*w1, DiffIndex) - __SEGGER_RTL_DIFF_BYTE(*w2, DiffIndex);
    //
#if __SEGGER_RTL_OPTIMIZE >= 1
  //
  } else {
    //
    // Choose to bring s1 to alignment.
    //
    while (__SEGGER_RTL_ALIGN_REM(s1)) {
      if (*s1 != *s2 || *s1 == 0) {
        return (unsigned char)*s1 - (unsigned char)*s2;
      }
      ++s1;
      ++s2;
    }
    //
    // s1 aligned but s2 not, proceed by word compares.
    //
    w1 = (const __SEGGER_RTL_WORD *)s1;
    //
    for (;;) {
      if (__SEGGER_RTL_UNLIKELY(*w1 != __SEGGER_RTL_RD_WORD(s2) || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0)) { break; } else { ++w1; s2 += sizeof(__SEGGER_RTL_WORD); }
#if __SEGGER_RTL_OPTIMIZE >= 2
      if (__SEGGER_RTL_UNLIKELY(*w1 != __SEGGER_RTL_RD_WORD(s2) || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0)) { break; } else { ++w1; s2 += sizeof(__SEGGER_RTL_WORD); }
      if (__SEGGER_RTL_UNLIKELY(*w1 != __SEGGER_RTL_RD_WORD(s2) || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0)) { break; } else { ++w1; s2 += sizeof(__SEGGER_RTL_WORD); }
      if (__SEGGER_RTL_UNLIKELY(*w1 != __SEGGER_RTL_RD_WORD(s2) || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0)) { break; } else { ++w1; s2 += sizeof(__SEGGER_RTL_WORD); }
#endif
    }
    //
    s1 = (const char *)w1;
#endif
  }
  //
  while ((ret = (unsigned char)*s1 - (unsigned char)*s2) == 0 && *s2) {
    ++s1;
    ++s2;
  }
  //
  return ret;
  //
#else
  //
  while ((ret = (unsigned char)*s1 - (unsigned char)*s2) == 0 && *s2) {
    ++s1;
    ++s2;
  }
  //
  return ret;
  //
#endif
}

/*********************************************************************
*
*       strcoll()
*
*  Function description
*    Collate strings.
*
*  Parameters
*    s1 - Pointer to string #1.
*    s2 - Pointer to string #2.
*
*  Return value
*    Returns an integer greater than, equal to, or less than zero, 
*    if the null-terminated array pointed to by s1 is greater than, 
*    equal to, or less than the null-terminated array pointed to by s2.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API strcoll(const char *s1, const char *s2) {
  return (strcmp)(s1, s2);
}

/*********************************************************************
*
*       strxfrm()
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
size_t __SEGGER_RTL_PUBLIC_API strxfrm(char *s1, const char *s2, size_t n) {
  strncpy(s1, s2, n);
  if (n > 0) {
    s1[n-1] = '\0';
  }
  return n;
}

/*********************************************************************
*
*       strcspn()
*
*  Function description
*    Compute size of string not prefixed by a set of characters.
*
*  Parameters
*    s1 - Pointer to string to search.
*    s2 - Pointer to string containing characters to skip.
*
*  Return value
*    Returns the length of the segment of string s1 prefixed
*    by characters from s2.
*
*  Additional information
*    Computes the length of the maximum initial segment of the string 
*    pointed to by s1 which consists entirely of characters not from the string 
*    pointed to by s2.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API strcspn(const char *s1, const char *s2) {
  const char *p;
  //
  p = s1;
  while (*p && (strchr)(s2, *p) == NULL) {
    ++p;
  }
  //
  return p - s1;
}

/*********************************************************************
*
*       strdup()
*
*  Function description
*    Duplicate string (POSIX.1).
*
*  Parameters
*    s1 - Pointer to string to duplicate.
*
*  Return value
*    Returns a pointer to the new string or a null pointer if the
*    new string cannot be created.  The returned pointer can be passed
*    to free().
*
*  Additional information
*    Duplicates the string pointed to by s1 by using malloc() to
*    allocate memory for a copy of s and then copyies s, including the
*    terminating null, to that memory
*
*  Conformance
*    POSIX.1-2008 and SC22 TR 24731-2.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strdup(const char *s1) {
  char   * pCopy;
  size_t   len;
  //
  len   = (strlen)(s1) + 1;
  pCopy = (char *)malloc(len);
  //
  if (pCopy) {
    (memcpy)(pCopy, s1, len);
  }
  //
  return pCopy;
}

/*********************************************************************
*
*       strlcat()
*
*  Function description
*    Concatenate strings, limit length, always zero terminate (BSD).
*
*  Parameters
*    s1 - Pointer to string to append to.
*    s2 - Pointer to string to append.
*    n  - Maximum number of characters, including terminating
*         null, in s1.
*
*  Return value
*    Returns the number of characters it tried to copy, which is
*    the sum of the lengths of the strings s1 and s2 or n,
*    whichever is smaller.
*
*  Additional information
*    Appends no more than n-strlen(s1}-1 characters pointed to by s2 into
*    the array pointed to by s1 and always terminates the result with a
*    null character if n is greater than zero.  Both the strings s1 and
*    s2 must be terminated with a null character on entry to strlcat()
*    and a character position for the terminating null should be included
*    in n.
*
*    The behavior of strlcat() is undefined if copying takes place between
*    objects that overlap.
*
*  Conformance
*    Commonly found in BSD libraries.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API strlcat(char *s1, const char *s2, size_t n) {
  char       * d   = s1;
  const char * s   = s2;
  size_t       siz = n;
  size_t       dlen;
  //
  // Find the end of dst and adjust bytes left but don't go past end.
  //
  while (siz-- && *d) {
    ++d;
  }
  dlen = d - s1;
  siz  = n - dlen;
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
      --siz;
    }
    ++s;
  }
  *d = 0;
  //
  // Count does not include terminating null.
  //
  return s - s2 + dlen;
}

/*********************************************************************
*
*       strlcpy()
*
*  Function description
*    Copy string, limit length, always zero terminate (BSD).
*
*  Parameters
*    s1 - Pointer to string to copy to.
*    s2 - Pointer to string to copy.
*    n  - Maximum number of characters, including terminating
*         null, in s1.
*
*  Return value
*    Returns the number of characters it tried to copy, which is
*    the length of the string s2 or n, whichever is smaller.
*
*  Additional information
*    Copies up to n-1 characters from the string pointed to by
*    s2 into the array pointed to by s1 and always terminates
*    the result with a null character.
*    
*    The behavior of strlcpy() is undefined if copying takes place between
*    objects that overlap.
*
*  Conformance
*    Commonly found in BSD libraries and contrasts with strncpy()
*    in that the resulting string is always terminated with a null
*    character.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API strlcpy(char *s1, const char *s2, size_t n) {
  const char *s = s2;
  size_t siz = n;
  //
  // Copy as many chars as will fit.
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
*       strncat()
*
*  Function description
*    Concatenate strings, limit length.
*
*  Parameters
*    s1 - String to append to.
*    s2 - String to append.
*    n  - Maximum number of characters in s1.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Appends not more than n characters from the array pointed 
*    to by s2 to the end of the string pointed to by s1. A null character 
*    in s1 and characters that follow it are not appended. The initial character 
*    of s2 overwrites the null character at the end of s1. A terminating 
*    null character is always appended to the result.
*    
*    The behavior of strncat() is undefined if copying takes place between
*    objects that overlap.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strncat(char *s1, const char *s2, size_t n) {
  char *p = s1;
  //
  // Find end of destination.
  //
  while (*s1) {
    ++s1;
  }
  //
  // Append up to n characters.
  //
  while (n--) {
    if ((*s1++ = *s2++) == 0) {
      return p;
    }
  }
  //
  // Appended exactly n characters, terminate string.
  //
  *s1 = '\0';
  return p;
}

/*********************************************************************
*
*       strnchr()
*
*  Function description
*    Find character within string, forward, limit length.
*
*  Parameters
*    s - String to search.
*    n - Number of characters to search.
*    c - Character to search for.
*
*  Return value
*    Returns a pointer to the located character, or a null pointer 
*    if c does not occur in the string.
*
*  Additional information
*    Searches not more than n characters to locate the first occurrence
*    of c (converted to a char) in the string pointed to by s.  The
*    terminating null character is considered to be part of the string.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strnchr(const char *s, size_t n, int c) {
  //
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  // Acceleration possible with optimized zero-byte check.
  //
  const __SEGGER_RTL_WORD * pWord;
  __SEGGER_RTL_WORD         Word;
  __SEGGER_RTL_WORD         Pattern;
  unsigned                  ZeroIndex;
  unsigned                  MatchIndex;
  //
  // Dispatch simple cases and avoid one-time setup costs.
  //
  if (n == 0) {
    return NULL;
  }
  //
  c &= 0xFF;
  //
  //
  pWord   = __SEGGER_RTL_ALIGN_PTR(s);               // Pointer to natural alignment
  n      += __SEGGER_RTL_ALIGN_REM(s);               // Account for pointer alignment
  Word    = __SEGGER_RTL_FILL_HEAD(s, *pWord, c^1);  // Fill bytes before 's' with non-matching values.
  Pattern = __SEGGER_RTL_BYTE_PATTERN(c);
  //
  // Iterate over all words that have no matching bytes.
  //
  for (;;) {
    if (n >= sizeof(Word) && __SEGGER_RTL_ZBYTE_CHECK(Word) == 0 && __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0) { Word = *++pWord; n -= sizeof(Word); } else { break; }
#if (__SEGGER_RTL_OPTIMIZE >= 2)
    if (n >= sizeof(Word) && __SEGGER_RTL_ZBYTE_CHECK(Word) == 0 && __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0) { Word = *++pWord; n -= sizeof(Word); } else { break; }
    if (n >= sizeof(Word) && __SEGGER_RTL_ZBYTE_CHECK(Word) == 0 && __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0) { Word = *++pWord; n -= sizeof(Word); } else { break; }
    if (n >= sizeof(Word) && __SEGGER_RTL_ZBYTE_CHECK(Word) == 0 && __SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) == 0) { Word = *++pWord; n -= sizeof(Word); } else { break; }
#endif
  }
  //
  if (n == 0) {
    return 0;
  }
  //
  // Find index of any matching byte within the word, even bytes beyond the
  // specified range.
  //
  ZeroIndex  = __SEGGER_RTL_ZBYTE_INDEX(Word);
  MatchIndex = __SEGGER_RTL_ZBYTE_INDEX(Word ^ Pattern);
  //
  // If found within the specified range, return appropriate pointer, else null.
  //
  return MatchIndex <= ZeroIndex && MatchIndex < n ? (char *)pWord + MatchIndex : NULL;
  //
#else
  //
  while (n > 0 && *s && *s != (char)c) {
    ++s;
    --n;
  }
  //
  return n > 0 && *s == (char)c ? (char *)s : 0;
  //
#endif
}

/*********************************************************************
*
*       strncmp()
*
*  Function description
*    Compare strings, limit length.
*
*  Parameters
*    s1 - Pointer to string #1.
*    s2 - Pointer to string #2.
*    n  - Maximum number of characters to compare.
*
*  Return value
*    Returns an integer greater than, equal to, or less than zero, 
*    if the possibly null-terminated array pointed to by s1 is greater than, 
*    equal to, or less than the possibly null-terminated array pointed to by s2.
*
*  Additional information
*    Compares not more than n characters from the array pointed 
*    to by s1 to the array pointed to by s2. Characters that follow 
*    a null character are not compared.
*
*  Thread safety
*    Safe.
*/
int __SEGGER_RTL_PUBLIC_API strncmp(const char *s1, const char *s2, size_t n) {
  //
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  const __SEGGER_RTL_WORD * w1;
  const __SEGGER_RTL_WORD * w2;
  //
  // Dispatch simple cases and avoid one-time setup costs.
  //
  if (n == 0) {
    return 0;
  } else if (*s1 != *s2) {
    return (unsigned char)*s1 - (unsigned char)*s2;
  } else if (*s1 == 0) {
    return 0;
  }
  //
  // Potentially, can both be simultaneously aligned?
  //
  if (__SEGGER_RTL_ALIGN_REM(s1) == __SEGGER_RTL_ALIGN_REM(s2)) {
    //
    // They can, try forcing both to alignment.
    //
    while (__SEGGER_RTL_ALIGN_REM(s1) != 0 && n > 0 && *s1 == *s2 && *s1 != 0) {
      ++s1;
      ++s2;
      --n;
    }
    //
    if (n == 0) {
      return 0;
    }
    //
    // If both are aligned, use accelerated compares.
    //
    if (__SEGGER_RTL_ALIGN_REM(s1) == 0) {
      w1 = (const __SEGGER_RTL_WORD *)s1;
      w2 = (const __SEGGER_RTL_WORD *)s2;
      //
      for (;;) {
        if (n < sizeof(__SEGGER_RTL_WORD) || *w1 != *w2 || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0) { break; } else { ++w1; ++w2; n -= sizeof(__SEGGER_RTL_WORD); }
  #if __SEGGER_RTL_OPTIMIZE >= 2
        if (n < sizeof(__SEGGER_RTL_WORD) || *w1 != *w2 || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0) { break; } else { ++w1; ++w2; n -= sizeof(__SEGGER_RTL_WORD); }
        if (n < sizeof(__SEGGER_RTL_WORD) || *w1 != *w2 || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0) { break; } else { ++w1; ++w2; n -= sizeof(__SEGGER_RTL_WORD); }
        if (n < sizeof(__SEGGER_RTL_WORD) || *w1 != *w2 || __SEGGER_RTL_ZBYTE_CHECK(*w1) != 0) { break; } else { ++w1; ++w2; n -= sizeof(__SEGGER_RTL_WORD); }
  #endif
      }
      //
      if (n == 0) {
        return 0;
      }
      //
      s1 = (const char *)w1;
      s2 = (const char *)w2;
    }
  } else {
    //
    // Choose to bring s1 to alignment.
    //
    while (__SEGGER_RTL_ALIGN_REM(s1) != 0) {
      if (n == 0) {
        return 0;
      }
      if (*s1 != *s2) {
        return (unsigned char)*s1 - (unsigned char)*s2;
      } else if (*s1 == 0) {
        return 0;
      }
      ++s1;
      ++s2;
      --n;
    }
    //
    if (n == 0) {
      return 0;
    }
    //
    // s1 aligned but s2 not, proceed by word compares.
    //
    w1 = (const __SEGGER_RTL_WORD *)s1;
    //
    while (n >= sizeof(__SEGGER_RTL_WORD) && *w1 == __SEGGER_RTL_RD_WORD(s2) && __SEGGER_RTL_ZBYTE_CHECK(*w1) == 0) {
      w1 += 1;
      s2 += sizeof(__SEGGER_RTL_WORD);
      n  -= sizeof(__SEGGER_RTL_WORD);
    }
    //
    if (n == 0) {
      return 0;
    }
    //
    s1 = (const char *)w1;
  }
  //
  while (--n && *s1 && *s1 == *s2) {
    ++s1;
    ++s2;
  }
  //
  // Return where strings differ.
  //
  return (unsigned char)*s1 - (unsigned char)*s2;
  //
#else
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
  // Return where strings differ.
  //
  return (unsigned char)*s1 - (unsigned char)*s2;
  //
#endif
}

/*********************************************************************
*
*       strncpy()
*
*  Function description
*    Copy string, limit length.
*
*  Parameters
*    s1 - String to copy to.
*    s2 - String to copy.
*    n  - Maximum number of characters to copy.
*
*  Return value
*    Returns the value of s1.
*
*  Additional information
*    Copies not more than n characters from the array pointed 
*    to by s2 to the array pointed to by s1. Characters that follow 
*    a null character in s2 are not copied. The behavior of strncpy()
*    is undefined if copying takes place between objects that overlap.
*    If the array pointed to by s2 is a string that is shorter than n
*    characters, null characters are appended to the copy in the array
*    pointed to by s1, until n characters in all have been written.
*
*  Notes
*    No null character is implicitly appended to the end of s1, so s1
*    will only be terminated by a null character if the length of the
*    string pointed to by s2 is less than n.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strncpy(char *s1, const char *s2, size_t n) {
  char *s0;
  //
  // Copy string until count exhausted or trailing zero.
  //
  s0 = s1;
  while (n && (*s1++ = *s2++)) {
    --n;
  }
  //
  // Pad with zeroes to write exactly n characters.
  //
  if (n) {
    while (--n) {
      *s1++ = '\0';
    }
  }
  //
  // Return original destination.
  //
  return s0;
}

/*********************************************************************
*
*       stpncpy()
*
*  Function description
*    Copy string, limit length, return end.
*
*  Parameters
*    s1 - String to copy to.
*    s2 - String to copy.
*    n  - Maximum number of characters to copy.
*
*  Return value
*    stpncpy() returns a pointer to the terminating null byte in s1
*    after it is copied to, or, if s1 is not null-terminated, s1+n.
*
*  Additional information
*    Copies not more than n characters from the array pointed 
*    to by s2 to the array pointed to by s1. Characters that follow 
*    a null character in s2 are not copied. The behavior of strncpy()
*    is undefined if copying takes place between objects that overlap.
*    If the array  pointed to by s2 is a string that is shorter than n
*    characters, null characters are appended to the copy in the array
*    pointed to by s1, until n characters in all have been written.
*
*  Notes
*    No null character is implicitly appended to the end of s1, so s1 will only be
*    terminated by a null character if the length of the string pointed to by s2 is
*    less than n.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API stpncpy(char *s1, const char *s2, size_t n) {
  size_t size;
  //
  size = (strnlen)(s2, n);
  (memcpy)(s1, s2, size);
  s1 += size;
  if (size == n) {
    return s1;
  }
  return (memset)(s1, '\0', n-size);
}

/*********************************************************************
*
*       strndup()
*
*  Function description
*    Duplicate string, limit length (POSIX.1).
*
*  Parameters
*    s - Pointer to string to duplicate.
*    n - Maximum number of characters to duplicate.
*
*  Return value
*    Returns a pointer to the new string or a null pointer if the
*    new string cannot be created.  The returned pointer can be
*    passed to free().
*
*  Additional information
*    Duplicates at most n characters from the the string pointed to
*    by s by using malloc() to allocate memory for a copy of s.
*    
*    If the length of string pointed to by s is greater than n characters,
*    only n characters will be duplicated.  If n is greater than the length of
*    the string pointed to by s, all characters in the string are copied into the
*    allocated array including the terminating null character.
*
*  Conformance
*    Conforms to POSIX.1-2008 and SC22 TR 24731-2.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strndup(const char *s, size_t n) {
  size_t  max;
  char  * result;
  //
  max = (strnlen)(s, n);
  result = (char *)malloc(max + 1);
  if (result) {
    (memcpy)(result, s, max);
    result[max] = 0;
  }
  return result;
}

/*********************************************************************
*
*       strnlen()
*
*  Function description
*    Calculate length of string, limit length (POSIX.1).
*
*  Parameters
*    s - Pointer to string.
*    n - Maximum number of characters to examine.
*
*  Return value
*    Returns the length of the string pointed to by s, up
*    to a maximum of n characters.  strnlen() only examines
*    the first n characters of the string s.
*
*  Conformance
*    POSIX.1-2008.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API strnlen(const char *s, size_t n) {
  //
#if (__SEGGER_RTL_OPTIMIZE >= 0) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  // Acceleration possible with optimized zero-byte check.
  //
  const char              * s0;
  const __SEGGER_RTL_WORD * pWord;
  __SEGGER_RTL_WORD         Word;
  unsigned                  Index;
  //
  // Dispatch simple cases and avoid one-time setup costs.
  //
  if (n == 0 || *s == 0) {
    return 0;
  }
  //
  s0    = s;
  pWord = __SEGGER_RTL_ALIGN_PTR(s);                // Pointer to natural alignment
  n    += __SEGGER_RTL_ALIGN_REM(s);                // Account for pointer alignment
  Word  = __SEGGER_RTL_FILL_HEAD(s, *pWord, 0xFF);  // Fill bytes before 's' with nonzero values.
  //
  // Iterate over all words that do not have zero bytes.
  //
  for (;;) {
    if (n < sizeof(Word) || __SEGGER_RTL_ZBYTE_CHECK(Word) != 0) { break; } else { Word = *++pWord; n -= sizeof(Word); }
#if __SEGGER_RTL_OPTIMIZE >= 2
    if (n < sizeof(Word) || __SEGGER_RTL_ZBYTE_CHECK(Word) != 0) { break; } else { Word = *++pWord; n -= sizeof(Word); }
    if (n < sizeof(Word) || __SEGGER_RTL_ZBYTE_CHECK(Word) != 0) { break; } else { Word = *++pWord; n -= sizeof(Word); }
    if (n < sizeof(Word) || __SEGGER_RTL_ZBYTE_CHECK(Word) != 0) { break; } else { Word = *++pWord; n -= sizeof(Word); }
#endif
  }
  //
  // Advance over first byte that is zero within the current word.
  //
  Index = __SEGGER_RTL_ZBYTE_INDEX(Word);
  if (Index > n) {
    Index = n;
  }
  return (char *)pWord - s0 + Index;
  //
#else
  //
  const char *s0;
  //
  // Find end of string.
  //
  s0 = s;
  while (*s && n > 0) {
    ++s;
    --n;
  }
  //
  // Compute and return length.
  //
  return s - s0;
  //
#endif
}

/*********************************************************************
*
*       strnlen_s()
*
*  Function description
*    Calculate length of string, limit length (C11).
*
*  Parameters
*    s - Pointer to string or NULL.
*    n - Maximum number of characters to examine.
*
*  Return value
*    If s is NULL, returns 0.  If s is nonnull, returns the length
*    of the string pointed to by s, up to a maximum of n characters.
*    strnlen_s() only examines the first n characters of the string s.
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <string.h> to access this function.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API strnlen_s(const char *s, size_t n) {
  if (s == NULL) {
    return 0;
  } else {
    return (strnlen)(s, n);
  }
}

/*********************************************************************
*
*       strnstr()
*
*  Function description
*    Find string within string, forward, limit length (BSD).
*
*  Parameters
*    s1 - String to search.
*    s2 - String to search for.
*    n  - Maximum number of characters to search for.
*
*  Return value
*    Returns a pointer to the located string, or a null pointer if 
*    the string is not found. If s2 points to a string with zero
*    length, strnstr() returns s1.
*
*  Additional information
*    Searches at most n characters to locate the first occurrence
*    in the string pointed to by s1 of the sequence of characters (excluding the
*    terminating null character) in the string pointed to by s2.
*
*  Conformance
*    Commonly found in Linux and BSD C libraries.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strnstr(const char *s1, const char *s2, size_t n) {
  size_t snippetLen;
  //
  // Handle degenerate case quickly.
  //
  snippetLen = (strlen)(s2);
  if (snippetLen == 0) {
    return (char *)s1;
  }
  //
  // Iterate over string.
  //
  while (n >= snippetLen) {
    //
    // Find next potential match.
    //
    char *fnd = strnchr(s1, n, s2[0]);
    if (fnd == 0) {
      return 0;
    }
    //
    // Did we match here?
    //
    if (strncmp(s2, fnd, snippetLen) == 0) {
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
*       strrchr()
*
*  Function description
*    Find character within string, reverse.
*
*  Parameters
*    s - String to search.
*    c - Character to search for.
*
*  Return value
*    Returns a pointer to the located character, or a null pointer 
*    if c does not occur in the string.
*
*  Additional information
*    Locates the last occurrence of c (converted to a char) in the
*    string pointed to by s. The terminating null character is
*    considered to be part of the string.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strrchr(const char *s, int c) {
  //
#if 0 && (__SEGGER_RTL_OPTIMIZE >= 2) && defined(__SEGGER_RTL_ZBYTE_CHECK)
  //
  // This is temporarily disabled until it can be analyzed.
  //
  const __SEGGER_RTL_WORD * pWord;
  __SEGGER_RTL_WORD         Word;
  __SEGGER_RTL_WORD         Pattern;
  __SEGGER_RTL_WORD         MatchWord;
  char                    * MatchStr;
  unsigned                  n;
  //
  // Searching for 8-bit values.
  //
  c &= 0xFF;
  //
  pWord   = __SEGGER_RTL_ALIGN_PTR(s);               // Pointer to natural alignment
  Word    = __SEGGER_RTL_FILL_HEAD(s, *pWord, c^1);  // Fill bytes before 's' with nonmatching values.
  Pattern = __SEGGER_RTL_BYTE_PATTERN(c);
  //
  // Iterate over all words that do not have zero bytes.
  //
  MatchStr = NULL;
  for (;;) {
    //
    // Remember where last match occurred.
    //
    if (__SEGGER_RTL_ZBYTE_CHECK(Word ^ Pattern) != 0) {
      MatchWord = Word;
      MatchStr  = (char *)pWord;
    }
    //
    // Exit if word contains a zero character.
    //
    if (__SEGGER_RTL_ZBYTE_CHECK(Word) != 0) {
      break;
    }
    //
    // Load next word.
    //
    Word = *++pWord;
  }
  //
  // No word found containing the target character?
  //
  if (MatchStr == NULL) {
    return NULL;
  }
  //
  // MatchWord is guaranteed to contain matching character but
  // may also contain a terminator.
  //
  n = __SEGGER_RTL_ZBYTE_INDEX(MatchWord);  // Number of characters to terminator
  if (n > sizeof(__SEGGER_RTL_WORD)-1) {
    n = sizeof(__SEGGER_RTL_WORD)-1;
  }
  for (MatchStr += n; *MatchStr != c; --MatchStr) {
    /* Pass */
  }
  return MatchStr;
  //
#elif __SEGGER_RTL_OPTIMIZE >= 0
  //
  const char *s0;
  //
  if (c == 0) {
    return (strchr)(s, c);
  }
  //
  s0 = s = (strchr)(s, c);
  while (s != NULL) {
    s0 = s;
    s = (strchr)(s+1, c);
  }
  //
  return (char *)s0;
  //
#else
  //
  const char *last = NULL;
  //
  for (;;) {
    if (*s == c) {
      last = s;
    }
    if (*s == 0) {
      return (char *)last;
    }
    ++s;
  }
  //
#endif
}

/*********************************************************************
*
*       strsep()
*
*  Function description
*    Break string into tokens (BSD).
*
*  Parameters
*    stringp - Pointer to pointer to zero-terminated string.
*    delim   - Pointer to delimiter set string.
*
*  Return value
*    See below.
*
*  Additional information
*    Locates, in the string referenced by *stringp, the first
*    occurrence of any character in the string delim (or the
*    terminating null character) and replaces it with a null
*    character.  The location of the next character after the
*    delimiter character (or NULL, if the end of the string was
*    reached) is stored in *stringp. The original value
*    of *stringp is returned. 
*
*    An empty field (that is, a character in the string delim occurs as the
*    first character of *stringp) can be detected by comparing the location
*    referenced by the returned pointer to the null wide character.
*
*    If *stringp is initially null, strsep() returns null.
*
*  Conformance
*    Commonly found in Linux and BSD C libraries.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strsep(char **stringp, const char *delim) {
  char *res;
  //
  // Degenerate simple cases.
  //
  if (stringp == NULL || *stringp == NULL || **stringp == 0) {
    return 0;
  }
  //
  // Remember start of token.
  //
  res = *stringp;
  //
  // Span non-delimiters.
  //
  while (**stringp && (strchr)(delim, **stringp) == NULL) {
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
*       strspn()
*
*  Function description
*    Compute size of string prefixed by a set of characters.
*
*  Parameters
*    s1 - Pointer to zero-terminated string to search.
*    s2 - Pointer to zero-terminated acceptable-set string.
*
*  Return value
*    Returns the length of the string pointed to by s1 which
*    consists entirely of characters from the string pointed
*    to by s2
*
*  Additional information
*    Computes the length of the maximum initial segment of the string 
*    pointed to by s1 which consists entirely of characters from the string 
*    pointed to by s2.
*
*  Thread safety
*    Safe.
*/
size_t __SEGGER_RTL_PUBLIC_API strspn(const char *s1, const char *s2) {
  const char *p;
  //
  p = s1;
  while (*p && (strchr)(s2, *p) != NULL) {
    ++p;
  }
  return p - s1;
}

/*********************************************************************
*
*       strstr()
*
*  Function description
*    Find string within string, forward.
*
*  Parameters
*    s1 - String to search.
*    s2 - String to search for.
*
*  Return value
*    Returns a pointer to the located string, or a null pointer if 
*    the string is not found. If s2 points to a string with zero
*    length, strstr() returns s1.
*
*  Additional information
*    Locates the first occurrence in the string pointed to by s1
*    of the sequence of characters (excluding the terminating null
*    character) in the string pointed to by s2.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strstr(const char *s1, const char *s2) {
  const char *p;
  //
  if (*s2 == 0) {
    return (char *)s1;  // cast away const
  }
  //
  for (p = s1; *p; ++p) {
    const char *xs1 = p;
    const char *xs2 = s2;
    //
    while (*xs1 && *xs2 && *xs1 == *xs2) {
      ++xs1;
      ++xs2;
    }
    //
    if (*xs2 == 0) {
      return (char *)p;  // cast away const
    }
  }
  //
  return 0;
}

/*********************************************************************
*
*       strpbrk()
*
*  Function description
*    Find first occurrence of characters within string.
*
*  Parameters
*    s1 - Pointer to string to search.
*    s2 - Pointer to string to search for.
*
*  Return value
*    Returns a pointer to the first character, or a null pointer if
*    no character from s2 occurs in s1.
*
*  Additional information
*    Locates the first occurrence in the string pointed to by s1
*    of any character from the string pointed to by s2.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strpbrk(const char *s1, const char *s2) {
  const char *p;
  //
  p = s1;
  while (*p && (strchr)(s2, *p) == NULL) {
    ++p;
  }
  return *p ? (char *)p : NULL;
}

/*********************************************************************
*
*       strtok()
*
*  Function description
*    Break string into tokens.
*
*  Parameters
*    s1 - Pointer to zero-terminated string to parse.
*    s2 - Pointer to zero-terminated set of separators.
*
*  Return value
*    NULL if no further tokens else a pointer to the next token.
*
*  Additional information
*    A sequence of calls to strtok() breaks the string pointed to by s1 
*    into a sequence of tokens, each of which is delimited by a character from the 
*    string pointed to by s2. The first call in the sequence has a non-null 
*    first argument; subsequent calls in the sequence have a null first argument. 
*    The separator string pointed to by s2 may be different from call to call. 
*    
*    The first call in the sequence searches the string pointed to by s1
*    for the first character that is not contained in the current separator string 
*    pointed to by s2. If no such character is found, then there are no tokens 
*    in the string pointed to by s1 and strtok() returns a null pointer. 
*    If such a character is found, it is the start of the first token.
*    
*    strtok() then searches from there for a character that is contained in 
*    the current separator string. If no such character is found, the current token 
*    extends to the end of the string pointed to by s1, and subsequent searches 
*    for a token will return a null pointer. If such a character is found, it is 
*    overwritten by a null character, which terminates the current token. strtok()
*    saves a pointer to the following character, from which the next search for a 
*    token will start.
*    
*    Each subsequent call, with a null pointer as the value of the first argument, 
*    starts searching from the saved pointer and behaves as described above.
*
*  Thread safety
*    Safe [if configured].
*    
*  See also
*    strsep(), strtok_r().
*/
char * __SEGGER_RTL_PUBLIC_API strtok(char *s1, const char *s2) {
  char *start;
  //
  // Null input starts scanning from end of last token.
  //
  if (!s1) {
    s1 = __SEGGER_RTL_strtok_state;
  }
  //
  // Skip leading separators.
  //
  while (*s1 && (strchr)(s2, *s1) != NULL) {
    ++s1;
  }
  //
  // If end of string, no further tokens.
  //
  if (*s1 == '\0') {
    return 0;
  }
  //
  // Save start of token.
  //
  start = s1;
  //
  // Find end of token.
  //
  while (*s1 && (strchr)(s2, *s1) == NULL) {
    ++s1;
  }
  //
  if (*s1) {
    //
    // Break string.
    //
    *s1++ = '\0';
  }
  //
  // Prime for next scan.
  //
  __SEGGER_RTL_strtok_state = s1;
  //
  // Return start of token.
  //
  return start;
}

/*********************************************************************
*
*       strtok_r()
*
*  Function description
*    Break string into tokens, restartable (POSIX.1).
*
*  Parameters
*    s1    - Pointer to zero-terminated string to parse.
*    s2    - Pointer to zero-terminated set of separators.
*    lasts - Pointer to pointer to char that maintains parse state.
*
*  Return value
*    NULL if no further tokens else a pointer to the next token.
*
*  Additional information
*    strtok_r() is a restartable version of the function strtok()
*    where the state is maintained in the object of type char *
*    pointed to by s3.
*    
*  Conformance
*    POSIX.1-2008.
*
*  Thread safety
*    Safe.
*
*  See also
*    strtok()
*/
char * __SEGGER_RTL_PUBLIC_API strtok_r(char *s1, const char *s2, char **lasts) {
  char *start;
  //
  // Null input starts scanning from end of last token.
  //
  if (!s1) {
    s1 = *lasts;
  }
  //
  // Skip leading separators.
  //
  while (*s1 && (strchr)(s2, *s1) != NULL) {
    s1++;
  }
  //
  // If end of string, no further tokens.
  //
  if (*s1 == '\0') {
    return 0;
  }
  //
  // Save start of token.
  //
  start = s1;
  //
  // Find end of token.
  //
  while (*s1 && (strchr)(s2, *s1) == NULL) {
    s1++;
  }
  //
  if (*s1) {
    //
    // Break string.
    //
    *s1 = '\0';
    //
    // Prime for next scan.
    //
    *lasts = s1+1;
  } else {
    *lasts = s1;
  }
  //
  // Return start of token.
  //
  return start;
}

/*********************************************************************
*
*       strerror()
*
*  Function description
*    Decode error code.
*
*  Parameters
*    num - Error number.
*
*  Return value
*    Returns a pointer to the message string.
*    The program must not modify the returned message string.
*    The message may be overwritten by a subsequent call to strerror().
*
*  Additional information
*    Maps the number in num to a message string. Typically, 
*    the values for num come from errno, but strerror() can 
*    map any value of type int to a message.
*
*  Thread safety
*    Safe.
*/
char * __SEGGER_RTL_PUBLIC_API strerror(int num){
  const char *s;
  //
  // Decode standard libc errors.  Don't use case statement
  // as EABIness has non-const Exxx macros.
  //
  if      (num == 0)      { s = "success";                }
  else if (num == EDOM)   { s = "domain error";           }
  else if (num == EILSEQ) { s = "illegal sequence error"; }
  else if (num == ERANGE) { s = "range error";            }
  else if (num == EHEAP)  { s = "corrupt heap";           }
  else if (num == ENOMEM) { s = "no memory";              }
  else if (num == EINVAL) { s = "invalid argument";       }
  else                    { s = "unknown error";          }
  //
  return (char *)s;
}

#if __SEGGER_RTL_INCLUDE_AEABI_API && !__SEGGER_RTL_STRING_ASM

/*********************************************************************
*
*       __aeabi_memcpy()
*
*  Function description
*    Copy memory.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of __aeabi_memcpy() is
*    undefined if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memcpy(void *s1, const void *s2, size_t n) {
  __SEGGER_RTL_memcpy_inline(s1, s2, n);
}

/*********************************************************************
*
*       __aeabi_memcpy4()
*
*  Function description
*    Copy memory, 4-byte-aligned inputs.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of __aeabi_memcpy() is
*    undefined if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memcpy4(void *s1, const void *s2, size_t n) {
  __aeabi_memcpy(s1, s2, n);
}

/*********************************************************************
*
*       __aeabi_memcpy8()
*
*  Function description
*    Copy memory, 8-byte-aligned inputs.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of __aeabi_memcpy() is
*    undefined if copying takes place between objects that overlap.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memcpy8(void *s1, const void *s2, size_t n) {
  __aeabi_memcpy(s1, s2, n);
}

/*********************************************************************
*
*       __aeabi_memmove()
*
*  Function description
*    Copy memory, tolerate overlaps.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1 ensuring that if s1 and s2 overlap,
*    the copy works correctly. Copying takes place as if the n
*    characters from the object pointed to by s2 are first copied
*    into a temporary array of n characters that does not overlap
*    the objects pointed to by s1 and s2, and then the n characters
*    from the temporary array are copied into the object pointed to
*    by s1.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memmove(void *s1, const void *s2, size_t n) {
  __SEGGER_RTL_memmove_inline(s1, s2, n);
}

/*********************************************************************
*
*       __aeabi_memmove4()
*
*  Function description
*    Copy memory, tolerate overlaps, 4-byte-aligned inputs.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1 ensuring that if s1 and s2 overlap,
*    the copy works correctly. Copying takes place as if the n
*    characters from the object pointed to by s2 are first copied
*    into a temporary array of n characters that does not overlap
*    the objects pointed to by s1 and s2, and then the n characters
*    from the temporary array are copied into the object pointed to
*    by s1.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memmove4(void *s1, const void *s2, size_t n) {
  __aeabi_memmove(s1, s2, n);
}

/*********************************************************************
*
*       __aeabi_memmove8()
*
*  Function description
*    Copy memory, tolerate overlaps, 8-byte-aligned inputs.
*
*  Parameters
*    s1 - Pointer to destination object.
*    s2 - Pointer to source object.
*    n  - Number of characters to copy.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1 ensuring that if s1 and s2 overlap,
*    the copy works correctly. Copying takes place as if the n
*    characters from the object pointed to by s2 are first copied
*    into a temporary array of n characters that does not overlap
*    the objects pointed to by s1 and s2, and then the n characters
*    from the temporary array are copied into the object pointed to
*    by s1.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memmove8(void *s1, const void *s2, size_t n) {
  __aeabi_memmove(s1, s2, n);
}

/*********************************************************************
*
*       __aeabi_memset()
*
*  Function description
*    Set memory to character.
*
*  Parameters
*    s - Pointer to destination object.
*    n - Length of destination object in characters.
*    c - Character to copy.
*
*  Additional information
*    Copies the value of c (converted to an unsigned char) 
*    into each of the first n characters of the object pointed to by s.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_NO_BUILTIN __aeabi_memset(void *s, size_t n, int c) {
  __SEGGER_RTL_memset_inline(s, c, n);
}

/*********************************************************************
*
*       __aeabi_memset4()
*
*  Function description
*    Set memory to character, 4-byte-aligned inputs.
*
*  Parameters
*    s - Pointer to destination object.
*    n - Length of destination object in characters.
*    c - Character to copy.
*
*  Additional information
*    Copies the value of c (converted to an unsigned char) 
*    into each of the first n characters of the object pointed to by s.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_NO_BUILTIN __aeabi_memset4(void *s, size_t n, int c) {
  __aeabi_memset(s, n, c);
}

/*********************************************************************
*
*       __aeabi_memset8()
*
*  Function description
*    Set memory to character, 8-byte-aligned inputs.
*
*  Parameters
*    s - Pointer to destination object.
*    n - Length of destination object in characters.
*    c - Character to copy.
*
*  Additional information
*    Copies the value of c (converted to an unsigned char) 
*    into each of the first n characters of the object pointed to by s.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_NO_BUILTIN __aeabi_memset8(void *s, size_t n, int c) {
  __aeabi_memset(s, n, c);
}

/*********************************************************************
*
*       __aeabi_memclr()
*
*  Function description
*    Set memory to zero.
*
*  Parameters
*    s - Pointer to destination object.
*    n - Length of destination object in characters.
*
*  Additional information
*    Copies zero into each of the first n characters of the object
*    pointed to by s.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memclr(void *s, size_t n) {
  __aeabi_memset(s, n, 0);
}

/*********************************************************************
*
*       __aeabi_memclr4()
*
*  Function description
*    Set memory to zero, 4-byte-aligned input.
*
*  Parameters
*    s - Pointer to destination object.
*    n - Length of destination object in characters.
*
*  Additional information
*    Copies zero into each of the first n characters of the object
*    pointed to by s.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memclr4(void *s, size_t n) {
  __aeabi_memset(s, n, 0);
}

/*********************************************************************
*
*       __aeabi_memclr8()
*
*  Function description
*    Set memory to zero, 8-byte-aligned input.
*
*  Parameters
*    s - Pointer to destination object.
*    n - Length of destination object in characters.
*
*  Additional information
*    Copies zero into each of the first n characters of the object
*    pointed to by s.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API __aeabi_memclr8(void *s, size_t n) {
  __aeabi_memset(s, n, 0);
}

#endif

/*************************** End of file ****************************/
