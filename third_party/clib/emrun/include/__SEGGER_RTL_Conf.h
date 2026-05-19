/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_CONF_H
#define __SEGGER_RTL_CONF_H

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// This attempts to detect the applicable target based on preprocessor
// symbols.
//

#if defined(__SEGGER_RTL_FORCE_GENERIC)
  //
  // Override autodetection, force generic version.
  //
  #include "__SEGGER_RTL_Generic_Conf.h"
#elif defined(__ARM_ARCH_ISA_ARM) || defined(__ARM_ARCH_ISA_THUMB)
  //
  // GNU C doesn't define __ARM_ACLE but does set the feature-test macros,
  // so use the ISA macros, one of which must be defined.
  //
  #include "__SEGGER_RTL_Arm_Conf.h"
#elif defined(__riscv)
  #include "__SEGGER_RTL_RISCV_Conf.h"
#elif defined(__XC16__)
  #include "__SEGGER_RTL_XC16_Conf.h"
#elif defined(__tricore__)
  #include "__SEGGER_RTL_Tricore_Conf.h"
#elif defined(_MSC_VER)
  #include "__SEGGER_RTL_MSVC_Conf.h"
#elif defined(__aarch64__ )
  #include "__SEGGER_RTL_Arm64_Conf.h"
#else
  #include "__SEGGER_RTL_Generic_Conf.h"
#endif

#endif

/*************************** End of file ****************************/
