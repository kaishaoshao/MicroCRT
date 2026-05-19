/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_ASSERT_H
#define __SEGGER_RTL_ASSERT_H

#include "__SEGGER_RTL.h"

#undef assert

#ifdef NDEBUG
  #define assert(ignore) ((void)0)
#else
  #define assert(e) ((e) ? (void)0 : __SEGGER_RTL_X_assert(#e, __FILE__, __LINE__))
#endif

#if (__STDC_VERSION__ >= __SEGGER_RTL_STDC_VERSION_C11) && !defined(__cplusplus)
  #define static_assert _Static_assert
#endif

#endif
