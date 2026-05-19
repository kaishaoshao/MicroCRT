/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File        : floatasmops_arm.s
Purpose     : SEGGER Floating Point Emulator for Arm processors.

References  : [1]  Run-time ABI for the ARM Architecture
                   http://infocenter.arm.com/help/topic/com.arm.doc.ihi0043d/IHI0043D_rtabi.pdf

Implementation notes
--------------------

Section 4.1.1.1 of [1] provides a dispensation that subnormals
may be flushed to zero.  This implementation will flush inputs
that are subnormal to zero; also, it never generates a result
that is subnormal, subnormal results are flushed to zero.  This
model reflects FPUs that run in a standard mode where subnormals
are flushed to zero.  It also has a benfit as it reduces code
footprint and removes the complexity of dealing with inputs
and outputs that occur rarely in practice.

Entry points:

  __aeabi_fadd()
  __aeabi_fmul()
  __aeabi_fdiv()
  __aeabi_fsub()
  __aeabi_frsub()
  __aeabi_fcmpeq()
  __aeabi_fcmplt()
  __aeabi_fcmple()
  __aeabi_fcmpgt()
  __aeabi_fcmpge()
  __aeabi_fcmpun()
  __aeabi_cfcmpeq()
  __aeabi_cfcmple()
  __aeabi_cfrcmple()
  __aeabi_f2iz()
  __aeabi_f2uiz()
  __aeabi_f2lz()
  __aeabi_f2ulz()
  __aeabi_f2d()
  __aeabi_i2f()
  __aeabi_ui2f()
  __aeabi_l2f()
  __aeabi_ul2f()

  __aeabi_dadd()
  __aeabi_dmul()
  __aeabi_ddiv()
  __aeabi_dsub()
  __aeabi_drsub()
  __aeabi_dcmpeq()
  __aeabi_dcmplt()
  __aeabi_dcmple()
  __aeabi_dcmpgt()
  __aeabi_dcmpge()
  __aeabi_dcmpun()
  __aeabi_cdcmpeq()
  __aeabi_cdcmple()
  __aeabi_cdrcmple()
  __aeabi_d2iz()
  __aeabi_d2uiz()
  __aeabi_d2lz()
  __aeabi_d2ulz()
  __aeabi_d2f()
  __aeabi_i2d()
  __aeabi_ui2d()
  __aeabi_l2d()
  __aeabi_ul2d()

*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#if !defined(__aarch64__)

@ gnu_syntax

#include "asmdefs_arm.ah"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#if __SEGGER_RTL_INCLUDE_AEABI_API == 2  // AEABI API with assembly speedups

/*********************************************************************
*
*       Local data
*
**********************************************************************
*/

// Reciprocal table for single divide

#if __SEGGER_RTL_OPTIMIZE >= 2

LOCAL_RODATA __aeabi_fdiv_reciprocal_table, 4

// 6 bits in, 32 bits out
       .word 0x820fe040, 0x860fa233, 0x890f6604, 0x8d0f2b9d
       .word 0x900ef2eb, 0x930ebbdb, 0x970e865b, 0x990e525a
       .word 0x9c0e1fc8, 0x9f0dee96, 0xa20dbeb6, 0xa40d901b
       .word 0xa60d62b8, 0xa90d3681, 0xab0d0b6a, 0xad0ce169
       .word 0xaf0cb872, 0xb10c907e, 0xb30c6981, 0xb50c4373
       .word 0xb70c1e4c, 0xb80bfa03, 0xba0bd691, 0xbc0bb3ee
       .word 0xbd0b9214, 0xbf0b70fc, 0xc00b509e, 0xc10b30f6
       .word 0xc30b11fd, 0xc40af3ae, 0xc50ad603, 0xc70ab8f7
       .word 0xc80a9c85, 0xc90a80a8, 0xca0a655c, 0xcb0a4a9d
       .word 0xcc0a3066, 0xcd0a16b3, 0xce09fd81, 0xcf09e4cb
       .word 0xd009cc8e, 0xd109b4c7, 0xd2099d72, 0xd309868d
       .word 0xd3097013, 0xd4095a02, 0xd5094458, 0xd6092f11
       .word 0xd7091a2b, 0xd70905a4, 0xd808f178, 0xd908dda5
       .word 0xd908ca2a, 0xda08b703, 0xdb08a430, 0xdb0891ac
       .word 0xdc087f78, 0xdc086d90, 0xdd085bf3, 0xde084aa0
       .word 0xde083993, 0xdf0828cc, 0xdf081849, 0xe0080808

END_RODATA __aeabi_fdiv_reciprocal_table

#endif

// Bipartite reciprocal tables for double divide.

LOCAL_RODATA __aeabi_ddiv_reciprocal_table, 2

// 7-bits-in, 16-bits out table TH, each entry is floor(2^16/(1.<b1..b7> + 2^-7))
       .hword 0xfe00, 0xfc0c, 0xfa20, 0xf83c, 0xf660, 0xf488, 0xf2b8, 0xf0f0
       .hword 0xef2c, 0xed70, 0xebbc, 0xea0c, 0xe864, 0xe6c0, 0xe524, 0xe38c
       .hword 0xe1fc, 0xe070, 0xdee8, 0xdd64, 0xdbe8, 0xda74, 0xd900, 0xd794
       .hword 0xd628, 0xd4c4, 0xd368, 0xd20c, 0xd0b4, 0xcf64, 0xce14, 0xcccc
       .hword 0xcb84, 0xca44, 0xc904, 0xc7cc, 0xc698, 0xc564, 0xc434, 0xc30c
       .hword 0xc1e4, 0xc0c0, 0xbfa0, 0xbe80, 0xbd68, 0xbc50, 0xbb3c, 0xba2c
       .hword 0xb920, 0xb814, 0xb70c, 0xb608, 0xb508, 0xb408, 0xb30c, 0xb214
       .hword 0xb11c, 0xb02c, 0xaf38, 0xae4c, 0xad60, 0xac74, 0xab8c, 0xaaa8
       .hword 0xa9c8, 0xa8e8, 0xa808, 0xa72c, 0xa654, 0xa57c, 0xa4a8, 0xa3d4
       .hword 0xa304, 0xa234, 0xa168, 0xa0a0, 0x9fd8, 0x9f10, 0x9e4c, 0x9d88
       .hword 0x9cc8, 0x9c08, 0x9b4c, 0x9a90, 0x99d4, 0x991c, 0x9868, 0x97b4
       .hword 0x9700, 0x964c, 0x95a0, 0x94f0, 0x9444, 0x9398, 0x92f0, 0x9248
       .hword 0x91a0, 0x90fc, 0x9058, 0x8fb8, 0x8f14, 0x8e78, 0x8dd8, 0x8d3c
       .hword 0x8ca0, 0x8c08, 0x8b70, 0x8ad8, 0x8a40, 0x89ac, 0x8918, 0x8888
       .hword 0x87f4, 0x8764, 0x86d8, 0x8648, 0x85bc, 0x8534, 0x84a8, 0x8420
       .hword 0x8398, 0x8310, 0x828c, 0x8208, 0x8184, 0x8100, 0x8080, 0x8000
       
// 7-bits-in, 8-bits out table TL, each entry is floor(2^8/(1.<b1..b7> + 2^-7)^2)
       .byte 0xfc, 0xf8, 0xf4, 0xf0, 0xec, 0xe8, 0xe6, 0xe2, 0xde, 0xdc, 0xd8, 0xd4, 0xd2, 0xd0, 0xcc, 0xca
       .byte 0xc6, 0xc4, 0xc2, 0xbe, 0xbc, 0xba, 0xb6, 0xb4, 0xb2, 0xb0, 0xae, 0xac, 0xaa, 0xa8, 0xa4, 0xa2
       .byte 0xa0, 0x9e, 0x9c, 0x9a, 0x9a, 0x98, 0x96, 0x94, 0x92, 0x90, 0x8e, 0x8c, 0x8c, 0x8a, 0x88, 0x86
       .byte 0x84, 0x84, 0x82, 0x80, 0x80, 0x7e, 0x7c, 0x7a, 0x7a, 0x78, 0x76, 0x76, 0x74, 0x74, 0x72, 0x70
       .byte 0x70, 0x6e, 0x6e, 0x6c, 0x6c, 0x6a, 0x68, 0x68, 0x66, 0x66, 0x64, 0x64, 0x62, 0x62, 0x60, 0x60
       .byte 0x60, 0x5e, 0x5e, 0x5c, 0x5c, 0x5a, 0x5a, 0x58, 0x58, 0x58, 0x56, 0x56, 0x54, 0x54, 0x54, 0x52
       .byte 0x52, 0x52, 0x50, 0x50, 0x4e, 0x4e, 0x4e, 0x4c, 0x4c, 0x4c, 0x4a, 0x4a, 0x4a, 0x4a, 0x48, 0x48
       .byte 0x48, 0x46, 0x46, 0x46, 0x44, 0x44, 0x44, 0x44, 0x42, 0x42, 0x42, 0x42, 0x40, 0x40, 0x40, 0x40

END_RODATA __aeabi_ddiv_reciprocal_table

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

       .syntax unified

/*********************************************************************
*
*       __aeabi_fadd()
*
*  Function description
*    Add, single floating.
*
*  Prototype
*    float __aeabi_fadd(float x, float, y);
*
*  Parameters
*    r0 - x - Augend.
*    r1 - y - Addend.
*
*  Return value
*    r0 - Sum, x+y.
*/

#undef L
#define L(label) .L__aeabi_fadd_##label

ARM_GLOBAL_FUNC __aeabi_fadd

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vadd.f32 s0, s0, s1
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define ADD32_WORKING_SET  r4, r5

// Generate 0x80000000.
        movs    r2, #1
        lsls    r2, r2, #31

// Do addend and augend have the same sign?
LOCAL_ENTRY .L__aeabi_fadd_fast_entry
        movs    r3, r0
        eors    r3, r3, r1
        bmi     L(subtract)             // No, this is a subtraction

// Save working registers.
        push    {ADD32_WORKING_SET}

// Get largest of x and y into x, smallest into y.  Hence
// augend is largest.
L(add_same_sign):
        subs    r3, r0, r1              // compute z=x-y
        bcs     L(add_already_ordered)  // x>y?
        subs    r0, r0, r3              // x=x-(x-y) = x-x+y = y
        adds    r1, r1, r3              // y=y+(x-y) = y+x-y = x
L(add_already_ordered):

// Discard sign bit of smaller, align significand.
        lsls    r3, r1, #1

// Extract exponent of larger into r4.
        lsls    r4, r0, #1
        lsrs    r4, r4, #24

// If the exponent of x (the largest) is Inf or NaN, return the largest.
        cmp     r4, #0xFF
        beq     L(add_inf_or_nan)

// If the exponent of the largest is zero, then we need to sort out adding -0.
        CBZx    r4, L(zero)

// Extract exponent of smaller.  If adding zero or subnormal, then we're done.
        lsrs    r3, r3, #24
        beq     L(add_done)

// Find difference between exponents which is the number of places
// needed to align them.  We cancel the sign bits in the subtraction
// because we know they are the same in addend and augend.
        subs    r3, r4, r3

// If the difference is big, come out immediately.
        cmp     r3, #24
        bgt     L(add_done)

// Isolate significand of augend into high 24 bits and set the hidden.
// bit.  We know this is never a subnormal.
        lsls    r1, r1, #8
        orrs    r1, r1, r2

// Extract exponent and sign of larger.
        lsrs    r4, r0, #23

// Move significand of largest into high part and set hidden bit.
        lsls    r0, r0, #8
        orrs    r0, r0, r2

// Compute number of bits to shift smaller to align significands for addition.
        negs    r5, r3
        adds    r5, r5, #25

// Calculate residual that is not added to retained part of significand, but
// will be required for correct rounding.
        movs    r2, r1
        lsls    r2, r2, r5
        lsrs    r2, r2, #7

// Align smaller significand.
        lsrs    r1, r1, r3

// Compute sum of significands.
        adds    r5, r0, r1

// If we carried out, normalize sum.
        bcc     L(add_no_normalization)
        lsrs    r5, r5, #1              // Re-normalize.
        adds    r4, r4, #2              // Adjust exponent.

// Move exponent into position.
L(add_no_normalization):
        subs    r4, r4, #1
        lsls    r4, r4, #23

// Remove residual part.
        lsrs    r0, r5, #8

// Generate 0x80000000 into r1 as loading it closer to the time it's used
// will destroy the carry bit we wish to preserve.
        movs    r1, #1
        lsls    r1, r1, #31

// Get excess signifand bits used for rounding (the bits not stored into the result).
        lsls    r5, r5, #24
        orrs    r2, r2, r5

// Combine significand with exponent and sign and round, the rounding
// is in the carry bit.
        sbcs    r2, r2, r1              // Generate rounding bit
        adcs    r0, r0, r4

// Check for sum overflow ignoring sign of exponent.
        lsls    r1, r0, #1
        lsrs    r1, r1, #24
        cmp     r1, #0xFF
        bcc     L(add_done)

// Overflowed with the exponent set to 0xff with correct sign.
// Clear significand bits to zero to generate a correctly-signed Inf.
        lsrs    r0, r0, #23
        lsls    r0, r0, #23

// All done.
L(add_done):
        pop     {ADD32_WORKING_SET}
        bx      lr

// Adding +0 + +0 or -0 + -0, so return correctly-signed zero.
L(zero):
        lsrs    r0, r0, #31
        lsls    r0, r0, #31
        b       L(add_done)

// The largest (now lhs) is Inf or NaN; if Inf, return, else convert to NaN.
L(add_inf_or_nan):
        lsls    r2, r0, #9              // Discard sign and exponent, isolate significand
        beq     L(add_done)             // If Inf, return Inf

// Generate an AEABI NaN.
        movs    r0, #0x7F               // 0x7f
        lsls    r0, r0, #4              // 0x7f0
        adds    r0, r0, #0xC            // 0x7fc
        lsls    r0, r0, #20             // 0x7fc00000
        b       L(add_done)

// Signs differ so must do subtract.
L(subtract):

// For subtract, we require fewer registers than addition.  Amazing.
        mov     ip, r4

// Make signs the same in order to compare magnitudes.
        eors    r1, r1, r2

// Get largest of x and y into x, smallest into y.
        subs    r3, r0, r1              // compute z=x-y
        bcs     L(sub_already_ordered)  // x>y?
        eors    r3, r3, r2              // Flip sign
        subs    r0, r0, r3              // x=x-(x-y) = x-x+y = y
        adds    r1, r1, r3              // y=y+(x-y) = y+x-y = x
L(sub_already_ordered):

// Discard sign bit of smaller, top 8 bits of r3 are exponent.
        lsls    r3, r1, #1

// Shift significand into position and set the hidden bit.
        lsls    r1, r1, #8
        orrs    r1, r1, r2

// Extract exponent of smaller into r3.
        lsrs    r3, r3, #24

// Extract exponent of larger into r4.
        lsls    r4, r0, #1
        lsrs    r4, r4, #24

// If the exponent of x (the largest) is Inf or NaN, handle it out of line
// as this is not the common case.
        cmp     r4, #0xFF
        beq     L(sub_inf_or_nan)

// If smaller is subnormal or zero, handle out of line.
        CBZx    r3, L(sub_zero)

// Compute difference of exponents.  An extra unit is subtracted to
// differentiate a couple of distinguished cases.
        negs    r3, r3
        adds    r3, r3, r4
        cmp     r3, #1

// If exactly aligned, subtract now.  Because the subtract (and thus compare)
// above compared the larger exponent and the smaller exponent, the only way
// that the carry can be clear is if the two exponents were equal on compare
// and the carry was subtracted leaving the borrow clear.
        bcc     L(exponents_equal)

// If they have a difference of exactly one, fall through.  That is, if
// exponent(x) - exponent(y) - 1 != 0, i.e. exponent(x) - exponent(y) != 1.
        bne     L(exponents_differ_by_more_than_1)

// Here the exponents differ by exactly one.
// Extract exponent and sign of bigger to r3.
        lsrs    r3, r0, #23

// Align significand of larger to msb--we drop the hidden bit, it
// doesn't matter.
        lsls    r0, r0, #9

// Do the subtraction.  As the exponents differ by one, we can never
// generate a zero difference...
        subs    r0, r0, r1

// ...however, we will will either have some normalizations to do
// (e.g. 1000 - 0100 = 0100) or no normalization to do (1100 - 0100 = 1000).
        bcc     L(normalization_steps)

// Here we have a zero-step normalization case (1100 - 0100, for instance).
// Move result exponent and sign into position.
        lsls    r3, r3, #23

// Can't do this any other way, I think, as we need that carry bit too many
// times.  Take a copy of the difference.
        movs    r1, r0

// Move significand into position.
        lsrs    r0, r0, #9

// As exponents differ by only a single bit, we have a single bit which
// determines rounding, as the significands are misaligned by a single bit...
// Move single bit to rounding bit to top, carry set is non-zero shifted out.
        lsls    r1, r1, #23

// Generate rounding bit to carry; r2 is 0x80000000.
        sbcs    r1, r1, r2

// Round, insert exponent and sign, and done.
        adcs    r0, r0, r3
L(sub_done):
        mov     r4, ip
        bx      lr

// Subtracting zero or subnormal.
L(sub_zero):
        tst     r4, r4                  // non-zero-x - 0 is non-zero-x
        bne     L(sub_done)
        movs    r0, #0                  // 0-0 is +0
        b       L(sub_done)

// Here we subtract two values with equal exponents; these will cancel the leading
// bits at least, so we know we have at least one normalization step.
L(exponents_equal):

// Extract exponent and sign of bigger to r3.
        lsrs    r3, r0, #23

// Align significand of larger to msb--we drop the hidden bit, it
// doesn't matter.
        lsls    r0, r0, #9

// Align significands and compute difference.
        lsls    r1, r1, #1
        subs    r0, r0, r1

// If difference is exactly zero, we're done.
        beq     L(sub_done)

// The computed difference must now be normalized...  r1 contains the number
// of shifts required to normalize.  It would be nice to directly adjust the
// exponent in r3, but r3 also contains the result sign in bit 8, so detecting
// underflow is a problem (e.g. 0x108 with 10 places to shift generates 0xfe,
// which is the same as 0x0fe with 0 places to shift.)
L(normalization_steps):
        movs    r1, #0

// Normalize result.
L(sub_normalize):
        adds    r1, r1, #1
        lsls    r0, r0, #1
        bcc     L(sub_normalize)

// Will the result underflow?  If the exponent is very small and we needed
// a lot of steps in normalization, which will generate a zero or negative
// exponent, then the exponent will be too small and we can flush to zero.
        cmp     r4, r1
        bls     L(underflow)

// Adjust exponent by number of places shifted and move into position.
        subs    r3, r3, r1
        lsls    r3, r3, #23

// Move significand into position.
        lsrs    r0, r0, #9

// Combine exponent and sign with significancd, and we're done.
        adds    r0, r0, r3
        b       L(sub_done)

// Result underflowed.  r3 contains the exponent and sign (9 bits), so 
L(underflow):
        lsrs    r0, r3, #8              // extract sign to lsb
        lsls    r0, r0, #31             // move sign into position
        b       L(sub_done)

// If the difference is big, come out immediately.  Subtracting a small
// number from a significantly larger one is no problem.
L(exponents_differ_by_more_than_1):
        cmp     r3, #25
        bhi     L(sub_done)

// Adjust exponent for shifts.
        adds    r3, r3, #1

// Shift r1 right by 32-r3 bits into r4--this is the residual that we're not subtracting.
// We don't have a register left to compute 32-r3, so synthesize it.  We also align
// the significand of the subtrahend (in r1) with that of the minuend (in r0).
        movs    r4, r1
        lsrs    r1, r1, r3
        rors    r4, r4, r3
        eors    r4, r4, r1

// Fold bits into low order subtrahend; if any of the bits shifted out are non-zero,
// the we need to fold it into r1.
        cmp     r4, #1                  // C=1 if r4 >= 1, i.e. r4 != 0
        adcs    r1, r1, r1

// Extract sign and exponent of minuend into r3.
        lsrs    r3, r0, #23

// Align minuend significand to msb and insert the hidden bit.
        lsls    r0, r0, #8
        orrs    r0, r0, r2

// Do the subtraction.  You might find this slightly strange that we're computing
// the difference into r4, but we do this in order that we can do rounding below at
// (*) et seq.
        subs    r4, r0, r1

// Correctly normalize result.  In this case we know that the subtraction can
// never need more than a single normalization step because the most significant
// bit can only change from a one to a zero as the exponents differ by 1 or more.
        bmi     L(sub_already_normalized)

// Shift r4 lest one bit to normalize and ensure that top bit is zero.  We do
// this by shifting two left and one right.
        lsls    r4, r4, #2
        lsrs    r4, r4, #1
L(sub_already_normalized):

// Adjust exponent.
        subs    r3, r3, #1

// Move exponent into position.
        lsls    r3, r3, #23

// Move significand into position.  (*)
        lsrs    r0, r4, #8

// Generate rounding bit from remainder in lower order 8 bits of difference.
        lsls    r4, r4, #24
        sbcs    r4, r4, r2              // r2 = 0x80000000

// Insert exponent and sign and rounding bit.
        adcs    r0, r0, r3

// Restore working register and return.
        mov     r4, ip
        bx      lr

// LHS is Inf or NaN.
L(sub_inf_or_nan):
        cmp     r3, #0xFF               // rhs is Inf or NaN?
        beq     L(sub_nan)              // Inf-Inf is NaN, Inf+NaN is NaN, NaN+Inf is NaN.

// RHS a NaN?
        lsls    r1, r0, #9
        beq     L(sub_done)

// Generate an AEABI NaN.
L(sub_nan):
        movs    r0, #0x7F               // 0x7f
        lsls    r0, r0, #4              // 0x7f0
        adds    r0, r0, #0xC            // 0x7fc
        lsls    r0, r0, #20             // 0x7fc00000
        b       L(sub_done)

#else

// Pre-load exponent mask.
        mov     ip, #0xFF000000

// Do addend and augend have the same sign?
        teq     r0, r1
        bmi     L(subtract)

// Get largest of x and y into x, smallest into y.
        subs    r3, r0, r1              // compute z=x-y
        itt     cc
        subcc   r0, r0, r3              // if x<y, x=x-(x-y) = x-x+y = y
        addcc   r1, r1, r3              // if x<y, y=y+(x-y) = y+x-y = x

// If the exponent of x (the largest) is Inf or NaN, return the largest.
        cmp     ip, r0, lsl #1
        bls     L(add_inf_or_nan)

// If the exponent of the smallest is zero, then it's finite+0.
        tst     ip, r1, lsl #1
        beq     L(add_zero)

// Isolate excess-127 exponent and significand signs of augend.
        lsrs    r2, r0, #23             // Use narrow Thumb-2 encoding

// Find difference between exponents which is the number of places
// needed to align them.  We cancel the sign bits in the subtraction
// because we know they are the same in addend and augend.
        subs    r3, r2, r1, lsr #23

#if __SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T32
// If the difference is big, exit immediately.
        cmp     r3, #24
        it      gt
        bxgt    lr
#endif

// Normalize and materialize hidden bit.
        mov     ip, #0x80000000
        orr     r0, ip, r0, lsl #8
        orr     r1, ip, r1, lsl #8

// Calculate residual that is not added to retained part of significand, but
// will be required for correct rounding.
        rsb     ip, r3, #32

// Form non-rounded non-normalized sum in r0.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        lsl.w   ip, r1, ip              // discarded part held in ip
        lsrs.w  r1, r1, r3              // Require full shift here, hence force wide
        adds    r0, r0, r1
#else
        lsl     ip, r1, ip              // discarded part held in ip
        adds    r0, r0, r1, lsr r3
#endif

// Form non-rounded normalized sum.
        itee    cc
        subcc   r2, r2, #1              // No carry, correctly normalized to top bit, adjust exponent.
        movscs  r0, r0, rrx             // With carry, overflowed so rotate back setting C...
        orrcs   r0, r0, #1              // ...and preserve sticky bit

// Round result breaking ties to nearest even.
        orrs    ip, ip, r0, lsl #25     // Bring in any normalized part
        itt     eq
        tsteq   r0, #0x100              // Z=1 if tied
        biceq   r0, r0, #0x80           // If tied, round to even

// Move significand into position.
        lsrs    r0, r0, #8

// Merge exponent and sign and fold rounding.
        adc     r0, r0, r2, lsl #23

// Check for sum overflow ignoring sign of sum.
        mov     r1, #0x01000000
        cmn     r1, r0, lsl #1
        it      cc
        bxcc    lr                      // No overflow, return

// Overflowed with the exponent set to 0xff with correct sign.
// Clear significand bits to zero to generate a correctly-signed Inf.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        bfc     r0, #0, #23
#else
        lsrs    r0, r0, #23             // Use narrow Thumb-2 encoding
        lsls    r0, r0, #23             // Use narrow Thumb-2 encoding
#endif
        bx      lr

L(add_zero):
        tst     ip, r0, lsl #1
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        bne     L(return)
#else
        bxne    lr
#endif

// Adding +0 + +0 or -0 + -0, so return correctly-signed zero.
L(zero):
        and     r0, r0, #0x80000000
L(return):
        bx      lr

// Signs differ so must do subtract.
L(subtract):

// Make signs the same in order to compare magnitudes.
        eor     r1, r1, #0x80000000

// Get largest of x and y into x, smallest into y.
        subs    r3, r0, r1              // compute z=x-y
        ittt    cc
        eorcc   r3, r3, #0x80000000     // Flip sign
        subcc   r0, r0, r3              // if x<y, x=x-(x-y) = x-x+y = y
        addcc   r1, r1, r3              // if x<y, y=y+(x-y) = y+x-y = x

// If the exponent of x (the largest) is Inf or NaN, handle it out of line
// as this is not the common case.
        cmp     ip, r0, lsl #1
        bls     L(sub_inf_or_nan)

// Subtracting zero.
        tst     ip, r1, lsl #1
        beq     L(sub_zero)

// Isolate excess-127 exponent and significand signs of augend.
        lsrs    r2, r0, #23

// Find difference between exponents which is the number of places
// needed to align them.  We cancel the sign bits in the subtraction
// because we know they are the same in subtrahend and minuend.
        sub     r3, r2, r1, lsr #23

// Isolate significand and materialize hidden bits.
        mov     ip, #0x80000000
        orr     r1, ip, r1, lsl #8
        orr     r0, ip, r0, lsl #8

// Set flags for non-retained significand bits.
        rsb     ip, r3, #32             // Compute number of shifts
        lsls    ip, r1, ip              // Set Z=1 only if all bits shifted out are zero

// Shift significand right jamming sticky bit.
        lsr     r1, r1, r3
        it      ne
        orrne   r1, r1, #1              // If bits shifted out are non-zero, remember them.

// Subtract (aligned) significands.
        subs    r0, r0, r1
        bmi     L(sub_already_normalized)

// If result is zero, then return zero immediately as normalization
// isn't required.
        beq     L(done)

// Break significand sign and exponent into different parts.
        lsrs    r3, r2, #8              // Save result sign in r3.0
        UXTBs   r2, r2                  // Exponent only.

// Normalization.
        NORM32D r0, r0, r2, r1, f_exp   // r0 is normalized, r2 decremented by number of shifts required to normalize, r1 is a temnporary in case of CLZ, set flags based on exponent (r2)
        ble     L(sub_zero_result)

// Recombine sign and exponent.
        add     r2, r2, r3, lsl #8

// Subtract done and normalized.  Now round the difference.
L(sub_already_normalized):

// Pack result irrespective of underflow.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        ubfx    r1, r0, #8, #23
        lsls    r0, r0, #25             // Shift guard bits
#else
        lsls    r0, r0, #1              // Move significand to lower 24 bits.
        lsrs    r1, r0, #9
        lsls    r0, r0, #24
#endif
        adc     r0, r1, r2, lsl #23     // Place in result exponent and round up.

// Break ties.
        it      cc
        bxcc    lr
        it      eq
        biceq   r0, r0, #1
        bx      lr

// Subtracting zero is simple, apart from flushing subnormals to zero.
L(sub_zero):
        tst     ip, r0, lsl #1
        it      eq
        moveq   r0, #0
L(done):
        bx      lr

// Result underflowed, flush to signed zero.
L(sub_zero_result):
        lsls    r0, r3, #31             // Place in result sign. Use narrow Thumb-2 encoding
        bx      lr

// On entry we know that the numerically largest value in r0 is either an
// Inf or NaN.  Z is set for Inf and clear for NaN.  If we have the lhs
// as Inf, the rhs cannot be a NaN (else it would have been moved to the
// lhs) so return Inf.

// We only need determine if we are performing +Inf + -Inf or -Inf + +Inf
// in which case we need to generate a NaN.
L(sub_inf_or_nan):
        cmp     ip, r1, lsl #1          // rhs is Inf?
        beq     L(return_nan)           // Inf - Inf is NaN

// The largest (now lhs) is Inf or NaN; if Inf, return, else convert to NaN.
L(add_inf_or_nan):
        cmp     ip, r0, lsl #1          // Is LHS NaN?
        it      eq
        bxeq    lr                      // No, return Inf.

L(return_nan):
        li      r0, 0x7fc00000          // Return an AEBI NaN.
        bx      lr

#endif

END_FUNC __aeabi_fadd

/*********************************************************************
*
*       __aeabi_dadd()
*
*  Function description
*    Add, double floating.
*
*  Prototype
*    double __aeabi_dadd(double x, double y);
*
*  Parameters
*    r1:r0 - x - Augend, hi:lo.
*    r3:r2 - y - Addend, hi:lo.
*
*  Return value
*    r1:r0 - Sum, hi:lo, x+y.
*/

#undef L
#define L(label) .L__aeabi_dadd_##label

ARM_GLOBAL_FUNC __aeabi_dadd

#if __SEGGER_RTL_FP_HW >= 2

// Enter with integer registers, use FPU to calculate.
        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vadd.f64 d0, d0, d1
        vmov    xl, xh, d0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define ADD64_WORKING_SET r4, r5, r6, r7

        push    {ADD64_WORKING_SET}

// Generate 0x80000000
        movs    r5, #1
        lsls    r5, r5, #31

// If subtraction, do subtraction.
LOCAL_ENTRY .L__aeabi_dadd_fast_entry
        movs    r7, xh
        eors    r7, r7, yh
        bmi     L(subtract)

// Signs identical: 64-bit compare of magnitudes of addend and augend.
L(signs_identical):
        cmp     yh, xh
        bne     1f
        cmp     yl, xl
1:      bls     L(already_ordered)

// Addend is greater than augend: swap addend and augend.
        eors    xl, xl, yl
        eors    yl, yl, xl
        eors    xl, xl, yl
        eors    xh, xh, yh
        eors    yh, yh, xh
        eors    xh, xh, yh

// Extract exponent of lhs to r6.
L(already_ordered):
        adds    r6, xh, xh
        lsrs    r6, r6, #21

// Extract exponent of rhs to r7.
        adds    r7, yh, yh
        lsrs    r7, r7, #21

// If rhs exponent is zero, we're adding zero (including denormalized numbers.)
        beq     L(add_zero)

// If lhs exponent is Inf or NaN, then return that.
        adds    r4, r6, #1              // Topple 0x7ff to 0x800
        lsls    r4, r4, #21             // shift off sign bit, if zero then input exponent was 0x7ff
        beq     L(done)

// Compute difference in exponents.
        subs    r4, r6, r7

// If we can't align the exponents, the addend does not contribute to the sum.
        cmp     r4, #53
        bhi     L(done)

// Extract smaller exponent.
        lsrs    r6, xh, #20

// Materialize hidden bit in smaller summand.
        lsls    yh, yh, #11
        orrs    yh, yh, r5
        lsrs    yh, yh, #11

// Materialize hidden bit in larger summand.
        lsls    xh, xh, #11
        orrs    xh, xh, r5
        asrs    xh, xh, #11             // This is an ASR to drag in one bits from the left.
                                        // We can use this to determine whether the sum is
                                        // normalized

// When we are aligning significands, the extra bits shifted off are captured
// in r7 to allow us to round correctly.

// Pre-initialize sticky bit bin with lower half of significand as we know
// we must shift anyway.
        movs    r7, yl

// Exponents are aligned more than 32 bits apart?
        cmp     r4, #32
        bcs     L(add_shifted_word)

// Exponents differ by less than 32. In this case we must align both
// halves of the significand and add them together.  Shift yl right r4 places
// and the bits shifted off are captured in r7 (already initialized above).
        rors    r7, r7, r4
        lsrs    yl, yl, r4
        eors    r7, r7, yl

// Shift yh right r4 places and the bits shifted off are captured into the
// (zero) high bits of yl, the low-order word.
        rors    yh, yh, r4
        eors    yl, yl, yh
        lsls    yh, yh, r4
        lsrs    yh, yh, r4
        eors    yl, yl, yh

// Initialize a "zero" register for use in adc instructions.
        movs    r4, #0

// Compute sum.
        adds    xl, xl, yl
        adcs    xh, xh, yh

// If result is normalized already, no need for a normalization step.
        bcc     L(already_normalized)

// Addition requires normalization as it overflowed.
L(normalization_required):

// Exponent overflowed?  Compare to 0x7fe by adding 2 amd seeing if the result is zero modulo 0x800.
        adds    yl, r6, #2
        lsls    yl, yl, #21
        beq     L(inf)

// We are going to shift xh:xl right by one bit and shift off a single bit.
// We also wish to maintain the sticky bits already held in r7.  To do this,
// we move the bit shifted off into the msb of the sticky bit bin and compress
// the existing bits in the sticky bit bin (to a single bit in the lsb).
        lsls    yl, xl, #31             // Shift lsb of significand to (virtual) msb of sticky bit bin for rounding
        cmp     r7, #1                  // C=0 iff sticky bit bin == 0, 1 otherwise
        adcs    yl, yl, r4              // Combine compressed sticky bit with rounding bit shifted out
        movs    r7, yl                  // r7 becomes the combined sticky bit bin

// Shift xh:xl right one bit.
        lsrs    xl, xl, #1
        lsls    yh, xh, #31
        orrs    xl, xl, yh
        lsrs    xh, xh, #1

// Move exponent into position.
L(already_normalized):
        adds    r6, r6, #1              // Adjust exponent bias; sign already present in r6.
        lsls    r6, r6, #20             // Move exponent into position

// Perform rounding and merge exponent.
        lsrs    yl, xl, #1              // C = lsb of significand
        sbcs    r7, r7, r5
        adcs    xl, xl, r4
        adcs    xh, xh, r6

// All done.
L(done):
        pop     {ADD64_WORKING_SET}
        bx      lr

// Exponents differ by 32 bits or more.  Sticky bit bin already 
// initialized with lower half of significand.
L(add_shifted_word):

// If exponents differ by exactly 32, then r7 is already primed
// as the sticky bit bin and we just need to add the shifted
// significand.
        subs    r4, #32
        beq     L(already_aligned)

// Need to shift yh:yl right by 32+r4 bits to align significands.
// When shifting, we catch the bits shifted out in the high order
// bits of r7.
        movs    r7, yh
        rors    r7, r7, r4
        lsrs    yh, yh, r4
        eors    r7, r7, yh
        movs    r4, #0
        cmp     yl, #1
        adcs    r7, r7, r4

// Form sum. Precondition is that r4 == 0 on entry.
L(already_aligned):
        adds    xl, xl, yh
        adcs    xh, xh, r4
        bcc     L(already_normalized)
        b       L(normalization_required)

// We know the rhs is to be treated as zero.  If the lhs is non-zero, then
// return the lhs.
L(add_zero):
        cmp     r6, #0                  // if lhs exponent is non-zero then we have a normal
        bne     L(done)

// 0 + 0 = 0 and -0 + -0 = -0, so extract sign.
        ands    xh, xh, r5
        movs    xl, #0
        b       L(done)

// Generate an infinity.
L(inf):
        adds    r6, r6, #1              // r6 is now sign + exponent of Inf
        lsls    xh, r6, #20             // move exponent and sign into position
        movs    xl, #0                  // Inf requires zero significand
        b       L(done)

// Know lhs is Inf or NaN.  If rhs is Inf or NaN then the result is NaN
// as Inf-Inf is NaN and any NaN operand generates a NaN.
L(sub_inf_or_nan):
        cmn     r7, r4                  // Exactly Inf?
        bne     L(done)                 // No, we're through.

// Generate an ARM EABI NaN.
L(nan):
        movs    xl, #0
        mvns    xh, xl                  // 0xffffffff
        lsrs    xh, xh, #20             // 0x00000fff
        lsls    xh, xh, #19             // 0x7ff80000:0x00000000
        b       L(done)

// Signs differ so must do subtract.
L(subtract):

// Make signs the same.
        eors    yh, yh, r5

// Get largest of x and y into x, smallest into y.
        movs    r6, xh
        subs    r7, xl, yl
        sbcs    r6, r6, yh
        bcs     L(sub_already_ordered)  // x>y

// Exchange x and y using some basic algebra and configure resulting sign.
        eors    r6, r6, r5              // invert output signs
        subs    xl, xl, r7
        sbcs    xh, xh, r6              // y=y-(x-y) = y-x+y = x  (Use narrow encoding in Thumb-2)
        adds    yl, yl, r7              // x=x-(x-y) = x-x+y = y
        adcs    yh, yh, r6

// Discard signs ready to compare exponents to special Inf/NaN exponent.
L(sub_already_ordered):
        adds    r6, xh, xh              // Cast out lhs sign, ditto for rhs
        adds    r7, yh, yh

// Compare largest exponent to Inf exponent shifted one bit (i.e.
// 0xffe00000 which is -0x00200000).
        lsrs    r4, r5, #10             // generate 0x00200000
        cmn     r6, r4
        bcs     L(sub_inf_or_nan)

// Know that lhs and rhs are both normal.  Extract exponents of lhs and rhs.
        lsrs    r6, r6, #21             // exponent of larger
        lsrs    r7, r7, #21             // exponent of smaller
        beq     L(subtracting_zero)     // rhs is zero on subtract

// Compute difference in exponents which is the number of bits to align significands.
        subs    r4, r6, r7
        cmp     r4, #54
        bhi     L(done)

// Save sign and exponent.
        mov     ip, r6
        lsrs    r6, xh, #20

// Materialize hidden bit in rhs.
        lsls    yh, yh, #11
        orrs    yh, yh, r5
        lsrs    yh, yh, #11

// Materialize hidden bit in lhs.
        lsls    xh, xh, #11
        orrs    xh, xh, r5
        lsrs    xh, xh, #11

// Special-case some subtractions.  We have distinguish three cases:
// exponents differ by 0, 1, or more than one which use different paths
// through the code.  In the case of 0 or 1, we have the issue of
// leading digit cancellation and normalization, with 2 and above, we don't.
        cmp     r4, #1
        bhi     L(sub_align_far)
        bne     L(sub_already_aligned)

// Difference in exponents is one, so align by a single bit and use r4 as
// a significand extension register.
        lsls    r4, yl, #31             // r4{31} is the extended significand, all other bits of r4 are zero
        lsrs    yl, yl, #1              // move low down, yl{31} zero
        lsls    r7, yh, #31             // extract lsb of high part to r7{31}
        orrs    yl, yl, r7              // merge with low part
        lsrs    yh, yh, #1              // and finally shift high part down to align

// Exponents are correctly aligned.  When we subtract now we can cause
// cancellation of leading bits and require normalization.
L(sub_already_aligned):
        lsrs    r7, r5, #11             // generate 0x00100000 which is the mask to test for normalization

// Subtract; r4 holds a single bit in r4{31} that is the result of a
// a single-bit normalization.
        negs    r4, r4                  // start with 0-significand extension
        sbcs    xl, xl, yl
        sbcs    xh, xh, yh

// If the high word of the difference significand is zero, we may
// be able to shift quickly.  We assume that cancellation of
// leading digits is unlikely and move it off the mainline.
        beq     L(high_word_cancelled)

// If the significand is already normalized, then we must round as we have
// a rounding bit left in r4{31}.
        movs    yl, #0                  // precondtion for entering sub_normalized
        tst     xh, r7
        bne     L(sub_normalized)

// This is the first normalization step, and we have a single bit in the
// subtrahend's signifcand extension register r4.  In this case we don't need
// to break any ties or consider round-to-nearest rounding.
L(first_normalization_step):
        adds    r4, r4, r4              // Move r4{31} to carry and set r4=0 always
        adcs    xl, xl, xl              // normalization step
        adcs    xh, xh, xh
        movs    r4, #1                  // one normalization step done, set normalization count.

// If we're already normalized, that's great.
        tst     xh, r7
        bne     L(normalized)

// Perform bit-by-bit normalization of xh:xl, keeping normalization count in r4.
L(normalize):
        adds    r4, r4, #1              // keep count of normalization steps required
        adds    xl, xl, xl              // one bit at a time...
        adcs    xh, xh, xh
L(pre_normalize):
        tst     xh, r7                  // normalized?
        beq     L(normalize)            // ...not yet

// Now have significand normalized in xh:xl.  If the normalization count is less than the
// input exponent, we have a subnormal.  In this case we flush to an appropriately-signed zero.
L(normalized):
        mov     yl, ip
        cmp     yl, r4
        bls     L(signed_zero)

// Adjust exponent by number of places required to normalize difference's significand.
        subs    r6, r6, r4

// Move exponent into position put into result.
        subs    r6, r6, #1              // adjust exponent as we have a '1' in the hidden bit which is the lsb of the exponent
        lsls    r6, r6, #20             // shift into position
        adds    xh, xh, r6              // merge with significand, and adjust by one because of exponent...
        b       L(done)

// Return a signed zero.  The sign bit is stored in r6[11].
L(signed_zero):
        lsrs    xh, r6, #11             // sign bit to xl[0].
        lsls    xh, xh, #31             // ...into position
        movs    xl, #0                  // low significand is zero
        b       L(done)                 // ...and done

// Subtracting zero.  0-0 is always +ve zero.
L(subtracting_zero):

// Is lhs zero (include subnormals)?  If it is then we generate a +ve zero
// otherwise we return a positive zero, clearing out subnormals.
        tst     r6, r6
        bne     L(done)

// Return a +ve zero for 0-0 for any sign of zero operands.
        movs    xl, #0
        movs    xh, #0
        b       L(done)

// High word has become zero, see if the low word can be moved into the
// high word.  We have to make sure that we don't "over normalize".
L(high_word_cancelled):

// If we have generated a zero, we're done and return an appropriate
// zero, otherwise we need to normalize.
        movs    yl, xl
        orrs    yl, yl, r4
        beq     L(done)

// Can we move the low word to the high word without over-normalizing?
// If not, do so, we've had at least 32 bits of leading digit cancellation...
        cmp     xl, r7
        bcs     L(first_normalization_step)

// At least 32 leading zero bits have been generated across xh:xl.  In
// this case we shift the difference's significand by 32 bits immediately.
        movs    xh, xl
        movs    xl, r4
        movs    r4, #32                 // Normalization count is 32, we've just shifted 32 bits
        bne     L(pre_normalize)

// Difference in exponents is 2 or more bits.  In this case leading digit cancellation
// is not an issue.  As the larger sigificand has its msb set, and the smaller significand
// has its leading two bits clear, we do not expect any more than one normalization step.
// For instance, subtracting the least 1000...-0011... = 0101... which requires one
// step, and 1111...-0010... = 1101... which requires zero normalization steps.
L(sub_align_far):
        cmp     r4, #32
        ble     L(aligned_on_top)

// This is now what we wish to subtract, in general, with the exponent difference
// between the minuend and subtrahend as 32+r4

//   +--------------+--------------+---------------+
//   |      xh      |      xl      |000000000000000|
//   +--------------+--------------+----+----------*----+
// -                     |      yh      |      yl  |****|
//                       +--------------+----------*----+

// We can easily shift by 32 bits by changing registers, and adjust the shift
// count by 32 to perform alignment of yh with xl.  Because we must round correctly,
// we need to compute the 96-bit difference using xh:xl extended with zero bits.
// The low-order bits of the difference are computed into r7.
//
// In the diagram above, the bits marked with stars are shifted off the end and
// caught as sticky bits.

// Adjust shift count as we'll add high to low which is an implicit shift by 32.
        subs    r4, r4, #32

// Shift yh:yl right r4 places to align significands, and catch bits shifted out
// into high bits of r7.  r7 is will contain the low order bits of the subtrahend
// which align to the right.
        rors    yl, yl, r4
        movs    r7, yl
        lsls    yl, yl, r4
        lsrs    yl, yl, r4
        eors    r7, r7, yl
        rors    yh, yh, r4
        eors    yl, yl, yh
        lsls    yh, yh, r4
        lsrs    yh, yh, r4
        eors    yl, yl, yh

// We now have
//
//   +--------------+--------------+---------------+
//   |      xh      |      xl      |000000000000000|
//   +--------------+--------------+---------------*------------+
// -                |0000    yh    |      yl       |**** r7 0000|
//                  +--------------+---------------*------------+
//
// We need to fold the sticky bits in r7 into yl.  That is, if r7 is non-zero,
// we need to set the lsb of yl, and if r7 is zero, then no change to yl.
// We can do this with the help of the carry flag.
        cmp     r7, #1                  // C is true iff r7<>0
        sbcs    r7, r7, r7              // R7=0 iff C, -1 iff not C
        adds    r7, r7, #1              // R7=1 iff C, 0 iff not C
        orrs    r7, r7, yl              // set bit iff C; r7 now takes the place of yl
        movs    yl, #0                  // prepare for sbcs below

// Subtract (r7+yl) from and place rounding bits into r4.
        negs    r4, r7                  // 0 - (r7+yl) above
        sbcs    xl, xl, yh              // xl-yh above
        sbcs    xh, xh, yl              // xh-0 above
        b       L(sub_normalize)

// Shift fewer than 32 bits...
L(aligned_on_top):

// This is now what we wish to subtract, in general, with the exponent difference
// between the minuend and subtrahend as 32+r4

//   +--------------+--------------+---------------+
//   |      xh      |      xl      |000000000000000|
//   +--------------+--------------+----+----------+
// -      |      yh      |      yl |******|
//        +--------------+---------*------+

// In the diagram above, the bits marked with stars are shifted off the end and
// caught as sticky bits in r7.

// Shift yh:yl right r4 places to align significands, and catch bits shifted out
// into high bits of r7.  r7 is will contain the low order bits of the subtrahend
// which align to the right.
        movs    r7, yl
        rors    r7, r7, r4
        lsrs    yl, yl, r4
        eors    r7, r7, yl              // yl is now aligned to subtract from xl and
                                        // r7 contains bits shifted out from yl
        rors    yh, yh, r4              // Rotate right, keep all bits working
        eors    yl, yl, yh              // Mix in the upper and low bits to yl, we'll unmix them later (at *)
        lsls    yh, yh, r4              // Clear upper bits of yh
        lsrs    yh, yh, r4
        eors    yl, yl, yh              // Remove excess bits (*)

// Now have:

//   +--------------+--------------+-------------+
//   |      xh      |      xl      |0000000000000|
//   +--------------+--------------+-------------+
// - |0000    yh    |      yl      |**** r7 00000|
//   +--------------+--------------*-------------+

// Do subtraction above and place rounding bits into r4.
        negs    r4, r7                  // 0 - * above
        sbcs    xl, xl, yl              // xl-yl
        sbcs    xh, xh, yh              // xh-yh
        movs    yl, #0                  // Precondition when branching to sub_normalized

// Test to see whether we're normalized (bit 21)
L(sub_normalize):
        lsls    r7, xh, #12
        bcs     L(sub_normalized)

// Adjust exponent and normalize one place.
        subs    r6, r6, #1
        adds    r4, r4, r4
        adcs    xl, xl, xl
        adcs    xh, xh, xh

// NB: All paths that enter here must have yl==0.
L(sub_normalized):

// Adjust exponent by 1 as we already have a '1' in the exponent position
// because we've normalized and have the hidden bit materialized (in the lsb
// of the exponent).
        subs    r6, r6, #1

// Move exponent into position
        lsls    r6, r6, #20

// Round and insert exponent.
        lsrs    r7, xl, #1
        sbcs    r4, r4, r5
        adcs    xl, xl, yl
        adcs    xh, xh, r6

// And done.
        pop     {ADD64_WORKING_SET}
        bx      lr

#else

// z0 and z1 are registers that will be used as temporaries.  In Thumb-2 we don't use
// lr and ip because that requires wide T2 instructions rather than T1 instructions.
// Instead we push r6 and r7 to the stack and use r6/r7 in preference to ip/lr, both of
// which are in the low register set.

#define ADD64_WORKING_SET r4, r5
#define z0 ip
#define z1 lr

// Save working registers.
        push    {ADD64_WORKING_SET, lr}

// Do addend and augend have the same sign?
        teq     xh, yh
        bmi     L(subtract)

// Get largest of x and y into x, smallest into y.
        subs    r4, xl, yl              // compute z[z0:r4]=x-y
        sbcs    z0, xh, yh
        bcs     L(add_already_ordered)  // x>y

// Exchange x and y using some basic algebra.
        adds    yl, yl, r4              // x=x-(x-y) = x-x+y = y
        adcs    yh, yh, z0              // Use narrow encoding in Thumb-2
        subs    xl, xl, r4
        sbcs    xh, xh, z0              // y=y-(x-y) = y-x+y = y (Use narrow encoding in Thumb-2)

// Pre-load exponent mask.
L(add_already_ordered):
        li      z0, 0xFFE00000

// Special cases for subnormal or zero addend or augend.  Involving zeroes we can
// only have +0 + +0 or -0 + -0.  In this case, if the largest exponent indicates
// a zero then we know the other operand must also be zero.
        tst     z0, yh, lsl #1          // addend zero?
        beq     L(add_zero)

// If the exponent of x (the largest) is Inf or NaN, return the largest.
// As all NaNs are larger than Inf, Inf+NaN is magically returned as NaN.
        cmp     z0, xh, lsl #1          // drop sign from Inf and NaN
        bls     L(add_inf_nan)          // Sort out addition of Infs and NaNs

// We now have correctly-normalized non-zero finite operands.  Isolate
// excess-1023 exponent and significand signs of augend.
        lsrs    r4, xh, #20

// Find difference between exponents which is the number of places
// needed to align them.  The sign bits are canceled in the subtraction
// because we know they are the same in addend and augend.
        sub     z1, r4, yh, lsr #20

// Remove exponents and set hidden bits.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        movs    z0, #1
        bfi     xh, z0, #20, #12
        bfi     yh, z0, #20, #12
#else
        bics    xh, xh, z0              // Use narrow encoding in Thumb-2
        orr     xh, xh, #0x00100000
        bics    yh, yh, z0
        orr     yh, yh, #0x00100000
#endif

// Compute number of bits required to align significands.  In fact, we do this in two separate
// code lumps, one when there is at most 32 bits and one when there are more than 32 bits.
        rsbs    z0, z1, #32
        bcc     L(add_shifted_word)

// At most 32 bits to shift which implies that both halves of the summands are significant
// after aligning, so add long significands.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        adds    xl, xl, yh, lsl z0      // add aligned low to low sum
        adc     xh, xh, #0
        adds    xl, xl, yl, lsr z1      // add aligned to low sum and include carry over
        adc     xh, xh, yh, lsr z1      // add aligned to high sum
        mov     yl, yl, lsl z0          // align low-order non-added bits to MSB of yl
#else
        lsl     r5, yh, z0              // add aligned low to low sum
        adds    xl, xl, r5
        adc     xh, xh, #0
        lsr     r5, yl, z1              // add aligned to low sum and include carry over
        adds    xl, xl, r5
        lsr     r5, yh, z1              // add aligned to high sum
        adcs    xh, xh, r5
        lsls    yl, yl, z0              // align low-order non-added bits to MSB of yl
#endif

// Did an overflow happen and realignment is required?  Do realignment out of line as
// this is the non-common case.
L(align_check):
        lsls    r5, xh, #11
        bcc     L(round)

// The sum overflowed so shift significand to align it correctly.
        lsrs    xh, xh, #1              // Normalize
        movs    xl, xl, rrx
        movs    yl, yl, rrx             // Remember bits shifted out of sum
        it      cs
        orrcs   yl, yl, #1              // Fold sticky bit
        adds    r4, r4, #1              // Adjust exponent accordingly.  (Use narrow encoding in Thumb-2)

// Perform rounding.
L(round):
        subs    r4, r4, #1              // Rather than remove the hidden bit, just adjust the exponent down by one and add in
        lsrs    z0, xl, #1
        sbcs    yl, yl, #0x80000000
        adcs    xl, xl, #0
        adcs    xh, xh, r4, lsl #20

// Did result overflow?
        mov     yh, #0x00200000
        cmn     yh, xh, lsl #1
        it      cc
        RET2cc  cc, ADD64_WORKING_SET

// Overflowed.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        bfc     xh, #0, #20
#else
        lsrs    xh, xh, #20             // isolate Inf exponent including sign.  (Use narrow encoding in Thumb-2)
        lsls    xh, xh, #20             // Use narrow encoding in Thumb-2
#endif
        movs    xl, #0                  // Inf requires zero significand
        RET2    ADD64_WORKING_SET

// Here we need to add two summands which differ in exponents by more than 32.
// The if the exponents differ by more than 53 bits then all significance is
// lost and the larger must be returned as the sum.
L(add_shifted_word):

// Adjust shift count as we'll add high to low which is an implicit shift by 32.
        subs    z1, z1, #32             // Use narrow encoding in Thumb-2
        cmp     z1, #32
        bcs     L(too_many_shifts)

// Now need to add high part (shifted) of smaller summand to larger summand.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        adds    xl, xl, yh, lsr z1
#else
        lsr     r5, yh, z1
        adds    xl, xl, r5
#endif
        adc     xh, xh, #0

// Compute number of bits to shift to align rounding bits.  We should shift by 32-z1 here,
// but we want to transfer one bit from yl which reduces the shift count.
        rsb     z0, z1, #32

// Compress digits lost.
        tst     yl, yl
        it      ne
        movne   yl, #1
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orr     yl, yl, yh, lsl z0
#else
        lsl     r5, yh, z0
        orrs    yl, yl, r5              // Use narrow encoding in Thumb-2
#endif
        b       L(align_check)

// Zero subtracted, flush subnormal to positive zero if needed (+0 + -0 = +0)
L(sub_zero):
        tst     xh, z0, lsr #1          // If non-zero exponent, cannot be subnormal
#if __SEGGER_RTL_CORE_HAS_CLRM
        it      eq
        clrmeq  {xl, xh}
#else
        itt     eq
        moveq   xl, #0                  // If zero exponent, truncate result to zero.
        moveq   xh, #0
#endif
L(zero_add):
        RET2    ADD64_WORKING_SET

L(add_zero):
        tst     z0, xh, lsl #1          // If augend is zero (ignoring sign), sort which zero to return out of line
        itt     eq
        andeq   xh, xh, #0x80000000
        moveq   xl, #0
        RET2    ADD64_WORKING_SET

// Zero added to zero.
L(add_zeroes):
        and     xh, xh, #0x80000000
        movs    xl, #0                  // Use narrow encoding in Thumb-2
        RET2    ADD64_WORKING_SET

// All significance lost in one summand, return other.
L(too_many_shifts):
        bic     xh, xh, #0x00100000     // Remove hidden bit.
        add     xh, xh, r4, lsl #20     // Splice result exponent into sum.
        RET2    ADD64_WORKING_SET

// If either input for addition was a NaN, return an AEABI-compliant NaN, otherwise
// return a correctly-signed infinity.
L(add_inf_nan):
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        tst     xl, xl                  // Fold low-order significant (NaN bits) to high order.
        it      ne
        orrne   xh, xh, #1
#endif
        cmp     z0, xh, lsl #1          // drop sign from Inf and NaN
        it      eq
        RET2cc  eq, ADD64_WORKING_SET

// Return an AEABI-compliant NaN.
L(nan_result):
        mov     xh, #0x7f000000         
        orr     xh, xh, #0x00f80000
        movs    xl, #0                  // Use narrow encoding in Thumb-2
        RET2    ADD64_WORKING_SET

// Signs differ so must do subtract.
L(subtract):

// Make signs the same.
        eor     yh, yh, #0x80000000

// Get largest of x and y into x, smallest into y.
        subs    r4, xl, yl              // compute z[z0:r4]=x-y
        sbcs    z1, xh, yh
        bcs     L(sub_already_ordered)  // x>y

// Exchange x and y using some basic algebra and configure resulting sign.
        eor     z1, z1, #0x80000000
        subs    xl, xl, r4
        sbcs    xh, xh, z1              // y=y-(x-y) = y-x+y = x  (Use narrow encoding in Thumb-2)
        adds    yl, yl, r4              // x=x-(x-y) = x-x+y = y
        adcs    yh, yh, z1              // Use narrow encoding in Thumb-2

// Pre-load exponent mask.
L(sub_already_ordered):
        li      z0, 0xFFE00000

// Special cases for subnormal or zero addend or augend.
        tst     z0, yh, lsl #1          // If addend is zero (ignoring sign), sort it out of line
        beq     L(sub_zero)

// If the exponent of x (the largest) is Inf or NaN, return the largest.
// As all NaNs are larger than Inf, Inf+NaN is magically returned as NaN.
        cmp     z0, xh, lsl #1          // Drop sign from Inf and NaN
        bls     L(sub_inf_nan)          // Sort out what to do with Inf and NaN operands.

// We now have correctly-normalized non-zero finite operands.  Isolate
// excess-1023 exponent and significand sign of largest in magnitude.
        lsrs    r4, xh, #20

// Find difference between exponents which is the number of places
// needed to align them.  We cancel the sign bits in the subtraction
// because we know they are the same in addend and augend.
        sub     z1, r4, yh, lsr #20

// Remove exponents, align, and insert hidden bits.
#if __SEGGER_RTL_CORE_HAS_LSLL_LSRL_ASRL
        lsll    xl, xh, #11
#else
        lsls    xh, xh, #11
        orr     xh, xh, xl, lsr #21
        lsls    xl, xl, #11
#endif
        orr     xh, xh, #0x80000000
#if __SEGGER_RTL_CORE_HAS_LSLL_LSRL_ASRL
        lsll    yl, yh, #11
#else
        lsls    yh, yh, #11
        orr     yh, yh, yl, lsr #21
        lsls    yl, yl, #11
#endif
        orr     yh, yh, #0x80000000

// Compute number of bits required to align significands.  In fact, we do this in two separate
// code lumps, one when there is at most 32 bits and one when there are more than 32 bits.
        rsbs    z0, z1, #32
        bcc     L(sub_shifted_word)

// At most 32 bits to shift which implies that both halves of the summands are significant
// after aligning, so subtract long significands.
        lsls    r5, yl, z0
        lsr     yl, yl, z1
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orr     yl, yl, yh, lsl z0
#else
        lsl     r5, yh, z0
        orr     yl, yl, r5
#endif
        it      ne
        orrne   yl, yl, #1
        lsr     yh, yh, z1
        subs    xl, xl, yl              // subtract aligned from low sum and include carry over
        sbcs    xh, xh, yh              // subtract aligned from high sum (Use narrow encoding in Thumb-2)

// Now we need to renormalize.
L(subtract_normalization):
        lsrs    yh, r4, #11             // Save result sign in yh
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        ubfx    r4, r4, #0, #11
#else
        lsls    r4, r4, #21             // Extract exponent
        lsrs    r4, r4, #21             // Use narrow encoding in Thumb-2
#endif

// Optimize for 0-bit and 1-bit shifts.
        tst     xh, #0xC0000000
        bmi     L(normalized)
        beq     L(general_normalize)

// Normalize by one bit.
        subs    r4, r4, #1
        lsls    xl, xl, #1
        adcs    xh, xh, xh

// Did subtraction underflow?
L(normalized):
        subs    r4, r4, #1
        bmi     L(underflow)

// Move residual bits for rounding.
        lsls    yl, xl, #21

// Align result significand.
#if __SEGGER_RTL_CORE_HAS_LSLL_LSRL_ASRL
        lsrl    xl, xh, #11
#else
        lsrs    xl, xl, #11
        orr     xl, xl, xh, lsl #21
        lsrs    xh, xh, #11
#endif

// Merge sign.
        orrs    xh, xh, yh, lsl #31

// Merge exponent and round breaking ties. 
        lsrs    z0, xl, #1
        sbcs    yl, yl, #0x80000000
        adcs    xl, xl, #0
        adcs    xh, xh, r4, lsl #20

// Restore saved registers and exit.
        RET2    ADD64_WORKING_SET

L(normalize_again):
        movs    xh, xl                  // Fast shift by 32 bits.
        beq     L(sub_zero_result)
        movs    xl, #0
        subs    r4, r4, #32

// Normalization proper.
L(general_normalize):
        cmp     xh, #0
        beq     L(normalize_again)

#if __SEGGER_RTL_CORE_HAS_CLZ

        clz     z0, xh
        rsbs    z1, z0, #32
        lsls    xh, xh, z0
        lsrs    r5, xl, z1
        lsls    xl, xl, z0
        orrs    xh, xh, r5
        sub     r4, r4, z0

#else

// Normalize with non-zero 32-bit high part, at least 16 bits to shift?
        cmp     xh, #0x00010000
        itttt   cc
        subcc   r4, r4, #16
        movcc   xh, xh, lsl #16
        orrcc   xh, xh, xl, lsr #16
        movcc   xl, xl, lsl #16

// Next normalization, at least 8 bits to shift?
        cmp     xh, #0x01000000
        itttt   cc
        subcc   r4, r4, #8
        lslcc   xh, xh, #8
        orrcc   xh, xh, xl, lsr #24
        lslcc   xl, xl, #8

// Next normalization, at least 4 bits to shift?
        cmp     xh, #0x10000000
        itttt   cc
        subcc   r4, r4, #4
        lslcc   xh, xh, #4
        orrcc   xh, xh, xl, lsr #28
        lslcc   xl, xl, #4

// Tricky part.  We have between zero and three places to shift, so test the top two bits.
// If both bits are zero, we know we have at least two bits to shift.
        tst     xh, #0xc0000000
        itttt   eq
        subeq   r4, r4, #2              // If both bits zero, we can shift two bits,
        lsleq   xh, xh, #2              // and adjust the exponent
        orreq   xh, xh, xl, lsr #30
        lsleq   xl, xl, #2
        tst     xh, xh
L(one_bit_normalize):
        itttt   pl
        subpl   r4, r4, #1              // If top bit is zero, we need to shift once more,
        lslpl   xh, xh, #1              // and adjust the exponent
        orrpl   xh, xh, xl, lsr #31
        lslpl   xl, xl, #1

#endif

        b       L(normalized)

// Underflow.  Flush to signed zero.
L(underflow):
        movs    xl, #0                  // Use narrow encoding in Thumb-2
        lsls    xh, yh, #31             // Use narrow encoding in Thumb-2
        RET2    ADD64_WORKING_SET

L(sub_shifted_word):
// Adjust shift count as we'll add high to low which is an implicit shift by 32.
        sub     z1, z1, #32
        cmp     z1, #32
        bcs     L(sub_too_many_shifts)

// Shift right keeping sticky bit.
        rsb     z0, z1, #32
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orrs    yl, yl, yh, lsl z0
        mov     yh, yh, lsr z1
#else
        lsl     r5, yh, z0
        lsrs    yh, yh, z1
        orrs    yl, yl, r5
#endif
        it      ne
        orrne   yh, yh, #1

// Now need to subtract high part (shifted) of smaller from larger.
        subs    xl, xl, yh
        sbc     xh, xh, #0
        b       L(subtract_normalization)

// Remove hidden bit and align result significand.
L(sub_too_many_shifts):
        bic     xh, xh, #0x80000000
        lsrs    xl, xl, #11             // Use narrow encoding in Thumb-2
        orr     xl, xl, xh, lsl #21
        lsrs    xh, xh, #11             // Use narrow encoding in Thumb-2

// Insert exponent and sign and return.
        add     xh, xh, r4, lsl #20
L(sub_zero_result):
        RET2    ADD64_WORKING_SET

// Restore registers and exit.
L(positive_zero_result):
#if __SEGGER_RTL_CORE_HAS_CLRM
        clrm    {xl, xh}
#else
        movs    xl, #0                  // Use narrow encoding in Thumb-2
        movs    xh, #0                  // Use narrow encoding in Thumb-2
#endif
        RET2    ADD64_WORKING_SET

// If either input for subtraction was a NaN, return an AEABI-compliant NaN, otherwise
// return a correctly-signed infinity.
L(sub_inf_nan):
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        tst     xl, xl                  // Fold low-order significant (NaN bits) to high order.
        it      ne
        orrne   xh, xh, #1
#endif
        cmp     z0, xh, lsl #1          // drop sign from Inf and NaN
        bne     L(nan_result)
        cmp     z0, yh, lsl #1          // finite?
        beq     L(nan_result)           // No, Inf + -Inf is NaN
        RET2    ADD64_WORKING_SET

#endif

END_FUNC __aeabi_dadd

/*********************************************************************
*
*       __aeabi_fsub()
*
*  Function description
*    Subtract, single floating.
*
*  Prototype
*    float __aeabi_fsub(float x, float, y);
*
*  Parameters
*    r0 - x - Minuend.
*    r1 - y - Subtrahend.
*
*  Return value
*    r0 - Difference, x-y.
*/

ARM_GLOBAL_FUNC __aeabi_fsub

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vsub.f32 s0, s0, s1
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Generate 0x80000000.
        movs    r2, #1
        lsls    r2, r2, #31

// Set y = -y and add.
        eors    r1, r1, r2
        la      r3, .L__aeabi_fadd_fast_entry
        bx      r3

#else

// Set y = -y.
        eor     r1, #0x80000000
        b       __aeabi_fadd

#endif

END_FUNC __aeabi_fsub

/*********************************************************************
*
*       __aeabi_dsub()
*
*  Function description
*    Subtract, double floating.
*
*  Prototype
*    double __aeabi_dsub(double x, double y);
*
*  Parameters
*    r1:r0 - x - Minuend.
*    r3:r2 - y - Subtrahend.
*
*  Return value
*    r1:r0 - Difference, hi:lo, x-y.
*/

ARM_GLOBAL_FUNC __aeabi_dsub

#if __SEGGER_RTL_FP_HW >= 2
        
        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vsub.f64 d0, d0, d1
        vmov    xl, xh, d0
        bx lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

        push    {ADD64_WORKING_SET}

// Generate 0x80000000.
        movs    r5, #1
        lsls    r5, r5, #31

// Set y = -y.
        eors    yh, yh, r5
        la      r4, .L__aeabi_dadd_fast_entry
        bx      r4

#else

// Set y = -y.
        eor     yh, yh, #0x80000000
        b       __aeabi_dadd

#endif

END_FUNC __aeabi_dsub

/*********************************************************************
*
*       __aeabi_frsub()
*
*  Function description
*    Reverse subtract, single floating.
*
*  Prototype
*    float __aeabi_frsub(float x, float, y);
*
*  Parameters
*    r0 - x - Minuend.
*    r1 - y - Subtrahend.
*
*  Return value
*    r0 - Difference, y-x.
*/

ARM_GLOBAL_FUNC __aeabi_frsub

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vsub.f32 s0, s1, s0
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Generate 0x80000000.
        movs    r2, #1
        lsls    r2, r2, #31

// Set x = -x.
        eors    r0, r0, r2
        la      r3, .L__aeabi_fadd_fast_entry
        bx      r3

#else

// Set x = -x.
        eor     r0, r0, #0x80000000     // y-x == -x + y
        b       __aeabi_fadd

#endif

END_FUNC __aeabi_frsub

/*********************************************************************
*
*       __aeabi_drsub()
*
*  Function description
*    Reverse subtract, double floating.
*
*  Prototype
*    float __aeabi_drsub(float x, float, y);
*
*  Parameters
*    r1:r0 - x - Minuend.
*    r3:r2 - y - Subtrahend.
*
*  Return value
*    r1:r0 - Difference, hi:lo, y-x.
*/

ARM_GLOBAL_FUNC __aeabi_drsub

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vsub.f64 d0, d1, d0
        vmov    xl, xh, d0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

        push    {ADD64_WORKING_SET}

// Generate 0x80000000.
        movs    r5, #1
        lsls    r5, r5, #31

// Set x = -x.
        eors    xh, xh, r5
        la      r4, .L__aeabi_dadd_fast_entry
        bx      r4

#else

// Set x = -x.
        eor     xh, xh, #0x80000000
        b       __aeabi_dadd

#endif

END_FUNC __aeabi_drsub

/*********************************************************************
*
*       __aeabi_fmul()
*
*  Function description
*    Multiply, float.
*
*  Prototype
*    float __aeabi_fmul(float x, float, y);
*
*  Parameters
*    r0 - x - Multiplicand.
*    r1 - y - Multiplier.
*
*  Return value
*    r0 - Product, x*y.
*/

ARM_GLOBAL_FUNC __aeabi_fmul

#undef L
#define L(label) .L__aeabi_fmul_##label

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vmul.f32 s0, s0, s1
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define MUL_WORKING_SET   r4, r5

// Save working set.
        push    {MUL_WORKING_SET}

// Standard opening: compute sign of product into r5.
        movs    r5, r0
        eors    r5, r5, r1
        movs    r4, #1                  // Form 0x80000000
        lsls    r4, r4, #31
        ands    r5, r5, r4              // Isolate sign

// Continue with opening book: discard sign of operands, move exponents
// to high byte of operand registers.
        lsls    r2, r0, #1
        lsls    r3, r1, #1

// Exponent of lhs to r2 low order byte.
        lsrs    r2, r2, #24
        beq     L(lhs_zero_or_subnormal)

// Exponent of rhs to r3 low order byte.
        lsrs    r3, r3, #24
        beq     L(rhs_zero_or_subnormal)

// Exponent of lhs indicates Inf or NaN?
        cmp     r2, #0xFF
        beq     L(lhs_inf_or_nan)

// Exponent of rhs indicates Inf or NaN?
        cmp     r3, #0xFF
        beq     L(rhs_inf_or_nan)

// Compute product's exponent in r2, but with double-bias.
        adds    r2, r2, r3

// Thumb cores only have a 32x32->32 multiplier which means we can't
// take advantage of a 32x32->64 product.  In this case we need to
// break down the signficand multiply into pieces where the partial
// products are no wider than 32 bits, which means the multiplier
// and multiplicand can be no wider than 16 bits each, or 8 and
// 24 bits.

// We elect to break this into three 8x24 multiplies and sum the
// partial products.

// Get rid of exponents and insert assumed bit for multiplier and multiplicand.
        lsls    r0, r0, #8              // left-align significand most significant bit, dropping exponent
        orrs    r0, r0, r4              // insert assumed bit
        lsrs    r0, r0, #8              // right-align significand
        lsls    r1, r1, #8              // ...ditto
        orrs    r1, r1, r4
        lsrs    r1, r1, #8

// Compute partial product, low 8 bits of lhs * rhs
        UXTBs   r4, r0
        muls    r4, r4, r1              // form first partial product in r4

// Now compute second partial product, middle 8 bits of lhs * rhs
        lsrs    r0, r0, #8              // shift significand down loosing low 8 bits, exposing upper 16 bits
        UXTBs   r3, r0                  // extract middle 8 bits of original lhs to r3
        muls    r3, r3, r1              // form second partial product in r3

// Now compute third and final product, high 8 bits of lhs * rhs
        lsrs    r0, r0, #8              // shift significand down loosing low 8 bits, exposing upper 8 bits
        muls    r0, r0, r1              // form third partial product in r0

// Now we need to sum the partial products.  All the partial products cover the full 32 bits
// bits of the partial product registers r4, r3, and r0, but each to be aligned correctly:
//
//   r0{31:24}  r0{23:16}  r1{15:8}   |  r1{7:0}   
//              r3{31:24}  r3{23:16}  |  r3{15:8}   r3{7:0}
//                         r4{31:24}  |  r4{23:16}  r4{15:8}  r4{7:0}
//
// We so IEEE rounding breaking ties using the sum r3[7:0]<<8 + r4[15:0], as indicated
// by the vertical line above.

        lsrs    r1, r4, #8              // Align for addition, discard r4[7:0]
        adds    r3, r3, r1              // sum partial product r3[31:0] + r4[31:8]
        lsrs    r1, r3, #8              // r1 is now the partial product summation register

// Compute final summation of the partial products in r1
        adds    r1, r1, r0

// If already normalized, no normalization step required.
// At most one normalization step is required (e.g. 1 * 1 = 1, whereas
// 1.5 * 1.5 = 2.25).
        bmi     L(normalized)
        lsls    r1, r1, #1              // Normalize significand
        subs    r2, r2, #1              // Adjust exponent

// Significand is normalized, now pack and check for overflow.
L(normalized):
        lsrs    r0, r1, #8              // Move significand into position

// Remove the double-bias.  If close to underflow, decide what to do with rounding.
        subs    r2, #127
        blt     L(zero_or_underflow)

// If overflowed, return a signed infinity.
        cmp     r2, #254
        bcs     L(inf)

// Move exponent into position.
        lsls    r2, r2, #23

// If the low-order bits that we have not folded into the sum are non-zero,
// then fold them into the sum now.
        orrs    r4, r4, r3
        lsls    r4, r4, #24
        beq     L(no_fold)              // Zero indicates low-order bits are insignificant
        movs    r4, #1                  // Fold sticky bit into sum
        orrs    r1, r1, r4

// Extract guard bits from r1.
L(no_fold):
        UXTBs   r1, r1

// Round up if required and insert exponent.
        cmp     r1, #0x80
        adcs    r0, r0, r2

// Break tie by rounding to nearest even.
        cmp     r1, #0x80
        bne     L(apply_sign)           // if no tie, just apply sign
        lsrs    r0, r0, #1              // clear lsb of r0
        lsls    r0, r0, #1

// Insert sign into product and return.
L(apply_sign):
        orrs    r0, r0, r5              // product sign is in r5

// Restore and return.
L(done):
        pop     {MUL_WORKING_SET}
        bx      lr

// Generate a positive infinity then apply the product sign.
L(inf):
        movs    r0, #0xFF
        lsls    r0, r0, #23             // 0x7f800000
        b       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// Extract exponent of rhs to low bits of r3.
        lsrs    r3, r3, #24

// If is either Inf or NaN, then it's a NaN
        cmp     r3, #0xFF
        beq     L(nan)

// It's 0*normal, which is a signed zero.
L(signed_zero):
        movs    r0, r5
        b       L(done)

// We know that the LHS is Inf or NaN.
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.
        lsls    r0, r0, #9
        bne     L(nan)

// Know lhs is Inf.  If the rhs is neither Inf nor NaN, then Inf * non-zero-normal is Inf.
        cmp     r3, #0xFF
        bne     L(inf)

// If rhs is NaN then the result is NaN.
        lsls    r1, r1, #9
        bne     L(nan)

// Inf * Inf is Inf.
        b       L(inf)

// We know that the rhs is zero or subnormal and the lhs is non-zero.
// Treat subnormals as zero anyway.  0 * normal is a signed zero.
L(rhs_zero_or_subnormal):
        cmp     r2, #0xFF
        bne     L(signed_zero)

// non-zero*Inf and non-zero*NaN are NaNs.

// Result is AEABI NaN.
L(nan):
        movs    r0, #0x7F
        lsls    r0, r0, #4
        adds    r0, r0, #0xC
        lsls    r0, r0, #20
        b       L(done)

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.
        lsls    r1, r1, #9
        bne     L(nan)

// We also know that the lhs is normal (not subnormal, not Inf, not NaN), so
// the product is Inf * normal which is Inf.
        b       L(inf)

// The product has resulted in an exponent of zero or less.  This may not actually
// mean that the product should be treated as a zero as there is still rounding
// to do.
L(zero_or_underflow):

// Shift significand into position and save computed product in r3 as we may need it below.
        lsls    r3, r0, #9

// If the biased exponent is less than zero, then this would generate a subnormal
// and we don't support subnormals, so return a signed zero.
        adds    r2, r2, #1
        bne     L(signed_zero)

// Exponent is exactly zero and on the verge of underflow except for rounding.
// So, now round significand--we only need to differentiate between a zero result
// and a result that rounds to the smallest normal value.
        asrs    r3, r3, #9
        adds    r3, r3, #2
        bls     L(signed_zero)          // Rounding didn't cause carry and therefore needs
                                        // a signed zero result

// Rounding rounded to smallest normalized value with a non-zero exponent.
        lsls    r0, r3, #23             // 0x00800000
        b       L(signed_zero)

#else

// Faster floating multiply...


// Load exponent mask.
        mov     ip, #0xff               // There is no narrow encoding as we're using IP

// Extract exponent of multiplier and multiplicand, set Z if either zero.
        ands    r2, ip, r0, lsr #23     // Set Z if multiplicand exponent zero
        ittt    ne
        andsne  r3, ip, r1, lsr #23     // Extract multiplier exponent and set Z if multiplier exponent zero

// Check if either exponent is 0xff which signals Inf or NaN.
        cmpne   r2, #0xFF               // Only considering Z flag match here
        cmpne   r3, #0xFF               // And here

// Special-case Inf, NaN, zero, and subnormal operands.
        beq     L(inf_nan_zero)

// Calculate product's exponent by addition-of-powers law.  r3 is now free.
        add     r2, r2, r3

// Generate sign of product into N flag; MI => negative result.
// We could generate the sign here, but on piplined processors we can do useful work
// after the umull if we don't read the result registers, so let's try to bury the
// computation of the sign bit and the product's exponent in that window so the
// processor isn't stalled.
// Compute product's sign bit.  (*)
        mov     ip, #0x80000000
        eors    r3, r0, r1
        ands    r3, r3, ip

// Normalize significand of multiplier and multiplicand and materialize hidden bits.
        orr     r1, ip, r1, lsl #8
        orr     r0, ip, r0, lsl #8

// Perform 32-bit by 32-bit multiply to get product's significand in r1:ip.
// At most one normalization step is required (e.g. 1 * 1 = 1, whereas
// 1.5 * 1.5 = 2.25).
        umull   ip, r1, r0, r1

// Test to see if normalization is required.  In this case, normalization
// is required if the high part of the result is less than 0x80000000 and
// after the compare C will be zero.
        cmp     r1, #0x80000000
        sbc     r2, r2, #0x7f
        it      cc
        lslcc   r1, r1, #1              // If normalization required, normalize

// Fold in high-order rounding bits into low-order significand.  The lower 16 bits
// of ip are zero as the two significands have their eight least significant bits
// zero.  We use ip as a "sticky" register.
        orr     ip, ip, r1, lsl #25

// Did the exponent overflow or underflow?  We use a non-signed compare here
// as negative exponents will always be larger than 0xfe.  If so, handle this
// non-common case out of line.
        cmp     r2, #0xfe
        bcs     L(overflow_or_underflow)

// Perform IEEE round to nearest.
        orrs    r0, r3, r1, lsr #8      // Carry is set to lsb of product
        orr     ip, ip, r0, lsl #31     // ip is now zero iff all low product bits are zero
        it      cs
        subscs  r3, ip, #1              // Compute rounding bit

// Insert exponent and round.
        adc     r0, r0, r2, lsl #23

// All done.
        bx      lr

// Here we know that the computed exponent, in r2, is either too big or too
// small, so we divide the cases simply using the sign of the exponent.
L(overflow_or_underflow):

// Underflowed so exponent is negative?  This is more likely than overflow
// when used with the math libraries, so this case executes faster.
        tst     r2, r2
        itt     mi
        movmi   r0, r3
        bxmi    lr

// Overflowed, generate a correctly-signed infinity.
        movs    r0, #0xff
        orr     r0, r3, r0, lsl #23     // Apply sign of result to Inf
        bx      lr

// ...continue with code common to both above...

// Handle the special case where one of the operands is Inf, NaN, or zero.
// The non-zero finite x non-zero finite case is the general case above so
// there is no need to decode it.
//
//   x |   0  nzf   Inf   NaN
// ----+---------------------
//   0 |   0    0   NaN   NaN
// nzf |   0    -   Inf   NaN
// Inf | NaN  Inf   Inf   NaN
// NaN | NaN  NaN   NaN   NaN
//
L(inf_nan_zero):

// Flush subnormal inputs to a correctly-signed zero.
        tst     r0, ip, lsl #23
        it      eq
        andeq   r0, #0x80000000         // subnormal or zero, flush to signed zero
        tst     r1, ip, lsl #23
        it      eq
        andeq   r1, #0x80000000         // subnormal, flush to signed zero

// Get exponent of multiplicand, we may not have computed it beforehand.
        and     r3, ip, r1, lsr #23

// Test for either multiplier and multiplicand Inf or NaN.
        cmp     r2, #0xff
        it      ne
        cmpne   r3, #0xff
        beq     L(inf_nan)

// Multiplicand or multiplier are zero, so the result is a signed zero.
        eors    r0, r0, r1              // Use narrow Thumb-2 encoding
        and     r0, #0x80000000
        bx      lr

// Here we know that either the multiplier or multiplicand is Inf or NaN.
L(inf_nan):

// Is the multiplicand or multiplier either +0 or -0?
        cmp     r0, #0x0                // Z=1 if +0
        ittt    ne
        cmpne   r0, #0x80000000         // Z=1 if +0 or -0, Z=0 if finite, Inf or NaN
        cmpne   r1, #0x0
        cmpne   r1, #0x80000000

// If either the multiplier or multiplicand is zero, we have 0*Inf, 0*NaN, NaN*0, or Inf*0
// all of which must generate a NaN result.
        beq     L(nan_result)

// If multiplicand is not Inf or NaN, consider multiplier
        cmp     r2, #0xff
        bne     L(test_multiplier)

// Know multiplicand is Inf or NaN, distinguish between these cases,
// for which we use the significand.  If NaN, generate a NaN result.
        lsls    r2, r0, #9              // Z=1 if Inf, 0 if NaN
        bne     L(nan_result)           // if Z=0 we have NaN * x = NaN

// Test the multiplier; we know the multiplicand is a non-zero finite or Inf.
L(test_multiplier):
        cmp     r3, #0xff               // Multiplicand is finite?

// If it is, we know the multiplicand *must* be Inf because we've already
// filtered out zero and finite multiplicands, so it's an Inf product.
        bne     L(inf_result)

// Know multiplicer is Inf or NaN, distinguish between these cases,
// for which we use the significand.  If NaN, generate a NaN result.
        lsls    r2, r1, #9
        bne     L(nan_result)           // x * NaN = NaN

// Generate a signed Inf result.
L(inf_result):
#if (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32) && __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        eors    r0, r0, r1              // Compute sign of result
        movs    r1, #0x7f800000         // Load +Inf
        bfi     r0, r1, #0, #31         // Overwrite everything but sign bit
#else
        eors    r0, r0, r1              // Compute sign of result
        lsls    r0, r0, #1
        mov     r0, #0xff000000
        rrxs    r0, r0
#endif
        bx      lr


L(nan_result):
        li      r0, 0x7fc00000
        bx      lr

#endif

END_FUNC __aeabi_fmul

/*********************************************************************
*
*       __aeabi_dmul()
*
*  Function description
*    Multiply, double floating.
*
*  Prototype
*    double __aeabi_dmul(double x, double y);
*
*  Parameters
*    r1:r0 - x - Multiplicand.
*    r3:r2 - y - Multiplier.
*
*  Return value
*    r1:r0 - Product, hi:lo, x*y.
*/

#undef L
#define L(label) .L__aeabi_dmul_##label

#if __SEGGER_RTL_FP_HW >= 2

ARM_GLOBAL_FUNC __aeabi_dmul

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vmul.f64 d0, d0, d1
        vmov    xl, xh, d0
        bx lr

END_FUNC __aeabi_dmul

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

ARM_LOCAL_FUNC __aeabi_dmul_aux

// lhs has an exponent that indicates NaN or Inf.  If NaN, Nan*anything is NaN.
L(lhs_nan_or_inf):
        bne     L(nan)                  // Not exactly 0xffe00000, i.e. must be a NaN as significand is non-zero
        CBNZx   xl, L(nan)              // other part of Inf/NaN indicates NaN?

// Lhs is now known to be Inf.  If rhs is NaN, the result is NaN.
        cmn     r5, r6
        bhi     L(nan)                  // Known to be NaN
        bcc     L(rhs_could_be_zero)    // Normal or zero

// Could have NaN if lower part of significand is non-zero.
        CBNZx   yl, L(nan)              // low part of significand

// lhs is Inf, rhs is normal or zero.  Inf*normal is Inf, Inf*0 is NaN.
L(rhs_could_be_zero):

// Extract significand digits, Inf*normal is float64_mul_inf
        lsrs    r5, r5, #21
        bne     L(inf)

// ...left with Inf*0 which is NaN.
L(nan):
        movs    xl, #0
        mvns    xh, xl                  // 0xffffffff
        lsrs    xh, xh, #20             // 0x00000fff
        lsls    xh, xh, #19             // 0x7ff80000:0x00000000

// All done, restore registers and return.
L(done):
        RET4    r4, r5, r6, r7

// lhs has an exponent that indicates NaN or Inf.  If NaN, Nan*anything is NaN.
L(rhs_nan_or_inf):
        bne     L(nan)
        tst     yl, yl                  // low part of significand
        bne     L(nan)

// Know rhs is now Inf.  0*Inf is NaN, normal*Inf is Inf.
        lsrs    r4, r4, #21             // Exponent of lhs indicates zero (or subnormal)?
        beq     L(nan)                  // yes, it's 0*Inf which is NaN.

// Generate a signed infinity.
L(inf):
        movs    xl, #0
        mvns    xh, xl                  // 0xffffffff
        lsls    xh, xh, #21             // 0xffe00000
        lsrs    xh, xh, #1              // 0x7ff00000
        b       L(apply_sign)           // apply sign to Inf.

// Generate a signed zero.
L(signed_zero):
        movs    xl, #0                  // low part of significand
        mov     xh, ip                  // sign only in r1
        RET4    r4, r5, r6, r7

ALIAS_LABEL __aeabi_dmul

// Save working set.  Can't save r8 in this push, so can't free up an extra
// high register which would be nice to dump things into.
        push    {r4-r7, lr}

// Generate 0x80000000.
        movs    r7, #1
        lsls    r7, r7, #31

// Compute sign of product into ip[31], zeros in ip[31:0].
        movs    r6, xh
        eors    r6, r6, yh
        ands    r6, r6, r7
        mov     ip, r6

// Generate 0x00200000.
        lsrs    r6, r7, #10

// Remove sign from muliplier and multiplicand significand and align to msb.
        adds    r4, xh, xh
        adds    r5, yh, yh

// Is lhs NaN or Inf?  If so, handle special case out of line.
        cmn     r4, r6
        bcs     L(lhs_nan_or_inf)

// Is rhs NaN or Inf?  If so, handle special case out of line.
        cmn     r5, r6
        bcs     L(rhs_nan_or_inf)

// Extract multiplicand exponent.  If zero, we have 0*normal or 0*0, which is zero.
        lsrs    r4, r4, #21
        beq     L(signed_zero)

// Extract multiplier exponent.  If zero, we have non-zero-normal*0 which is zero.
        lsrs    r5, r5, #21
        beq     L(signed_zero)

// Compute product's exponent into r4, frees r5.
        adds    r4, r4, r5

// Shift off exponent and materialize hidden bit in muliplicand significand.
// Align significand to msb.
        lsls    xh, xh, #11
        orrs    xh, xh, r7              // Insert hidden bit
        lsrs    r6, xl, #21             // Shift xh:xl left 11 bits.  See (*) below...  r6 only free register at present
        lsls    xl, xl, #11
        orrs    xh, xh, r6

// Shift off exponent and materialize hidden bit in multiplier significand.
        lsls    yh, yh, #11
        orrs    yh, yh, r7
        lsrs    yh, yh, #11

// We now have the daunting task of a 53x53-bit multiply using only a 32x32->32
// instruction--which means we need to break this up into sixteen 16x16-bit
// multiplies with summation of the partial products.

// Current register use is:
//   xl - lhs significand low part
//   xh - lhs significand high part with hidden bit set
//   yl - rhs significand low part
//   yh - rhs significand with hidden bit set

// This is the multiplication we're going to do, written longhand as you would
// usually compute at school.

//                                        |  R3H  :  R3L  |  R2H  :  R2L  |
//                                  x     |  R1H  :  R1L  |  R0H  :  R0L  |
//                                      -------------------------------------
//                                                        |  R2L  x  R0L  |
//                                                |  R2H  x  R0L  |
//                                        |  R3L  x  R0L  |
//                                |  R3H  x  R0L  |
//                                                |  R2L  x  R0H  |
//                                        |  R2H  x  R0H  |
//                                |  R3L  x  R0H  |
//                        |  R3H  x  R0H  |
//                                        |  R2L  x  R1L  |
//                                |  R2H  x  R1L  |
//                        |  R3L  x  R1L  |
//                |  R3H  x  R1L  |
//                                |  R2L  x  R1H  |
//                        |  R2H  x  R1H  |
//                |  R3L  x  R1H  |
//        |  R3H  x  R1H  |

//  We can rearrange this so:

//           *7      *6      *5      *4      *3      *2      *1      *0
//                                                        |  R2L  x  R0L  |  #1
//                                                |  R2H  x  R0L  |          #2
//                                                |  R2L  x  R0H  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                        |  R2L  x  R1L  |                  #6
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                |  R2H  x  R1L  |                          #9
//                                |  R2L  x  R1H  |                          #10
//                        |  R3H  x  R0H  |                                  #11
//                        |  R3L  x  R1L  |                                  #12
//                        |  R2H  x  R1H  |                                  #13
//                |  R3H  x  R1L  |                                          #14
//                |  R3L  x  R1H  |                                          #15
//        |  R3H  x  R1H  |                                                  #16

// Now, this would be just dandy if both significands were aligned to the lsb of
// their respective register pair, but then I'd have 22 zero high order bits in
// the product and I'd have to shift everything left 11 bits to align the product
// significand for packing.  (*)  If I shift the multiplier or multiplicand left by
// 11 bits first, then my product is correctly aligned when I finish *and*
// crucially the bits required for rounding are in just the right position to
// use the ARM instruction set to easily round (i.e. they're aligned in the
// lsb of the high product register and the msb of the low product register).
//
// For the 106-bit product, we require 53 bits computed plus a rounding and
// sticky bit.  The rounding bit says whether to round up or down and the sticky
// bit breaks ties and is zero when the low 51 bits are zero and one when they
// are non-zero.  This allows us to compute the product but we do not need to
// keep all 106 bits of the product around, as we can compress the low order
// product bits "as we go".

// To do this we maintain a "running product" and "sticky bit bin".  The running
// product is computed and as bits are shifted out they are or-ed into the
// "sticky bit bin" which accumulates any 'one' bits.

// Save original xh (we'll clobber xh during calculation), and save prodoct
// exponent register freeing all registers for use to hold partial products.
        push    {r4}
        mov     lr, xh

// #1: r7 = R2L x R0L
//                                                        |  R2L  x  R0L  |  #1  -  in r7
        UXTHs   r5, xl                  // r5 = R0L
        UXTHs   r7, yl                  // r7 = R2L
        muls    r7, r7, r5

// #2: R4 = R2H x R0L
//                                                |  R2H  x  R0L  |          #2  -  in r4
        lsrs    r4, yl, #16             // r4 = R2H
        muls    r4, r4, r5              // r4 = R2H x R0L

// Generate product summand from partial products #2 + high(#1) giving running product in R4:r7L.
// This can never generate carry as 0xffff x 0xffff = 0xfffe0001, adding 0xffff gives 0xffff0000.
//                                                        |  R2L  x  R0L  |  #1
//                                           +    |  R2H  x  R0L  |          #2
//                                                |------r4-------|
//                                                        |xxxxxxx---r7---|  - high 16 bits of r7 are garbage
        lsrs    r6, r7, #16
        adds    r4, r4, r6

// Rearrange product.
//                                                        |  R2L  x  R0L  |  #1
//                                           +    |  R2H  x  R0L  |          #2
//                                        |0000000---r6---|------r7-------|  - high 16 bits of r6 are zero
        lsrs    r6, r4, #16
        lsls    r4, r4, #16
        UXTHs   r7, r7                  // remove garbage
        orrs    r7, r7, r4

// Now start #4, skipping #3 for the moment as we have R0L in r5.
//                                                        |  R2L  x  R0L  |  #1
//                                                |  R2H  x  R0L  |          #2
//                                    +   |  R3L  x  R0L  |                  #4
//                                        |------r4-------|------r7-------|
        UXTHs   r4, yh                  // r4 is now R3L
        muls    r4, r4, r5              // r4 = R3L x R0L
        adds    r4, r4, r6              // r6 contains high of partial product R2H x R0L

// #7: R1 = R3H x R0L
//                                |  R3H  x  R0L  |                          #7  -  in xh
        lsrs    xh, yh, #16
        muls    xh, xh, r5

//                                                        |  R2L  x  R0L  |  #1
//                                                |  R2H  x  R0L  |          #2
//                                        |  R3L  x  R0L  |                  #4
//                            +   |  R3H  x  R0L  |                          #7  -  in xh
//                                |------xh-------|--r4---|------r7-------|      - high 16 bits or r4 are zero
        lsrs    r5, r4, #16
        UXTHs   r4, r4
        adds    xh, xh, r5        // Addition can never overflow, as stated previously

//                                                        |  R2L  x  R0L  |  #1
//                                                |  R2H  x  R0L  |          #2
//                                        |  R3L  x  R0L  |                  #4
//                            +   |  R3H  x  R0L  |                          #7
//                        |0000000---xh---|------r4-------|------r7-------|  - high 16 bits of xh are zero
        lsls    r5, xh, #16
        lsrs    xh, xh, #16
        adds    r4, r4, r5

// #3: r5 = R0H x R2L
//                                                |  R2L  x  R0H  |          #3
        lsrs    xl, xl, #16
        UXTHs   r5, yl
        muls    r5, r5, xl

// We would now like to calculate the sum:
//
//                                                        |  R2L  x  R0L  |  #1
//                                                |  R2H  x  R0L  |          #2
//                                        |  R3L  x  R0L  |                  #4
//                                |  R3H  x  R0L  |                          #7
//                             +                  |  R2L  x  R0H  |          #3  -  in r5
//                        |0000000---xh---|------r4-------|------r7-------|  - high 16 bits of xh are zero
//
// We can do this by adding the low part of r5 to the high part of r7 and catching the low bits of #1
// shifted out at this time to be caught in the sticky bit bin making r7 the sticky bit bin.
        lsrs    r6, r7, #16
        adds    r6, r6, r5              // Sum high(#1+#2) + #3, can't overflow, $ffff*$ffff+$ffff+$ffff = $ffffffff
        orrs    r7, r7, r6              // sitcky bit bin
        UXTHs   r7, r7                  // Now r7 will be non-zero IFF the sum of the partial products above in r7 (modulo 2^32) is non-zero
        lsrs    r6, r6, #16

// Now have two running (partial) sums, and note that the partial product from #2 is
// broken across both of them:
//
//                                                        |  R2L  x  R0L  |  #1
//                                                |  ...  x  R0L  |          #2
//                                                |  R2L  x  R0H  |          #3  -  in r5
//                                                
//                                        |0000000---r6---|0000000---bin--|  - high 16 bits of r6 are zero, r7 low 16 bits are sticky bit bin
//
// and:
//                                                |  R2H  x  ...  |          #2
//                                        |  R3L  x  R0L  |                  #4
//                                |  R3H  x  R0L  |                          #7
//                        |0000000---xh---|------r4-------|                  - high 16 bits of xh are zero
//
// Also note that r6 and r4 overlap.

// #5: r5 = R2H x R0H
//                                        |  R2H  x  R0H  |                  #5
        lsrs    r5, yl, #16
        muls    r5, r5, xl

// Now accumulate #5 into one of the running sums:
//                                                        |  R2L  x  R0L  |  #1
//                                                |  ...  x  R0L  |          #2
//                                                |  R2L  x  R0H  |          #3
//                                        |  R2H  x  R0H  |                  #5
//                                        |------r5-------|0000000---bin---|  - high 16 bits of r6 are zero, r7 low 16 bits are sticky bit bin
        adds    r5, r5, r6

// Rearrange the running sums:
//

//                                        |  R3L  x  ...  |                  #4
//                                |  R3H  x  R0L  |                          #7
//                        |0000000---xh---|
//                                |0000000---r4---|
//
// and:
//                                                        |  R2L  x  R0L  |  #1
//                                                |  ...  x  R0L  |          #2
//                                                |  R2L  x  R0H  |          #3
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  ...  x  R0L  |                          #7
//                                        |------r5-------|0000000---bin---|  - high 16 bits of r6 are zero, r7 low 16 bits are sticky bit bin

// We'll now combine them into a single diagram, note #6 has not been accumulated yet.
//                                                        |  R2L  x  R0L  |  #1
//                                                |  R2H  x  R0L  |          #2
//                                                |  R2L  x  R0H  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                        |0000000---xh---|
//                                |0000000---r4---|
//                                        |------r5-------|0000000---bin--|  - high 16 bits of r6 are zero, r7 low 16 bits are sticky bit bin
        UXTHs   r6, r4
        lsrs    r4, r4, #16
        adds    r5, r5, r6

// Now use high 16 bits of r7 as a place to store the low part of r5
//                                                |  R2H  x  ...  |          #2
//                                                |  R2L  x  ...  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                        |0000000---xh---|
//                                |0000000---r4---|
//                                |0000000---r6---|       |0000000---bin--|  - high 16 bits of r6 are zero, r7 low 16 bits are sticky bit bin
//                                                |--r7---         --bin--|                                 r7 high 16 bits contribute to sum
        lsls    r6, r5, #16
        orrs    r7, r7, r6
        lsrs    r6, r5, #16

// New partial sum:
//                                                |  R2H  x  ...  |          #2
//                                                |  R2L  x  ...  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                        |0000000---xh---|
//                                |000000x---r6---|       |0000000---bin--|  - high 16 bits of r6 are zero, r7 low 16 bits are sticky bit bin
//                                                |--r7---         --bin--|                                 r7 high 16 bits contribute to sum
        adds    r6, r6, r4              // r6 ranges between 0x0000 and 0x1fffe after this addition...

// #8: r5 = R3L * R0H
//                                |  R3L  x  R0H  |                          #8
        UXTHs   r5, yh
        muls    r5, r5, xl

//                                                |  R2H  x  ...  |          #2
//                                                |  R2L  x  ...  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                        |0000000---xh---|       |--r7---         --bin--|  - r7 high 16 bits contribute to sum
//                                |  R3L  x  R0H  |                          #8
//                                |------r5-------|
//                                                
        adds    r5, r5, r6

// Rearrange product again.

//                                                |  R2H  x  ...  |          #2
//                                                |  R2L  x  ...  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                        |000000x---xh---|       |--r7---         --bin--|  - r7 high 16 bits contribute to sum                              -- xh can be between 0x0000 and 0x1fffe
//                                |  R3L  x  R0H  |                          #8
//                                |0000000---r4---|         
//                                                
        UXTHs   r4, r5
        lsrs    r6, r5, #16
        adds    xh, xh, r6              // xh ranges between 0x0000 and 0x1fffe after this addition...

// #11: r5 = R3H x R0H
//                        |  R3H  x  R0H  |                                  #11
        lsrs    r5, yh, #16
        muls    r5, r5, xl

// New partial sum:
//                                                |  R2H  x  ...  |          #2
//                                                |  R2L  x  ...  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                        |------r5-------|       |--r7---         --bin--|  - r7 high 16 bits contribute to sum
//                                |  R3L  x  R0H  |                          #8
//                                |0000000---r4---|         
//                                                
        adds    r5, r5, xh              // can't overflow, 0xfffe0001 + 0x1fffe = 0xffffffff

// Combine running products:
//                                                |  R2H  x  ...  |          #2
//                                                |  R2L  x  ...  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                |0000000---xh---|------r4-------|--r7---         --bin--|  - r7 high 16 bits contribute to sum
        lsls    r6, r5, #16
        lsrs    xh, r5, #16
        orrs    r4, r4, r6

// Retrieve old xh.
        mov     xl, lr

// #6: r5 = R1L x R2L
//                                        |  R2L  x  R1L  |                  #6
        UXTHs   xl, xl
        UXTHs   r5, yl
        muls    r5, r5, xl

// We can now compute the partial product sum down column *2.  We only need to know whether
// these bits are zero or not and so contribute to the sticky bit bin as they're used only
// for rounding, so currently have this:
//
//                                                |  R2H  x  ...  |          #2
//                                                |  R2L  x  ...  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                |0000000---xh---|------r4-------|--r7---         --bin--|  - r7 high 16 bits contribute to sum
//                                        |  R2L  x  R1L  |                  #6
//                                        |------r5-------|
//
// We sum the low parts of r7 and r5 and or that into the sticky bit bin (in r7).
        lsrs    r6, r7, #16             // extract high 16 bits of r7 which are part of the product
        adds    r6, r6, r5              // sum all of column *2 into r6, but we're only interested in the low 16 bits
        orrs    r7, r7, r6              // combine into low 16 bits of r7, the high 16 bits are considered garbage.
        UXTHs   r7, r7
        lsrs    r6, r6, #16

// Simplifying by discarding products rolled into the sticky bit bin, we now have this:
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                |0000000---xh---|------r4-------|0000000---bin--|          - r7 low 16 bits are sticky bit bin, high 16 are zero
//                                        |  R2L  x  ...  |                  #6
//                                |0000000---r6---|

// Therefore, we have processed all partial products from #1 to #8, and have #9 to #16 to do.

// #9: r5 = R2H x R1L
//                                |  R2H  x  R1L  |                          #9
        lsrs    r5, yl, #16             // r5 = ylh
        muls    r5, r5, xl              // r5 = ylh * xhl

//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                |0000000---xh---|------r4-------|0000000---bin--|          - r7 low 16 bits are sticky bit bin, high 16 are zero
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                                |------r5-------|              
        adds    r5, r5, r6              // can't overflow, max is $fffe0001+$ffff = $ffff0000

// Combine two running partial products:
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                |0000000---xh---|               |0000000---bin--|          - r7 low 16 bits are sticky bit bin, high 16 are zero
//                        |0000000----r4--|
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  ...  x  R0L  |                          #7
//                                |  ...  x  R0H  |                          #8
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                                |------r5-------|              
        UXTHs   r6, r4
        adds    r5, r5, r6
        lsrs    r4, r4, #16

// Rearrange and combine running partial products, use upper half of sticky bit bin for temporary storage.
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                |0000000---xh---|       |--r7-----bin---|                  - r7 low 16 bits are sticky bit bin, high 16 are zero
//                        |000000x----r6--|                                  - r6 can be at most 0x1fffe
        lsls    r6, r5, #16
        orrs    r7, r7, r6
        lsrs    r6, r5, #16
        adds    r6, r6, r4

// #12: r5 = R3L x R1L
//                        |  R3L  x  R1L  |                                  #12
        UXTHs   r5, yh
        muls    r5, r5, xl

// Combine into partial product:
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |0000000---xh---|       |--r7-----bin---|                  - r7 low 16 bits are sticky bit bin, high 16 are zero
//                        |------r5-------|                                  - r6 can't overflow
        adds    r5, r5, r6

// Rearrange partial products:
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |000000x---xh---|       |--r7-----bin---|                  - xh can be at most 0x1fffe
//                        |0000000---r4---|                                  - r6 can't overflow
        UXTHs   r4, r5
        lsrs    r6, r5, #16
        adds    xh, xh, r6

// #14: r5 = R3H x R1L
//                |  R3H  x  R1L  |                                          #14
        lsrs    r5, yh, #16
        muls    r5, r5, xl

// Combine into partial product:
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//                |------r5-------|       |--r7-----bin---|                  - xh won't overflow
//                        |0000000---r4---|                                  - r6 can't overflow
        adds    r5, r5, xh              // Won't overflow, 0xfffe0001 + 0x1ffff = 0xffffffff

// Rearrange partial products
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//        |0000000---xh---|------r4-------|--r7-----bin---|
        lsls    r6, r5, #16
        orrs    r4, r4, r6
        lsrs    xh, r5, #16

// Retrieve old xh.
        mov     r5, lr

// #10: xl = R2L x R1H
//                                |  R2L  x  R1H  |                          #10
        lsrs    r5, r5, #16
        UXTHs   xl, yl
        muls    xl, xl, r5

// Combine partial products:
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//        |0000000---xh---|------r4-------|
//                                |  R2L  x  R1H  |                          #10
//                                |------xl-------|
//                                        |--r6-----bin---|

        lsrs    r6, r7, #16             // Isolate sum column *3 in r6
        adds    xl, xl, r6              // xl + column sum *3, can't overflow
        lsls    r6, xl, #16
        UXTHs   r7, r7
        orrs    r6, r6, r7

// Rearrange and combine partial products:
//                                        |  R3L  x  ...  |                  #4
//                                        |  R2H  x  ...  |                  #5
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                        |  R2L  x  ...  |                  #6
//                                |  R2H  x  R1L  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//                                |  R2L  x  R1H  |                          #10
//        |0000000---xh---|------r4-------|
//                        |000000x---r7---|                                  - r7 can be max 0x1fffe
//                                |  R2L  x  R1H  |                          #10
//                                        |--r6-----bin---|
        lsrs    r7, xl, #16
        UXTHs   xl, r4
        adds    r7, r7, xl

// #13: yl = R2H x R1H
//                        |  R2H  x  R1H  |                                  #13
        lsrs    yl, yl, #16
        muls    yl, yl, r5

// Combine partial products and simplify diagram whose products are fully formed
//                                |  R3H  x  ...  |                          #7
//                                |  R3L  x  ...  |                          #8
//                                |  R2H  x  ...  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//                                |  R2L  x  ...  |                          #10
//                        |  R2H  x  R1H  |                                  #13
//        |0000000---xh---|------r4-------|
//                        |------xl-------|--r6-----bin---|
        adds    xl, yl, r7

// Rearrange partial products:
//                                |  R3H  x  ...  |                          #7
//                                |  R3L  x  ...  |                          #8
//                                |  R2H  x  ...  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//                                |  R2L  x  ...  |                          #10
//                        |  R2H  x  R1H  |                                  #13
//                        |------r4-------|
//        |0000000---xh---|0000000--xl----|--r6-----bin---|
//                |0000000---yl---|
//                                        
        lsrs    yl, xl, #16
        UXTHs   xl, xl

// Combine partial products
//                                |  R3H  x  ...  |                          #7
//                                |  R3L  x  ...  |                          #8
//                                |  R2H  x  ...  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//                                |  R2L  x  ...  |                          #10
//                        |  R2H  x  R1H  |                                  #13
//        |0000000---xh---|0000000--xl----|--r6-----bin---|
//                |000000x---r4---|
//                                        
        lsrs    r4, r4, #16
        adds    r4, r4, yl              // r4 is max 0x1fffe

// #15: yl = R3L x R1H
//                |  R3L  x  R1H  |                                          #15
        UXTHs   yl, yh
        muls    yl, yl, r5

// Accumulate partial products:
//                                |  R3H  x  ...  |                          #7
//                                |  R3L  x  ...  |                          #8
//                                |  R2H  x  ...  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//                                |  R2L  x  ...  |                          #10
//                        |  R2H  x  R1H  |                                  #13
//                |  R3L  x  R1H  |                                          #15
//        |0000000---xh---|0000000--xl----|--r6-----bin---|
//                |------yl-------|
//                                        
        adds    yl, yl, r4

// Combine partial products:
//                                |  R3H  x  ...  |                          #7
//                                |  R3L  x  ...  |                          #8
//                                |  R2H  x  ...  |                          #9
//                        |  R3L  x  R1L  |                                  #12
//                |  R3H  x  R1L  |                                          #14
//                                |  R2L  x  ...  |                          #10
//                        |  R2H  x  R1H  |                                  #13
//                |  R3L  x  R1H  |                                          #15
//        |000000x---xh---|------xl-------|--r6-----bin---|
//                                        
        lsls    r4, yl, #16
        orrs    xl, xl, r4
        lsrs    yl, yl, #16
        adds    xh, xh, yl              // can't overflow

// #16: yh = R3H x R1L
//        |  R3H  x  R1H  |                                                  #16
        lsrs    yh, yh, #16
        muls    yh, yh, r5

// Combine partial products, and we have:
//                                                        |  R2L  x  R0L  |  #1
//                                                |  R2H  x  R0L  |          #2
//                                                |  R2L  x  R0H  |          #3
//                                        |  R3L  x  R0L  |                  #4
//                                        |  R2H  x  R0H  |                  #5
//                                        |  R2L  x  R1L  |                  #6
//                                |  R3H  x  R0L  |                          #7
//                                |  R3L  x  R0H  |                          #8
//                                |  R2H  x  R1L  |                          #9
//                                |  R2L  x  R1H  |                          #10
//                        |  R3H  x  R0H  |                                  #11
//                        |  R3L  x  R1L  |                                  #12
//                        |  R2H  x  R1H  |                                  #13
//                |  R3H  x  R1L  |                                          #14
//                |  R3L  x  R1H  |                                          #15
//        |  R3H  x  R1H  |                                                  #16
//        |------xh-------|------xl-------|--r6-----bin-------------------|
        adds    xh, xh, yh

// Restore product's exponent.
        pop     {r4}

// Generate 0x00100000 which is the msb of the product with hidden bit set.
        movs    r7, #1
        lsls    r5, r7, #20

// Is the product normalized?
        tst     xh, r5
        bne     L(normalized)

// No, shift product and adjust exponent.
        adds    r6, r6, r6
        adcs    xl, xl, xl
        adcs    xh, xh, xh
        subs    r4, r4, #1              // exponent adjust

// Product is normalized, must now round.
L(normalized):
        lsls    r7, r7, #31             // generate 0x80000000 in r7
        lsrs    r5, r7, #21             // generate 0x3ff in r5
        subs    r5, r5, #1

// Remove IEEE bias.  If exponent went negative, return a signed zero.
        subs    r4, r4, r5
        blt     L(signed_zero_trampoline)

// If exponent overflowed, generate a signed infinity.
        lsls    r5, r5, #1              // Generate 0x7fe
        cmp     r4, r5
        bge     L(inf_trampoline)

// Move exponent into position.
        lsls    r4, r4, #20

// Load zero here, but it doesn't really matter becasue MOVS only affects
// N and Z, not C.
        movs    r5, #0                  // does not affect carry anyway
        lsrs    yl, xl, #1              // lsb of stored product to carry
        sbcs    r6, r6, r7              // generate rounding bit
        adcs    xl, xl, r5              // shift rounding bit into significand
        adcs    xh, xh, r4              // combine carry-out from low half and exponent

// Apply sign to product.
L(apply_sign):
        mov     r6, ip
        orrs    xh, xh, r6
        RET4    r4, r5, r6, r7

L(signed_zero_trampoline):
        b      L(signed_zero)

L(inf_trampoline):
        b      L(inf)

END_FUNC __aeabi_dmul_aux

#else

#define MUL64_WORKING_SET r4, r5, r6, r7

ARM_GLOBAL_FUNC __aeabi_dmul

// Save working registers.
        push    {MUL64_WORKING_SET, lr}

// Load exponent mask.
        li      r6, 0x7ff

// Compute sign of result into ip{31}.  See (*) below.
        eor     ip, xh, yh

// Extract exponents and check for multiplication by zero.
// Subnormals are flushed to zero.
        ands    lr, r6, xh, lsr #20     // set lr to exponent of multiplicand, set Z if exponent is zero
        ittt    ne
        cmpne   lr, r6                  // test for Inf or NaN (conditionally)
        andsne  r5, r6, yh, lsr #20     // set r4 to exponent of multiplier, set Z only if exponent is zero
        cmpne   r5, r6                  // test for Inf or NaN (conditionally)
        beq     L(inf_nan_zero)         // one or other exponent is zero, Inf, or NaN...

// Extract significands and set hidden bit.
        bic     xh, xh, r6, lsl #21
        orr     xh, xh, #0x100000

// Perform multiplication by summing partial products.
        umull   r4, r7, xh, yl          // r7:r4 = high multiplier x low multiplicand
        bic     yh, yh, r6, lsl #21     // Extract significand and set hidden bit.
        orr     yh, yh, #0x100000
        add     lr, lr, r5              // Compute resulting exponent.
        umull   r6, r5, xl, yh          // r5:r6 = low multiplier x high mulr7licand
        and     ip, ip, #0x80000000     // Bury some work here (*) while multiplier busy
        adds    r6, r6, r4              // Sum middle order products freeing r4 and r7
        adcs    r5, r5, r7              // Set the C bit on output for carry below ($)
        umull   r4, r7, xh, yh          // r7:r4 = high multiplier x high multiplicand (product is r7:r4+r5:r6:0)
        adc     yh, r7, #0              // ($) Add carry in (product is now yh:r4+r5:r6)
        umull   xh, r7, xl, yl          // r7:xh = low multiplier x low multiplicand (product is now yh:r4+r5:r6+r7:xh)
        adds    r6, r6, r7              // Reduce partial sum (product is now yh:r4+r5+c:r6)
        adcs    r5, r5, r4              // Reduce partial sum (product is now yh+c:r5:r6)
        adcs    yh, yh, #0              // Reduce partial sum (product is now yh:r5:r6)

// We need to discard the low order product word but fold in lower-order digits.
        tst     xh, xh                  // Non-zero low product word - round to middle order
        it      ne
        orrne   r6, r6, #1

// Test for one normalization step (c.f. 11*11 = 1001 whereas 10*10 = 0100).
        li      r4, 0x7fe
        cmp     yh, #0x200
        bcs     L(must_norm)

// Remove double bias and check for underflow or overflow.
        sub     lr, lr, #0x400
        cmp     lr, r4
        bcs     L(underflow_or_overflow)

// Shift yh:r5:r6 left by 12 bits into xh:xl which achieves normalization.
        orr     xh, ip, yh, lsl #12
        lsls    xl, r5, #12
        orr     xh, xh, r5, lsr #20
        orr     xl, xl, r6, lsr #20

// Round result.  If Result > 0.5 then increment; and then if Result == 0.5, clear lsb
        lsls    yl, r6, #12
        lsrs    r5, xl, #1
        sbcs    yl, yl, #0x80000000
        adcs    xl, xl, #0
        adcs    xh, xh, lr, lsl #20
        RET4    MUL64_WORKING_SET

// Remove double bias and check for underflow or overflow.
L(must_norm):
#if __SEGGER_RTL_CORE_HAS_ADDW_SUBW
        sub     lr, lr, #0x3ff
#else
        sub     lr, lr, #0x400
        add     lr, lr, #1
#endif
        cmp     lr, r4
        bcs     L(underflow_or_overflow)

// Shift yh:r5:r6 left by 11 bits into xh:xl which achieves normalization.
        orr     xh, ip, yh, lsl #11
        lsls    xl, r5, #11
        orr     xh, xh, r5, lsr #21
        orr     xl, xl, r6, lsr #21

// Round result.  If Result > 0.5 then increment; and then if Result == 0.5, clear lsb
        lsls    yl, r6, #11
        lsrs    r5, xl, #1
        sbcs    yl, yl, #0x80000000
        adcs    xl, xl, #0
        adcs    xh, xh, lr, lsl #20
        RET4    MUL64_WORKING_SET

// Reform unbiased exponent and determine whether the original exponent
// overflowed or underflowed.
L(underflow_or_overflow):
        tst     lr, lr
        bpl     L(overflow)

// Handle underflow and flush to corrctly-signed zero.
L(underflow):
        and     xh, ip, #0x80000000     // Set sign bit of zero
        movs    xl, #0
        RET4    MUL64_WORKING_SET

// Handle the special case where one of the operands is Inf, NaN, or zero.
// The non-zero finite x non-zero finite case is the general case above so
// there is no need to decode it.
//
//   x |   0  nzf   Inf   NaN
// ----+---------------------
//   0 |   0    0   NaN   NaN
// nzf |   0    -   Inf   NaN
// Inf | NaN  Inf   Inf   NaN
// NaN | NaN  NaN   NaN   NaN
//
L(inf_nan_zero):

// Flush subnormals to appropriately-signed zero (only ms word as ls word is immaterial).
// Compress low-order significand bits of NaNs and finites into high order bits, hence
// the high-order word of each is enough to distinguish zero, normal, Inf, and NaN.
        tst     r6, xh, lsr #20         // zero exponent?
        itt     eq
        andeq   xh, xh, #0x80000000     // Yes, flush subnormal to signed zero
        moveq   xl, #0
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        tst     xl, xl                  // Test low-order bits
        it      ne
        orrne   xh, xh, #1              // Fold into high-order bits
#endif
        tst     r6, yh, lsr #20
        itt     eq
        andeq   yh, #0x80000000
        moveq   yl, #0
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        tst     yl, yl
        it      ne
        orrne   yh, yh, #1
#endif

// Get exponent of multiplicand, we may not have computed it beforehand.
        and     r4, r6, yh, lsr #20

// Check for +-0 * +-0.
        cmp     r4, r6
        it      ne
        cmpne   lr, r6
        beq     L(inf_nan)

// Multiplicand or multiplier are zero, so the result is a signed zero.
        and     xh, ip, #0x80000000
        movs    xl, #0                  // Use narrow encoding in Thumb-2
        RET4    MUL64_WORKING_SET

// Here we know that either the multiplier or multiplicand is Inf or NaN. Get rid
// of sign bits wich confuse matters.
L(inf_nan):
        bic     xh, xh, #0x80000000
        bic     yh, yh, #0x80000000

// Get rid of NaN input cases early.
        cmp     xh, r6, lsl #20
        bhi     L(nan_result)
        cmp     yh, r6, lsl #20
        bhi     L(nan_result)

// The cases we're left with are Inf*0, Inf*finite, 0*Inf, Inf*finite.  
// Any 0*Inf is NaN.
//
// If either the multiplier or multiplicand is zero, we have 0*Inf, 0*NaN, NaN*0, or Inf*0
// all of which must generate a NaN result.

#if __SEGGER_RTL_CORE_HAS_CBZ_CBNZ
        cbz     xh, L(nan_result)
        cbz     yh, L(nan_result)
#else
        cmp     xh, #0x0                // Z=1 if +0
        it      ne
        cmpne   yh, #0x0
        beq     L(nan_result)
#endif

// Generate an Inf with correct sign.
L(overflow):
        and     xh, ip, #0x80000000     // Isolate sign
        orr     xh, xh, #0x7f000000     // Load Inf to result registers
        orr     xh, xh, #0x00f00000
        movs    xl, #0
        RET4    MUL64_WORKING_SET

// Generate an AEABI-compliant NaN result.
L(nan_result):
        li      xh, 0x7ff80000
        movs    xl, #0                  // Use narrow encoding in Thumb-2
        RET4    MUL64_WORKING_SET

END_FUNC __aeabi_dmul

#endif

/*********************************************************************
*
*       __aeabi_fdiv()
*
*  Function description
*    Divide, single floating.
*
*  Prototype
*    float __aeabi_fdiv(float x, float, y);
*
*  Parameters
*    r0 - x - Dividend.
*    r1 - y - Divisor.
*
*  Return value
*    r0 - Quotient, x/y.
*/

#undef L
#define L(label) .L__aeabi_fdiv_##label

ARM_GLOBAL_FUNC __aeabi_fdiv

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vdiv.f32 s0, s0, s1
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define DIV_WORKING_SET   r4, r5

// Save working set.
        push    {DIV_WORKING_SET}

// Standard opening: compute sign of quotient into r5.
        movs    r5, r0
        eors    r5, r5, r1
        movs    r4, #1                  // Form 0x80000000
        lsls    r4, r4, #31
        ands    r5, r5, r4              // Isolate sign

// Continue with opening book: discard sign of operands, move exponents
// to high byte of operand registers.
        lsls    r2, r0, #1
        lsls    r3, r1, #1

// Exponent of lhs to r2 low order byte.
        lsrs    r2, r2, #24
        beq     L(lhs_zero_or_subnormal)

// Exponent of rhs to r3 low order byte.
        lsrs    r3, r3, #24
        beq     L(rhs_zero_or_subnormal)

// Exponent of lhs indicates Inf or NaN?
        cmp     r2, #0xff
        beq     L(lhs_inf_or_nan)

// Exponent of rhs indicates Inf or NaN?
        cmp     r3, #0xff
        beq     L(rhs_inf_or_nan)

// Compute quotients's exponent in r2, now true exponent without bias.
        subs    r2, r2, r3

// Standard form for division: align significands right and insert assumed bit.
        lsls    r0, r0, #8              // align to right
        orrs    r0, r0, r4              // insert sign bit, r4 is 0x80000000
        lsls    r1, r1, #8
        orrs    r1, r1, r4

// Move significands down leaving zero on high.
        lsrs    r3, r0, #1              // use r3 as dividend as we'll form the
        lsrs    r1, r1, #1              // quotient in r0 so we eliminate a movs.

// Set a single bit in r0 which is a sentinel and clear out quotient.
// On each iteration as the quotient is developed, the sentinel bit
// shifts too and when it pops out into the carry, we have iterated
// 23 times with the quotient is developed into the same register.
        lsrs    r0, r4, #22

// First subtraction step.  We don't use a trial compare, we'll just adjust
// by adding back if we can't do the subtraction.
        subs    r3, r3, r1
        bcs     L(shift_and_subtract)

// Adjust exponent accordingly, and shift in a zero quotient bit.
        subs    r2, r2, #1
        lsls    r3, r3, #1

// Add divisor back.
        adds    r3, r3, r1

// Shift-and-subtract loop proper.
L(shift_and_subtract):
        lsls    r3, r3, #1
        cmp     r3, r1
        bcc     L(cant_subtract)
        subs    r3, r3, r1
L(cant_subtract):
        adcs    r0, r0, r0
        bcc     L(shift_and_subtract)

// Apply the IEEE bias.  If close to underflow, decide what to do with rounding.
        adds    r2, #127
        ble     L(signed_zero)

// If overflowed, return a signed infinity.
        cmp     r2, #255
        bcs     L(inf)

// Quotient is now in r0[23:0] with r0[31:24] zero.

// Move exponent into position--do this now to avoid nuking the flags later.
        lsls    r2, r2, #23

// We use the remainder after division to find which direction
// to round in and break ties.  r3 is the remainder after division,
// and r0 is the quotient [see above], and r1 the divisor.
        subs    r1, r1, r3              // divisor - remainder
        cmp     r3, r1                  // compare with remainder, i.e. compare to 0.5.

// Round up if required and insert exponent.
        adcs    r0, r0, r2

// In rounding, it's impossible for a carry out of the 24th bit
// of the quotient.  The reason is that the quotient would have
// to be 0xFFFFFF and this isn't possible, given the range of
// normalized inputs (where the lower 24 bits are always zero),
// except in an isolated case (0x7FFFFF800000/0x800000), where
// there is no rounding required because there is no remainder.

// Insert sign into product.
L(apply_sign):
        orrs    r0, r0, r5

// Restore and return.
L(done):
        pop     {DIV_WORKING_SET}
        bx      lr

// Generate a positive infinity, then apply quotient sign.
L(inf):
        movs    r0, #0xff
        lsls    r0, r0, #23             // 0x7f800000
        b       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// Extract exponent of rhs to low bits of r3.
        lsrs    r3, r3, #24

// Treat subnormals as zero; if zero we're computing 0/0 which is NaN.
        beq     L(nan)

// If exponent is non-zero then the rhs is non-zero normal and not Inf
// and not NaN, hence 0/normal is signed zero.
        cmp     r3, #0xff
        bne     L(signed_zero)          // Return signed zero.

// rhs is either Inf or Nan.  We distinguish between the two by the significand
// bits.  If it's Inf, then 0/Inf is a signed zero.
        lsls    r1, r1, #9
        beq     L(signed_zero)          // Return signed zero.

// Result is AEABI NaN.
L(nan):
        movs    r0, #0x7f
        lsls    r0, r0, #4
        adds    r0, r0, #0xc
        lsls    r0, r0, #20
        b       L(done)

// We know that the LHS is Inf or NaN, but we haven't a clue what the rhs is (yet).
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.  Use significand bits to differentiate
// between NaN and Inf.
        lsls    r0, r0, #9
        bne     L(nan)                  // non-zero significand implies lhs NaN

// Now know the lhs is Inf.  If rhs is normal then Inf/zero-normal is Inf.
        cmp     r3, #0xff
        bne     L(inf)

// We now have a rhs of Inf or NaN, and Inf/Inf and Inf/NaN are both NaN.
        b       L(nan)

// We know that the rhs is zero or subnormal and the lhs is non-zero, Inf, or NaN.
L(rhs_zero_or_subnormal):

// If the lhs in normal, then normal/0 is a signed infinity.
        cmp     r2, #0xff
        bne     L(inf)

// Know the lhs is either Inf or NaN.  If NaN, NaN/normal is NaN.
        lsls    r0, r0, #9              // non-zero significand implies lhs NaN
        bne     L(nan)                  // ...and a NaN result

// Inf/0 is still Inf.
        b       L(inf)

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// Non-zero normal/NaN is NaN.
        lsls    r1, r1, #9              // non-zero significand implies rhs NaN
        bne     L(nan)                  // ...and a NaN result

// Non-zero normal/Inf is a signed zero.
L(signed_zero):
        movs    r0, r5                  // Correctly signed zero
        b       L(done)

#else

#if __SEGGER_RTL_OPTIMIZE >= 2 && !__SEGGER_RTL_CORE_HAS_IDIV

// If high optimization, try this version.
// If this #if is changed, also change the conditional definition

// Algorithm derived from "An Overview of Floating-Point Support and Math Library on the Intel XScale Architecture", Ping Tak Peter Tang.

#define DIV_WORKING_SET   r4, r5

        push    {DIV_WORKING_SET}
        mov     r3, #0x3fc
        ands    r5, r3, r1, lsr #21
        ittt    ne
        andsne  r4, r3, r0, lsr #21
        cmpne   r3, r5
        cmpne   r3, r4
        beq     L(special)

// Load approximation to reciprocal (32 bits) into ip using
// leading six bits of significand.
        la      ip, __aeabi_fdiv_reciprocal_table
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        ubfx    r3, r1, #17, #6
        ldr     ip, [ip, r3, lsl #2]
#else
        and     r3, r1, #0x7e0000
        ldr     ip, [ip, r3, lsr #15]
#endif

// Compute sign of quotient and store it into r4{0}.  r4{1} is
// kept zero, r4{31:2} is the quotient exponent.
        eor     r3, r0, r1              // Sign to r3{31}
        orr     r4, r4, r3, lsr #31     // Sign to r4{0}
        subs    r4, r4, r5              // Compute exponent of quotient into r4{31:2}.

// Break reciprocal into sign-extended high 8 bits in r2 and
// lower 24 bits in ip.
        asr     r2, ip, #24
        bic     ip, ip, #0xff000000

// Perform fractional multiply.
        lsls    r3, r1, #15
        lsrs    r3, r3, #25
        sbc     r3, r3, #63
        mla     r2, r3, r2, ip          // r2 := r2*r3 + ip

// Clear away exponent and set hidden bit.  Allow multiply to
// complete before next multiply starts.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        movw    ip, #1
        bfi     r1, ip, #23, #9
#else
        bic     r1, r1, #0xff000000
        orr     r1, r1, #0x00800000
#endif

// Avoid hazard with above multiply.
        mul     r3, r1, r2

// Clear away exponent and set hidden bit.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        bfi     r0, ip, #23, #9
#else
        bic     r0, r0, #0xff000000
        orr     r0, r0, #0x00800000
#endif

// Fractional multiply reciprocal by dividend which approximates
// the quotient in r5.  We have no use for the low part dumped
// into ip.
        smull   ip, r5, r3, r2

// Normalize ensuring r0 >= r1.
        cmp     r0, r1
        itte    cc
        subcc   r4, r4, #4              // Decrement exponent in r4{31:2} by 1.
        lslcc   r0, r0, #3              // Normalize
        lslscs  r0, r0, #2
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        rsc     r2, r5, r2, lsl #11
#else
        lsls    r2, r2, #11
        sbc     r2, r2, r5
#endif

// Fractional multiply reciprocal by dividend which approximates
// the quotient in r5.  We have no use for the low part dumped
// into ip.
        smull   ip, r5, r2, r0

// Get unshifted, biased quotient exponent into r2.  Hoisted
// so that multipliy can proceed without hazards.
        asrs    r2, r4, #2
        adds    r2, r2, #0x7e

// Apply correction.
        rsb     r0, r1, r0, lsl #22
        mul     r3, r5, r1

// Check for underflow or overflow condition.
        cmp     r2, #0xfe
        bcs     L(underflow_or_overflow)

// Rounding, compare remainder.
        subs    r0, r3, r0
        it      mi
        addmi   r5, r5, #1              // Setup ready for C=1 at adc below
        lsrs    r4, r4, #1              // Result sign to carry
        rrxs    r0, r5                  // Apply sign
        adc     r0, r0, r2, lsl #23
        pop     {DIV_WORKING_SET}
        bx      lr

// Underflow and overflow handling.
L(underflow_or_overflow):
        lsl     r0, r4, #31             // Put sign into r4{31}, make signed zero.
        itt     ge
        orrge   r0, r0, #0x7f000000     // If overflow, or-in Inf
        addge   r0, r0, #0x00800000
        pop     {DIV_WORKING_SET}
        bx      lr

// Get exponent of divisor, we have not computed it beforehand.
L(special):
        pop     {DIV_WORKING_SET}
        lsrs    r3, r3, #2              // Set r3 := #0xff
        and     r2, r3, r0, lsr #23

#else

#define DIV_WORKING_SET   r4, r5

// Load exponent mask.
        movs    r3, #0xff

// Extract exponent of dividend and divisor.
        ands    r2, r3, r0, lsr #23     // Set Z if dividend exponent zero
        ittt    ne
        andsne  r3, r3, r1, lsr #23     // Extract divisor exponent and set Z if dividend exponent zero

// Is either exponent 0xff which signals Inf or NaN?
        cmpne   r2, #0xff               // Only considering Z flag match here
        cmpne   r3, #0xff               // And here
        beq     L(inf_nan_zero)

// Calculate quotient's exponent using addition-of-powers law.
        subs    r2, r2, r3              // cc doesn't matter, use more compact Thumb coding
        adds    r2, r2, #0x7f

// Calculate quotient's sign.
        eor     ip, r1, r0
        and     ip, ip, #0x80000000     // Set ip=80000000 if -ve product, 0 otherwise.

// Normalize significand of dividend and divisor and dump in hidden bits.
        mov     r3, #0x80000000
        orr     r0, r3, r0, lsl #8      // Shift significand to upper 24 bits, add hidden bit
        orr     r1, r3, r1, lsl #8

//
// Division depends on whether the core has a divide instruction we can
// use to accelerate or whether we need to do the division by clockwork.
//

#if 0 && (__SEGGER_RTL_OPTIMIZE >= 0) && __SEGGER_RTL_CORE_HAS_IDIV

// Moeller division of r1*2^32 / r0.
// See reference code in floatops.c, function __SEGGER_RTL_Div64by32_Moeller().

// As the CM divider is fast, this code turns out to be slower than the smaller,
// byte-at-a-time division code below.

// Save registers.
        push    {r4, r5, r6}

// Ensure dividend < divisor.
        lsrs    r0, r0, #1

// Calculate reciprocal.
        movs    r6, #1
        add     r4, r6, r1, lsr #11     // (d >> 11) + 1
        lsrs    r3, r1, #22
        li      r6, 0xFFC200
        udiv    r5, r6, r3              // v0 (in r5) := ((1uL << 24) - (1uL<<14) + (1uL << 9)) / (d >> 22)
        mul     r3, r5, r5              // v0*v0
        umull   r3, r6, r3, r4          // __SEGGER_RTL_UMULL_HI(v0*v0, (d >> 11) + 1)  [r3 is a bit bucket]
        mvns    r6, r6                  // v1 (in r4) := (v0 << 4) - __SEGGER_RTL_UMULL_HI(v0*v0, (d >> 11) + 1)
        add     r4, r6, r5, lsl #4
        and     r3, r1, #1              // d0 (in r3) := d & 1
        rsbs    r6, r3, #0
        and     r6, r6, r4, lsr #1
        add     r3, r3, r1, lsr #1      // (d >> 1) + d0
        mls     r6, r4, r3, r6          // e := 0u - (v1 * ((d >> 1) + d0))
        umull   r6, r3, r4, r6          // __SEGGER_RTL_UMULL_HI(v1, e) [r6 is a bit bucket]
        lsls    r4, r4, #15             // v1 << 15
        add     r4, r4, r3, lsr #1      // v2 (in r4) = (v1 << 15) + (__SEGGER_RTL_UMULL_HI(v1, e) >> 1);
        mov     r3, r1                  // p0 = d
        mov     r6, r1                  // p1 = d
        umlal   r3, r6, r4, r1          // __SEGGER_RTL_UMLAL(p0, p1, v2, d)

// Now calculate quotient.
        sub     r6, r4, r6              // v2 - p1
        umull   r4, r3, r6, r0          // __SEGGER_RTL_UMULL(q0, q1, v2 - p1, u1)
        add     r0, r0, r3              // q1 += u1 + 1
        adds    r3, r0, #1
        mul     r6, r3, r1              // q1*d
        rsbs    r6, r6, #0              // r = 0u - q1*d
        cmp     r4, r6                  // if (r > q0)
        ite     cs
        movcs   r0, r3
        mlscc   r6, r3, r1, r1
        cmp     r6, r1                  // if (r >= d)...
        adcs    r0, r0, #0              // ...q1 += 1;

// Normalize.
        itt     pl
        lslpl   r0, r0, #1              // Adjust dividend
        subpl   r2, r2, #1              // And adjust exponent

// Insert sign, align significand, shift out least-significant bit of quotient into C.
        subs    r2, r2, #1
        orrs    r0, ip, r0, lsr #8

// Insert exponent taking care of overflow and also inserting the rounding bit.
// In rounding, it's impossible for a carry out of the 24th bit of the quotient.
// The reason is that the quotient would have to be 0xFFFFFF and this isn't
// possible, given the range of normalized inputs (where the lower 24 bits are
// always zero), except in an isolated case (0x7FFFFF800000/0x800000), where
// there is no rounding required because there is no remainder.
        adcs    r0, r0, r2, lsl #23
        adds    r2, r2, #1

// Restore registers.
        pop     {r4, r5, r6}

#elif (__SEGGER_RTL_OPTIMIZE >= 0) && __SEGGER_RTL_CORE_HAS_IDIV

//
// Divide r0/r1, generating 8 bits of quotient each time.
//

// Add an initial step so dividend < divisor.
        push    {DIV_WORKING_SET}
        subs    r3, r0, r1              // Trial subtraction, carry clear if failed
        itt     cc
        addcc   r3, r1, r3, lsl #1      // Subtraction failed, normalize dividend by shifting
        subcc   r2, r2, #1              // And adjust exponent

// Shift divisor to low 24 bits, divisor is exactly 24 bits in width and the dividend
// occupies the high 24 bits aligned to bit 31.
        lsrs    r1, r1, #8

// Develop eight fractional bits of quotient into r5 and remainder into r3.
        udiv    r5, r3, r1              // r5 := r3/r1
        mls     r3, r5, r1, r3          // r3 := r3 - r5*r1, i.e. r3 = r3 % r1 from above

// Develop eight fractional bits of quotient into r0 and remainder into r3.
        lsls    r3, r3, #8              // Shift remainder in order to fractional divide 32/24 with 8-bit quotient
        udiv    r0, r3, r1              // r0 := r3/r1
        mls     r3, r1, r0, r3          // r3 := r3 - r0*r1, i.e. r3 = r3 % r1 from above
        add     r0, r0, r5, lsl #8      // Combine fractions

// Develop eight fractional bits of quotient into r5 and no requirement for remainder.
        lsls    r3, r3, #8              // Shift remainder in order to fractional divide 32/24 with 8-bit quotient
        udiv    r5, r3, r1

// Combine fractional quotients.
        add     r0, r5, r0, lsl #8

// Insert sign, align significand, shift out least-significant bit of quotient into C.
        orrs    r0, ip, r0, lsr #1

// Insert exponent taking care of overflow and also inserting the rounding bit.
// In rounding, it's impossible for a carry out of the 24th bit of the quotient.
// The reason is that the quotient would have to be 0xFFFFFF and this isn't
// possible, given the range of normalized inputs (where the lower 24 bits are
// always zero), except in an isolated case (0x7FFFFF800000/0x800000), where
// there is no rounding required because there is no remainder.
        adcs    r0, r0, r2, lsl #23
        pop     {DIV_WORKING_SET}

#else

// Move sign into position, it's going to be shifted anyway.
        mov     ip, ip, lsr #23

// Run 24 division iterations.
        lsrs    r3, r0, #1
        lsrs    r1, r1, #1
        subs    r3, r3, r1
        itt     cc
        subcc   r2, r2, #1
        addcc   r3, r1, r3, lsl #1
#if __SEGGER_RTL_OPTIMIZE >= 0
       .rept    23
        rsbs    r3, r1, r3, lsl #1
        it      cc
        addcc   r3, r3, r1
        adc     ip, ip, ip
       .endr
#else
        push    {r4}
        movs    r4, #23
L(divide):
        rsbs    r3, r1, r3, lsl #1
        it      cc
        addcc   r3, r3, r1
        adc     ip, ip, ip
        subs    r4, r4, #1
        bne     L(divide)
        pop     {r4}
#endif

// Round result by comparing remainder and insert exponent.
        subs    r1, r1, r3              // divisor - remainder
        cmp     r3, r1                  // compare with remainder, i.e. compare to 0.5.
        adcs    r0, ip, r2, lsl #23     // insert exponent and round

#endif

// Handle zero out of line as this is not the common case.
        cmp     r2, #0
        ble     L(underflow)

// Return if no overflow.
        cmp     r2, #0xff
        it      cc
        bxcc    lr

// Handle overflow case, return +Inf or -Inf according to result sign.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        mov     r0, #0x7f800000         // Load +Inf*2
        ands    r1, ip, #0x80000000     // Isolate sign from computed significand
        orrs    r0, r0, r1              // Apply sign of result to Inf
        bx      lr
#else
        mov     r0, #0xff000000         // Load +Inf*2
        and     ip, ip, #0x80000000     // Isolate sign from computed significand
        orr     r0, ip, r0, lsr #1      // Apply sign of result to Inf
        bx      lr
#endif

// Handle underflow case, return signed zero.
L(underflow):
        and     r0, ip, #0x80000000
        bx      lr

#endif

// Handle the special case where one of the operands is Inf, NaN, or zero.
// The non-zero finite / non-zero finite case is the general case above so
// there is no need to decode it.
//
//   / |   0  nzf   Inf   NaN
// ----+---------------------
//   0 | NaN    0     0   NaN
// nzf | Inf    -     0   NaN
// Inf | Inf  Inf   NaN   NaN
// NaN | NaN  NaN   NaN   NaN
//
L(inf_nan_zero):

// Get exponent of divisor, we may not have computed it beforehand.
        and     r3, r3, r1, lsr #23

// If divisor is finite, skip this test.
        cmp     r3, #0xff
        bne     L(divisor_finite)

// If dividend is Inf or NaN, the result is NaN.
        cmp     r2, #0xff
        beq     L(nan_result)

// If divisor is Inf, result is zero.
        movs    ip, r1, lsl #9
        beq     L(zero_result)

// Divisor is NaN, result is NaN.
        mov     r0, #0x00c00000         // Pre-load NaN indicator               
        b       L(inf_nan_common)       // Or in common Inf/NaN indicator and return.

// Deal with the case of a finite divisor.
L(divisor_finite):

// If the dividend is Inf or NaN, the result is Inf or NaN with appropriate sign.
        cmp     r2, #0xff
        bne     L(dividend_finite)

// Dividend is Inf or NaN.  Inf returns Inf with appropriate sign, NaNs are converted to AEABI NaNs.
        lsls    r3, r0, #1
        cmp     r3, 0xff000000          // Dividend is Inf?
        beq     L(inf_result)
        b       L(nan_result)

// Zero divided by zero is NaN.
L(dividend_finite):
        orrs    r2, r2, r3
        beq     L(nan_result)

// Finite divided by zero is Inf.
        cmp     r3, #0
        beq     L(inf_result)

L(zero_result):
        eors    r0, r0, r1              // Compute sign of result (use narrow Thumb-2 encoding)
        and     r0, #0x80000000         // Remove excess significand bits
        bx      lr

// If the dividend is zero, the result is NaN else the result is Inf.
L(nan_result):
        mov     r0, #0x00c00000         // Pre-load NaN indicator
        b       L(inf_nan_common)       // Or in common Inf/NaN indicator.

L(inf_result):
        eors    r0, r0, r1              // Compute sign of Inf (use narrow Thumb-2 encoding)
        and     r0, #0x80000000         // Remove excess significand bits
        orr     r0, #0x00800000         // Or in Inf indicator.
L(inf_nan_common):
        orr     r0, #0x7f000000         // Or in common Inf/NaN indicator.
        bx      lr

#endif

END_FUNC __aeabi_fdiv

/*********************************************************************
*
*       __aeabi_ddiv()
*
*  Function description
*    Divide, double floating.
*
*  Prototype
*    double __aeabi_ddiv(double x, double y);
*
*  Parameters
*    r1:r0 - x - Dividend, hi:lo.
*    r2:r1 - y - Divisor, hi:lo.
*
*  Return value
*    r1:r0 - Quotient, hi:lo, x/y.
*/

#undef z0
#undef L
#define L(label) .L__aeabi_ddiv_##label

ARM_GLOBAL_FUNC __aeabi_ddiv

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vdiv.f64 d0, d0, d1
        vmov    xl, xh, d0
        bx lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// This is coded to be small, not fast.  We assume that Cortex-M0 devices
// will be short on flash and so we should do everything we can to minimize
// code size, hence the use of branches rather than straight-lining in the
// not-common cases.  Except in the divide loop proper, though, as here we
// manage to reduce execution time by having two different code paths per
// division iteration.  We could common it up by introducing an extra
// instruction to clear the carry, but that would add an extra 26 cycles
// on average, so we don't do it.

#define DIV64_WORKING_SET  r4, r5, r6, r7

        push    {DIV64_WORKING_SET}

// Generate 0x80000000.
        movs    r5, #1
        lsls    r5, r5, #31

// Compute sign of quotient into ip.
        movs    r4, xh
        eors    r4, r4, yh
        ands    r4, r4, r5              // Isolate sign bit only
        mov     ip, r4

// Generate 0x00200000; this is used to compare below.
        lsrs    r6, r5, #10

// Remove sign bit from lhs and rhs and shift significands and exponents into position.
        adds    r4, xh, xh
        adds    r7, yh, yh

// If the lhs has a special exponent indicating Inf or NaN, do that out of line.
        cmn     r4, r6
        bcs     L(lhs_inf_or_nan)
        cmn     r7, r6
        bcs     L(rhs_inf_or_nan)

// Align lhs exponent to lsb.  If the exponent is zero, that indicates that the
// lhs is zero or subnormal and requires special treatment.
        lsrs    r4, r4, #21
        beq     L(lhs_zero_or_subnormal)

// Now know that the lhs operand is normal and the rhs operand is zero,
// subnormal, or normal.  Align rhs exponent to lsb.  If the rhs exponent is
// zero, then we have normal/0 which is a signed Inf.
        lsrs    r7, r7, #21
        beq     L(inf)

// Compute quotient's exponent into r4, freeing r7.
        subs    r4, r4, r7

// Materialize hidden bit in lhs, moving significand to r7.
        lsls    xh, xh, #11             // lhs aligned to msb
        orrs    xh, xh, r5              // materialize hidden bit
        lsrs    r7, xh, #11             // move back into position, but in r7.

// Do the same for the rhs.
        lsls    yh, yh, #11
        orrs    yh, yh, r5
        lsrs    yh, yh, #11

// First subtraction step.  We don't use a trial compare, we'll just adjust
// by adding back if we can't do the subtraction.
        movs    r6, xl
        subs    r6, r6, yl
        sbcs    r7, r7, yh

// If subtraction can be done, we enter the loop proper.
        bcs     L(shift_and_subtract)

// Adjust exponent accordingly, and shift in a zero quotient bit.
        subs    r4, r4, #1
        adds    r6, r6, r6
        adcs    r7, r7, r7

// Add divisor back.
        adds    r6, r6, yl
        adcs    r7, r7, yh

// Apply double precision bias (3FF) to exponent.
L(shift_and_subtract):
        asrs    xl, r5, #9              // 0xFFC00000
        lsrs    xl, xl, #22             // 0x000003FF
        adds    r4, r4, xl              // apply bias

// If exponent underflowed to zero then return a signed zero--we don't
// support subnormals.
        ble     L(signed_zero)

// If overflowed, i.e. exponent >= 0x7FF, then Inf.
        adds    xl, xl, xl              // 0x000007FE
        cmp     r4, xl                  // > 0x7FE implies >= 0x7FF
        bhi     L(inf)

// Prepare quotient registers; set a sentinel bit to indicate
// when to exit the loop.  As the quotient is is shifted on each
// iteration when developing the quotient, the sentinel bit will
// eventually pop into the carry indication completion.
        lsrs    xl, r5, #19
        movs    xh, #0

// Shift-and-subtract loop proper.
L(divide_one_bit):

// Shift dividend.
        adds    r6, r6, r6
        adcs    r7, r7, r7

// Trial subtract of divisor.
        subs    r6, r6, yl
        sbcs    r7, r7, yh
        bcc     L(cant_subtract)

// Can do the subtraction, so shift a one bit into quotent.
        adcs    xl, xl, xl
        adcs    xh, xh, xh

// If the sentinel didn't pop out, need to carry on.
        bcc     L(divide_one_bit)
        b       L(rounding)

// Can't subtract, so add back.
L(cant_subtract):
        adds    r6, r6, yl
        adcs    r7, r7, yh

// Shift zero bit into quotent
        adds    xl, xl, xl
        adcs    xh, xh, xh

// If the sentinel didn't pop out, need to carry on.
        bcc     L(divide_one_bit)

// All iterations done.
L(rounding):

// Move exponent into position.
        lsls    r4, r4, #20

// Now need to round.  Test remainder, if < 1/2 the divisor then there
// is no rounding to be done.  What we actually do is double both sides
// of the inequality, i.e. if remainder < divisor/2 is 2*remainder < divisor.
        adds    r6, r6, r6              // double remainder
        adcs    r7, r7, r7
        subs    r6, r6, yl              // compare with divisor by subtraction
        sbcs    r7, r7, yh
        bcc     L(insert_exponent)

// Because we subtracted rather than compared above, we now have zero in
// the remainder registers iff the remainder is exactly half the divisor
// which is how we deal with breaking ties in round-to-nearest mode.
        orrs    r7, r7, r6              // r7 is zero iff remainder is exactly divisor/2
        lsls    yl, xl, #31             // move lsb of quotient ready to break ties
        orrs    r7, r7, yl              // ...we only need zero or non-zero, could be anywhere...
        movs    yl, #0                  // zero register as adc won't take immediate and we want flags preserved
        cmp     r7, #1                  // C=1 iff r7 is not zero
        adcs    xl, xl, yl              // ...and round

// Insert exponent into significand; the exponent is in r4.  We also add the
// carry held over from the previous instruction.  If we land up here from
// the branch above, the carry is clear anyway and makes no contibution.
L(insert_exponent):
        adcs    xh, xh, r4

// Apply quotient sign to result; the quotient sign is in ip.
L(apply_sign):
        mov     yl, ip
        orrs    xh, xh, yl

// All done, so restore working set and return.
L(done):
        pop     {DIV64_WORKING_SET}
        bx      lr

// Know exponent is 0x7ff indicating Inf or NaN.  If significand is non-zero,
// it's a NaN and NaN/anything is NaN.
L(lhs_inf_or_nan):
        bne     L(nan)
        CBNZx   xl, L(nan)

// Inf/Inf is NaN, and Inf/NaN is NaN, so there's no need to distinguish
// whether the rhs is Inf or NaN as they both generate NaN anyway.
        cmn     r7, r6
        bcs     L(nan)

// Now we know that the lhs is Inf and the rhs is normal, zero, or subnormal.
// Inf/0 or Inf/Normal is Inf.
L(inf):
        movs    xl, #0
        mvns    xh, xl
        lsrs    xh, xh, #21
        lsls    xh, xh, #20
        b       L(apply_sign)

// Know the lhs is normal or zero.  If the rhs is NaN then normal/NaN or
// 0/NaN is NaN.  If the rhs is Inf then 0/Inf and Normal/Inf is a signed zero.
L(rhs_inf_or_nan):
        CBNZx   yl, L(nan)
        cmn     r7, r6
        beq     L(signed_zero)

// Generate a EABI NaN result.
L(nan):
        movs    xl, #0
        mvns    xh, xl                  // 0xffffffff
        lsrs    xh, xh, #20             // 0x00000fff
        lsls    xh, xh, #19             // 0x7ff80000
        b       L(done)

// Know lhs is zero or subnormal and rhs is zero or subnormal (i.e. not Inf
// and not NaN).  If rhs is zero or subnormal then we have 0/0 which is NaN.
L(lhs_zero_or_subnormal):
        lsrs    r7, r7, #21             // Extract rhs exponent
        beq     L(nan)                  // 0/0 or 0/subnormal is NaN.

// Have 0/normal which is a signed zero.
L(signed_zero):
        movs    xl, #0
        mov     xh, ip
        b       L(done)

#elif __SEGGER_RTL_OPTIMIZE >= 0

// Algorithm derived from "An overview of floating-point support and math
// library on the Intel XScale architecture".

#define DIV64_WORKING_SET r4, r5, r6, r7, r8, r9

// Save working registers.
        push    {DIV64_WORKING_SET, lr}

// Compute quotient sign.
        eors    lr, xh, yh
        and     lr, #0x80000000

// Test if the dividend and divisor are both finite.
        li      r7, 0xffe00000
        ands    r5, r7, xh, asl #1      // set Z if exponent of dividend is zero
        ittt    ne
        cmpne   r5, r7                  // test for Inf or NaN (conditionally)
        andsne  r5, r7, yh, lsl #1      // set Z if exponent of divisor is zero
        cmpne   r5, r7                  // test for Inf or NaN (conditionally)
        beq     L(inf_nan_zero)         // one or other exponent is zero, test for 0*Inf and 0*NaN

// Generate 0x7ff.
        lsrs    r7, r7, #21

// Extract exponent and significand from divisor.
        ands    r6, r7, yh, lsr #20     // r6 is biased exponent of divisor
        bic     yh, yh, r7, lsl #21     // Clear exponent from divisor
        orr     yh, yh, #0x100000       // Set hidden bit

// Extract exponent and significand from dividend.
        ands    r5, r7, xh, lsr #20     // r5 is biased exponent of dividend
        bic     xh, xh, r7, lsl #21     // Clear exponent from dividend
        orr     xh, xh, #0x100000       // Set hidden bit

// Compute biased exponent of quotient into r5.
        subs    r5, r5, r6
#if __SEGGER_RTL_CORE_HAS_ADDW_SUBW
        add     r5, r5, #0x3fd
#else
        add     r5, r5, #0x300
        adds    r5, r5, #0x0fd
#endif

//
// Compute approximation to reciprocal of divisor into r7.  We can
// do this either by using a bipartite table if we don't have a
// division instruction, or by using a division instruction.  The
// quotient is accurate to at least 12 bits in the bipartite case.
//

#if __SEGGER_RTL_CORE_HAS_IDIV

//
// Compute reciprocal using division, but ensure we do not overestimate
// the reciprocal too high.
//

        lsr     ip, yh, #7              // 12 leading bits of dividend
        adds    ip, ip, #1              // Always underestimate the reciprocal
        mov     r7, #0x80000000
        udiv    r7, r7, ip              // Compute 1/x to 18 bits
        lsls    r7, r7, #5

#else

//
// No divider, compute reciprocal using bipartite table.
//

// Set ip as pointer to 7-bits-in, 16-bits out reciprocal table TH.
// Point r9 to 7-bit-in, 8-bit out bipartite table TL. 
        la      ip, __aeabi_ddiv_reciprocal_table-0x100
        add     r9, ip, #0x180

// Load table entry TL.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        ldrb    r9, [r9, yh, lsr #13]
#else
        lsrs    r7, yh, #13
        ldrb    r9, [r9, r7]
#endif

// Load table entry TH.
        mvn     r7, #1
        and     r7, r7, yh, lsr #12
        ldrh    r7, [ip, r7]

// Compute Br = TH*2^7 + 2^15 * (1 - 2^7 - f*2^(-7)) * TL where f is b14..b8 of divisor.
        lsrs    ip, yh, #5              // Isolate f (b14..b8 of divisor)
        ands    ip, ip, #0xfe           // Now f in bits ip{7:1} with ip{0} = 0.
        rsbs    ip, ip, #0xfe           // Negate f
        lsls    r7, r7, #7              // TH *= 2^7
        mla     r7, r9, ip, r7          // Compute Br(r7) = TL(r9) * -f(ip) + TH*2^7(r7)

#endif

// Compute R = -B * Br, where B is the divisor, in r4:ip:r9.
        umull   r9, ip, yl, r7          // ip:r9 = Br * B[low] 
        movs    r4, #0
        umlal   ip, r4, yh, r7          // r4:ip:r9 = Br * (B[high] * 2^32) + Br * B[low]
        rsbs    r9, r9, #0
        sbc     ip, ip, ip, lsl #1      // xh = xh-2xh-c = xh-c

// A[r,0] = A * Br where A is the dividend.  A is held in xh:xl
        umull   r6, yl, xl, r7          // yl:r6 = Br * A[low] 
        movs    xl, #0
        umlal   yl, xl, xh, r7          // A[r,0] = xl:yl:r6 = Br * (A[high] * 2^32) + Br * A[low]

// Iteration #1.
// A'[j+1] = (A'[j]low << 11) + R*d[j]
// Q[j+1]  = (Q[j] << 11) + d[j]
// Note that d[j] == floor(A[r,j] * 2^(11j) / 2^(64)) which can be 
// obtained directly from the leading bits of A[r,j].

// Note: We hoist loading a zero at the head of the iteration (c.f. movs r7, #0 below)
// into the previous iteration to enable adc x, x, #0 to be replaced with adx x, x, Rn
// where Rn is zero, enabling a narrow instruction to be coded with no loss of generality.
        movs    r7, #0
        lsls    r8, r6, #11             // A'[j]low << 11
        umlal   r8, r7, xl, r9          // A'[j+1] r7:r8 += A'[j]low * Rlow
        lsrs    r6, r6, #21             // Q[j+1] = Q[j] << 11 + d[j]
        orr     r6, r6, yl, lsl #11
        lsrs    yl, yl, #21
        adds    r6, r6, r7
        movs    r7, #0                  // Hoisted
        adcs    yl, yl, r7
        umlal   r6, yl, xl, ip          // A'[j+1] yl:r6 += A'[j]low * Rhigh

// Iteration #2.  Do a total of five iterations.  The general form
// is identical to above except that after each iteration the registers
// used to hold A'[j] and Q[j] change; we don't rearrange as that
// adds instructions and cycles.

        lsls    yh, r8, #11
        umlal   yh, r7, yl, r9
        lsrs    r8, r8, #21
        orr     r8, r8, r6, lsl #11
        lsrs    r6, r6, #21
        adds    r8, r8, r7
        movs    xh, #0                  // Hoisted
        adcs    r6, r6, xh
        umlal   r8, r6, yl, ip
        add     xl, yl, xl, lsl #11

// Iteration #3.
        lsls    yl, yh, #11
        umlal   yl, xh, r6, r9
        lsrs    yh, yh, #21
        orr     yh, yh, r8, lsl #11
        lsrs    r8, r8, #21
        adds    yh, yh, xh
        movs    r7, #0                  // Hoisted
        adcs    r8, r8, r7
        umlal   yh, r8, r6, ip

// Iteration #4.
        lsls    xh, yl, #11
        umlal   xh, r7, r8, r9
        lsrs    yl, yl, #21
        orr     yl, yl, yh, lsl #11
        lsrs    yh, yh, #21
        adds    yl, yl, r7
        movs    r7, #0
        adcs    yh, yh, r7              // Hoisted
        umlal   yl, yh, r8, ip
        add     r6, r8, r6, lsl #11

// Iteration #5.
        lsls    r8, xh, #11
        umlal   r8, r7, yh, r9
        lsrs    xh, xh, #21
        orr     xh, xh, yl, lsl #11
        lsrs    yl, yl, #21
        adds    xh, xh, r7
        adc     yl, yl, #0
        umlal   xh, yl, yh, ip

// If A'[5] > B*Br then Q[5] = Q[5] + 1
        lsls    r7, xl, #1
        adds    r9, r9, r8
        adcs    ip, ip, xh
        sbcs    r4, yl, r4              // C bit now indicates A'[5] > B*Br from subtraction
        adcs    xl, yh, r6, lsl #11
        adc     yh, r7, r6, lsr #21
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        movs    r6, #2
        cmp     yh, #0x800000
        adc     r6, r6, #0
        adc     r5, r5, #0
#else
        movs    r6, #0
        cmp     yh, #0x800000
        adcs    r6, r6, r6
        adds    r5, r5, r6
        adds    r6, r6, #2
#endif

// Align significand yh:xl, capture shifted-out bits into r4.
        rsbs    r9, r6, #32
        lsls    r4, xl, r9
        lsrs    xl, xl, r6
        lsls    yl, yh, r9
        orrs    xl, xl, yl
        lsrs    yh, yh, r6

// Round, there can never be a tie.
        lsrs    r4, r4, #31             // Shifted-out bits are captive in r4
        adds    xl, xl, r4
        adc     yh, yh, #0

// Ensure normalized.
        cmp     yh, #0x200000
        itt     cs
        lsrcs   yh, yh, #1
        addcs   r5, r5, #1

// And exponent overflow.
        li      r7, 0x7fe
        cmp     r5, r7
        bcs     L(zero_or_inf_result)

// Pack.
        orr     xh, lr, r5, lsl #20     // Exponent and sign
        add     xh, xh, yh              // Significand

// Return.
        RET6    DIV64_WORKING_SET

// Check for exponent underflow.
L(zero_or_inf_result):
        tst     r5, r5
        bpl     L(inf_result)

// Generate a zero result.
L(zero_result):
        mov     xh, lr
        b       L(zero_low_half_return)

// Fold NaN significand bits from low order, leaves Inf unchanged.
L(inf_nan_zero):
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        tst     xl, xl
        it      ne
        orrne   xh, xh, #1
        tst     yl, yl
        it      ne
        orrne   yh, yh, #1
#endif

// Propagate NaNs, if either operand is a NaN the result is a NaN.
        cmp     r7, xh, lsl #1
        ite     cs
        cmpcs   r7, yh, lsl #1
        bcc     L(nan_result)

// Inf/Inf is NaN, if both operands are Inf the result is a NaN.
        cmp     r7, xh, lsl #1
        itt     eq
        cmpeq   r7, yh, lsl #1
        beq     L(nan_result)

// 0/0 is NaN.
        tst     r7, xh, lsl #1
        itt     eq
        tsteq   r7, yh, lsl #1
        beq     L(nan_result)

// 0/anything is zero.
        tst     r7, xh, lsl #1
        beq     L(zero_result)
        
// Inf/0 and finite/0 are appropriately-signed Inf.
        tst     r7, yh, lsl #1
        beq     L(inf_result)

// Now we either have Inf/finite or finite/Inf.
        cmp     r7, yh, lsl #1          // Divisor Inf?
        beq     L(zero_result)          // finite/Inf == correctly-signed 0.

// Generate an Inf result.
L(inf_result):
        orr     xh, lr, #0x7f000000
        orr     xh, xh, #0x00f00000
L(zero_low_half_return):
        movs    xl, #0
        RET6    DIV64_WORKING_SET

// Geneate a NaN result.
L(nan_result):
        li      xh, 0x7ff80000
        b       L(zero_low_half_return)

#else

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
  #define DIV64_WORKING_SET r4, r5, r6, lr
  #define z0 ip
#else
  #define DIV64_WORKING_SET r4, r5, r6, r7, lr
  #define z0 r7
#endif

// Save working registers.
        push    {DIV64_WORKING_SET}

// Compute quotient sign.
        eor     lr, xh, yh
        and     lr, #0x80000000

// Test if the dividend and divisor are both finite.
        li      z0, 0xffe00000
        ands    r5, z0, xh, asl #1      // set Z if exponent of dividend is zero
        ittt    ne
        teqne   r5, z0                  // test for Inf or NaN (conditionally)
        andsne  r4, z0, yh, lsl #1      // set Z if exponent of divisor is zero
        teqne   r4, z0                  // test for Inf or NaN (conditionally)
        beq     L(inf_nan_zero)         // one or other exponent is zero, test for 0*Inf and 0*NaN

// Compute difference in exponents to give quotient exponent.
        lsrs    r5, r5, #21             // Use narrow Thumb-2 encoding
        sub     z0, r5, r4, lsr #21
        add     z0, z0, #0x400          // 0x400 as true subtraction of 0x3ff is performed below (*).

// Align dividend significand to msb in r4:xl.
        mov     r6, #0x80000000
        orr     r4, r6, xh, lsl #11
        orr     r4, r4, xl, lsr #21
        lsls    xl, xl, #11             // Use narrow Thumb-2 encoding

// Align divisor significand to msb in yh:yl.
        orr     yh, r6, yh, lsl #11
        orr     yh, yh, yl, lsr #21
        lsls    yl, yl, #11             // Use narrow Thumb-2 encoding

// More complex case of division where the divisor spans two words.
// Use a non-restoring algorithm which is more complex to write but
// has a good performance benefit.  We have dividend in r4:xl, divisor
// in yh:yl.

// Do the first subtraction.  If dividend >= divisor, then
// the remainder is good and all that is left to do is to set
// up the loop and plug in the first quotient bit and go.
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcs     L(aligned_correctly)

// Otherwise, dividend < divisor.  So fix up the remainder as
// though we'd shifted the dividend up 1 bit before the trial.
        adds    xl, xl, xl
        adcs    r4, r4, r4
        adds    xl, xl, yl
        adcs    r4, r4, yh

// Adjust the exponent, since we effectively left-shifted the dividend.
        subs    z0, z0, #1

// Now, shift the divisor down by one bit.
L(aligned_correctly):
        movs    yh, yh, lsr #1
        mov     yl, yl, rrx

// xh:r5 holds developing quotient.
#if __SEGGER_RTL_CORE_HAS_CLRM
        clrm    {xh, r5}
#else
        movs    r5, #0
        movs    xh, #0
#endif

// The first quotient bit is always '1'.  Insert it.
        orr     xl, xl, #1

// Prepare the loop counter and start the division loop.
        mov     r6, #7
        b       L(step_1x)              // Jump in

// Eight bits of the quotient are produced each loop apart from the last
// which develops four.

// Quotient bit = '1' portion of coroutine.
L(step_1):
        subs    xl, xl, yl              // Perform trial subtraction for quotient bit Qn
        sbcs    r4, r4, yh
        bcc     L(step_0a)              // Qn = 0, go make it so
L(step_1a):
        adcs    xl, xl, xl              // Qn = 1
        adcs    r4, r4, r4
L(step_1x):
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcc     L(step_0b)
L(step_1b):
        adcs    xl, xl, xl
        adcs    r4, r4, r4
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcc     L(step_0c)
L(step_1c):
        adcs    xl, xl, xl
        adcs    r4, r4, r4
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcc     L(step_0d)
L(step_1d):
        adcs    xl, xl, xl
        adcs    r4, r4, r4
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcc     L(step_0e)
L(step_1e):
        adcs    xl, xl, xl
        adcs    r4, r4, r4
        subs    r6, r6, #1              // Another 8 bits to go?
        beq     L(step_1i)
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcc     L(step_0f)
L(step_1f):
        adcs    xl, xl, xl
        adcs    r4, r4, r4
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcc     L(step_0g)
L(step_1g):
        adcs    xl, xl, xl
        adcs    r4, r4, r4
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcc     L(step_0h)
L(step_1h):
        adcs    xl, xl, xl
        adcs    r4, r4, r4
        lsls    xh, xh, #8              // Shift quotient up eight bits
        add     xh, xh, r5, lsr #24
        eors    r5, xl, r5, lsl #8      // Merge quotient to lower eight bits (messing upper 24 bits)
        bic     xl, xl, #0xff           // Remove quotient bits from remainder
        eors    r5, r5, xl              // Restore upper 24 bits of quotient
        b       L(step_1)

// Loop ends on '1' bit generation.  Develop one extra bit of quotient.
L(step_1i):
        subs    xl, xl, yl
        sbcs    r4, r4, yh
        bcc     L(no_rounding)
        b       L(with_rounding)

// P is negative, quotient bit = '0' portion of coroutine.
L(step_0):
        adds    xl, xl, yl              // Perform trial addition for quotient bit Qn
        adcs    r4, r4, yh
        bcs     L(step_1a)              // Qn = 1, go make it so
L(step_0a):
        adds    xl, xl, xl              // Qn = 0
        adcs    r4, r4, r4
        adds    xl, xl, yl
        adcs    r4, r4, yh
        bcs     L(step_1b)
L(step_0b):
        adds    xl, xl, xl
        adcs    r4, r4, r4
        adds    xl, xl, yl
        adcs    r4, r4, yh
        bcs     L(step_1c)
L(step_0c):
        adds    xl, xl, xl
        adcs    r4, r4, r4
        adds    xl, xl, yl
        adcs    r4, r4, yh
        bcs     L(step_1d)
L(step_0d):
        adds    xl, xl, xl
        adcs    r4, r4, r4
        adds    xl, xl, yl
        adcs    r4, r4, yh
        bcs     L(step_1e)
L(step_0e):
        adds    xl, xl, xl
        adcs    r4, r4, r4
        subs    r6, r6, #1              // Another 8 bits to go?
        beq     L(step_0i)
        adds    xl, xl, yl
        adcs    r4, r4, yh
        bcs     L(step_1f)
L(step_0f):
        adds    xl, xl, xl
        adcs    r4, r4, r4
        adds    xl, xl, yl
        adcs    r4, r4, yh
        bcs     L(step_1g)
L(step_0g):
        adds    xl, xl, xl
        adcs    r4, r4, r4
        adds    xl, xl, yl
        adcs    r4, r4, yh
        bcs     L(step_1h)
L(step_0h):
        adds    xl, xl, xl
        adcs    r4, r4, r4
        lsls    xh, xh, #8              // Shift quotient up eight bits (Use narrow Thumb-2 encoding)
        add     xh, xh, r5, lsr #24
        eors    r5, xl, r5, lsl #8      // Merge quotient to lower eight bits (messing upper 24 bits)
        bic     xl, xl, #0xff           // Remove quotient bits from remainder
        eors    r5, r5, xl              // Restore upper 24 bits of quotient (Use narrow Thumb-2 encoding)
        b       L(step_0)

// Loop ends on '0' bit generation.  Develop one extra bit of quotient.
L(step_0i):
        adds    xl, xl, yl
        adcs    r4, r4, yh
        bcc     L(no_rounding)          // Next bit of quotient is zero so no rounding required

// Fold 
L(with_rounding):
        lsrs    yl, xl, #5              // Test remainder bits (Use narrow Thumb-2 encoding)
        it      ne
        orrne   r4, r4, #1              // Fold non-zero low remainder bits to high remainder bits.

// Pack significand.
        lsls    xh, xh, #5              // Use narrow Thumb-2 encoding
        add     xh, xh, r5, lsr #27
        and     xl, xl, #0x1f
        add     xl, xl, r5, lsl #5

// Remove hidden bit.
        bic     xh, xh, #0x00100000

// Pack sign.
        orr     xh, xh, lr

// Round up which we require.
        adds    xl, xl, #1
        adc     xh, xh, #0

// If remainder is zero then we need to break the tie to nearest even.
        tst     r4, r4
        it      eq
        biceq   xl, xl, #1

// Test for exponent underflow and overflow.
        cmp     z0, #0x800              // Overflow?
        bge     L(inf_result)           // Yep, too bad.
        subs    z0, z0, #1              // (*) Compute true exponent
        ble     L(zero_result)
        add     xh, xh, z0, lsl #20     // Pack true exponent
        pop     {DIV64_WORKING_SET}
        bx      lr

// Pack significand.
L(no_rounding):
        lsls    xh, xh, #5              // Use narrow Thumb-2 encoding
        add     xh, xh, r5, lsr #27
        and     xl, xl, #0x1f
        add     xl, xl, r5, lsl #5

// Remove hidden bit.
        bic     xh, xh, #0x00100000

// Pack sign.
        orr     xh, xh, lr

// Test for exponent underflow and overflow.
        cmp     z0, #0x800              // Overflow?
        bge     L(inf_result)           // Yep, too bad.
        subs    z0, z0, #1              // (*) Compute true exponent
        ble     L(zero_result)
        add     xh, xh, z0, lsl #20     // Pack true exponent
        pop     {DIV64_WORKING_SET}
        bx      lr

// Fold NaN significand bits from low order, leaves Inf unchanged.
L(inf_nan_zero):
        tst     xl, xl
        it      ne
        orrne   xh, xh, #1
        tst     yl, yl
        it      ne
        orrne   yh, yh, #1

// Propagate NaNs, if either operand is a NaN the result is a NaN.
        cmp     z0, xh, lsl #1
        ite     cs
        cmpcs   z0, yh, lsl #1
        bcc     L(nan_result)

// Inf/Inf is NaN, if both operands are Inf the result is a NaN.
        cmp     z0, xh, lsl #1
        itt     eq
        cmpeq   z0, yh, lsl #1
        beq     L(nan_result)

// 0/0 is NaN.
        tst     z0, xh, lsl #1
        itt     eq
        tsteq   z0, yh, lsl #1
        beq     L(nan_result)

// 0/anything is zero.
        tst     z0, xh, lsl #1
        beq     L(zero_result)
        
// Inf/0 and finite/0 are appropriately-signed Inf.
        tst     z0, yh, lsl #1
        beq     L(inf_result)

// Now we either have Inf/finite or finite/Inf.
        cmp     z0, yh, lsl #1          // Divisor Inf?
        beq     L(zero_result)          // finite/Inf == correctly-signed 0.

// Generate an Inf result.
L(inf_result):
        orr     xh, lr, #0x7f000000
        orr     xh, xh, #0x00f00000
L(zero_low_half_return):
        movs    xl, #0                  // Use narrow Thumb-2 encoding
        pop     {DIV64_WORKING_SET}
        bx      lr

// Geneate a NaN result.
L(nan_result):
        mov     xh, #0x7f000000
        orr     xh, #0x00f80000
        b       L(zero_low_half_return)

// Generate a zero result.
L(zero_result):
        mov     xh, lr
        b       L(zero_low_half_return)

#endif

END_FUNC __aeabi_ddiv

/*********************************************************************
*
*       __aeabi_fcmple()
*
*  Function description
*    Less than or equal, single floating.
*
*  Prototype
*    int __aeabi_fcmple(float x, float y);
*
*  Parameters
*    r0 - x - Left-hand operand.
*    r1 - y - Right-hand operand.
*
*  Return value
*    r0 - One if x, y are non-NaN and x <= y; else zero.
*/

#undef L
#define L(label) .L__aeabi_fcmple_##label

ARM_GLOBAL_FUNC __aeabi_fcmple

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmpe.f32 s0, s1
        fmstat
        CSETx   r0, ls
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7fffffff) > 0x7f800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xff000000.  The ARM gives us a great way to perform
// this comparison using compare negated, CMN.
        movs    r2, #1
        lsls    r2, r2, #24
        lsls    r3, r0, #1
        cmn     r2, r3                  // Is lhs NaN?
        bhi     L(zero)                 // Comparison with NaN is invalid.
        lsls    r3, r1, #1              // If lhs isn't NaN, see if rhs is NaN
        cmn     r2, r3
        bhi     L(zero)

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        movs    r2, r0
        orrs    r2, r2, r1
        lsls    r2, r2, #1
        beq     L(one)                  // LHS and RHS are both +-0.
        bcs     L(negative)

// Both positive, generate truth value.
        cmp     r1, r0
        sbcs    r0, r0, r0
        adds    r0, r0, #1
        bx      lr

// Either negative, generate truth.
L(negative):
        cmp     r0, r1
        sbcs    r0, r0, r0
        adds    r0, r0, #1
        bx      lr

L(one):
        movs    r0, #1
        bx      lr

L(zero):
        movs    r0, #0
        bx      lr

#else

// Is LHS NaN...?  LS if not NaN, HI if NaN.
        lsls    r2, r0, #1
        lsls    r3, r1, #1
        cmn     r2, #0x01000000

// ...or RHS NaN?
        ite     ls
        cmnls   r3, #0x01000000

// If either are NaN, unordered and return false.
        bhi     L(zero)

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        orrs    r2, r0, r1
        lsls    r2, r2, #1
        beq     L(one)                 // LHS and RHS are both +-0.
        bcs     L(negative)

// Both positive, generate truth value.
        cmp     r1, r0
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cs
#else
        sbcs    r0, r0, r0
        adds    r0, r0, #1
#endif
        bx      lr

// Either negative, generate truth.
L(negative):
        cmp     r0, r1
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cs
#else
        sbcs    r0, r0, r0
        adds    r0, r0, #1
#endif
        bx      lr

L(zero):
        movs    r0, #0
        bx      lr

L(one):
        movs    r0, #1
        bx      lr

#endif

END_FUNC __aeabi_fcmple

/*********************************************************************
*
*       __aeabi_dcmple()
*
*  Function description
*    Less than or equal, double floating.
*
*  Prototype
*    int __aeabi_dcmple(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    r0 - One if x, y are non-NaN and x <= y; else zero.
*/

#undef L
#define L(label) .L__aeabi_dcmple_##label

ARM_GLOBAL_FUNC __aeabi_dcmple

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmpe.f64 d0, d1
        fmstat
        CSETx   r0, ls
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define CMP64LE_WORKING_SET {r4, r5}

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        push    CMP64LE_WORKING_SET
        movs    r5, #1
        lsls    r5, #21                 // 0x00200000
        lsls    r4, xh, #1              // cast out sign bits
        cmn     r5, r4                  // Is lhs NaN?
        bhi     L(zero)
        lsls    r4, yh, #1
        cmn     r5, r4                  // Is rhs NaN?
        bhi     L(zero)

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        movs    r4, xh
        orrs    r4, r4, yh
        lsls    r4, r4, #1
        orrs    r4, r4, xl
        orrs    r4, r4, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(one)
        bcs     L(negative)

// We now have non-zero operands.
        cmp     yl, xl
        sbcs    yh, yh, xh
        sbcs    r0, r0, r0
        adds    r0, r0, #1
        pop     CMP64LE_WORKING_SET
        bx      lr

L(negative):
        cmp     xl, yl
        sbcs    xh, xh, yh
        sbcs    r0, r0, r0
        adds    r0, r0, #1
        pop     CMP64LE_WORKING_SET
        bx      lr

// Return false; operands unordered.
L(zero):
        movs    r0, #0
        pop     CMP64LE_WORKING_SET
        bx      lr

// Return true.
L(one):
        movs    r0, #1
        pop     CMP64LE_WORKING_SET
        bx      lr

#else

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        mov     ip, #0x00200000
        cmn     ip, xh, lsl #1          // Is lhs NaN?
        it      ls
        cmnls   ip, yh, lsl #1          // If lhs isn't NaN, see if rhs is NaN
        bhi     L(zero)                 // Comparison with any NaN, unordered.

// OK, now get down to business of the compare.  Test for zero and different
// signs...
        orrs    ip, xh, yh              // combine high parts, msb is "at least one negative"
        orrs    ip, xl, ip, lsl #1      // maintain invariant: Z iff x == +0.0 or -0.0; C is now "at least one negative"
        orrs    ip, ip, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(one)
        bcs     L(negative)             // C iff x or y were negative

// Do 64-bit compare.
        cmp     yl, xl
        sbcs    yh, yh, xh
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cs
#else
        sbcs    r0, r0, r0
        adds    r0, r0, #1
#endif
        bx      lr

L(negative):
        cmp     xl, yl
        sbcs    xh, xh, yh
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cs
#else
        sbcs    r0, r0, r0
        adds    r0, r0, #1
#endif
        bx      lr

// Unordered always returns false.
L(zero):
        movs    r0, #0
        bx      lr

// Return true.
L(one):
        movs    r0, #1
        bx      lr

#endif

END_FUNC __aeabi_dcmple

/*********************************************************************
*
*       __aeabi_fcmpeq()
*
*  Function description
*    Equal, single floating.
*
*  Prototype
*    int __aeabi_fcmpeq(float x, float y);
*
*  Parameters
*    r0 - Left-hand operand.
*    r1 - Right-hand operand.
*
*  Return value
*    r0 - One if x, y are non-NaN and x == y; else zero.
*/

#undef L
#define L(label) .L__aeabi_fcmpeq_##label

ARM_GLOBAL_FUNC __aeabi_fcmpeq

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmpe.f32 s0, s1
        fmstat
        CSETx   r0, eq
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// -0 and +0 compare equal even though signs differ, so compare low 31 bits
// of both to zero.
        movs    r2, r0
        orrs    r2, r2, r1
        lsls    r2, r2, #1
        beq     L(equal)

// Operands are both non-zero.  If both do not compare bitwise equal,
// which includes comparisons with NaN which are always unequal, then we
// know they are not equal.
        cmp     r0, r1
        bne     L(notequal)

// Two bit patterns compare bitwise equal, but we must deal with NaNs.
// NaNs, even if they have identical bit patterns, always compare not-equal.
        lsls    r0, r0, #1
        movs    r1, #1
        lsls    r1, r1, #24
        cmn     r0, r1
        bls     L(equal)

L(notequal):
        movs    r0, #0
        bx      lr

L(equal):
        movs    r0, #1
        bx      lr

#else

// -0 and +0 compare equal even though signs differ, so compare low 31 bits
// of both to zero.
        orrs    r2, r0, r1
        lsls    r2, r2, #1
        beq     L(equal)

// Operands are both non-zero.  If both do not compare bitwise equal,
// which includes comparisons with NaN which are always unequal, then we
// know they are not equal.
        cmp     r0, r1
        bne     L(notequal)

// Two bit patterns compare bitwise equal, but we must deal with NaNs.
// NaNs, even if they have identical bit patterns, always compare not-equal.
        lsls    r0, r0, #1
        cmn     r0, #0x01000000
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, ls
        bx      lr
#else
        bls     L(equal)
#endif

L(notequal):
        movs    r0, #0
        bx      lr

L(equal):
        movs    r0, #1
        bx      lr

#endif

END_FUNC __aeabi_fcmpeq

/*********************************************************************
*
*       __aeabi_dcmpeq()
*
*  Function description
*    Equal, double floating.
*
*  Prototype
*    int __aeabi_dcmpeq(double x, double , y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    r0 - One if x, y are non-NaN and x == y; else zero.
*/

#undef L
#define L(label) .L__aeabi_dcmpeq_##label

ARM_GLOBAL_FUNC __aeabi_dcmpeq

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmpe.f64 d0, d1 //fcmped  d0, d1
        fmstat
        CSETx   r0, eq
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define CMP64EQ_WORKING_SET {r4, r5, r6}

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        push    CMP64EQ_WORKING_SET
        movs    r6, #1
        lsls    r6, #21                 // 0x00200000
        lsls    r4, xh, #1              // cast out sign bits
        lsls    r5, yh, #1
        cmn     r6, r4                  // Is lhs NaN?
        bhi     L(zero)
        cmn     r6, r5                  // Is rhs NaN?
        bhi     L(zero)

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        orrs    r5, r5, r4
        orrs    r5, r5, xl
        orrs    r5, r5, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(one)

// We now have non-zero operands.
L(compare):
        cmp     xh, yh
        bne     L(zero)                 // If high parts the same, low parts break the tie.
        cmp     xl, yl
        bne     L(zero)

// Return true.
L(one):
        movs    r0, #1
        pop     CMP64EQ_WORKING_SET
        bx      lr

// Either both operands are unequal or one of them is a NaN.
L(zero):
        movs    r0, #0
        pop     CMP64EQ_WORKING_SET
        bx      lr

#else

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        mov     ip, #0x00200000
        cmn     ip, xh, lsl #1          // Is lhs NaN?
        it      ls
        cmnls   ip, yh, lsl #1          // If lhs isn't NaN, see if rhs is NaN
        bhi     L(zero)                 // Comparison with any NaN, unordered.

// OK, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        orrs    ip, xl, xh, lsl #1      // Z iff x == +0.0 or -0.0
        it      eq
        orrseq  ip, yl, yh, lsl #1      // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(one)

// Equality if bit patterns equal.
L(compare):
        cmp     xl, yl
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        it      eq
        cmpeq   xh, yh
        cset    r0, eq
#else
        itte    eq
        cmpeq   xh, yh
        moveq   r0, #1
        movne   r0, #0
#endif
        bx      lr

// Unordered always returns false.
L(zero):
        movs    r0, #0
        bx      lr

// Both operands equal.
L(one):
        movs    r0, #1
        bx      lr

#endif

END_FUNC __aeabi_dcmpeq

/*********************************************************************
*
*       __aeabi_fcmpge()
*
*  Function description
*    Greater than or equal, single floating.
*
*  Prototype
*    int __aeabi_fcmpge(float x, float y);
*
*  Parameters
*    r0 - Left-hand operand.
*    r1 - Right-hand operand.
*
*  Return value
*    r0 - One if x, y are non-NaN and x >= y; else zero.
*/

#undef L
#define L(label) .L__aeabi_fcmpge_##label

ARM_GLOBAL_FUNC __aeabi_fcmpge

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmpe.f32 s0, s1 //fcmpes  s0, s1
        fmstat
        CSETx   r0, ge
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7fffffff) > 0x7f800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xff000000.  The ARM gives us a great way to perform
// this comparison using compare negated, CMN.
        movs    r2, #1
        lsls    r2, r2, #24
        lsls    r3, r0, #1
        cmn     r2, r3                  // Is lhs NaN?
        bhi     L(zero)                 // Comparison with NaN is invalid.
        lsls    r3, r1, #1              // If lhs isn't NaN, see if rhs is NaN
        cmn     r2, r3
        bhi     L(zero)

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        movs    r2, r0
        orrs    r2, r2, r1
        lsls    r2, r2, #1
        beq     L(one)                  // LHS and RHS are both +-0.
        bcs     L(negative)

// Both positive, generate truth value.
        cmp     r0, r1
        sbcs    r0, r0, r0
        adds    r0, r0, #1
        bx      lr

// Either negative, generate truth.
L(negative):
        cmp     r1, r0
        sbcs    r0, r0, r0
        adds    r0, r0, #1
        bx      lr

L(one):
        movs    r0, #1
        bx      lr

L(zero):
        movs    r0, #0
        bx      lr

#else

// Is LHS NaN...?  LS if not NaN, HI if NaN.
        lsls    r2, r0, #1
        lsls    r3, r1, #1
        cmn     r2, #0x01000000

// ...or RHS NaN?
        ite     ls
        cmnls   r3, #0x01000000

// If either are NaN, unordered and return false.
        bhi     L(zero)

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        orrs    r2, r0, r1
        lsls    r2, r2, #1
        beq     L(one)                  // LHS and RHS are both +-0.
        bcs     L(negative)

// Both positive, generate truth value.
        cmp     r0, r1
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cs
#else
        sbcs    r0, r0, r0
        adds    r0, r0, #1
#endif
        bx      lr

// Either negative, generate truth.
L(negative):
        cmp     r1, r0
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cs
#else
        sbcs    r0, r0, r0
        adds    r0, r0, #1
#endif
        bx      lr

L(zero):
        movs    r0, #0
        bx      lr

L(one):
        movs    r0, #1
        bx      lr

#endif

END_FUNC __aeabi_fcmpge

/*********************************************************************
*
*       __aeabi_dcmpge()
*
*  Function description
*    Greater than or equal, double floating.
*
*  Prototype
*    int __aeabi_dcmpge(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    r0 - One if x, y are non-NaN and x >= y; else zero.
*/

#undef L
#define L(label) .L__aeabi_dcmpge_##label

ARM_GLOBAL_FUNC __aeabi_dcmpge

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmpe.f64 d0, d1 //fcmped  d0, d1
        fmstat
        CSETx   r0, ge
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define CMP64GE_WORKING_SET {r4, r5}

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        push    CMP64GE_WORKING_SET
        movs    r5, #1
        lsls    r5, #21                 // 0x00200000
        lsls    r4, xh, #1              // cast out sign bits
        cmn     r5, r4                  // Is lhs NaN?
        bhi     L(zero)
        lsls    r4, yh, #1
        cmn     r5, r4                  // Is rhs NaN?
        bhi     L(zero)

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        movs    r4, xh
        orrs    r4, r4, yh
        lsls    r4, r4, #1
        orrs    r4, r4, xl
        orrs    r4, r4, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(one)
        bcs     L(negative)

// We now have non-zero operands.
        cmp     xl, yl
        sbcs    xh, xh, yh
        sbcs    r0, r0, r0
        adds    r0, r0, #1
        pop     CMP64GE_WORKING_SET
        bx      lr

L(negative):
        cmp     yl, xl
        sbcs    yh, yh, xh
        sbcs    r0, r0, r0
        adds    r0, r0, #1
        pop     CMP64GE_WORKING_SET
        bx      lr

// Return false; operands unordered.
L(zero):
        movs    r0, #0
        pop     CMP64GE_WORKING_SET
        bx      lr

// Return true.
L(one):
        movs    r0, #1
        pop     CMP64GE_WORKING_SET
        bx      lr

#else

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        mov     ip, #0x00200000
        cmn     ip, xh, lsl #1          // Is lhs NaN?
        it      ls
        cmnls   ip, yh, lsl #1          // If lhs isn't NaN, see if rhs is NaN
        bhi     L(zero)                 // Comparison with any NaN, unordered.

// OK, now get down to business of the compare.  Test for zero and different
// signs...
        orrs    ip, xh, yh              // combine high parts, msb is "at least one negative"
        orrs    ip, xl, ip, lsl #1      // maintain invariant: Z iff x == +0.0 or -0.0; C is now "at least one negative"
        orrs    ip, ip, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(one)
        bcs     L(negative)             // C iff x or y were negative

// Do 64-bit compare.
        cmp     xl, yl
        sbcs    xh, xh, yh
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cs
#else
        sbcs    r0, r0, r0
        adds    r0, r0, #1
#endif
        bx      lr

L(negative):
        cmp     yl, xl
        sbcs    yh, yh, xh
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cs
#else
        sbcs    r0, r0, r0
        adds    r0, r0, #1
#endif
        bx      lr

// Unordered always returns false.
L(zero):
        movs    r0, #0
        bx      lr

// Return true.
L(one):
        movs    r0, #1
        bx      lr

#endif

END_FUNC __aeabi_dcmpge

/*********************************************************************
*
*       __aeabi_fcmpgt()
*
*  Function description
*    Greater than, single floating.
*
*  Prototype
*    int __aeabi_fcmpgt(float x, float y);
*
*  Parameters
*    r0 - x - Left-hand operand.
*    r1 - y - Right-hand operand.
*
*  Return value
*    r0 - One if x, y are non-NaN and x > y; else zero.
*/

#undef L
#define L(label) .L__aeabi_fcmpgt_##label

ARM_GLOBAL_FUNC __aeabi_fcmpgt

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmpe.f32 s0, s1
        fmstat
        CSETx   r0, gt
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7fffffff) > 0x7f800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xff000000.  The ARM gives us a great way to perform
// this comparison using compare negated, CMN.
        movs    r2, #1
        lsls    r2, r2, #24
        lsls    r3, r0, #1
        cmn     r2, r3                  // Is lhs NaN?
        bhi     L(zero)                 // Comparison with NaN is invalid.
        lsls    r3, r1, #1              // If lhs isn't NaN, see if rhs is NaN
        cmn     r2, r3
        bhi     L(zero)

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        movs    r2, r0
        orrs    r2, r2, r1
        lsls    r2, r2, #1
        beq     L(zero)                 // LHS and RHS are both +-0.
        bcs     L(negative)

// Both positive, generate truth value.
        cmp     r1, r0
        sbcs    r0, r0, r0
        negs    r0, r0
        bx      lr

// Either negative, generate truth.
L(negative):
        cmp     r0, r1
        sbcs    r0, r0, r0
        negs    r0, r0
        bx      lr

L(zero):
        movs    r0, #0
        bx      lr

#else

// Is LHS NaN...?  LS if not NaN, HI if NaN.
        lsls    r2, r0, #1
        lsls    r3, r1, #1
        cmn     r2, #0x01000000

// ...or RHS NaN?
        ite     ls
        cmnls   r3, #0x01000000

// If either are NaN, unordered and return false.
        bhi     L(zero)

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        orrs    r2, r0, r1
        lsls    r2, r2, #1
        beq     L(zero)                 // LHS and RHS are both +-0.
        bcs     L(negative)

// Both positive, generate truth value.
        cmp     r1, r0
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cc
#else
        sbcs    r0, r0, r0
        negs    r0, r0
#endif
        bx      lr

// Either negative, generate truth.
L(negative):
        cmp     r0, r1
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cc
#else
        sbcs    r0, r0, r0
        negs    r0, r0
#endif
        bx      lr

L(zero):
        movs    r0, #0
        bx      lr

#endif

END_FUNC __aeabi_fcmpgt

/*********************************************************************
*
*       __aeabi_dcmpgt()
*
*  Function description
*    Greater than, double floating.
*
*  Prototype
*    int __aeabi_dcmpgt(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    r0 - One if x, y are non-NaN and x > y; else zero.
*/

#undef L
#define L(label) .L__aeabi_dcmpgt_##label

ARM_GLOBAL_FUNC __aeabi_dcmpgt

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmpe.f64 d0, d1
        fmstat
        CSETx   r0, gt
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define CMP64GT_WORKING_SET {r4, r5}

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        push    CMP64GT_WORKING_SET
        movs    r5, #1
        lsls    r5, #21                 // 0x00200000
        lsls    r4, xh, #1              // cast out sign bits
        cmn     r5, r4                  // Is lhs NaN?
        bhi     L(zero)
        lsls    r4, yh, #1
        cmn     r5, r4                  // Is rhs NaN?
        bhi     L(zero)

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        movs    r4, xh
        orrs    r4, r4, yh
        lsls    r4, r4, #1
        orrs    r4, r4, xl
        orrs    r4, r4, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(zero)
        bcs     L(negative)

// We now have non-zero operands.
        cmp     yl, xl
        sbcs    yh, yh, xh
        sbcs    r0, r0, r0
        negs    r0, r0
        pop     CMP64GT_WORKING_SET
        bx      lr

L(negative):
        cmp     xl, yl
        sbcs    xh, xh, yh
        sbcs    r0, r0, r0
        negs    r0, r0
        pop     CMP64GT_WORKING_SET
        bx      lr

// Return false; operands unordered.
L(zero):
        movs    r0, #0
        pop     CMP64GT_WORKING_SET
        bx      lr

#else

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        mov     ip, #0x00200000
        cmn     ip, xh, lsl #1          // Is lhs NaN?
        it      ls
        cmnls   ip, yh, lsl #1          // If lhs isn't NaN, see if rhs is NaN
        bhi     L(zero)                 // Comparison with any NaN, unordered.

// OK, now get down to business of the compare.  Test for zero and different
// signs...
        orrs    ip, xh, yh              // combine high parts, msb is "at least one negative"
        orrs    ip, xl, ip, lsl #1      // maintain invariant: Z iff x == +0.0 or -0.0; C is now "at least one negative"
        orrs    ip, ip, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(zero)
        bcs     L(negative)             // C iff x or y were negative

// Do 64-bit compare.
        cmp     yl, xl
        sbcs    yh, yh, xh
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cc
#else
        sbcs    r0, r0, r0
        negs    r0, r0
#endif
        bx      lr

L(negative):
        cmp     xl, yl
        sbcs    xh, xh, yh
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cc
#else
        sbcs    r0, r0, r0
        negs    r0, r0
#endif
        bx      lr

// Unordered always returns false.
L(zero):
        movs    r0, #0
        bx      lr

#endif

END_FUNC __aeabi_dcmpgt

/*********************************************************************
*
*       __aeabi_fcmplt()
*
*  Function description
*    Less than, single floating.
*
*  Prototype
*    int __aeabi_fcmplt(float x, float y);
*
*  Parameters
*    r0 - x - Left-hand operand.
*    r1 - y - Right-hand operand.
*
*  Return value
*    r0 - One if x, y are non-NaN and x < y; else zero.
*/

#undef L
#define L(label) .L__aeabi_fcmplt_##label

ARM_GLOBAL_FUNC __aeabi_fcmplt

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmpe.f32 s0, s1 //fcmpes  s0, s1
        fmstat
        CSETx   r0, mi
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7fffffff) > 0x7f800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xff000000.  The ARM gives us a great way to perform
// this comparison using compare negated, CMN.
        movs    r2, #1
        lsls    r2, r2, #24
        lsls    r3, r0, #1
        cmn     r2, r3                  // Is lhs NaN?
        bhi     L(zero)                 // Comparison with NaN is invalid.
        lsls    r3, r1, #1              // If lhs isn't NaN, see if rhs is NaN
        cmn     r2, r3
        bhi     L(zero)

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        movs    r2, r0
        orrs    r2, r2, r1
        lsls    r2, r2, #1
        beq     L(zero)                 // LHS and RHS are both +-0.
        bcs     L(negative)

// Both positive, generate truth value.
        cmp     r0, r1
        sbcs    r0, r0, r0
        negs    r0, r0
        bx      lr

// Either negative, generate truth.
L(negative):
        cmp     r1, r0
        sbcs    r0, r0, r0
        negs    r0, r0
        bx      lr

L(zero):
        movs    r0, #0
        bx      lr

#else

// Is LHS NaN...?  LS if not NaN, HI if NaN.
        lsls    r2, r0, #1
        lsls    r3, r1, #1
        cmn     r2, #0x01000000

// ...or RHS NaN?
        ite     ls
        cmnls   r3, #0x01000000

// If either are NaN, unordered and return false.
        bhi     L(zero)

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        orrs    r2, r0, r1
        lsls    r2, r2, #1
        beq     L(zero)                 // LHS and RHS are both +-0.
        bcs     L(negative)

// Both positive, generate truth value.
        cmp     r0, r1
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cc
#else
        sbcs    r0, r0, r0
        negs    r0, r0
#endif
        bx      lr

// Either negative, generate truth.
L(negative):
        cmp     r1, r0
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cc
#else
        sbcs    r0, r0, r0
        negs    r0, r0
#endif
        bx      lr

L(zero):
        movs    r0, #0
        bx      lr

#endif

END_FUNC __aeabi_fcmplt

/*********************************************************************
*
*       __aeabi_dcmplt()
*
*  Function description
*    Less than, double floating.
*
*  Prototype
*    int __aeabi_dcmplt(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    r0 - One if x, y are non-NaN and x < y; else zero.
*/

#undef L
#define L(label) .L__aeabi_dcmplt_##label

ARM_GLOBAL_FUNC __aeabi_dcmplt

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmpe.f64 d0, d1
        fmstat
        CSETx   r0, mi
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define CMP64LT_WORKING_SET {r4, r5}

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        push    CMP64LT_WORKING_SET
        movs    r5, #1
        lsls    r5, #21                 // 0x00200000
        lsls    r4, xh, #1              // cast out sign bits
        cmn     r5, r4                  // Is lhs NaN?
        bhi     L(zero)
        lsls    r4, yh, #1
        cmn     r5, r4                  // Is rhs NaN?
        bhi     L(zero)

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        movs    r4, xh
        orrs    r4, r4, yh
        lsls    r4, r4, #1
        orrs    r4, r4, xl
        orrs    r4, r4, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(zero)
        bcs     L(negative)

// We now have non-zero operands.
        cmp     xl, yl
        sbcs    xh, xh, yh
        sbcs    r0, r0, r0
        negs    r0, r0
        pop     CMP64LT_WORKING_SET
        bx      lr

L(negative):
        cmp     yl, xl
        sbcs    yh, yh, xh
        sbcs    r0, r0, r0
        negs    r0, r0
        pop     CMP64LT_WORKING_SET
        bx      lr

// Return false; operands unordered.
L(zero):
        movs    r0, #0
        pop     CMP64LT_WORKING_SET
        bx      lr

#else

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        mov     ip, #0x00200000
        cmn     ip, xh, lsl #1          // Is lhs NaN?
        it      ls
        cmnls   ip, yh, lsl #1          // If lhs isn't NaN, see if rhs is NaN
        bhi     L(zero)                 // Comparison with any NaN, unordered.

// OK, now get down to business of the compare.  Test for zero and different
// signs...
        orrs    ip, xh, yh              // combine high parts, msb is "at least one negative"
        orrs    ip, xl, ip, lsl #1      // maintain invariant: Z iff x == +0.0 or -0.0; C is now "at least one negative"
        orrs    ip, ip, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(zero)
        bcs     L(negative)             // C iff x or y were negative

// Do 64-bit compare.
        cmp     xl, yl
        sbcs    xh, xh, yh
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cc
#else
        sbcs    r0, r0, r0
        negs    r0, r0
#endif
        bx      lr

L(negative):
        cmp     yl, xl
        sbcs    yh, yh, xh
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cset    r0, cc
#else
        sbcs    r0, r0, r0
        negs    r0, r0
#endif
        bx      lr

// Unordered always returns false.
L(zero):
        movs    r0, #0
        bx      lr

#endif

END_FUNC __aeabi_dcmplt

/*********************************************************************
*
*       __aeabi_fcmpun()
*
*  Function description
*    Unordered, single floating.
*
*  Prototype
*    int __aeabi_fcmpun(float x, float y);
*
*  Parameters
*    r0 - x - Left-hand operand.
*    r1 - y - Right-hand operand.
*
*  Return value
*    r0 - One if x or y are NaN; else zero.
*/

#undef L
#define L(label) .L__aeabi_fcmpun_##label

ARM_GLOBAL_FUNC __aeabi_fcmpun

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmp.f32 s0, s1
        vmrs    APSR_nzcv, fpscr
        CSETx   r0, vs
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Most likely is that both operands are ordered, so optimize hot trace
// for that situation.

// Here we want to test for NaN.  NaN is signalled if the exponent of
// the operand is 0xff and the significand is non-zero.  If the exponent
// is 0xff, we know the operand is either Inf or Nan.
        lsls    r2, r0, #1               // Discard sign bit, isolate exponent in MSB
        lsrs    r2, r2, #24
        cmp     r2, #0xff
        bne     L(lhs_finite)            // No, then not Inf and not NaN, so finite

// Now differentiate Inf from NaN.  If the significand of the lhs is non-zero,
// then we have a NaN input and we know the operands are unordered.
        lsls    r2, r0, #9               // Shift off sign and exponent leaving significand
        bne     L(unordered)             // Keep off the hot trace

// Now do the same set of tests for the rhs.
L(lhs_finite):
        lsls    r2, r1, #1               // Discard sign bit, isolate exponent in MSB
        lsrs    r2, r2, #24
        cmp     r2, #0xff
        bne     L(rhs_finite)
        lsls    r2, r1, #9
        bne     L(unordered)             // Keep off the hot trace

// Both lhs and rhs are either finite or Inf, so they are ordered.
L(rhs_finite):
        movs    r0, #0
        bx      lr

// Unordered arguments: return true
L(unordered):
        movs    r0, #1
        bx      lr

#else

// Most likely is that both operands are ordered, so optimize hot trace
// for that situation.

// Here we want to test for NaN.  NaN is signalled if the exponent of
// the operand is 0xff and the significand is non-zero.
        lsls    r0, r0, #1
        cmp     r0, 0xff000000

// Now do the same set of tests for the rhs.

#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        itt     ls
        lslls   r1, r1, #1
        cmpls   r1, 0xff000000
        cset    r0, hi
        bx      lr
#else
        ittet   ls
        lslls   r1, r1, #1
        cmpls   r1, 0xff000000
        movhi   r0, #1                  // Unordered arguments: return true
        movls   r0, #0                  // Both lhs and rhs are either finite or Inf, so they are ordered.
        bx      lr
#endif

#endif

END_FUNC __aeabi_fcmpun

/*********************************************************************
*
*       __aeabi_dcmpun()
*
*  Function description
*    Unordered, double floating.
*
*  Prototype
*    int __aeabi_dcmpun(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    r0 - One if x or y are NaN; else zero.
*/

#undef L
#define L(label) .L__aeabi_dcmpun_##label

ARM_GLOBAL_FUNC __aeabi_dcmpun

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmp.f64 d0, d1
        vmrs    APSR_nzcv, fpscr
        CSETx   r0, vs
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Most likely is that both operands are ordered, so optimize hot trace
// for that situation.

// Here we want to test for NaN.  NaN is signalled if the exponent of
// the operand is 0x7ff and the significand is non-zero.  If we invert the
// operand, we know that if the exponent is zero then the operand is
// either Inf or NaN.
        lsls    xl, xh, #1              // Discard sign bit, isolate exponent in ms bits
        mvns    xl, xl
        lsrs    xl, xl, #21
        bne     L(lhs_finite)           // No, then not Inf and not NaN, so finite

// Now differentiate Inf from NaN.  If the significand of the lhs is non-zero,
// then we have a NaN input and we know the operands are unordered.  The AEABI
// gives us a dispensation from full-NaN checking and allows us to only test
// the high word of the 64-bit value as it specifies all AEABI NaNs have at
// least one bit set in the high word.
        lsls    xh, xh, #12             // Shift off sign and exponent leaving significand
        bne     L(unordered)            // Keep off the hot trace

// Now do the same set of tests for the rhs.
L(lhs_finite):
        lsls    yl, yh, #1
        mvns    yl, yl
        lsrs    yl, yl, #21
        bne     L(rhs_finite)
        lsls    yh, yh, #12
        bne     L(unordered)            // Keep off the hot trace

// Both lhs and rhs are either finite or Inf, so they are ordered.
L(rhs_finite):
        movs    r0, #0
        bx      lr

// Unordered arguments: return true
L(unordered):
        movs    r0, #1
        bx      lr

#else

// Most likely is that both operands are ordered, so optimize hot trace
// for that situation.

// Here we want to test for NaN.
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        cmp     xl, #0
        it      ne
        orrne   xh, xh, #1
#endif
        lsls    xh, xh, #1              // Discard sign bit, isolate exponent in ms bits
        cmp     xh, #0xffe00000
        bhi     L(unordered)

// Now do the same set of tests for the rhs.
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        cmp     yl, #0
        it      ne
        orrne   yh, yh, #1
#endif
        lsls    yh, yh, #1              // Discard sign bit, isolate exponent in ms bits
        cmp     yh, #0xffe00000

#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
L(unordered):
        cset    r0, hi
        bx      lr
#else
        bhi     L(unordered)

// Both lhs and rhs are either finite or Inf, so they are ordered.
        movs    r0, #0
        bx      lr

// Unordered arguments: return true
L(unordered):
        movs    r0, #1
        bx      lr
#endif

#endif

END_FUNC __aeabi_dcmpun

/*********************************************************************
*
*       __aeabi_cfcmpeq()
*
*  Function description
*    Three-way compare equal, non-excepting, single floating.
*
*  Prototype
*    <Z,C> __aeabi_cfcmpeq(float x, float y);
*
*  Parameters
*    r0 - x - Left-hand operand.
*    r1 - y - Right-hand operand.
*
*  Return value
*    <Z> == 1 only if x, y are ordered and x = y.
*    <C> == 0 only if x, y are ordered and x < y.
*
*  Implementation notes
*    According to the ARM RTABI: "The threee-way, status-returning
*    comparison functions preserve all core registers except ip, lr,
*    and the CPSR."
*/

#undef L
#define L(label) .L__aeabi_cfcmpeq_##label

ARM_GLOBAL_FUNC __aeabi_cfcmpeq

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmp.f32 s0, s1
        vmrs    apsr_nzcv, fpscr
        bx      lr

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

        push    {r0-r3, lr}
        bl      __aeabi_cfcmp_impl
        cmp     r0, #0 // Z flag
        bpl     1f
        movs    r1, #0
        cmn     r0, r1 // C flag
1:
        RET4    r0, r1, r2, r3

#else

        push    {r0-r3, lr}
        bl      __aeabi_cfcmp_impl
        cmp     r0, #0 // Z flag
        bpl     1f
        movs    r1, #0
        cmn     r0, r1 // C flag
1:
        it      mi
        cmnmi   r0, #0 // C flag
        RET4    r0, r1, r2, r3

#endif

#endif

END_FUNC __aeabi_cfcmpeq

/*********************************************************************
*
*       __aeabi_cdcmpeq()
*
*  Function description
*    Three-way compare equal, non-excepting, double floating.
*
*  Prototype
*    <Z,C> __aeabi_cdcmpeq(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    <Z> == 1 only if x, y are ordered and x = y.
*    <C> == 0 only if x, y are ordered and x < y.
*
*  Implementation notes
*    According to the ARM RTABI: "The threee-way, status-returning
*    comparison functions preserve all core registers except ip, lr,
*    and the CPSR."
*/

#undef L
#define L(label) .L__aeabi_cdcmpeq_##label

ARM_GLOBAL_FUNC __aeabi_cdcmpeq

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmp.f64 d0, d1
        vmrs    apsr_nzcv, fpscr
        bx      lr

#else

        push    {r0-r3, lr}
        bl      __aeabi_cdcmp_impl
        cmp     r0, #0 // Z flag
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16
        bpl     1f
        movs    r1, #0
        cmn     r0, r1 // C flag
1:
#else
        it      mi
        cmnmi   r0, #0 // C flag
#endif

        RET4    r0, r1, r2, r3

#endif

END_FUNC __aeabi_cdcmpeq

/*********************************************************************
*
*       __aeabi_cfcmple()
*
*  Function description
*    Three-way compare less or equal, non-excepting, single floating.
*
*  Prototype
*    <Z,C> __aeabi_cfcmple(float x, float y);
*
*  Parameters
*    r0 - x - Left-hand operand.
*    r1 - y - Right-hand operand.
*
*  Return value
*    <Z> == 1 only if x, y are ordered and x = y.
*    <C> == 0 only if x, y are ordered and x < y.
*
*  Implementation notes
*    According to the ARM RTABI: "The threee-way, status-returning
*    comparison functions preserve all core registers except ip, lr,
*    and the CPSR."
*/

#undef L
#define L(label) .L__aeabi_cfcmple_##label

ARM_GLOBAL_FUNC __aeabi_cfcmple

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmpe.f32 s0, s1
        vmrs    apsr_nzcv, fpscr
        bx      lr

#else

        push    {r0-r3, lr}
        bl      __aeabi_cfcmp_impl
        cmp     r0, #0 // Z flag
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16
        bpl     1f
        movs    r1, #0
        cmn     r0, r1 // C flag
1:
#else
        it      mi
        cmnmi   r0, #0 // C flag
#endif

        RET4    r0, r1, r2, r3

#endif

END_FUNC __aeabi_cfcmple

/*********************************************************************
*
*       __aeabi_cdcmple()
*
*  Function description
*    Three-way compare less or equal, non-excepting, double floating.
*
*  Prototype
*    <Z,C> __aeabi_cfcmple(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    <Z> == 1 only if x, y are ordered and x = y.
*    <C> == 0 only if x, y are ordered and x < y.
*
*  Implementation notes
*    According to the ARM RTABI: "The threee-way, status-returning
*    comparison functions preserve all core registers except ip, lr,
*    and the CPSR."
*/

#undef L
#define L(label) .L__aeabi_cdcmple_##label

ARM_GLOBAL_FUNC __aeabi_cdcmple

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmpe.f64 d0, d1
        vmrs    apsr_nzcv, fpscr
        bx      lr

#else

        push    {r0-r3, lr}
        bl      __aeabi_cdcmp_impl
        cmp     r0, #0 // Z flag
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16
        bpl     1f
        movs    r1, #0
        cmn     r0, r1 // C flag
1:
#else
        it      mi
        cmnmi   r0, #0 // C flag
#endif

        RET4    r0, r1, r2, r3

#endif

END_FUNC __aeabi_cdcmple

/*********************************************************************
*
*       __aeabi_cfrcmple()
*
*  Function description
*    Three-way reverse compare less or equal, non-excepting, single floating.
*
*  Prototype
*    <Z,C> __aeabi_cfcmple(float x, float y);
*
*  Parameters
*    r0 - x - Left-hand operand.
*    r1 - y - Right-hand operand.
*
*  Return value
*    <Z> == 1 only if x, y are ordered and x = y.
*    <C> == 0 only if x, y are ordered and y < x.
*
*  Implementation notes
*    According to the ARM RTABI: "The threee-way, status-returning
*    comparison functions preserve all core registers except ip, lr,
*    and the CPSR."
*/

#undef L
#define L(label) .L__aeabi_cfrcmple_##label

ARM_GLOBAL_FUNC __aeabi_cfrcmple

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vmov    s1, r1
        vcmpe.f32 s1, s0
        vmrs    apsr_nzcv, fpscr
        bx      lr

#else

        push    {r0-r3, lr}
        mov     lr, r0
        movs    r0, r1
        mov     r1, lr
        bl      __aeabi_cfcmp_impl
        cmp     r0, #0 // Z flag
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16
        bpl     1f
        movs    r1, #0
        cmn     r0, r1 // C flag
1:
#else
        it      mi
        cmnmi   r0, #0 // C flag
#endif

        RET4    r0, r1, r2, r3

#endif

END_FUNC __aeabi_cfrcmple

/*********************************************************************
*
*       __aeabi_cdrcmple()
*
*  Function description
*    Three-way reverse compare less or equal, non-excepting, double floating.
*
*  Prototype
*    <Z,C> __aeabi_cfcmple(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    <Z> == 1 only if x, y are ordered and x = y.
*    <C> == 0 only if x, y are ordered and y < x.
*
*  Implementation notes
*    According to the ARM RTABI: "The threee-way, status-returning
*    comparison functions preserve all core registers except ip, lr,
*    and the CPSR."
*/

#undef L
#define L(label) .L__aeabi_cdrcmple_##label

ARM_GLOBAL_FUNC __aeabi_cdrcmple

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        vcmp.f64 d1, d0
        vmrs    apsr_nzcv, fpscr
        bx      lr

#else

        push    {r0-r3, lr}
        mov     lr, r0
        movs    r0, r2
        mov     r2, lr
        mov     lr, r1
        movs    r1, r3
        mov     r3, lr
        bl      __aeabi_cdcmp_impl
        cmp     r0, #0 // Z flag
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16
        bpl     1f
        movs    r1, #0
        cmn     r0, r1 // C flag
1:
#else
        it      mi
        cmnmi   r0, #0 // C flag
#endif

        RET4    r0, r1, r2, r3

#endif

END_FUNC __aeabi_cdrcmple

/*********************************************************************
*
*       __aeabi_cfcmp_impl()
*
*  Function description
*    Three-way compare, single floating.
*
*  Prototype
*    int __aeabi_cfcmp_impl(float x, float y);
*
*  Parameters
*    r0 - x - Left-hand operand.
*    r2 - y - Right-hand operand.
*
*  Return value
*    <  0 - x < y.
*    == 0 - x == y
*    >  0 - x > y
*/

#undef L
#define L(label) .L__aeabi_cfcmp_impl_##label

ARM_LOCAL_FUNC __aeabi_cfcmp_impl

#if __SEGGER_RTL_FP_HW >= 1

        fmsr    s0, r0
        fmsr    s1, r1
        fcmps   s0, s1
        fmstat
        beq     L(zflagset)
        bcs     L(cflagset)
        movs    r0, #-1
        bx      lr
L(cflagset):
        movs    r0, #1
        bx      lr
L(zflagset):
        movs    r0, #0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7fffffff) > 0x7f800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xff000000.  The ARM gives us a great way to perform
// this comparison using compare negated, CMN.
        movs    r2, #1
        lsls    r2, r2, #24
        lsls    r3, r0, #1
        cmn     r2, r3                  // Is lhs NaN?
        bhi     L(both_operands_zero)   // Comparison with NaN is invalid so arbitrarily return equal.
        lsls    r3, r1, #1              // If lhs isn't NaN, see if rhs is NaN
        cmn     r2, r3
        bhi     L(both_operands_zero)

// Ok, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// check if the sign bits differ and use two different comparisons.  The only fly in the
// ointment is that -0 and +0 are considered the same when comparing.
        movs    r2, r0
        orrs    r2, r2, r1              // Or together significands, exponents, and signs.
        lsls    r2, r2, #1              // Cast out sign, set Z if both operands are zero irrespective of sign.
        beq     L(both_operands_zero)

// We now have non-zero operands.  Make the distinction between same-signed and
// differing-sign operands.
        bcs     L(different_signs)
        subs    r0, r0, r1
        beq     L(done)
        bcs     L(one)
        sbcs    r0, r0, r0
        bx      lr
L(different_signs):
        subs    r0, r1, r0
        beq     L(done)
        bcs     L(one)
        sbcs    r0, r0, r0
        bx      lr
L(one): movs    r0, #+1
L(done): bx      lr

// Either both operands are zero or one of them is a NaN.
L(both_operands_zero):
        movs    r0, #0
        bx      lr

#else

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7fffffff) > 0x7f800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xff000000.  The ARM gives us a great way to perform
// this comparison using compare negated, CMN.
        mov     r2, #0x1000000
        cmn     r2, r0, lsl #1          // Is lhs NaN?
        it      ls
        cmnls   r2, r1, lsl #1          // If lhs isn't NaN, see if rhs is NaN
        it      hi
        bhi     L(both_operands_zero)   // Comparison with NaN is invalid so arbitrarily return equal.

// Ok, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// check if the sign bits differ and use two different comparisons.  The only fly in the
// ointment is that -0 and +0 are considered the same when comparing.
        orr     r2, r0, r1              // Or together significands, exponents, and signs.
        lsls    r2, r2, #1              // Cast out sign, set Z if both operands are zero irrespective of sign.
        it      eq
        beq     L(both_operands_zero)

// We now have non-zero operands.  Make the distinction between same-signed and
// differing-sign operands.
        bcs     L(different_signs)
        subs    r0, r0, r1
        it      eq
        bxeq    lr
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        movs    r0, #1
        cneg    r0, r0, cc
#else
        ite     cs
        movcs   r0, #+1
        movcc   r0, #-1
#endif
        bx      lr
L(different_signs):
        subs    r0, r1, r0
        it      eq
        bxeq    lr
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        movs    r0, #1
        cneg    r0, r0, cc
#else
        ite     cs
        movcs   r0, #+1
        movcc   r0, #-1
#endif
        bx      lr

// Either both operands are zero or one of them is a NaN.
L(both_operands_zero):
        movs    r0, #0
        bx      lr

#endif

/*********************************************************************
*
*       __aeabi_cdcmp_impl()
*
*  Function description
*    Three-way compare, double floating.
*
*  Prototype
*    int __aeabi_cdcmp_impl(double x, double y);
*
*  Parameters
*    r1:r0 - x - Left-hand operand, hi:lo.
*    r3:r2 - y - Right-hand operand, hi:lo.
*
*  Return value
*    <  0 - x < y.
*    == 0 - x == y
*    >  0 - x > y
*/

#undef L
#define L(label) .L__aeabi_cdcmp_impl_##label

ARM_LOCAL_FUNC __aeabi_cdcmp_impl

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vmov    d1, yl, yh
        fcmpd   d0, d1
        fmstat
        beq     L(zflagset)
        bcs     L(cflagset)
        movs    r0, #-1
        bx lr
L(cflagset):
        movs    r0, #1
        bx lr
L(zflagset):
        movs    r0, #0
        bx lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

#define CMP64_WORKING_SET {r4, r5, r6}

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        push    CMP64_WORKING_SET
        movs    r6, #1
        lsls    r6, #20                 // 0x00100000
        lsls    r4, xh, #1              // cast out sign bits
        lsls    r5, yh, #1
        cmn     r6, r4                  // Is lhs NaN?
        bhi     L(inf_nan)
        cmn     r6, r5                  // Is rhs NaN?
        bhi     L(inf_nan)              // Comparison of Infs or NaNs

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        movs    r6, r5
        orrs    r6, r6, r4
        orrs    r6, r6, xl
        orrs    r6, r6, yl              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(both_operands_equal)

// Weed out equality.
        cmp     xl, yl
        bne     L(compare)
        cmp     xh, yh
        beq     L(both_operands_equal)

// We now have non-zero operands.  Make the distinction between same-signed and
// differing-sign operands.
L(compare):
// Compare sign.  If signs differ, nothing more to do.
        movs    r6, xh
        eors    r6, yh
        bmi     L(not_rhs_sign)

// Compare operands if signs identical.
        cmp     xh, yh
        bne     L(compared)             // If high parts the same, low parts break the tie.
        cmp     xl, yl
L(compared):
        bcc     L(not_rhs_sign)

// Return -1/1 based on the lhs sign.
        asrs    r0, xh, #31
        movs    r6, #1
        orrs    r0, r0, r6
        pop     CMP64_WORKING_SET
        bx      lr

// Return -1/1 based on inverse of the rhs sign.
L(not_rhs_sign):
        asrs    r0, yh, #31
        mvns    r0, r0
        movs    r6, #1
        orrs    r0, r0, r6
        pop     CMP64_WORKING_SET
        bx      lr

// Compress significands; doesn't matter about finites as this doesn't change
// the ordering but allows us to distinuish Infs from NaNs easily.
L(inf_nan):
        movs    r6, #1
        CBZx    xl, 1f
        orrs    xh, xh, r6
1:      CBZx    yl, 2f
        orrs    yh, yh, r6
2:

// Check for either operand being a NaN.
        lsls    r6, r6, #20
        lsls    r4, xh, #1
        cmn     r6, r4                  // Is lhs NaN?
        bhi     L(both_operands_equal)
        lsls    r4, yh, #1
        cmn     r6, r4                  // Or this a NaN?
        bls     L(compare)              // No, must be Inf or finite, so use a regular test.

// Either both operands are zero or one of them is a NaN.
L(both_operands_equal):
        movs    r0, #0
        pop     CMP64_WORKING_SET
        bx      lr

#else

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        mov     ip, #0x00100000
        cmn     ip, xh, lsl #1          // Is lhs NaN?
        it      ls
        cmnls   ip, yh, lsl #1          // If lhs isn't NaN, see if rhs is NaN
        it      hi
        bhi     L(inf_nan)              // Comparison of Infs or NaNs

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        orrs    ip, xl, xh, lsl #1      // Z iff x == +0.0 or -0.0
        it      eq
        orrseq  ip, yl, yh, lsl #1      // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beq     L(both_operands_equal)

// Weed out equality.
        cmp     xl, yl
        itt     eq
        cmpeq   xh, yh
        beq     L(both_operands_equal)

// We now have non-zero operands.  Make the distinction between same-signed and
// differing-sign operands.
L(compare):
        cmn     xl, #0                  // C=0

// Compare sign.  If signs differ, nothing more to do.
        teq     xh, yh

// Compare operands if signs identical.
        it      pl
        cmppl   xh, yh

// If high parts the same, low parts break the tie.
        it      eq
        cmpeq   xl, yl

// Generate result.
        ite     cs
        movcs   r0, xh, asr #31
        mvncc   r0, yh, asr #31
        orr     r0, r0, #1
        bx      lr

// Compress significands; doesn't matter about finites as this doesn't change
// the ordering but allows us to distinuish Infs from NaNs easily.
L(inf_nan):
        tst     xl, xl
        it      ne
        orrne   xh, xh, #1
        tst     yl, yl
        it      ne
        orrne   yh, #1

// Check for either operand being a NaN.
        cmn     ip, xh, lsl #1          // Is lhs NaN?
        itt     ls
        cmnls   ip, yh, lsl #1          // Or this a NaN?
        bls     L(compare)              // No, must be Inf or finite, so use a regular test.

// Either both operands are zero or one of them is a NaN.
L(both_operands_equal):
        movs    r0, #0
        bx      lr

#endif

END_FUNC __aeabi_cdcmp_impl

/*********************************************************************
*
*       __aeabi_f2iz()
*
*  Function description
*    Convert float to int, round to zero.
*
*  Prototype
*    int __aeabi_f2iz(float x);
*
*  Parameters
*    r0 - x - Floating value to convert.
*
*  Return value
*    r0 - Integerized value.
*/

#undef L
#define L(label) .L__aeabi_f2iz_##label

ARM_GLOBAL_FUNC __aeabi_f2iz

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        vcvt.s32.f32 s0, s0
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Extract biased exponent to r1, magnitude to r2.
        lsls    r2, r0, #1
        lsrs    r1, r2, #24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        subs    r1, r1, #0x7f
        bmi     L(zero_result)          // 2^(-1) and below are always zero.

// Pre-load constant.
        movs    r3, #1
        lsls    r3, r3, #31

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        negs    r1, r1
        adds    r1, r1, #0x1f
        ble     L(overflow_result)

// Set carry to sign of operand.
        lsls    r0, r0, #1              // See below (*)
        bcs     L(negative)

// Shift off exponent bits.
        lsls    r0, r0, #7

// Set hidden bit.
        orrs    r0, r0, r3

// Integerize with rounding towards zero.
        lsrs    r0, r0, r1
        bx      lr

L(negative):
// Shift off exponent bits.
        lsls    r0, r0, #7

// Set hidden bit.
        orrs    r0, r0, r3

// Integerize with rounding towards zero.
        lsrs    r0, r0, r1

// Operand sign was negative, negate integer result.
        negs    r0, r0
        bx      lr

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        movs    r0, #0
        bx      lr

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet.
L(overflow_result):
        movs    r1, #0xff
        lsls    r1, r1, #24
        cmp     r2, r1                  // NaN if > 0xff000000
        bls     L(not_nan)
        bics    r0, r0, r3              // If NaN, clear sign bit to truncate to max +ve integer
L(not_nan):
        ands    r0, r0, r3              // Isolate sign and set maximum -ve 32-bit integer
        bmi     L(done)
        mvns    r0, r3                  // If positive, saturate to +ve maximum
L(done):
        bx      lr

#else

// Extract exponent to r1.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        ubfx    r1, r0, #23, #8
#else
        lsls    r1, r0, #1
        lsrs    r1, r1, #24
#endif

// Compute shift count and check for special conditions.  Thumb-2
// honors full shift count in wide instructions.
#if __SEGGER_RTL_CORE_HAS_LONG_SHIFT
        rsbs    r1, r1, #0x9e
        bls     L(overflow_result)
#else
        rsbs    r1, r1, #0x7e
        bhs     L(zero_result)          // 2^(-1) and below are always zero.
        adds    r1, r1, #32
        bls     L(overflow_result)
#endif

// Set hidden bit and test sign of input.
        orrs    r0, r0, #0x800000

// Integerize rounding to zero.
        lsl     r0, r0, #8
#if __SEGGER_RTL_CORE_HAS_LONG_SHIFT
        lsr.w   r0, r0, r1
#else
        lsr     r0, r0, r1
#endif

// Negate if input was negative.
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cneg    r0, r0, mi
#else
        it      mi
        negmi   r0, r0
#endif

// Return.
        bx      lr

#if !__SEGGER_RTL_CORE_HAS_LONG_SHIFT
// Result must be zero.
L(zero_result):
        mov     r0, #0
        bx      lr
#endif

// Argument is outside the integer range so saturate result based on sign.
// NaNs are converted to INT_MAX irrespective of sign.
L(overflow_result):
        lsls    r2, r0, #1
        cmp     r2, #0xff000000         // NaN if > 0xff000000
        it      hi
        bichi   r0, #0x80000000         // If NaN, clear sign bit to truncate to max +ve integer
        ands    r0, r0, #0x80000000     // Isolate sign and set maximum -ve 32-bit integer
        it      pl
        mvnpl   r0, #0x80000000         // If positive, saturate to +ve maximum
        bx      lr
#endif

END_FUNC __aeabi_f2iz

/*********************************************************************
*
*       __aeabi_d2iz()
*
*  Function description
*    Convert double to int, round to zero.
*
*  Prototype
*    int __aeabi_d2iz(double x);
*
*  Parameters
*    r1:r0 - x - Floating value to convert, hi:lo.
*
*  Return value
*    a0 - Integerized value.
*/

#undef L
#define L(label) .L__aeabi_d2iz_##label

ARM_GLOBAL_FUNC __aeabi_d2iz

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vcvt.s32.f64 s0, d0
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Save return address in ip which is spare.
        mov     ip, lr

// Is argument negative?
        adds    r2, xh, xh
        bcs     L(negative)

// Argument is positive: truncate to zero.
        bl      .L__aeabi_d2uiz_fast_entry

// Handily the flags are correctly set when returning from __float64_to_int32.
// If the result is positive, then truncation didn't overflow.
        bpl     L(return)

// Argument is >= 2^31, so saturate to most positive signed integer.
        asrs    r0, r0, #31             // Know r0's msb is 1, this replicates it through all bits
        lsrs    r0, r0, #1              // Put a zero in the top bit
L(return):
        bx      ip

// Argument is negative; use trunc(x) = -trunc(-x) for x < 0.
// However, detect NaN and Inf early and saturate to +ve max.
L(negative):
        movs    r3, #1
        lsls    r3, r3, #21             // 0x00200000
        cmn     r3, r2                  // special?
        bcs     L(nan_inf)              // Handle Inf and NaN specially
L(inf): bl      .L__aeabi_d2uiz_fast_entry

// Overflow occurs if the sign of the result is not the same as the sign
// of the argument.  If the converted value is outside the range which
// we can represent as a 32-bit signed, saturate.
        bpl     L(negate)
        movs    r0, #1
        lsls    r0, r0, #31
L(negate):
        negs    r0, r0
        bx      ip

// Argument is Inf or NaN--distinguish which by significand bits.
L(nan_inf):
        bne     L(nan)
        tst     xl, xl
        beq     L(inf)                  // Handle infinity by passing through regular channels

// It's a NaN, saturate to +ve maximum.  On entry we know r2 has a NaN
// bit pattern shifted left 1 bit to lose the sign and, therefore, has
// bit 31 set.  Use this fact to construct the output.
L(nan):
        asrs    r0, r2, #31             // r0 = 0xffffffff  
        lsrs    r0, r0, #1              // r0 = 0x7fffffff
        bx      lr

#else

// Extract biased exponent to r2, magnitude to r3.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX

        ubfx    r3, xh, #20, #11
        li      r2, 0x3ff+31
        subs    r2, r2, r3
        ble     L(overflow_result)
        cmp     r2, #32
        bhi     L(zero_result)

#else

        lsls    r2, xh, #1              // Use narrow Thumb-2 encoding
        lsrs    r2, r2, #21
        subs    r2, r2, #0x300
        subs    r2, r2, #0x0ff

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        bmi     L(zero_result)          // 2^(-1) and below are always zero.
                                        // This only required when the shift count
                                        // is taken modulo n by the architecture.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        rsbs    r2, r2, #0x1f
        ble     L(overflow_result)

#endif

// Shift off exponent bits.
        lsls    r3, xh, #11
        orr     xl, r3, xl, lsr #21

// Set hidden bit.
        orr     xl, xl, #0x80000000

// Integerize with rounding towards zero.
        lsrs    xl, xl, r2

// If the operand sign was negative, negate integer result.
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cmp     xh, #0
        cneg    r0, xl, mi
#else
        eors    xl, xl, xh, asr #31
        subs    r0, xl, xh, asr #31
#endif
        bx      lr

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        movs    r0, #0                  // Use narrow Thumb-2 encoding
        bx      lr

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet.
L(overflow_result):
        lsls    r3, xh, #1              // Use narrow Thumb-2 encoding
        tst     xl, xl                  // Fold in significand bits from low-order to
        it      ne                      // ensure that a NaN is correctly detected (if high significand bits are zero)
        orrne   r3, r3, #1
        cmn     r3, #0x00200000         // NaN if > 0xffe000000
        it      hi
        bichi   xh, #0x80000000         // If NaN, clear sign bit to truncate to max +ve integer
        ands    r0, xh, #0x80000000     // Isolate sign and set maximum -ve 32-bit integer
        it      pl
        mvnpl   r0, #0x80000000         // If positive, saturate to +ve maximum
        bx      lr

#endif

END_FUNC __aeabi_d2iz

#undef L
#define L(label) .L__aeabi_f2lz_##label

/*********************************************************************
*
*       __aeabi_f2lz()
*
*  Function description
*    Convert float to long long, round to zero.
*
*  Prototype
*    long long __aeabi_f2lz(float x);
*
*  Parameters
*    r0 - x - Floating value to convert.
*
*  Return value
*    r1:r0 - Integerized value, hi:lo.
*/

ARM_GLOBAL_FUNC __aeabi_f2lz

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Extract biased exponent to r2, magnitude to r3.
        lsls    r3, r0, #1
        lsrs    r2, r3, #24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        subs    r2, r2, #0x7f
        bmi     L(zero_result)          // 2^(-1) and below are always zero.

// Break shifting into two sets: one shifts by less than 32 places, the other by more.
        cmp     r2, #32
        bge     L(long_shift)

// Pre-load constant.
        movs    r3, #1
        lsls    r3, r3, #31

// Value is less than 2^32 so shift is by fewer than 32 bits.
// Compute number of bits to shift significand by to align correctly.
        negs    r2, r2
        adds    r2, r2, #0x1f

// Set carry to sign of operand.
        lsls    r0, r0, #1
        bcs     L(negative_0)

// Shift off exponent bits.
        lsls    r0, r0, #7

// Set hidden bit.
        orrs    r0, r0, r3

// Integerize with rounding towards zero.
        movs    zl, r0
        lsrs    zl, r2

// Set the high 32 bits of the result correctly.
        movs    zh, #0                  // +ve, zero extend
        bx      lr

// Shift off exponent bits.
L(negative_0):
        lsls    r0, r0, #7

// Set hidden bit.
        orrs    r0, r0, r3

// Integerize with rounding towards zero.
        movs    zl, r0
        lsrs    zl, r2

// Set the high 32 bits of the result correctly.
        negs    zl, zl                  // -ve, negate result
        movs    zh, 1                   // sign extend
        negs    zh, zh
        bx      lr

// Value is >= 2^32 so shift is by 32 bits or more.
// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
L(long_shift):
        negs    r2, r2
        adds    r2, r2, #0x3f
        ble     L(overflow_result)

// Pre-load constant.
        movs    r3, #1
        lsls    r3, r3, #31

// Set carry to sign of operand.
        lsls    zh, r0, #1              // See below (*)
        bcs     L(negative_1)

// Shift off exponent bits.
        lsls    zh, zh, #7

// Set hidden bit.
        orrs    zh, zh, r3

// Integerize with rounding towards zero.
        movs    zl, zh
        rors    zl, zl, r2
        lsrs    zh, zh, r2
        eors    zl, zl, zh              // Remove bits that should be shifted out

// Return as operand was positive.
        bx      lr

// Shift off exponent bits.
L(negative_1):
        lsls    zh, zh, #7

// Set hidden bit.
        orrs    zh, zh, r3

// Integerize with rounding towards zero.
        movs    zl, zh
        rors    zl, zl, r2
        lsrs    zh, zh, r2
        eors    zl, zl, zh              // Remove bits that should be shifted out

// Negate result and return.
        movs    r3, #0
        negs    zh, zh
        negs    zl, zl
        sbcs    zh, zh, r3
        bx      lr

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        movs    zl, #0x0                // Use narrow Thumb-2 encoding
#if __SEGGER_RTL_OPTIMIZE <= -1
        b       L(copy_lo_to_hi)
#else
        movs    zh, #0x0                // Use narrow Thumb-2 encoding
        bx      lr
#endif

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet.
L(overflow_result):
        movs    r2, #0xff
        lsls    r2, r2, #24
        cmp     r3, r2                  // NaN if > 0xff000000
        bls     L(not_nan)
        lsls    r0, r0, #1              // If NaN, clear sign bit to truncate to max +ve 64-bit integer
        lsrs    r0, r0, #1
L(not_nan):
        lsrs    zh, r0, #31
        movs    zl, #0                  // Isolate sign and set maximum -ve 64-bit integer
        lsls    zh, zh, #31
        bmi     L(done)
        subs    zl, zl, #1              // If positive, saturate to +ve maximum
L(copy_lo_to_hi):
        lsrs    zh, zl, #1
L(done):
        bx      lr

#else

// Extract biased exponent to r2, magnitude to r3.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        ubfx    r2, r0, #23, #8
#else
        lsls    r2, r0, #1
        lsrs    r2, r2, #24
#endif

// Adjust exponent to shift count for smaller numnbers less than 2^32.
        rsbs    r2, r2, #0x7f+31

// Break shifting into two sets: one shifts by less than 32 places, the other by more.
        cmp     r2, #32
        bcs     L(long_shift)

// Set carry to sign of operand.
        lsls    r0, r0, #1             // See below (*)

// Shift off exponent bits.
        lsl     r0, r0, #7

// Set hidden bit.
        orr     r0, r0, #0x80000000

// Integerize with rounding towards zero.
        lsr     zl, r0, r2

// Set the high 32 bits of the result correctly.
        eors    zh, zh, zh              // Set to zero, do not affect carry
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cneg    zl, zl, cs
        cinv    zh, zh, cs
#else
        itt     cs
        negcs   zl, zl                  // -ve, negate result...
        mvncs   zh, zh                  // ...and sign extend
#endif
        bx      lr

// Value is >= 2^32 so shift is by 32 bits or more.
// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
L(long_shift):
        adds    r2, r2, #32
        bls     L(overflow_or_zero)

// Set carry to sign of operand.
        lsls    zh, r0, #1              // See below (*)

// Shift off exponent bits.
        lsl     zh, zh, #7

// Set hidden bit.
        orr     zh, zh, #0x80000000

// Integerize with rounding towards zero.
        ror     zl, zh, r2
        lsr     zh, zh, r2
        eor     zl, zl, zh              // Remove bits that should be shifted out

// Return if operand was positive. (*)
        it      cc
        bxcc    lr

// Negate result and return.
        rsbs    zl, zl, #0
        sbc     zh, zh, zh, lsl #1
        bx      lr

L(overflow_or_zero):
        ble     L(overflow_result)

// Return zero, magnitude of argument is smaller than 1.0.
#if __SEGGER_RTL_CORE_HAS_CLRM
        clrm    {zl, zh}
#else
        movs    zl, #0x0                // Use narrow Thumb-2 encoding
        movs    zh, #0x0                // Use narrow Thumb-2 encoding
#endif
        bx      lr

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet.
L(overflow_result):
        lsls    r3, r0, #1
        cmp     r3, #0xff000000         // NaN if > 0xff000000
        it      hi
        bichi   r0, r0, #0x80000000     // If NaN, clear sign bit to truncate to max +ve 64-bit integer
        ands    zh, r0, #0x80000000     // Isolate sign and set maximum -ve 64-bit integer
        mov     zl, #0
        itt     pl
        mvnpl   zh, #0x80000000         // If positive, saturate to +ve maximum
        mvnpl   zl, #0
        bx      lr

#endif

END_FUNC __aeabi_f2lz

/*********************************************************************
*
*       __aeabi_d2lz()
*
*  Function description
*    Convert double to long long, round to zero.
*
*  Prototype
*    long long __aeabi_d2lz(double x);
*
*  Parameters
*    r1:r0 - x - Floating value to convert, hi:lo.
*
*  Return value
*    r1:r0 - Integerized value, hi:lo.
*/

#undef L
#define L(label) .L__aeabi_d2lz_##label

ARM_GLOBAL_FUNC __aeabi_d2lz

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Extract biased exponent to r2, magnitude to r3.
        lsls    r2, xh, #1
        lsrs    r2, r2, #21

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        movs    r3, #1
        lsls    r3, r3, #10             // r3 = 0x400
        subs    r2, r2, r3              // r2 = r2 - 0x400...
        adds    r2, r2, #1              // r2 = r2 - 0x3ff
        bmi     L(zero_result)          // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        negs    r2, r2
        adds    r2, r2, 0x3f
        ble     L(overflow_result)

// Save sign of operand in ip.
        mov     ip, xh

// Shift off exponent bits.
        lsls    xh, xh, #11
        lsrs    r3, xl, #21
        orrs    xh, xh, r3
        lsls    xl, xl, #11

// Set hidden bit.
        movs    r3, #1
        lsls    r3, r3, #31
        orrs    xh, xh, r3

// Quickly shift by 32 bits if needed.
        cmp     r2, #32
        bcc     L(full_shift)
        movs    xl, xh
        movs    xh, #0
        subs    r2, r2, #32

// Integerize with rounding towards zero.
L(full_shift):
        rors    xh, xh, r2
        lsrs    xl, xl, r2
        eors    xl, xl, xh
        lsls    xh, xh, r2
        lsrs    xh, xh, r2
        eors    xl, xl, xh

// Conditionally negate.
        mov     r3, ip
        tst     r3, r3
        bpl     L(positive)

// Form magnitude of argument by negating it.
        movs    r3, #0                  // 64-bit negation, Thumb-style.
        negs    xl, xl
        sbcs    r3, r3, xh
        movs    xh, r3
L(positive):
        bx      lr

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        movs    xl, #0
        movs    xh, #0
        bx      lr

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet or signed or not.
L(overflow_result):
        CBZx    xl, L(no_fold)          // Fold in significand bits from low-order
        movs    r2, #1
        orrs    xh, xh, r2              // ensure that a NaN is correctly detected (if high significand bits are zero)
L(no_fold):
        lsls    r3, xh, #1
        movs    r2, #1
        lsls    r2, r2, #21
        cmn     r3, r2                  // NaN if > 0xffe000000
        bls     L(not_nan)
        lsrs    xh, r3, #1              // If NaN, clear sign bit to truncate to max +ve integer
L(not_nan):
        movs    xl, #0
        lsrs    xh, xh, #31             // Isolate sign and set maximum -ve 32-bit integer
        lsls    xh, xh, #31
        bmi     L(done)
        mvns    xl, xl                  // If positive, saturate to +ve maximum
        lsrs    xh, xl, #1
L(done):
        bx      lr

#else

// Extract biased exponent to r2.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        ubfx    r2, xh, #20, #11
#else
        lsls    r2, xh, #1
        lsrs    r2, r2, #21
#endif

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      r3, 0x3ff+0x1f
        subs    r2, r3, r2
        bge     L(small_shift)

// Quickly shift by 32 bits if needed.
        adds    r2, r2, #32
        ble     L(overflow_result)

// Save sign of operand.
        asrs    r3, xh, #31

// Shift off exponent bits.
        lsls    xh, xh, #11
        orr     xh, xh, xl, lsr #21
        lsls    xl, xl, #11

// Set hidden bit.
        orr     xh, xh, #0x80000000

// Integerize with rounding towards zero.
        rors    xh, xh, r2
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        eor     xl, xh, xl, lsr r2
#else
        lsrs    xl, xl, r2
        eors    xl, xl, xh
#endif
        lsls    xh, xh, r2
        lsrs    xh, xh, r2
        eors    xl, xl, xh

// If the operand sign was positive we're done.
        adds    r3, r3, #1
#if __SEGGER_RTL_OPTIMIZE < 0
        CBNZx   r3, L(done)
#else
        it      ne
        bxne    lr
#endif

// Form magnitude of argument by negating it.
        negs    xl, xl
        sbc     xh, r3, xh
        bx      lr

// Save sign of operand.
L(small_shift):
        asrs    r3, xh, #31

// Shift off exponent bits.
        lsls    xh, xh, #11
        orr     xh, xh, xl, lsr #21
        lsls    xl, xl, #11

// Set hidden bit.
        orr     xh, xh, #0x80000000

// Limit shift count.
        cmp     r2, #32
        it      cs
        movcs   r2, #32

// Integerize with shift of 32 bits or more to small value with rounding towards zero.
        lsrs    xl, xh, r2
        movs    xh, r3

// Return if positive.
        it      eq
        bxeq    lr

// Form magnitude of argument by negating it.
        negs    xl, xl
        sbcs    xh, xh, xh
L(done):
        bx      lr

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet.
L(overflow_result):
        lsls    r3, xh, #1              // Use narrow Thumb-2 encoding
        tst     xl, xl                  // Fold in significand bits from low-order to
        it      ne
        orrne   r3, r3, #1              // ensure that a NaN is correctly detected (if high significand bits are zero)
        cmp     r3, #0xffe00000         // NaN if > 0xffe000000
        it      hi
        bichi   xh, #0x80000000         // If NaN, clear sign bit to truncate to max +ve integer
        movs    xl, #0
        ands    xh, xh, #0x80000000     // Isolate sign and set maximum -ve 32-bit integer
        itt     pl
        mvnpl   xh, #0x80000000         // If positive, saturate to +ve maximum
        mvnpl   xl, xl
        bx      lr

#endif

END_FUNC __aeabi_d2lz

/*********************************************************************
*
*       __aeabi_f2uiz()
*
*  Function description
*    Convert float to unsigned, round to zero.
*
*  Prototype
*    unsigned __aeabi_f2uiz(float x);
*
*  Parameters
*    r0 - x - Floating value to convert.
*
*  Return value
*    r0 - Integerized value.
*/

#undef L
#define L(label) .L__aeabi_f2uiz_##label

ARM_GLOBAL_FUNC __aeabi_f2uiz

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        ftouizs s0, s0
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Move off sign.  If negative (not the common case) truncate to 0.
        lsls    r1, r0, #1
        bcs     L(zero_result)

// Extract exponent.
        lsrs    r1, r1, #24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        subs    r1, r1, #0x7f
        bmi     L(zero_result)          // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
        negs    r1, r1
        adds    r1, r1, #0x1f
        bmi     L(max_result)

// Shift off exponent bits.
        lsls    r0, r0, #8

// Set hidden bit.
        movs    r2, #1
        lsls    r2, r2, #31
        orrs    r0, r0, r2

// Integerize with rounding towards zero and return.
        lsrs    r0, r0, r1              // Truncate argument (Use narrow Thumb-2 encoding)
        bx      lr

// If argument is >=2^32, saturate to MAX_UINT
L(max_result):
        asrs    r0, r1, #31             // Know r1<0
        bx      lr

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        movs    r0, #0
        bx      lr

#elif __SEGGER_RTL_CORE_HAS_LONG_SHIFT

// Extract exponent and sign together so we can test simultaneously.
        lsrs    r1, r0, #23             // Use narrow Thumb-2 encoding

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        rsbs    r1, r1, #0x7f+32-1
        blt     L(negative_or_overflow) // Result is -ve, or the exponent is small, or the exponent is large

// Shift off exponent bits.
        lsls    r0, r0, #8              // Use narrow Thumb-2 encoding

// Set hidden bit.
        orr     r0, r0, #0x80000000

// Integerize with rounding towards zero and return.
        lsr.w   r0, r0, r1              // Truncate argument.  Here we use a WIDE ENCODING
                                        // when a narrow one seems obvious.  This is because
                                        // the shift count is LARGER than 32 and we require
                                        // the total shift count to be used, not taken modulo 32.
        bx      lr

// Exceptional exponent or sign: either -ve or very large.
L(negative_or_overflow):
        lsls    r0, r0, #1              // Set C to sign bit
        eors    r0, r0, r0              // r0 := 0 without affecting C and pre-set -ve inputs to zero
        it      cc
        subcc   r0, r0, #1              // If argument is >=2^32, saturate to MAX_UINT
        bx      lr

#else

// Extract exponent and sign.
        lsrs    r1, r0, #23             // Use narrow Thumb-2 encoding

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        rsbs    r1, r1, #0x7f+32-1
        blt     L(negative_or_overflow) // Result is -ve, or the exponent is small, or the exponent is large

// Shift off exponent bits.
        lsls    r0, r0, #8              // Use narrow Thumb-2 encoding

// Set hidden bit.
        orr     r0, r0, #0x80000000

// Non-thumb-2 targets do not honor entire shift count...
        cmp     r1, #32
        it      cs
        movcs   r0, #0

// Integerize with rounding towards zero and return.
        lsrs    r0, r0, r1              // Truncate argument (Use narrow Thumb-2 encoding)
        bx      lr

// Exceptional exponent or sign: either -ve or very large.
L(negative_or_overflow):
        lsls    r0, r0, #1              // Set C to sign bit
        eors    r0, r0, r0              // r0 := 0 without affecting C and pre-set -ve inputs to zero
        it      cc
        subcc   r0, r0, #1              // If argument is >=2^32, saturate to MAX_UINT
        bx      lr

#endif

END_FUNC __aeabi_f2uiz

/*********************************************************************
*
*       __aeabi_d2uiz()
*
*  Function description
*    Convert double to unsigned, round to zero.
*
*  Prototype
*    unsigned __aeabi_d2uiz(double x);
*
*  Parameters
*    r1:r0 - x - Floating value to convert, hi:lo.
*
*  Return value
*    a0 - Integerized value.
*/

#undef L
#define L(label) .L__aeabi_d2uiz_##label

ARM_GLOBAL_FUNC __aeabi_d2uiz

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        ftouizd s0, d0
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Move off sign.  If negative (not the common case) truncate to 0.
        lsls    r2, xh, #1
        bcs     L(zero_result)

// Extract biased exponent to r2.
LOCAL_ENTRY .L__aeabi_d2uiz_fast_entry
        lsrs    r2, r2, #21

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        movs    r3, #0x03               // subtract 0x3ff
        lsls    r3, r3, #8              // 0x400
        adds    r3, r3, #0xFF           // 0x3ff
        subs    r2, r2, r3
        bmi     L(zero_result)          // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        negs    r2, r2
        adds    r2, #31
        bmi     L(overflow_result)

// Shift off exponent bits.
        lsrs    xl, xl, #21
        lsls    xh, xh, #11
        orrs    xl, xl, xh

// Set assumed bit.
        movs    r3, #1
        lsls    r3, r3, #31
        orrs    xl, xl, r3

// Integerize with rounding towards zero.
#if __SEGGER_RTL_BYTE_ORDER < 0
        lsrs    r0, xl, r2
#else
        movs    r0, xl
        lsrs    r0, r0, r2
#endif
        bx      lr

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        movs    r0, #0
        bx      lr

// Argument is outside the integer range so saturate result.
L(overflow_result):
        asrs    r0, r2, #31             // Know r2<0
        bx      lr

#else

// Extract biased exponent to r2, magnitude to r3.
        lsrs    r3, xh, #20
        li      r2, 0x3ff+31
        subs    r2, r2, r3
        blt     L(overflow_result)

// Shift off exponent bits.
        lsls    r3, xh, #11
        orr     xl, r3, xl, lsr #21

// Set hidden bit.
        orr     xl, xl, #0x80000000

// Integerize with rounding towards zero.
        cmp     r2, #32                 // Limit shift count for cores that do not honor full shift count.
        it      cs
        movcs   r2, #32
        lsrs    r0, xl, r2
        bx      lr

// Argument is outside the integer range so saturate result based on sign.
L(overflow_result):
        lsls    xh, xh, #1
        sbcs    r0, r0, r0
        bx      lr

#endif

END_FUNC __aeabi_d2uiz

/*********************************************************************
*
*       __aeabi_f2ulz()
*
*  Function description
*    Convert float to unsigned long long, round to zero.
*
*  Prototype
*    unsigned long long __aeabi_f2ulz(float x);
*
*  Parameters
*    r0 - x - Floating value to convert.
*
*  Return value
*    r1:r0 - Integerized value, hi:lo.
*/

#undef L
#define L(label) .L__aeabi_f2ulz_##label

ARM_GLOBAL_FUNC __aeabi_f2ulz

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Move off sign.  If negative (not the common case) truncate to 0.
        lsls    r2, r0, #1
        bcs     L(zero_result)

// Extract biased exponent to r2.
        lsrs    r2, r2, #24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        subs    r2, r2, #0x7f
        bmi     L(zero_result)          // (1,2]*2^(-1) and below are truncated to zero.

// Pre-load constant 0x80000000.
        movs    r3, #0x80
        lsls    r3, r3, #24

// Break shifting into two sets: one shifts by less than 32 places, the other by more.
        cmp     r2, #32
        bge     L(long_shift)

// Value is less than 2^32 so shift is by fewer than 32 bits.
// Compute number of bits to shift significand by to align correctly.
        negs    r2, r2
        adds    r2, r2, #0x1f

// Shift off exponent bits.
        lsls    r0, r0, #8

// Set hidden bit.
        orrs    r0, r0, r3

// Integerize with rounding towards zero.
#if __SEGGER_RTL_BYTE_ORDER < 0
        lsrs    zl, r0, r2
#else
        movs    zl, r0
        lsrs    zl, zl, r2
#endif

// Set the high 32 bits of the result and return.
        movs    zh, #0
        bx      lr

// Value is >= 2^32 so shift is by 32 bits or more.
// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
L(long_shift):
        negs    r2, r2
        adds    r2, r2, #0x3f
        blt     L(overflow_result)

// Shift off exponent bits.
        lsls    zh, r0, #8

// Set hidden bit.
        orrs    zh, zh, r3

// Integerize with rounding towards zero.
        movs    zl, zh
        rors    zl, zl, r2
        lsrs    zh, zh, r2
        eors    zl, zl, zh              // Remove bits that should be shifted out
        bx      lr

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        movs    zl, #0
#if __SEGGER_RTL_OPTIMIZE <= -1
        b       L(copy_to_high)
#else
        movs    zh, zl
        bx      lr
#endif

// Argument is outside the integer range so saturate result based on sign.
L(overflow_result):
        asrs    zl, r2, #31             // Know r2<0, saturate to +ve maximum
L(copy_to_high):
        movs    zh, zl
        bx      lr

#else

// Move off sign.  If negative (not the common case) truncate to 0.
        lsls    r2, r0, #1
        bcs     L(zero_result)

// Extract biased exponent to r2.
        lsrs    r2, r2, #24

// Adjust exponent to shift count for smaller numnbers less than 2^32.
        rsbs    r2, r2, #0x7f+0x1f

// If larger than 2^32, it's either too small or is a larger shift.
        cmp     r2, #32
        bcs     L(long_shift)

// Value is less than 2^32 so shift is by fewer than 32 bits.
// Compute number of bits to shift significand by to align correctly.

// Shift off exponent bits.
        lsls    r0, r0, #8              // Use narrow Thumb-2 encoding

// Set hidden bit.
        orr     r0, r0, #0x80000000

// Integerize with rounding towards zero.
        lsrs    zl, r0, r2              // Use narrow Thumb-2 encoding

// Set the high 32 bits of the result and return.
        movs    zh, #0                  // Use narrow Thumb-2 encoding
        bx      lr

// Value is >= 2^32 so shift is by 32 bits or more.
// Compute number of bits to shift significand by to align correctly.
L(long_shift):
        adds    r2, r2, #32
        bcc     L(overflow_or_zero)

// Shift off exponent bits.
        lsls    zh, r0, #8

// Set hidden bit.
        orr     zh, zh, #0x80000000

// Integerize with rounding towards zero.
        ror     zl, zh, r2
        lsrs    zh, zh, r2
        eors    zl, zl, zh              // Remove bits that should be shifted out
        bx      lr

L(overflow_or_zero):
        ble     L(overflow_result)

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
#if __SEGGER_RTL_CORE_HAS_CLRM
        clrm    {zl, zh}
#else
        movs    zl, #0
        movs    zh, #0
#endif
        bx      lr

// Argument is outside the integer range so saturate result based on sign.
L(overflow_result):
        mvn     zl, #0                  // Saturate to +ve maximum
        mov     zh, zl
        bx      lr

#endif

END_FUNC __aeabi_f2ulz

/*********************************************************************
*
*       __aeabi_d2ulz()
*
*  Function description
*    Convert double to unsigned long long, round to zero.
*
*  Prototype
*    unsigned long long __aeabi_d2ulz(double x);
*
*  Parameters
*    r1:r0 - x - Floating value to convert, hi:lo.
*
*  Return value
*    r1:r0 - Integerized value, hi:lo.
*/

#undef L
#define L(label) .L__aeabi_d2ulz_##label

ARM_GLOBAL_FUNC __aeabi_d2ulz

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Move off sign.  If negative (not the common case) truncate to 0.
        lsls    r3, xh, #1
        bcs     L(zero_result)

// Extract biased exponent to r2, magnitude to r3.
        lsrs    r2, r3, #21

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        movs    r3, #1
        lsls    r3, r3, #10             // r3 = 0x400
        subs    r2, r2, r3              // r2 = r2 - 0x400...
        adds    r2, r2, #1              // r2 = r2 - 0x3ff
        bmi     L(zero_result)          // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        negs    r2, r2
        adds    r2, r2, #0x3f
        blt     L(overflow_result)

// Shift off exponent bits.
        lsls    xh, xh, #11
        lsrs    r3, xl, #21
        orrs    xh, xh, r3
        lsls    xl, xl, #11

// Set hidden bit.
        movs    r3, #1
        lsls    r3, r3, #31
        orrs    xh, xh, r3

// Quickly shift by 32 bits if needed.
        cmp     r2, #32
        bcc     1f
        movs    xl, xh
        movs    xh, #0
        subs    r2, r2, #32

// Integerize with rounding towards zero.
1:      rors    xh, xh, r2
        lsrs    xl, xl, r2
        eors    xl, xl, xh
        lsls    xh, xh, r2
        lsrs    xh, xh, r2
        eors    xl, xl, xh
        bx      lr

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        movs    xl, #0
#if __SEGGER_RTL_OPTIMIZE <= -1
        b       L(copy_to_high)
#else
        movs    xh, #0
        bx      lr
#endif

// Argument is outside the integer range so saturate to maximum.
L(overflow_result):
        asrs    xl, r2, #31             // Know r2<0, set to -1
L(copy_to_high):
        movs    xh, xl
        bx      lr

#else

// Extract biased exponent to r2, drag sign into the mix.
        asrs    r2, xh, #20

// Shift off exponent bits.
        lsls    xh, xh, #11
        orr     xh, xh, xl, lsr #21
        lsls    xl, xl, #11

// Set hidden bit.
        orr     xh, xh, #0x80000000

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      r3, 0x3ff+0x1f
        subs    r2, r3, r2
        bge     L(small_shift)

// Quickly shift by 32 bits if needed.
        adds    r2, r2, #32
        bmi     L(overflow_result)

// Integerize full 64-bit value rounding towards zero.
        rors    xh, xh, r2
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        eors    xl, xh, xl, lsr r2
#else
        lsrs    xl, xl, r2
        eors    xl, xl, xh
#endif
        lsls    xh, xh, r2
        lsrs    xh, xh, r2
        eors    xl, xl, xh
        bx      lr

// Integerize with shift of 32 bits or more to small value with rounding towards zero.
L(small_shift):
        cmp     r2, #32                 // Limit shift count.
        it      cs
        movcs   r2, #32
        lsrs    xl, xh, r2
        movs    xh, #0
        bx      lr

// Argument is outside the integer range so saturate to maximum.
L(overflow_result):
        mvn     xh, #0
        mov     xl, xh                  // More compact than mvn in Thumb-2
        bx      lr

#endif

END_FUNC __aeabi_d2ulz

/*********************************************************************
*
*       __aeabi_i2f()
*
*  Function description
*    Convert int to float.
*
*  Prototype
*    float __aeabi_i2f(int x);
*
*  Parameters
*    r0 - x - Integer value to convert.
*
*  Return value
*    r0 - Floating value.
*/

#undef L
#define L(label) .L__aeabi_i2f_##label

ARM_GLOBAL_FUNC __aeabi_i2f

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        fsitos  s0, s0
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Move sign bit of operand into place just above exponent work area.
        lsrs    r2, r0, #31
        lsls    r2, r2, #8

// Set r2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the binary point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// of one here.
        adds    r2, r2, #31+0x7f-1

// Tests the sign bit and duplicates argument into r1 and r3.
        movs    r1, r0

// Floating 0 to 0.0 is easy.  Also tests the sign bit and duplicates argument
// into r1.
        beq     L(done)

// If argument is positive, use common float code.
        bpl     L(normalize)

// float(x), x<0 === -float(|x|)
        negs    r1, r1

// Test for degenerate case 0x80000000.
        bmi     L(normalized)

// Iteratively normalize; anything faster turns out rather ugly.
L(normalize):
        subs    r2, r2, #1
        lsls    r1, r1, #1
        bpl     L(normalize)

// Move exponent into final position.
L(normalized):
        lsls    r2, r2, #23

// Move significand into position.
        lsrs    r0, r1, #8

// Scrub off upper bits of significand and prepare to round
// on lower order bits only.
        lsls    r1, r1, #24

// 0x rounds down, 1 may need to round up or down...
        adcs    r1, r1, r1
        bcc     L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        cmp     r1, #1

// Insert exponent, sign, and break-tie flag.  Overflow gets nicely handled
L(round_down):
        adcs    r0, r0, r2
L(done):
        bx      lr

#elif __SEGGER_RTL_CORE_HAS_CLZ

// Note: using cbz to pre-test input being zero results in a 1-cycle
// penalty, so defer this test to after the normalization, the result
// of which can only be zero if the input is zero. You lean something
// new about the core each and every time you write some Arm code...

// Stow sign and form absolute if required.
        ands    r1, r0, #0x80000000
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cneg    r0, r0, ne
#else
        it      ne
        negne   r0, r0                  // Form magnitude.
#endif

// Normalization.  Do this by counting leading zeroes courtesy of CLZ.
        clz     r2, r0
        lsls    r3, r0, r2
        beq     L(done)                 // Exactly 0.0
        rsbs    r2, r2, #31+0x7f-1      // -1 to compensate for hidden bit

// Move exponent into position and combine with sign.
        orr     r1, r1, r2, lsl #23

#if __SEGGER_RTL_OPTIMIZE < 0

// This is the smaller option.

// Round result.
        lsls    r2, r3, #24             // Set carry bit to R0.7 and Z=1 if value stored is zero
        sbcs    r2, r2, #0x80000000
        adc     r0, r1, r3, lsr #8      // Must be add (not orr) because hidden bit is removed by exponent adjustment
L(done):
        bx      lr

#else

// This is the (marginally) faster option...

// Combine sign+exponent (r1) and normalized significand (r3).
        add     r0, r1, r3, lsr #8      // Must be add (not orr) because hidden bit is removed by exponent adjustment

// Round result.
        lsls    r3, r3, #25             // Set carry bit to R0.7 and Z=1 if value stored is zero
        it      cc
        bxcc    lr                      // No rounding required
        add     r0, r0, #1              // Round up
        it      ne
        bxne    lr
        bic     r0, r0, #1
L(done):
        bx      lr

// It would appear that you could be even faster by hoisting the add and folding it into an adc like this:
//
//      lsls    r2, r3, #25             // Set carry bit to R0.7 and Z=1 if value stored is zero
//      adc     r0, r1, r3, lsr #8      // Must be add (nor orr) because hidden bit is removed by exponent adjustment
//      it      cc
//      bxcc    lr                      // No rounding required
//      it      eq                      // Break tie if exactly 0.5
//      biceq   r0, r0, #1
//      bx      lr
//
// ...but this executes slower on the CM4.  It must be that the lsl followed immediately by
// a predicated instruction based on C is able to execute faster than when they are separated.

#endif

#else

// Set r2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the decimal point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// by one here.
        movs    r2, #31+0x7f-1

// Floating 0 to 0.0 is easy.  Also tests the sign bit.
        cmp     r0, #0
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        bxeq    lr
#else
        beq     L(done)
#endif

// Preserve sign of argument (only) in r2.8 which adjoins the eight exponent bits.
// Form magnitude of argument.
        itt     mi
        negmi   r0, r0
        orrmi   r2, #0x100              // preserve sign in exponent register

// Normalization.
        NORM32D r0, r0, r2, r3          // r0 is normalized, r2 decremented by number of shifts required to normalize, r3 is a temporary in case of CLZ

// Move exponent into position.
        lsls    r2, r2, #23

// Round result.
        lsls    r1, r0, #24             // Set carry bit to R0.7 and Z=1 if value stored is zero
        sbcs    r1, r1, #0x80000000
        adc     r0, r2, r0, lsr #8      // Must be add (not orr) because hidden bit is removed by exponent adjustment
L(done):
        bx      lr

#endif

END_FUNC __aeabi_i2f

/*********************************************************************
*
*       __aeabi_i2d()
*
*  Function description
*    Convert int to float.
*
*  Prototype
*    double __aeabi_i2d(int x);
*
*  Parameters
*    r0 - x - Integer value to convert.
*
*  Return value
*    r1:r0 - Floating value, hi:lo.
*/

#undef L
#define L(label) .L__aeabi_i2d_##label

ARM_GLOBAL_FUNC __aeabi_i2d

#if __SEGGER_RTL_FP_HW >= 2

        vmov    s0, r0
        fsitod  d0, s0
        vmov    zl, zh, d0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Float positive argument.
        la      r3, __aeabi_ui2d

// Negative argument?
        tst     r0, r0
        bpl     L(positive)

// float(x), x<0 === -float(-x)
        negs    r0, r0
        push    {lr}
        BLXx    __aeabi_ui2d, r3
        pop     {r3}

// Negate result
        movs    r2, #1
        lsls    r2, r2, #31
        orrs    zh, zh, r2

// All done.
L(positive):
        bx      r3

#else

// Floating 0 to 0.0 is easy, as long as we have initialized r1...
        ands    r1, r0, #0x80000000
        CBZx    r0, L(zero)

// Form magnitude of argument.
#if __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV
        cneg    r0, r0, mi
#else
        it      mi
        negmi   r0, r0
#endif

// Set r2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the decimal point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
        lsrs    r2, r1, #20             // Move sign bit to adjoin exponent
#if __SEGGER_RTL_CORE_HAS_ADDW_SUBW
        add     r2, r2, #0x41d
#else
        add     r2, r2, #0x400          // Subtract an extra '1' as this compensates
        add     r2, r2, #0x01d          // for the '1' added by the hidden bit which
                                        // we do not remove.  41D = 31+0x3ff-1
#endif

// Normalization.
        NORM32D r0, r0, r2, r3          // r0 is normalized, r2 decremented by number of shifts required to normalize, r3 is a temnporary in case of CLZ

// Pack signficand and exponent.
        lsrs    r3, r0, #11             // Use narrow Thumb-2 encoding
        lsls    zl, r0, #21             // Use narrow Thumb-2 encoding
        add     zh, r3, r2, lsl #20     // Insert sign and exponent (see above)
L(zero):
        bx      lr

#endif

END_FUNC __aeabi_i2d

/*********************************************************************
*
*       __aeabi_l2f()
*
*  Function description
*    Convert long long to float.
*
*  Prototype
*    float __aeabi_l2f(long long x);
*
*  Parameters
*    r1:r0 - x - Integer value to convert, hi:lo.
*
*  Return value
*    r0 - Floating value.
*/

#undef L
#define L(label) .L__aeabi_l2f_##label

ARM_GLOBAL_FUNC __aeabi_l2f

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Prepare to enter standard unsigned float function
        la      r3, __aeabi_ul2f

// If argument is positive, just float it.
        tst     xh, xh
        bpl     L(positive)

// float(x), x<0 === -float(|x|)
        movs    r2, #0
        negs    xh, xh
        negs    xl, xl
        sbcs    xh, r2

// Save our return address, float argument.
        push    {lr}
        BLXx    __aeabi_ul2f, r3
        pop     {r3}

// Negate result to form -float(|x|).
        movs    r1, #1
        lsls    r1, r1, #31
        orrs    r0, r0, r1

// All done.
L(positive):
        bx      r3

#else

// Preserve sign in r2 and test it.
        lsrs    r2, xh, #31
        beq     L(positive)

// Preserve sign in bit 8.
        lsls    r2, r2, #8

// Form magnitude of argument by negating it.
        rsbs    xl, xl, #0
        sbc     xh, xh, xh, lsl #1      // xh = xh-2xh-c = xh-c

// Divide into two flows based on magnitide.
L(positive):
        CBZx    xh, L(high_word_zero)   // Top 32 bits are zero, so shift quickly.

#if __SEGGER_RTL_CORE_HAS_CLZ

// Normalization.
        clz     r3, xh
        adds    r2, r2, #63+0x7f-1
        subs    r2, r2, r3
        lsls    xh, xh, r3
        lsls    ip, xl, r3
        it      ne
        orrne   xh, xh, #1
        rsbs    r3, r3, #32
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orrs    xh, xh, xl, lsr r3
#else
        lsrs    r3, xl, r3
        orrs    xh, xh, r3
#endif

#else

        adds    r2, r2, #63+0x7f-1      // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.
// Normalization.
        cmp     xh, #0x10000
        itttt   cc
        subcc   r2, r2, #16
        movcc   xh, xh, lsl #16
        orrcc   xh, xh, xl, lsr #16
        movcc   xl, xl, lsl #16

// Next normalization, we know we have at least 8 bits to shift.
        cmp     xh, #0x01000000
        itttt   cc
        subcc   r2, r2, #8
        movcc   xh, xh, lsl #8
        orrcc   xh, xh, xl, lsr #24
        movcc   xl, xl, lsl #8

// Next normalization, we know we have at least 4 bits to shift.
        cmp     xh, #0x10000000
        itttt   cc
        subcc   r2, r2, #4
        movcc   xh, xh, lsl #4
        orrcc   xh, xh, xl, lsr #28
        movcc   xl, xl, lsl #4

// Tricky part.  We have between zero and three places to shift, so test the top two bits.
// If both bits are zero, we know we have at least two bits to shift.
        tst     xh, #0xc0000000
        itttt   eq
        subeq   r2, r2, #2              // If both bits zero, we can shift two bits,
        moveq   xh, xh, lsl #2          // and adjust the exponent and set the N flag.
        orreq   xh, xh, xl, lsr #30
        moveq   xl, xl, lsl #2
        tst     xh, xh
        ittt    pl
        movpl   xh, xh, lsl #1          // If top bit is zero, we need to shift once more,
        orrpl   xh, xh, xl, lsr #31
        subpl   r2, r2, #1              // and adjust the exponent.

        cmp     xl, #0                  // Fold lower half into upper half of argument
        it      ne
        orrne   xh, xh, #1
#endif

// Rounding.
        lsls    r2, r2, #23
        lsls    r3, xh, #25             // Set carry bit to R0.7 and Z=1 if value stored is zero
        adc     r0, r2, xh, lsr #8      // Insert sign and exponent (see above)
        it      cc
        bxcc    lr                      // No rounding required
        it      ne
        bxne    lr                      // No rounding required
        bic     r0, r0, #1
        bx      lr

// Normalization.
L(high_word_zero):
        CBZx    xl, L(done)
        adds    r2, r2, #31+0x7f-1      // Set exponent.
        NORM32D xl, xl, r2, r3          // xl is normalized, r2 is decremented by number of shifts required to normalize, r3 is a temporary for CLZ
        lsls    r2, r2, #23
        lsls    r3, xl, #25             // Set carry bit to R0.7 and Z=1 if value stored is zero
        adc     r0, r2, xl, lsr #8      // Insert sign and exponent (see above)
        it      cc
        bxcc    lr                      // No rounding required
        it      ne
        bxne    lr                      // No rounding required
        bic     r0, r0, #1
L(done):
        bx      lr

#endif

END_FUNC __aeabi_l2f

/*********************************************************************
*
*       __aeabi_l2d()
*
*  Function description
*    Convert long long to double.
*
*  Prototype
*    double __aeabi_l2d(long long x);
*
*  Parameters
*    r1:r0 - x - Integer value to convert, hi:lo.
*
*  Return value
*    r1:r0 - Floating value, hi:lo.
*/

#undef L
#define L(label) .L__aeabi_l2d_##label

ARM_GLOBAL_FUNC __aeabi_l2d

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Floating 0 to 0.0 is easy.
        movs    r3, xh
        orrs    r3, r3, xl
        beq     L(done)

// Preserve sign in r2.
        lsrs    r2, xh, #31

// Set r2 to the exponent of 2^63, i.e. 63.  However, we don't knock off the
// integer bit before the binary point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
        lsls    r2, r2, #3
        adds    r2, r2, #0x04           // Subtract an extra '1' as this compensates
        lsls    r2, r2, #8              // for the '1' added by the hidden bit which
        adds    r2, r2, #0x3d           // we do not remove.  43D = 63+0x3ff-1
                                        
// Form magnitude of argument.
        tst     xh, xh
        bpl     .L__aeabi_l2d_common

// Form magnitude of argument by negating it.
        movs    r3, #0                  // 64-bit negation, Thumb-style.
        negs    xh, xh
        negs    xl, xl
        sbcs    xh, r3

// Adjust so that ms word is non-zero in preparation for normalization.
// Quick tst for 32-bit normalization.
LOCAL_ENTRY .L__aeabi_l2d_common
        bne     L(no_quick_shift)
        movs    xh, xl
        movs    xl, #0
        subs    r2, r2, #32             // Adjust exponent

// Quick test for 16-bit normalization.
L(no_quick_shift):
        lsrs    r3, xh, #16
        bne     L(clockwork_shifts)
        subs    r2, r2, #16
        lsls    xh, xh, #16
        lsrs    r3, xl, #16
        orrs    xh, xh, r3
        lsls    xl, xl, #16

// Do normlization slowly, a bit at a time.
L(clockwork_shifts):

// If already normalized, don't need to do any shifting.
        tst     xh, xh
        bmi     L(normalized)

// Normalize by clockwork.
L(normalize):
        subs    r2, r2, #1
        adds    xl, xl, xl
        adcs    xh, xh, xh
        bpl     L(normalize)

// Now need to round and combine exponent and sign.
L(normalized):

// Save copy of low bits, we need them to decide on rounding.
        mov     ip, xl

// Move significand into position, right shift xh:xl by 11 bits
        lsrs    xl, xl, #11
        lsls    r3, xh, #21
        orrs    xl, xl, r3
        lsrs    xh, xh, #11

// Move result's sign and exponent into position (see below for insertion).
        lsls    r2, r2, #20

// Now round.  Recover low bits.
        mov     r3, ip

// Shift low-order bits rounding bits into high order.
        lsls    r3, r3, #21
        adcs    r3, r3, r3
        bcc     L(no_rounding)
        cmp     r3, #1                  // C=1 if r3 non-zero
        movs    r3, #0
        adcs    xl, xl, r3              // Rounds and will magically increment exponent too.  Result!
L(no_rounding):
        adcs    xh, xh, r2              // Insert sign and exponent (see above)

// All done.
L(done):
        bx      lr

#else

// Move sign bit to adjoin exponent
        lsrs    r2, xh, #31
        beq     L(positive)

// Preserve sign in bit 11.
        lsls    r2, r2, #11

// Form magnitude of argument by negating it.
        rsbs    xl, xl, #0
        sbc     xh, xh, xh, lsl #1      // xh = xh-2xh-c = xh-c

// Adjust so that ms word is non-zero in preparation for normalization.
L(positive):
        CBZx    xh, L(high_word_zero)

#if __SEGGER_RTL_CORE_HAS_ADDW_SUBW
        add     r2, r2, #0x43d          // Set exponent
#else
        add     r2, r2, #0x400          // Set exponent
        add     r2, r2, #0x03d
#endif

#if __SEGGER_RTL_CORE_HAS_CLZ

// Normalization.
        clz     r3, xh                  // r3 := Number of places to normalize
        subs    r2, r2, r3              // Adjust exponent
        rsb     ip, r3, #32             // 32-bit shift r1:r0 left by r3 places; no spare register for negated bitcount though...

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        lsls    xh, xh, r3
        orrs    xh, xh, xl, lsr ip
        lsls    xl, xl, r3
#else
        lsrs    ip, xl, ip
        lsls    xl, xl, r3
        lsls    xh, xh, r3
        orrs    xh, xh, ip
#endif

#else

// Normalization.
        cmp     xh, #0x10000
        itttt   cc
        subcc   r2, r2, #16
        lslcc   xh, xh, #16
        orrcc   xh, xh, xl, lsr #16
        lslcc   xl, xl, #16

// Next normalization, we know we have at least 8 bits to shift.
        cmp     xh, #0x01000000
        itttt   cc
        subcc   r2, r2, #8
        lslcc   xh, xh, #8
        orrcc   xh, xh, xl, lsr #24
        lslcc   xl, xl, #8

// Next normalization, we know we have at least 4 bits to shift.
        cmp     xh, #0x10000000
        itttt   cc
        subcc   r2, r2, #4
        lslcc   xh, xh, #4
        orrcc   xh, xh, xl, lsr #28
        lslcc   xl, xl, #4

// Tricky part.  We have between zero and three places to shift, so test the top two bits.
// If both bits are zero, we know we have at least two bits to shift.
        tst     xh, #0xc0000000
        itttt   eq
        subeq   r2, r2, #2              // If both bits zero, we can shift two bits,
        moveq   xh, xh, lsl #2          // and adjust the exponent, can't set N flag here.
        orreq   xh, xh, xl, lsr #30
        moveq   xl, xl, lsl #2
        tst     xh, xh
        itttt   pl
        lslpl   xh, xh, #1              // If top bit is zero, we need to shift once more,
        orrpl   xh, xh, xl, lsr #31
        lslpl   xl, xl, #1
        subpl   r2, r2, #1              // and adjust the exponent.

#endif

// Rounding.
        lsls    r3, xl, #22             // Set carry bit to xh.7; r3 indicates whether we have a tie in this case
        lsr     xl, xl, #11
        orr     xl, xl, xh, lsl #21
        lsr     xh, xh, #11             // Move significand into position
        add     xh, xh, r2, lsl #20     // Insert sign and exponent (see above)
        it      cc
        bxcc    lr                      // Condition LS = C=0 .or. Z=1
        adds    xl, xl, #1              // Rounds and will magically increment exponent too.  Result!
        adc     xh, xh, #0
        tst     r3, r3
        it      eq
        biceq   xl, xl, #1
        bx      lr

L(high_word_zero):
        CBZx    xl, L(done)

// Normalization.
#if __SEGGER_RTL_CORE_HAS_ADDW_SUBW
        add     r2, r2, #0x41d          // Set exponent
#else
        add     r2, r2, #0x400          // Set exponent
        add     r2, r2, #0x01d
#endif
        NORM32D xl, xl, r2, r3          // xl is normalized, r2 decremented by number of shifts required to normalize, r3 is a temnporary in case of CLZ

// 32 bits fit into a double exactly with no rounding.
        lsrs    xh, xl, #11             // Move significand into position
        lsls    xl, xl, #21
        add     xh, xh, r2, lsl #20     // Insert sign and exponent (see above)
L(done):
        bx      lr

#endif

END_FUNC __aeabi_l2d

/*********************************************************************
*
*       __aeabi_ui2f()
*
*  Function description
*    Convert unsigned to float.
*
*  Prototype
*    float __aeabi_i2f(unsigned x);
*
*  Parameters
*    r0 - x - Integer value to convert.
*
*  Return value
*    r0 - Floating value.
*/

#undef L
#define L(label) .L__aeabi_ui2f_##label

ARM_GLOBAL_FUNC __aeabi_ui2f

#if __SEGGER_RTL_FP_HW >= 1

        vmov    s0, r0
        fuitos  s0, s0
        vmov    r0, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Set r2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the decimal point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
        movs    r2, #31+0x7f-1

// Floating 0 to 0.0 is easy.  Also tests the sign bit and duplicates argument
// into r1.
        movs    r1, r0
        beq     L(done)

// Could be lucky and argument is already normalized.
        bmi     L(normalized)

// Iteratively normalize; anything faster turns out rather ugly.
L(normalize):
        subs    r2, r2, #1
        lsls    r1, r1, #1
        bpl     L(normalize)

// Move exponent into final position.
L(normalized):
        lsls    r2, r2, #23

// Move significand into position.
        lsrs    r0, r1, #8

// Scrub off upper bits of significand and prepare to round
// on lower order bits only.
        lsls    r1, r1, #24

// 0x rounds down, 1 may need to round up or down...
        adcs    r1, r1, r1
        bcc     L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        cmp     r1, #1

// Insert exponent and break-tie flag.  Overflow gets nicely handled
L(round_down):
        adcs    r0, r0, r2
L(done):
        bx      lr

#else

#if __SEGGER_RTL_CORE_HAS_CLZ

// Note: using cbz to pre-test input being zero results in a 1-cycle
// penalty, so defer this test to after the normalization, the result
// of which can only be zero if the input is zero. You lean something
// new about the core each and every time you write some Arm code...

// Normalization by CLZ.
        clz     r1, r0
        lsls    r3, r0, r1
        beq     L(done)                 // Exactly 0.0
        rsbs    r1, r1, #31+0x7f-1

#else

// Normalization by shifting.
        CBZx    r0, L(done)             // Test floating 0.
        movs    r1, #31+0x7f-1          // Load exponent for fully normalized input.
        NORM32D r3, r0, r1, unused      // r3 is normalized r0, r1 decremented by number of shifts required to normalize, temnporary is uuused

#endif

// Move exponent into position.
        lsls    r1, r1, #23

// Combine exponent (r1) and normalized significand (r3).
        add     r0, r1, r3, lsr #8

// Round result.
        lsls    r3, r3, #25             // Set carry bit to R0.7 and Z=1 if value stored is zero
        it      cc
        bxcc    lr                      // No rounding required
        add     r0, r0, #1              // Round up
        it      ne
        bxne    lr
        bic     r0, r0, #1
L(done):
        bx      lr

#endif

END_FUNC __aeabi_ui2f

/*********************************************************************
*
*       __aeabi_ui2d()
*
*  Function description
*    Convert unsigned to double.
*
*  Prototype
*    double __aeabi_ui2d(unsigned x);
*
*  Parameters
*    r0 - x - Integer value to convert.
*
*  Return value
*    r1:r0 - Floating value.
*/

#undef L
#define L(label) .L__aeabi_ui2d_##label

ARM_GLOBAL_FUNC __aeabi_ui2d

#if __SEGGER_RTL_FP_HW >= 2

        vmov    s0, r0
        fuitod  d0, s0
        vmov    zl, zh, d0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Set r2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the radix point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// by one here.
#if __SEGGER_RTL_CORE_HAS_MOVW_MOVT
        movw    r2, #0x41D
#else
        movs    r2, #0x42
        lsls    r2, r2, #4              // 0x420
        subs    r2, r2, #3              // 0x41D
#endif

// Floating zero is easy.
        movs    r1, r0
        beq     L(zero)

// Could be lucky and argument is already normalized.
        bmi     L(normalized)

// Iteratively normalize; anything faster turns out rather ugly.
L(normalize):
        subs    r2, r2, #1
        adds    r0, r0, r0
        bpl     L(normalize)

// Move exponent into final position.
L(normalized):
        lsls    r2, r2, #20             // Insert exponent

// Move significand into final position.
        lsrs    r3, r0, #11
        lsls    zl, r0, #21

// Combine exponent with significand.
        adds    zh, r3, r2
L(zero):
        bx      lr

#else

// Floating 0 to 0.0 is easy.
        CBZx    r0, L(zero)

// Set r2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the decimal point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
        li      r2, 0x41d               // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.  41D = 31+0x3ff-1

// Normalization.
L(normalize):
        NORM32D r0, r0, r2, r1          // r0 is normalized, r2 decremented by number of shifts required to normalize, r1 is a temnporary in case of CLZ
        
// Pack signficand and exponent.
        lsrs    r3, r0, #11             // Use narrow Thumb-2 encoding
        lsls    zl, r0, #21             // Use narrow Thumb-2 encoding
        add     zh, r3, r2, lsl #20     // Insert sign and exponent (see above)
        bx      lr

// Result is zero.
L(zero):
        movs    r1, r0
        bx      lr
#endif

END_FUNC __aeabi_ui2d

/*********************************************************************
*
*       __aeabi_ul2f()
*
*  Function description
*    Convert unsigned long long to float.
*
*  Prototype
*    float __aeabi_ul2f(unsigned long long x);
*
*  Parameters
*    r1:r0 - x - Integer value to convert, hi:lo.
*
*  Return value
*    r0 - Floating value.
*/

#undef L
#define L(label) .L__aeabi_ul2f_##label

ARM_GLOBAL_FUNC __aeabi_ul2f

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Set r2 to the exponent of 2^63, i.e. 63.  However, we don't knock off the
// integer bit before the decimal point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
        movs    r2, #63+0x7f-1

// If the value to float is less than 2^32, i.e. the high word is zero, then
// we can simply float a 32-bit.
        tst     xh, xh
        beq     L(small_value)

// Could be lucky and argument is already normalized.
        bmi     L(normalized)

// Iteratively normalize; anything faster turns out rather ugly.
L(normalize):
        subs    r2, r2, #1
        adds    xl, xl, xl
        adcs    xh, xh, xh
        bpl     L(normalize)

// Move exponent into final position.
L(normalized):
        lsls    r2, r2, #23

// Compress (fold) bits from lower half only if they are non-zero.
        cmp     xl, #1
        sbcs    xl, xl
        adds    xl, xl, #1
        orrs    xh, xl

// Move significand into position.
        lsrs    xl, xh, #8

// Scrub off upper bits of significand and prepare to round
// on lower order bits only (include fold).
        lsls    xh, xh, #24

// 0x rounds down, 1 may need to round up or down...
        adcs    xh, xh, xh
        bcc     L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        cmp     xh, #1

// Insert exponent and break-tie flag.  Overflow gets nicely handled
L(round_down):
        adcs    xl, xl, r2
#if __SEGGER_RTL_BYTE_ORDER > 0
        movs    r0, xl
#endif
        bx      lr

// Float a value less than 2^32.
L(small_value):
#if __SEGGER_RTL_BYTE_ORDER > 0
        movs    r0, r1
#endif
        la      r1, __aeabi_ui2f
        bx      r1

#else

// Check high word.
        CBZx    xh, L(high_word_zero)   // Top 32 bits are zero, so shift quickly.

#if __SEGGER_RTL_CORE_HAS_CLZ

// Normalization.
        clz     r3, xh
        rsbs    r2, r3, #63+0x7f-1
        lsls    xh, xh, r3
        lsls    ip, xl, r3
        it      ne
        orrne   xh, xh, #1              // Fold lower half into upper half of argument
        rsbs    r3, r3, #32
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        orrs    xh, xh, xl, lsr r3
#else
        lsrs    r3, xl, r3
        orrs    xh, xh, r3
#endif

#else

        movs    r2, #63+0x7f-1          // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.
// Normalization.
        cmp     xh, #0x10000
        itttt   cc
        subcc   r2, r2, #16
        movcc   xh, xh, lsl #16
        orrcc   xh, xh, xl, lsr #16
        movcc   xl, xl, lsl #16

// Next normalization, we know we have at least 8 bits to shift.
        cmp     xh, #0x01000000
        itttt   cc
        subcc   r2, r2, #8
        movcc   xh, xh, lsl #8
        orrcc   xh, xh, xl, lsr #24
        movcc   xl, xl, lsl #8

// Next normalization, we know we have at least 4 bits to shift.
        cmp     xh, #0x10000000
        itttt   cc
        subcc   r2, r2, #4
        movcc   xh, xh, lsl #4
        orrcc   xh, xh, xl, lsr #28
        movcc   xl, xl, lsl #4

// Tricky part.  We have between zero and three places to shift, so test the top two bits.
// If both bits are zero, we know we have at least two bits to shift.
        tst     xh, #0xc0000000
        itttt   eq
        subeq   r2, r2, #2              // If both bits zero, we can shift two bits,
        moveq   xh, xh, lsl #2          // and adjust the exponent and set the N flag.
        orreq   xh, xh, xl, lsr #30
        moveq   xl, xl, lsl #2
        tst     xh, xh
        ittt    pl
        movpl   xh, xh, lsl #1          // If top bit is zero, we need to shift once more,
        orrpl   xh, xh, xl, lsr #31
        subpl   r2, r2, #1              // and adjust the exponent.

// Fold lower half into upper half of argument
        cmp     xl, #0
        it      ne
        orrne   xh, xh, #1
#endif

// Rounding.
        lsls    r2, r2, #23
        lsls    r3, xh, #25             // Set carry bit to R0.7 and Z=1 if value stored is zero
        adc     r0, r2, xh, lsr #8      // Insert sign and exponent (see above)
        it      cc
        bxcc    lr                      // No rounding required
        it      eq
        biceq   r0, r0, #1
        bx      lr

// Normalization.
L(high_word_zero):
        CBZx    xl, L(done)

#if __SEGGER_RTL_CORE_HAS_CLZ
        clz     r3, xl
        lsls    xl, xl, r3
        rsbs    r2, r3, #31+0x7f-1
#else
        movs    r2, #31+0x7f-1          // Set exponent.
        NORM32D xl, xl, r2, unused      // r0 is normalized, r2 decremented by number of shifts required to normalize, temnporary is unused
#endif

// Pack significand, exponent, and sign.
        lsls    r2, r2, #23             // Move exponent and sign into position
        lsls    r3, xl, #25             // Set carry bit to R0.7 and Z=1 if value stored is zero
        adc     r0, r2, xl, lsr #8      // Insert sign and exponent (see above)
        it      cc
        bxcc    lr                      // No rounding required
        it      eq                      // Break tie if exactly 0.5
        biceq   r0, r0, #1
L(done):
        bx      lr

#endif

END_FUNC __aeabi_ul2f

/*********************************************************************
*
*       __aeabi_ul2d()
*
*  Function description
*    Convert unsigned long long to double.
*
*  Prototype
*    float __aeabi_ul2f(unsigned long long x);
*
*  Parameters
*    r1:r0 - x - Integer value to convert, hi:lo.
*
*  Return value
*    r1:r0 - Floating value, hi:lo.
*/

#undef L
#define L(label) .L__aeabi_ul2d_##label

ARM_GLOBAL_FUNC __aeabi_ul2d

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Floating 0 to 0.0 is easy.
        movs    r3, xh
        orrs    r3, r3, xl
        beq     L(done)

// Set r2 to the exponent of 2^63, i.e. 63.  However, we don't knock off the
// integer bit before the binary point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
        movs    r2, #0x04               // Subtract an extra '1' as this compensates
        lsls    r2, r2, #8              // for the '1' added by the hidden bit which
        adds    r2, r2, #0x3d           // we do not remove.  43D = 63+0x3ff-1

// Entry requirement to common code is to know about xh.
        tst     xh, xh
        la      r3, .L__aeabi_l2d_common
        bx      r3

L(done):
        bx      lr

#else

// Punt to logic.
        CBZx    xh, L(high_word_zero)

        li      r2, 0x43d               // Set exponent

#if __SEGGER_RTL_CORE_HAS_CLZ

// Normalization.
        clz     r3, xh                  // r3 := Number of places to normalize
        subs    r2, r2, r3              // Adjust exponent
        rsb     ip, r3, #32             // 32-bit shift r1:r0 left by r3 places; no spare register for negated bitcount though...
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM
        lsls    xh, xh, r3
        orrs    xh, xh, xl, lsr ip
        lsls    xl, xl, r3
#else
        lsrs    ip, xl, ip
        lsls    xl, xl, r3
        lsls    xh, xh, r3
        orrs    xh, xh, ip
#endif

#else

// Normalization.
        cmp     xh, #0x10000
        itttt   cc
        subcc   r2, r2, #16
        lslcc   xh, xh, #16
        orrcc   xh, xh, xl, lsr #16
        lslcc   xl, xl, #16

// Next normalization, we know we have at least 8 bits to shift.
        cmp     xh, #0x01000000
        itttt   cc
        subcc   r2, r2, #8
        lslcc   xh, xh, #8
        orrcc   xh, xh, xl, lsr #24
        lslcc   xl, xl, #8

// Next normalization, we know we have at least 4 bits to shift.
        cmp     xh, #0x10000000
        itttt   cc
        subcc   r2, r2, #4
        lslcc   xh, xh, #4
        orrcc   xh, xh, xl, lsr #28
        lslcc   xl, xl, #4

// Tricky part.  We have between zero and three places to shift, so test the top two bits.
// If both bits are zero, we know we have at least two bits to shift.
        tst     xh, #0xc0000000
        itttt   eq
        subeq   r2, r2, #2              // If both bits zero, we can shift two bits,
        moveq   xh, xh, lsl #2          // and adjust the exponent, can't set N flag here.
        orreq   xh, xh, xl, lsr #30
        moveq   xl, xl, lsl #2
        tst     xh, xh
        itttt   pl
        lslpl   xh, xh, #1              // If top bit is zero, we need to shift once more,
        orrpl   xh, xh, xl, lsr #31
        lslpl   xl, xl, #1
        subpl   r2, r2, #1              // and adjust the exponent.

#endif

// Rounding.
        lsls    r3, xl, #22             // Set carry bit to xh.7; r3 indicates whether we have a tie in this case
        lsr     xl, xl, #11
        orr     xl, xl, xh, lsl #21
        lsr     xh, xh, #11             // Move significand into position
        add     xh, xh, r2, lsl #20     // Insert sign and exponent (see above)
        it      cc
        bxcc    lr
        adds    xl, xl, #1              // Rounds and will magically increment exponent too.  Result!
        adc     xh, xh, #0
        tst     r3, r3
        it      eq
        biceq   xl, xl, #1
        bx      lr

L(high_word_zero):
        CBZx    xl, L(done)

// Normalization.
        li      r2, 0x41d               // Set exponent
        NORM32D xl, xl, r2, r3          // xl is normalized, r2 decremented by number of shifts required to normalize, r3 is a temnporary in case of CLZ

// 32 bits fit into a double exactly with no rounding.
        lsrs    xh, xl, #11             // Move significand into position
        lsls    xl, xl, #21
        add     xh, xh, r2, lsl #20     // Insert sign and exponent (see above)
L(done):
        bx      lr

#endif

END_FUNC __aeabi_ul2d

/*********************************************************************
*
*       __aeabi_f2d()
*
*  Function description
*    Convert float to double.
*
*  Prototype
*    double __aeabi_f2d(float x);
*
*  Parameters
*    r0 - x - Float value to convert.
*
*  Return value
*    r1:r0 - Double value.
*/

#undef L
#define L(label) .L__aeabi_f2d_##label

ARM_GLOBAL_FUNC __aeabi_f2d

#if __SEGGER_RTL_FP_HW >= 2

        vmov    s0, r0
        vcvt.f64.f32 d0, s0
        vmov    xl, xh, d0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Shift off sign leaving exponent in high order byte--we need to compare magnitude.
        lsls    r1, r0, #1
        mov     ip, r1

// Handle NaN and Inf out of line, not the common case.
        movs    r2, #1
        lsls    r2, r2, #24             // 0x01000000

// Flush subnormals to zero.
        cmp     r2, r1
        bhi     L(subnormal)

// Align single precision mantissa to its double precision position.
        lsrs    r1, r1, #4

// Re-bias to new exponent.
        movs    r3, #0x38               // Generate 0x38000000
        lsls    r3, r3, #24
        adds    r1, r1, r3

#if __SEGGER_RTL_BYTE_ORDER < 0

// Extract sign from single precision value and move to double precision placement.
        lsrs    r3, r0, #31             // v6M has no RRX so burn a register
        lsls    r3, r3, #31
        orrs    r1, r1, r3

// Move lower 4 bits of 23-bit significand to lower 4 bits of double significand.
        lsls    r0, r0, #29

#else

// Extract sign from single precision value and move to double precision placement.
        lsrs    r3, r0, #31             // v6M has no RRX so burn a register
        lsls    r3, r3, #31
        orrs    r3, r3, r1

// Move lower 4 bits of 23-bit significand to lower 4 bits of double significand.
        lsls    r1, r0, #29
        movs    r0, r3

#endif

// Was input an infinity or NaN?
        mov     r3, ip
        cmn     r2, r3
        bcs     L(inf_nan)
        bx      lr

// Zero or subnormal input: flush to signed zero.
L(subnormal):
        lsrs    zh, r0, #31             // Copy sign to double precision position
        lsls    zh, zh, #31

// Zero-extend significand and return.
        movs    zl, #0
        bx      lr

// Input is Inf or NaN.  Generate ponent is a double-precision Inf/NaN exponent.
L(inf_nan):
        bne     L(nan)
        movs    r3, #0x7f-0x47
        lsls    r3, r3, #24
        adds    zh, zh, r3
        bx      lr

// Adjust exponent and it's a quiet NaN.
L(nan):
        movs    r3, #0x7f-0x47
        lsls    r3, r3, #24
        adds    zh, zh, r3
        movs    r2, #1                  // Generate 0x00080000
        lsls    r2, r2, #19
        orrs    zh, zh, r2              // Ensure it's a quiet NaN.
        bx      lr

#elif (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_ARM)

// Dispose of sign bit, save it in carry.
        lsls    r2, r0, #1

// Shift remaning bits from single to low-order double result.
        lsl     xl, r2, #28

// Move single-float exponent to double-float position and recover sign.
        lsr     xh, r2, #3
        rrx     xh, xh

// Isolate incoming single-precision exponent.
        ands    r2, xh, #0x0ff00000
        
// Z flag indicates special exponent, 0 (set above) or $ff (checked here).
        cmpne   r2, #0xff00000

// If the expnent is not special, rebias double-float exponent and done.
        addne   xh, xh, #0x38000000
        bxne    lr

// Exponent is either zero or 0xff.
        cmp     r2, #0

// If exponent is zero, flush to signed zero.
        andeq   xh, xh, #0x80000000
        moveq   xl, #0
        bxeq    lr

// Now convert to Inf and NaN proper.  Adjust to Inf/NaN exponent.
        lsls    r2, xh, #4
        orr     xh, xh, #0x70000000

// Check for NaN.  If NaN, set double-float NaN indicator.
        cmp     xl, #0
        cmpeq   r2, #0xff000000
        orrne   xh, xh, #0x00080000

// Done.
        bx      lr

#else

// Convert subnormal inputs to zero.
#if __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX
        ubfx    r1, r0, #23, #8
        CBZx    r1, L(zero)
#else
        movs    ip, #0xff000000
        tst     ip, r0, lsl #1
        beq     L(zero)
#endif

// r2 is exponent plus significand but without sign bit; save sign bit in carry.
        lsls    r2, r0, #1

// Move lower 4 bits of 23-bit significand to lower 4 bits of double significand.
        lsl     xl, r0, #29

// Move high 20 bits of single significand to position in double significand,
// and move exponent to correct place also and drag in sign.
        lsr     xh, r2, #3
        rrx     xh, xh

// Only if the the input is not +/-0 do we adjust the single exponent (excess 127)
// to double exponent (excess 1023), i.e. add 3ff-7f = 0x380.  This doesn't
// convert NaNs and Infs correctly though...
        add     xh, xh, #0x38000000

// See if converted result is a converted Inf or NaN.
        cmp     r2, #0xFF000000
        it      cc
        bxcc    lr                      // If finite, job is done

// Now convert to Inf and NaN proper.
        add     xh, xh, #0x7ff00000-0x47f00000
        it      ne
        orrne   xh, xh, #0x00080000     // Ensure it's a quiet NaN, don't change Inf.
        bx      lr

// Result must be truncated to zero.
L(zero):
        ands    xh, r0, #0x80000000
        movs    xl, #0
        bx      lr

#endif

END_FUNC __aeabi_f2d

/*********************************************************************
*
*       __aeabi_d2f()
*
*  Function description
*    Convert double to float.
*
*  Prototype
*    float __aeabi_d2f(double x);
*
*  Parameters
*    r1:r0 - x - Double value to convert.
*
*  Return value
*    r0 - Float value.
*/

#undef L
#define L(label) .L__aeabi_d2f_##label

ARM_GLOBAL_FUNC __aeabi_d2f

#if __SEGGER_RTL_FP_HW >= 2

        vmov    d0, xl, xh
        vcvt.f32.f64 s0, d0
        vmov    xl, s0
        bx      lr

#elif __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

// Compare magnitude of input to find if we have an Inf or NaN which are
// not the common case.
        adds    r3, xh, xh              // r3 = input sans sign bit
        movs    r2, #1
        lsls    r2, r2, #21             // 0x00200000
        cmn     r2, r3                  // special?
        bcs     L(inf_nan)              // Handle Inf and NaN specially

// We now have a finite value, but we don't want it subnormal.
// Extract biased exponent to r3.
        lsrs    r3, r3, #21

// Compute exponent biased to single-precision.
        movs    r2, #7
        lsls    r2, r2, #7              // 0x00000380
        subs    r3, r3, r2

// If the exponent underflowed or input is subnormal or zero, need a signed zero.
        bls     L(underflow)

// If we can't represent this in single precision, overflow to Inf.
        cmp     r3, #255
        bcs     L(inf)

// Move exponent into dp position.
        lsls    r3, r3, #23

// Isolate sign into high bit of r2.
        lsrs    r2, xh, #31
        lsls    r2, r2, #31

// Copy argument's sign bit to result's sign bit.  r3 now contains
// the sign bit plus result's exponent and zero significand.
        orrs    r3, r3, r2

// Shift dp significand to align at bit 31 including low-order part.
        lsls    xh, xh, #12
        lsrs    r2, xl, #20
        orrs    xh, xh, r2

// Move significand into position for sp result.
        lsrs    xh, xh, #9

// Use low-order bits to break tie in rounding.
        lsls    r2, xl, #3
        adcs    r2, r2, r2
        bls     L(no_round_tie)
        adds    xh, xh, #1

// Rounding done; insert exponent and return.  NOTE: "add" is required,
// "or" will not work as rounding above "overflows" to adjust the exponent!
L(no_round_tie):
        adds    r0, xh, r3
        bx      lr

// Can't represent the argument in single precision as it underflowed.
// Generate a correctly-signed zero from the argument in xh into r0.
L(underflow):
        lsrs    r0, xh, #31
        lsls    r0, r0, #31
        bx      lr

// Distinguish between Inf and NaN.  We do process non-EABI NaNs where the
// low-order bits are also taken into account.
L(inf_nan):
        bne     L(nan)

// High part matches Inf pattern, but it may still be a NaN.
        CBNZx   xl, L(nan)

// Can't represent the argument in single precision as it overflowed or.
// the input is Inf.  Generate a correctly-signed zero from the argument
// in xh into r0.
L(inf):
        lsrs    r0, xh, #31           // Extract sign to lsb of r0
        lsls    r0, r0, #8            // make room for 0xFF bits.
        adds    r0, #0xff             // Maximum exponent
        lsls    r0, r0, #23           // Move exponent and sign into place
        bx      lr

// We now know the input is a NaN.  Convert the 64-bit NaN to a 32-bit NaN
// preserving the information in the significand bits.
L(nan):

// Generally useful constant 0x80000000 to manipulate sign bit.
        movs    r3, #1
        lsls    r3, r3, #31

// Preserve sign bit of NaN in r2.
        movs    r2, xh

// Shift significand bits from dp to sp position.
        lsls    xh, xh, #3              // Move significand digits
        lsrs    xl, xl, #32-3           // Wedge in lower bits
        orrs    r0, xl, xh

// Copy original sign of NaN to sp position.
        bics    r0, r0, r3              // Clear result sign bit
        ands    r2, r2, r3              // Isolate sign bit
        orrs    r0, r0, r2              // Copy in sign bit

// Ensure that the NaN indicator is set.
        lsrs    r3, r3, #9
        orrs    r0, r0, r3
        bx      lr

#else

// Extract sign to r2.
        and     r2, xh, #0x80000000

// Form high-word magnitude in r3.
        subs    r3, xh, r2

// Rebias exponent.
        sub     xh, r3, #+0x3ff00000-0x07f00000

// If underflow, return signed zero.
        cmp     xh, #0x100000
        blt     L(zero)

// If input is too big, or Inf or NaN, extra refinement is necessary.
        cmp     xh, #0xff00000
        bcs     L(overflow_inf_nan)

// Shift significand into position.
        orr     r2, r2, xh, lsl #3

// Round.
        lsls    xh, xl, #3
        sbcs    xh, xh, #0x80000000
        adc     r0, r2, xl, lsr #29
        bx      lr

// Input is subnormal or zero, so truncate to signed zero.
L(zero):
        mov     r0, r2
        bx      lr

// Input is overflow, or Inf or NaN.
L(overflow_inf_nan):
        cmp     xl, #1                  // Fold low-order word into high order low bit for true NaN compare
        adcs    r3, r3, r3

// Check for NaN input.
        cmp     r3, #0xffe00000
        bhi     L(nan)

// Input is too large or infinite.
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32
        orr     r0, r2, #0x7f800000    // Construct signed infinity
#else
        orr     r0, r2, #0x7f000000    // Construct signed infinity
        orr     r0, r0, #0x00800000
#endif
        bx      lr

// Input is a NaN, so preserve signaled bits and convert to a short NaN.
L(nan):
        lsls    xh, xh, #3              // Move significand digits
        orr     r0, xh, xl, lsr #32-3   // Wedge in lower bits
        orrs    r0, r0, r2              // Recover sign status bit
        orr     r0, r0, #0x00400000     // Ensure NaN bits set.
        orr     r0, r0, #0x40000000     // Ensure NaN bits set.
        bx      lr

#endif

END_FUNC __aeabi_d2f

#endif

#endif

       .end

/*************************** End of file ****************************/
