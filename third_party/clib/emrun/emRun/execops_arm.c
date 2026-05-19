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

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

const int __aeabi_SIGABRT   = 0;
const int __aeabi_SIGINT    = 1;
const int __aeabi_SIGILL    = 2;
const int __aeabi_SIGFPE    = 3;
const int __aeabi_SIGSEGV   = 4;
const int __aeabi_SIGTERM   = 5;

/*************************** End of file ****************************/

int
__aeabi_idiv0(int return_value)__attribute__((weak));
int
__aeabi_idiv0(int return_value)
{
  return return_value;
}

long long
__aeabi_ldiv0(long long return_value)__attribute__((weak));
long long
__aeabi_ldiv0(long long return_value)
{
  return return_value;
}
