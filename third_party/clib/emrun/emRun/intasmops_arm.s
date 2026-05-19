/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File        : intasmops_arm.s
Purpose     : SEGGER runtime support for Arm processors.

References  : [1]  Run-time ABI for the ARM Architecture
                   http://infocenter.arm.com/help/topic/com.arm.doc.ihi0043d/IHI0043D_rtabi.pdf

Implementation notes
--------------------

Entry points:

  __aeabi_llsl()
  __aeabi_llsr()
  __aeabi_lasr()
  __aeabi_lcmp()
  __aeabi_ulcmp()

  __aeabi_idiv()
  __aeabi_idivmod()
  __aeabi_uidiv()
  __aeabi_uidivmod()

  __aeabi_ldivmod()
  __aeabi_uldivmod()

  __clzsi2()
  __clzdi2()

Extended entry points for testing:

  __aeabi_imod()
  __aeabi_uimod()
  __aeabi_lmod()
  __aeabi_ulmod()

*/

#if !defined(__aarch64__)

@ gnu_syntax

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#define __SEGGER_RTL_ASSEMBLER_INCLUDE
#include "asmdefs_arm.ah"
#include "__SEGGER_RTL_ConfDefaults.h"

/*********************************************************************
*
*       Local data
*
**********************************************************************
*/

// 6-bit-in, 8-bit-out reciprocal LUT, 2^14/(01bbbbbb).
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

/*********************************************************************
*
*       Local macros
*
**********************************************************************
*/

       .macro   IDIV0
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16
        push    {r4, lr}
        bl      __aeabi_idiv0
        RET1    r4
#else
        b       __aeabi_idiv0
#endif
       .endm

       .macro   LDIV0
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16
        push    {r4, lr}
        bl      __aeabi_ldiv0
        RET1_r3 r4
#else
        b       __aeabi_ldiv0
#endif
       .endm

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       __aeabi_lcmp()
*
*  Function description
*    Compare, long long.
*
*  Prototype
*    int __aeabi_lcmp(long long x, long long y);
*
*  Parameters
*    r1:r0 - x - Left-hand value.
*    r3:r2 - y - Right-hand value.
*
*  Return value
*    <  0 - x < y.
*    == 0 - x == y
*    >  0 - x > y
*/

#undef L
#define L(label) .L__aeabi_lcmp##label

ARM_GLOBAL_FUNC __aeabi_lcmp

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

        cmp     xh, yh
        bgt     L(greater)
        blt     L(less)
        cmp     xl, yl
        bhi     L(greater)
        sbcs    r0, r0, r0
        bx      lr
L(greater):
        movs    r0, #1
        bx      lr
L(less):
        movs    r0, #0
        subs    r0, r0, #1
        bx      lr

#else

        cmp     xh, yh
        bgt     L(greater)
        blt     L(less)
        cmp     xl, yl
        ite     hi
        movhi   r0, #1
        sbcls   r0, r0, r0
        bx      lr
L(greater):
        movs    r0, #1
        bx      lr
L(less):
        movs    r0, #-1
        bx      lr

#endif

END_FUNC __aeabi_lcmp

/*********************************************************************
*
*       __aeabi_ulcmp()
*
*  Function description
*    Compare, unsigned long long.
*
*  Prototype
*    int __aeabi_ulcmp(unsigned long long x, unsigned long long y);
*
*  Parameters
*    r1:r0 - x - Left-hand value.
*    r3:r2 - y - Right-hand value.
*
*  Return value
*    <  0 - x < y.
*    == 0 - x == y
*    >  0 - x > y
*/

#undef L
#define L(label) .L__aeabi_ulcmp_##label
 
ARM_GLOBAL_FUNC __aeabi_ulcmp

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

        cmp     xh, yh
        bhi     L(greater)
        bcc     L(less)
        cmp     xl, yl
        bhi     L(greater)
L(less):
        sbcs    r0, r0, r0
        bx      lr
L(greater):
        movs    r0, #1
        bx      lr

#else

        cmp     xh, yh
        it      eq
        cmpeq   xl, yl
        ite     hi
        movhi   r0, #1
        sbcls   r0, r0, r0
        bx      lr

#endif

END_FUNC __aeabi_ulcmp
  
/*********************************************************************
*
*       __aeabi_llsl()
*
*  Function description
*    Logical shift left, unsigned long long.
*
*  Prototype
*    unsigned long long __aeabi_llsl(unsigned long long Operand, unsigned Positions);
*
*  Parameters
*    r1:r0 - Operand   - Value to shift.
*    r2    - Positions - Number of bit positions to shift by [0, 63].
*
*  Return value
*    r1:r0 - Shifted value.
*/

#undef L
#define L(label) .L__int64_lsl_##label

ARM_GLOBAL_FUNC __aeabi_llsl

#if __SEGGER_RTL_CORE_HAS_LSLL_LSRL_ASRL

        llsl    xl, xh, r2
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Check if count is 32 or greater and adjust.
        cmp     r2, #32
        bcs     L(LongShift)

// Shift count less than 32.
        movs    r3, xl                  // Save low-order word to shift into high-order
        lsls    xl, xl, r2              // Shift low-order word into place
        lsls    xh, xh, r2              // Shift high-order word into place
        negs    r2, r2                  // Calculate right-shift count
        adds    r2, r2, #32
        lsrs    r3, r3, r2              // Shift low-order word to high-order position...
        orrs    xh, xh, r3              // ...and combine
        bx      lr

// Shift count >= 32, so shift my moving registers.
L(LongShift):
        subs    r2, r2, #32             // Calculate shift count
        movs    xh, xl                  // Shift by 32 bits
        lsls    xh, xh, r2              // Shift remainder
        movs    xl, #0                  // Drag sign through low-order word
        bx      lr

#else

        subs    r3, r2, #32
        bcs     L(LongShift)
        rsb     r3, r2, #32
        lsls    xh, xh, r2
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orr     xh, xh, xl, lsr r3
#else
        lsrs    r3, xl, r3
        orrs    xh, xh, r3
#endif
        lsls    xl, xl, r2
        bx      lr

L(LongShift):
        lsl     xh, xl, r3
        movs    xl, #0  
        bx      lr
#endif   

END_FUNC __aeabi_llsl

#undef L
#define L(label) .L__int64_lsr_##label

/*********************************************************************
*
*       __aeabi_llsr()
*
*  Function description
*    Logical shift right, unsigned long long.
*
*  Prototype
*    unsigned long long __aeabi_llsr(unsigned long long Operand, unsigned Positions);
*
*  Parameters
*    r1:r0 - Operand   - Value to shift.
*    r2    - Positions - Number of bit positions to shift by [0, 63].
*
*  Return value
*    r1:r0 - Shifted value.
*/

ARM_GLOBAL_FUNC __aeabi_llsr

#if __SEGGER_RTL_CORE_HAS_LSLL_LSRL_ASRL

        llsr    xl, xh, r2
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Check if count is 32 or greater and adjust.
        cmp     r2, #32
        bcs     L(LongShift)

// Shift count less than 32.
        lsrs    xl, xl, r2              // Shift low-order word into place
        movs    r3, xh                  // Save high-order word to shift into low-order
        lsrs    xh, xh, r2              // Shift high-order word into place
        negs    r2, r2                  // Calculate right-shift count
        adds    r2, r2, #32
        lsls    r3, r3, r2              // Shift high-order word to low-order position...
        orrs    xl, xl, r3              // ...and combine
        bx      lr

// Shift count >= 32, so shift my moving registers.
L(LongShift):
        subs    r2, r2, #32             // Calculate shift count
        movs    xl, xh                  // Shift by 32 bits
        lsrs    xl, xl, r2              // Shift remainder
        movs    xh, #0                  // Drag zeros through high-order word
        bx      lr

#else

        subs    r3, r2, #32  
        bcs     L(LongShift)
        rsb     r3, r2, #32
        lsrs    xl, xl, r2
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orr     xl, xl, xh, lsl r3
#else
        lsl     r3, xh, r3
        orrs    xl, xl, r3
#endif
        lsrs    xh, xh, r2
        bx      lr
L(LongShift):
        lsr     xl, xh, r3
        movs    xh, #0 
        bx      lr
#endif

END_FUNC __aeabi_llsr

/*********************************************************************
*
*       __aeabi_lasr()
*
*  Function description
*    Arithmetic shift right, long long.
*
*  Prototype
*    long long __aeabi_lasr(long long Operand, unsigned Positions);
*
*  Parameters
*    r1:r0 - Operand   - Value to shift.
*    r2    - Positions - Number of bit positions to shift by [0, 63].
*
*  Return value
*    r1:r0 - Shifted value.
*/

#undef L
#define L(label) .L__int64_asr_##label

ARM_GLOBAL_FUNC __aeabi_lasr

#if __SEGGER_RTL_CORE_HAS_LSLL_LSRL_ASRL

        lasr    xl, xh, r2
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Check if count is 32 or greater and adjust.
        cmp     r2, #32
        bcs     L(LongShift)

// Shift count less than 32.
        lsrs    xl, xl, r2              // Shift low-order word into place
        movs    r3, xh                  // Save high-order word to shift into low-order
        asrs    xh, xh, r2              // Shift high-order word into place
        negs    r2, r2                  // Calculate right-shift count
        adds    r2, r2, #32
        lsls    r3, r3, r2              // Shift high-order word to low-order position...
        orrs    xl, xl, r3              // ...and combine
        bx      lr

// Shift count >= 32, so shift my moving registers.
L(LongShift):
        subs    r2, r2, #32             // Calculate shift count
        movs    xl, xh                  // Shift by 32 bits
        asrs    xl, xl, r2              // Shift remainder
        asrs    xh, xh, #31             // Drag sign through high-order word
        bx      lr

#else

        subs    r3, r2, #32
        bcs     L(LongShift)
        rsb     r3, r2, #32
        lsrs    xl, xl, r2
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orr     xl, xl, xh, lsl r3
#else
        lsl     r3, xh, r3
        orrs    xl, xl, r3
#endif 
        asrs    xh, xh, r2  
        bx      lr
L(LongShift):
        asrs    xl, xh, r3
        asrs    xh, xh, #31 
        bx      lr

#endif

END_FUNC __aeabi_lasr

#undef L
#define L(label) .L__aeabi_lmul_##label
 
/*********************************************************************
*
*       __aeabi_lmul()
*
*  Function description
*    Multiply, long long.
*
*  Prototype
*    long long __aeabi_lmul(long long Multiplicand, long long Multiplier);
*
*  Parameters
*    r1:r0 - Multiplicand.
*    r3:r2 - Multiplier.
*
*  Return value
*    r1:r0 - Product.
*/

#undef L
#define L(label) .L__SEGGER_lmul_##label

ARM_GLOBAL_FUNC __aeabi_lmul

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// xh:xl * yh:yl

// Save working registers and lr, so that we can return with a single POP.
        push    {r4-r5, lr}

// xhyl and xlyh are easily multiplied and accumulated into the high word of the result.
// xh and yh are only used in these cross products so can be working registers from here on.
        muls    xh, xh, yl
        muls    yh, xl, yh
        adds    r4, yh, xh              // r4 = xhyl + xlyh

// xlyl is more complex as T16 architectures don't have a 32x32->64 multiplier.
// Therefore this must be broken down into 4 16x16->32 multiplies and accumulated.
//
// Write u1 = high(xl), u0 = low(xl)
//       v1 = high(yl), v1 = low(yl)
//
// We require:
//           u0.v0
//   +    u1.v0
//   +    u0.v1
//   + u1.v1

// Extract u1 and v1 to xh and yh.
        lsrs    xh, xl, #16             // xh := u1
        lsrs    yh, yl, #16             // yh := v1

// Calculate u1*v1.
        movs    r5, xh
        muls    r5, yh, r5              // r5 := v1*u1 (assemblers complain about muls r5, r5, yh)
        adds    r4, r4, r5              // Accumulate into high order word of result

// Calculate u1*v0 to xh.
        UXTHs   yl, yl                  // yl := v0
        muls    xh, yl, xh              // xh := u1*v0

// Calculate u0*v0 to r5.
        UXTHs   xl, xl                  // xl := u0
        muls    yl, xl, yl              // r5 := u0*v0 (assemblers complain about muls r5, r5, yl)

// Accumulate u1*v0 and u0*v0 to 64-bit working result yl:r5, i.e. yl:r5 := (u1*v0)<<16 + (u0*v0)
        lsrs    r5, xh, #16
        lsls    xh, xh, #16
        adds    yl, yl, xh
        adcs    r5, r5, r4

// Calculate u0*v1 to xl.
        muls    xl, yh, xl              // xl := v1 * u0

// Accumulate u0*v1 product to outgoing 64-bit product, i.e.
// xh:xl := (u0*v1)<<16 + (u1*v0)<<16 + (u0*v0) + high-order product word
        lsrs    xh, xl, #16
        lsls    xl, xl, #16
        adds    xl, xl, yl
        adcs    xh, xh, r5

// All done.
        RET2    r4, r5

#else

        mul     ip, xl, yh
        mla     ip, xh, yl, ip
        umull   xl, xh, yl, xl
        add     xh, xh, ip
        bx      lr

#endif

END_FUNC __aeabi_lmul

/*********************************************************************
*
*       __aeabi_idiv()
*
*  Function description
*    Divide, int.
*
*  Prototype
*    int __aeabi_idiv(int Dividend, int Divisor);
*
*  Parameters
*    r0 - Dividend.
*    r1 - Divisor.
*
*  Return value
*    r0 - Quotient.
*/

#undef L
#define L(label) .L__SEGGER_idiv_##label

ARM_GLOBAL_FUNC __aeabi_idiv

#if __SEGGER_RTL_CORE_HAS_IDIV

        sdiv    r0, r0, r1
        bx      lr

#elif __SEGGER_RTL_CORE_HAS_IDIV_X

        BASE_X  r2                      // Set base of peripheral to r2
        IDIV_X  r0, r0, r1, r2, r3      // r0 := r0 / r1, peripheral base r2, working register r3
        bx      lr

#elif __SEGGER_RTL_FP_HW >= 2 && __SEGGER_RTL_USE_FPU_FOR_IDIV

        vmov         s0, r0
        vcvt.f64.s32 d16, s0
        vmov         s1, r1
        vcvt.f64.s32 d17, s1
        vdiv.f64     d16, d16, d17
        vcvt.s32.f64 s0, d16
        vmov         r0, s0             // Quotient into outgoing registers
        bx           lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Compute absolute value of r1 to r3.
        asrs    r2, r1, #31             // sign of r1 to r2
        eors    r1, r1, r2
        subs    r3, r1, r2

// Early-out for divide by zero.
        beq     L(divide_by_zero)

// Compute absolute value of r0 to r0.
        asrs    r1, r0, #31             // sign of r0 to r1
        eors    r2, r2, r1
        mov     ip, r2                  // Save signs of quotient
        eors    r0, r0, r1
        subs    r1, r0, r1

// Justify divisor until just less than or equal to dividend.
        lsrs    r0, r1, #1
        movs    r2, r3
        b       L(justifying)
L(justify):
        lsls    r2, r2, #1
L(justifying):
        cmp     r2, r0
        bls     L(justify)

// Zero accumulator and start division.  r1 is the remainder...
        movs    r0, #0
        b       L(dividing)
L(divide):
        lsrs    r2, r2, #1

// Trial subtraction.
L(dividing):
        cmp     r1, r2
        bcc     L(cant_subtract)

// Trial subtraction can be made and guaranteed C=1 afterwards.
        subs    r1, r1, r2

// Accumulate quotient bit.
L(cant_subtract):
        adcs    r0, r0, r0

// Terminate when subtracting complete.
        cmp     r2, r3
        bne     L(divide)

// Fix up signs of quotient and remainder.
        mov     r2, ip                  // recover sign of quotient

// Negate if quotient must be negative.
        eors    r0, r0, r2
        subs    r0, r0, r2

// All done.
        bx      lr

// Handle division by zero.
L(divide_by_zero):
        IDIV0

#elif __SEGGER_RTL_OPTIMIZE < 0

// Small division routine...

// ip.31 = sign of quotient; ip.30 sign of dividend.
// r0 = |r0|, r1 = |r1|.
        ands    r3, r1, #0x80000000
        it      mi
        negmi   r1, r1
        eors    ip, r3, r0, asr #32     // C = sign bit of dividend
        it      cs
        negcs   r0, r0

// Prepare to justify divisor; cheap check for division by zero.
        movs    r2, r1
        beq     L(divide_by_zero)

// Justify divisor until just less than or equal to dividend.

L(justify):
        cmp     r2, r0, lsr #1
        it      ls
        lslls   r2, r2, #1
        blo     L(justify)

// Trial subtraction.
L(divide):
        cmp     r0, r2
        it      cs
        subcs   r0, r0, r2              // real subtraction only executed if trial subtraction succeeded.
        adcs    r3, r3, r3              // accumulate result

// Terminate when shifted divisor back to original position.
        cmp     r2, r1
        lsr     r2, r2, #1
        bne     L(divide)

// Apply sign of quotient.
        eors    r0, r3, ip, asr #31
        subs    r0, r0, ip, asr #31

// All done.
        bx      lr

// Handle division by zero.
L(divide_by_zero):
        IDIV0

#else

// Capable core, speed required.

// Based on section 3.3.3 of "An Overview of Floating-Point Support
// and Math Library on the Intel XScale".

.macro i32_divstep bit, op
        cmp     r1, ip, lsr #\bit
        itt     ls
        subls   ip, ip, r1, lsl #\bit
        \op     r0, r0, #1 << \bit
.endm

// Are the operands of different sign?
        teq     r0, r1                  // Equivalent to eors r0, r1 without store to r0.
        bmi     L(opposite_signs)

// Both operands have the same sign, so convert both
// to positive values.  (-a) / (-b) == a / b.
        tst     r0, r0
        itt     mi
        negmi   r0, r0
        negmi   r1, r1

// Will we have a small quotient?  If N < 2^7*D then our quotient will
// require at most 7 bits.
        cmp     r1, r0, lsr #7
        it      ls
        rsbsls  r2, r1, #5
        bhi     L(positive_small_quotent)

// Find leading zero count in divisor and normalize in r3.
        NORM32  r3, r1, r2              // r3 is normalized r1, r4 set to number of shifts required to normalize

// Index into 6-bit-in, 8-bit-out reciprocal LUT.
        la      ip, __SEGGER_RTL_inverse_lut-64   // r3 will have bit 6 set, so subtract it out by folding into address computation.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        ldrb    ip, [ip, r3, lsr #25]
#else
        lsrs    r3, r3, #25
        ldrb    ip, [ip, r3]
#endif

// General division where we refine the quotient.

// R = 2^32 - B*B[r] = -(B*B[r]) with B[r] from LUT in ip.
        subs    r2, r2, #7
        lsl     ip, ip, r2              // No short encoding for lsls variant.
        mul     r1, ip, r1              // Need to use r1, ip, r1 rather than r1, r1, ip
        umull   r2, r0, ip, r0          // This is OK, r0:r2 is different from ip...
        negs    r1, r1                  // Burying this here avoids stall on next umlal.

// A[j]h = HIGH (R*A[j-1]h + A[j-1]l)
// A[j]l = LOW (R*A[j-1]h + A[j-1]l)  ... five times
        movs    r3, #0
        umlal   r2, r3, r1, r0
        add     r0, r0, r3

// Iteration 2.
        movs    ip, #0
        umlal   r2, ip, r1, r3
        add     r0, r0, ip

// Iteration 3.
        movs    r3, #0
        umlal   r2, r3, r1, ip
        add     r0, r0, r3

// Iteration 4.
        movs    ip, #0
        umlal   r2, ip, r1, r3
        add     r0, r0, ip

// Iteration 5 and correction.
        movs    r3, #0
        umlal   r2, r3, r1, ip
        cmn     r2, r1                  // Equivalent to adds r2, r1 except that result is discarded
        adc     r0, r0, r3
        bx      lr

// Handle quotient which is less than 2^7; compute one bit at
// a time...
L(positive_small_quotent):

// We specialize cases [0..4]...
        cmp     r1, #4
        bls     L(positive_special_cases)

// ...otherwise it's a general 7-bit divide by clockwork.
        mov     ip, r0
        movs    r0, #0                  // prep quotient

// Common case for For N and D where N/D < 4.
        cmp     r1, ip, lsr #2
        bhi     L(positive_extremely_small_quotient)

// Run six by-clockwork divide steps.
        i32_divstep 7, addls
        i32_divstep 6, addls
        i32_divstep 5, addls
        i32_divstep 4, addls
        i32_divstep 3, addls
        i32_divstep 2, addls

// Short quotent and final two divide steps, where the final one
// can be optimized.
L(positive_extremely_small_quotient):
        i32_divstep 1, addls

// Last division step with no need for update.
        cmp     r1, ip
        it      ls
        addls   r0, r0, #1
        bx      lr

// Division where the divisor is [0, 4].
L(positive_special_cases):

#if __SEGGER_RTL_CORE_HAS_TBB_TBH
// Processor has TBB.  Despatch based on divisor in r1 [0..4].
        tbb     [pc, r1]
L(positive_base):
       .byte    (L(positive_divide_by_0)-L(positive_base)) / 2
       .byte    (L(positive_divide_by_1)-L(positive_base)) / 2
       .byte    (L(positive_divide_by_2)-L(positive_base)) / 2
       .byte    (L(positive_divide_by_3)-L(positive_base)) / 2
       .byte    (L(positive_divide_by_4)-L(positive_base)) / 2
        INSN_ALIGN
#else
        cmp     r1, #3
        bhi     L(positive_divide_by_4)
        beq     L(positive_divide_by_3)
        cmp     r1, #1
        bhi     L(positive_divide_by_2)
        beq     L(positive_divide_by_1)
#endif

L(positive_divide_by_0):
        IDIV0

L(positive_divide_by_4):
        lsrs    r0, r0, #2
        bx      lr

L(positive_divide_by_3):
        li      r2, 0xaaaaaaab          // Load 1/3
        umull   r3, r0, r2, r0          // Multiply by 1/3
L(positive_divide_by_2):
        lsrs    r0, r0, #1              // Final correction
L(positive_divide_by_1):
        bx      lr

// Operands of opposite signs...
L(opposite_signs):

// Ensure divisor positive and dividend negative.  a / (-b) == (-a) / b.
        tst     r1, r1
        itt     mi
        negsmi  r0, r0
        negsmi  r1, r1

// Will we have a small quotient?  If N < 2^7*D then our quotient will
// require at most 7 bits.
        cmn     r1, r0, asr #7          // Equivalent to adds r1, r0, asr #7 except that result is discarded
        it      ls
        rsbsls  r2, r1, #5
        bhi     L(negative_small_quotent)

// Find leading zero count in divisor and normalize in r3.
        NORM32  r3, r1, r2              // r3 is normalized r1, r2 set to number of shifts required to normalize

// Index into 6-bit-in, 8-bit-out reciprocal LUT.
        la      ip, __SEGGER_RTL_inverse_lut-64   // r3 will have bit 6 set, so subtract it out by folding into address computation.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        ldrb    ip, [ip, r3, lsr #25]
#else
        lsrs    r3, r3, #25
        ldrb    ip, [ip, r3]
#endif

// General division where we refine the quotient.

// R = 2^32 - B*B[r] = -(B*B[r]) with B[r] from LUT in ip.
// We have a negative dividend, so apply identity -a/b = -(b/-a).  Also see (*).
        negs    r0, r0
        subs    r2, r2, #7
        lsl     ip, ip, r2              // No short encoding for lsls variant.
        mul     r1, ip, r1              // Need to use r1, ip, r1 rather than r1, r1, ip
        umull   r2, r0, ip, r0          // This is OK, r0:r2 is different from ip...
        negs    r1, r1                  // Burying this here avoids stall on next umlal.

// A[j]h = HIGH (R*A[j-1]h + A[j-1]l)
// A[j]l = LOW (R*A[j-1]h + A[j-1]l)  ... five times
        movs    r3, #0
        umlal   r2, r3, r1, r0
        add     r0, r0, r3

// Iteration 2.
        movs    ip, #0
        umlal   r2, ip, r1, r3
        add     r0, r0, ip

// Iteration 3.
        movs    r3, #0
        umlal   r2, r3, r1, ip
        add     r0, r0, r3

// Iteration 4.
        movs    ip, #0
        umlal   r2, ip, r1, r3
        add     r0, r0, ip

// Iteration 5 and correction.
        movs    r3, #0
        umlal   r2, r3, r1, ip
        cmn     r2, r1                  // Equivalent to adds r2, r1 except that result is discarded
        adc     r0, r0, r3
        negs    r0, r0                  // (*)
        bx      lr

// Handle quotient which is less than 2^7; compute one bit at
// a time...
L(negative_small_quotent):

// We specialize cases [0..4]...
        cmp     r1, #4
        bls     L(negative_special_cases)

// ...otherwise it's a general 7-bit divide by clockwork.
        negs    ip, r0
        movs    r0, #0                  // prep quotient

// Common case for For N and D where N/D < 4.-
        cmp     r1, ip, lsr #2
        bhi     L(negative_extremely_small_quotient)

// Run six by-clockwork divide steps.
        i32_divstep 7, subls
        i32_divstep 6, subls
        i32_divstep 5, subls
        i32_divstep 4, subls
        i32_divstep 3, subls
        i32_divstep 2, subls

// Short quotent and final two divide steps, where the final one
// can be optimized.
L(negative_extremely_small_quotient):
        i32_divstep 1, subls

// Last division step with no need for update.
        cmp     r1, ip
        it      ls
        subls   r0, r0, #1
        bx      lr

// Division where the divisor is [0, 4].
L(negative_special_cases):

#if __SEGGER_RTL_CORE_HAS_TBB_TBH
// Processor has TBB.  Despatch based on divisor in r1 [0..4].
        tbb     [pc, r1]
L(negative_base):
       .byte    (L(negative_divide_by_0)-L(negative_base)) / 2
       .byte    (L(negative_divide_by_1)-L(negative_base)) / 2
       .byte    (L(negative_divide_by_2)-L(negative_base)) / 2
       .byte    (L(negative_divide_by_3)-L(negative_base)) / 2
       .byte    (L(negative_divide_by_4)-L(negative_base)) / 2
        INSN_ALIGN
#else
        cmp     r1, #3
        bhi     L(negative_divide_by_4)
        beq     L(negative_divide_by_3)
        cmp     r1, #1
        bhi     L(negative_divide_by_2)
        beq     L(negative_divide_by_1)
#endif

L(negative_divide_by_0):
        IDIV0

L(negative_divide_by_2):
        adds    r0, r0, #1
        asrs    r0, r0, #1
L(negative_divide_by_1):
        bx      lr

L(negative_divide_by_3):
        li      r2, 0xaaaaaaab          // Load 1/3
        negs    r0, r0                  // (-a)/3 = -(a/3)
        umull   r3, r0, r2, r0
        lsrs    r0, r0, #1              // Final correction.
        negs    r0, r0                  // Apply correct sign
        bx      lr

L(negative_divide_by_4):
        adds    r0, r0, #3
        asrs    r0, r0, #2
        bx      lr

#endif

END_FUNC __aeabi_idiv

/*********************************************************************
*
*       __aeabi_idivmod()
*
*  Function description
*    Divide with remainder, int.
*
*  Prototype
*    int, int __aeabi_idivmod(int Dividend, int Divisor);
*
*  Parameters
*    r0 - Dividend.
*    r1 - Divisor.
*
*  Return value
*    r0 - Quotient.
*    r1 - Remainder.
*/

#undef L
#define L(label) .L__aeabi_idivmod_##label

ARM_GLOBAL_FUNC __aeabi_idivmod

#if __SEGGER_RTL_CORE_HAS_IDIV

        sdiv    r2, r0, r1              // r2 = floor(Dividend/Divisor)
#if __SEGGER_RTL_CORE_HAS_MLS
        mls     r1, r2, r1, r0          // r1 = Dividend - Divisor * (floor(Dividend/Divisor)), the remainder
#else
        muls    r1, r1, r2              // r3 = Divisor * (floor(Dividend/Divisor))
        subs    r1, r0, r1              // r1 = Dividend - Divisor * (floor(Dividend/Divisor)), the remainder
#endif
        movs    r0, r2                  // r0 = Quotient
        bx      lr

#elif __SEGGER_RTL_CORE_HAS_UDIV_X

        BASE_X  r2                      // Set base of peripheral to r2
        UDIVM_X r0, r1, r0, r1, r2, r3  // r0 := r0 / r1, peripheral base r2, working register r3
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Calculate quotient.
        push    {r0, r1, r2, lr}
        bl      __aeabi_idiv
        pop     {r1, r2, r3}

// Calculate unsigned remainder.
#if __SEGGER_RTL_CORE_HAS_MLS
        mls     r1, r0, r2, r1
#else
        muls    r2, r0, r2
        subs    r1, r1, r2
#endif
        pop     {pc}

#else

// Calculate quotient.
        push    {r0, r1, r2, lr}
        bl      __aeabi_idiv
        pop     {r1, r2, r3, lr}

// Calculate unsigned remainder.
#if __SEGGER_RTL_CORE_HAS_MLS
        mls     r1, r0, r2, r1
#else
        muls    r2, r0, r2
        subs    r1, r1, r2
#endif
        bx      lr

#endif

END_FUNC __aeabi_idivmod

/*********************************************************************
*
*       __aeabi_uidiv()
*
*  Function description
*    Divide, unsigned.
*
*  Prototype
*    unsigned __aeabi_uidiv(unsigned Dividend, unsigned Divisor);
*
*  Parameters
*    r0 - Dividend.
*    r1 - Divisor.
*
*  Return value
*    r0 - Quotient.
*/

#undef L
#define L(label) .L__SEGGER_uidiv_##label

ARM_GLOBAL_FUNC __aeabi_uidiv

#if __SEGGER_RTL_CORE_HAS_IDIV

        udiv    r0, r0, r1              // Quotient = floor(Dividend/Divisor)
        bx      lr

#elif __SEGGER_RTL_CORE_HAS_UDIV_X

        BASE_X  r2                      // Set base of peripheral to r2
        UDIV_X  r0, r0, r1, r2, r3      // r0 := r0 / r1, peripheral base r2, working register r3
        bx      lr

#elif __SEGGER_RTL_FP_HW >= 2 && __SEGGER_RTL_USE_FPU_FOR_IDIV

        vmov         s0, r0
        vcvt.f64.u32 d16, s0
        vmov         s1, r1
        vcvt.f64.u32 d17, s1
        vdiv.f64     d16, d16, d17
        vcvt.u32.f64 s0, d16
        vmov         r0, s0             // Quotient into outgoing registers
        bx           lr

#elif 0 && (__SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T16)

// Reference fully-unrolled code for Thumb-2 or ARM.  Constant-time implementation
// but slower and bigger than what our other algorithms provide.  Useful as
// a starting point for other architectures and algorithms.
        movs    r2, r0
        movs    r0, #0
       .set     bit, 31
       .rept    31
        cmp     r1, r2, lsr #bit
        itt     ls
        subls   r2, r2, r1, lsl #bit
        addls   r0, r0, #1<<bit
       .set     bit, bit-1
       .endr
        cmp     r1, r2
        it      ls
        addls   r0, r0, #1
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Save r1; opportunistic test for division by zero.
        movs    r2, r1
        beq     L(divide_by_zero)

// Justify divisor until just less than or equal to dividend.
        movs    r3, r0
        lsrs    r0, r0, #1
        b       L(justifying)
L(justify):
        lsls    r2, r2, #1
L(justifying):
        cmp     r2, r0
        bls     L(justify)

// Zero accumulator and start division.  r3 is the remainder...
        movs    r0, #0
        b       L(dividing)

// Non-restoring division.
L(shift_subtract):
        lsrs    r2, r2, #1
L(dividing):
        subs    r3, r3, r2
        bcc     L(cant_subtract)
L(cant_add):
        adcs    r0, r0, r0
        cmp     r2, r1
        bne     L(shift_subtract)
        bx      lr

L(shift_add):
        lsrs    r2, r2, #1
        adds    r3, r3, r2
        bcs     L(cant_add)
L(cant_subtract):
        adds    r0, r0, r0
        cmp     r2, r1
        bne     L(shift_add)
        bx      lr

// Handle division by zero.
L(divide_by_zero):
        IDIV0

// NOTE The following is deactivated as it's larger and slower than the version
// that divides using a multiply-by-reciprocal technique...

#elif 0 && __SEGGER_RTL_OPTIMIZE == 0 && __SEGGER_RTL_CORE_HAS_CLZ

// Check for division by zero.
        cmp     r0, #0
        beq     L(divide_by_zero)

// Prepare to divide: dividend to ip, initialize quotient to zero.
        mov     ip, r0
        movs    r0, #0

// Find difference in magnitudes between dividend and divisor.
        clz     r2, ip
        clz     r3, r1

// If divisor is bigger than dividend, quotient is zero.
        subs    r3, r3, r2
        it      mi
        bxmi    lr

// Calculate entry into unrolled division loop.  In Arm mode, each iteration
// is three words, or 12 bytes.  In Thumb-2 mode, each iteration is 7 half-words
// or 14 bytes.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        rsbs    r3, r3, #31
        add     r3, r3, r3, lsl #1      // r3 *= 3
        add     pc, pc, r3, lsl #2      // pc + 4*r3
        nop                             // Required because Arm's PC is ahead of the game
#else
        rsbs    r3, r3, #31
        tbb     [pc, r3]
L(base):
       .set     bit, 0
       .rept    32
       .byte    (L(target)-L(base)+14*bit) / 2
       .set     bit, bit+1
       .endr
        INSN_ALIGN
#endif

// Division loop, unrolled, 31 iterations.
L(target):
       .set     bit, 31
       .rept    31
        cmp     r1, ip, lsr #bit
        itt     ls
        subls   ip, ip, r1, lsl #bit
        addls.w r0, r0, #1<<bit
       .set     bit, bit-1
       .endr

// Final iteration doesn't need to update remainder.
        cmp     r1, ip
        it      ls
        addls   r0, r0, #1
        bx      lr

L(divide_by_zero):
        IDIV0

#elif __SEGGER_RTL_OPTIMIZE < 0

// Small division routine...

// Prepare to justify divisor; cheap check for division by zero.
        movs    r2, r1
        beq     L(divide_by_zero)

// Justify divisor until just less than or equal to dividend.
L(justify):
        cmp     r2, r0, lsr #1
        it      ls
        movls   r2, r2, lsl #1
        blo     L(justify)

// Prepare quotient.
        movs    r3, #0

// Trial subtraction.
L(divide):
        cmp     r0, r2
        it      cs
        subcs   r0, r0, r2              // real subtraction only executed if trial subtraction succeeded.
        adcs    r3, r3, r3              // accumulate result

// Terminate when shifted divisor back to original position.
        cmp     r2, r1
        lsr     r2, r2, #1
        bne     L(divide)

// Recover quotient.
        movs    r0, r3

// All done.
        bx      lr

L(divide_by_zero):
        IDIV0

#else

// Capable Arm.

// Based on section 3.3.3 of "An Overview of Floating-Point Support
// and Math Library on the Intel XScale".

.macro u32_divstep bit, op
        cmp     r1, ip, lsr #\bit
        itt     ls
        subls   ip, ip, r1, lsl #\bit
        addls   r0, r0, #1<<\bit
.endm

// Will we have a small quotient?  If N < 2^7*D then our quotient will
// require at most 7 bits.  We also take the opportunity to test for
// small divisors (0..4) if the small quotient test fails.
        cmp     r1, r0, lsr #7
        it      ls
        rsbsls  r3, r1, #5
        bhi     L(small_quotent)

// Find leading zero count in divisor and normalize in r3.
        NORM32  r3, r1, r2              // r3 is normalized r1, r2 set to number of shifts required to normalize

// Index into 6-bit-in, 8-bit-out reciprocal LUT.
        la      ip, __SEGGER_RTL_inverse_lut-64   // r3 will have bit 6 set, so subtract it out by folding into address computation.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        ldrb    ip, [ip, r3, lsr #25]
#else
        lsrs    r3, r3, #25
        ldrb    ip, [ip, r3]
#endif

// General division where we refine the quotient.

// R = 2^32 - B*B[r] = -(B*B[r]) with B[r] from LUT in ip.
        subs    r2, r2, #7
        lsl     ip, ip, r2              // No short encoding for lsls variant.
        mul     r1, ip, r1              // Need to use r1, ip, r1 rather than r1, r1, ip
        umull   r2, r0, ip, r0          // This is OK, r0:r2 is different from ip...
        negs    r1, r1                  // Burying this here avoids stall on next umlal.

// A[j]h = HIGH (R*A[j-1]h + A[j-1]l)
// A[j]l = LOW (R*A[j-1]h + A[j-1]l)  ... five times
        movs    r3, #0
        umlal   r2, r3, r1, r0
        add     r0, r0, r3

// Iteration 2.
        movs    ip, #0
        umlal   r2, ip, r1, r3
        add     r0, r0, ip

// Iteration 3.
        movs    r3, #0
        umlal   r2, r3, r1, ip
        add     r0, r0, r3

// Iteration 4.
        movs    ip, #0
        umlal   r2, ip, r1, r3
        add     r0, r0, ip

// Iteration 5 and correction.
        movs    r3, #0
        umlal   r2, r3, r1, ip
        cmn     r2, r1                  // Equivalent to adds r2, r1 except that result is discarded
        adcs    r0, r0, r3
        bx      lr

// Handle quotient which is less than 2^7 or divisor is 0..4.
L(small_quotent):

// We specialize cases [0..4]...
        cmp     r1, #4
        bls     L(special_cases)

// ...otherwise it's a general 7-bit divide by clockwork.
        mov     ip, r0
        movs    r0, #0                  // prep quotient

#if __SEGGER_RTL_CORE_HAS_CLZ && __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM

// This only works in Arm mode right now (fixed-width instruction encodings), and
// provides only a small increase in performance over the always-run-7-steps version
// as there is additional setup and a branch, equivalent to ~3 steps.
        clz     r3, ip
        clz     r2, r1
        subs    r2, r2, r3
        bmi     L(divide_by_1)          // Actually a return with quotient == 0
        rsbs    r2, r2, #7
        adds    r2, r2, r2, lsl #1      // r2*3
        add     pc, pc, r2, lsl #2      // r2*12
        nop
        u32_divstep 7
        u32_divstep 6
        u32_divstep 5
        u32_divstep 4
        u32_divstep 3
        u32_divstep 2
        u32_divstep 1
        cmp     r1, ip
        it      ls
        addls   r0, r0, #1
        bx      lr

#else

// Run seven by-clockwork divide steps.
        u32_divstep 7
        u32_divstep 6
        u32_divstep 5
        u32_divstep 4
        u32_divstep 3
        u32_divstep 2
        u32_divstep 1

// Last division step with no need for update.
        cmp     r1, ip
        it      ls
        addls   r0, r0, #1
        bx      lr

#endif

// Division where the divisor is [0, 4].
L(special_cases):

#if __SEGGER_RTL_CORE_HAS_TBB_TBH
// Processor has TBB.  Despatch based on divisor in r1 [0..4].
        tbb     [pc, r1]
L(base):
       .byte    (L(divide_by_0)-L(base)) / 2
       .byte    (L(divide_by_1)-L(base)) / 2
       .byte    (L(divide_by_2)-L(base)) / 2
       .byte    (L(divide_by_3)-L(base)) / 2
       .byte    (L(divide_by_4)-L(base)) / 2
        INSN_ALIGN
#else
        cmp     r1, #3
        bhi     L(divide_by_4)
        beq     L(divide_by_3)
        cmp     r1, #1
        bhi     L(divide_by_2)
        beq     L(divide_by_1)
#endif

L(divide_by_0):
        IDIV0

L(divide_by_4):
        lsrs    r0, r0, #2
        bx      lr

L(divide_by_3):
        li      r2, 0xaaaaaaab          // Load 1/3.
        umull   r3, r0, r2, r0          // Multiply by 1/3.
L(divide_by_2):
        lsrs    r0, r0, #1              // Final correction.
L(divide_by_1):
        bx      lr

#endif

END_FUNC __aeabi_uidiv

/*********************************************************************
*
*       __aeabi_uidivmod()
*
*  Function description
*    Divide with remainder, unsigned.
*
*  Prototype
*    unsigned, unsigned __aeabi_uidivmod(unsigned Dividend, unsigned Divisor);
*
*  Parameters
*    r0 - Dividend.
*    r1 - Divisor.
*
*  Return value
*    r0 - Quotient.
*    r1 - Remainder.
*/

#undef L
#define L(label) .L__int32_udivmod_##label

ARM_GLOBAL_FUNC __aeabi_uidivmod

#if __SEGGER_RTL_CORE_HAS_IDIV

        udiv    r2, r0, r1              // r2 = floor(Dividend/Divisor)
#if __SEGGER_RTL_CORE_HAS_MLS
        mls     r1, r2, r1, r0          // r1 = Dividend - Divisor * (floor(Dividend/Divisor)), the remainder
#else
        muls    r1, r1, r2              // r3 = Divisor * (floor(Dividend/Divisor))
        subs    r1, r0, r1              // r1 = Dividend - Divisor * (floor(Dividend/Divisor)), the remainder
#endif
        movs    r0, r2                  // r0 = Quotient
        bx      lr

#elif __SEGGER_RTL_CORE_HAS_UDIVM_X

        BASE_X  r2                      // Set base of peripheral to r2
        UDIVM_X r0, r1, r0, r1, r2, r3  // r0 := r0 / r1, r1 := r0 % r1, peripheral base r2, working register r3
        bx      lr

#elif __SEGGER_RTL_FP_HW >= 2 && __SEGGER_RTL_USE_FPU_FOR_IDIV

        vmov         s0, r0
        vcvt.f64.u32 d16, s0
        vmov         s1, r1
        vcvt.f64.u32 d17, s1
        vdiv.f64     d16, d16, d17
        vcvt.u32.f64 s0, d16            // Instruction always uses round-to-zero mode
        vmov         r2, s0             // Quotient into integer register
        mls          r1, r0, r2, r1     // Calculate remainder
        mov          r0, r2             // Quotient to outgoing register
        bx           lr

#else

        push    {r3, r4, r5, lr}

// Save dividend and divisor.
        movs    r4, r0
        negs    r5, r1

// Do the division proper.
        bl      __aeabi_uidiv

// Calculate unsigned remainder and done.
#if __SEGGER_RTL_CORE_HAS_MLA
        mla     r1, r0, r5, r4
#else
        muls    r5, r5, r0
        adds    r1, r4, r5
#endif
        RET3    r3, r4, r5

#endif

END_FUNC __aeabi_uidivmod

#undef L
#define L(label) .L__aeabi_ldivmod_##label

/*********************************************************************
*
*       __aeabi_ldivmod()
*
*  Function description
*    Division with remainder, long long.
*
*  Prototype
*    long long, long long __aeabi_ldivmod(long long Dividend, long long Divisor);
*
*  Parameters
*    r1:r0 - Dividend.
*    r3:r2 - Divisor.
*
*  Return value
*    r1:r0 - Quotient.
*    r3:r2 - Remainder.
*/

ARM_GLOBAL_FUNC __aeabi_ldivmod

#if __SEGGER_RTL_FP_HW >= 2 && __SEGGER_RTL_USE_FPU_FOR_IDIV

// Try to use the FPU for small divides
        cmp          xh, xl, asr #31
        cmpne        yh, yl, asr #31
        bne          L(x_big_or_y_big)

// Both are representable in 32-signed form.
        vmov         s0, xl
        vcvt.f64.s32 d16, s0
        vmov         s1, yl
        vcvt.f64.s32 d17, s1
        vdiv.f64     d16, d16, d17
        vcvt.s32.f64 s0, d16            // Instruction always uses round-to-zero mode
        vmov         ip, s0             // Recover quotient to integer registers
        mls          yl, ip, yl, xl     // Calculate remainder
        mov          xl, ip             // Quotient into outgoing registers
        bx           lr

// Handle using generic code
L(x_big_or_y_big):

#endif

        push    {r4, r5, r6, lr}

// Compute magnitude of dividend.
        asrs    r4, xh, 31
        eors    xl, xl, r4
        eors    xh, xh, r4
        subs    xl, xl, r4
        sbcs    xh, xh, r4

// Compute magnitude of divisor.
        asrs    r5, yh, 31
        eors    yl, yl, r5
        eors    yh, yh, r5
        subs    yl, yl, r5
        sbcs    yh, yh, r5

// Do the division proper.
        bl      __aeabi_uldivmod

// Sign of remainder takes sign of dividend.
        eors    yl, yl, r4
        eors    yh, yh, r4
        subs    yl, yl, r4
        sbcs    yh, yh, r4

// Quotient is negative if the dividend and divisor signs differ.
        eors    r4, r4, r5
        eors    xl, xl, r4
        eors    xh, xh, r4
        subs    xl, xl, r4
        sbcs    xh, xh, r4
        RET3_r3 r4, r5, r6

END_FUNC __aeabi_ldivmod

/*********************************************************************
*
*       __aeabi_uldivmod()
*
*  Function description
*    Division with remainder, unsigned long long.
*
*  Prototype
*    unsigned long long, long long __aeabi_ldivmod(unsigned long long Dividend, unsigned long long Divisor);
*
*  Parameters
*    r1:r0 - Dividend.
*    r3:r2 - Divisor.
*
*  Return value
*    r1:r0 - Quotient.
*    r3:r2 - Remainder.
*/

#undef L
#define L(label) .L__int64_udivmod_##label
 
ARM_GLOBAL_FUNC __aeabi_uldivmod

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#if __SEGGER_RTL_CORE_HAS_IDIV && __SEGGER_RTL_OPTIMIZE >= 0

// This case is activated for v6-M.Baseline which has a division instruction.

// Quickly discard cases where optimization isn't possible.
        CBNZx   yh, L(y_big)
        CBNZx   xh, L(x_big_y_small)

// Dividend and divisor both less than 2^32.  Calculate quotient.
        udiv    xh, xl, yl

// Calculate remainder.
        movs    yh, yl
        muls    yh, yh, xh
        negs    yh, yh
        adds    yl, yh, xl

// Move quotient to outgoing registers and clear out workings in high-order registers.
        movs    xl, xh
        movs    xh, #0
        movs    yh, #0
        bx      lr

// See if divisor is less than 2^16, where we can optimize with schoolbook
// division in base 2^16.
L(x_big_y_small):
        CBZx    yl, L(divide_by_zero)
        lsrs    yh, yl, #16
        bne     L(x_big_y_medium)

// Divisor is less than 2^16, use schoolbook division.
        push    {r4, r5}

// Generate first quotient digit.
        movs    r4, xh
        udiv    xh, xh, yl

// Generate remainder.  Thumb-2 equivalent: mls yh, yl, xh, r4
        movs    yh, yl
        muls    yh, yh, xh
        subs    yh, r4, yh
        
// Recombine.  Thumb-2 equivalent: orr yh, yh, xl, lsr #16
        lsls    yh, yh, #16
        lsrs    r5, xl, #16
        orrs    yh, yh, r5

// Generate second quotient digit.
        udiv    r4, yh, yl

// Generate remainder.  Thumb-2 equivalent: mls yh, yl, r4, yh
        movs    r5, yl
        muls    r5, r5, r4
        subs    yh, yh, r5

// Remcombine.  Thumb-2 equivalent: orr yh, xl, yh, lsl #16
        UXTHs   xl, xl
        lsls    yh, yh, #16
        orrs    yh, yh, xl

// Generate third quotient digit.  Thumb-2 equivalent: mls yl, yl, xl, yh
        udiv    xl, yh, yl
        muls    yl, yl, xl
        subs    yl, yh, yl

// Combine quotient digts.  Thumb-2 equivalent: orr xl, xl, r4, lsl #16
        lsls    r4, r4, #16
        orrs    xl, xl, r4
        movs    yh, #0
        pop     {r4, r5}
        bx      lr

L(divide_by_zero):
        IDIV0

// Restore high part of divisor to zero after we corrupted it in a magnitude test.
L(x_big_y_medium):
        movs    yh, #0

#elif __SEGGER_RTL_OPTIMIZE >= 0

// Quickly discard cases where optimization isn't possible.
        CBNZx   yh, L(y_big)
        CBNZx   xh, L(y_big)            // More correctly, L(x_big_y_small), but use generic 64-bit divide.

// Use 32-bit division function.
        push    {r4, lr}
#if __SEGGER_RTL_BYTE_ORDER < 0
        movs    r1, r2
        bl      __aeabi_uidivmod
        movs    r2, r1
        movs    r1, #0
        movs    r3, #0
#else
        movs    r0, r1
        movs    r1, r3
        bl      __aeabi_uidivmod
        movs    r3, r1
        movs    r2, #0
        movs    r1, r0
        movs    r0, #0
#endif
        RET1_r3 r4

#endif

// Use clockwork division.
// Save registers.
L(y_big):
        push    {r4, r5, r6}

// Initialize quotient register and set "division complete" flag
// in it.  The division-complete flag, when shifted out by an add,
// indicates that all shifts are done and the quotient is complete.
        movs    r4, #0
        movs    r5, #1
        lsls    r5, r5, #31

// Can we quickly normalize by 32 bits?
        CBNZx   yh, L(clockwork_normalization)
        CBZx    xh, L(clockwork_normalization)
        cmp     yh, xl
        bcs     L(clockwork_normalization)

// Do shift by 32.
        movs    yh, yl
        movs    yl, #0
        movs    r4, r5
        movs    r5, #0

// Normalize divisor to dividend.
L(clockwork_normalization):
        movs    r6, #0
L(normalize):
        adds    r4, r4, r6
        cmp     xh, yh
        bne     L(flags_valid)
        cmp     xl, yl
L(flags_valid):
        bls     L(divide)
L(normal_shift):
        adds    yl, yl, yl
        adcs    yh, yh, yh
        bcs     L(over_normalized)
        lsls    r6, r5, #31
        lsrs    r5, r5, #1
        lsrs    r4, r4, #1
        b       L(normalize)

// We normalized one position too far.  This happens when normalizing
// 0xC000000000000000 and 1, for instance.
L(over_normalized):
        lsls    r6, yh, #31
        lsrs    yh, yh, #1
        lsrs    yl, yl, #1
        adds    yl, yl, r6
        movs    r6, #1
        lsls    r6, r6, #31
        adds    yh, yh, r6
        b       L(divide)

// Non-restoring division algorithm, which is more complex and larger than
// a restoring algorithm, but it eliminates some instructions in each loop
// iteration.

// Shift divisor right as dividend is reduced.
L(shift_subtract):
        lsls    r6, yh, #31
        lsrs    yh, yh, #1
        lsrs    yl, yl, #1
        adds    yl, yl, r6

// Non-restorng shift-subtract.
L(divide):
        subs    xl, xl, yl              // Compute low-order subtraction result
        sbcs    xh, xh, yh              // High-order subtraction result recomputed
        bcc     L(cant_subtract)
        adcs    r4, r4, r4              // Shift bit into quotient and also shift out the "division complete" flag on the last shift
        adcs    r5, r5, r5
        bcc     L(shift_subtract)       // If sentinel bit appears, division proceeds
        b       L(done)

L(cant_subtract):
        adds    r4, r4, r4              // Shift bit into quotient and also shift out the "division complete" flag on the last shift
        adcs    r5, r5, r5
        bcs     L(correct_remainder)    // If sentinel bit appears, division proceeds

// Non-restoring shift-add.
        lsls    r6, yh, #31
        lsrs    yh, yh, #1
        lsrs    yl, yl, #1
        adds    yl, yl, r6
        adds    xl, xl, yl              // Compute low-order subtraction result
        adcs    xh, xh, yh              // High-order subtraction result recomputed
        bcc     L(cant_subtract)
        adcs    r4, r4, r4              // Shift bit into quotient and also shift out the "division complete" flag on the last shift
        adcs    r5, r5, r5
        bcc     L(shift_subtract)       // If sentinel bit appears, division proceeds
        b       L(done)

// Correct remainder in this branch.
L(correct_remainder):
        adds    xl, xl, yl
        adcs    xh, xh, yh

// Remainder is what's left of the dividend.
L(done):
        movs    yl, xl
        movs    yh, xh

// Quotient developed into <r5,r4>.
        movs    xl, r4
        movs    xh, r5

// Done.
        pop     {r4, r5, r6}
        bx      lr

#elif __SEGGER_RTL_CORE_HAS_IDIV && __SEGGER_RTL_CORE_HAS_CLZ

.macro MDIV u1, u0, d, r

// d10 = d >> 22;
        lsrs    r4, \d, 22

// v0  = ((1uL << 24) - (1uL << 14) + (1uL << 9)) / d10;
#if __SEGGER_RTL_CORE_HAS_MOVW_MOVT
        movw    r5, 0xFFC2
        lsls    r5, r5, #8
#else
        li      r5, 0xFFC200
#endif
        udiv    r4, r5, r4

// d21 = (d >> 11) + 1;
        lsrs    r6, \d, 11                                            // r6:d21
        adds    r6, r6, #1

// v1 = (v0 << 4) - __SEGGER_RTL_UMULL_HI(v0 * v0, d21) - 1;
        mul     r5, r4, r4                                            // r5    := v0*v0
        umull   r5, r6, r5, r6                                        // r6:r5 := v0*v0 * d21
        mvns    r6, r6                                                // r6    := -H(v0*v0 * d21)-1
        add     r6, r6, r4, lsl #4                                    // r6    := v0<<4 -H(v0*v0 * d21)-1

// d31 = (d >> 1) + d0;
        lsrs    r5, \d, #1
//      adc     r5, r5, #0                                            // Do this in the ITEE following, placing ITEE immediately after LSRS so CM4 executes IT in zero cycles

// e = (d0 ? v1>>1 : 0) - (v1 * d31);
        itee    cc
        movcc   r7, #0                                                // if ((d & 1) == 0) e = 0
        lsrcs   r7, r6, #1                                            // if ((d & 1) == 1) e = v1 >> 1
        addcs   r5, r5, #1
        mls     r7, r5, r6, r7                                        // e := e - (v1 * d31)

// v2  = (v1 << 15) + (__SEGGER_RTL_UMULL_HI(v1, e) >> 1);
        umull   r4, r7, r6, r7                                        // r7 := __SEGGER_RTL_UMULL_HI(v1, e)
        lsrs    r7, r7, #1
        add     r7, r7, r6, lsl #15

// p0 = d; p1 = d;                               // <p0,p1>  = <d,d>
// __SEGGER_RTL_UMLAL(p0, p1, v2, d);            // <p1,p0> += <v2,d>
// v = v2 - p1;
        movs    r4, \d
        movs    r5, \d
        umlal   r4, r5, r7, \d
        subs    r7, r7, r5

// q = __SEGGER_RTL_UMULL_X(v, u1) + u + (1uLL<<32);
        adds    r5, \u1, #1
        movs    r4, \u0
        umlal   r4, r5, r7, \u1

// r = u0 - q1 * d;
        mls     r6, r5, \d, \u0

// if (r > q0) { q1 -= 1; r  += d; }
        cmp     r6, r4
        itt     hi
        subhi   r5, r5, #1
        addhi   r6, r6, \d

// if (r >= d) { q1 += 1; r -= d; }
        cmp     r6, \d
        itt     cs
        addcs   r5, r5, #1
        subcs   r6, r6, \d

// r5 is quotient, r6 remainder

.endm

// Check for large divider.
        cmp     yh, #0
        bne     L(y_big)

// Check for large dividend.
        CBNZx   xh, L(x_big_y_small)

// Divisor and dividend are both less than 2^32.
        udiv    ip, xl, yl
        mls     yl, ip, yl, xl
        mov     xl, ip
        bx      lr

// The divisor is small, check dividend.
L(x_big_y_small):

#if __SEGGER_RTL_OPTIMIZE > 0
        lsrs    yh, yl, #16
        bne     L(x_big_y_medium)

// Divisor is less than 2^16, use schoolbook division.
        mov     ip, xh
        udiv    xh, xh, yl
        mls     yh, yl, xh, ip
        lsls    yh, yh, #16
        orr     yh, yh, xl, lsr #16
        udiv    ip, yh, yl
        mls     yh, yl, ip, yh
#if __SEGGER_RTL_CORE_HAS_PKHTB_PKHBT
        pkhbt   yh, xl, yh, lsl #16
#else
        UXTHs   xl, xl
        orr     yh, xl, yh, lsl #16
#endif
        udiv    xl, yh, yl
        mls     yl, yl, xl, yh
        orr     xl, xl, ip, lsl #16
        movs    yh, #0
        bx      lr
#endif

// Check to see if overflow will occur.
L(x_big_y_medium):
        cmp     xh, yl
        bcs     L(will_overflow)

// x/y cannot overflow, one division required.  Normalize both.
        push    {r4-r8, lr}
        clz     yh, yl
        lsls    yl, yl, yh
        lsls    xh, xh, yh
        rsbs    ip, yh, #32
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orrs    xh, xh, xl, lsr ip
#else
        lsrs    ip, xl, ip
        orrs    xh, xh, ip
#endif
        lsls    xl, xl, yh

// Divide.
#if __SEGGER_RTL_OPTIMIZE > 0
        MDIV    xh, xl, yl              // divide xh:xl by yl
#else
        bl      L(mdiv)
#endif
        movs    xl, r5                  // quotient
        lsrs    yl, r6, yh              // remainder
#if __SEGGER_RTL_CORE_HAS_CLRM
        clrm    {xh, yh}
#else
        movs    xh, #0                  // Clear out high part of quotient and remainder
        movs    yh, #0
#endif
        pop     {r4-r8, pc}

//
// x/y overflows, break into two halves.
//
L(will_overflow):
        push    {r4-r7, lr}
        udiv    ip, xh, yl              // q1 = u1 / v0
        mls     xh, ip, yl, xh          // u1 = u1 - q1*v0

// Normalize.
        clz     yh, yl                  // n = __SEGGER_RTL_CLZ_U32(v0)
        lsls    yl, yl, yh              // v0 << n
        lsls    xh, xh, yh              // __SEGGER_RTL_U64_MK(u1, u0) << n
        rsbs    r6, yh, #32
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orrs    xh, xh, xl, lsr r6
#else
        lsrs    r6, xl, r6
        orrs    xh, xh, r6
#endif
        lsls    xl, xl, yh

// Divide.
#if __SEGGER_RTL_OPTIMIZE > 0
        MDIV    xh, xl, yl              // q0 = __SEGGER_RTL_Div2By1_Moeller
#else
        bl      L(mdiv)
#endif
        mov     xh, ip
        movs    xl, r5                  // quotient
        movs    yl, r6                  // remainder
        lsrs    yl, yl, yh              // Undo normalization of divider in remaindeer
        movs    yh, #0
        pop     {r4-r7, pc}

//
// v >= 2^32.
//
L(y_big):
        push    {r0, r1, r4-r7, lr}     // Save u (xh:xl) to stack.

// Normalize divisor.
        clz     ip, yh                  // n  = __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_U64_H(v));

// Ensure no overflow by dividing u by 2.
        lsrs    xh, xh, #1              // u >> 1
        rrxs    xl, xl

// Normalize divisor.
        lsls    yh, yh, ip              // __SEGGER_RTL_U64_H(v << n)
        rsbs    r4, ip, #32
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orrs    yh, yh, yl, lsr r4
#else
        lsrs    r4, yl, r4
        orrs    yh, yh, r4
#endif

// Divide.
#if __SEGGER_RTL_OPTIMIZE > 0
        MDIV    xh, xl, yh              // divide xh:xl by yh
#else
        push    {r2, r3}                // mdiv dividex <xh,xl> by yl, so some register
        movs    yl, yh                  // management is required
        bl      L(mdiv)
        pop     {r2, r3}
#endif
        
// Undo normalization of divider.
        lsrs    yh, yh, ip

// Undo normalization and division of u by 2.
        rsbs    ip, ip, #31
        lsrs    r5, r5, ip              // q0 = ((__SEGGER_RTL_U64)q1 << n) >> 31;

// Make q0 correct or too small by 1.
        it      ne                      // if (q0 != 0) { q0 -= 1; }
        subne   r5, r5, #1

// Calculate remainder: u - q0*v.
        pop     {r0, r1}                // Restore u
        umull   r6, r7, r5, yl          // <r7,r6> := v0*q0
        mla     r7, r5, yh, r7          // <r7,r6> += <v1*q0,0>
        subs    xl, xl, r6              // <r1,r0> := <u1,u0> - <r7,r6>
        sbcs    xh, xh, r7

// Check is quotient is correct.
        cmp     xl, yl                  // if ((u - q0*v) >= v) {
        sbcs    ip, xh, yh
        bcc     L(no_correction)

// Correct quotient and adjust remainder.
        subs    yl, xl, yl
        sbcs    yh, xh, yh
        adds    xl, r5, #1
        movs    xh, 0
        pop     {r4-r7, pc}

// Move to outgoing registers.
L(no_correction):
        movs    yl, xl
        movs    yh, xh
        movs    xl, r5
        movs    xh, 0

// Restore and return.
        pop     {r4-r7, pc}

#if __SEGGER_RTL_OPTIMIZE <= 0
L(mdiv):
       .type    L(mdiv), function
        MDIV    xh, xl, yl              // divide xh:xl by yl
        bx      lr
#endif

#elif __SEGGER_RTL_OPTIMIZE > 0

// Try to use the FPU for small divides.
#if __SEGGER_RTL_FP_HW >= 2 && __SEGGER_RTL_USE_FPU_FOR_IDIV

        orrs         ip, xh, yh
        bne          L(x_big_or_y_big)

// Load up values to the FPU.
        vmov         s0, xl
        vcvt.f64.u32 d16, s0
        vmov         s1, yl
        vcvt.f64.u32 d17, s1
        vdiv.f64     d16, d16, d17
        vcvt.u32.f64 s0, d16
        vmov         ip, s0             // Recover quotient to integer registers
        mls          yl, ip, yl, xl     // Calculate remainder
        mov          xl, ip             // Quotient into outgoing registers
        bx           lr

L(x_big_or_y_big):

#endif

// Based on section 3.3.3 of "An Overview of Floating-Point Support
// and Math Library on the Intel XScale".
        push    {r4-r10}

// Is y <= 0x1'0000'0000'0000'0000 or x/y <= 2^7?
        rsbs    r4, yh, #1
        it      ls
        cmpls   yh, xh, lsr #7          // Is y < 2^7 * x, i.e. quotient <= 128?
        bhi     L(small_y_or_q)

// Calculate number of significant bits in divisor (high) and normalize high divisor part 
        NORM32  r6, yh, r4              // r6 is normalized yh, r4 set to number of shifts required to normalize
        rsbs    r5, r4, #32

// Index into 6-bit-in, 8-bit-out reciprocal LUT.
        la      ip, __SEGGER_RTL_inverse_lut-64
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        lsrs    r7, yl, r5              // Combine divisor low with normalized divisor high
        orrs    r6, r6, r7
        lsrs    r6, r6, #25
        ldrb    ip, [ip, r6]
#else
        orr     r6, r6, yl, lsr r5      // Combine divisor low with normalized divisor high
        ldrb    ip, [ip, r6, lsr #25]
#endif
        subs    r4, r4, #7              // Normalize reciprocal estimate
        lsls    ip, ip, r4

// General division where we refine the quotient.

// R = 2^64 - B*B[r] = -(B*B[r]) with B[r] from LUT in ip.
        umull   r8, r5, ip, yl          // r8:r5 = yl * reciprocal estimate
        mla     r9, ip, yh, r5
        umull   r7, r6, ip, xl
        movs    r10, #0
        umlal   r6, r10, ip, xh
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        negs    r9, r9
        negs    r8, r8
        sbcs    r9, r9, #0
#else
        rsbs    r8, r8, #0
        rsc     r9, r9, #0
#endif

// A[j]h = HIGH (R*A[j-1]h + A[j-1]l)
// A[j]l = LOW (R*A[j-1]h + A[j-1]l)  ... six times with early outs
.macro divstep w, m
        movs    r4, #0
        umlal   r7, r4, r8, \m
        movs    \w, #0
        adds    r6, r6, r4
        adcs    \w, \w, \w
        umlal   r6, \w, r9, \m
        add     r10, r10, \w
.endm

        divstep ip, r10
        divstep r5, ip
        CBZx    r5, L(mult_complete)    // High probability r5 is zero here
        divstep ip, r5
        cmp     ip, #0                  // High probability ip is zero here
        beq     L(mult_complete)
        divstep r5, ip
        divstep ip, r5
        divstep r5, ip

// Quotient is calculated into r10.
L(mult_complete):
        adds    r7, r7, r8
        adcs    r6, r6, r9
        adcs    r10, r10, #0

// Calculate remainder: r = x - q*y ==> r = x + (-y * q)
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        negs    yh, yh                  // -y
        negs    yl, yl
        sbcs    yh, yh, #0
#else
        rsbs    yl, yl, #0
        rsc     yh, yh, #0
#endif
        umlal   xl, xh, yl, r10         // x += -y * q, producing remainder
        movs    yl, xl                  // Move remainder-low into outgoing remainder registers
        mla     yh, r10, yh, xh
        movs    xh, #0                  // Quotient is less than 2^32, zero high order word
        movs    xl, r10                 // Move quotient to outgoing quotient registers
        pop     {r4-r10}
        bx      lr

// Check for y < 2^32
L(small_y_or_q):
        cmp     yh, #0                  // Too far for CBNZ to reach
        bne     L(small_q_or_large_y)

// y is < 2^32 (high 32 bits in yh are zero).  Check
// for divisors 0..4 handled specially.
        cmp     yl, #5
        bcc     L(very_small_y)

// Check for y >= 0x02000000 which generates a quotient of at most 38 bits
        cmp     yl, #0x02000000
        bcs     L(medium_y)

// Check for dividend < 2^32, which is specialized.
        cmp     xh, #0
        beq     L(small_x)

// Case where it have a large dividend and small divider,
// where we specialize using iterative Newton appoximations.
        NORM32  yh, yl, r4              // yh is normalized yl, r4 set to number of shifts required to normalize

        la      ip, __SEGGER_RTL_inverse_lut-64
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        lsrs    yh, yh, #25
        ldrb    ip, [ip, yh]
#else
        ldrb    ip, [ip, yh, lsr #25]
#endif
        sub     r4, r4, #7
        lsl     ip, ip, r4
        mul     r10, ip, yl
        rsb     r10, r10, #0
        umull   r4, r8, ip, xl
        movs    r9, #0
        umlal   r8, r9, ip, xh

.macro  divstp1 l, h, x, y, exit
        movs    \l, #0
        umlal   r4, \l, \x, r10
        movs    \h, #0
        umlal   \l, \h, \y, r10
       .ifnc    \exit,
        orrs    \exit, \x, \y
        beq     L(round)
       .endif
        adds    r8, r8, \l
        adc     r9, r9, \h
.endm

.macro  divstp2 l, i
        movs    \l, #0
        umlal   r4, \l, r10, \i
        adds    r8, r8, \l
        adcs    r9, r9, #0
.endm

// Do division, 64-bit partial.
        divstp1 yh, r6, r8, r9
        divstp1 ip, r7, yh, r6
        divstp1 yh, r6, ip, r7, r7
        divstp1 ip, r7, yh, r6, r6
        divstp1 yh, r6, ip, r7, r7

// Partial reduced to 32 bits.
        CBZx    yh, L(round)
        divstp2 r5, yh
        CBZx    r5, L(round)
        divstp2 yh, r5
        CBZx    r3, L(round)
        divstp2 r5, yh
        divstp2 yh, r5
        divstp2 r5, yh
        divstp2 yh, r5

// Round low-order quotient bits in r4 into r9:r8.
L(round):
        cmn     r10, r4                 // Equivalent to adds r10, r4 except that result is discarded
        adcs    r8, r8, #0
        adcs    r9, r9, #0

// Calculate remainder.
#if __SEGGER_RTL_CORE_HAS_MLS
        mls     yl, r8, yl, xl
#else
        negs    yl, yl
        mla     yl, r8, yl, xl
#endif
        movs    yh, #0
        mov     xh, r9
        mov     xl, r8
        pop     {r4-r10}
        bx      lr

L(medium_y):

.macro  medstp1 n
        cmp     yl, xh, lsr #\n
        itt     ls
        subls   xh, xh, yl, lsl #\n
        orrls   r6, r6, #1<<\n
.endm
.macro  medstp2 n
        subs    r8, xl, yl, lsl #\n
        sbcs    r9, xh, yl, lsr #32-\n
        ittt    cs
        movcs   xl, r8
        movcs   xh, r9
        orrcs   ip, ip, #1<<\n
.endm

// Develop ms part of quotient into r6.
        movs    r6, #0
        medstp1 6
        medstp1 5
        medstp1 4
        medstp1 3
        medstp1 2
        medstp1 1
        cmp     yl, xh
        itt     ls
        subls   xh, xh, yl
        orrls   r6, r6, #1

// Develop ms part of quotient into ip.
        mov     ip, #0
        medstp2 31
        medstp2 30
        medstp2 29
        medstp2 28
        medstp2 27
        medstp2 26
        medstp2 25
        medstp2 24
        medstp2 23
        medstp2 22
        medstp2 21
        medstp2 20
        medstp2 19
        medstp2 18
        medstp2 17
        medstp2 16
        medstp2 15
        medstp2 14
        medstp2 13
        medstp2 12
        medstp2 11
        medstp2 10
        medstp2 9
        medstp2 8
        medstp2 7
        medstp2 6
        medstp2 5
        medstp2 4
        medstp2 3
        medstp2 2
        medstp2 1
        medstp2 0

// Move quotient and remainer to outgoing registers.
        movs    yh, xh
        movs    yl, xl
        movs    xh, r6
        mov     xl, ip
        pop     {r4-r10}
        bx      lr

L(small_q_or_large_y):
        movs    r5, #0
        movs    r7, #0

.macro large_y_divstep x
        subs    r6, xl, yl, lsl #\x
        sbcs    r4, xh, yl, lsr #32-\x
        itt     cs
        subscs  r4, r4, yh, lsl #\x
        cmpcs   r7, yh, lsr #32-\x
        ittt    cs
        orrcs   r5, r5, #1<<\x
        movcs   xh, r4
        movcs   xl, r6
.endm

// Do division.
        large_y_divstep 6
        large_y_divstep 5
        large_y_divstep 4
        large_y_divstep 3
        large_y_divstep 2
        large_y_divstep 1
        subs    r6, xl, yl
        sbcs    r4, xh, yh
        ittt    cs
        orrcs   r5, r5, #1
        movcs   xl, r6
        movcs   xh, r4

// Move quotient and remainder to outgoing registers.
        movs    yh, xh
        movs    yl, xl
        movs    xh, #0
        movs    xl, r5
        pop     {r4-r10}
        bx      lr

L(small_x):
        NORM32  r7, yl, r4              // r7 is normalized yl, r4 set to number of shifts required to normalize

        la      ip, __SEGGER_RTL_inverse_lut-64
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        lsrs    r7, r7, #25
        ldrb    ip, [ip, r7]
#else
        ldrb    ip, [ip, r7, lsr #25]
#endif

// Branch to special-case code if quotient will be <= 127.
        cmp     yl, xl, lsr #7
        bhi     L(short_qotient)

.macro small_x_divstep o, i
        movs    \o, #0
        umlal   r5, \o, r9, \i
        add     r8, r8, \o
.endm

// Do division.
        subs    r4, r4, #7              // Align reciprocal estimate
        lsls    ip, ip, r4
        mul     r9, ip, yl              // Form initial guess at quotient using reciprocal estimate
        negs    r9, r9
        umull   r5, r8, ip, xl
        small_x_divstep r7, r8
        small_x_divstep ip, r7
        small_x_divstep r7, ip
        small_x_divstep ip, r7
        umlal   r5, r8, r9, ip
        cmn     r5, r9                  // Equivalent to adds r5, r9 except that result is discarded
        adc     r8, r8, #0

// Calculate remainder to outgoing register.
#if __SEGGER_RTL_CORE_HAS_MLS
        mls     yl, r8, yl, xl
#else
        negs    yl, yl
        mla     yl, r8, yl, xl
#endif

// Move quotient to outgoing register.
        mov     xl, r8
        pop     {r4-r10}
        bx      lr

L(short_qotient):
        mov     ip, xl
        movs    xl, #0

.macro  sdivstp n
       .if      \n
        cmp     yl, ip, lsr #\n
        itt     ls
        subls   ip, ip, yl, lsl #\n
       .else
        cmp     yl, ip
        itt     ls
        subls   ip, ip, yl
       .endif
        orrls   xl, xl, #1<<\n
.endm

// Do division.
        sdivstp 6
        sdivstp 5
        sdivstp 4
        sdivstp 3
        sdivstp 2
        sdivstp 1
        sdivstp 0

// Move remainder to outgoing register.
        mov     yl, ip
        pop     {r4-r10}
        bx      lr

L(very_small_y):
#if __SEGGER_RTL_CORE_HAS_TBB_TBH
// Processor has TBB.  Despatch based on divisor in yl [0..4].
        tbb     [pc, yl]
L(base):
       .byte    (L(y_is_0)-L(base)) / 2
       .byte    (L(y_is_1)-L(base)) / 2
       .byte    (L(y_is_2)-L(base)) / 2
       .byte    (L(y_is_3)-L(base)) / 2
       .byte    (L(y_is_4)-L(base)) / 2
        INSN_ALIGN
#else
        cmp     yl, #3
        bhi     L(y_is_4)
        beq     L(y_is_3)
        cmp     yl, #1
        bhi     L(y_is_2)
        bcc     L(y_is_0)
#endif

// If divisor is 1, remainder is zero.
L(y_is_1):
        pop     {r4-r10}
        movs    yl, #0
        bx      lr

// Divide by 0, go through AEABI exceptional input routine.
L(y_is_0):
        pop     {r4-r10}
        LDIV0

// Divide by 2.
L(y_is_2):
        pop     {r4-r10}
        ands    yl, xl, #1              // Remainder is lower bit of dividend; know yh=0 as divisor is 2.
        lsrs    xl, xl, #1
        orr     xl, xl, xh, lsl #31
        lsrs    xh, xh, #1
        bx      lr

// Divide by 3.
L(y_is_3):
        li      yl, 0x55555555
        movs    yh, #0
        umull   r7, r4, yl, xl          // Compute triple-long product r5:r6+r4:r7
        umull   r6, r5, yl, xh
        adds    r4, r4, r6              // Combine cross products, now r5:r4:r7
        adcs    r5, r5, yh              // yh=0
        adds    r7, r7, r4
        adcs    r6, r5, yh              // yh=0
        cmp     xl, xl                  // Set carry
        adcs    r7, r7, r6
        adcs    r4, r4, r6
        adcs    r5, r5, yh

// Compute remainder, x - 3*q ==> x - 2q - q
        subs    yl, xl, r4, lsl #1      // x - 2q
        subs    yl, yl, r4              // x - 2q - q   
        movs    xl, r4
        movs    xh, r5
        pop     {r4-r10}
        bx      lr

// Divide by 4 by shifting two bits down.
L(y_is_4):
        and     yl, xl, #3              // Remainder is lower two bits of dividend; know yh=0 as divisor is 4.
        lsrs    xl, xl, #2
        orr     xl, xl, xh, lsl #30
        lsrs    xh, xh, #2
        pop     {r4-r10}
        bx      lr

#elif !__SEGGER_RTL_CORE_HAS_CLZ || __SEGGER_RTL_OPTIMIZE <= -2

#if __SEGGER_RTL_OPTIMIZE >= 0
        orrs    ip, xh, yh
        bne     L(x_big_y_big)

// Use unsigned integer division function.
        push    {r4, r5, r6, lr}        // Need to push an even number of registers to maintain stack alignment

// Save divided and divisor as quotient required.
        movs    r4, xl
        negs    r5, yl                  // Negate divisor for use in mla to compute u - q*v.

// Rearrange registers for call to division code.
#if __SEGGER_RTL_BYTE_ORDER > 0
        movs    r0, xl
#endif
        movs    r1, yl
        bl      __aeabi_uidiv

// Calculate remainder.
        mla     yl, r0, r5, r4

// Clear out high parts of outgoing registers.
#if __SEGGER_RTL_BYTE_ORDER > 0
        movs    xl, r0
#endif
        movs    xh, #0
        movs    yh, #0

// Return maintaining r3's value.
        RET3_r3 r4, r5, r6

#endif

// Divisor, dividend, or both >= 2^32, or selected generic division to reduce code space
L(x_big_y_big):

// Save registers.
        push    {r4, r5}

// Initialize quotient register and set "division complete" flag
// in it.  The division-complete flag, when shifted out by an add,
// indicates that all shifts are done and the quotient is complete.
        movs    r4, #0
        movs    r5, #0x80000000

// Normalize divisor to dividend.
L(normalize):
        cmp     xh, yh
        it      eq
        cmpeq   xl, yl
        bls     L(divide)
        adds    yl, yl, yl
        adcs    yh, yh, yh
        bcs     L(over_normalized)
        lsrs    r5, r5, #1
        rrxs    r4, r4
        bls     L(normalize)
L(over_normalized):
        rrxs    yh, yh
        rrx     yl, yl
        b       L(divide)

// Shift divisor right as dividend is reduced.
L(shift_subtract):
        lsrs    yh, yh, #1
        rrxs    yl, yl

// Check to see whether subtraction can be made.
L(divide):
        cmp     xl, yl                  // Compare low parts
        sbcs    ip, xh, yh              // Compare with carry high parts and save result to ip...
        itt     cs                      // ...If the subtraction could me made, the carry flag is set
        subcs   xl, xl, yl              // Compute low-order subtraction result
        movcs   xh, ip                  // High-order subtraction result is already computed -- if we had to do this here, we'd corrupt the carry flag
        adcs    r4, r4, r4              // Shift bit into quotient and also shift out the "division complete" flag on the last shift
        adcs    r5, r5, r5
        bcc     L(shift_subtract)       // If carry is clear, division proceeds

// Remainder is what's left of the dividend.
        movs    yl, xl
        movs    yh, xh

// Quotient developed into <r5,r4>.
        movs    xl, r4
        movs    xh, r5

// Done.
        pop     {r4, r5}
        bx      lr

#else

// Dispatch on magnitudes of dividend and divisor.
        CBNZx   yh, L(y_big)
        CBNZx   xh, L(x_big_y_small)

// Use unsigned integer division function.
        push    {r4, r5, r6, lr}        // Need to push an even number of registers to maintain stack alignment

// Save divided and divisor as quotient required.
        movs    r4, xl
        negs    r5, yl                  // Negate divisor for use in mla to compute u - q*v.

// Rearrange registers for call to division code.
#if __SEGGER_RTL_BYTE_ORDER > 0
        movs    r0, xl
#endif
        movs    r1, yl
        bl      __aeabi_uidiv

// Calculate remainder.
        mla     yl, r0, r5, r4

// Clear out high parts of outgoing registers.
#if __SEGGER_RTL_BYTE_ORDER > 0
        movs    xl, r0
#endif
        movs    xh, #0
        movs    yh, #0

// Return maintaining r3's value.
        RET3_r3 r4, r5, r6

// Dividend is >= 2^32, divisor is < 2^32.
L(x_big_y_small):

// Now know 2^32 <= divisor <= dividend, hence dividend >= 2^32.
// Save registers.
        push    {r4, r5}

// Calculate difference in magnitudes (minus 32).
        clz     r4, yl
        clz     r5, xh
        subs    r4, r4, r5
        bpl     L(shift_left)

// Shift divisor to align with dividend, the divisor crosses words.
        negs    r4, r4
        lsrs    yh, yl, r4
        rsbs    r5, r4, #32
        lsls    yl, yl, r5

// Compensate if over-shifted.
        cmp     xh, yh
        it      eq
        cmpeq   xl, yl
        bhs     L(setup_0)
        lsrs    yh, yh, #1
        rrxs    yl, yl
        subs    r5, r5, #1

// Initialize quotient register and set "division complete" flag
// in it.  The division-complete flag, when shifted out by an add,
// indicates that all shifts are done and the quotient is complete.
L(setup_0):
        movs    r4, #0x80000000
        lsrs    r5, r4, r5
        movs    r4, #0
        b       L(divide)

// Shift divisor within words.
L(shift_left):
        lsls    yh, yl, r4
        movs    yl, #0

// Compensate if over-shifted.
        cmp     xh, yh
        it      eq
        cmpeq   xl, yl
        bhs     L(setup_2)
        lsrs    yh, yh, #1
        rrxs    yl, yl
        subs    r4, r4, #1

// Initialize quotient register and set "division complete" flag
// in it.  The division-complete flag, when shifted out by an add,
// indicates that all shifts are done and the quotient is complete.
L(setup_2):
        tst     r4, r4
        itttt   mi
        addmi   r4, r4, #32
        movmi   r5, #0x80000000
        lsrmi   r5, r5, r4
        movmi   r4, #0
        ittt    pl
        movpl   r5, #0x80000000
        lsrpl   r4, r5, r4
        movpl   r5, #0
        b       L(divide)

// Divisor is >= 2^32.
L(y_big):

// If divisor is greater than dividend, early out.
        cmp     xl, yl
        sbcs    ip, xh, yh
        bcc     L(early_out)

// Now know 2^32 <= divisor <= dividend, hence dividend >= 2^32.
// Save registers.
        push    {r4, r5}

// Calculate difference in magnitudes.
        clz     r4, yh
        clz     r5, xh
        subs    r4, r4, r5

// Shift divisor to align with dividend.
        rsbs    r5, r4, #32
        lsls    yh, yh, r4
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orrs    yh, yh, yl, lsr r5
#else
        lsrs    r5, yl, r5
        orrs    yh, yh, r5
#endif
        lsls    yl, yl, r4

// Compensate if over-shifted.
        cmp     xh, yh
        it      eq
        cmpeq   xl, yl
        bhs     L(setup)
        lsrs    yh, yh, #1
        rrxs    yl, yl
        subs    r4, r4, #1

// Initialize quotient register and set "division complete" flag
// in it.  The division-complete flag, when shifted out by an add,
// indicates that all shifts are done and the quotient is complete.
L(setup):
        movs    r5, #0x80000000
        lsrs    r5, r5, r4
        movs    r4, #0
        b       L(divide)

// Shift divisor right as dividend is reduced.
L(shift_subtract):
        lsrs    yh, yh, #1
        rrxs    yl, yl

// Check to see whether subtraction can be made.
L(divide):
        cmp     xl, yl                  // Compare low parts
        sbcs    ip, xh, yh              // Compare with carry high parts and save result to ip...
        itt     cs                      // ...If the subtraction could me made, the carry flag is set
        subcs   xl, xl, yl              // Compute low-order subtraction result
        movcs   xh, ip                  // High-order subtraction result is already computed -- if we had to do this here, we'd corrupt the carry flag
        adcs    r4, r4, r4              // Shift bit into quotient and also shift out the "division complete" flag on the last shift
        adcs    r5, r5, r5
        bcc     L(shift_subtract)       // If carry is clear, division proceeds

// Remainder is what's left of the dividend.
        movs    yl, xl
        movs    yh, xh

// Quotient developed into <r5,r4>.
        movs    xl, r4
        movs    xh, r5

// Done.
        pop     {r4, r5}
        bx      lr

// Enter here when dividend < divisor.
L(early_out):
        movs    yl, xl                  // Remainder is the dividend
        movs    yh, xh
        movs    xl, #0                  // Quotient is zero
        movs    xh, #0
        bx      lr

#endif
  
END_FUNC __aeabi_uldivmod

#undef L
#define L(label) .L__SEGGER_clzsi2_##label

ARM_GLOBAL_FUNC __clzsi2

#if __SEGGER_RTL_CORE_HAS_CLZ

        clz     r0, r0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T16

        NORM32  r0, r0, r2
        movs    r0, r2
        bx      lr

#elif __SEGGER_RTL_OPTIMIZE < -1
        movs    r3, #0

L(count):
        adds    r3, r3, #1
        lsls    r0, r0, #1
        bcc     L(count)
L(done):
        subs    r0, r3, #1
        bx      lr

#else

        movs    r3, #0
        lsrs    r2, r0, #16
        bne     L(1)
        lsls    r0, r0, #16
        adds    r3, r3, #16
L(1):
        lsrs    r2, r0, #24
        bne     L(2)
        lsls    r0, r0, #8
        adds    r3, r3, #8
L(2):
        lsrs    r2, r0, #28
        bne     L(3)
        lsls    r0, r0, #4
        adds    r3, r3, #4
L(3):
        lsrs    r2, r0, #30
        bne     L(4)
        lsls    r0, r0, #2
        adds    r3, r3, #2
L(4):
        mvns    r0, r0
        lsrs    r0, r0, #31
        adds    r0, r0, r3
        bx      lr

#endif

END_FUNC __clzsi2

#undef L
#define L(label) .L__SEGGER_clzdi2_##label

ARM_GLOBAL_FUNC __clzdi2

#if __SEGGER_RTL_CORE_HAS_CLZ && (__SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T16)

        cmp     xh, #0
        itee    ne
        clzne   r0, xh
        clzeq   r0, xl
        addeq   r0, r0, #32
        bx      lr

#else

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

//
// If the high word is zero, initialize count to 32 and shift by 32.
//
        movs    r3, #32
        CBZx    xh, L(high_zero)
        movs    r3, #0
        movs    xl, xh
L(high_zero):

#else

//
// If the high word is zero, initialize count to 32 and shift by 32.
//
        movs    r3, #32
        cmp     xh, #0
        itt     ne
        movne   r3, #0
        movne   xl, xh

#endif

#if __SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T16

        lsrs    r2, xl, #16
        lsleq   xl, xl, #16
        addeq   r3, r3, #16
        lsrs    r2, xl, #24
        lsleq   xl, xl, #8
        addeq   r3, r3, #8
        lsrs    r2, xl, #28
        lsleq   xl, xl, #4
        addeq   r3, r3, #4
        lsrs    r2, xl, #30
        lsleq   xl, xl, #2
        addeq   r3, r3, #2
        mvns    xl, xl
        lsrs    xl, xl, #31
        adds    r3, r3, xl
        movs    r0, r3
        bx      lr

#elif __SEGGER_RTL_OPTIMIZE < -1

L(count):
        adds    r3, r3, #1
        lsls    xl, xl, #1
        bcc     L(count)
L(done):
        subs    r0, r3, #1
        bx      lr

#else
        lsrs    r2, xl, #16
        bne     L(1)
        lsls    xl, xl, #16
        adds    r3, r3, #16
L(1):
        lsrs    r2, xl, #24
        bne     L(2)
        lsls    xl, xl, #8
        adds    r3, r3, #8
L(2):
        lsrs    r2, xl, #28
        bne     L(3)
        lsls    xl, xl, #4
        adds    r3, r3, #4
L(3):
        lsrs    r2, xl, #30
        bne     L(4)
        lsls    xl, xl, #2
        adds    r3, r3, #2
L(4):
        mvns    xl, xl
        lsrs    r0, xl, #31
        adds    r0, r0, r3
        bx      lr
#endif

#endif

END_FUNC __clzdi2

#undef L
#define L(label) .L__SEGGER_clzdi2_##label

/*********************************************************************
*
*       __aeabi_imod()
*
*  Function description
*    Remainder, signed.
*
*  Prototype
*    int __aeabi_uimod(int Dividend, int Divisor);
*
*  Parameters
*    r0 - Dividend.
*    r1 - Divisor.
*
*  Return value
*    r0 - Remainder.
*
*  Additional information
*    This is not part of the Arm EABI standard but is here in order for
*    the tests to recover the remainder from __aeabi_idivmod().
*/
ARM_GLOBAL_FUNC __aeabi_imod

       push     {r3, lr}
       bl       __aeabi_idivmod
       movs     r0, r1
       RET1     r3

END_FUNC __aeabi_imod

/*********************************************************************
*
*       __aeabi_uimod()
*
*  Function description
*    Remainder, unsigned.
*
*  Prototype
*    unsigned __aeabi_uimod(unsigned Dividend, unsigned Divisor);
*
*  Parameters
*    r0 - Dividend.
*    r1 - Divisor.
*
*  Return value
*    r0 - Remainder.
*
*  Additional information
*    This is not part of the Arm EABI standard but is here in order for
*    the tests to recover the remainder from __aeabi_uidivmod().
*/
ARM_GLOBAL_FUNC __aeabi_uimod

       push     {r3, lr}
       bl       __aeabi_uidivmod
       movs     r0, r1
       RET1     r3

END_FUNC __aeabi_uimod

/*********************************************************************
*
*       __aeabi_ulmod()
*
*  Function description
*    Remainder, unsigned.
*
*  Prototype
*    long long __aeabi_lmod(long long Dividend, long long Divisor);
*
*  Parameters
*    r1:r0 - Dividend.
*    r3:r2 - Divisor.
*
*  Return value
*    r0 - Remainder.
*
*  Additional information
*    This is not part of the Arm EABI standard but is here in order for
*    the tests to recover the remainder from __aeabi_ldivmod().
*/
ARM_GLOBAL_FUNC __aeabi_lmod

        push    {r3, lr}
        bl      __aeabi_ldivmod
        movs    r0, r2                  // This works for any word order (LE, BE)
        movs    r1, r3
        RET1    r3

END_FUNC __aeabi_lmod

/*********************************************************************
*
*       __aeabi_ulmod()
*
*  Function description
*    Remainder, unsigned.
*
*  Prototype
*    unsigned long long __aeabi_ulmod(unsigned long long Dividend, unsigned long long Divisor);
*
*  Parameters
*    r1:r0 - Dividend.
*    r3:r2 - Divisor.
*
*  Return value
*    r0 - Remainder.
*
*  Additional information
*    This is not part of the Arm EABI standard but is here in order for
*    the tests to recover the remainder from __aeabi_uldivmod().
*/
ARM_GLOBAL_FUNC __aeabi_ulmod

        push    {r3, lr}
        bl      __aeabi_uldivmod
        movs    r0, r2                  // This works for any word order (LE, BE)
        movs    r1, r3
        RET1    r3

END_FUNC __aeabi_ulmod

/*********************************************************************
*
*       __aeabi_idiv0()
*
*  Function description
*    Divide by zero handler.
*
*  Prototype
*    int_or_unsigned __aeabi_idiv0(int_or_unsigned Dividend, int_or_unsigned Divisor);
*
*  Parameters
*    r0 - Dividend.
*    r1 - Divisor.
*
*  Return value
*    r0 - Quotient set to zero.
*/
ARM_GLOBAL_FUNC __aeabi_idiv0

        movs    r0, #0
        movs    r1, #0
        bx      lr

END_FUNC __aeabi_idiv0

/*********************************************************************
*
*       __aeabi_ldiv0()
*
*  Function description
*    Divide by zero handler.
*
*  Prototype
*    int_or_unsigned_long_long __aeabi_idiv0(int_or_unsigned_long_long Dividend, int_or_unsigned_long_long Divisor);
*
*  Parameters
*    r1:r0 - Dividend.
*    r3:r2 - Divisor.
*
*  Return value
*    r1:r0 - Quotient set to zero.
*    r3:r2 - Remainder set to zero.
*/
ARM_GLOBAL_FUNC __aeabi_ldiv0

        movs    xl, #0
        movs    xh, #0
        movs    yl, #0
        movs    yh, #0
        bx      lr

END_FUNC __aeabi_ldiv0

#endif

       .end

/*************************** End of file ****************************/
