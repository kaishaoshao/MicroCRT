//===-- include/libc/string.h - MicroCRT String Operations ------*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares memory copying, comparison, setting, search routines,
/// and standard C string manipulation operations.
///
//===----------------------------------------------------------------------===//

#ifndef __STRING_H__
#define __STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

//===----------------------------------------------------------------------===//
// Standard Types and Constants
//===----------------------------------------------------------------------===//

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
/// Unsigned integer type returned by the sizeof operator.
typedef unsigned long size_t;
#endif

/// Null pointer constant.
#undef NULL
#define NULL ((void *)0)

//===----------------------------------------------------------------------===//
// 1. Memory Block Operations
//===----------------------------------------------------------------------===//

/// Copies n bytes from memory area s2 to s1 (non-overlapping).
void *memcpy(void *s1, const void *s2, size_t n);

/// Copies n bytes from s2 to s1, returning a pointer to following byte.
void *mempcpy(void *s1, const void *s2, size_t n);

/// Copies at most n bytes from s2 to s1, stopping after character c is copied.
void *memccpy(void *s1, const void *s2, int count, size_t n);

/// Copies n bytes from s2 to s1, handling potential memory overlap.
void *memmove(void *s1, const void *s2, size_t n);

/// Fills the first n bytes of memory area s with constant byte c.
void *memset(void *s, int c, size_t n);

/// Compares the first n bytes of memory areas s1 and s2.
int memcmp(const void *s1, const void *s2, size_t n);

/// Computes length of common prefix between s1 and s2 up to n bytes.
size_t memlcp(const void *s1, const void *s2, size_t n);

/// Scans the first n bytes of s for the first occurrence of c.
void *memchr(const void *s, int c, size_t n);

/// Scans the first n bytes of s backward for the last occurrence of c.
void *memrchr(const void *s, int c, size_t n);

/// Locates the first occurrence of s2 (of length n2) in s1 (of length n1).
void *memmem(const void *s1, size_t n1, const void *s2, size_t n2);

//===----------------------------------------------------------------------===//
// 2. String Copy and Concatenation
//===----------------------------------------------------------------------===//

/// Copies null-terminated string s2 into buffer s1.
char *strcpy(char *s1, const char *s2);

/// Copies string s2 to s1, returning pointer to terminating null byte in s1.
char *strpcpy(char *s1, const char *s2);

/// Copies at most n bytes from s2 to s1, padding with null bytes if needed.
char *strncpy(char *s1, const char *s2, size_t n);

/// Copies at most n bytes from s2 to s1, returning pointer to end in s1.
char *stpncpy(char *s1, const char *s2, size_t n);

/// Appends string s2 to the end of string s1.
char *strcat(char *s1, const char *s2);

/// Appends at most n bytes from s2 to the end of string s1.
char *strncat(char *s1, const char *s2, size_t n);

/// Bounded string copy ensuring null termination.
size_t strlcpy(char *s1, const char *s2, size_t n);

/// Bounded string concatenation ensuring null termination.
size_t strlcat(char *s1, const char *s2, size_t n);

/// Allocates and creates a duplicate copy of string s.
char *strdup(const char *s);

/// Allocates and creates a duplicate copy of at most n characters from s.
char *strndup(const char *s, size_t n);

//===----------------------------------------------------------------------===//
// 3. String Comparison
//===----------------------------------------------------------------------===//

/// Compares string s1 to string s2 lexicographically.
int strcmp(const char *s1, const char *s2);

/// Compares at most n characters of string s1 to s2 lexicographically.
int strncmp(const char *s1, const char *s2, size_t n);

/// Compares s1 and s2 ignoring case.
int strcasecmp(const char *s1, const char *s2);

/// Compares at most n characters of s1 and s2 ignoring case.
int strncasecmp(const char *s1, const char *s2, size_t n);

/// Compares s1 and s2 using the current locale collation rules.
int strcoll(const char *s1, const char *s2);

/// Transforms s2 into a form such that strcmp matches strcoll.
size_t strxfrm(char *s1, const char *s2, size_t n);

//===----------------------------------------------------------------------===//
// 4. String Search and Tokenization
//===----------------------------------------------------------------------===//

/// Locates first occurrence of character c in string s.
char *strchr(const char *s, int c);

/// Locates last occurrence of character c in string s.
char *strrchr(const char *s, int c);

/// Locates first occurrence of c within first n characters of s.
char *strnchr(const char *s, size_t n, int c);

/// Locates first occurrence of substring s2 in string s1.
char *strstr(const char *s1, const char *s2);

/// Locates first occurrence of substring s2 in string s1 up to n characters.
char *strnstr(const char *s1, const char *s2, size_t n);

/// Locates first occurrence of substring s2 in string s1 ignoring case.
char *strcasestr(const char *s1, const char *s2);

/// Locates first occurrence of s2 in s1 within n characters ignoring case.
char *strncasestr(const char *s1, const char *s2, size_t n);

/// Locates first occurrence in s1 of any character from string s2.
char *strpbrk(const char *s1, const char *s2);

/// Calculates length of initial segment of s1 consisting entirely of chars in s2.
size_t strspn(const char *s1, const char *s2);

/// Calculates length of initial segment of s1 consisting of chars not in s2.
size_t strcspn(const char *s1, const char *s2);

/// Breaks string s1 into a sequence of tokens separated by chars in s2.
char *strtok(char *s1, const char *s2);

/// Reentrant version of strtok using state pointer lasts.
char *strtok_r(char *s1, const char *s2, char **lasts);

//===----------------------------------------------------------------------===//
// 5. String Length and Error Info
//===----------------------------------------------------------------------===//

/// Computes length of string s excluding terminating null byte.
size_t strlen(const char *s);

/// Computes length of s up to a maximum of n bytes.
size_t strnlen(const char *s, size_t n);

/// Returns a descriptive textual string corresponding to error code errnum.
char *strerror(int errnum);

/// Thread-safe version of strerror storing message in caller-supplied buffer.
int strerror_r(int errnum, char *buf, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif // __STRING_H__
