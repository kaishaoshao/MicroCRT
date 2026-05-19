/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_ERRNO_H
#define __SEGGER_RTL_ERRNO_H

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Error names
*
*  Description
*    Symbolic error names for raised errors.
*/
#define EDOM      0x01   // Domain error
#define EILSEQ    0x02   // Illegal multibyte sequence in conversion
#define ERANGE    0x03   // Range error
#define EHEAP     0x04   // Heap is corrupt (emRun)
#define ENOMEM    0x05   // Out of memory (POSIX.1-2001)
#define EINVAL    0x06   // Invalid parameter (POSIX.1-2001)
#define ESPIPE    0x07   // Invalid seek (POSIX.1-2001)
#define EAGAIN    0x08   // Resource unavailable, try again (POSIX.1-2001)
#define ECHILD    0x09   // No child processes (POSIX.1-2001)
#define EMLINK    0x0A   // Too many links (POSIX.1-2001)
#define ENOENT    0x0B   // No such file or directory (POSIX.1-2001)
#define EEXIST    0x0C
#define ENODEV    0x0D
#define EBUSY     0x0E
#define ENOSYS    0x0F
#define ENOSPC    0x10
#define EFAULT    0x11
#define ENOTSUP   0x12
#define ENOBUFS   0x13
#define ETIMEDOUT 0x14
#define ENOMSG    0x15
#define EPERM     0x16
#define EDEADLK   0x17
#define EALREADY  0x18

// Modify for AEABI compliance
#if defined(_AEABI_PORTABILITY_LEVEL) && (_AEABI_PORTABILITY_LEVEL != 0)

extern const int __aeabi_EDOM;
extern const int __aeabi_EILSEQ;
extern const int __aeabi_ERANGE;

#undef EDOM
#undef EILSEQ
#undef ERANGE

#define EDOM     (__aeabi_EDOM)
#define EILSEQ   (__aeabi_EILSEQ)
#define ERANGE   (__aeabi_ERANGE)

#endif


/*********************************************************************
*
*       errno
*
*  Description
*    Macro returning the current error.
*
*  Additional information
*    The value in errno is significant only when the return value of the
*    call indicated an error.  A function that succeeds is allowed to
*    change errno.  The value of errno is never set to zero by a library
*    function.
*
*  Thread safety
*    Safe [if configured].
*/
#define errno   (*__SEGGER_RTL_X_errno_addr())

#if defined(__STDC_WANT_LIB_EXT1__) && (__STDC_WANT_LIB_EXT1__ == 1) && !defined(__SEGGER_RTL_ERRNO_T_DEFINED)
/*********************************************************************
*
*       errno_t
*
*  Description
*    Type describing errors (C11).
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <errno.h> to access this type.
*
*    This type is used by the C11/C18 bounds-checking functions.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*/
typedef int errno_t;
//
#define __SEGGER_RTL_ERRNO_T_DEFINED
#endif

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
