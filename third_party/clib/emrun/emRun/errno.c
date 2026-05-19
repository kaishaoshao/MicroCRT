/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Int.h"
#include "errno.h"

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

volatile int * __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_errno_addr(void);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static __SEGGER_RTL_STATE_THREAD volatile int __SEGGER_RTL_errno;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_X_errno_addr()
*
*  Function description
*    Return pointer to object holding errno.
*
*  Return value
*    Pointer to errno object.
*
*  Additional information
*    The default implementation of this function is to return the
*    address of a variable declared with the __SEGGER_RTL_STATE_THREAD
*    storage class.  Thus, for multithreaded environments that
*    implement thread-local variables through __SEGGER_RTL_STATE_THREAD,
*    each thread receives its own thread-local errno.
*
*    It is beyond the scope of this manual to describe how
*    thread-local variables are implemented by the compiler and
*    any associated real-time operating system.
*
*    When __SEGGER_RTL_STATE_THREAD is defined as an empty macro, this
*    function returns the address of a singleton errno object.
*/
volatile int * __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_errno_addr(void) {
  return &__SEGGER_RTL_errno;
}

/*************************** End of file ****************************/
