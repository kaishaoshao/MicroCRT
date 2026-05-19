/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_CTYPE_H
#define __SEGGER_RTL_CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

int isalpha (int __c);
int isupper (int __c);
int islower (int __c);
int isdigit (int __c);
int isxdigit(int __c);
int isspace (int __c);
int ispunct (int __c);
int isalnum (int __c);
int isprint (int __c);
int isgraph (int __c);
int iscntrl (int __c);
int toupper (int __c);
int tolower (int __c);
int isblank (int __c);  /* C99 */
int isascii (int __c);  /* Common Unix, Linux & BSD */

#ifndef __EXCLUDE_POSIX

#include "__SEGGER_RTL.h"

#ifndef __SEGGER_RTL_LOCALE_T_DEFINED
#define __SEGGER_RTL_LOCALE_T_DEFINED
typedef struct __SEGGER_RTL_POSIX_locale_s *locale_t;
#endif

int isalpha_l (int __c, locale_t __loc);
int isupper_l (int __c, locale_t __loc);
int islower_l (int __c, locale_t __loc);
int isdigit_l (int __c, locale_t __loc);
int isxdigit_l(int __c, locale_t __loc);
int isspace_l (int __c, locale_t __loc);
int ispunct_l (int __c, locale_t __loc);
int isalnum_l (int __c, locale_t __loc);
int isprint_l (int __c, locale_t __loc);
int isgraph_l (int __c, locale_t __loc);
int iscntrl_l (int __c, locale_t __loc);
int toupper_l (int __c, locale_t __loc);
int tolower_l (int __c, locale_t __loc);
int isblank_l (int __c, locale_t __loc);  /* C99 */
int isascii_l (int __c, locale_t __loc);  /* Common Unix, Linux & BSD */

#endif

#ifdef __cplusplus
}
#endif

#endif
