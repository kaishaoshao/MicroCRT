/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_SIGNAL_H
#define __SEGGER_RTL_SIGNAL_H

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
*       Signal numbers
*
*  Description
*    Signals that may be raised.
*/
#define SIGABRT     __SEGGER_RTL_SIGNAL_SIGABRT     // Abnormal termination, initiated by abort()
#define SIGFPE      __SEGGER_RTL_SIGNAL_SIGFPE      // Floating-point exception
#define SIGILL      __SEGGER_RTL_SIGNAL_SIGILL      // Illegal instruction or similar
#define SIGINT      __SEGGER_RTL_SIGNAL_SIGINT      // Interactive attention signal
#define SIGSEGV     __SEGGER_RTL_SIGNAL_SIGSEGV     // Segmentation violation, invalid access
#define SIGTERM     __SEGGER_RTL_SIGNAL_SIGTERM     // Termination request

/*********************************************************************
*
*       Signal action and return values
*
*  Description
*    Special signal values that are compare not-equal to any valid function
*    address.
*/
#define SIG_DFL     __SEGGER_RTL_SIGNAL_SIG_DFL     // Default signal handling
#define SIG_ERR     __SEGGER_RTL_SIGNAL_SIG_ERR     // Error signal handling (abort)
#define SIG_IGN     __SEGGER_RTL_SIGNAL_SIG_IGN     // Ignored signal handling

/*********************************************************************
*
*       Data types
*
**********************************************************************
*/

/*********************************************************************
*
*       sig_atomic_t
*
*  Description
*    Atomic signal type.
*
*  Additional information
*    Describes a type that can be accessed as an atomic entity,
*    even in the presence of interrupts.
*/
typedef __SEGGER_RTL_SIG_ATOMIC_T sig_atomic_t;

/*********************************************************************
*
*       __SEGGER_RTL_SIGNAL_FUNC
*
*  Description
*    Prorotype for signal function
*
*  Parameters
*    sig - Signal number, one of SIGABRT, SIGFPE, SIGILL, SIGSEGV,
*          or SIGTERM.
*/
typedef void __SEGGER_RTL_SIGNAL_FUNC(int sig);

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

__SEGGER_RTL_SIGNAL_FUNC * signal  (int __sig, __SEGGER_RTL_SIGNAL_FUNC *__func);
int                        raise   (int __sig);

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
