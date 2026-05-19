/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File        : phantomops.c
Purpose     : Placeholders for documentation, DO NOT COMPILE.

*/

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#if 0  // This is for documentation only

/*********************************************************************
*
*       assert
*
*  Function description
*    Place assertion.
*
*  Parameters
*    e - expression that should evaluate nonzero.
*
*  Additional information
*    If \tt{NDEBUG} is defined as a macro name at the point in the source file 
*    where \tt{<assert.h>} is included, the \tt{assert()} macro is defined 
*    as: 
*    
*    \tt{#define assert(ignore) ((void)0)}
*    
*    If \tt{NDEBUG} is not defined as a macro name at the point in the source 
*    file where \tt{<assert.h>} is included, the \tt{assert()} macro expands 
*    to a void expression that calls __SEGGER_RTL_X_assert(). 
*    
*    When such an assert is executed and e is false, \tt{assert()} calls the
*    function __SEGGER_RTL_X_assert() with information about the particular
*    call that failed: the text of the argument, the name of the source
*    file, and the source line number. These are the stringized expression
*    and the values of the preprocessing macros __FILE__ and __LINE__.
*    
*  Notes
*    The \tt{assert()} macro is redefined according to the current state of
*    \tt{NDEBUG} each time that \tt{<assert.h>} is included.
*/
#define assert(e) ...

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       setjmp()
*
*  Function description
*    Save calling environment for non-local jump.
*
*  Parameters
*    buf - Buffer to save context into.
*
*  Return value
*    On return from a direct invocation, returns the value zero. 
*    On return from a call to the longjmp() function, returns a
*    nonzero value determined by the call to longjmp().
*
*  Additional information
*    Saves its calling environment in env for later use by the
*    longjmp() function.
*
*    The environment saved by a call to setjmp () consists of
*    information sufficient for a call to the longjmp() function
*    to return  execution to the correct block and invocation of
*    that block, were it called recursively.
*
*  Thread safety
*    Safe.
*/
int setjmp(jmp_buf buf) {
  /* Empty */
}

/*********************************************************************
*
*       longjmp()
*
*  Function description
*    Restores the saved environment.
*
*  Parameters
*    buf - Buffer to restore context from.
*    val - Value to return to setjmp() call.
*
*  Additional information
*    Restores the environment saved by setjmp() in the corresponding
*    env argument. If there has been no such invocation, or if the
*    function containing the invocation of setjmp() has terminated
*    execution in the interim, the behavior of longjmp() is undefined.
*    
*    After longjmp() is completed, program execution continues as if
*    the corresponding invocation of setjmp() had just returned the
*    value specified by val. 
*    
*    Objects of automatic storage allocation that are local to the
*    function containing the invocation of the corresponding setjmp()
*    that do not have volatile-qualified type and have been changed
*    between the setjmp() invocation and longjmp() call are indeterminate.
*
*  Notes
*    longjmp() cannot cause setjmp() to return the value 0; if 
*    val is 0, setjmp() returns the value 1.
*
*  Thread safety
*    Safe.
*/
void longjmp(jmp_buf buf, int val) {
  // pass
}

/*********************************************************************
*
*       __SEGGER_RTL_X_atomic_lock()
*
*  Function description
*    Lock for atomic access.
*
*  Additional information
*    This function is called to provide exclusive access to an object.
*    Typically, interrupts are disabled by the function, but in a
*    multi-tasking system a mutex can be used.
*
*  Return value
*    User-defined value to be passed to matching atomic unlock.
*/
int __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_atomic_lock(void) {
  /* Pass */
}

/*********************************************************************
*
*       __SEGGER_RTL_X_atomic_unlock()
*
*  Function description
*    Unlock atomic access.
*
*  Parameters
*    state - Value returned by __SEGGER_RTL_X_atomic_lock().
*
*  Additional information
*    This function is called to relinquish previously-locked atomic
*    access.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_atomic_unlock(int state) {
  /* Pass */
}

/*********************************************************************
*
*       __SEGGER_RTL_X_atomic_synchronize()
*
*  Function description
*    Full memory barrier.
*
*  Additional information
*    This function is called to ensure all memory reads and writes
*    are complete before continuing.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_atomic_synchronize(void) {
  /* Pass */
}

#endif

/*************************** End of file ****************************/
