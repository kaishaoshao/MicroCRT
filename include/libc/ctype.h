//===-- include/libc/ctype.h - MicroCRT Character Handling ------*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares character classification and case-mapping functions.
///
//===----------------------------------------------------------------------===//

#ifndef __CTYPE_H__
#define __CTYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

//===----------------------------------------------------------------------===//
// Character Classification Functions
//===----------------------------------------------------------------------===//

/// Checks for alphanumeric character: equivalent to (isalpha(c) || isdigit(c)).
int isalnum(int c);

/// Checks for alphabetic character: equivalent to (isupper(c) || islower(c)).
int isalpha(int c);

/// Checks for a blank character: either space or horizontal tab.
int isblank(int c);

/// Checks for any control character (ASCII 0x00..0x1F and 0x7F).
int iscntrl(int c);

/// Checks for a decimal digit: '0' through '9'.
int isdigit(int c);

/// Checks for any printable character except space.
int isgraph(int c);

/// Checks for a lowercase alphabetic letter: 'a' through 'z'.
int islower(int c);

/// Checks for any printable character including space.
int isprint(int c);

/// Checks for any printable character that is not space or alphanumeric.
int ispunct(int c);

/// Checks for white-space character: space, '\t', '\n', '\v', '\f', '\r'.
int isspace(int c);

/// Checks for an uppercase alphabetic letter: 'A' through 'Z'.
int isupper(int c);

/// Checks for a hexadecimal digit: '0'..'9', 'a'..'f', 'A'..'F'.
int isxdigit(int c);

//===----------------------------------------------------------------------===//
// Character Case Mapping Functions
//===----------------------------------------------------------------------===//

/// Converts an uppercase letter to its corresponding lowercase letter.
int tolower(int c);

/// Converts a lowercase letter to its corresponding uppercase letter.
int toupper(int c);

//===----------------------------------------------------------------------===//
// ASCII Extensions
//===----------------------------------------------------------------------===//

/// Tests whether c fits into the 7-bit US-ASCII character set (0..127).
int isascii(int c);

/// Truncates c to a 7-bit unsigned char value (c & 0x7F).
int toascii(int c);

#ifdef __cplusplus
}
#endif

#endif // __CTYPE_H__
