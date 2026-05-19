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
#include "fenv.h"

/*********************************************************************
*
*       Package-public data
*
**********************************************************************
*/

const fenv_t __SEGGER_RTL_default_fenv;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#if __SEGGER_RTL_INCLUDE_C_API

/*********************************************************************
*
*       feclearexcept()
*
*  Function description
*    Clear floating-point exceptions.
*
*  Parameters
*    excepts - Bitmask of floating-point exceptions to clear.
*
*  Additional information
*    This function attempts to clear the floating-point exceptions
*    indicated by excepts.
*
*  Return value
*    == 0 - Floating-point exceptions successfully cleared.
*    != 0 - Floating-point exceptions not cleared or not supported.
*
*  Notes
*    This function has no return value in ISO C (1999) and an
*    integer return value in ISO C (2008).
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API feclearexcept(int excepts) {
  return excepts == 0 ? 0 : 1;
}

/*********************************************************************
*
*       feraiseexcept()
*
*  Function description
*    Raise floating-point exceptions.
*
*  Parameters
*    excepts - Bitmask of floating-point exceptions to raise.
*
*  Additional information
*    This function attempts to raise the floating-point exceptions
*    indicated by excepts.
*
*  Return value
*    == 0 - All floating-point exceptions successfully raised.
*    != 0 - Floating-point exceptions not successuly raised or not supported.
*
*  Notes
*    This function has no return value in ISO C (1999) and an
*    integer return value in ISO C (2008).
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API feraiseexcept(int excepts) {
  return excepts == 0 ? 0 : 1;
}

/*********************************************************************
*
*       fetestexcept()
*
*  Function description
*    Test floating-point exceptions.
*
*  Parameters
*    excepts - Bitmask of floating-point exceptions to test.
*
*  Additional information
*    This function determines which of the floating-point exceptions
*    indicated by excepts are currently set.
*
*  Return value
*    The bitmask of all floating-point exceptions that are currently
*    set and are specified in excepts.
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API fetestexcept(int excepts) {
  __SEGGER_RTL_USE_PARA(excepts);
  return 0;
}

/*********************************************************************
*
*       fegetexceptflag()
*
*  Function description
*    Get floating-point exceptions.
*
*  Parameters
*    flagp   - Pointer to object that receives the floating-point exception state.
*    excepts - Bitmask of floating-point exceptions to store.
*
*  Additional information
*    This function attempts to save the floating-point exceptions indicated by
*    excepts to the object pointed to by flagp.
*
*  Return value
*    == 0 - Floating-point exceptions correctly stored.
*    != 0 - Floating-point exceptions not correctly stored.
*
*  See also
*    fesetexceptflag().
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API fegetexceptflag(fexcept_t *flagp, int excepts) {
  __SEGGER_RTL_USE_PARA(flagp);
  __SEGGER_RTL_USE_PARA(excepts);
  //
  return 0;
}

/*********************************************************************
*
*       fesetexceptflag()
*
*  Function description
*    Set floating-point exceptions.
*
*  Parameters
*    flagp   - Pointer to object containing a previously-stored
*              floating-point exception state.
*    excepts - Bitmask of floating-point exceptions to restore.
*
*  Additional information
*    This function attempts to restore the floating-point exceptions indicated by
*    excepts from the object pointed to by flagp.  The exceptions to restore
*    as indicated by excepts must have at least been specified when storing the
*    exceptions using fegetexceptflag().
*
*  Return value
*    == 0 - Floating-point exceptions correctly restored.
*    != 0 - Floating-point exceptions not correctly restored.
*
*  See also
*    fegetexceptflag().
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API fesetexceptflag(const fexcept_t *flagp, int excepts) {
  __SEGGER_RTL_USE_PARA(flagp);
  //
  return excepts == 0 ? 0 : 1;
}

/*********************************************************************
*
*       fegetround()
*
*  Function description
*    Get floating-point rounding mode.
*
*  Additional information
*    This function attempts to read the current floating-point rounding
*    mode.
*
*  Return value
*    >= 0 - Current floating-point rounding mode.
*    <  0 - Floating-point rounding mode cannot be determined.
*
*  See also
*    fesetround().
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API fegetround(void) {
  return FE_TONEAREST;
}

/*********************************************************************
*
*       fesetround()
*
*  Function description
*    Set floating-point rounding mode.
*
*  Parameters
*    round - Rounding mode to set.
*
*  Additional information
*    This function attempts to set the current floating-point rounding
*    mode to round.
*
*  Return value
*    == 0 - Current floating-point rounding mode is set to round.
*    != 0 - Requested floating-point rounding mode cannot be set.
*
*  See also
*    fegetround().
*
*  Thread safety
*    Safe [if configured].
*/
int __SEGGER_RTL_PUBLIC_API fesetround(int round) {
  return round == FE_TONEAREST ? 0 : 1;
}

/*********************************************************************
*
*       fegetenv()
*
*  Function description
*    Get floating-point environment.
*
*  Parameters
*    envp - Pointer to object that receives the floating-point
*           environment.
*
*  Additional information
*    This function attempts to store the current floating-point environment
*    to the object pointed to by envp.
*
*  Return value
*    == 0 - Current floating-point environment successfully stored.
*    != 0 - Floating-point environment cannot be stored.
*
*  Notes
*    This function has no return value in ISO C (1999) and an
*    integer return value in ISO C (2008).
*
*  See also
*    fesetenv().
*
*  Thread safety
*    Safe [if configured].
*/
int fegetenv(fenv_t *envp) {
  envp->__control = 0;
  return 0;
}

/*********************************************************************
*
*       fesetenv()
*
*  Function description
*    Set floating-point environment.
*
*  Parameters
*    envp - Pointer to object containing previously-stored
*           floating-point environment.
*
*  Additional information
*    This function attempts to restore the floating-point environment
*    from the object pointed to by envp.
*
*  Return value
*    == 0 - Current floating-point environment successfully restored.
*    != 0 - Floating-point environment cannot be restored.
*
*  Notes
*    This function has no return value in ISO C (1999) and an
*    integer return value in ISO C (2008).
*
*  See also
*    fegetenv().
*
*  Thread safety
*    Safe [if configured].
*/
int fesetenv(const fenv_t *envp) {
  __SEGGER_RTL_USE_PARA(envp);
  //
  return 0;
}

/*********************************************************************
*
*       feupdateenv()
*
*  Function description
*    Restore floating-point environment and reraise exceptions.
*
*  Parameters
*    envp - Pointer to object containing previously-stored
*           floating-point environment.
*
*  Additional information
*    This function attempts to save the currently raised floating-point
*    exceptions, restore the floating-point environment from the object
*    pointed to by envp, and raise the saved exceptions.
*
*  Return value
*    == 0 - Environment restored and exceptions raised successfully.
*    != 0 - Failed to restore environment and raise exceptions.
*
*  Notes
*    This function has no return value in ISO C (1999) and an
*    integer return value in ISO C (2008).
*
*  Thread safety
*    Safe [if configured].
*/
int feupdateenv(const fenv_t *envp) {
  __SEGGER_RTL_USE_PARA(envp);
  //
  return 0;
}

/*********************************************************************
*
*       feholdexcept()
*
*  Function description
*    Save floating-point environment and set non-stop mode.
*
*  Parameters
*    envp - Pointer to object that receives the floating-point
*           environment.
*
*  Additional information
*    This function function saves the current floating-point environment
*    to the object pointed to by envp, clears the floating-point status
*    flags, and then installs a non-stop mode for all floating-point
*    exceptions
*
*  Return value
*    == 0 - Environment stored and non-stop mode set successfully.
*    != 0 - Failed to store environment or set non-stop mode.
*
*  Thread safety
*    Safe [if configured].
*/
int feholdexcept(fenv_t *envp) {
  __SEGGER_RTL_USE_PARA(envp);
  //
  return 0;
}

#endif

/*************************** End of file ****************************/
