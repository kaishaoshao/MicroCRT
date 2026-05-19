/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_STDDEF_H
#define __SEGGER_RTL_STDDEF_H

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#ifndef NULL

/*********************************************************************
*
*       NULL
*
*  Description
*    Null-pointer constant.
*/
#define NULL 0            // Null pointer

#endif

/*********************************************************************
*
*       offsetof
*
*  Description
*    Calculate offset of member from start of structure.
*
*  Parameters
*    s - Type name of structure.
*    m - Name of member within structure.
*/
#define offsetof(s,m) __SEGGER_RTL_OFFSETOF(s, m)

/*********************************************************************
*
*       Data types
*
**********************************************************************
*/

#ifndef __SEGGER_RTL_SIZE_T_DEFINED
#define __SEGGER_RTL_SIZE_T_DEFINED

/*********************************************************************
*
*       size_t
*
*  Description
*    Unsigned integral type returned by the sizeof operator.
*/
typedef __SEGGER_RTL_SIZE_T size_t;

#endif

#if !defined(__SEGGER_RTL_WCHAR_T_DEFINED) && !defined(__cplusplus)
#define __SEGGER_RTL_WCHAR_T_DEFINED

/*********************************************************************
*
*       wchar_t
*
*  Description
*    Integral type that can hold one wide character.
*/
typedef __SEGGER_RTL_WCHAR_T wchar_t;

#endif

/*********************************************************************
*
*       ptrdiff_t
*
*  Description
*    Signed integral type of the result of subtracting two pointers.
*/
typedef __SEGGER_RTL_PTRDIFF_T ptrdiff_t;

/*********************************************************************
*
*       max_align_t
*
*  Description
*    Type whose alignment requirement is at least as strict as that
*    of every scalar type.
*
*  Additional information
*    'double' works for Arm and RISC-V targets.
*/
typedef double max_align_t;

#endif

/*************************** End of file ****************************/
