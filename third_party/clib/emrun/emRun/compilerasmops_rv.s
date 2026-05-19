/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Purpose: Compiler runtime support.

*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "asmdefs_rv.ah"

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

#define L(label) .L__riscv_save_##label

/*********************************************************************
*
*       __riscv_save_12(), __riscv_save_11(), ...
*
*  Function description
*    Save registers to stack.
*/

#if defined(__riscv_abi_rve)

GLOBAL_FUNC  __riscv_save_2
GLOBAL_LABEL __riscv_save_1
GLOBAL_LABEL __riscv_save_0
        addi    sp, sp, -12*__riscv_xlen/32
        SF      s1, 0
        SF      s0, 4
        SF      ra, 8
        jr      t0

END_FUNC __riscv_save_2

#else

GLOBAL_FUNC __riscv_save_12
        addi    sp, sp, -64*__riscv_xlen/32
        li      t1, 0
        SF      s11, 12
        j       L(s10_down)

GLOBAL_LABEL __riscv_save_11
GLOBAL_LABEL __riscv_save_10
GLOBAL_LABEL __riscv_save_9
GLOBAL_LABEL __riscv_save_8
        addi    sp, sp, -64*__riscv_xlen/32
        li      t1, 16*__riscv_xlen/32

L(s10_down):
        SF      s10, 16
        SF      s9, 20
        SF      s8, 24
        SF      s7, 28
        j       L(s6_down)

GLOBAL_LABEL __riscv_save_7
GLOBAL_LABEL __riscv_save_6
GLOBAL_LABEL __riscv_save_5
GLOBAL_LABEL __riscv_save_4
        addi    sp, sp, -64*__riscv_xlen/32
        li      t1, 32*__riscv_xlen/32

L(s6_down):
        SF      s6, 32
        SF      s5, 36
        SF      s4, 40
        SF      s3, 44
        SF      s2, 48
        SF      s1, 52
        SF      s0, 56
        SF      ra, 60
        add     sp, sp, t1
        jr      t0

END_FUNC __riscv_save_12

#undef L

GLOBAL_FUNC  __riscv_save_3
GLOBAL_LABEL __riscv_save_2
GLOBAL_LABEL __riscv_save_1
GLOBAL_LABEL __riscv_save_0
        addi    sp, sp, -16*__riscv_xlen/32
        SF      s2, 0
        SF      s1, 4
        SF      s0, 8
        SF      ra, 12
        jr      t0

END_FUNC __riscv_save_3

#endif

#undef L
#define L(label) .L__riscv_restore_##label

/*********************************************************************
*
*       __riscv_restore_12(), __riscv_restore_11(), ...
*
*  Function description
*    Restore registers from stack.
*/

#if defined(__riscv_abi_rve)

GLOBAL_FUNC  __riscv_restore_2
        LF      s1, 0
GLOBAL_LABEL __riscv_restore_1
        LF      s0, 4
GLOBAL_LABEL __riscv_restore_0
        LF      ra, 8
        addi    sp, sp, 12*__riscv_xlen/32
        ret

END_FUNC __riscv_restore_2

#else

GLOBAL_FUNC __riscv_restore_12

        LF      s11, 12
        addi    sp, sp, 16*__riscv_xlen/32

GLOBAL_LABEL __riscv_restore_11
        LF      s10, 0
GLOBAL_LABEL __riscv_restore_10
        LF      s9, 4
GLOBAL_LABEL __riscv_restore_9
        LF      s8, 8
GLOBAL_LABEL __riscv_restore_8
        LF      s7, 12
        addi    sp, sp, 16*__riscv_xlen/32

GLOBAL_LABEL __riscv_restore_7
        LF      s6, 0
GLOBAL_LABEL __riscv_restore_6
        LF      s5, 4
GLOBAL_LABEL __riscv_restore_5
        LF      s4, 8
GLOBAL_LABEL __riscv_restore_4
        LF      s3, 12
        addi    sp, sp, 16*__riscv_xlen/32

GLOBAL_LABEL __riscv_restore_3
        LF      s2, 0
GLOBAL_LABEL __riscv_restore_2
        LF      s1, 4
GLOBAL_LABEL __riscv_restore_1
        LF      s0, 8
GLOBAL_LABEL __riscv_restore_0
        LF      ra, 12
        addi    sp, sp, 16*__riscv_xlen/32
        ret

END_FUNC __riscv_restore_12

#endif

/*************************** End of file ****************************/
