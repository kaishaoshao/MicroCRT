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
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "signal.h"

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

typedef struct {
  void                   * object;
  __SEGGER_RTL_dtor_func   func;
} SEGGER_RTL_dtor_entry;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

//
// Destructor array.
//
static SEGGER_RTL_dtor_entry __SEGGER_RTL_dtor_list[__SEGGER_RTL_ATEXIT_COUNT];

//
// Number of destructors to call.
//
#if __SEGGER_RTL_ATEXIT_COUNT < 255
static unsigned char __SEGGER_RTL_dtor_count;
#elif __SEGGER_RTL_ATEXIT_COUNT < 65535
static unsigned short __SEGGER_RTL_dtor_count;
#else
static unsigned __SEGGER_RTL_dtor_count;
#endif

//
// Registered signal handler.
//
static __SEGGER_RTL_SIGNAL_FUNC * __SEGGER_RTL_aSigTab[__SEGGER_RTL_SIGNAL_MAX];

//
// Registered constraint violation handler.
//
static constraint_handler_t __SEGGER_RTL_constraint_handler = abort_handler_s;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_puts_no_nl()
*
*  Function description
*    Write string to standard output, no automatic newline at end.
*
*  Parameters
*    s - Pointer to zero-terminated string.
*
*  Additional information
*    Writes the string pointed to by s to the standard output 
*    stream as if by repeated calls to putchar().
*/
static void __SEGGER_RTL_puts_no_nl(const char *s) {
  (void)__SEGGER_RTL_X_file_write(stdout, s, (strlen)(s));
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       abort()
*
*  Function description
*    Abort execution.
*
*  Additional information
*    Calls exit() with the exit status EXIT_FAILURE.
*
*  Thread safety
*    Not applicable.
*/
void __SEGGER_RTL_PUBLIC_API abort(void) {
#if defined(__SEGGER_RTL_HALT)
  __SEGGER_RTL_HALT(EXIT_FAILURE);
#else
  for (;;) {
    raise(SIGABRT);
  }
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_constraint_violation()
*
*  Function description
*    Call registered constraint violation handler.
*
*  Parameters
*    msg   - Pointer to a null-terminated character string describing the
*            constraint violation.
*    ptr   - A null pointer or a pointer to an implementation-defined
*            object.  This implementation always passes NULL.
*    error - If the function calling the handler has a return type declared
*            as errno_t, the return value of the function is passed.
*            Otherwise, a positive value of type errno_t is passed.
*
*  Thread safety
*    Not applicable.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_constraint_violation(const char *msg, void *ptr, errno_t error) {
  __SEGGER_RTL_constraint_handler(msg, ptr, error);
}

/*********************************************************************
*
*       __SEGGER_RTL_X_assert()
*
*  Function description
*    User-defined behavior for the assert macro.
*
*  Parameters
*    expr     - Stringized expression that caused failure.
*    filename - Filename of the source file where the failure was signaled.
*    line     - Line number of the failed assertion.
*
*  Additional information
*    The default implementation of __SEGGER_RTL_X_assert() prints the
*    filename, line, and error message to standard output and then
*    calls abort().
*
*    __SEGGER_RTL_X_assert() is defined as a weak function and can be
*    replaced by user code.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_assert(const char *expr, const char *filename, int line) {
  char acLine[16];
  //
  // We implement it like this so as not to unduly bring in formatting support.
  //
  itoa(line, acLine, 10);
  __SEGGER_RTL_puts_no_nl(filename);
  __SEGGER_RTL_puts_no_nl(":");
  __SEGGER_RTL_puts_no_nl(acLine);
  __SEGGER_RTL_puts_no_nl(": assertion failed: ");
  __SEGGER_RTL_puts_no_nl(expr);
  __SEGGER_RTL_puts_no_nl("\n");
  abort();
}

/*********************************************************************
*
*       __aeabi_atexit()
*
*  Function description
*    Register static destructor.
*
*  Parameters
*    object     - Pointer to object.
*    destructor - Pointer to function that destroys object.
*    dso_handle - Pointer to shared object handle.
*
*  Return value
*    == 0 - Registered OK.
*    != 0 - Registration failure.
*/
int __SEGGER_RTL_PUBLIC_API __aeabi_atexit(void *object, void (*destructor)(void *), void *dso_handle) {
  __SEGGER_RTL_USE_PARA(dso_handle);
  __SEGGER_RTL_USE_PARA(object);
  //
  if (__SEGGER_RTL_dtor_count < __SEGGER_RTL_ATEXIT_COUNT) {
    __SEGGER_RTL_dtor_list[__SEGGER_RTL_dtor_count].func   = destructor;
    __SEGGER_RTL_dtor_list[__SEGGER_RTL_dtor_count].object = NULL;
    __SEGGER_RTL_dtor_count++;
    return 0;
  }
  return 1;
}

/*********************************************************************
*
*       atexit()
*
*  Function description
*    Set function to be called on exit.
*
*  Parameters
*    fn - Function to register.
*
*  Additional information
*    Registers function fn to be called when the application has
*    exited. The functions registered with atexit() are executed
*    in reverse order of their registration.
*
*  Return value
*    == 0 - Success registering function.
*    != 0 - Did not register function.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API atexit(__SEGGER_RTL_exit_func fn) {
  return __aeabi_atexit(NULL, (__SEGGER_RTL_dtor_func)fn, NULL);
}

/*********************************************************************
*
*       __SEGGER_RTL_execute_at_exit_fns()
*
*  Function description
*    Execute at-exit functions.
*
*  Additional information
*    Executes all functions registered by calls to atexit() in reverse
*    order of their registration.  It does this in the caller's
*    execution context.
*
*  Thread safety
*    Not applicable.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_execute_at_exit_fns(void) {
  while (__SEGGER_RTL_dtor_count) {
    __SEGGER_RTL_dtor_count--;
    __SEGGER_RTL_dtor_list[__SEGGER_RTL_dtor_count].func(__SEGGER_RTL_dtor_list[__SEGGER_RTL_dtor_count].object);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_SIGNAL_SIG_DFL()
*
*  Function description
*    Unique function to indicate default signal handling.
*/
void __SEGGER_RTL_SIGNAL_SIG_DFL(int sig) {
  __SEGGER_RTL_USE_PARA(sig);
}

/*********************************************************************
*
*       __SEGGER_RTL_SIGNAL_SIG_ERR()
*
*  Function description
*    Unique function to indicate error signal handling.
*/
void __SEGGER_RTL_SIGNAL_SIG_ERR(int sig) {
  __SEGGER_RTL_USE_PARA(sig);
}

/*********************************************************************
*
*       __SEGGER_RTL_SIGNAL_SIG_IGN()
*
*  Function description
*    Unique function to indicate ignored signal handling.
*/
void __SEGGER_RTL_SIGNAL_SIG_IGN(int sig) {
  __SEGGER_RTL_USE_PARA(sig);
}

/*********************************************************************
*
*       signal()
*
*  Function description
*    Register signal function.
*
*  Parameters
*    sig  - Signal being registered.
*    func - Function to call when signal raised.
*
*  Return value
*    Previously-registered signal handler.
*
*  Thread safety
*    Safe.
*/
__SEGGER_RTL_SIGNAL_FUNC *signal(int sig, __SEGGER_RTL_SIGNAL_FUNC *func) {
  __SEGGER_RTL_SIGNAL_FUNC *registered;
  //
  // Check signal number.
  //
  if (sig < 0 || __SEGGER_RTL_SIGNAL_MAX <= sig) {
    return SIG_ERR;
  }
  //
  // Register function.
  //
  registered = __SEGGER_RTL_aSigTab[sig];
  if (registered == NULL) {
    registered = SIG_DFL;
  }
  __SEGGER_RTL_aSigTab[sig] = func;
  //
  // Previously installed handler is returned.
  //
  return registered;
}

/*********************************************************************
*
*       raise()
*
*  Function description
*    Raise a signal.
*
*  Parameters
*    sig - Signal to raise.
*
*  Additional information
*    Signal handlers are executed in the context of the calling
*    thread, if any.  Signal handlers should not access or maniplate
*    thread-local data.
*
*  Return value
*    Zero if success.
*
*  Thread safety
*    Safe.
*/
int raise(int sig) {
  __SEGGER_RTL_SIGNAL_FUNC *registered;
  //
  // Set signal to be ignored, retrieve current handler.
  //
  registered = signal(sig, SIG_IGN);
  //
  // Can't use a switch statement on a function...
  //
  if (registered == SIG_ERR) {
    //
    // Error.
    //
    return -1;
    //
  } else if (registered == SIG_IGN) {
    //
    // Ignore.
    //
    return 0;
    //
  } else if (registered == SIG_DFL) {
    //
    // Default handling is to exit.
    //
    exit(EXIT_FAILURE);
    __SEGGER_RTL_UNREACHABLE();
    //
  } else {
    //
    // Restore default handling.
    //
    signal(sig, SIG_DFL);
    //
    // Execute signal handler.
    //
    registered(sig);
    //
    // Worked OK.
    //
    return 0;
  }
}

/*********************************************************************
*
*       abort_handler_s()
*
*  Function description
*    Abort on runtime constraint violation (C11).
*
*  Parameters
*    msg   - Pointer to a null-terminated character string describing the
*            constraint violation.
*    ptr   - A null pointer or a pointer to an implementation-defined
*            object.  This implementation always passes NULL.
*    error - If the function calling the handler has a return type declared
*            as errno_t, the return value of the function is passed.
*            Otherwise, a positive value of type errno_t is passed.
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <stdlib.h> to access this function.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*
*  Thread safety
*    Not applicable.
*/
void __SEGGER_RTL_PUBLIC_API abort_handler_s(const char *msg, void *ptr, errno_t error) {
  char acText[16];
  //
  __SEGGER_RTL_USE_PARA(ptr);
  //
  // We implement it like this so as not to unduly bring in formatting support.
  //
  itoa(error, acText, 10);
  __SEGGER_RTL_puts_no_nl("runtime constraint violation: ");
  __SEGGER_RTL_puts_no_nl(msg);
  __SEGGER_RTL_puts_no_nl(" -- error=");
  __SEGGER_RTL_puts_no_nl(acText);
  __SEGGER_RTL_puts_no_nl(" (");
  __SEGGER_RTL_puts_no_nl(strerror(error));
  __SEGGER_RTL_puts_no_nl(")\n\n");
  //
  abort();
}

/*********************************************************************
*
*       ignore_handler_s()
*
*  Function description
*    Ignore runtime constraint violations (C11).
*
*  Parameters
*    msg   - Pointer to a null-terminated character string describing the
*            constraint violation.
*    ptr   - A null pointer or a pointer to an implementation-defined
*            object.  This implementation always passes NULL.
*    error - If the function calling the handler has a return type declared
*            as errno_t, the return value of the function is passed.
*            Otherwise, a positive value of type errno_t is passed.
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <stdlib.h> to access this function.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API ignore_handler_s(const char *msg, void *ptr, errno_t error) {
  __SEGGER_RTL_USE_PARA(msg);
  __SEGGER_RTL_USE_PARA(ptr);
  __SEGGER_RTL_USE_PARA(error);
}

/*********************************************************************
*
*       set_constraint_handler_s()
*
*  Function description
*    Set runtime constraint handler (C11).
*
*  Parameters
*    handler - Pointer to function that will handle runtime
*              constraint violations.  If NULL, the implementation-default
*              abort handler is used.
*
*  Return value
*    The previously-set constraint handler.  If the previous handler was
*    registered by calling set_constraint_handler() with a null pointer
*    argument, a pointer to the implementation-default handler is returned
*    (not NULL).
*
*  Additional information
*    The macro __STDC_WANT_LIB_EXT1__ must be set to 1 before
*    including <stdlib.h> to access this function.
*
*  Conformance
*    ISO/IEC 9899:2011 (C11).
*
*  Thread safety
*    Unsafe.
*/
constraint_handler_t set_constraint_handler_s(constraint_handler_t handler) {
  constraint_handler_t old;
  //
  old = __SEGGER_RTL_constraint_handler;
  __SEGGER_RTL_constraint_handler = handler ? handler : abort_handler_s;
  //
  return old;
}

/*************************** End of file ****************************/
