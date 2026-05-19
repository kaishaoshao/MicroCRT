/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_STDLIB_H
#define __SEGGER_RTL_STDLIB_H

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "__SEGGER_RTL.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Defined, fixed
*
**********************************************************************
*/

#ifndef NULL
#define NULL 0
#endif

#ifndef __SEGGER_RTL_SIZE_T_DEFINED
#define __SEGGER_RTL_SIZE_T_DEFINED
typedef __SEGGER_RTL_SIZE_T size_t;
#endif

#if !defined(__SEGGER_RTL_WCHAR_T_DEFINED) && !defined(__cplusplus)
#define __SEGGER_RTL_WCHAR_T_DEFINED
typedef __SEGGER_RTL_WCHAR_T wchar_t;
#endif

#ifndef __SEGGER_RTL_LOCALE_T_DEFINED
#define __SEGGER_RTL_LOCALE_T_DEFINED
typedef struct __SEGGER_RTL_POSIX_locale_s *locale_t;
#endif

#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1
#define RAND_MAX    32767
#define MB_CUR_MAX  __SEGGER_RTL_mb_cur_max()

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  int quot;
  int rem;
} div_t;

typedef struct {
  long quot;
  long rem;
} ldiv_t;

typedef struct {
  long long quot;
  long rem;
} lldiv_t;

typedef struct {
  __SEGGER_RTL_INTMAX_T quot;
  __SEGGER_RTL_INTMAX_T rem;
} imaxdiv_t;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

int                      abs            (int __j);
int                      at_quick_exit  (void (*__func)(void));
long int                 labs           (long int __j);
long long int            llabs          (long long int __j);
div_t                    div            (int __numer, int __denom);
ldiv_t                   ldiv           (long int __numer, long int __denom);
lldiv_t                  lldiv          (long long int __numer, long long int __denom);
imaxdiv_t                imaxdiv        (__SEGGER_RTL_INTMAX_T __numer, __SEGGER_RTL_INTMAX_T __denom);
void                   * malloc         (size_t __size);
void                   * aligned_alloc  (size_t alignment, size_t __size);
void                   * calloc         (size_t __nobj, size_t __size);
void                   * realloc        (void *__p, size_t __size);
void                     free           (void *__p);
double                   atof           (const char *__nptr);
double                   strtod         (const char *__nptr, char **__endptr);
float                    strtof         (const char *__nptr, char **__endptr);
long double              strtold        (const char *__nptr, char **__endptr);
int                      atoi           (const char *__nptr);
long int                 atol           (const char *__nptr);
long long int            atoll          (const char *__nptr);
long int                 strtol         (const char *__nptr, char **__endptr, int __base);
long long int            strtoll        (const char *__nptr, char **__endptr, int __base);
unsigned long int        strtoul        (const char *__nptr, char **__endptr, int __base);
unsigned long long int   strtoull       (const char *__nptr, char **__endptr, int __base);
int                      rand           (void);
void                     srand          (unsigned int __seed);
void                   * bsearch        (const void *__key, const void *__buf, size_t __num, size_t __size, int (*__compare)(const void *, const void *));
void                     qsort          (void *__buf, size_t __num, size_t __size, int (*__compare)(const void *, const void *));
int                      atexit         (void (*__func)(void));
char                   * getenv         (const char *__name);
int                      system         (const char *__command);
char                   * itoa           (int __val, char *__buf, int __radix);
char                   * utoa           (unsigned val, char *buf, int radix);
char                   * ltoa           (long __val, char *__buf, int __radix);
char                   * ultoa          (unsigned long __val, char *__buf, int __radix);
char                   * lltoa          (long long __val, char *__buf, int __radix);
char                   * ulltoa         (unsigned long long __val, char *__buf, int __radix);
int                      mblen          (const char *__s, size_t __n);
int                      mblen_l        (const char *__s, size_t __n, locale_t __loc);
size_t                   mbstowcs       (wchar_t *__pwcs, const char *__s, size_t __n);
size_t                   mbstowcs_l     (wchar_t *__pwcs, const char *__s, size_t __n, locale_t __loc);
int                      mbtowc         (wchar_t *__pwc, const char *__s, size_t __n);
int                      mbtowc_l       (wchar_t *__pwc, const char *__s, size_t __n, locale_t __loc);
int                      wctomb         (char *__s, wchar_t __wc);
int                      wctomb_l       (char *__s, wchar_t __wc, locale_t __loc);
size_t                   wcstombs       (char *__s, const wchar_t *__pwcs, size_t __n);
size_t                   wcstombs_l     (char *__s, const wchar_t *__pwcs, size_t __n, locale_t __loc);

__SEGGER_RTL_NO_RETURN void abort       (void);
__SEGGER_RTL_NO_RETURN void exit        (int __exit_code);
__SEGGER_RTL_NO_RETURN void _exit       (int __exit_code);
__SEGGER_RTL_NO_RETURN void _Exit       (int __exit_code);
__SEGGER_RTL_NO_RETURN void quick_exit  (int __exit_code);

#if defined(__STDC_WANT_LIB_EXT1__) && (__STDC_WANT_LIB_EXT1__ == 1)

#ifndef __SEGGER_RTL_ERRNO_T_DEFINED
#define __SEGGER_RTL_ERRNO_T_DEFINED
typedef int errno_t;
#endif

typedef void (*constraint_handler_t)(const char *__msg, void *__ptr, errno_t __error);

constraint_handler_t     set_constraint_handler_s (constraint_handler_t __handler);
void                     abort_handler_s          (const char *__msg, void *__ptr, errno_t __error);
void                     ignore_handler_s         (const char *__msg, void *__ptr, errno_t __error);

#endif

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
