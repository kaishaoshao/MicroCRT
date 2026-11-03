//===-- include/libc/stdlib.h - MicroCRT General Utilities ------*- C -*-===//
//
// Part of the MicroCRT Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares general utility functions, memory allocation,
/// integer arithmetic, search/sort algorithms, and process control.
///
//===----------------------------------------------------------------------===//

#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

//===----------------------------------------------------------------------===//
// Standard Types and Return Structures
//===----------------------------------------------------------------------===//

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
/// Unsigned integer type returned by the sizeof operator.
typedef unsigned long size_t;
#endif

#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED
/// Integral type whose range of values can represent distinct wide characters.
typedef int wchar_t;
#endif

/// Return type of div() containing quotient and remainder.
typedef struct {
  int quot;
  int rem;
} div_t;

/// Return type of ldiv() containing long quotient and remainder.
typedef struct {
  long quot;
  long rem;
} ldiv_t;

/// Return type of lldiv() containing long long quotient and remainder.
typedef struct {
  long long quot;
  long long rem;
} lldiv_t;

//===----------------------------------------------------------------------===//
// Standard Constants
//===----------------------------------------------------------------------===//

/// Successful program execution status.
#define EXIT_SUCCESS 0
/// Unsuccessful program execution status.
#define EXIT_FAILURE 1

/// Maximum value returned by rand().
#define RAND_MAX 32767
/// Maximum number of bytes in a multibyte character.
#define MB_CUR_MAX 1

/// Null pointer constant.
#undef NULL
#define NULL ((void *)0)

//===----------------------------------------------------------------------===//
// 1. String to Number Conversions
//===----------------------------------------------------------------------===//

/// Converts the initial portion of str to double.
double atof(const char *str);

/// Converts the initial portion of str to int.
int atoi(const char *str);

/// Converts the initial portion of str to long.
long atol(const char *str);

/// Converts the initial portion of str to long long.
long long atoll(const char *str);

/// Converts a string to a double, reporting any unparsed trailing characters.
double strtod(const char *str, char **endptr);

/// Converts a string to a float, reporting any unparsed trailing characters.
float strtof(const char *str, char **endptr);

/// Converts a string to a long double.
long double strtold(const char *str, char **endptr);

/// Converts a string to a long according to the given numeric base.
long strtol(const char *str, char **endptr, int base);

/// Converts a string to a long long according to the given numeric base.
long long strtoll(const char *str, char **endptr, int base);

/// Converts a string to an unsigned long according to the given base.
unsigned long strtoul(const char *str, char **endptr, int base);

/// Converts a string to an unsigned long long according to the given base.
unsigned long long strtoull(const char *str, char **endptr, int base);

//===----------------------------------------------------------------------===//
// 2. Pseudo-Random Number Generation
//===----------------------------------------------------------------------===//

/// Generates a pseudo-random integer in the range [0, RAND_MAX].
int rand(void);

/// Seeds the pseudo-random number generator used by rand().
void srand(unsigned int seed);

//===----------------------------------------------------------------------===//
// 3. Dynamic Memory Management
//===----------------------------------------------------------------------===//

/// Allocates size bytes of uninitialized memory.
void *malloc(size_t size);

/// Allocates zero-initialized memory for an array of num elements.
void *calloc(size_t num, size_t size);

/// Reallocates the memory block ptr to new_size bytes.
void *realloc(void *ptr, size_t new_size);

/// Deallocates the memory block pointed to by ptr.
void free(void *ptr);

/// Allocates size bytes with the specified byte alignment.
void *aligned_alloc(size_t alignment, size_t size);

//===----------------------------------------------------------------------===//
// 4. Process and Program Control
//===----------------------------------------------------------------------===//

/// Aborts execution by raising SIGABRT and terminating.
void abort(void);

/// Terminates the process normally, calling registered atexit handlers.
void exit(int status);

/// Terminates the process immediately without calling atexit handlers.
void _Exit(int status);

/// Registers a function to be called at normal program termination.
int atexit(void (*func)(void));

/// Passes command to the host command execution environment.
int system(const char *command);

/// Retrieves the value of the environment variable named by name.
char *getenv(const char *name);

//===----------------------------------------------------------------------===//
// 5. Search and Sorting Algorithms
//===----------------------------------------------------------------------===//

/// Performs binary search for key in sorted array base of nitems elements.
void *bsearch(const void *key, const void *base, size_t nitems, size_t size,
              int (*compar)(const void *, const void *));

/// Sorts the array base of nitems elements according to compar.
void qsort(void *base, size_t nitems, size_t size,
           int (*compar)(const void *, const void *));

//===----------------------------------------------------------------------===//
// 6. Integer Arithmetic Functions
//===----------------------------------------------------------------------===//

/// Computes the absolute value of integer n.
int abs(int n);

/// Computes the absolute value of long integer n.
long labs(long n);

/// Computes the absolute value of long long integer n.
long long llabs(long long n);

/// Computes numerator / denominator and numerator % denominator simultaneously.
div_t div(int numer, int denom);

/// Computes long integer division and remainder simultaneously.
ldiv_t ldiv(long numer, long denom);

/// Computes long long integer division and remainder simultaneously.
lldiv_t lldiv(long long numer, long long denom);

//===----------------------------------------------------------------------===//
// 7. Low-Level Target Retargeting Stubs
//===----------------------------------------------------------------------===//

/// Terminates the execution environment passing returncode to host/debugger.
void _sys_exit(int returncode);

/// Requests additional heap memory blocks from the platform memory manager.
size_t __user_heap_extend(int ignore, void **base, size_t requestedsize);

#ifdef __cplusplus
}
#endif

#endif // __STDLIB_H__
