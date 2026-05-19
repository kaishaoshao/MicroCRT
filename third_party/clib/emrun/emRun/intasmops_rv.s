/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#include "__SEGGER_RTL_Conf.h"
#include "asmdefs_rv.ah"

/*********************************************************************
*
*       Setup, not configurable
*
**********************************************************************
*/

       .set __SEGGER_RTL_inverse_lut_REQUIRED, 0

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#if __SEGGER_RTL_INCLUDE_GNU_API >= 2

/*********************************************************************
*
*       __ashldi3()
*
*  Function description
*    Shift left, long long.
*
*  Parameters
*    a1:a0 - Long long value to shift, hi-lo.
*    a2    - Number of bits to shift by [0, 63].
*
*  Return value
*    a1:a0 - Long long value, hi-lo.
*/

#undef L
#define L(label) .L__ashldi3##label

GLOBAL_FUNC __ashldi3

#if __SEGGER_RTL_TYPESET == 64

        sll     a0, a0, a1
        ret

#else

#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbs     a2, 5, L(LongShift)
#else
        and     a5, a2, 32
        bnez    a5, L(LongShift)
#endif
        srl     a5, a0, 1
        xor     a4, a2, -1
        srl     a5, a5, a4
        sll     a1, a1, a2
        or      a1, a1, a5
        sll     a0, a0, a2
        ret

L(LongShift):
        sll     a1, a0, a2
        li      a0, 0
        ret

#endif

END_FUNC __ashldi3

/*********************************************************************
*
*       __lshrdi3()
*
*  Function description
*    Logical shift right, long long.
*
*  Parameters
*    a1:a0 - Long long value to shift, hi-lo.
*    a2    - Number of bits to shift by [0, 63].
*
*  Return value
*    a1:a0 - Long long value, hi-lo.
*/

#undef L
#define L(label) .L__lshrdi3##label

GLOBAL_FUNC __lshrdi3

#if __SEGGER_RTL_TYPESET == 64

        srl     a0, a0, a1
        ret

#else

#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbs     a2, 5, L(LongShift)
#else
        and     a5, a2, 32
        bnez    a5, L(LongShift)
#endif
        sll     a5, a1, 1
        xor     a4, a2, -1
        sll     a5, a5, a4
        srl     a0, a0, a2
        or      a0, a5, a0
        srl     a1, a1, a2
        ret

L(LongShift):
        srl     a0, a1, a2
        li      a1, 0
        ret

#endif

END_FUNC __lshrdi3

/*********************************************************************
*
*       __ashrdi3()
*
*  Function description
*    Arithmetic shift right, long long.
*
*  Parameters
*    a1:a0 - Long long value to shift, hi-lo.
*    a2    - Number of bits to shift by [0, 63].
*
*  Return value
*    a1:a0 - Long long value, hi-lo.
*/

#undef L
#define L(label) .L__ashrdi3##label

GLOBAL_FUNC __ashrdi3

#if __SEGGER_RTL_TYPESET == 64

        sra     a0, a0, a1
        ret

#else

#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbs     a2, 5, L(LongShift)
#else
        and     a5, a2, 32
        bnez    a5, L(LongShift)
#endif
        sll     a5, a1, 1
        xor     a4, a2, -1
        sll     a5, a5, a4
        srl     a0, a0, a2
        or      a0, a0, a5
        sra     a1, a1, a2
        ret

L(LongShift):
        sra     a0, a1, a2
        sra     a1, a1, 31
        ret

#endif

END_FUNC __ashrdi3

#endif

#if __SEGGER_RTL_INCLUDE_GNU_API > 1

/*********************************************************************
*
*       __mulsi3()
*
*  Function description
*    Multiply, integer.
*
*  Parameters
*    a0 - Multiplicand.
*    a1 - Multiplier.
*
*  Return value
*    a0 - Product.
*/

#undef L
#define L(label) .L__mulsi3##label

GLOBAL_FUNC __mulsi3

#if __SEGGER_RTL_TYPESET == 64 && __SEGGER_RTL_CORE_HAS_MUL_MULH

        mulw    a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_MUL_MULH

        mul     a0, a0, a1
        ret

#else

// Smallest in magnitude to a0.
        bleu    a0, a1, L(no_swap)
        mv      a2, a0
        mv      a0, a1
        mv      a1, a2
L(no_swap):
        mv      a5, a0
        li      a0, 0
        beqz    a5, L(return)
L(multiply):
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sll     a4, a5, 31
        sra     a4, a4, 31
        and     a4, a4, a1
        add     a0, a0, a4
        srli    a5, a5, 1
#else
        andi    a4, a5, 1
        srli    a5, a5, 1
        beqz    a4, L(noadd)
        add     a0, a0, a1
L(noadd):
#endif
        slli    a1, a1, 1
        bnez    a5, L(multiply)
L(return):
        ret

#endif

END_FUNC __mulsi3

/*********************************************************************
*
*       __divsi3()
*
*  Function description
*    Divide, integer.
*
*  Parameters
*    a0 - Dividend.
*    a1 - Divisor.
*
*  Return value
*    a0 - Quotient.
*/

#undef L
#define L(label) .L__divsi3##label

GLOBAL_FUNC __divsi3

#if __SEGGER_RTL_TYPESET == 64 && __SEGGER_RTL_CORE_HAS_DIV

        divw    a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_DIV

        div     a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_MUL_MULH

//
// Use fast divide by reciprocal then apply sign.
//

// Save register state
        addi    sp, sp, -16
        sw      ra, 0(sp)

// Save sign of quotient.
        xor     a2, a0, a1
        sw      a2, 4(sp)

// Compute |a0|.
        sra     a2, a0, 31
        xor     a0, a0, a2
        sub     a0, a0, a2

// Compute |a1|.
        sra     a2, a1, 31
        xor     a1, a1, a2
        sub     a1, a1, a2

// Divide.
        jal     __udivsi3               // a0 = a0 / a1

// Apply sign to quotient.
        lw      a2, 4(sp)
        sra     a2, a2, 31
        xor     a0, a0, a2
        sub     a0, a0, a2

// Discard stack frame and return.
        lw      ra, 0(sp)
        addi    sp, sp, 16
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// Clockwork division.
//

// Compute absolute value of a1 to a3.
        sra     a2, a1, 31              // replication of sign of divisor to a2
        xor     a1, a1, a2
        sub     a3, a1, a2

// Early-out for divide by zero.
        beqz    a3, L(divide_by_zero)

// Compute absolute value of a0 to a0.
        sra     a1, a0, 31              // sign of a0 to a1
        xor     a2, a2, a1
        mv      a4, a2                  // Save sign of quotient
        xor     a0, a0, a1
        sub     a1, a0, a1

// Justify divisor until just less than or equal to dividend.
        srl     a0, a1, 1
        mv      a2, a3
        j       L(justifying)
L(justify):
        sll     a2, a2, 1
L(justifying):
        bleu    a2, a0, L(justify)

// Zero accumulator and start division.  a1 is the remainder...
        li      a0, 0
        j       L(dividing)

L(divide):
        srl     a2, a2, 1

// Trial subtraction.
L(dividing):
        add     a0, a0, a0              // Develop 0 quotient bit
        bltu    a1, a2, L(cant_subtract)

// Subtraction can be made.
        sub     a1, a1, a2
        add     a0, a0, 1               // Replace 0 quotient bit with 1 bit

// Terminate when subtracting complete.
L(cant_subtract):
        bne     a2, a3, L(divide)

// Negate if quotient must be negative.
        xor     a0, a0, a4
        sub     a0, a0, a4

// All done.
        ret

// Handle division by zero.
L(divide_by_zero):
        li      a0, 0
        ret

#elif __SEGGER_RTL_TYPESET == 64

// 64-bit division of 32-bit integers should never be required...
        tail    __divdi3

#else

#error Not supported

#endif

END_FUNC __divsi3

/*********************************************************************
*
*       __modsi3()
*
*  Function description
*    Modulus, integer.
*
*  Parameters
*    a0 - Dividend.
*    a1 - Divisor.
*
*  Return value
*    a0 - Remainder.
*/

#undef L
#define L(label) .L__modsi3##label

GLOBAL_FUNC __modsi3

#if __SEGGER_RTL_TYPESET == 64 && __SEGGER_RTL_CORE_HAS_DIV

        remw    a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_DIV

        rem     a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_MUL_MULH

//
// Use fast divide by reciprocal, calculate remainder, then apply sign.
//

// Save return address.
        addi    sp, sp, -16
        sw      ra, 0(sp)

// Compute |a0|.
        sra     a2, a0, 31
        xor     a0, a0, a2
        sub     a0, a0, a2

// Save sign of remainder.
        sw      a2, 4(sp)

// Compute |a1|.
        sra     a2, a1, 31
        xor     a1, a1, a2
        sub     a1, a1, a2

// Save |a0| and |a1| in order to calculate remainder.
        sw      a0, 8(sp)
        sw      a1, 12(sp)

// Divide.
        jal     __udivsi3               // a0 = a0 / a1

// Calculate remainder, r = u - v.floor(u/v)
        lw      a1, 12(sp)
        mul     a0, a0, a1
        lw      a1, 8(sp)
        sub     a0, a1, a0

// Apply sign to remainder.
        lw      a2, 4(sp)
        xor     a0, a0, a2
        sub     a0, a0, a2

// Discard stack frame and return.
        lw      ra, 0(sp)
        addi    sp, sp, 16
        ret

#elif __SEGGER_RTL_TYPESET == 32

// Compute absolute value of a1 to a3.
        sra     a2, a1, 31              // replication of sign of divisor to a2
        xor     a1, a1, a2
        sub     a3, a1, a2

// Early-out for divide by zero.
        beqz    a3, L(divide_by_zero)

// Compute absolute value of a0 to a0.
        sra     a4, a0, 31              // sign of a0 to a1
        xor     a0, a0, a4
        sub     a1, a0, a4

// Justify divisor until just less than or equal to dividend.
        srl     a0, a1, 1
        mv      a2, a3
        j       L(justifying)
L(justify):
        sll     a2, a2, 1
L(justifying):
        bleu    a2, a0, L(justify)

// Zero accumulator and start division.  a1 is the remainder...
        li      a0, 0
        j       L(dividing)

L(divide):
        srl     a2, a2, 1

// Trial subtraction.
L(dividing):
        add     a0, a0, a0              // Develop 0 quotient bit
        bltu    a1, a2, L(cant_subtract)

// Subtraction can be made.
        sub     a1, a1, a2
        add     a0, a0, 1               // Replace 0 quotient bit with 1 bit

// Terminate when subtracting complete.
L(cant_subtract):
        bne      a2, a3, L(divide)

// Negate if quotient must be negative.
        xor     a0, a1, a4
        sub     a0, a0, a4

// All done.
        ret

// Handle division by zero.
L(divide_by_zero):
        li      a0, 0
        ret

#elif __SEGGER_RTL_TYPESET == 64

// Division of 32-bit integers on RV64 should never be required.
        tail    __moddi3

#else

#error Not supported

#endif

END_FUNC __modsi3

/*********************************************************************
*
*       __udivsi3()
*
*  Function description
*    Divide, unsigned integer.
*
*  Parameters
*    a0 - Dividend.
*    a1 - Divisor.
*
*  Return value
*    a0 - Quotient.
*/

#undef L
#define L(label) .L__udivsi3_##label

GLOBAL_FUNC __udivsi3

#if __SEGGER_RTL_TYPESET == 64 && __SEGGER_RTL_CORE_HAS_DIV

        divuw   a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_DIV

        divu    a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_MUL_MULH

// Based on section 3.3.3 of "An Overview of Floating-Point Support
// and Math Library on the Intel XScale".  Activated when core has
// a multiplier but no divider.

.macro u32_divstep bit

        srli    t0, a4, \bit
        bgtu    a1, t0, 1f
        slli    t0, a1, \bit
        sub     a4, a4, t0
        addi    a0, a0, 1<<\bit
1:
.endm

// Will we have a small quotient?  If N < 2^7*D then our quotient will
// require at most 7 bits.
        li      a3, 4
        bleu    a1, a3, L(special_cases)
        srli    t0, a0, 7
        bgtu    a1, t0, L(small_quotent)

// Find leading zero count in divisor and normalize in a3.
        mv      a3, a1
        NORM32  a3, a2, t0              // a3 is normalized, a2 set to number of shifts required to normalize, t0 is temporary

// Index into 6-bit-in, 8-bit-out reciprocal LUT.
       .set __SEGGER_RTL_inverse_lut_REQUIRED, 1
#if __SEGGER_RTL_WORKAROUND_CLANG_AS_SYMBOL_BUG
        la      a4, __SEGGER_RTL_inverse_lut      // a3 will have bit 6 set, so subtract it out by folding into address computation.
        addi    a4, a4, -64
#else
        la      a4, __SEGGER_RTL_inverse_lut-64   // a3 will have bit 6 set, so subtract it out by folding into address computation.
#endif
        srl     a3, a3, 25
        add     a4, a4, a3
        lbu     a4, (a4)

// General division where we refine the quotient.

// R = 2^32 - B*B[r] = -(B*B[r]) with B[r] from LUT in a4.
        addi    a2, a2, -7
        sll     a4, a4, a2
        mul     a1, a4, a1
        mul     a2, a4, a0              // a0:a2 = a4*a0
        mulhu   a0, a4, a0
        neg     a1, a1

// A[j]h = HIGH (R*A[j-1]h + A[j-1]l)
// A[j]l = LOW  (R*A[j-1]h + A[j-1]l)  ... five times
        mulhu   a3, a1, a0              // t0:a3 = a1*a0
        mul     t0, a1, a0
        add     a2, a2, t0              // a0:a2 += high(t0:a3)
        sltu    t0, a2, t0
        add     a3, a3, t0
        add     a0, a0, a3

// Iteration 2.
        mulhu   a4, a1, a3
        mul     t0, a1, a3
        add     a2, a2, t0
        sltu    t0, a2, t0
        add     a4, a4, t0
        add     a0, a0, a4

// Iteration 3.
        mulhu   a3, a1, a4
        mul     t0, a1, a4
        add     a2, a2, t0
        sltu    t0, a2, t0
        add     a3, a3, t0
        add     a0, a0, a3

// Iteration 4.
        mulhu   a4, a1, a3
        mul     t0, a1, a3
        add     a2, a2, t0
        sltu    t0, a2, t0
        add     a4, a4, t0
        add     a0, a0, a4

// Iteration 5.
        mulhu   a3, a1, a4
        mul     t0, a1, a4
        add     a2, a2, t0
        sltu    t0, a2, t0
        add     a3, a3, t0
        add     a0, a0, a3

// Rounding correction.
        add     a2, a2, a1
        sltu    a2, a2, a1
        add     a0, a0, a2
        ret

// Handle quotient which is less than 2^7; compute quotient by clockwork.
L(small_quotent):
        mv      a4, a0
        li      a0, 0                   // prep quotient

// Common case for N and D where N/D < 4.
        srli    t0, a4, 2
        bgtu    a1, t0, L(extremely_small_quotient)

// Run six by-clockwork divide steps.
        u32_divstep 7
        u32_divstep 6
        u32_divstep 5
        u32_divstep 4
        u32_divstep 3
        u32_divstep 2

// Short quotent and final two divide steps, where the final one
// can be optimized.
L(extremely_small_quotient):
        u32_divstep 1

// Last division step with no need for update.
        bgtu    a1, a4, L(complete)
        addi    a0, a0, 1
L(complete):
        ret

// Division where the divisor is [0, 4].
L(special_cases):
        li      a2, 3
        bgtu    a1, a2, L(divide_by_4)
        beq     a1, a2, L(divide_by_3)
        li      a2, 1
        bgtu    a1, a2, L(divide_by_2)
        beq     a1, a2, L(divide_by_1)

L(divide_by_0):
        li      a0, 0
        ret

L(divide_by_4):
        srl     a0, a0, 2
        ret

L(divide_by_3):
        li      a2, 0xaaaaaaab          // Load 1/3.
        mulhu   a0, a0, a2              // Multiply by 1/3.
L(divide_by_2):
        srl     a0, a0, 1               // Final correction.
L(divide_by_1):
        ret

#elif __SEGGER_RTL_TYPESET == 32

// Save a1; opportunistic test for division by zero.
        beqz    a1, L(divide_by_zero)
        mv      a2, a1

// Justify divisor until just less than or equal to dividend.
        mv      a3, a0
        srl     a0, a0, 1
        j       L(justifying)
L(justify):
        sll     a2, a2, 1
L(justifying):
        bleu    a2, a0, L(justify)

// Zero accumulator and start division.  a3 is the remainder...
        li      a0, 0
        j       L(dividing)
L(divide):
        srl     a2, a2, 1

// Trial subtraction.
L(dividing):
        sll     a0, a0, 1
        bltu    a3, a2, L(cant_subtract)

// Trial subtraction can be made.
        sub     a3, a3, a2
        addi    a0, a0, 1

// Accumulate quotient bit.
L(cant_subtract):

// Terminate when subtracting complete.
        bne     a2, a1, L(divide)

// All done.
        ret

// Handle division by zero.
L(divide_by_zero):
        li      a0, 0
        ret

#elif __SEGGER_RTL_TYPESET == 64

// 64-bit division of 32-bit integers should never be required...
        sll     a0, a0, 32
        srl     a0, a0, 32
        sll     a1, a1, 32
        srl     a1, a1, 32
        tail    __udivdi3

#else

#error Not supported

#endif

END_FUNC __udivsi3

/*********************************************************************
*
*       __umodsi3()
*
*  Function description
*    Modulus, unsigned integer.
*
*  Parameters
*    a0 - Dividend.
*    a1 - Divisor.
*
*  Return value
*    a0 - Remainder.
*/

#undef L
#define L(label) .L__umodsi3_##label

GLOBAL_FUNC __umodsi3

#if __SEGGER_RTL_TYPESET == 64 && __SEGGER_RTL_CORE_HAS_DIV

        remuw   a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_DIV

        remu    a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 32 && __SEGGER_RTL_CORE_HAS_MUL_MULH

// Save register state
        addi    sp, sp, -16
        sw      ra, 12(sp)
        sw      a0, 8(sp)
        sw      a1, 4(sp)

// Divide.
        jal     __udivsi3               // a0 = a0 / a1

// Calculate remainder.
        lw      a1, 4(sp)
        mul     a0, a0, a1              // a0 = (a0 / a1) * a1
        lw      a1, 8(sp)
        sub     a0, a1, a0              // a0 = a0 - (a0 / a1) * a1

// Discard stack frame and return.
        lw      ra, 12(sp)
        addi    sp, sp, 16
        ret

#elif __SEGGER_RTL_TYPESET == 32

// Save a1; opportunistic test for division by zero.
        beqz    a1, L(divide_by_zero)
        mv      a2, a1

// Justify divisor until just less than or equal to dividend.
        mv      a3, a0
        srl     a0, a0, 1
        j       L(justifying)
L(justify):
        sll     a2, a2, 1
L(justifying):
        bleu    a2, a0, L(justify)

// Zero accumulator and start division.  a3 is the remainder...
        li      a0, 0
        j       L(dividing)
L(divide):
        srl     a2, a2, 1

// Trial subtraction.
L(dividing):
        sll     a0, a0, 1
        bltu    a3, a2, L(cant_subtract)

// Trial subtraction can be made and guaranteed C=1 afterwards.
        sub     a3, a3, a2
        addi    a0, a0, 1

// Accumulate quotient bit.
L(cant_subtract):

// Terminate when subtracting complete.
        bne     a2, a1, L(divide)

// All done.
        mv      a0, a3
        ret

// Handle division by zero.
L(divide_by_zero):
        li      a0, 0
        ret

#elif __SEGGER_RTL_TYPESET == 64

// 64-bit division of 32-bit integers should never be required...
        sll     a0, a0, 32
        srl     a0, a0, 32
        sll     a1, a1, 32
        srl     a1, a1, 32
        tail    __umoddi3

#else

#error Not supported

#endif

END_FUNC __umodsi3

/*********************************************************************
*
*       Local data
*
**********************************************************************
*/

// 6-bit-in, 8-bit-out reciprocal LUT, 2^14/(01bbbbbb).
.if __SEGGER_RTL_inverse_lut_REQUIRED
LOCAL_RODATA __SEGGER_RTL_inverse_lut, 1
       .byte    0xFC, 0xF8, 0xF4, 0xF0, 0xED, 0xEA, 0xE6, 0xE3
       .byte    0xE0, 0xDD, 0xDA, 0xD7, 0xD4, 0xD2, 0xCF, 0xCC
       .byte    0xCA, 0xC7, 0xC5, 0xC3, 0xC0, 0xBE, 0xBC, 0xBA
       .byte    0xB8, 0xB6, 0xB4, 0xB2, 0xB0, 0xAE, 0xAC, 0xAA
       .byte    0xA8, 0xA7, 0xA5, 0xA3, 0xA2, 0xA0, 0x9F, 0x9D
       .byte    0x9C, 0x9A, 0x99, 0x97, 0x96, 0x94, 0x93, 0x92
       .byte    0x90, 0x8F, 0x8E, 0x8D, 0x8C, 0x8A, 0x89, 0x88
       .byte    0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80
END_RODATA __SEGGER_RTL_inverse_lut
.endif

#endif

/*************************** End of file ****************************/
