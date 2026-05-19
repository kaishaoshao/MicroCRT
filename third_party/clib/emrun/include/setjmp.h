/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_SETJMP_H
#define __SEGGER_RTL_SETJMP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL.h"

/*********************************************************************
*
*       Data types
*
**********************************************************************
*/

#if defined(__riscv_xlen) && __riscv_xlen == 32 && defined(__riscv_abi_rve)
  typedef struct {
    long ireg[4];
  } jmp_buf[1];
#elif defined(__riscv_xlen)
  #if !defined(__riscv_flen)
    typedef struct {
      long  ireg[14];
    } jmp_buf[1];
  #elif __riscv_flen == 32
    typedef struct {
      long  ireg[14];
      float freg[12];
    } jmp_buf[1];
  #elif __riscv_flen == 64
    typedef struct {
      long   ireg[14];
      double freg[12];
    } jmp_buf[1];
  #endif
#elif defined(__arch64__)
  typedef unsigned long jmp_buf[24];  // R19-R29,LR,FP,SP,D8-D15
#elif __SEGGER_RTL_FP_HW > 0
  typedef unsigned long long jmp_buf[14];  // R4-R14, D8-D15
#else
  typedef unsigned long jmp_buf[11];  // R4-R14
#endif

/*********************************************************************
*
*       Functions
*
**********************************************************************
*/

                       int   setjmp   (jmp_buf __env);
__SEGGER_RTL_NO_RETURN void  longjmp  (jmp_buf __env, int __val);

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
