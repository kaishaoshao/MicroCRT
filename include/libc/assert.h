//===-- include/libc/assert.h - MicroCRT Program Assertion ------*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the standard assert macro and fault callback stub.
///
//===----------------------------------------------------------------------===//

#ifndef __ASSERT_H__
#define __ASSERT_H__

#ifdef __cplusplus
extern "C" {
#endif

/// Low-level handler invoked when an assertion expression evaluates to false.
void __assert_fail(const char *assertion, const char *file, unsigned int line,
                   const char *function);

#undef assert

#ifdef NDEBUG
/// In release/NDEBUG mode, assertions are compiled out.
#define assert(expr) ((void)0)
#else
/// Evaluates expression and calls __assert_fail upon failure.
#define assert(expr)                                                           \
  ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__, __func__))
#endif

#ifdef __cplusplus
}
#endif

#endif // __ASSERT_H__
