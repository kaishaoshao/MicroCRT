/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Purpose : Common header for Stack Overflow Prevention.
*/

#ifndef SEGGER_STOP_H            // Avoid multiple inclusion.
#define SEGGER_STOP_H

/*********************************************************************
*
*       Defines, defaults
*
**********************************************************************
*/
#ifndef   __SEGGER_STOP_USE_PSP
  #define __SEGGER_STOP_USE_PSP 1
#endif

#ifndef  __SEGGER_STOP_LIMIT_BUFFER
  #if !defined (__SOFTFP__)
    #define __SEGGER_STOP_LIMIT_BUFFER (8 * 8) + (13 * 4) + (8 * 4) + 4 // Buffer on the stack when FPU is used.
  #else
    #define __SEGGER_STOP_LIMIT_BUFFER (13 * 4) + (8 * 4) + 4           // Buffer on the stack when no FPU is used.
  #endif
#endif

#if !defined(__ASSEMBLER__) || (__ASSEMBLER__ == 0)

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

extern int __SEGGER_STOP_Limit_MSP;
extern int __SEGGER_STOP_Limit_PSP;

/*********************************************************************
*
*       API functions / Function prototypes
*
**********************************************************************
*/
__attribute__((naked, no_stack_overflow_check)) void __SEGGER_STOP_X_OnError(void);
__attribute__((no_stack_overflow_check))        void __SEGGER_STOP_X_InitLimits(void);

#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
