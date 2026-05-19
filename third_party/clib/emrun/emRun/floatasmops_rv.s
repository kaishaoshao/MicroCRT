/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Purpose: Floating-point emulator for RV32I, RV32E, and RV64I.


*/

// ENHANCEMENT -- Unify overflow handling in RV32 fix functions to mirror the
//                overflow handing in the RV64 version.

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "asmdefs_rv.ah"

#if __SEGGER_RTL_INCLUDE_GNU_API == 2

/*********************************************************************
*
*       Setup, not configurable
*
**********************************************************************
*/

       .set     __SEGGER_RTL_fdiv_reciprocal_table_REQUIRED, 0
       .set     __SEGGER_RTL_ddiv_reciprocal_table_REQUIRED, 0
       .set     __SEGGER_RTL_2pow64_REQUIRED,                0
       .set     __SEGGER_RTL_2pow32_REQUIRED,                0
       .set     __SEGGER_RTL_2powNeg32_REQUIRED,             0

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       __subsf3()
*
*  Function description
*    Subtract, float.
*
*  Prototype
*    float __subsf3(float x, float, y);
*
*  Parameters
*    a0 - x - Minuend.
*    a1 - y - Subtrahend.
*
*  Return value
*    a0 - Difference.
*/

GLOBAL_FUNC __subsf3

#if __SEGGER_RTL_FP_ABI >= 1

        fsub.s  fa0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Negate subtrahend and add.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        binvi   a1, a1, 31
#else
        li      a2, 0x80000000
        xor     a1, a1, a2
#endif
        tail    __addsf3

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Negate subtrahend and add.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        binvi   a1, a1, 31
#else
        li      a2, 0xFFFFFFFF80000000  // This is possible as the upper 32 bits of a float are 'undefined by ABI definition'
        xor     a1, a1, a2
#endif
        tail    __addsf3

#else

#error Bad configuration

#endif

END_FUNC __subsf3

/*********************************************************************
*
*       __subdf3()
*
*  Function description
*    Subtract, double.
*
*  Prototype
*    double __subsf3(double x, double y);
*
*  Parameters
*    a1:a0 / a0 - Minuend.
*    a3:a2 / a1 - Subtrahend.
*
*  Return value (RV32)
*    a1:a0 / a0 - Difference.
*/

GLOBAL_FUNC __subdf3

#if __SEGGER_RTL_FP_ABI >= 2

        fsub.d  fa0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32 
//

// Negate subtrahend and add.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        binvi   a3, a3, 31
#else
        li      a4, 0x80000000
        xor     a3, a3, a4
#endif
        tail    __adddf3

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Negate subtrahend and add.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        binvi   a3, a3, 63
#else
        li      a2, 1                   // li a2, 0x8000000000000000...
        sll     a2, a2, 63              // ...but faster
        xor     a1, a1, a2
#endif
        tail    __adddf3

#else

#error Bad configuration

#endif

END_FUNC __subdf3

/*********************************************************************
*
*       __addsf3()
*
*  Function description
*    Add, float.
*
*  Prototype
*    float __addsf3(float x, float, y);
*
*  Parameters
*    a0 - x - Augend.
*    a1 - y - Addend.
*
*  Return value
*    a0 - Sum.
*/

#undef L
#define L(label) .L__addsf3_##label

GLOBAL_FUNC __addsf3

#if __SEGGER_RTL_FP_ABI >= 1

        fadd.s  fa0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32 
//

// Generate 0x80000000.
        li      a2, 0x80000000

// Do addend and augend have the same sign?
        xor     a3, a0, a1
        bltz    a3, L(subtract)         // No, this is a subtraction

// Get largest of x and y into x, smallest into y.  Hence
// augend is largest.
        bgeu    a0, a1, L(add_already_ordered)
        mv      a3, a0
        mv      a0, a1
        mv      a1, a3
L(add_already_ordered):

// Extract exponent of smaller into a3, larger into a4.
        BFOZ    a4, a0, 30, 23
        BFOZ    a3, a1, 30, 23

// If the exponent of x (the largest) is Inf or NaN, return the largest.
        li      t0, 0xFF
        beq     a4, t0, L(add_inf_or_nan)

// If the exponent of the largest is zero, then we need to sort out adding -0.
        beqz    a4, L(zero)

// Extract exponent of smaller.  If adding zero or subnormal, then we're done.
        beqz    a3, L(add_done)

// Find difference between exponents which is the number of places
// needed to align them.  We cancel the sign bits in the subtraction
// because we know they are the same in addend and augend.
        sub     a3, a4, a3

// If the difference is big, come out immediately.
        li      t0, 24
        bgt     a3, t0, L(add_done)

// Isolate significand of augend into high 24 bits and set the hidden.
// bit.  We know this is never a subnormal.
        sll     a1, a1, 8
        or      a1, a1, a2

// Extract exponent and sign of larger.
        srl     a4, a0, 23

// Move significand of largest into high part and set hidden bit.
        sll     a0, a0, 8
        or      a0, a0, a2

// Compute number of bits to shift smaller to align significands for addition.
        li      a5, 25
        sub     a5, a5, a3

// Calculate residual that is not added to retained part of significand, but
// will be required for correct rounding.
        sll     a2, a1, a5
        srl     a2, a2, 7

// Align smaller significand.
        srl     a1, a1, a3

// Compute sum of significands.
        add     a5, a0, a1

// If we carried out, normalize sum.  a0 can never be zero.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltu    t0, a5, a0              // If no overflow, no change in state from the following two instructions
        srl     a5, a5, t0              // Bring in a zero, the hidden bit has transformed to a zero.
        LEAH    a4, a4, t0              // Increment exponent to compensate for the shift, and increment again to replace the lost hidden bit
#else
        bgeu    a5, a0, L(add_no_normalization)
        srl     a5, a5, 1               // Bring in a zero, the hidden bit has transformed to a zero.
        add     a4, a4, 2               // Increment exponent to compensate for the shift, and increment again to replace the lost hidden bit
#endif

// Move exponent into position.
L(add_no_normalization):
        add     a4, a4, -1              // Clear hidden bit from combined value by adjusting exponent down to compensate

// Check for exponent overflow, ignoring sign of significand.
        and     a1, a4, 0xFF
        addi    a1, a1, -0xFF
        beqz    a1, L(inf)

// Shift exponent into position.
        sll     a4, a4, 23

// Remove residual part.
        srl     a0, a5, 8

// Get excess significand bits used for rounding (the bits not stored into the result).
        sll     a5, a5, 24
        or      a5, a5, a2
        bgez    a5, L(no_tie)           // Less than 0.5 so no rounding
        sll     a5, a5, 1               // Shift off leading '1' bit leaving residual bits
        add     a0, a0, 1               // Greater or equal to 0.5 so round up
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        seqz    a5, a5                  // a5=0 if not exactly 0.5, a5=1 if exactly 0.5
        ANDNx   a0, a0, a5              // Break tie with round to even
#else
        bnez    a5, L(no_tie)           // Not exactly 0.5, so no tie to break
        and     a0, a0, ~1              // Break tie with round to even
#endif

// Combine significand with exponent and sign.
L(no_tie):
        add     a0, a0, a4
        ret

// Overflowed with the exponent set to 0xFF with correct sign.
// Clear significand bits to zero to generate a correctly-signed Inf.
L(inf):
        sll     a0, a4, 23
L(add_done):
        ret

// Adding +0 + +0 or -0 + -0, so return correctly-signed zero.
L(zero):
        srl     a0, a0, 31
        sll     a0, a0, 31
        ret

// The largest (now lhs) is Inf or NaN; if Inf, return, else convert to NaN.
L(add_inf_or_nan):
        sll     a2, a0, 9               // Discard sign and exponent, isolate significand
        beqz    a2, L(add_done)         // If Inf, return Inf

// Generate a NaN.
L(return_nan):
        li      a0, 0x7FC00000
        ret

// Signs differ so must do subtract.
L(subtract):

// Make signs the same in order to compare magnitudes.
        xor     a1, a1, a2

// Get largest of x and y into x, smallest into y.
        sub     a3, a0, a1
        bgeu    a0, a1, L(sub_already_ordered)  // x>y?
        xor     a3, a3, a2
        sub     a0, a0, a3
        add     a1, a1, a3
L(sub_already_ordered):

// Discard sign bit of smaller, top 8 bits of a3 are exponent.
// Extract exponent of larger into a4.
        BFOZ    a3, a1, 30, 23
        BFOZ    a4, a0, 30, 23

// Shift significand into position and set the hidden bit.
        sll     a1, a1, 8
        or      a1, a1, a2

// If the exponent of x (the largest) is Inf or NaN, handle it out of line
// as this is not the common case.
        li      t0, 0xFF
        beq     a4, t0, L(sub_inf_or_nan)

// If smaller is subnormal or zero, handle out of line.
        beqz    a3, L(sub_zero)

// Compute difference of exponents.  An extra unit is subtracted to
// differentiate a couple of distinguished cases.
        sub     a3, a4, a3

// If exactly aligned, subtract now.  Because the subtract (and thus compare)
// above compared the larger exponent and the smaller exponent, the only way
// that the carry can be clear is if the two exponents were equal on compare
// and the carry was subtracted leaving the borrow clear.
        beqz    a3, L(exponents_equal)

// If they have a difference of exactly one, fall through.  That is, if
// exponent(x) - exponent(y) - 1 != 0, i.e. exponent(x) - exponent(y) != 1.
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bnec    a3, 1, L(exponents_differ_by_more_than_1)
#else
        li      t0, 1
        bne     a3, t0, L(exponents_differ_by_more_than_1)
#endif

// Here the exponents differ by exactly one.
// Extract exponent and sign of bigger to a3.
        srl     a3, a0, 23

// Align significand of larger to msb--we drop the hidden bit, it
// doesn't matter.
        sll     a0, a0, 9

// Do the subtraction.  As the exponents differ by one, we can never
// generate a zero difference...
        sltu    t0, a0, a1
        sub     a0, a0, a1

// ...however, we will will either have some normalizations to do
// (e.g. 1000 - 0100 = 0100) or no normalization to do (1100 - 0100 = 1000).
        bnez    t0, L(normalization_steps)

// Here we have a zero-step normalization case (1100 - 0100, for instance).
// Move result exponent and sign into position.
        sll     a3, a3, 23

// Take a copy of the difference, aligned to bit 31.
        sll     a1, a0, 23

// Move significand into position.
        srl     a0, a0, 9

// As exponents differ by only a single bit, we have a single bit which
// determines rounding, as the significands are misaligned by a single bit...
// Move single bit to rounding bit to top.
        bgez    a1, L(sub_no_tie_single)         // Less than 0.5
        add     a0, a0, 1                        // Exactly 0.5
        and     a0, a0, ~1

// Insert exponent and sign, and done.
L(sub_no_tie_single):
        add     a0, a0, a3
L(sub_done):
        ret

// Here we subtract two values with equal exponents; these will cancel the leading
// bits at least, so we know we have at least one normalization step.
L(exponents_equal):

// Extract exponent and sign of bigger to a3.
        srl     a3, a0, 23

// Align significand of larger to msb--we drop the hidden bit, it
// doesn't matter.
        sll     a0, a0, 9

// Align significands and compute difference.
        sll     a1, a1, 1
        sub     a0, a0, a1

// If difference is exactly zero, we're done.
        beqz    a0, L(sub_done)

// The computed difference must now be normalized...  a1 contains the number
// of shifts required to normalize.  It would be nice to directly adjust the
// exponent in a3, but a3 also contains the result sign in bit 8, so detecting
// underflow is a problem (e.g. 0x108 with 10 places to shift generates 0xFE,
// which is the same as 0x0FE with 0 places to shift.)
L(normalization_steps):
        NORM32  a0, a1, a5
        add     a1, a1, 1

// Continue.
        sll     a0, a0, 1

// Will the result underflow?  If the exponent is very small and we needed
// a lot of steps in normalization, which will generate a zero or negative
// exponent, then the exponent will be too small and we can flush to zero.
        bleu    a4, a1, L(underflow)

// Adjust exponent by number of places shifted and move into position.
        sub     a3, a3, a1
        sll     a3, a3, 23

// Move significand into position.
        srl     a0, a0, 9

// Combine exponent and sign with significancd, and we're done.
        add     a0, a0, a3
        ret

// Result underflowed.  a3 contains the exponent and sign (9 bits), so...
L(underflow):
        srl     a0, a3, 8               // extract sign to lsb
        sll     a0, a0, 31              // move sign into position
        ret

// If the difference is big, come out immediately.  Subtracting a small
// number from a significantly larger one is no problem.
L(exponents_differ_by_more_than_1):
        li      t0, 25
        bgtu    a3, t0, L(sub_done)

// Adjust exponent for shifts.
        add     a3, a3, 1

// Shift a1 right by 32-a3 bits into a4--this is the residual that we're not subtracting.
// We don't have a register left to compute 32-a3, so synthesize it.  We also align
// the significand of the subtrahend (in a1) with that of the minuend (in a0).
        neg     a4, a3
        sll     a4, a1, a4
        srl     a1, a1, a3

// Fold bits into low order subtrahend; if any of the bits shifted out are non-zero,
// the we need to fold it into a1.
        snez    a4, a4                  // a4=1 if a4 >= 1, i.e. a4 != 0
        LEAH    a1, a4, a1              // a1 = a4 + 2*a1

// Extract sign and exponent of minuend into a3.
        srl     a3, a0, 23

// Align minuend significand to msb and insert the hidden bit.
        sll     a0, a0, 8
        or      a0, a0, a2

// Do the subtraction.  You might find this slightly strange that we're computing
// the difference into a4, but we do this in order that we can do rounding below at
// (*) et seq.
        sub     a4, a0, a1

// Correctly normalize result.  In this case we know that the subtraction can
// never need more than a single normalization step because the most significant
// bit can only change from a one to a zero as the exponents differ by 1 or more.
        bltz    a4, L(sub_already_normalized)

// Shift a4 left one bit to normalize and ensure that top bit is zero.  We do
// this by shifting two left and one right.
        sll     a4, a4, 2
        srl     a4, a4, 1
L(sub_already_normalized):

// Adjust exponent.
        add     a3, a3, -1

// Move exponent into position.
        sll     a3, a3, 23

// Move significand into position.  (*)
        srl     a0, a4, 8

// Generate rounding from remainder in lower order 8 bits of difference.
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbc     a4, 7, L(sub_no_tie)
        sll     a4, a4, 25
#else
        sll     a4, a4, 24
        bgez    a4, L(sub_no_tie)
        sll     a4, a4, 1
#endif
        add     a0, a0, 1
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE || __SEGGER_RTL_CORE_HAS_ANDN_ORN_XORN
        seqz    a2, a4
        ANDNx   a0, a0, a2
#else
        bnez    a4, L(sub_no_tie)
        and     a0, a0, ~1
#endif

// Insert exponent and sign and rounding bit.
L(sub_no_tie):
        add     a0, a0, a3

// Restore working register and return.
        ret

// LHS is Inf or NaN.
L(sub_inf_or_nan):
        li      t0, 0xFF                // rhs is Inf or NaN?
        beq     a3, t0, L(return_nan)   // Inf-Inf is NaN, Inf+NaN is NaN, NaN+Inf is NaN.

// RHS a NaN?
        sll     a1, a0, 9
        beqz    a1, L(sub_done)
        j       L(return_nan)

// Subtracting zero or subnormal.
L(sub_zero):
        bnez    a4, L(sub_done)         // non-zero-x - 0 is non-zero-x
        li      a0, 0                   // 0-0 is +0
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Incoming floats have undefined high-order 32 bits.
        sext.w  a0, a0
        sext.w  a1, a1

// Do addend and augend have the same sign?
        xor     a3, a0, a1
        bltz    a3, L(subtract)         // No, this is a subtraction

// Get largest of x and y into x, smallest into y.  Hence
// augend is largest.
        bgeu    a0, a1, L(add_already_ordered)
        mv      a3, a0
        mv      a0, a1
        mv      a1, a3
L(add_already_ordered):

// Extract exponent of smaller into a3, larger into a4.
        BFOZ    a4, a0, 30, 23
        BFOZ    a3, a1, 30, 23

// If the exponent of x (the largest) is Inf or NaN, return the largest.
        li      t0, 0xFF
        beq     a4, t0, L(add_inf_or_nan)

// If the exponent of the largest is zero, then we need to sort out adding -0.
        beqz    a4, L(zero)

// Extract exponent of smaller.  If adding zero or subnormal, then we're done.
        beqz    a3, L(add_done)

// Find difference between exponents which is the number of places
// needed to align them.  We cancel the sign bits in the subtraction
// because we know they are the same in addend and augend.
        sub     a3, a4, a3

// If the difference is big, come out immediately.
        li      t0, 24
        bgt     a3, t0, L(add_done)

// Generate 0x80000000'00000000.
        li      a2, 1
        sll     a2, a2, 63

// Isolate significand of augend into high 24 bits and set the hidden.
// bit.  We know this is never a subnormal.
        sll     a1, a1, 32+8
        or      a1, a1, a2

// Extract exponent and sign of larger.
        srl     a4, a0, 23

// Move significand of largest into high part and set hidden bit.
        sll     a0, a0, 32+8
        or      a0, a0, a2

// Compute number of bits to shift smaller to align significands for addition.
        li      a5, 25
        sub     a5, a5, a3

// Calculate residual that is not added to retained part of significand, but
// will be required for correct rounding.
        sll     a2, a1, a5
        srl     a2, a2, 7

// Align smaller significand.
        srl     a1, a1, a3

// Compute sum of significands.
        add     a5, a0, a1

// If we carried out, normalize sum.  a0 can never be zero.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltu    t0, a5, a0              // If no overflow, no change in state from the following two instructions
        srl     a5, a5, t0              // Bring in a zero, the hidden bit has transformed to a zero.
        LEAH    a4, a4, t0              // Increment exponent to compensate for the shift, and increment again to replace the lost hidden bit
#else
        bgeu    a5, a0, L(add_no_normalization)
        srl     a5, a5, 1               // Bring in a zero, the hidden bit has transformed to a zero.
        add     a4, a4, 2               // Increment exponent to compensate for the shift, and increment again to replace the lost hidden bit
#endif

// Move exponent into position.
L(add_no_normalization):
        add     a4, a4, -1              // Clear hidden bit from combined value by adjusting exponent down to compensate

// Check for exponent overflow, ignoring sign of significand.
        and     a1, a4, 0xFF
        addi    a1, a1, -0xFF
        beqz    a1, L(inf)

// Shift exponent into position.
        sll     a4, a4, 23

// Remove residual part.
        srl     a0, a5, 32+8

// Get excess significand bits used for rounding (the bits not stored into the result).
        sll     a5, a5, 24
        or      a5, a5, a2
        bgez    a5, L(no_tie)           // Less than 0.5 so no rounding
        sll     a5, a5, 1               // Shift off leading '1' bit leaving residual bits
        add     a0, a0, 1               // Greater or equal to 0.5 so round up
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        seqz    a5, a5                  // a5=0 if not exactly 0.5, a5=1 if exactly 0.5
        ANDNx   a0, a0, a5              // Break tie with round to even
#else
        bnez    a5, L(no_tie)           // Not exactly 0.5, so no tie to break
        and     a0, a0, ~1              // Break tie with round to even
#endif

// Combine significand with exponent and sign.
L(no_tie):
        add     a0, a0, a4
        ret

// Overflowed with the exponent set to 0xFF with correct sign.
// Clear significand bits to zero to generate a correctly-signed Inf.
L(inf):
        sll     a0, a4, 23
L(add_done):
        ret

// Adding +0 + +0 or -0 + -0, so return correctly-signed zero.
L(zero):
        srl     a0, a0, 31
        sll     a0, a0, 31
        ret

// The largest (now lhs) is Inf or NaN; if Inf, return, else convert to NaN.
L(add_inf_or_nan):
        sll     a2, a0, 32+12           // Discard sign and exponent, isolate significand
        beqz    a2, L(add_done)         // If Inf, return Inf

// Generate a NaN.
L(return_nan):
        li      a0, 0x7FC00000
        ret

// Signs differ so must do subtract.
L(subtract):

// Generate 0x80000000.
        li      a2, 1
        sll     a2, a2, 31

// Make signs the same in order to compare magnitudes.
        xor     a1, a1, a2
        sext.w  a1, a1

// Get largest of x and y into x, smallest into y.
        sub     a3, a0, a1
        bgeu    a0, a1, L(sub_already_ordered)  // x>y?
        xor     a3, a3, a2
        sub     a0, a0, a3
        add     a1, a1, a3
L(sub_already_ordered):

// Discard sign bit of smaller, top 8 bits of a3 are exponent.
// Extract exponent of larger into a4.
        BFOZ    a3, a1, 30, 23
        BFOZ    a4, a0, 30, 23

// Shift significand into position and set the hidden bit.
        sll     a2, a2, 32              // Generate 0x80000000'00000000
        sll     a1, a1, 32+8
        or      a1, a1, a2

// If the exponent of x (the largest) is Inf or NaN, handle it out of line
// as this is not the common case.
        li      t0, 0xFF
        beq     a4, t0, L(sub_inf_or_nan)

// If smaller is subnormal or zero, handle out of line.
        beqz    a3, L(sub_zero)

// Compute difference of exponents.  An extra unit is subtracted to
// differentiate a couple of distinguished cases.
        sub     a3, a4, a3

// If exactly aligned, subtract now.  Because the subtract (and thus compare)
// above compared the larger exponent and the smaller exponent, the only way
// that the carry can be clear is if the two exponents were equal on compare
// and the carry was subtracted leaving the borrow clear.
        beqz    a3, L(exponents_equal)

// If they have a difference of exactly one, fall through.  That is, if
// exponent(x) - exponent(y) - 1 != 0, i.e. exponent(x) - exponent(y) != 1.
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bnec    a3, 1, L(exponents_differ_by_more_than_1)
#else
        li      t0, 1
        bne     a3, t0, L(exponents_differ_by_more_than_1)
#endif

// Here the exponents differ by exactly one.
// Extract exponent and sign of bigger to r3.
        srl     a3, a0, 23

// Align significand of larger to msb--we drop the hidden bit, it
// doesn't matter.
        sll     a0, a0, 32+9

// Do the subtraction.  As the exponents differ by one, we can never
// generate a zero difference...
        sltu    t0, a0, a1
        sub     a0, a0, a1

// ...however, we will will either have some normalizations to do
// (e.g. 1000 - 0100 = 0100) or no normalization to do (1100 - 0100 = 1000).
        bnez    t0, L(normalization_steps)

// Here we have a zero-step normalization case (1100 - 0100, for instance).
// Move result exponent and sign into position.
        sll     a3, a3, 23

// Take a copy of the difference, aligned to bit 31.
        sll     a1, a0, 23

// Move significand into position.
        srl     a0, a0, 32+9

// As exponents differ by only a single bit, we have a single bit which
// determines rounding, as the significands are misaligned by a single bit...
// Move single bit to rounding bit to top.
        bgez    a1, L(sub_no_tie_single) // Less than 0.5
        add     a0, a0, 1                // Exactly 0.5
        and     a0, a0, ~1

// Insert exponent and sign, and done.
L(sub_no_tie_single):
        add     a0, a0, a3
L(sub_done):
        ret

// Here we subtract two values with equal exponents; these will cancel the leading
// bits at least, so we know we have at least one normalization step.
L(exponents_equal):

// Extract exponent and sign of bigger to a3.
        srl     a3, a0, 23

// Align significand of larger to msb--we drop the hidden bit, it
// doesn't matter.
        sll     a0, a0, 32+9

// Align significands and compute difference.
        sll     a1, a1, 1
        sub     a0, a0, a1

// If difference is exactly zero, we're done.
        beqz    a0, L(sub_done)

// The computed difference must now be normalized...  a1 contains the number
// of shifts required to normalize.  It would be nice to directly adjust the
// exponent in a3, but a3 also contains the result sign in bit 8, so detecting
// underflow is a problem (e.g. 0x108 with 10 places to shift generates 0xFE,
// which is the same as 0x0FE with 0 places to shift.)
L(normalization_steps):
        NORM64  a0, a1, a5
        add     a1, a1, 1

// Continue.
        sll     a0, a0, 1

// Will the result underflow?  If the exponent is very small and we needed
// a lot of steps in normalization, which will generate a zero or negative
// exponent, then the exponent will be too small and we can flush to zero.
        bleu    a4, a1, L(underflow)

// Adjust exponent by number of places shifted and move into position.
        sub     a3, a3, a1
        sll     a3, a3, 23

// Move significand into position.
        srl     a0, a0, 32+9

// Combine exponent and sign with significancd, and we're done.
        add     a0, a0, a3
        ret

// Result underflowed.  a3 contains the exponent and sign (9 bits), so...
L(underflow):
        srl     a0, a3, 8               // extract sign to lsb
        sll     a0, a0, 31              // move sign into position
        ret

// If the difference is big, come out immediately.  Subtracting a small
// number from a significantly larger one is no problem.
L(exponents_differ_by_more_than_1):
        li      t0, 25
        bgtu    a3, t0, L(sub_done)

// Adjust exponent for shifts.
        add     a3, a3, 1

// Shift a1 right by 32-a3 bits into a4--this is the residual that we're not subtracting.
// We don't have a register left to compute 32-a3, so synthesize it.  We also align
// the significand of the subtrahend (in a1) with that of the minuend (in a0).
        neg     a4, a3
        sll     a4, a1, a4
        srl     a1, a1, a3

// Fold bits into low order subtrahend; if any of the bits shifted out are non-zero,
// the we need to fold it into a1.
        snez    a4, a4                  // a4=1 if a4 >= 1, i.e. a4 != 0
        LEAH    a1, a4, a1              // a1 = a4 + 2*a1

// Extract sign and exponent of minuend into a3.
        srl     a3, a0, 23

// Align minuend significand to msb and insert the hidden bit.
        sll     a0, a0, 32+8
        or      a0, a0, a2

// Do the subtraction.  You might find this slightly strange that we're computing
// the difference into a4, but we do this in order that we can do rounding below at
// (*) et seq.
        sub     a4, a0, a1

// Correctly normalize result.  In this case we know that the subtraction can
// never need more than a single normalization step because the most significant
// bit can only change from a one to a zero as the exponents differ by 1 or more.
        bltz    a4, L(sub_already_normalized)

// Shift a4 left one bit to normalize and ensure that top bit is zero.  We do
// this by shifting two left and one right.
        sll     a4, a4, 2
        srl     a4, a4, 1
L(sub_already_normalized):

// Adjust exponent.
        add     a3, a3, -1

// Move exponent into position.
        sll     a3, a3, 23

// Move significand into position.  (*)
        srl     a0, a4, 32+8

// Generate rounding from remainder in lower order 8 bits of difference.
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbc     a4, 7, L(sub_no_tie)
        sll     a4, a4, 25
#else
        sll     a4, a4, 24
        bgez    a4, L(sub_no_tie)
        sll     a4, a4, 1
#endif
        add     a0, a0, 1
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE || __SEGGER_RTL_CORE_HAS_ANDN_ORN_XORN
        seqz    a4, a4
        ANDNx   a0, a0, a4
#else
        bnez    a4, L(sub_no_tie)
        and     a0, a0, ~1
#endif

// Insert exponent and sign and rounding bit.
L(sub_no_tie):
        add     a0, a0, a3

// Restore working register and return.
        ret

// LHS is Inf or NaN.
L(sub_inf_or_nan):
        li      t0, 0xFF                // rhs is Inf or NaN?
        beq     a3, t0, L(return_nan)   // Inf-Inf is NaN, Inf+NaN is NaN, NaN+Inf is NaN.

// RHS a NaN?
        sll     a1, a0, 32+9
        beqz    a1, L(sub_done)
        j       L(return_nan)

// Subtracting zero or subnormal.
L(sub_zero):
        bnez    a4, L(sub_done)         // non-zero-x - 0 is non-zero-x
        li      a0, 0                   // 0-0 is +0
        ret

#else

#error Bad configuration

#endif

END_FUNC __addsf3

/*********************************************************************
*
*       __adddf3()
*
*  Function description
*    Add, double.
*
*  Prototype
*    double __adddf3(double x, double y);
*
*  Parameters
*    a1:a0 / a0 - x - Augend.
*    a3:a2 / a1 - y - Addend.
*
*  Return value
*    a1:a0 / a0 - Sum.
*
*  Configuration
*    If __SEGGER_RTL_OPTIMIZE < 0 then a shorter sequence is used to order
*    the summands such that x > y; else a longer sequence is used that
*    has slightly better performance.
*
*  Register use (RV32)
*    a4 - Difference between larger and smaller exponents, i.e. alignment shift count.
*    a5 - Constant 0x80000000.
*    a6 - Exponent of sum.
*    a7 - Scratch then sticky bit bucket on addition path and normalization mask on subtraction path.
*    t0 - Temporary register for immediate compares and flag register for carry.
*    t1 - Temporary register for auxiliary carry.
*    t2 - Temporary stash for sign and exponent.
*/

#undef L
#define L(label) .L__adddf3_##label

GLOBAL_FUNC __adddf3

#if __SEGGER_RTL_FP_ABI >= 2

        fadd.d  fa0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32 
//

#if __SEGGER_RTL_CORE_HAS_ISA_RVE
        add     sp, sp, -STACK_ALIGN(8)
        sw      s0, 0(sp)
        sw      s1, 4(sp)
#define a6 s0
#define a7 s1
#endif

// Generate 0x80000000
        li      a5, 0x80000000

#if __SEGGER_RTL_OPTIMIZE < 0

// Prefer a smaller image

// Get single-bit "different sign" flag into a7{31}.
        xor     a7, a1, a3
        and     a7, a7, a5

// Make summand signs identical as we wish to compare magnitudes.
        xor     a3, a3, a7

// Get largest in magnitude of x and y into x, smallest into y.
// Signs identical: 64-bit compare of magnitudes of addend and augend.
        bltu    a3, a1, L(already_ordered)
        bne     a3, a1, L(must_exchange)
        bltu    a2, a0, L(already_ordered)

// Addend is greater than augend: swap addend and augend.
L(must_exchange):
        xor     a3, a3, a7              // Restore sign of larger, this is the sign of the difference
        xor     a0, a0, a2
        xor     a2, a2, a0
        xor     a0, a0, a2
        xor     a1, a1, a3
        xor     a3, a3, a1
        xor     a1, a1, a3

// If subtraction, do subtraction.
L(already_ordered):
        bltz    a7, L(subtract)

#else

// If subtraction, do subtraction.
        xor     a7, a1, a3
        bltz    a7, L(subtract)

// Signs identical: pseudo-64-bit compare of magnitudes of addend and augend.
// We don't need to test the lower-order bits of addend and augend for
// a strict ordering, we just need to make sure that the operand with
// the largest exponent ends up in a1:a0.
        bltu    a3, a1, L(add_already_ordered)

// Addend is greater than augend: swap addend and augend.
        xor     a0, a0, a2
        xor     a2, a2, a0
        xor     a0, a0, a2
        xor     a1, a1, a3
        xor     a3, a3, a1
        xor     a1, a1, a3

L(add_already_ordered):

#endif

// Extract exponent of lhs to a6 and rhs to a7.
        BFOZ    a6, a1, 30, 20
        BFOZ    a7, a3, 30, 20

// If rhs exponent is zero, we're adding zero (including subnormals).
        beqz    a7, L(add_zero)

// If lhs exponent is Inf or NaN, then return that.
        add     a4, a6, 1               // Topple 0x7FF to 0x800
        sll     a4, a4, 21              // shift off sign bit, if zero then input exponent was 0x7FF
        beqz    a4, L(done)

// Compute difference in exponents.
        sub     a4, a6, a7

// If we can't align the exponents, the addend does not contribute to the sum.
        li      t0, 53
        bgtu    a4, t0, L(done)

// Extract smaller exponent.
        srl     a6, a1, 20

// Materialize hidden bit in smaller summand.
        sll     a3, a3, 11
        or      a3, a3, a5
        srl     a3, a3, 11

// Materialize hidden bit in larger summand.
        sll     a1, a1, 11
        or      a1, a1, a5
        sra     a1, a1, 11              // This is an SRA to drag in one bits from the left.
                                        // We can use this to determine whether the sum is
                                        // normalized

// When we are aligning significands, the extra bits shifted off are captured
// in a7 to allow us to round correctly.

// Exponents are aligned more than 32 bits apart?
        li      t0, 32
        bgeu    a4, t0, L(add_shifted_word)
        li      a7, 0                   // Prepare a7 as 0 in case no shift happens which means no bits shifted out
        beqz    a4, L(add_no_shift)

// Long logical shift right a3:a2 by a4 places capturing the shifted-out
// bits into a7. Exponents differ by less than 32 so we have a maximum 31
// places to shift.
        neg     t0, a4
        sll     a7, a2, t0
        srl     a2, a2, a4
        sll     t0, a3, t0
        add     a2, a2, t0
        srl     a3, a3, a4

// Compute sum.
L(add_no_shift):
        add     a0, a0, a2
        sltu    t0, a0, a2              // carry out of low addition
        add     a1, a1, a3
        sltu    t1, a1, a3              // carry out of high addition
        add     a1, a1, t0              // propagate carry from low addition
        bnez    t1, L(normalization_required)
        bgeu    a1, t0, L(already_normalized)

// Addition requires normalization as it overflowed.
L(normalization_required):

// Exponent overflowed?  Compare to 0x7FE by adding 2 and seeing if the result is zero modulo 0x800.
        add     a2, a6, 2
        sll     a2, a2, 21
        beqz    a2, L(inf)

// We are going to shift a1:a0 right by one bit and shift off a single bit.
// We also wish to maintain the sticky bits already held in a7.  To do this,
// we move the bit shifted off into the msb of the sticky bit bin and compress
// the existing bits in the sticky bit bin (to a single bit in the lsb).
        sll     a2, a0, 31              // Shift lsb of significand to (virtual) msb of sticky bit bin for rounding
        snez    t0, a7                  // t0=0 iff sticky bit bin == 0, 1 otherwise
        add     a7, a2, t0              // Combine compressed sticky bit with rounding bit shifted out

// Shift a1:a0 right one bit.
        srl     a0, a0, 1
        sll     a3, a1, 31
        or      a0, a0, a3
        srl     a1, a1, 1

// Move exponent into position.
L(already_normalized):
        add     a6, a6, 1               // Adjust exponent bias; sign already present in a6.
        sll     a6, a6, 20              // Move exponent into position

// Perform rounding.
L(perform_rounding):
        bgez    a7, L(add_no_tie)       // Low-order signifand bits are less than 0.5
        add     a0, a0, 1               // >= 0.5 so round up
        seqz    t0, a0                  // carry out...
        add     a1, a1, t0              // ...into high word
        sll     a7, a7, 1               // Test for exactly 0.5
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE || __SEGGER_RTL_CORE_HAS_ANDN_ORN_XORN
        seqz    a7, a7
        ANDNx   a0, a0, a7
#else
        bnez    a7, L(add_no_tie)       // Not 0.5, so rounding stands
        and     a0, a0, ~1              // Round to nearest even
#endif

// Merge exponent.
L(add_no_tie):
        add     a1, a1, a6

// All done.
L(done):

#if __SEGGER_RTL_CORE_HAS_ISA_RVE
        lw      s0, 0(sp)
        lw      s1, 4(sp)
        add     sp, sp, +STACK_ALIGN(8)
#endif

        ret

// Exponents differ by 32 bits or more.
L(add_shifted_word):

// Initialize sticky bit bin with lower half of significand as we know
// we must shift anyway.
        mv      a7, a2

// If exponents differ by exactly 32, then a7 is already primed
// as the sticky bit bin and we just need to add the shifted
// significand.
        add     a4, a4, -32
        beqz    a4, L(already_aligned)

// Need to shift a3:a2 right by 32+a4 bits to align significands.
// When shifting, we catch the bits shifted out in the high order
// bits of a7.
        neg     a7, a4
        sll     a7, a3, a7
        srl     a3, a3, a4
        snez    a4, a2
        add     a7, a7, a4

// Form sum.
L(already_aligned):
        add     a0, a0, a3
        sltu    t0, a0, a3              // t0 = carry out from low 32 bits
        add     a1, a1, t0              // carry from low word to high word
        bgeu    a1, t0, L(already_normalized)
        j       L(normalization_required)

// We know the rhs is to be treated as zero.  If the lhs is non-zero, then
// return the lhs.
L(add_zero):
        bnez    a6, L(done)             // if lhs exponent is non-zero then we have a normal

// 0 + 0 = 0 and -0 + -0 = -0, so extract sign.
        and     a1, a1, a5
        li      a0, 0
        j       L(done)

// Generate an infinity.
L(inf):
        add     a6, a6, 1               // a6 is now sign + exponent of Inf
        sll     a1, a6, 20              // move exponent and sign into position
        li      a0, 0                   // Inf requires zero significand
        j       L(done)

// Know lhs is Inf or NaN.  If rhs is Inf or NaN then the result is NaN
// as Inf-Inf is NaN and any NaN operand generates a NaN.
L(sub_inf_nan):
        bne     a7, a4, L(done)

// Generate a NaN.
L(nan):
        li      a1, 0x7FF80000
        li      a0, 0
        j       L(done)

// Signs differ so must do subtract.
L(subtract):

#if __SEGGER_RTL_OPTIMIZE >= 0

// Make signs the same as we wish to compare magnitudes.
        xor     a3, a3, a5

// Get largest in magnitude of x and y into x, smallest into y.
// Signs identical: 64-bit compare of magnitudes of addend and augend.
        bltu    a3, a1, L(sub_already_ordered)
        bne     a3, a1, L(sub_must_exchange)
        bltu    a2, a0, L(sub_already_ordered)

// Addend is greater than augend: swap addend and augend.
L(sub_must_exchange):
        xor     a3, a3, a5              // Restore sign of larger, this is the sign of the difference
        xor     a0, a0, a2
        xor     a2, a2, a0
        xor     a0, a0, a2
        xor     a1, a1, a3
        xor     a3, a3, a1
        xor     a1, a1, a3
#endif

// Discard signs ready to compare exponents to special Inf/NaN exponent.
L(sub_already_ordered):
        add     a6, a1, a1              // Cast out lhs sign, ditto for rhs
        add     a7, a3, a3

// Check for Inf and NaN.
        li      a4, 0xFFE00000
        bgeu    a6, a4, L(sub_inf_nan)

// Know that lhs and rhs are both normal.  Extract exponents of lhs and rhs.
        srl     a6, a6, 21              // exponent of larger
        srl     a7, a7, 21              // exponent of smaller
        beqz    a7, L(subtracting_zero) // rhs is zero on subtract

// Compute difference in exponents which is the number of bits to align significands.
        sub     a4, a6, a7
        li      t0, 54
        bgtu    a4, t0, L(done)

// Save sign and exponent.
        mv      t2, a6
        srl     a6, a1, 20

// Materialize hidden bit in rhs.
        sll     a3, a3, 11
        or      a3, a3, a5
        srl     a3, a3, 11

// Materialize hidden bit in lhs.
        sll     a1, a1, 11
        or      a1, a1, a5
        srl     a1, a1, 11

// Special-case some subtractions.  We have distinguish three cases:
// exponents differ by 0, 1, or more than one which use different paths
// through the code.  In the case of 0 or 1, we have the issue of
// leading digit cancellation and normalization, with 2 and above, we don't.
        li      t0, 1
        bgtu    a4, t0, L(sub_align_far)
        bne     a4, t0, L(sub_already_aligned)

// Difference in exponents is one, so align by a single bit and use a4 as
// a significand extension register.
        sll     a4, a2, 31              // a4{31} is the extended significand, all other bits of a4 are zero
        srl     a2, a2, 1               // move low down, a2{31} zero
        sll     a7, a3, 31              // extract lsb of high part to a7{31}
        or      a2, a2, a7              // merge with low part
        srl     a3, a3, 1               // and finally shift high part down to align

// Exponents are correctly aligned.  When we subtract now we can cause
// cancellation of leading bits and require normalization.
L(sub_already_aligned):

// Subtract; a4 holds a single bit in a4{31} that is the result of a
// a single-bit normalization.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        sub64   a0, a0, a2
#else
        mv      t0, a0
        sub     a0, a0, a2              // Subtract low parts
        sltu    t0, t0, a0              // Carry out of subtraction
        sub     a1, a1, a3              // Subtract high parts
        sub     a1, a1, t0              // Subtract carry
#endif
        beqz    a4, L(sub_single_done)
        seqz    t0, a0
        add     a0, a0, -1
        sub     a1, a1, t0

// If the high word of the difference significand is zero, we may
// be able to shift quickly.  We assume that cancellation of
// leading digits is unlikely and move it off the mainline.
L(sub_single_done):
        beqz    a1, L(high_word_cancelled)

// If the significand is already normalized, then we must round as we have
// a rounding bit left in a4{31}.
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbs     a1, 31-11, L(sub_normalized)
#else
        sll     t0, a1, 11
        bltz    t0, L(sub_normalized)
#endif

// This is the first normalization step, and we have a single bit in the
// subtrahend's signifcand extension register a4.  In this case we don't need
// to break any ties or consider round-to-nearest rounding.
L(first_normalization_step):

// Shift left a1:a0.<a4> one bit, knowing a4 is either 0x80000000 or 0x00000000
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a0
#else
        sltz    t0, a0                  // Carry out from low to high
        add     a0, a0, a0              // Shift a1:a0 left one...
        LEAH    a1, t0, a1              // ...carrying low to high
#endif
        srl     a4, a4, 31              // Shift in a4{31} knowing a0{0} is zero
        add     a0, a0, a4
        li      a4, 1                   // one normalization step done, set normalization count.

#if __SEGGER_RTL_OPTIMIZE >= 0

L(try_shift_4):
        srl     t0, a1, 21-4
        bnez    t0, L(cant_shift_4)
        add     a4, a4, 4
        sll     a1, a1, 4
        srl     t0, a0, 32-4
        sll     a0, a0, 4
        add     a1, a1, t0
        j       L(try_shift_4)

#endif

// If we're already normalized, that's great.
L(cant_shift_4):
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbs     a1, 31-11, L(normalized)
#else
        sll     t0, a1, 11
        bltz    t0, L(normalized)
#endif

// Perform bit-by-bit normalization of a1:a0, keeping normalization count in a4.
L(normalize):
        add     a4, a4, 1               // keep count of normalization steps required, one bit at a time...
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a0
#else
        sltz    t0, a0                  // generate carry out
        add     a0, a0, a0              // low part
        LEAH    a1, t0, a1              // high part
#endif
L(pre_normalize):
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbc     a1, 31-11, L(normalize)
#else
        sll     t0, a1, 11              // normalized?
        bgez    t0, L(normalize)        // ...not yet
#endif

// Now have significand normalized in a1:a0.  If the normalization count is less than the
// input exponent, we have a subnormal.  In this case we flush to an appropriately-signed zero.
L(normalized):
        mv      a2, t2
        bleu    a2, a4, L(signed_zero)

// Adjust exponent by number of places required to normalize difference's significand.
        sub     a6, a6, a4

// Move exponent into position put into result.
        add     a6, a6, -1              // adjust exponent as we have a '1' in the hidden bit which is the lsb of the exponent
        sll     a6, a6, 20              // shift into position
        add     a1, a1, a6              // merge with significand, and adjust by one because of exponent...
        j       L(done)

// Return a signed zero.  The sign bit is stored in a6[11].
L(signed_zero):
        srl     a1, a6, 11              // sign bit to a1[0].
        sll     a1, a1, 31              // ...into position
        li      a0, 0                   // low significand is zero
        j       L(done)                 // ...and done

// Subtracting zero.  0-0 is always +ve zero.
L(subtracting_zero):

// Is lhs zero (include subnormals)?  If it is then we generate a +ve zero
// otherwise we return a positive zero, clearing out subnormals.
        bnez    a6, L(done)

// Return a +ve zero for 0-0 for any sign of zero operands.
        li      a0, 0
        li      a1, 0
        j       L(done)

// High word has become zero, see if the low word can be moved into the
// high word.  We have to make sure that we don't "over normalize".
L(high_word_cancelled):

// If we have generated a zero, we're done and return an appropriate
// zero, otherwise we need to normalize.
        or      a2, a0, a4
        beqz    a2, L(done)

// Can we move the low word to the high word without over-normalizing?
// If not, do so, we've had at least 32 bits of leading digit cancellation...
        li      a7, 0x00100000                          // ENHANCEMENT: Could shift a0 right and test for nonzero
        bgeu    a0, a7, L(first_normalization_step)

// At least 32 leading zero bits have been generated across a1:a0.  In
// this case we shift the difference's significand by 32 bits immediately.
        mv      a1, a0
        mv      a0, a4
        li      a4, 32                  // Normalization count is 32, we've just shifted 32 bits
        j       L(pre_normalize)

// Difference in exponents is 2 or more bits.  In this case leading digit cancellation
// is not an issue.  As the larger sigificand has its msb set, and the smaller significand
// has its leading two bits clear, we do not expect any more than one normalization step.
// For instance, subtracting the least 1000...-0011... = 0101... which requires one
// step, and 1111...-0010... = 1101... which requires zero normalization steps.
L(sub_align_far):
        li      t0, 32
        blt     a4, t0, L(aligned_on_top)
        beq     a4, t0, L(word_aligned_on_top)

// This is now what we wish to subtract, in general, with the exponent difference
// between the minuend and subtrahend as 32+a4

//   +--------------+--------------+---------------+
//   |      a1      |      a0      |000000000000000|
//   +--------------+--------------+----+----------*----+
// -                     |      a3      |      a2  |****|
//                       +--------------+----------*----+

// We can easily shift by 32 bits by changing registers, and adjust the shift
// count by 32 to perform alignment of a3 with a0.  Because we must round correctly,
// we need to compute the 96-bit difference with a1:a0 extended with zero bits
// and the low-order bits of the difference are computed into a7.
//
// In the diagram above, the bits marked with stars are shifted off the end and
// caught as sticky bits.

// Adjust shift count as we'll add high to low which is an implicit shift by 32.
        add     a4, a4, -32

// Shift a3:a2 right a4 places to align significands, and catch bits shifted out
// into high bits of a7.  a7 is will contain the low order bits of the subtrahend
// which align to the right.
        neg     t0, a4
        srl     t1, a2, a4
        sll     a7, a2, t0
        sll     a2, a3, t0
        add     a2, a2, t1
        srl     a3, a3, a4

// We now have
//
//   +--------------+--------------+---------------+
//   |      a1      |      a0      |000000000000000|
//   +--------------+--------------+---------------*------------+
// -                |0000    a3    |      a2       |**** a7 0000|
//                  +--------------+---------------*------------+
//
// We need to fold the sticky bits in a7 into a2.  That is, if a7 is non-zero,
// we need to set the lsb of a2, and if a7 is zero, then no change to a2.
        snez    a7, a7
        or      a7, a7, a2              // a7 now takes the place of a2, it holds the sticky bits
        li      a2, 0

// Subtract (<0>:a3.<0>) from (a1:a0.<0>) and place rounding bits into a4.
        mv      t0, a0
        sub     a0, a0, a3
        sltu    t0, t0, a0              // Carry out of subtraction
        sub     a1, a1, t0

// Subtract (<0>:<0>.<a7>) from (a1:a0.<0>) and place rounding bits into a4.
        neg     a4, a7
        beqz    a4, L(sub_normalize)
        seqz    t0, a0
        add     a0, a0, -1
        sub     a1, a1, t0
        j       L(sub_normalize)

// This is now what we wish to subtract, in general, with the exponent difference
// between the minuend and subtrahend as 32+a4
//
//   +--------------+--------------+---------------+
//   |      a1      |      a0      |000000000000000|
//   +--------------+--------------+----+----------+
// -      |      a3      |      a2 |******|
//        +--------------+---------*------+
//
// In the diagram above, the bits marked with stars are shifted off the end and
// caught as sticky bits in a7.

// Shift a3:a2 right a4 places to align significands, and catch bits shifted out
// into high bits of a7.  a7 is will contain the low order bits of the subtrahend
// which align to the right.

// Shift exactly 32 bits...
L(word_aligned_on_top):
        mv      a7, a2
        mv      a2, a3
        li      a3, 0
        j       L(aligned_subtract)

// Shift fewer than 32 bits...
L(aligned_on_top):
        neg     t0, a4
        srl     t1, a2, a4
        sll     a7, a2, t0
        sll     a2, a3, t0
        add     a2, a2, t1
        srl     a3, a3, a4

// Now have:

//   +--------------+--------------+-------------+
//   |      a1      |      a0      |0000000000000|
//   +--------------+--------------+-------------+
// - |0000    a3    |      a2      |**** a7 00000|
//   +--------------+--------------*-------------+

// Do subtraction above and place rounding bits into a4.
L(aligned_subtract):

// Subtract (a3:a2.<0>) from (a1:a0.<0>) and place rounding bits into a4.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        sub64   a0, a0, a2
#else
        mv      t0, a0
        sub     a0, a0, a2
        sltu    t0, t0, a0              // Carry out of subtraction
        sub     a1, a1, a3
        sub     a1, a1, t0
#endif

// Subtract (<0>:<0>.<a7>) from (a1:a0.<0>) and place rounding bits into a4.
        neg     a4, a7                  // 0 - * above
        beqz    a4, L(sub_normalize)
        seqz    t0, a0
        add     a0, a0, -1
        sub     a1, a1, t0

// Test to see whether we're normalized (bit 21)
L(sub_normalize):
        sll     a7, a1, 12
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbs     a1, 31-11, L(sub_normalized)
#else
        sll     t0, a1, 11
        bltz    t0, L(sub_normalized)
#endif

// Adjust exponent and normalize one place.  This doubles a1:a0:a4.
        add     a6, a6, -1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a0
#else
        sltz    t0, a0                  // Double a1:a0, this is carry from low to high
        add     a0, a0, a0              // Double a1:a0 without carry
        LEAH    a1, t0, a1              // Carry into high
#endif
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        sltz    t1, a4                  // Now double sub-low-order bits in a4 and ripple through
        add     a4, a4, a4
        li      t2, 0
        add64   a0, a0, t1
#else
        sltz    t0, a4                  // Now double sub-low-order bits in a4 and ripple through
        add     a4, a4, a4
        add     a0, a0, t0              // Add to low order bits
        sltu    t0, a0, t0              // Compute carry to high order
        add     a1, a1, t0              // Accumulate into high order
#endif

// Adjust exponent by 1 as we already have a '1' in the exponent position
// because we've normalized and have the hidden bit materialized (in the lsb
// of the exponent).
L(sub_normalized):
        add     a6, a6, -1

// Move exponent into position
        sll     a6, a6, 20

// Round; we have the rounding bits in a4 but reuse the add path and put rounding in r7.
        mv      a7, a4
        j       L(perform_rounding)

#if __SEGGER_RTL_CORE_HAS_ISA_RVE
#undef a6
#undef a7
#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Load Inf/NaN exponent and generate 0x80000000'00000000.
        li      t0, 0x7FF
        sll     a2, t0, 63

// Do addend and augend have the same sign?
        xor     a3, a0, a1
        bltz    a3, L(subtract)         // No, this is a subtraction

// Get largest of x and y into x, smallest into y.  Hence
// augend is largest.
        bgeu    a0, a1, L(add_already_ordered)
        mv      a3, a0
        mv      a0, a1
        mv      a1, a3
L(add_already_ordered):

// Extract exponent of smaller into a3, larger into a4.
        BFOZ    a4, a0, 62, 52
        BFOZ    a3, a1, 62, 52

// If the exponent of x (the largest) is Inf or NaN, return the largest.
        li      t0, 0x7FF
        beq     a4, t0, L(add_inf_or_nan)

// If the exponent of the largest is zero, then we need to sort out adding -0.
        beqz    a4, L(zero)

// Extract exponent of smaller.  If adding zero or subnormal, then we're done.
        beqz    a3, L(add_done)

// Find difference between exponents which is the number of places
// needed to align them.  We cancel the sign bits in the subtraction
// because we know they are the same in addend and augend.
        sub     a3, a4, a3

// If the difference is big, come out immediately.
        li      t0, 53
        bgt     a3, t0, L(add_done)

// Isolate significand of augend into high 53 bits and set the hidden.
// bit.  We know this is never a subnormal.
        sll     a1, a1, 11
        or      a1, a1, a2

// Extract exponent and sign of larger.
        srl     a4, a0, 52

// Move significand of largest into high part and set hidden bit.
        sll     a0, a0, 11
        or      a0, a0, a2

// Compute number of bits to shift smaller to align significands for addition.
        li      a5, 54
        sub     a5, a5, a3

// Calculate residual that is not added to retained part of significand, but
// will be required for correct rounding.
        sll     a2, a1, a5
        srl     a2, a2, 7

// Align smaller significand.
        srl     a1, a1, a3

// Compute sum of significands.
        add     a5, a0, a1

// If we carried out, normalize sum.  a0 can never be zero.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltu    t0, a5, a0              // If no overflow, no change in state from the following two instructions
        srl     a5, a5, t0              // Bring in a zero, the hidden bit has transformed to a zero.
        LEAH    a4, a4, t0              // Increment exponent to compensate for the shift, and increment again to replace the lost hidden bit
#else
        bgeu    a5, a0, L(add_no_normalization)
        srl     a5, a5, 1               // Bring in a zero, the hidden bit has transformed to a zero.
        add     a4, a4, 2               // Increment exponent to compensate for the shift, and increment again to replace the lost hidden bit
#endif

// Move exponent into position.
L(add_no_normalization):
        add     a4, a4, -1              // Clear hidden bit from combined value by adjusting exponent down to compensate

// Check for exponent overflow, ignoring sign of significand.
        and     a1, a4, 0x7FF
        addi    a1, a1, -0x7FF
        beqz    a1, L(inf)

// Shift exponent into position.
        sll     a4, a4, 52

// Remove residual part.
        srl     a0, a5, 11

// Get excess significand bits used for rounding (the bits not stored into the result).
        sll     a5, a5, 53
        or      a5, a5, a2
        bgez    a5, L(no_tie)           // Less than 0.5 so no rounding
        sll     a5, a5, 1               // Shift off leading '1' bit leaving residual bits
        add     a0, a0, 1               // Greater or equal to 0.5 so round up
        bnez    a5, L(no_tie)           // Not exactly 0.5, so no tie to break
        and     a0, a0, ~1              // Break tie with round to even

// Combine significand with exponent and sign.
L(no_tie):
        add     a0, a0, a4
        ret

// Overflowed with the exponent set to 0x7FF with correct sign.
// Clear significand bits to zero to generate a correctly-signed Inf.
L(inf):
        sll     a0, a4, 52
L(add_done):
        ret

// Adding +0 + +0 or -0 + -0, so return correctly-signed zero.
L(zero):
        srl     a0, a0, 63
        sll     a0, a0, 63
        ret

// The largest (now lhs) is Inf or NaN; if Inf, return, else convert to NaN.
L(add_inf_or_nan):
        sll     a2, a0, 1               // Discard sign
        li      t0, -1
        sll     t0, t0, 53
        beq     a2, t0, L(add_done)     // Yes, return Inf

// Generate a NaN.
L(return_nan):
        li      a0, -1
        sll     a0, a0, 52
        srl     a0, a0, 1
        ret

// Signs differ so must do subtract.
L(subtract):

// Make signs the same in order to compare magnitudes.
        xor     a1, a1, a2

// Get largest of x and y into x, smallest into y.
        sub     a3, a0, a1
        bgeu    a0, a1, L(sub_already_ordered)  // x>y?
        xor     a3, a3, a2
        sub     a0, a0, a3
        add     a1, a1, a3
L(sub_already_ordered):

// Discard sign bit of smaller, top 12 bits of a3 are exponent.
// Extract exponent of larger into a4.
        BFOZ    a3, a1, 62, 52
        BFOZ    a4, a0, 62, 52

// Shift significand into position and set the hidden bit.
        sll     a1, a1, 11
        or      a1, a1, a2

// If the exponent of x (the largest) is Inf or NaN, handle it out of line
// as this is not the common case.
        beq     a4, t0, L(sub_inf_or_nan)

// If smaller is subnormal or zero, handle out of line.
        beqz    a3, L(sub_zero)

// Compute difference of exponents.  An extra unit is subtracted to
// differentiate a couple of distinguished cases.
        sub     a3, a4, a3

// If exactly aligned, subtract now.  Because the subtract (and thus compare)
// above compared the larger exponent and the smaller exponent, the only way
// that the carry can be clear is if the two exponents were equal on compare
// and the carry was subtracted leaving the borrow clear.
        beqz    a3, L(exponents_equal)

// If they have a difference of exactly one, fall through.  That is, if
// exponent(x) - exponent(y) - 1 != 0, i.e. exponent(x) - exponent(y) != 1.
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bnec    a3, 1, L(exponents_differ_by_more_than_1)
#else
        li      t0, 1
        bne     a3, t0, L(exponents_differ_by_more_than_1)
#endif

// Here the exponents differ by exactly one.
// Extract exponent and sign of bigger to r3.
        srl     a3, a0, 52

// Align significand of larger to msb--we drop the hidden bit, it
// doesn't matter.
        sll     a0, a0, 12

// Do the subtraction.  As the exponents differ by one, we can never
// generate a zero difference...
        sltu    t0, a0, a1
        sub     a0, a0, a1

// ...however, we will will either have some normalizations to do
// (e.g. 1000 - 0100 = 0100) or no normalization to do (1100 - 0100 = 1000).
        bnez    t0, L(normalization_steps)

// Here we have a zero-step normalization case (1100 - 0100, for instance).
// Move result exponent and sign into position.
        sll     a3, a3, 52

// Take a copy of the difference, aligned to bit 31.
        sll     a1, a0, 52

// Move significand into position.
        srl     a0, a0, 12

// As exponents differ by only a single bit, we have a single bit which
// determines rounding, as the significands are misaligned by a single bit...
// Move single bit to rounding bit to top.
        bgez    a1, L(sub_no_tie_single) // Less than 0.5
        add     a0, a0, 1                // Exactly 0.5
        and     a0, a0, ~1

// Insert exponent and sign, and done.
L(sub_no_tie_single):
        add     a0, a0, a3
L(sub_done):
        ret

// Here we subtract two values with equal exponents; these will cancel the leading
// bits at least, so we know we have at least one normalization step.
L(exponents_equal):

// Extract exponent and sign of bigger to r3.
        srl     a3, a0, 52

// Align significand of larger to msb--we drop the hidden bit, it
// doesn't matter.
        sll     a0, a0, 12

// Align significands and compute difference.
        sll     a1, a1, 1
        sub     a0, a0, a1

// If difference is exactly zero, we're done.
        beqz    a0, L(sub_done)

// The computed difference must now be normalized...  a1 contains the number
// of shifts required to normalize.
L(normalization_steps):
        NORM64  a0, a1, a5
        add     a1, a1, 1

// Continue.
        sll     a0, a0, 1

// Will the result underflow?  If the exponent is very small and we needed
// a lot of steps in normalization, which will generate a zero or negative
// exponent, then the exponent will be too small and we can flush to zero.
        bleu    a4, a1, L(underflow)

// Adjust exponent by number of places shifted and move into position.
        sub     a3, a3, a1
        sll     a3, a3, 52

// Move significand into position.
        srl     a0, a0, 12

// Combine exponent and sign with significancd, and we're done.
        add     a0, a0, a3
        ret

// Result underflowed.  a3 contains the exponent and sign (9 bits), so...
L(underflow):
        srl     a0, a3, 11              // extract sign to lsb
        sll     a0, a0, 63              // move sign into position
        ret

// If the difference is big, come out immediately.  Subtracting a small
// number from a significantly larger one is no problem.
L(exponents_differ_by_more_than_1):
        li      t0, 54
        bgtu    a3, t0, L(sub_done)

// Adjust exponent for shifts.
        add     a3, a3, 1

// Shift r1 right by 32-r3 bits into r4--this is the residual that we're not subtracting.
// We don't have a register left to compute 32-r3, so synthesize it.  We also align
// the significand of the subtrahend (in r1) with that of the minuend (in r0).
        neg     a4, a3
        sll     a4, a1, a4
        srl     a1, a1, a3

// Fold bits into low order subtrahend; if any of the bits shifted out are non-zero,
// the we need to fold it into r1.
        snez    a4, a4                  // a4=1 if a4 >= 1, i.e. a4 != 0
        LEAH    a1, a4, a1              // a1 = a4 + 2*a1

// Extract sign and exponent of minuend into a3.
        srl     a3, a0, 52

// Align minuend significand to msb and insert the hidden bit.
        sll     a0, a0, 11
        or      a0, a0, a2

// Do the subtraction.  You might find this slightly strange that we're computing
// the difference into a4, but we do this in order that we can do rounding below at
// (*) et seq.
        sub     a4, a0, a1

// Correctly normalize result.  In this case we know that the subtraction can
// never need more than a single normalization step because the most significant
// bit can only change from a one to a zero as the exponents differ by 1 or more.
        bltz    a4, L(sub_already_normalized)

// Shift a4 left one bit to normalize and ensure that top bit is zero.  We do
// this by shifting two left and one right.
        sll     a4, a4, 2
        srl     a4, a4, 1
L(sub_already_normalized):

// Adjust exponent.
        add     a3, a3, -1

// Move exponent into position.
        sll     a3, a3, 52

// Move significand into position.  (*)
        srl     a0, a4, 11

// Generate rounding from remainder in lower order bits of difference.
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bbc     a4, 11, L(sub_no_tie)
        sll     a4, a4, 53
#else
        sll     a4, a4, 53
        bgez    a4, L(sub_no_tie)
        sll     a4, a4, 1
#endif
        add     a0, a0, 1
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE || __SEGGER_RTL_CORE_HAS_ANDN_ORN_XORN
        seqz    a4, a4
        ANDNx   a0, a0, a4
#else
        bnez    a4, L(sub_no_tie)
        and     a0, a0, ~1
#endif

// Insert exponent and sign and rounding bit.
L(sub_no_tie):
        add     a0, a0, a3

// Restore working register and return.
        ret

// LHS is Inf or NaN.
L(sub_inf_or_nan):
        li      t0, 0x7FF               // rhs is Inf or NaN?
        beq     a3, t0, L(return_nan)   // Inf-Inf is NaN, Inf+NaN is NaN, NaN+Inf is NaN.

// RHS a NaN?
        sll     a1, a0, 12
        beqz    a1, L(sub_done)
        j       L(return_nan)

// Subtracting zero or subnormal.
L(sub_zero):
        bnez    a4, L(sub_done)         // non-zero-x - 0 is non-zero-x
        li      a0, 0                   // 0-0 is +0
        ret

#else

#error Bad configuration

#endif

END_FUNC __adddf3

/*********************************************************************
*
*       __mulsf3()
*
*  Function description
*    Multiply, float.
*
*  Prototype
*    float __mulsf3(float x, float y);
*
*  Parameters
*    a0 - x - Multiplicand.
*    a1 - y - Multiplier.
*
*  Return value
*    a0 - Product.
*/

#undef L
#define L(label) .L__mulsf3_##label

GLOBAL_FUNC __mulsf3

#if __SEGGER_RTL_FP_ABI >= 1

        fmul.s  fa0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32 
//

// Valuable constants.
        li      a4, 0x80000000
        li      t0, 0xFF

// Standard opening: compute sign of product into a5.
        xor     a5, a0, a1
        and     a5, a5, a4              // Isolate sign

// Continue with opening book: discard sign of operands, extract exponents.
        BFOZ    a2, a0, 30, 23
        BFOZ    a3, a1, 30, 23

// Take care of zero/subnormal inputs.
        beqz    a2, L(lhs_zero_or_subnormal)
        beqz    a3, L(rhs_zero_or_subnormal)

// Take care of Inf/NaN inputs.
        beq     a2, t0, L(lhs_inf_or_nan)
        beq     a3, t0, L(rhs_inf_or_nan)

// Compute product's exponent in a2, but with double-bias.
        add     a2, a2, a3

// Get rid of exponents and insert assumed bit for multiplier and multiplicand.
        sll     a0, a0, 8               // left-align significand most significant bit, dropping exponent
        or      a0, a0, a4              // insert assumed bit
        sll     a1, a1, 8               // ...ditto
        or      a1, a1, a4

#if __SEGGER_RTL_CORE_HAS_ISA_SIMD

// Unsigned multiply a1:a0 = a0*a1.
        mulr64  a0, a0, a1

// Compress low-order rounding bits into high order.
        snez    a0, a0
        or      a0, a0, a1

#elif __SEGGER_RTL_CORE_HAS_MUL_MULH

// Perform multiply, a0:a3 = a0*a1.
        mul     a3, a0, a1
        mulhu   a0, a0, a1

// Compress low-order rounding bits into high order.
        snez    a3, a3
        or      a0, a0, a3

#else

// No multiplier so perform clockwork 24x24->48 multiply, t0:a3 = a0*a1.

// Zero product but include a sentinel bit used as a loop counter.
        li      a3, 0
        li      t0, 1<<7

// Shift product left one bit, including the sentinel.
L(clockwork_multiply):
        sltz    t1, a3
        add     a3, a3, a3
        LEAH    t0, t1, t0

// If multiplier bit is zero, don't add.
        bgez    a1, L(zero_bit)

// Add multiplicand to product.
        add     a3, a3, a0
        sltu    t1, a3, a0
        add     t0, t0, t1

// Shift multiplier to expose next bit.
L(zero_bit):
        add     a1, a1, a1

// Check sentinel in developed product register, go round again.
        bgtz    t0, L(clockwork_multiply)

// Standardize on output, a0:a3 = a0*a1.
        sll     a0, t0, 8
        srl     t1, a3, 24
        add     a0, a0, t1
        sll     a3, a3, 8

// Restore t0 constant.
        li      t0, 0xFF

// Compress low-order rounding bits into high order.
        snez    a3, a3
        or      a0, a0, a3

#endif

// If already normalized, no normalization step required.
// At most one normalization step is required (e.g. 1 * 1 = 1, whereas
// 1.5 * 1.5 = 2.25).  The product can never be zero as zero
// inputes are handled separately in the opening book.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        slt     a3, zero, a0            // a0 is never zero, 0 < a0 is the same as 0 <= a0.
        sll     a0, a0, a3
        sub     a2, a2, a3
#else
        bltz    a0, L(normalized)
        sll     a0, a0, 1               // Normalize significand
        add     a2, a2, -1              // Adjust exponent
#endif

// Significand is normalized, now pack and check for overflow.
L(normalized):

// Remove the double-bias.  If close to underflow, decide what to do with rounding.
        add     a2, a2, -127
        bltz    a2, L(zero_or_underflow)

// If overflowed, return a signed infinity.
        add     t0, t0, -1
        bge     a2, t0, L(inf)

// Split significand into result and rounding.
        sll     a3, a0, 24
        srl     a0, a0, 8

// Move exponent into position.
        sll     a2, a2, 23
        add     a0, a0, a2

// Perform rounding.
        bgez    a3, L(apply_sign)
        add     a0, a0, 1
        sll     a3, a3, 1

#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE || __SEGGER_RTL_CORE_HAS_ANDN_ORN_XORN
        seqz    a3, a3
        ANDNx   a0, a0, a3
#else
        bnez    a3, L(apply_sign)
        and     a0, a0, ~1
#endif

// Insert sign into product and return.
L(apply_sign):
        or      a0, a0, a5              // product sign is in a5
        ret

// Generate a positive infinity then apply the product sign.
L(inf):
        li      a0, 0x7F800000
        j       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// If is either Inf or NaN, then it's a NaN
        beq     a3, t0, L(nan)

// It's 0*normal, which is a signed zero.
L(signed_zero):
        mv      a0, a5
        ret

// We know that the LHS is Inf or NaN.
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.
        sll     a0, a0, 9
        bnez    a0, L(nan)

// Know lhs is Inf.  If the rhs is neither Inf nor NaN, then Inf * non-zero-normal is Inf.
        bne     a3, t0, L(inf)

// If rhs is NaN then the result is NaN.
        sll     a1, a1, 9
        bnez    a1, L(nan)

// Inf * Inf is Inf.
        j       L(inf)

// We know that the rhs is zero or subnormal and the lhs is non-zero.
// Treat subnormals as zero anyway.  0 * normal is a signed zero.
L(rhs_zero_or_subnormal):
        bne     a2, t0, L(signed_zero)

// non-zero*Inf and non-zero*NaN are NaNs.
L(nan):
        li      a0, 0x7FC00000
        ret

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.
        sll     a1, a1, 9
        bnez    a1, L(nan)

// We also know that the lhs is normal (not subnormal, not Inf, not NaN), so
// the product is Inf * normal which is Inf.
        j       L(inf)

// The product has resulted in an exponent of zero or less.  This may not actually
// mean that the product should be treated as a zero as there is still rounding
// to do.
L(zero_or_underflow):

// If the biased exponent is less than zero, then this would generate a subnormal
// and we don't support subnormals, so return a signed zero.
        add     a2, a2, 1
        bnez    a2, L(signed_zero)

// Exponent is exactly zero and on the verge of underflow except for rounding.
// So, now round significand--we only need to differentiate between a zero result
// and a result that rounds to the smallest normal value.
        sra     a0, a0, 8
        add     t0, a0, 1
        add     a0, a0, 2
        bnez    t0, L(signed_zero)

// Rounding rounded to smallest normalized value with a non-zero exponent.
        li      a0, 0x00800000
        j       L(apply_sign)

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Valuable constants.
        li      a4, 1
        sll     a4, a4, 31              // Load 0x80000000 to a4
        li      t0, 0xFF

// Standard opening: compute sign of product into a5.
        xor     a5, a0, a1
        and     a5, a5, a4              // Isolate sign

// Continue with opening book: discard sign of operands, extract exponents.
        BFOZ    a2, a0, 30, 23
        BFOZ    a3, a1, 30, 23

// Take care of zero/subnormal inputs.
        beqz    a2, L(lhs_zero_or_subnormal)
        beqz    a3, L(rhs_zero_or_subnormal)

// Take care of Inf/NaN inputs.
        beq     a2, t0, L(lhs_inf_or_nan)
        beq     a3, t0, L(rhs_inf_or_nan)

// Compute product's exponent in a2, but with double-bias.
        add     a2, a2, a3

// Get rid of exponents and insert assumed bit for multiplier and multiplicand.
        sll     a0, a0, 32+8            // left-align significand most significant bit, dropping exponent
        srl     a0, a0, 32
        or      a0, a0, a4              // insert assumed bit
        sll     a1, a1, 32+8            // ...ditto
        srl     a1, a1, 32
        or      a1, a1, a4

#if __SEGGER_RTL_CORE_HAS_MUL_MULH

// Perform multiply.
        mul     a0, a0, a1

// Compress low-order rounding bits into high order.
        sll     a3, a0, 32
        snez    a3, a3
        sra     a0, a0, 32
        or      a0, a0, a3

#else

// No multiplier so perform clockwork 24x24->48 multiply, a0 = a0*a1.

#if __SEGGER_RTL_OPTIMIZE < 2

// Zero product but include a sentinel bit used as a loop counter.
        li      t0, 1<<7

// Shift product left one bit, including the sentinel.
L(clockwork_multiply):
        add     t0, t0, t0

#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE

        sra     t1, a1, 63              // Generate mask from leading bit
        and     t1, t1, a0              // Generate 0 or multiplicand
        add     t0, t0, t1              // Add multiplicand to product

#else

        bgez    a1, L(zero_bit)         // If multiplier bit is zero, don't add.
        add     t0, t0, a0              // Add multiplicand to product.
L(zero_bit):

#endif

// Shift multiplier to expose next bit.
        add     a1, a1, a1

// Check sentinel in developed product register, go round again.
        bgtz    t0, L(clockwork_multiply)

#else

// Zero product.
        li      t0, 0

// Open-code multiply as above.
       .rept    63-7
        add     t0, t0, t0
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sra     t1, a1, 63
        and     t1, t1, a0
        add     t0, t0, t1
#else
        bgez    a1, 1f
        add     t0, t0, a0
1:
#endif
        add     a1, a1, a1
       .endr

#endif

// Standardize on output, a0 = a0*a1.
        sll     a0, t0, 8

// Restore t0 constant.
        li      t0, 0xFF

// Compress low-order rounding bits into high order.
        sll     a3, a0, 32
        snez    a3, a3
        sra     a0, a0, 32
        or      a0, a0, a3

#endif

// If already normalized, no normalization step required.
// At most one normalization step is required (e.g. 1 * 1 = 1, whereas
// 1.5 * 1.5 = 2.25).  The product can never be zero as zero
// inputes are handled separately in the opening book.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        slt     a3, zero, a0            // a0 is never zero, 0 < a0 is the same as 0 <= a0.
        sll     a0, a0, a3
        sub     a2, a2, a3
#else
        bltz    a0, L(normalized)
        sll     a0, a0, 1               // Normalize significand
        add     a2, a2, -1              // Adjust exponent
#endif

// Significand is normalized, now pack and check for overflow.
L(normalized):

// Remove the double-bias.  If close to underflow, decide what to do with rounding.
        add     a2, a2, -127
        bltz    a2, L(zero_or_underflow)

// If overflowed, return a signed infinity.
        add     t0, t0, -1
        bge     a2, t0, L(inf)

// Split significand into result and rounding.
        sll     a3, a0, 32+24
        srlw    a0, a0, 8

// Move exponent into position.
        sll     a2, a2, 23
        add     a0, a0, a2

// Perform rounding.
        bgez    a3, L(apply_sign)
        add     a0, a0, 1
        sll     a3, a3, 1

#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE || __SEGGER_RTL_CORE_HAS_ANDN_ORN_XORN
        seqz    a3, a3
        ANDNx   a0, a0, a3
#else
        bnez    a3, L(apply_sign)
        and     a0, a0, ~1
#endif

// Insert sign into product and return.
L(apply_sign):
        or      a0, a0, a5              // product sign is in a5
        ret

// Generate a positive infinity then apply the product sign.
L(inf):
        li      a0, 0x7F800000
        j       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// If is either Inf or NaN, then it's a NaN
        beq     a3, t0, L(nan)

// It's 0*normal, which is a signed zero.
L(signed_zero):
        mv      a0, a5
        ret

// We know that the LHS is Inf or NaN.
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.
        sll     a0, a0, 32+9
        bnez    a0, L(nan)

// Know lhs is Inf.  If the rhs is neither Inf nor NaN, then Inf * non-zero-normal is Inf.
        bne     a3, t0, L(inf)

// If rhs is NaN then the result is NaN.
        sll     a1, a1, 32+9
        bnez    a1, L(nan)

// Inf * Inf is Inf.
        j       L(inf)

// We know that the rhs is zero or subnormal and the lhs is non-zero.
// Treat subnormals as zero anyway.  0 * normal is a signed zero.
L(rhs_zero_or_subnormal):
        bne     a2, t0, L(signed_zero)

// non-zero*Inf and non-zero*NaN are NaNs.
L(nan):
        li      a0, 0x7FC00000
        ret

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.
        sll     a1, a1, 32+9
        bnez    a1, L(nan)

// We also know that the lhs is normal (not subnormal, not Inf, not NaN), so
// the product is Inf * normal which is Inf.
        j       L(inf)

// The product has resulted in an exponent of zero or less.  This may not actually
// mean that the product should be treated as a zero as there is still rounding
// to do.
L(zero_or_underflow):

// If the biased exponent is less than zero, then this would generate a subnormal
// and we don't support subnormals, so return a signed zero.
        add     a2, a2, 1
        bnez    a2, L(signed_zero)

// Exponent is exactly zero and on the verge of underflow except for rounding.
// So, now round significand--we only need to differentiate between a zero result
// and a result that rounds to the smallest normal value.
        sra     a0, a0, 8
        add     t0, a0, 1
        add     a0, a0, 2
        bnez    t0, L(signed_zero)

// Rounding rounded to smallest normalized value with a non-zero exponent.
        li      a0, 0x00800000
        j       L(apply_sign)

#else

#error Bad configuration

#endif

END_FUNC __mulsf3

/*********************************************************************
*
*       __muldf3()
*
*  Function description
*    Multiply, double.
*
*  Prototype
*    double __muldf3(double x, double y);
*
*  Parameters
*    a1:a0 / a0 - x - Multiplicand.
*    a3:a2 / a1 - y - Multiplier.
*
*  Return value
*    a1:a0 / a0 - Product.
*/

#undef L
#define L(label) .L__muldf3_##label

GLOBAL_FUNC __muldf3

#if __SEGGER_RTL_FP_ABI >= 2

        fmul.d  fa0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32 
//

#if __SEGGER_RTL_CORE_HAS_ISA_RVE && !__SEGGER_RTL_CORE_HAS_MUL_MULH

// Version for RVE without a multiplier

// Lay out frame
#define FRAME_S0        0  // saved s0
#define FRAME_S1        4  // saved s1
#define FRAME_RA        8  // saved ra
#define FRAME_SIGN     12  // sign of product
#define FRAME_EXPONENT 16  // working exponent of product
#define FRAME_CNT      20  // intermediate double-length product high 32 bits

// Create frame and save registers.
        add     sp, sp, -STACK_ALIGN(24)
        sw      s0, FRAME_S0(sp)
        sw      s1, FRAME_S1(sp)
        sw      ra, FRAME_RA(sp)

// Useful constants.
        li      s1, 0x80000000

// Compute sign of product into t4{31}, others zero.
        xor     s0, a1, a3
        and     s0, s0, s1
        sw      s0, FRAME_SIGN(sp)

// Remove sign from muliplier and multiplicand significand and align to msb.
        add     a4, a1, a1
        add     a5, a3, a3

// Is lhs NaN or Inf?  If so, handle special case out of line.
        li      s0, 0xFFE00000
        bgeu    a4, s0, L(lhs_nan_or_inf)

// Is rhs NaN or Inf?  If so, handle special case out of line.
        bgeu    a5, s0, L(rhs_nan_or_inf)

// Extract multiplicand exponent.  If zero, we have 0*normal or 0*0, which is zero.
        srl     a4, a4, 21
        beqz    a4, L(signed_zero)

// Extract multiplier exponent.  If zero, we have non-zero-normal*0 which is zero.
        srl     a5, a5, 21
        beqz    a5, L(signed_zero)

// Compute product's exponent, frees a5.
        add     a5, a5, a4
        sw      a5, FRAME_EXPONENT(sp)

// Shift off exponent and materialize hidden bit in muliplicand significand.
// Align significand to msb.
        sll     a1, a1, 11
        or      a1, a1, s1              // Insert hidden bit
        srl     s0, a0, 21              // Shift a1:a0 left 11 bits.  See (*) below...  s0 only free register at present
        sll     a0, a0, 11
        or      a1, a1, s0

// Shift off exponent and materialize hidden bit in multiplier significand.
        sll     a3, a3, 11
        or      a3, a3, s1
        srl     a3, a3, 11

// We now have the task of a 53x53-bit multiply.
// Do multiply a1:a0.<s0> = a1:a0.<0> * a3:a2.<0> where s0 contains rounding bits

// Multiply low 32 bits.
        li      s0, 32
        sw      s0, FRAME_CNT(sp)

// t3:t2:t1:t0 is the product register.
        li      t0, 0
        li      t1, 0
        li      t2, 0
        li      ra, 0

// a4:a1:a0 is the multiplicand shift register.
        li      a4, 0

// Accumulation of carries for each word.
        li      s1, 0
        li      a5, 0

// This only accumulates into t2:t1:t0 as it can't overflow to t3.
L(form_low_product):

// Test multiplier bit.
        and     s0, a2, 1
        srl     a2, a2, 1
        beqz    s0, L(low_zero_bit)

// Bit is one, accumulate multiplicand shift register into product.
        add     t0, t0, a0              // t3:t2:t1:t0+a0
        sltu    s0, t0, a0              // ...carry out
        add     s1, s1, s0              // ....accumulate
//
        add     t1, t1, a1              // t3:t2:t1_a1:t0+a0
        sltu    s0, t1, a1              // ...carry out
        add     a5, a5, s0              // ....accumulate
//
        add     t2, t2, a4              // t3:t2+a4:t1+a1:t0+a0

// Shift multiplicand register.
L(low_zero_bit):
        sll     a4, a4, 1
        sltz    s0, a1
        add     a4, a4, s0
        sll     a1, a1, 1
        sltz    s0, a0
        add     a1, a1, s0
        sll     a0, a0, 1

// Do this for 32 bits.
        lw      s0, FRAME_CNT(sp)
        add     s0, s0, -1
        sw      s0, FRAME_CNT(sp)
        bnez    s0, L(form_low_product)

// Fold column carries to product.
        add     t1, t1, s1
        sltu    s0, t1, s1
        add     t2, t2, a5
        add     t2, t2, s0

// a0:a4:a1 is now the multiplicand shift register.  a0 is already zero.

// Multiply high 21 bits.
        li      s0, 21
        sw      s0, FRAME_CNT(sp)

// Accumulation of carries for each word.
        li      s1, 0
        li      a5, 0

// This only accumulates into t3:t2:t1 as t0 is done.
L(form_high_product):

// Test multiplier bit.
        and     s0, a3, 1
        srl     a3, a3, 1
        beqz    s0, L(high_zero_bit)

// Bit is one, accumulate multiplicand shift register into product.
        add     t1, t1, a1              // t3:t3:t2:t1+a1
        sltu    s0, t1, a1              // ...carry out
        add     s1, s1, s0              // ....accumulate
//
        add     t2, t2, a4              // t3:t3:t2+a4:t1+a1
        sltu    s0, t2, a4              // ...carry out
        add     a5, a5, s0              // ....accumulate
//
        add     ra, ra, a0              // t3:t3+a0:t2+a4+c:t1+a1

// Shift multiplicand register.
L(high_zero_bit):
        sltz    s0, a4
        LEAH    a0, s0, a0
        sltz    s0, a1
        LEAH    a4, s0, a4
        sll     a1, a1, 1

// Do this for 21 bits.
        lw      s0, FRAME_CNT(sp)
        add     s0, s0, -1
        sw      s0, FRAME_CNT(sp)
        bnez    s0, L(form_high_product)

// Fold column carries to product.
        add     t2, t2, s1
        sltu    s0, t2, s1
        add     ra, ra, a5
        add     ra, ra, s0

// Discard the low order product word after folding in lower-order digits.
        snez    t0, t0
        or      t1, t1, t0

// Rearrange.
        mv      s0, t1
        mv      a0, t2
        mv      a1, ra

// Recover exponent to a4.
        lw      a4, FRAME_EXPONENT(sp)

// Is the product normalized?
        sll     t0, a1, 11
        bltz    t0, L(normalized)

// No, shift product and adjust exponent.
        sltz    t0, s0
        add     s0, s0, s0
        sltz    t1, a0
        LEAH    a0, t0, a0
        LEAH    a1, t1, a1
        add     a4, a4, -1              // exponent adjust

// Product is normalized, must now round.
L(normalized):
        li      a5, 0x3FF

// Remove IEEE bias.  If exponent went negative, return a signed zero.
        sub     a4, a4, a5
        bltz    a4, L(signed_zero)

// If exponent overflowed, generate a signed infinity.
        sll     a5, a5, 1               // Generate 0x7FE
        bge     a4, a5, L(inf)

// Move exponent into position.
        sll     a4, a4, 20

// Pack.
        add     a1, a1, a4              // Add exponent
        bgez    s0, L(apply_sign)       // No tie
        add     a0, a0, 1               // Round up
        seqz    a2, a0
        add     a1, a1, a2
        sll     s0, s0, 1
        bnez    s0, L(apply_sign)
        and     a0, a0, ~1

// Apply sign to product.
L(apply_sign):
        lw      a2, FRAME_SIGN(sp)
        or      a1, a1, a2
        j       L(done)

// lhs has an exponent that indicates NaN or Inf.  If NaN, NaN*anything is NaN.
L(lhs_nan_or_inf):
        bne     a4, s0, L(nan)          // Not exactly 0xFFE00000, i.e. must be a NaN as significand is non-zero
        bnez    a0, L(nan)              // other part of Inf/NaN indicates NaN?

// Lhs is now known to be Inf.  If rhs is NaN, the result is NaN.
        bgtu    a5, s0, L(nan)          // Known to be NaN
        bltu    a5, s0, L(rhs_could_be_zero)    // Normal or zero

// Could have NaN if lower part of significand is non-zero.
        bnez    a2, L(nan)

// lhs is Inf, rhs is normal or zero.  Inf*normal is Inf, Inf*0 is NaN.
L(rhs_could_be_zero):

// Extract significand digits, Inf*normal is float64_mul_inf
        srl     a5, a5, 21
        bnez    a5, L(inf)

// ...left with Inf*0 which is NaN.
L(nan):
        li      a1, 0x7FF80000
L(load_zero_lo):
        li      a0, 0

// All done, restore registers and return.
L(done):
        lw      s0, FRAME_S0(sp)
        lw      s1, FRAME_S1(sp)
        lw      ra, FRAME_RA(sp)
        add     sp, sp, +STACK_ALIGN(24)
        ret

// lhs has an exponent that indicates NaN or Inf.  If NaN, Nan*anything is NaN.
L(rhs_nan_or_inf):
        bne     a5, s0, L(nan)
        bnez    a2, L(nan)              // low part of significand

// Know rhs is now Inf.  0*Inf is NaN, normal*Inf is Inf.
        srl     a4, a4, 21              // Exponent of lhs indicates zero (or subnormal)?
        beqz    a4, L(nan)              // yes, it's 0*Inf which is NaN.

// Generate a signed infinity.
L(inf):
        li      a1, 0x7FF00000
        li      a0, 0
        j       L(apply_sign)           // apply sign to Inf.

// Generate a signed zero.
L(signed_zero):
        lw      a1, FRAME_SIGN(sp)      // sign only in a1
        j       L(load_zero_lo)

#else

#if __SEGGER_RTL_CORE_HAS_ISA_RVE

// Lay out frame
#define FRAME_S0        0  //  0(sp), saved s0
#define FRAME_S1        4  //  4(sp), saved s1

#define a6 s0
#define a7 s1
#define t4 t2  // We can use t2 in place of t4 here as t2 is nuked only using the clockwork multiply

// Save registers
        addi    sp, sp, -STACK_ALIGN(8)
        sw      s0, FRAME_S0(sp)
        sw      s1, FRAME_S1(sp)

#endif

// Useful constants.
        li      a7, 0x80000000

// Compute sign of product into t4{31}, others zero.
        xor     a6, a1, a3
        and     t4, a6, a7

// Remove sign from muliplier and multiplicand significand and align to msb.
        add     a4, a1, a1
        add     a5, a3, a3

// Is lhs NaN or Inf?  If so, handle special case out of line.
        li      a6, 0xFFE00000
        bgeu    a4, a6, L(lhs_nan_or_inf)

// Is rhs NaN or Inf?  If so, handle special case out of line.
        bgeu    a5, a6, L(rhs_nan_or_inf)

// Extract multiplicand exponent.  If zero, we have 0*normal or 0*0, which is zero.
        srl     a4, a4, 21
        beqz    a4, L(signed_zero)

// Extract multiplier exponent.  If zero, we have non-zero-normal*0 which is zero.
        srl     a5, a5, 21
        beqz    a5, L(signed_zero)

// Shift off exponent and materialize hidden bit in multiplier significand.
        sll     a3, a3, 11
        or      a3, a3, a7
        srl     a3, a3, 11

// Shift off exponent and materialize hidden bit in muliplicand significand.
// Align significand to msb.
        sll     a1, a1, 11
        or      a1, a1, a7              // Insert hidden bit
        srl     a6, a0, 21              // Shift a1:a0 left 11 bits.  See (*) below...  a6 only free register at present
        sll     a0, a0, 11
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD || __SEGGER_RTL_CORE_HAS_MUL_MULH
        add     t0, a1, a6
#else
        add     a1, a1, a6
#endif

// We now have the task of a 53x53-bit multiply.
// Do multiply a1:a0.<a6> = a1:a0.<0> * a3:a2.<0> where a6 contains rounding bits

#if __SEGGER_RTL_CORE_HAS_ISA_SIMD || __SEGGER_RTL_CORE_HAS_MUL_MULH

// Compute product's exponent into t5, frees a5:a4.
        add     t1, a4, a5

#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        mulr64  a4, a0, a2              //    a5:a4 = low x low
        mulr64  a6, a0, a3              // a7:a6    = low x high
#else
        mul     a4, a0, a2
        mulhu   a5, a0, a2
        mul     a6, a0, a3
        mulhu   a7, a0, a3
#endif
        add     a6, a6, a5              // a0 is now unused
        sltu    a5, a6, a5
        add     a7, a7, a5              //    a7:a6:a4  - product

#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        mulr64  a0, t0, a2              //    a1:a0     - high x low
#else
        mul     a0, t0, a2
        mulhu   a1, t0, a2
#endif
        add     a6, a6, a0              //              - Add to middle-order
        sltu    a0, a6, a0              //    a0        - Carry out from lower order
        add     a7, a7, a1              //
        sltu    a1, a7, a1              // a1           - Carry put
        add     a7, a7, a0              //              - Add carry from lower oeder
        sltu    a0, a7, a0              // a0           - Carry out
        add     a2, a0, a1              // a2           - Combined carry out

#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        mulr64  a0, t0, a3              // a1:a0        - high x high
#else
        mul     a0, t0, a3
        mulhu   a1, t0, a3
#endif
        add     a0, a0, a7
        sltu    a7, a0, a7
        add     a1, a1, a7
        add     a1, a1, a2
        snez    a4, a4                  // Compress low-order bits into middle order
        or      a6, a6, a4              // ...ditto

// Recover exponent to a4.
        mv      a4, t1

#else

// Compute product's exponent into t5, frees a5:a4.
#if __SEGGER_RTL_CORE_HAS_ISA_RVE
        add     a7, a4, a5
#else
        add     t5, a4, a5
#endif

// Multiply by clockwork.
// Do multiply a1:a0.<a6> := a1:a0.<0> * a3:a2.<0> where a6 contains rounding bits

// t3:t2:t1:t0 is the product register.
        li      t0, 0
        li      t1, 0
        li      t2, 0
        li      t3, 0

// Multiply low 32 bits.
        li      t6, 32

// a4:a1:a0 is the multiplicand shift register.
        li      a4, 0

// Accumulation of carries for each word.
        li      a7, 0
        li      a5, 0

// This only accumulates into t2:t1:t0 as it can't overflow to t3.
L(form_low_product):

// Test multiplier bit.
        and     a6, a2, 1
        srl     a2, a2, 1
        beqz    a6, L(low_zero_bit)

// Bit is one, accumulate multiplicand shift register into product.
        add     t0, t0, a0              // t3:t2:t1:t0+a0
        sltu    a6, t0, a0              // ...carry out
        add     a7, a7, a6              // ....accumulate
//
        add     t1, t1, a1              // t3:t2:t1_a1:t0+a0
        sltu    a6, t1, a1              // ...carry out
        add     a5, a5, a6              // ....accumulate
//
        add     t2, t2, a4              // t3:t2+a4:t1+a1:t0+a0

// Shift multiplicand register.
L(low_zero_bit):
        sltz    a6, a1
        LEAH    a4, a6, a4
        sltz    a6, a0
        LEAH    a1, a6, a1
        sll     a0, a0, 1

// Do this for 32 bits.
        add     t6, t6, -1
        bnez    t6, L(form_low_product)

// Fold column carries to product.
        add     t1, t1, a7
        sltu    a6, t1, a7
        add     t2, t2, a5
        add     t2, t2, a6

// a0:a4:a1 is now the multiplicand shift register.  a0 is already zero.

// Multiply high 21 bits.
        li      t6, 21

// Accumulation of carries for each word.
        li      a7, 0
        li      a5, 0

// This only accumulates into t3:t2:t1 as t0 is done.
L(form_high_product):

// Test multiplier bit.
        and     a6, a3, 1
        srl     a3, a3, 1
        beqz    a6, L(high_zero_bit)

// Bit is one, accumulate multiplicand shift register into product.
        add     t1, t1, a1              // t3:t3:t2:t1+a1
        sltu    a6, t1, a1              // ...carry out
        add     a7, a7, a6              // ....accumulate
//
        add     t2, t2, a4              // t3:t3:t2+a4:t1+a1
        sltu    a6, t2, a4              // ...carry out
        add     a5, a5, a6              // ....accumulate
//
        add     t3, t3, a0              // t3:t3+a0:t2+a4+c:t1+a1

// Shift multiplicand register.
L(high_zero_bit):
        sltz    a6, a4
        LEAH    a0, a6, a0
        sltz    a6, a1
        LEAH    a4, a6, a4
        sll     a1, a1, 1

// Do this for 21 bits.
        add     t6, t6, -1
        bnez    t6, L(form_high_product)

// Fold column carries to product.
        add     t2, t2, a7
        sltu    a6, t2, a7
        add     t3, t3, a5
        add     t3, t3, a6

// Discard the low order product word after folding in lower-order digits.
        snez    t0, t0
        or      t1, t1, t0

// Rearrange.
        mv      a6, t1
        mv      a0, t2
        mv      a1, t3

// Recover exponent to a4.
#if __SEGGER_RTL_CORE_HAS_ISA_RVE
        mv      a4, a7
#else
        mv      a4, t5
#endif

#endif

// Is the product normalized?
        sll     t0, a1, 11
        bltz    t0, L(normalized)

// No, shift product a1:a0:a6 and adjust exponent.
        sltz    t0, a6
        add     a6, a6, a6
        sltz    t1, a0
        LEAH    a0, t0, a0
        LEAH    a1, t1, a1
        add     a4, a4, -1              // exponent adjust

// Product is normalized, must now round.
L(normalized):
        li      a5, 0x3FF

// Remove IEEE bias.  If exponent went negative, return a signed zero.
        sub     a4, a4, a5
        bltz    a4, L(signed_zero)

// If exponent overflowed, generate a signed infinity.
        sll     a5, a5, 1               // Generate 0x7FE
        bge     a4, a5, L(inf)

// Move exponent into position.
        sll     a4, a4, 20

// Pack.
        add     a1, a1, a4              // Add exponent
        bgez    a6, L(apply_sign)       // No tie
        add     a0, a0, 1               // Round up
        seqz    a2, a0
        add     a1, a1, a2

// Round.
        sll     a6, a6, 1
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE || __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        seqz    a2, a6
        ANDNx   a0, a0, a2
#else
        bnez    a6, L(apply_sign)
        and     a0, a0, ~1
#endif

// Apply sign to product.
L(apply_sign):
        add     a1, a1, t4

#if __SEGGER_RTL_CORE_HAS_ISA_RVE
L(ret):
        lw      s0, FRAME_S0(sp)
        lw      s1, FRAME_S1(sp)
        addi    sp, sp, +STACK_ALIGN(8)
#endif
        ret

// lhs has an exponent that indicates NaN or Inf.  If NaN, NaN*anything is NaN.
L(lhs_nan_or_inf):
        bne     a4, a6, L(nan)          // Not exactly 0xFFE00000, i.e. must be a NaN as significand is non-zero
        bnez    a0, L(nan)              // other part of Inf/NaN indicates NaN?

// Lhs is now known to be Inf.  If rhs is NaN, the result is NaN.
        bgtu    a5, a6, L(nan)          // Known to be NaN
        bltu    a5, a6, L(rhs_could_be_zero)    // Normal or zero

// Could have NaN if lower part of significand is non-zero.
        bnez    a2, L(nan)

// lhs is Inf, rhs is normal or zero.  Inf*normal is Inf, Inf*0 is NaN.
L(rhs_could_be_zero):

// Extract significand digits, Inf*normal is float64_mul_inf
        srl     a5, a5, 21
        bnez    a5, L(inf)

// ...left with Inf*0 which is NaN.
L(nan):
        li      a1, 0x7FF80000
L(load_zero_lo):
        li      a0, 0

// All done, restore registers and return.
L(done):
#if __SEGGER_RTL_CORE_HAS_ISA_RVE
        j       L(ret)
#else
        ret
#endif

// lhs has an exponent that indicates NaN or Inf.  If NaN, Nan*anything is NaN.
L(rhs_nan_or_inf):
        bne     a5, a6, L(nan)
        bnez    a2, L(nan)              // low part of significand

// Know rhs is now Inf.  0*Inf is NaN, normal*Inf is Inf.
        srl     a4, a4, 21              // Exponent of lhs indicates zero (or subnormal)?
        beqz    a4, L(nan)              // yes, it's 0*Inf which is NaN.

// Generate a signed infinity.
L(inf):
        li      a1, 0x7FF00000
        li      a0, 0
        j       L(apply_sign)           // apply sign to Inf.

// Generate a signed zero.
L(signed_zero):
        mv      a1, t4                  // sign only in a1
        j       L(load_zero_lo)

#endif

#undef a7
#undef a6
#undef t4
#undef FRAME_S0
#undef FRAME_S1
#undef FRAME_SIGN
#undef FRAME_EXPONENT
#undef FRAME_RA
#undef FRAME_CNT

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Valuable constants.
        li      t0, 0x7FF
        sll     a4, t0, 63              // 0x80000000'00000000

// Standard opening: compute sign of product into a5.
        xor     a5, a0, a1
        and     a5, a5, a4              // Isolate sign

// Continue with opening book: discard sign of operands, extract exponents.
        BFOZ    a2, a0, 62, 52
        BFOZ    a3, a1, 62, 52

// Take care of zero/subnormal inputs.
        beqz    a2, L(lhs_zero_or_subnormal)
        beqz    a3, L(rhs_zero_or_subnormal)

// Take care of Inf/NaN inputs.
        beq     a2, t0, L(lhs_inf_or_nan)
        beq     a3, t0, L(rhs_inf_or_nan)

// Compute product's exponent in a2, but with double-bias.
        add     a2, a2, a3

// Get rid of exponents and insert assumed bit for multiplier and multiplicand.
        sll     a0, a0, 11              // left-align significand most significant bit, dropping exponent
        or      a0, a0, a4              // insert assumed bit
        sll     a1, a1, 11              // ...ditto
        or      a1, a1, a4

#if __SEGGER_RTL_CORE_HAS_MUL_MULH

// Perform multiply, a0:a3 = a0*a1.
        mul     a3, a0, a1
        mulhu   a0, a0, a1

// Compress low-order rounding bits into high order.
        snez    a3, a3
        or      a0, a0, a3

#else

// No multiplier so perform clockwork 53x53->106 multiply, t0:a3 = a0*a1.

// Zero product but include a sentinel bit used as a loop counter.
        li      a3, 0
        li      t0, 1<<10

// Shift product left one bit, including the sentinel.
L(clockwork_multiply):
        sltz    t1, a3
        add     a3, a3, a3
        LEAH    t0, t1, t0

// If multiplier bit is zero, don't add.
        bgez    a1, L(zero_bit)

// Add multiplicand to product.
        add     a3, a3, a0
        sltu    t1, a3, a0
        add     t0, t0, t1

// Shift multiplier to expose next bit.
L(zero_bit):
        add     a1, a1, a1

// Check sentinel in developed product register, go round again.
        bgtz    t0, L(clockwork_multiply)

// Standardize on output, a0:a3 = a0*a1.
        sll     a0, t0, 11
        srl     t1, a3, 64-11
        add     a0, a0, t1
        sll     a3, a3, 11

// Restore t0 constant.
        li      t0, 0x7FF

// Compress low-order rounding bits into high order.
        snez    a3, a3
        or      a0, a0, a3

#endif

// If already normalized, no normalization step required.
// At most one normalization step is required (e.g. 1 * 1 = 1, whereas
// 1.5 * 1.5 = 2.25).  The product can never be zero as zero
// inputes are handled separately in the opening book.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        slt     a3, zero, a0            // a0 is never zero, 0 < a0 is the same as 0 <= a0.
        sll     a0, a0, a3
        sub     a2, a2, a3
#else
        bltz    a0, L(normalized)
        sll     a0, a0, 1               // Normalize significand
        add     a2, a2, -1              // Adjust exponent
#endif

// Significand is normalized, now pack and check for overflow.
L(normalized):

// Remove the double-bias.  If close to underflow, decide what to do with rounding.
        add     a2, a2, -0x3FF
        bltz    a2, L(signed_zero)

// If overflowed, return a signed infinity.
        add     t0, t0, -1
        bge     a2, t0, L(inf)

// Split significand into result and rounding.
        sll     a3, a0, 53
        srl     a0, a0, 11

// Move exponent into position.
        sll     a2, a2, 52
        add     a0, a0, a2

// Perform rounding.
        bgez    a3, L(apply_sign)
        add     a0, a0, 1
        sll     a3, a3, 1

#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE || __SEGGER_RTL_CORE_HAS_ANDN_ORN_XORN
        seqz    a3, a3
        ANDNx   a0, a0, a3
#else
        bnez    a3, L(apply_sign)
        and     a0, a0, ~1
#endif

// Insert sign into product and return.
L(apply_sign):
        or      a0, a0, a5              // product sign is in a5
        ret

// Generate a positive infinity then apply the product sign.
L(inf):
        li      a0, -1
        sll     a0, a0, 53
        srl     a0, a0, 1
        j       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// If is either Inf or NaN, then it's a NaN
        beq     a3, t0, L(nan)

// It's 0*normal, which is a signed zero.
L(signed_zero):
        mv      a0, a5
        ret

// We know that the LHS is Inf or NaN.
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.
        sll     a0, a0, 12
        bnez    a0, L(nan)

// Know lhs is Inf.  If the rhs is neither Inf nor NaN, then Inf * non-zero-normal is Inf.
        bne     a3, t0, L(inf)

// If rhs is NaN then the result is NaN.
        sll     a1, a1, 12
        bnez    a1, L(nan)

// Inf * Inf is Inf.
        j       L(inf)

// We know that the rhs is zero or subnormal and the lhs is non-zero.
// Treat subnormals as zero anyway.  0 * normal is a signed zero.
L(rhs_zero_or_subnormal):
        bne     a2, t0, L(signed_zero)

// non-zero*Inf and non-zero*NaN are NaNs.
L(nan):
        li      a0, -1
        sll     a0, a0, 52
        srl     a0, a0, 1
        ret

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.
        sll     a1, a1, 12
        bnez    a1, L(nan)

// We also know that the lhs is normal (not subnormal, not Inf, not NaN), so
// the product is Inf * normal which is Inf.
        j       L(inf)

#else

#error Bad configuration

#endif

END_FUNC __muldf3

/*********************************************************************
*
*       __divsf3()
*
*  Function description
*    Divide, float.
*
*  Prototype
*    float __divsf3(float x, float, y);
*
*  Parameters
*    a0 - x - Dividend.
*    a1 - y - Divisor.
*
*  Return value
*    a0 - Quotient.
*/

#undef L
#define L(label) .L__divsf3_##label

GLOBAL_FUNC __divsf3

#if __SEGGER_RTL_FP_ABI >= 1

        fdiv.s  fa0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32 
//

#if __SEGGER_RTL_CORE_HAS_MUL_MULH && (__SEGGER_RTL_OPTIMIZE >= 0)

// Set Inf/NaN exponent.
        li      t0, 0xFF

// Continue with opening book: discard sign of operands, move exponents
// to low byte of operand registers.
        BFOZ    a4, a0, 30, 23
        BFOZ    a5, a1, 30, 23

// Compute sign of quotient and store it into t1{31}.
        xor     t1, a0, a1              // Sign to t1{31}
        srl     t1, t1, 31
        sll     t1, t1, 31

// Special handling of exponents 0x00 and 0xFF for zero/subnormal and Inf/NaN.
        beqz    a4, L(lhs_zero_or_subnormal)
        beqz    a5, L(rhs_zero_or_subnormal)
        beq     a4, t0, L(lhs_inf_or_nan)
        beq     a5, t0, L(rhs_inf_or_nan)

// Compute exponent of quotient.
        sub     a4, a4, a5

// Load approximation to reciprocal (32 bits) into t0 using
// leading six bits of significand.
       .set     __SEGGER_RTL_fdiv_reciprocal_table_REQUIRED, 1
        la      t0, __SEGGER_RTL_fdiv_reciprocal_table
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bfoz    a3, a1, 22, 17
        lea.w   a3, t0, a3
#else
        srl     a3, a1, 15
        and     a3, a3, 0xFC
        add     a3, a3, t0
#endif
        lw      a5, (a3)

// Break reciprocal into sign-extended high 8 bits in a2 and
// lower 24 bits in a5 at (*) below.
        sra     a2, a5, 24

// Perform fractional multiply.
        BFOZ    a3, a1, 16, 9
        and     t0, a3, 1
        srl     a3, a3, 1
        add     a3, a3, -64
        add     a3, a3, t0
        mul     a2, a2, a3
        BFOZ    a5, a5, 23, 0           // (*) Schedule this here rather than above to overlap multiply
        add     a2, a2, a5

// Clear away exponent and set hidden bit.
        BFOZ    a1, a1, 23, 0
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a1, a1, 23
#else
        li      a5, 0x00800000
        or      a1, a1, a5
#endif

// Start multiply early.
        mul     a3, a1, a2

// Clear away exponent and set hidden bit.
        BFOZ    a0, a0, 23, 0
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 23
#else
        or      a0, a0, a5
#endif

// Fractional multiply reciprocal by dividend which approximates
// the quotient in a5.
        mulh    a5, a3, a2

// Normalize ensuring a0 >= a1.
        sltu    t0, a0, a1               // Conditionallly normalize
        sll     a0, a0, t0               // ...conditionally shift
        sub     a4, a4, t0               // ...conditionally decrement exponent
        srl     a3, a2, 31-11
        and     a3, a3, 1
        xor     a3, a3, 1
        sll     a2, a2, 11
        sub     a2, a2, a5
        sub     a2, a2, a3

// Fractional multiply reciprocal by dividend which approximates
// the quotient in a5.
        sll     a0, a0, 2
        mulh    a5, a2, a0

// Get unshifted, biased quotient exponent into a2.  Hoisted
// so that multiply can proceed without hazards.
        add     a2, a4, 0x7E

// Apply correction.
        sll     a0, a0, 22
        sub     a0, a0, a1
        mul     a3, a5, a1              // Multiply early to hide result latency

// Check for underflow or overflow condition.
        li      t0, 0xFE
        bgeu    a2, t0, L(underflow_or_overflow)

// Rounding, compare remainder.
        sub     a0, a3, a0
        sltz    t0, a0                  // Round up if normalized
        add     a5, a5, t0
        and     a0, a5, 1               // Rounding bit
        srl     a5, a5, 1               // Significand to position
        add     a0, a0, a5              // Significand to position with rounding
        sll     a2, a2, 23              // Exponent to position
        add     a0, a0, a2              // Apple exponent
        add     a0, a0, t1              // Apply sign
        ret

// Underflow and overflow handling.
L(underflow_or_overflow):
        mv      a0, t1                  // Put sign into a0{31}, make signed zero.
        blt     a2, t0, L(done)
        li      t1, 0x7F800000          // If overflow, make Inf

// Insert sign into quotient.
L(apply_sign):
        add     a0, a0, t1
L(done):
        ret

// Generate a positive infinity, then apply quotient sign.
L(inf):
        li      a0, 0x7F800000
        j       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// Treat subnormals as zero; if zero we're computing 0/0 which is NaN.
        beqz    a5, L(nan)

// If exponent is non-zero then the rhs is non-zero normal and not Inf
// and not NaN, hence 0/normal is signed zero.
        bne     a5, t0, L(signed_zero)             // Return signed zero.

// rhs is either Inf or Nan.  We distinguish between the two by the significand
// bits.  If it's Inf, then 0/Inf is a signed zero.
        sll     a1, a1, 9
        beqz    a1, L(signed_zero)             // Return signed zero.

// Result is NaN.
L(nan):
        li      a0, 0x7FC00000
        ret

// We know that the LHS is Inf or NaN, but we haven't a clue what the rhs is (yet).
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.  Use significand bits to differentiate
// between NaN and Inf.
        sll     a0, a0, 9
        bnez    a0, L(nan)                     // non-zero significand implies lhs NaN

// Now know the lhs is Inf.  If rhs is normal then Inf/zero-normal is Inf.
        bne     a5, t0, L(inf)

// We now have a rhs of Inf or NaN, and Inf/Inf and Inf/NaN are both NaN.
        j       L(nan)

// We know that the rhs is zero or subnormal and the lhs is non-zero, Inf, or NaN.
L(rhs_zero_or_subnormal):

// If the lhs in normal, then normal/0 is a signed infinity.
        bne     a4, t0, L(inf)

// Know the lhs is either Inf or NaN.  If NaN, NaN/normal is NaN.
        sll     a0, a0, 9               // non-zero significand implies lhs NaN
        bnez    a0, L(nan)              // ...and a NaN result

// Inf/0 is still Inf.
        j       L(inf)

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// Non-zero normal/NaN is NaN.
        sll     a1, a1, 9               // non-zero significand implies rhs NaN
        bnez    a1, L(nan)              // ...and a NaN result

// Non-zero normal/Inf is a signed zero.
L(signed_zero):
        mv      a0, t1                  // Correctly signed zero
        ret

#else

// Generally-useful constant.
        li      t1, 0xFF

// Standard opening: compute sign of quotient into a5.
        xor     a5, a0, a1
        li      a4, 0x80000000
        and     a5, a5, a4              // Isolate sign

// Continue with opening book: discard sign of operands, extract exponents
// to low byte of operand registers.
        BFOZ    a2, a0, 30, 23
        BFOZ    a3, a1, 30, 23

// Handle zero/subnormal inputs.
        beqz    a2, L(lhs_zero_or_subnormal)
        beqz    a3, L(rhs_zero_or_subnormal)

// Handle inf/NaN inputs.
        beq     a2, t1, L(lhs_inf_or_nan)
        beq     a3, t1, L(rhs_inf_or_nan)

// Compute quotients's exponent in a2, now true exponent without bias.
        sub     a2, a2, a3

// Standard form for division: align significands right and insert hidden bit.
        sll     a0, a0, 8               // align to right
        or      a0, a0, a4              // insert sign bit, a4 is 0x80000000
        sll     a1, a1, 8
        or      a1, a1, a4

// Divide, a0/a1

#if __SEGGER_RTL_CORE_HAS_MUL_MULH && __SEGGER_RTL_CORE_HAS_DIV

// Add an initial step so that we generate 25 bits of quotient: 24 bits are the result,
// with one rounding bit.
        sub     a3, a0, a1              // Divide step.
        bgeu    a0, a1, L(ordered)
        LEAH    a3, a1, a3              // Subtraction failed
        add     a2, a2, -1              // And adjust exponent

// Shift divisor to low 24 bits, divisor is exactly 24 bits in width and the dividend
// occupies the high 24 bits aligned to bit 31.
L(ordered):
        srl     a1, a1, 8

// Develop eight fractional bits of quotient into a4 and remainder into a3.
#if __SEGGER_RTL_CORE_HAS_FUSED_DIVREM
        divu    a4, a3, a1              // a4 := a3 / a1
        remu    a3, a3, a1              // a3 := a3 % a1
#else
        divu    a4, a3, a1              // a4 := a3 / a1
        mul     t0, a4, a1              // a3 := a3 - a4*a1, i.e. a3 = a3 % a1 from above
        sub     a3, a3, t0
#endif

// Develop eight fractional bits of quotient into a0 and remainder into a3.
        sll     a3, a3, 8               // Shift remainder in order to fractional divide 32/24 with 8-bit quotient
#if __SEGGER_RTL_CORE_HAS_FUSED_DIVREM
        divu    a0, a3, a1              // a0 := a3 / a1
        remu    a3, a3, a1              // a3 := a3 % a1
#else
        divu    a0, a3, a1              // a0 := a3 / a1
        mul     t0, a0, a1              // a3 := a3 - a0*a1, i.e. a3 = a3 % a1 from above
        sub     a3, a3, t0
#endif

// Combine fractional parts of quotient.
        sll     a4, a4, 8
        add     a0, a0, a4

// Develop eight fractional bits of quotient into a4 and no requirement for remainder.
        sll     a3, a3, 8               // Shift remainder in order to fractional divide 32/24 with 8-bit quotient
        divu    a4, a3, a1

// Combine fractional parts of quotient.
        sll     a0, a0, 8
        add     a0, a0, a4

// Insert sign, align significand, shift out least-significant bit of quotient into C.
        andi    a1, a0, 1
        srl     a0, a0, 1

// Insert exponent taking care of overflow and also inserting the rounding bit.
// In rounding, it's impossible for a carry out of the 24th bit of the quotient.
// The reason is that the quotient would have to be 0xFFFFFF and this isn't
// possible, given the range of normalized inputs (where the lower 24 bits are
// always zero), except in an isolated case (0x7FFFFF800000/0x800000), where
// there is no rounding required because there is no remainder.
        add     a0, a0, a1

#else

// Move significands down leaving zero on high.
        srl     a3, a0, 1               // use a3 as dividend as we'll form the
        srl     a1, a1, 1               // quotient in a0.

// Set a single bit in a0 which is a sentinel and clear out quotient.
// On each iteration as the quotient is developed, the sentinel bit
// shifts too and when it pops into the sign bit, we have iterated
// 23 times with the quotient is developed into the same register.
        li      a0, 0x100

// First subtraction step which is special depending on magnitudes of significands.
        bgeu    a3, a1, L(can_subtract)

// Can't subtract, adjust dividend and exponent accordingly.
        add     a2, a2, -1
        sll     a3, a3, 1

// Can subtract, so do so.
L(can_subtract):
        sub     a3, a3, a1

// Shift-and-subtract loop proper.
L(shift_and_subtract):
        sll     a3, a3, 1                   // Shift dividend
        sll     a0, a0, 1                   // Shift quotient to develop a 0 quotient bit
        bltu    a3, a1, L(cant_subtract)    // If can't subtract, don't, and leave developed quotient bit as 0
        sub     a3, a3, a1                  // Subtract divisor from working dividend
        add     a0, a0, 1                   // Replace developed 0 quotient bit with a 1 quotient bit
L(cant_subtract):
        bgez    a0, L(shift_and_subtract)

// Remove sentinel from quotient.
        sub     a0, a0, a4

// Quotient is now in a0[23:0] with a0[31:24] zero.

// We use the remainder after division to find which direction
// to round in and break ties.  a3 is the remainder after division,
// and a0 is the quotient [see above], and a1 the divisor.
//
// In rounding, it's impossible for a carry out of the 24th bit
// of the quotient.  The reason is that the quotient would have
// to be 0xFFFFFF and this isn't possible, given the range of
// normalized inputs (where the lower 24 bits are always zero),
// except in an isolated case (0x7FFFFF800000/0x800000), where
// there is no rounding required because there is no remainder.
//
        sub     a1, a1, a3              // divisor - remainder
        slt     a3, a3, a1              // compare with remainder, i.e. compare to 0.5.
        xor     a3, a3, 1
        add     a0, a0, a3

#endif

// Apply the IEEE bias.  If close to underflow, decide what to do with rounding.
        add     a2, a2, 127
        blez    a2, L(signed_zero)

// If overflowed, return a signed infinity.
        bgeu    a2, t1, L(inf)

// Insert the exponent.
        sll     a2, a2, 23              // Align exponent to correct position
        add     a0, a0, a2

// Insert sign into quotient.
L(apply_sign):
        or      a0, a0, a5
        ret

// Generate a positive infinity, then apply quotient sign.
L(inf):
        li      a0, 0x7F800000
        j       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// Treat subnormals as zero; if zero we're computing 0/0 which is NaN.
        beqz    a3, L(nan)

// If exponent is non-zero then the rhs is non-zero normal and not Inf
// and not NaN, hence 0/normal is signed zero.
        bne     a3, t1, L(signed_zero)             // Return signed zero.

// rhs is either Inf or Nan.  We distinguish between the two by the significand
// bits.  If it's Inf, then 0/Inf is a signed zero.
        sll     a1, a1, 9
        beqz    a1, L(signed_zero)             // Return signed zero.

// Result is NaN.
L(nan):
        li      a0, 0x7FC00000
        ret

// We know that the LHS is Inf or NaN, but we haven't a clue what the rhs is (yet).
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.  Use significand bits to differentiate
// between NaN and Inf.
        sll     a0, a0, 9
        bnez    a0, L(nan)                     // non-zero significand implies lhs NaN

// Now know the lhs is Inf.  If rhs is normal then Inf/zero-normal is Inf.
        bne     a3, t1, L(inf)

// We now have a rhs of Inf or NaN, and Inf/Inf and Inf/NaN are both NaN.
        j       L(nan)

// We know that the rhs is zero or subnormal and the lhs is non-zero, Inf, or NaN.
L(rhs_zero_or_subnormal):

// If the lhs in normal, then normal/0 is a signed infinity.
        bne     a2, t1, L(inf)

// Know the lhs is either Inf or NaN.  If NaN, NaN/normal is NaN.
        sll     a0, a0, 9               // non-zero significand implies lhs NaN
        bnez    a0, L(nan)              // ...and a NaN result

// Inf/0 is still Inf.
        j       L(inf)

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// Non-zero normal/NaN is NaN.
        sll     a1, a1, 9               // non-zero significand implies rhs NaN
        bnez    a1, L(nan)              // ...and a NaN result

// Non-zero normal/Inf is a signed zero.
L(signed_zero):
        mv      a0, a5                  // Correctly signed zero
        ret

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_CORE_HAS_MUL_MULH && (__SEGGER_RTL_OPTIMIZE >= 0) && !__SEGGER_RTL_CORE_HAS_DIV

// Set Inf/NaN exponent.
        li      t0, 0xFF

// Continue with opening book: discard sign of operands, move exponents
// to low byte of operand registers.
        BFOZ    a4, a0, 30, 23
        BFOZ    a5, a1, 30, 23

// Compute sign of quotient and store it into t1{31}.
        xor     t1, a0, a1              // Sign to t1{31}
        srl     t1, t1, 31
        sll     t1, t1, 31

// Special handling of exponents 0x00 and 0xFF for zero/subnormal and Inf/NaN.
        beqz    a4, L(lhs_zero_or_subnormal)
        beqz    a5, L(rhs_zero_or_subnormal)
        beq     a4, t0, L(lhs_inf_or_nan)
        beq     a5, t0, L(rhs_inf_or_nan)

// Compute exponent of quotient.
        sub     a4, a4, a5

// Load approximation to reciprocal (32 bits) into t0 using
// leading six bits of significand.
       .set     __SEGGER_RTL_fdiv_reciprocal_table_REQUIRED, 1
        la      t0, __SEGGER_RTL_fdiv_reciprocal_table
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bfoz    a3, a1, 22, 17
        lea.w   a3, t0, a3
#else
        srl     a3, a1, 15
        and     a3, a3, 0xFC
        add     a3, a3, t0
#endif
        lw      a5, (a3)

// Break reciprocal into sign-extended high 8 bits in a2 and
// lower 24 bits in a5 at (*) below.
        sra     a2, a5, 24

// Perform fractional multiply.
        BFOZ    a3, a1, 16, 9
        and     t0, a3, 1
        srl     a3, a3, 1
        add     a3, a3, -64
        add     a3, a3, t0
        mul     a2, a2, a3
        BFOZ    a5, a5, 23, 0           // (*) Schedule this here rather than above to overlap multiply
        add     a2, a2, a5

// Clear away exponent and set hidden bit.
        BFOZ    a1, a1, 23, 0
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a1, a1, 23
#else
        li      a5, 0x00800000
        or      a1, a1, a5
#endif

// Start multiply early.
        mulw    a3, a1, a2

// Clear away exponent and set hidden bit.
        BFOZ    a0, a0, 23, 0
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 23
#else
        or      a0, a0, a5
#endif

// Fractional multiply reciprocal by dividend which approximates
// the quotient in a5.
        mul     a5, a3, a2
        srl     a5, a5, 32

// Normalize ensuring a0 >= a1.
        sltu    t0, a0, a1               // Conditionallly normalize
        sll     a0, a0, t0               // ...conditionally shift
        sub     a4, a4, t0               // ...conditionally decrement exponent
        srl     a3, a2, 31-11
        and     a3, a3, 1
        xor     a3, a3, 1
        sll     a2, a2, 11
        sub     a2, a2, a5
        sub     a2, a2, a3

// Fractional multiply reciprocal by dividend which approximates
// the quotient in a5.
        sll     a0, a0, 2
        sll     a2, a2, 32
        srl     a2, a2, 32
        mul     a5, a2, a0
        srl     a5, a5, 32

// Get unshifted, biased quotient exponent into a2.  Hoisted
// so that multiply can proceed without hazards.
        add     a2, a4, 0x7E

// Apply correction.
        sll     a0, a0, 22
        sub     a0, a0, a1
        mulw    a3, a5, a1              // Multiply early to hide result latency

// Check for underflow or overflow condition.
        li      t0, 0xFE
        bgeu    a2, t0, L(underflow_or_overflow)

// Rounding, compare remainder.
        subw    a0, a3, a0
        sltz    t0, a0                  // Round up if normalized
        add     a5, a5, t0
        and     a0, a5, 1               // Rounding bit
        srl     a5, a5, 1               // Significand to position
        add     a0, a0, a5              // Significand to position with rounding
        sll     a2, a2, 23              // Exponent to position
        add     a0, a0, a2              // Apple exponent
        add     a0, a0, t1              // Apply sign
        ret

// Underflow and overflow handling.
L(underflow_or_overflow):
        mv      a0, t1                  // Put sign into a0{31}, make signed zero.
        blt     a2, t0, L(done)
        li      t1, 0x7F800000          // If overflow, make Inf

// Insert sign into quotient.
L(apply_sign):
        add     a0, a0, t1
L(done):
        ret

// Generate a positive infinity, then apply quotient sign.
L(inf):
        li      a0, 0x7F800000
        j       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// Treat subnormals as zero; if zero we're computing 0/0 which is NaN.
        beqz    a5, L(nan)

// If exponent is non-zero then the rhs is non-zero normal and not Inf
// and not NaN, hence 0/normal is signed zero.
        bne     a5, t0, L(signed_zero)  // Return signed zero.

// rhs is either Inf or Nan.  We distinguish between the two by the significand
// bits.  If it's Inf, then 0/Inf is a signed zero.
        sll     a1, a1, 32+9
        beqz    a1, L(signed_zero)      // Return signed zero.

// Result is NaN.
L(nan):
        li      a0, 0x7FC00000
        ret

// We know that the LHS is Inf or NaN, but we haven't a clue what the rhs is (yet).
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.  Use significand bits to differentiate
// between NaN and Inf.
        sll     a0, a0, 32+9
        bnez    a0, L(nan)              // non-zero significand implies lhs NaN

// Now know the lhs is Inf.  If rhs is normal then Inf/zero-normal is Inf.
        bne     a5, t0, L(inf)

// We now have a rhs of Inf or NaN, and Inf/Inf and Inf/NaN are both NaN.
        j       L(nan)

// We know that the rhs is zero or subnormal and the lhs is non-zero, Inf, or NaN.
L(rhs_zero_or_subnormal):

// If the lhs in normal, then normal/0 is a signed infinity.
        bne     a4, t0, L(inf)

// Know the lhs is either Inf or NaN.  If NaN, NaN/normal is NaN.
        sll     a0, a0, 32+9            // non-zero significand implies lhs NaN
        bnez    a0, L(nan)              // ...and a NaN result

// Inf/0 is still Inf.
        j       L(inf)

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// Non-zero normal/NaN is NaN.
        sll     a1, a1, 32+9            // non-zero significand implies rhs NaN
        bnez    a1, L(nan)              // ...and a NaN result

// Non-zero normal/Inf is a signed zero.
L(signed_zero):
        mv      a0, t1                  // Correctly signed zero
        ret

#else

// Generally-useful constant.
        li      t1, 0xFF

// Standard opening: compute sign of quotient into a5.
        xor     a5, a0, a1
        li      a4, 1                   // Load 0x80000000
        sll     a4, a4, 31
        and     a5, a5, a4              // Isolate sign

// Continue with opening book: discard sign of operands, extract exponents
// to low byte of operand registers.
        BFOZ    a2, a0, 30, 23
        BFOZ    a3, a1, 30, 23

// Handle zero/subnormal inputs.
        beqz    a2, L(lhs_zero_or_subnormal)
        beqz    a3, L(rhs_zero_or_subnormal)

// Handle inf/NaN inputs.
        beq     a2, t1, L(lhs_inf_or_nan)
        beq     a3, t1, L(rhs_inf_or_nan)

// Compute quotient's exponent in a2, now true exponent without bias.
        sub     a2, a2, a3

// Standard form for division: align significands right and insert hidden bit.
        sll     a4, a4, 32              // Generate 0x80000000'00000000.
        sll     a0, a0, 32+8            // align to right
        or      a0, a0, a4              // insert sign bit, a4 is 0x80000000
        sll     a1, a1, 32+8
        or      a1, a1, a4

// Divide, a0/a1

#if __SEGGER_RTL_CORE_HAS_MUL_MULH && __SEGGER_RTL_CORE_HAS_DIV

// Add an initial step so that we generate 25 bits of quotient: 24 bits are the result,
// with one rounding bit.
        sub     a3, a0, a1              // Divide step.
        bgeu    a0, a1, L(ordered)
        LEAH    a3, a1, a3              // Subtraction failed
        add     a2, a2, -1              // And adjust exponent

L(ordered):

// Align divisor to complete division with one additional quotient bit,
// and do fractional divide.
        srl     a1, a1, 24
        divu    a0, a3, a1

// Round result.
        andi    a1, a0, 1
        srl     a0, a0, 1

// Insert exponent taking care of overflow and also inserting the rounding bit.
// In rounding, it's impossible for a carry out of the 24th bit of the quotient.
// The reason is that the quotient would have to be 0xFFFFFF and this isn't
// possible, given the range of normalized inputs (where the lower 24 bits are
// always zero), except in an isolated case (0x7FFFFF800000/0x800000), where
// there is no rounding required because there is no remainder.
        add     a0, a0, a1

#else

// Move significands down leaving zero on high.
        srl     a3, a0, 1               // use a3 as dividend as we'll form the
        srl     a1, a1, 1               // quotient in a0.

// Set a single bit in a0 which is a sentinel and clear out quotient.
// On each iteration as the quotient is developed, the sentinel bit
// shifts too and when it pops into the sign bit, we have iterated
// 23 times with the quotient is developed into the same register.
        li      a0, 1<<(8+32)

// First subtraction step which is special depending on magnitudes of significands.
        bgeu    a3, a1, L(can_subtract)

// Can't subtract, adjust dividend and exponent accordingly.
        add     a2, a2, -1
        sll     a3, a3, 1

// Can subtract, so do so.
L(can_subtract):
        sub     a3, a3, a1

// Shift-and-subtract loop proper.
L(shift_and_subtract):
        sll     a3, a3, 1                   // Shift dividend
        sll     a0, a0, 1                   // Shift quotient to develop a 0 quotient bit
        bltu    a3, a1, L(cant_subtract)    // If can't subtract, don't, and leave developed quotient bit as 0
        sub     a3, a3, a1                  // Subtract divisor from working dividend
        add     a0, a0, 1                   // Replace developed 0 quotient bit with a 1 quotient bit
L(cant_subtract):
        bgez    a0, L(shift_and_subtract)

// Remove sentinel from quotient.
        sub     a0, a0, a4

// Quotient is now in a0[23:0] with a0[31:24] zero.

// We use the remainder after division to find which direction
// to round in and break ties.  a3 is the remainder after division,
// and a0 is the quotient [see above], and a1 the divisor.
//
// In rounding, it's impossible for a carry out of the 24th bit
// of the quotient.  The reason is that the quotient would have
// to be 0xFFFFFF and this isn't possible, given the range of
// normalized inputs (where the lower 24 bits are always zero),
// except in an isolated case (0x7FFFFF800000/0x800000), where
// there is no rounding required because there is no remainder.
//
        sub     a1, a1, a3              // divisor - remainder
        slt     a3, a3, a1              // compare with remainder, i.e. compare to 0.5.
        xor     a3, a3, 1
        add     a0, a0, a3

#endif

// Apply the IEEE bias.  If close to underflow, decide what to do with rounding.
        add     a2, a2, 127
        blez    a2, L(signed_zero)

// If overflowed, return a signed infinity.
        bgeu    a2, t1, L(inf)

// Insert the exponent.
        sll     a2, a2, 23              // Align exponent to correct position
        add     a0, a0, a2

// Insert sign into quotient.
L(apply_sign):
        or      a0, a0, a5
        ret

// Generate a positive infinity, then apply quotient sign.
L(inf):
        li      a0, 0x7F800000
        j       L(apply_sign)

// We know that the LHS is zero or subnormal.  Treat subnormals as zero anyway.
L(lhs_zero_or_subnormal):

// Treat subnormals as zero; if zero we're computing 0/0 which is NaN.
        beqz    a3, L(nan)

// If exponent is non-zero then the rhs is non-zero normal and not Inf
// and not NaN, hence 0/normal is signed zero.
        bne     a3, t1, L(signed_zero)         // Return signed zero.

// rhs is either Inf or Nan.  We distinguish between the two by the significand
// bits.  If it's Inf, then 0/Inf is a signed zero.
        sll     a1, a1, 32+9
        beqz    a1, L(signed_zero)             // Return signed zero.

// Result is NaN.
L(nan):
        li      a0, 0x7FC00000
        ret

// We know that the LHS is Inf or NaN, but we haven't a clue what the rhs is (yet).
L(lhs_inf_or_nan):

// If the lhs is NaN, the result must be NaN.  Use significand bits to differentiate
// between NaN and Inf.
        sll     a0, a0, 32+9
        bnez    a0, L(nan)              // non-zero significand implies lhs NaN

// Now know the lhs is Inf.  If rhs is normal then Inf/zero-normal is Inf.
        bne     a3, t1, L(inf)

// We now have a rhs of Inf or NaN, and Inf/Inf and Inf/NaN are both NaN.
        j       L(nan)

// We know that the rhs is zero or subnormal and the lhs is non-zero, Inf, or NaN.
L(rhs_zero_or_subnormal):

// If the lhs in normal, then normal/0 is a signed infinity.
        bne     a2, t1, L(inf)

// Know the lhs is either Inf or NaN.  If NaN, NaN/normal is NaN.
        sll     a0, a0, 32+9            // non-zero significand implies lhs NaN
        bnez    a0, L(nan)              // ...and a NaN result

// Inf/0 is still Inf.
        j       L(inf)

// We know that the rhs is Inf or NaN and the lhs is non-zero normal.
L(rhs_inf_or_nan):

// Non-zero normal/NaN is NaN.
        sll     a1, a1, 32+9            // non-zero significand implies rhs NaN
        bnez    a1, L(nan)              // ...and a NaN result

// Non-zero normal/Inf is a signed zero.
L(signed_zero):
        mv      a0, a5                  // Correctly signed zero
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __divsf3

/*********************************************************************
*
*       __divdf3()
*
*  Function description
*    Divide, double.
*
*  Prototype
*    double __divdf3(double x, double y);
*
*  Parameters
*    a1:a0 - x - Dividend.
*    a2:a1 - y - Divisor.
*
*  Return value
*    a1:a0 - Quotient.
*/

#undef L
#define L(label) .L__divdf3_##label

GLOBAL_FUNC __divdf3

#if __SEGGER_RTL_FP_ABI >= 2

        fdiv.d  fa0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32 
//

#if (__SEGGER_RTL_OPTIMIZE < 0) || !__SEGGER_RTL_CORE_HAS_MUL_MULH

// Compute sign of quotient into t1.
        xor     t1, a1, a3
        li      t0, 0x80000000          // Isolate sign bit only
        and     t1, t1, t0

// Remove sign bit from lhs and rhs and shift significands and exponents into position.
        add     a4, a1, a1
        add     a5, a3, a3

// If the lhs/rhs has a special exponent indicating Inf or NaN, do that out of line.
        li      t2, 0xFFE00000
        bgeu    a4, t2, L(lhs_inf_or_nan)
        bgeu    a5, t2, L(rhs_inf_or_nan)

// Align lhs exponent to lsb.  If the exponent is zero, that indicates that the
// lhs is zero or subnormal and requires special treatment.
        srl     a4, a4, 21
        srl     a5, a5, 21
        beqz    a4, L(lhs_zero_or_subnormal)

// Now know that the lhs operand is normal and the rhs operand is zero,
// subnormal, or normal.  Align rhs exponent to lsb.  If the rhs exponent is
// zero, then we have normal/0 which is a signed Inf.
        beqz    a5, L(inf)

// Compute quotient's exponent into t2.
        sub     t2, a4, a5

// Materialize hidden bit in lhs, moving significand to a5:a4.
        sll     a1, a1, 11              // lhs aligned to msb
        or      a1, a1, t0              // materialize hidden bit
        srl     a5, a1, 11              // move back into position, but in a5.
        mv      a4, a0

// Do the same for the rhs.
        sll     a3, a3, 11
        or      a3, a3, t0
        srl     a3, a3, 11

// First subtraction step which is special depending on magnitudes of significands.
// See if subtraction will succeed.
        bgtu    a5, a3, L(first_can_subtract)
        bne     a5, a3, L(first_cant_subtract)
        bgtu    a4, a2, L(first_can_subtract)

// Can't subtract, adjust dividend and exponent accordingly.
L(first_cant_subtract):
        add     t2, t2, -1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a4, a4, a4
#else
        sltz    t0, a4                  // Calculate low-part carry out
        add     a4, a4, a4
        LEAH    a5, t0, a5              // Carry in
#endif

// Can subtract, so do so.
L(first_can_subtract):
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        sub64   a4, a4, a2
#else
        mv      t0, a4                  // 64-bit subtract, a5:a4 -= a3:a2
        sub     a4, a4, a2
        sltu    t0, t0, a4
        sub     a5, a5, a3
        sub     a5, a5, t0
#endif

// Apply double precision bias (3FF) to exponent.
        li      a0, 0x3FF
        add     t2, t2, a0              // apply bias

// If exponent underflowed to zero then return a signed zero--we don't
// support subnormals.
        blez    t2, L(signed_zero)

// If overflowed, i.e. exponent >= 0x7FF, then Inf.
        add     a0, a0, a0              // 0x000007FE
        bgtu    t2, a0, L(inf)          // > 0x7FE implies >= 0x7FF

// Prepare quotient registers; set a sentinel bit to indicate
// when to exit the loop.  As the quotient is is shifted on each
// iteration when developing the quotient, the sentinel bit will
// eventually pop into the sign bit indication completion.
        li      a0, 0x800
        li      a1, 0

// Shift-and-subtract loop proper.
L(divide_one_bit):

// Shift dividend.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a4, a4, a4
#else
        sltz    t0, a4                  // Calculate low-part carry out
        add     a4, a4, a4
        LEAH    a5, t0, a5              // Carry in
#endif

// Shift quotient to develop a 0 quotient bit.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a0
#else
        sltz    t0, a0                  // Calculate low-part carry out
        add     a0, a0, a0
        LEAH    a1, t0, a1              // Carry in
#endif

// See if subtraction will succeed.
        bltu    a5, a3, L(nth_cant_subtract)
        bne     a5, a3, L(nth_can_subtract)
        bltu    a4, a2, L(nth_cant_subtract)

// Subtract divisor from working dividend.
L(nth_can_subtract):
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        sub64   a4, a4, a2
#else
        mv      t0, a4                  // 64-bit subtract, a5:a4 -= a3:a2
        sub     a4, a4, a2
        sltu    t0, t0, a4
        sub     a5, a5, a3
        sub     a5, a5, t0
#endif

// Replace developed 0 quotient bit with a 1 quotient bit.
        add     a0, a0, 1

// If the sentinel didn't pop out, need to carry on.
L(nth_cant_subtract):
        bgez    a1, L(divide_one_bit)

// Remove sentinel from quotient.
        sll     a1, a1, 1
        srl     a1, a1, 1

// Move exponent into position.
        sll     t2, t2, 20

// Now need to round.  Test remainder, if < 1/2 the divisor then there
// is no rounding to be done.  What we actually do is double both sides
// of the inequality, i.e. if remainder < divisor/2 is 2*remainder < divisor.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a4, a4, a4
#else
        sltz    t0, a4                  // Calculate low-part carry out
        add     a4, a4, a4              // double remainder
        LEAH    a5, t0, a5
#endif
        bltu    a5, a3, L(insert_exponent)
        bne     a5, a3, L(rounding)
        bltu    a4, a2, L(insert_exponent)

// Must now round. The less-than-half divisor case is already done
// so we unconditionally round up the quotient and then knock out
// bit zero if the remainder is exactly half the divisor or, put
// another way, if twice the remainder is equal to the divisor.
L(rounding):
        add     a0, a0, 1               // Unconditionally round quotient up
        seqz    t0, a0
        add     a1, a1, t0

// If twice the remainder is not equal to the divisor, no tie to break.
        bne     a5, a3, L(no_tie)
        bne     a4, a2, L(no_tie)

// Break tie to nearest even.
        and     a0, a0, ~1
L(no_tie):

// Insert exponent into significand; the exponent is in t2.  We also add the
// carry held over from the previous instruction.  If we land up here from
// the branch above, the carry is clear anyway and makes no contibution.
L(insert_exponent):
        add     a1, a1, t2

// Apply quotient sign to result; the quotient sign is in t1.
L(apply_sign):
        or      a1, a1, t1

// All done, so restore working set and return.
L(done):
        ret

// Know exponent is 0x7FF indicating Inf or NaN.  If significand is non-zero,
// it's a NaN and NaN/anything is NaN.
L(lhs_inf_or_nan):
        bne     a4, t2, L(nan)
        bnez    a0, L(nan)

// Inf/Inf is NaN, and Inf/NaN is NaN, so there's no need to distinguish
// whether the rhs is Inf or NaN as they both generate NaN anyway.
        bgeu    a5, t2, L(nan)

// Now we know that the lhs is Inf and the rhs is normal, zero, or subnormal.
// Inf/0 or Inf/Normal is Inf.
L(inf):
        li      a1, 0x7FF00000
        li      a0, 0
        j       L(apply_sign)

// Know the lhs is normal or zero.  If the rhs is NaN then normal/NaN or
// 0/NaN is NaN.  If the rhs is Inf then 0/Inf and Normal/Inf is a signed zero.
L(rhs_inf_or_nan):
        bnez    a2, L(nan)
        beq     a5, t2, L(signed_zero)

// Generate a EABI NaN result.
L(nan):
        li      a1, 0x7FF80000
        li      a0, 0
        j       L(done)

// Know lhs is zero or subnormal and rhs is zero or subnormal (i.e. not Inf
// and not NaN).  If rhs is zero or subnormal then we have 0/0 which is NaN.
L(lhs_zero_or_subnormal):
        beqz    a5, L(nan)              // 0/0 or 0/subnormal is NaN.

// Have 0/normal which is a signed zero.
L(signed_zero):
        li      a0, 0
        mv      a1, t1
        j       L(done)

#elif __SEGGER_RTL_CORE_HAS_ISA_RVE                                     \
   || ((__SEGGER_RTL_OPTIMIZE == 0) && !__SEGGER_RTL_CORE_HAS_ISA_SIMD) \
   || ((__SEGGER_RTL_OPTIMIZE >= 0) &&  __SEGGER_RTL_CORE_HAS_ISA_SIMD)

// This algorithm is only faster than the multiply-only algorithm below if compiled
// for SIMD to take advantage of the rich 64-bit operations.  Regular RV32IMAC is
// better served by the multiply-only version below for outright speed.  This is the
// preferred option for RV32E when compiling for balanced and above.

#if __SEGGER_RTL_CORE_HAS_ISA_RVE

#define a6 s0
#define a7 s1
#define t3 ra

        add     sp, sp, -STACK_ALIGN(12)
        sw      s0, 0(sp)
        sw      s1, 4(sp)
        sw      ra, 8(sp)
#endif

// t2:t1 can be a register pair
// a5:a4 can be a register pair

// Extract exponents.
        BFOZ    a6, a3, 30, 20
        BFOZ    a7, a1, 30, 20

// Generate quotient sign to t2.
        xor     t2, a1, a3
        srl     t2, t2, 31
        sll     t2, t2, 31

// Deal with special cases of Inf, NaN, zero, subnormal
        li      t0, 0x7FF               // Exponent for Inf and NaN
        beq     a7, t0, L(inf_nan_over) // {Inf, NaN} / x
        beqz    a6, L(div_zero)         // x / 0
        beq     a6, t0, L(div_inf_nan)  // x / {Inf, NaN}, x is finite or zero
        beqz    a7, L(signed_zero)      // 0 / finite is zero

// Hot trace: operands are both normal. Calculate exponent of quotient.
        sub     a7, a7, a6
        addi    a7, a7, 0x3FF           // Reinstate excess-1023 exponent of binary64 format

// Extract significands.
        BFOZ    a1, a1, 19, 0           // a1:a0 = dividend
        BFOZ    a3, a3, 19, 0           // a3:a2 = divisor

// Materialize hidden bit.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a1, a1, 20
        bseti   a3, a3, 20
#else
        li      a4, 1<<20
        or      a1, a1, a4
        or      a3, a3, a4
#endif

// We require dividend > divisor, but this might not be the case.
// Generating leading bit by trial subtraction.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        sub64   a0, a0, a2
#else
        sltu    a4, a0, a2
        add     a4, a4, a3
        sub     a1, a1, a4
        sub     a0, a0, a2
#endif

// High bits cancelled without underflow?
        bgez    a1, L(can_subtract)

// Underflowed, dividend significand less than divisor significand... adjust.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a0              // 2(x - y)
        add64   a0, a0, a2              // 2(x - y) + y = 2x - 2y + y = 2x -y
#else
        sltz    a4, a0                  // a4 := carry out of a0 shift
        LEAH    a1, a4, a1              // a1 := a4 + 2a1
        add     a1, a1, a3
        LEAH    a0, a2, a0
        sltu    a4, a0, a2
        add     a1, a1, a4
#endif

// Adjust quotient exponent to recognize dividend shift.
        addi    a7, a7, -1

// No more adjustments to the exponent will be made, it's final here.
// Check for overflow and underflow.
L(can_subtract):
        bge     a7, t0, L(signed_inf)
        blez    a7, L(signed_zero)

// Shift dividend significand to have msb=1 at bit position 62.
        slli    a1, a1, 10              // a1:a0 <<= 10
        srli    a4, a0, 22
        or      a1, a1, a4
        slli    a0, a0, 10

// Let Q' = X/Y' be an approximation to X/Y where Y' is smaller than Y.
// We test the correctness of the quotient estimate Q' by calculating
// Q' * Y with Y to full precision.  If X < Q' * Y then Q' is overestimated
// with the quotient too big and an adjustment must be made with Q' = X/Y' - 1.
// 
// This is simmple to code by calculating Q' := X/Y'; X := X - Q' * Y.
// If X is nonnegative, then then Q' is exact.  If X is negative, then Q'
// is too big and the adjustment Q := Q'-1 is made.  At the same time, the
// dividend update X := X - Q' * Y must also be adjusted.  We require
// X := X - (Q'-1) * Y but have X := X - Q' * Y, hence the correction
// is X := X - Y (as X - (Q'+1)*Y is X - Q'*Y - Y).

// The true quotient is developed over five division steps, developing
// 10, 10, 11, 11, 11 bits at a time to produce the 53-bit quotient
// with a single rounding bit.

// a6     hold the high-order 20 bits of quotient developed over two divisions.
// t0/t1  hold the middle-order 22 bits of quotient developed over two divisions.

#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        divu    a6, a1, a3              // 10-bit estimate of quotient Q' using 31 bits of Y'.
        msubr32 a1, a6, a3              // X[high] := X - Q' * Y[high]
        mulr64  a4, a6, a2              // T       :=     Q' * Y[low]
        sub64   a0, a0, a4              // X       := X - Q' * Y[low]
#else
#if __SEGGER_RTL_CORE_HAS_FUSED_DIVREM
        divu    a6, a1, a3              // 10-bit estimate of quotient Q' using 31 bits of Y'.
        remu    a1, a1, a3              // X[high] = X[high] - X/Y' * Y'
#else
        divu    a6, a1, a3              // 10-bit estimate of quotient Q' using 31 bits of Y'.
        mul     t3, a6, a3
        sub     a1, a1, t3
#endif
        mul     a4, a6, a2              // T[high:low]  := Q' * Y[low]...
        mulhu   a5, a6, a2              // ...using two multiplies
        sltu    t3, a0, a4              // Compute carry out of subtraction...
        add     a5, a5, t3              // ...and fold into high-order subtraction
        sub     a0, a0, a4              // X[low]  := X[low]  - Q' * Y[low]
        sub     a1, a1, a5              // X[high] := X[high] - Q' * Y[low] - carry from previous step
#endif
        bgez    a1, L(qdash_correct_1)  // If X nonnegative, no correction required and Q := Q'

// Correction step required as outlined above.
        addi    a6, a6, -1              // Q := Q'-1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a2              // X := X + Y
#else
        add     a0, a0, a2              // X := X + Y longform
        add     a1, a1, a3
        sltu    t0, a0, a2
        add     a1, a1, t0
#endif

// Now continue with division.  The 10 leading bits of dividend are zero (after
// the subtraction), and so the dividend is shifted 10 bits ready to generate the
// following group of 10 quotient bits into t0.
L(qdash_correct_1):
        slli    a1, a1, 10              // a1:a0 <<= 10
        srli    t0, a0, 22
        add     a1, a1, t0              // or a1, a1, t0 can't be compact
        slli    a0, a0, 10

// Run a division step as above.  Comments omitted...
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        divu    t0, a1, a3
        mulr64  a4, t0, a2
        msubr32 a1, t0, a3
        sub64   a0, a0, a4
#else
#if __SEGGER_RTL_CORE_HAS_FUSED_DIVREM
        divu    t0, a1, a3
        remu    a1, a1, a3
#else
        divu    t0, a1, a3
        mul     a4, t0, a3
        sub     a1, a1, a4
#endif
        mul     a4, t0, a2
        mulhu   a5, t0, a2
        sltu    t3, a0, a4
        add     a5, a5, t3
        sub     a0, a0, a4
        sub     a1, a1, a5
#endif
        bgez    a1, L(qdash_correct_2)
        addi    t0, t0, -1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a2
#else
        add     a0, a0, a2
        add     a1, a1, a3
        sltu    t3, a0, a2
        add     a1, a1, t3
#endif

// Fold the next group of 10 quotient bits from t0 into a6.
L(qdash_correct_2):
        slli    a6, a6, 10
        add     a6, a6, t0

// Now continue with division.  The ten leading bits of dividend are zero (after
// the subtraction), and so the dividend is shifted 11 bits ready to generate the
// following group of 11 quotient bits into t0.
        slli    a1, a1, 11
        srli    t3, a0, 21
        add     a1, a1, t3              // Alternate "or a1, a1, t3" can't be compact
        slli    a0, a0, 11

// Run a division step as above.  Comments omitted...
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        divu    t0, a1, a3
        mulr64  a4, t0, a2
        msubr32 a1, t0, a3
        sub64   a0, a0, a4
#else
#if __SEGGER_RTL_CORE_HAS_FUSED_DIVREM
        divu    t0, a1, a3
        remu    a1, a1, a3
#else
        divu    t0, a1, a3
        mul     a4, t0, a3
        sub     a1, a1, a4
#endif
        mul     a4, t0, a2
        mulhu   a5, t0, a2
        sltu    t3, a0, a4
        add     a5, a5, t3
        sub     a0, a0, a4
        sub     a1, a1, a5
#endif
        bgez    a1, L(qdash_correct_3)
        addi    t0, t0, -1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a2
#else
        add     a0, a0, a2
        add     a1, a1, a3
        sltu    t3, a0, a2
        add     a1, a1, t3
#endif

// The next group of 11 quotient bits is held in t0.  We don't accumulate them
// here, we construct the quotient after all 33 bits have been calculated
// into different registers...
L(qdash_correct_3):

// Now continue with division.  The 11 leading bits of dividend are zero (after
// the subtraction), and so the dividend is shifted 11 bits ready to generate the
// following group of 11 quotient bits into t1.
        slli    a1, a1, 11
        srli    t3, a0, 21
        add     a1, a1, t3
        slli    a0, a0, 11

// Run a division step as above.  Comments omitted...
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        divu    t1, a1, a3
        mulr64  a4, t1, a2
        msubr32 a1, t1, a3
        sub64   a0, a0, a4
#else
#if __SEGGER_RTL_CORE_HAS_FUSED_DIVREM
        divu    t1, a1, a3
        remu    a1, a1, a3
#else
        divu    t1, a1, a3
        mul     a4, t1, a3
        sub     a1, a1, a4
#endif
        mul     a4, t1, a2
        mulhu   a5, t1, a2
        sltu    t3, a0, a4
        add     a5, a5, t3
        sub     a0, a0, a4
        sub     a1, a1, a5
#endif
        bgez    a1, L(qdash_correct_4)
        addi    t1, t1, -1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a2
#else
        add     a0, a0, a2
        add     a1, a1, a3
        sltu    t3, a0, a2
        add     a1, a1, t3
#endif

// The next group of 11 quotient bits is held in t1.  Accumulate them here.
// The low-order 10 bits of running quotient are zero.
L(qdash_correct_4):
        slli    t0, t0, 21
        slli    t1, t1, 10
        add     t0, t0, t1

// Now continue with division.  The 11 leading bits of dividend are zero (after
// the subtraction), and so the dividend is shifted 11 bits ready to generate the
// following group of 11 quotient bits into t1.
        slli    a1, a1, 11
        srli    t3, a0, 21
        add     a1, a1, t3
        slli    a0, a0, 11

// Run a division step as above.  However, the correction is truncated as
// the dividend doesn't need to be adjusted, we're done with it.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        divu    t1, a1, a3
        mulr64  a4, t1, a2
        msubr32 a1, t1, a3
        sub64   a0, a0, a4
#else
#if __SEGGER_RTL_CORE_HAS_FUSED_DIVREM
        divu    t1, a1, a3
        remu    a1, a1, a3
#else
        divu    t1, a1, a3
        mul     a4, t1, a3
        sub     a1, a1, a4
#endif
        mul     a4, t1, a2
        mulhu   a5, t1, a2
        sltu    t3, a0, a4
        add     a5, a5, t3
//      sub     a0, a0, a4              // Difference from before: don't need to apply low order fix-up, we only need leading bit of Q'
        sub     a1, a1, a5
#endif

// Final group of 11 quotient bits are in t1.
//
// If X nonnegative, correction to quotient is Q := Q'
// If X is negative, correction to quotient is Q := Q' - 1.
        sra     a1, a1, 31              // Generate 0 or -1 correction from leading X bit...
        add     t1, t1, a1              // ...Apply appropriate correction

// Position exponent into high-order 32 bits of IEEE quotient and combine with
// leading 20 bits of quotient.
        slli    a7, a7, 20
        add     a1, a6, a7

// Combine 21 bits of low-order quotient witb final 11 bits of quotient,
// discarding the final low-order bit, that is used for rounding.
        srli    a0, t1, 1
        add     a0, a0, t0

// Final bit is used for rounding quotient up, the only way it can go as
// no ties are ever possible apart from the single 0x80000000 / 1 which
// is exact and never rounds up.
        andi    t1, t1, 1

// Combine final bits of quotient with 
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, t1              // Combine rounding and sign
#else
        add     a0, a0, t1              // Combine final bits of quotient 
        sltu    a4, a0, t1              // Calculate carry out and...
        add     a1, a1, a4              // ...combine carry out
        add     a1, a1, t2              // Combine sign
#endif

#if __SEGGER_RTL_CORE_HAS_ISA_RVE
L(ret):
        lw      s0, 0(sp)
        lw      s1, 4(sp)
        lw      ra, 8(sp)
        add     sp, sp, +STACK_ALIGN(12)
#endif
        ret

// {NaN,Inf} / x
L(inf_nan_over):
        slli    a1, a1, 12
        beq     a6, t0, L(return_nan)   // Division by zero
        or      a1, a1, a0              // Fold low-order bits of Inf/NaN into high order word
        bnez    a1, L(return_nan)       // NaN / x with x {Inf,NaN}

// Return signed Inf.
L(signed_inf):
        li      a1, 0x7FF00000
        j       L(apply_sign)

// Division by {Inf,NaN}.
L(div_inf_nan):
        slli    a3, a3, 12
        bnez    a3, L(return_nan)       // Return NaN if divisor is NaN.

// Division by Inf, return a signed zero.
L(signed_zero):
        li      a1, 0
L(apply_sign):
        add     a1, a1, t2
L(clr_low_ret):
        li      a0, 0
#if __SEGGER_RTL_CORE_HAS_ISA_RVE
        j       L(ret)
#else
        ret
#endif

// Divide by zero.  0 / 0 is NaN, anything else is Inf (as NaN inputs already processed).
L(div_zero):
        bnez    a7, L(signed_inf)

// Return a canonical NaN.
L(return_nan):
        li      a1, 0x7FF80000
        j       L(clr_low_ret)

#undef a6
#undef a7
#undef t3

#else

// Algorithm derived from "An overview of floating-point support and math
// library on the Intel XScale architecture".

// Compute sign of quotient into t2{31}, all other bits zero.
        xor     t2, a1, a3
        srl     t2, t2, 31
        sll     t2, t2, 31

// Remove sign bit from lhs and rhs and shift significands and exponents into position.
        add     a5, a1, a1
        add     a6, a3, a3

// If the lhs/rhs has a special exponent indicating Inf or NaN, do that out of line.
        li      t0, 0xFFE00000
        bgeu    a5, t0, L(lhs_inf_or_nan)
        bgeu    a6, t0, L(rhs_inf_or_nan)

// Compute biased exponent of quotient into a5.
        srl     a5, a5, 21
        srl     a6, a6, 21
        beqz    a5, L(lhs_zero_or_subnormal)
        beqz    a6, L(inf)
        sub     a5, a5, a6
        add     a5, a5, 0x3FD

// Materialize hidden bit in lhs, moving significand to a1 and a3.
        BFOZ    a1, a1, 19, 0
        BFOZ    a3, a3, 19, 0
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a1, a1, 20
        bseti   a3, a3, 20
#else
        li      a4, 1<<20
        add     a1, a1, a4
        add     a3, a3, a4
#endif

//
// Compute approximation to reciprocal of divisor into a7.  We can
// do this either by using a bipartite table if we don't have a
// division instruction, or by using a division instruction.  The
// quotient is accurate to at least 12 bits in the bipartite case.
//

// It is generally faster to use the bipartite table than to employ
// the divider for the cores I have seen.  Your mileage may vary...
//

#if __SEGGER_RTL_CORE_HAS_DIV && (__SEGGER_RTL_OPTIMIZE < 0)

//
// Compute reciprocal using division, but ensure we do not overestimate
// the reciprocal such that it is too high.
//

        srl     t0, a3, 7               // 12 leading bits of dividend
        add     t0, t0, 1               // Always underestimate the reciprocal
        li      a7, 0x80000000          // Load 1.0
        divu    a7, a7, t0              // Compute 1/x to 18 bits
        sll     a7, a7, 5

#else

//
// No divider, compute reciprocal using bipartite table.
//

// At this point both a6 and a3 are free for use as temporaries

// Set a4 as pointer to 7-bits-in, 16-bits out reciprocal table TH.
// Immediately following is a 7-bit-in, 8-bit out bipartite table TL.

       .set     __SEGGER_RTL_ddiv_reciprocal_table_REQUIRED, 1
#if __SEGGER_RTL_WORKAROUND_CLANG_AS_SYMBOL_BUG
        la      a4, __SEGGER_RTL_ddiv_reciprocal_table
        addi    a4, a4, -0x100
#else
        la      a4, __SEGGER_RTL_ddiv_reciprocal_table-0x100
#endif

// Load table entry TL.
        srl     a7, a3, 13
        add     a6, a4, a7
        lbu     a6, 0x180(a6)

// Load table entry TH.
        LEAH    a7, a4, a7
        lhu     a7, (a7)

// Compute Br = TH*2^7 + 2^15 * (1 - 2^7 - f*2^(-7)) * TL where f is b14..b8 of divisor.
        srl     t0, a3, 5             // Isolate f (b14..b8 of divisor)
        andi    t0, t0, 0xFE          // Now f in bits t0{7:1} with t0{0} = 0.fe
        neg     t0, t0
        add     t0, t0, 0xFE          // Negate f
        mul     a6, a6, t0            // TL(a6) *= -f(t0) 
        sll     a7, a7, 7             // TH *= 2^7
        add     a7, a7, a6            // Compute Br(a7) = TL(a6) * -f(t0) + TH*2^7(a7)

#endif

// Compute R = -B * Br, where B is the divisor.
        mul     t1, a2, a7            // t0:t1 = Br * B[low] 
        mulhu   t0, a2, a7
        mul     t3, a3, a7            // a4:t0:t1 = Br * (B[high] * 2^32) + Br * B[low]
        mulhu   a4, a3, a7
        add     t0, t0, t3
        snez    t3, t1
        neg     t1, t1
        neg     t0, t0
        sub     t0, t0, t3

// A[r,0] = A * Br where A is the dividend.  A is held in a1:a0
        mul     a6, a0, a7            // a2:a6 = Br * A[low] 
        mulhu   a2, a0, a7
        mul     t3, a1, a7
        mulhu   a0, a1, a7
        add     a2, a2, t3            // A[r,0] = a0:a2:a6 = Br * (A[high] * 2^32) + Br * A[low]
        sltu    t4, a2, t3
        add     a0, a0, t4

// Iteration #1.
// A'[j+1] = (A'[j]low << 11) + R*d[j]
// Q[j+1]  = (Q[j] << 11) + d[j]
// Note that d[j] == floor(A[r,j] * 2^(11j) / 2^(64)) which can be 
// obtained directly from the leading bits of A[r,j].

// NOTE: All sequences are scheduled to try to provide the best performance
// possible on a generic RISC-V with a fast multiplier.

        mul     t5, a0, t1            // A'[j+1] a7:t4 += A'[j]low * Rlow
        sll     t4, a6, 11            // A'[j]low << 11
        mulhu   a7, a0, t1
        add     t4, t4, t5
        sltu    t5, t4, t5
        add     a7, a7, t5
        srl     a6, a6, 21            // Q[j+1] = Q[j] << 11 + d[j]
        sll     t5, a2, 11
        add     a6, a6, t5
        srl     a2, a2, 21
        add     a6, a6, a7
        sltu    t5, a6, a7
        add     a2, a2, t5
        mul     t5, a0, t0            // A'[j+1] a2:a6 += A'[j]low * Rhigh
        mulhu   t6, a0, t0
        add     a6, a6, t5
        add     a2, a2, t6
        sltu    t5, a6, t5
        add     a2, a2, t5

// Iteration #2.  Do a total of five iterations.  The general form
// is identical to above except that after each iteration the registers
// used to hold A'[j] and Q[j] change; we don't rearrange as that
// adds instructions and cycles.
        mul     a1, a2, t1
        sll     a3, t4, 11
        mulhu   a7, a2, t1
        add     a3, a3, a1
        sltu    a1, a3, a1
        add     a7, a7, a1
        srl     t4, t4, 21
        sll     a1, a6, 11
        add     t4, t4, a1
        srl     a6, a6, 21
        add     t4, t4, a7
        sltu    a7, t4, a7
        mul     t5, a2, t0
        add     a6, a6, a7
        mulhu   t6, a2, t0
        add     t4, t4, t5
        sltu    t5, t4, t5
        add     a6, a6, t5
        add     a6, a6, t6
        sll     a0, a0, 11
        add     a0, a0, a2

// Iteration #3.
        mul     a7, a6, t1
        sll     a2, a3, 11
        mulhu   a1, a6, t1
        add     a2, a2, a7
        sltu    a7, a2, a7
        add     a1, a1, a7
        srl     a3, a3, 21
        sll     t5, t4, 11
        add     a3, a3, t5
        srl     t4, t4, 21
        add     a3, a3, a1
        sltu    a7, a3, a1
        mul     t6, a6, t0
        add     t4, t4, a7
        mulhu   a1, a6, t0
        add     a3, a3, t6
        sltu    t6, a3, t6
        add     t4, t4, t6
        add     t4, t4, a1

// Iteration #4.
        mul     t5, t4, t1
        sll     a1, a2, 11
        mulhu   a7, t4, t1
        add     a1, a1, t5
        sltu    t5, a1, t5
        add     a7, a7, t5
        srl     a2, a2, 21
        sll     t5, a3, 11
        add     a2, a2, t5
        srl     a3, a3, 21
        add     a2, a2, a7
        sltu    a7, a2, a7
        mul     t5, t4, t0
        add     a3, a3, a7
        mulhu   a7, t4, t0
        add     a2, a2, t5
        sltu    t5, a2, t5
        add     a3, a3, a7
        add     a3, a3, t5
        sll     a6, a6, 11
        add     a6, a6, t4

// Iteration #5.
        mul     t5, a3, t1
        sll     t4, a1, 11
        mulhu   a7, a3, t1
        add     t4, t4, t5
        sltu    t5, t4, t5
        add     a7, a7, t5
        srl     a1, a1, 21
        sll     t5, a2, 11
        add     a1, a1, t5
        srl     a2, a2, 21
        add     a1, a1, a7
        sltu    a7, a1, a7
        mul     t5, a3, t0
        add     a2, a2, a7
        mulhu   a7, a3, t0
        add     a1, a1, t5
        sltu    t5, a1, t5
        add     a2, a2, t5
        add     a2, a2, a7

// If A'[5] > B*Br then Q[5] = Q[5] + 1
        sll     a7, a0, 1
        add     t0, t0, a1
        bltu    t0, a1, L(carry_out)
        add     t1, t1, t4
        sltu    t1, t1, t4
        add     t0, t0, t1
        bltu    t0, t1, L(carry_out)
        //
        sltu    t0, a4, a2
        j       L(no_inc)

L(carry_out):
        sltu    t0, a2, a4
        xor     t0, t0, 1

L(no_inc):
        sll     a4, a6, 11
        add     a0, a3, a4
        sltu    a4, a0, a4
        srl     a6, a6, 21
        add     a3, a7, a6
        add     a3, a3, a4
        add     a0, a0, t0
        sltu    t0, a0, t0
        add     a3, a3, t0

// Normalize.
        li      t0, 0x800000
        sltu    a6, a3, t0
        xor     a6, a6, 1
        add     a5, a5, a6
        add     a6, a6, 2

// Align significand a3:a0.
        neg     t1, a6
        sll     t4, a0, t1
        srl     a0, a0, a6
        sll     a2, a3, t1
        add     a0, a0, a2
        srl     a3, a3, a6

// Round, there can never be a tie.
        srl     t4, t4, 31
        add     a0, a0, t4
        sltu    t4, a0, t4
        add     a3, a3, t4

// Ensure normalized.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        li      t0, 0x200000
        sltu    a4, a3, t0
        xor     a4, a4, 1
        srl     a3, a3, a4
        add     a5, a5, a4
#else
        li      t0, 0x200000
        bltu    a3, t0, L(no_norm_1)
        srl     a3, a3, 1
        add     a5, a5, 1
L(no_norm_1):
#endif

// Check for exponent underflow.
        bltz    a5, L(zero_result)

// And exponent overflow.
        li      a7, 0x7FE
        bgeu    a5, a7, L(inf)

// Pack.
        sll     a5, a5, 20
        or      a1, t2, a5
        add     a1, a1, a3
        ret

// Know exponent is 0x7FF indicating Inf or NaN.  If significand is non-zero,
// it's a NaN and NaN/anything is NaN.
L(lhs_inf_or_nan):
        bne     a5, t0, L(nan)
        bnez    a0, L(nan)

// Inf/Inf is NaN, and Inf/NaN is NaN, so there's no need to distinguish
// whether the rhs is Inf or NaN as they both generate NaN anyway.
        bgeu    a6, t0, L(nan)

// Now we know that the lhs is Inf and the rhs is normal, zero, or subnormal.
// Inf/0 or Inf/Normal is Inf.
L(inf):
        li      a1, 0x7FF00000
        add     a1, a1, t2
L(zero_low_half_return):
        li      a0, 0
        ret

// Know the lhs is normal or zero.  If the rhs is NaN then normal/NaN or
// 0/NaN is NaN.  If the rhs is Inf then 0/Inf and Normal/Inf is a signed zero.
L(rhs_inf_or_nan):
        bnez    a2, L(nan)
        beq     a6, t0, L(zero_result)

// Generate a EABI NaN result.
L(nan):
        li      a1, 0x7FF80000
        li      a0, 0
        ret

// Know lhs is zero or subnormal and rhs is zero or subnormal (i.e. not Inf
// and not NaN).  If rhs is zero or subnormal then we have 0/0 which is NaN.
L(lhs_zero_or_subnormal):
        beqz    a6, L(nan)              // 0/0 or 0/subnormal is NaN.

// Generate a signed zero result, t2 is the sign.
L(zero_result):
        mv      a1, t2
        j       L(zero_low_half_return)

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Load useful constants.
        li      a4, 0x7ff
        li      t2, 0xFFFFFFFF
        sll     a5, t2, 63

// Compute product sign.
        xor     t3, a0, a1
        and     t3, t3, a5

// Extract exponents.  a2 is x exponent, a3 is y exponent.
        srl     a2, a0, 52
        and     a2, a2, a4
        srl     a3, a1, 52
        and     a3, a3, a4

// Handle special exponents out of line.
        beq     a2, a4, L(lhs_inf_nan)
        beq     a3, a4, L(rhs_inf_nan)
        beqz    a2, L(lhs_zero)
        beqz    a3, L(return_inf)       // non-zero finite / 0 is Inf

// Both are normals.  Calculate quotient exponent.
        sub     a2, a2, a3
        add     a2, a2, 0x3FD

// Isolate significands.
        sll     a0, a0, 12
        srl     a0, a0, 12
        sll     a1, a1, 12
        srl     a1, a1, 12

// Insert hidden bit into both significands.
        srl     a5, a5, 11
        or      a0, a0, a5
        or      a1, a1, a5

#if __SEGGER_RTL_CORE_HAS_DIV

// Execute 128/64 division.  Calculate first quotient digit.
        sll     a1, a1, 10

// vn1 = __SEGGER_RTL_U64_H(v);
        srl     a4, a1, 32              // vn1 into a4
        and     t1, a1, t2              // vn0 into t1

// __SEGGER_RTL_DIVMOD_U64(q1, rhat, u1, vn1);
        divu    a5, a0, a4              // a5 is q1
        mul     a6, a5, a4              // a6 is rhat
        sub     a6, a0, a6              // ...remainder after division

// do ... if (q1 < base && q1*vn0 <= base*rhat) { break; }
L(retry_digit_1):
        bgtu    a5, t2, L(adjust_digit_1)
        mul     t0, a5, t1              // t0 := q1*vn0
        sll     a3, a6, 32              // t2 := rhat * base (as base is 2^32)
        bleu    t0, a3, L(have_digit_1)

// q1   -= 1;
// rhat += vn1;
L(adjust_digit_1):
        addi    a5, a5, -1
        add     a6, a6, a4

// ...while (rhat < base);
        bleu    a6, t2, L(retry_digit_1)

// un21 = u1*base - q1*v;
L(have_digit_1):
        sll     a0, a0, 32              // u1*base
        mul     t0, a5, a1              // q1*v
        sub     a0, a0, t0              // u1*base - q1*v

// __SEGGER_RTL_DIVMOD_U64(q0, rhat, un21, vn1);
        divu    a1, a0, a4              // a1 is q0
        mul     a6, a1, a4              // a6 is rhat
        sub     a6, a0, a6              // ...remainder after division

// do ... if (q0 < base && q0*vn0 <= base*rhat) { break; }
L(retry_digit_0):
        bgtu    a1, t2, L(adjust_digit_0)
        mul     t0, a1, t1              // t0 := q1*vn0
        sll     a3, a6, 32              // t2 := rhat * base (as base is 2^32)
        bleu    t0, a3, L(have_digit_0)

// q0   -= 1;
// rhat += vn1;
L(adjust_digit_0):
        addi    a1, a1, -1
        add     a6, a6, a4

// ...while (rhat < base);
        bleu    a6, t2, L(retry_digit_0)

// Combine.
L(have_digit_0):
        sll     a5, a5, 32
        add     a0, a1, a5

#else

// Zero quotient.
        li      a4, 0

#if __SEGGER_RTL_OPTIMIZE < 2

// Generate 55-bit quotient
        li      a5, 55

// Shift-subtract loop.
L(shift_subtract):
        sll     a4, a4, 1
        bltu    a0, a1, L(cant_subtract)
        sub     a0, a0, a1
        add     a4, a4, 1
L(cant_subtract):
        sll     a0, a0, 1
        add     a5, a5, -1
        bnez    a5, L(shift_subtract)

#else

// Open-coded shift-subtracts.
       .rept    55
        sll     a4, a4, 1
        bltu    a0, a1, 1f
        sub     a0, a0, a1
        add     a4, a4, 1
1:      sll     a0, a0, 1
       .endr

#endif

// Move quotient to return register.
        mv      a0, a4

#endif

// Normalize.
        sll     a3, a0, 9
        bgez    a3, L(round)
        srl     a0, a0, 1
        addi    a2, a2, 1

// Round.
L(round):
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        andi    a1, a0, 1
        add     a0, a0, a1
        srl     a3, a0, 54              // a3 is 1/0 depending upon normalization
        srl     a0, a0, a3              // Normalize or not based on a3
        add     a2, a2, a3
#else
        andi    a1, a0, 1
        beqz    a1, L(overflow_underflow)
        addi    a0, a0, 1
        sll     a3, a0, 9
        bgez    a3, L(overflow_underflow)
        srl     a0, a0, 1
        addi    a2, a2, 1
#endif

// Check for exponent overflow and underflow.
L(overflow_underflow):
        bltz    a2, L(return_zero)
        li      a3, 0x7FE
        bgeu    a2, a3, L(return_inf)   // Overflow

// Combine sign, exponent, significand.
        srl     a0, a0, 1
        add     a0, a0, t3
        sll     a2, a2, 52
        add     a0, a0, a2
        ret

// {Inf,NaN} / any
L(lhs_inf_nan):
        sll     a4, a4, 53              // Generate Inf*2, 0xFFE00000'00000000
        sll     a0, a0, 1
        bgtu    a0, a4, L(return_nan)   // NaN / any

// Inf/{0,normal} is Inf, Inf/{Inf,NaN} is NaN
        sll     a1, a1, 1
        bgeu    a1, a4, L(return_nan)   // Inf/{Inf,NaN} is NaN

// Return signed Inf.
L(return_inf):
        li      a0, 0x7FF
        sll     a0, a0, 52
        add     a0, a0, t3
        ret

// {0,normal} / {Inf,NaN}
L(rhs_inf_nan):
        sll     a4, a4, 53              // Generate Inf*2, 0xFFE00000'00000000
        sll     a1, a1, 1
        bgtu    a1, a4, L(return_nan)   // Dividing by NaN is NaN

// 0/{0,normal}, so distinguish 0/0 from 0/Normal
L(lhs_zero):
        beqz    a3, L(return_nan)

// Underflow or dividing by Inf means just a signed zero,
L(return_zero):
        mv      a0, t3
        ret

// Return a canonical NaN.
L(return_nan):
        li      a0, 0xFFF
        sll     a0, a0, 51
        ret

#else

#error Bad configuration

#endif

END_FUNC __divdf3

/*********************************************************************
*
*       __ltsf2()
*
*  Function description
*    Less than, float.
*
*  Parameters
*    a0 - Left-hand operand.
*    a1 - Right-hand operand.
*
*  Return value
*    a0 - Return < 0 if both operands are non-NaN and a0 < a1
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__ltsf2_##label

GLOBAL_FUNC __ltsf2

#if __SEGGER_RTL_FP_ABI >= 1

        flt.s   a0, fa0, fa1
        neg     a0, a0
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFF000000
        sll     a3, a0, 1
        bgtu    a3, a2, L(zero)         // Comparison with NaN is invalid.
        sll     a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(zero)         // Comparison with NaN is invalid.

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sll     a3, a2, 1
        beqz    a3, L(zero)             // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sltu    a0, a0, a1
        neg     a0, a0
        ret

// Either negative, generate truth.
L(negative):
        sltu    a0, a1, a0
        neg     a0, a0
        ret

L(zero):
        li      a0, 0
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFFFFFFFFFF000000
        sllw    a3, a0, 1
        bgtu    a3, a2, L(zero)         // Comparison with NaN is invalid.
        sllw    a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(zero)         // Comparison with NaN is invalid.

// Incoming 32-bit arguments have undefined high 32 bits, so extend to
// 64 bits as the compares below use require 64 valid bits.
        sext.w  a0, a0
        sext.w  a1, a1

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sllw    a3, a2, 1
        beqz    a3, L(zero)             // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sltu    a0, a0, a1
        neg     a0, a0
        ret

// Either negative, generate truth.
L(negative):
        sltu    a0, a1, a0
        neg     a0, a0
        ret

L(zero):
        li      a0, 0
        ret

#else

#error Bad configuration

#endif

END_FUNC __ltsf2

/*********************************************************************
*
*       __ltdf2()
*
*  Function description
*    Less than, double.
*
*  Parameters
*    a1:a0 / a0 - Left-hand operand.
*    a3:a2 / a1 - Right-hand operand.
*
*  Return value
*    a0 - Return < 0 if both operands are non-NaN and a1:a0 < a3:a2
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__ltdf2_##label

GLOBAL_FUNC __ltdf2

#if __SEGGER_RTL_FP_ABI >= 2

        flt.d   a0, fa0, fa1
        neg     a0, a0
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a5, 0xFFE00000
        sll     a4, a1, 1               // cast out sign bits
        bgtu    a4, a5, L(not_less)     // Is lhs NaN?
        sll     a4, a3, 1
        bgtu    a4, a5, L(not_less)     // Is rhs NaN?

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        or      a4, a1, a3
        sll     a5, a4, 1
        or      a5, a5, a0
        or      a5, a5, a2              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beqz    a5, L(not_less)
        bltz    a4, L(negative)

// We now have non-zero operands.
        bltu    a1, a3, L(less)
        bne     a1, a3, L(not_less)
        bltu    a0, a2, L(less)
L(not_less):
        li      a0, 0
        ret

L(negative):
        bltu    a3, a1, L(less)
        bne     a3, a1, L(not_less)
        bgeu    a2, a0, L(not_less)
L(less):
        li      a0, -1
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller
        sll     a3, a0, 1
        bgtu    a3, a2, L(zero)         // Comparison with NaN is invalid.
        sll     a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(zero)         // Comparison with NaN is invalid.

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sll     a3, a2, 1
        beqz    a3, L(zero)             // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sltu    a0, a0, a1
        neg     a0, a0
        ret

// Either negative, generate truth.
L(negative):
        sltu    a0, a1, a0
        neg     a0, a0
        ret

L(zero):
        li      a0, 0
        ret

#else

#error Bad configuration

#endif

END_FUNC __ltdf2

/*********************************************************************
*
*       __lesf2()
*
*  Function description
*    Less than or equal, float.
*
*  Parameters
*    a0 - Left-hand operand.
*    a1 - Right-hand operand.
*
*  Return value
*    a0 - Return <= 0 if both operands are non-NaN and a0 <= a1
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__lesf2_##label

GLOBAL_FUNC __lesf2

#if __SEGGER_RTL_FP_ABI >= 1

        fle.s   a0, fa0, fa1
        neg     a0, a0
        add     a0, a0, 1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFF000000
        sll     a3, a0, 1
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.
        sll     a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sll     a3, a2, 1
        beqz    a3, L(zero)             // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sgtu    a0, a0, a1              // a <= b = !(a > b)
        ret

// Either negative, generate truth.
L(negative):
        sgtu    a0, a1, a0
        ret

L(nan):
        li      a0, 1
        ret

L(zero):
        li      a0, 0
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFFFFFFFFFF000000
        sllw    a3, a0, 1
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.
        sllw    a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.

// Incoming 32-bit arguments have undefined high 32 bits, so extend to
// 64 bits as the compares below use require 64 valid bits.
        sext.w  a0, a0
        sext.w  a1, a1

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sllw    a3, a2, 1
        beqz    a3, L(zero)             // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sgtu    a0, a0, a1              // a <= b = !(a > b)
        ret

// Either negative, generate truth.
L(negative):
        sgtu    a0, a1, a0
        ret

L(nan):
        li      a0, 1
        ret

L(zero):
        li      a0, 0
        ret

#else

#error Bad configuration

#endif

END_FUNC __lesf2

/*********************************************************************
*
*       __ledf2()
*
*  Function description
*    Less than or equal, double.
*
*  Parameters
*    a1:a0 / a0 - Left-hand operand.
*    a3:a2 / a1 - Right-hand operand.
*
*  Return value
*    a0 - Return <= 0 if both operands are non-NaN and a1:a0 <= a3:a2
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__ledf2_##label

GLOBAL_FUNC __ledf2

#if __SEGGER_RTL_FP_ABI >= 2

        fle.d   a0, fa0, fa1
        neg     a0, a0
        add     a0, a0, 1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a5, 0xFFE00000
        sll     a4, a1, 1               // cast out sign bits
        bgtu    a4, a5, L(not_less_equal)     // Is lhs NaN?
        sll     a4, a3, 1
        bgtu    a4, a5, L(not_less_equal)     // Is rhs NaN?

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        or      a4, a1, a3
        sll     a5, a4, 1
        or      a5, a5, a0
        or      a5, a5, a2              // Z iff x == (+0.0 or -0.0) and y == (+0.0 or -0.0)
        beqz    a5, L(less_equal)
        bltz    a4, L(negative)

// We now have non-zero operands.
        bltu    a1, a3, L(less_equal)
        bne     a1, a3, L(not_less_equal)
        bgeu    a2, a0, L(less_equal)
L(not_less_equal):
        li      a0, 1
        ret

L(negative):
        bltu    a3, a1, L(less_equal)
        bne     a3, a1, L(not_less_equal)
        bltu    a0, a2, L(not_less_equal)
L(less_equal):
        li      a0, -1
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller
        sll     a3, a0, 1
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.
        sll     a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        or      a2, a0, a1
        sll     a3, a2, 1
        beqz    a3, L(less_equal)
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sgtu    a0, a0, a1              // a <= b = !(a > b)
        ret

// Either negative, generate truth.
L(negative):
        sgtu    a0, a1, a0
        ret

L(nan):
        li      a0, 1
        ret

L(less_equal):
        li      a0, -1
        ret

#else

#error Bad configuration

#endif

END_FUNC __ledf2

/*********************************************************************
*
*       __gtsf2()
*
*  Function description
*    Greater than, float.
*
*  Parameters
*    a0 - Left-hand operand.
*    a1 - Right-hand operand.
*
*  Return value
*    a0 - Return > 0 if both operands are non-NaN and a0 > a1
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__gtsf2_##label

GLOBAL_FUNC __gtsf2

#if __SEGGER_RTL_FP_ABI >= 1

        fgt.s   a0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFF000000
        sll     a3, a0, 1
        bgtu    a3, a2, L(zero)         // Comparison with NaN is invalid.
        sll     a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(zero)         // Comparison with NaN is invalid.

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sll     a3, a2, 1
        beqz    a3, L(zero)             // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sgtu    a0, a0, a1
        ret

// Either negative, generate truth.
L(negative):
        sgtu    a0, a1, a0
        ret

L(zero):
        li      a0, 0
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFFFFFFFFFF000000
        sllw    a3, a0, 1
        bgtu    a3, a2, L(nan_zero)     // Comparison with NaN is invalid.
        sllw    a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(nan_zero)     // Comparison with NaN is invalid.

// Incoming 32-bit arguments have undefined high 32 bits, so extend to
// 64 bits as the compares below use require 64 valid bits.
        sext.w  a0, a0
        sext.w  a1, a1

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sllw    a3, a2, 1
        beqz    a3, L(nan_zero)         // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sgtu    a0, a0, a1              // a <= b = !(a > b)
        ret

// Either negative, generate truth.
L(negative):
        sgtu    a0, a1, a0
        ret

L(nan_zero):
        li      a0, 0
        ret

#else

#error Bad configuration

#endif

END_FUNC __gtsf2

/*********************************************************************
*
*       __gtdf2()
*
*  Function description
*    Greater than, double.
*
*  Parameters
*    a1:a0 / a0 - Left-hand operand.
*    a3:a2 / a1 - Right-hand operand.
*
*  Return value
*    a0 - Return > 0 if both operands are non-NaN and a1:a0 > a3:a2
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__gtdf2_##label

GLOBAL_FUNC __gtdf2

#if __SEGGER_RTL_FP_ABI >= 2

        fgt.d   a0, fa0, fa1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a5, 0xFFE00000
        sll     a4, a1, 1               // cast out sign bits
        bgtu    a4, a5, L(not_greater)  // Is lhs NaN?
        sll     a4, a3, 1
        bgtu    a4, a5, L(not_greater)  // Is rhs NaN?

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        or      a4, a1, a3
        sll     a5, a4, 1
        or      a5, a5, a0
        or      a5, a5, a2              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beqz    a5, L(not_greater)
        bltz    a4, L(negative)

// We now have non-zero operands.
        bgtu    a1, a3, L(greater)
        bne     a1, a3, L(not_greater)
        bgtu    a0, a2, L(greater)
L(not_greater):
        li      a0, 0
        ret

L(negative):
        bgtu    a3, a1, L(greater)
        bne     a3, a1, L(not_greater)
        bleu    a2, a0, L(not_greater)
L(greater):
        li      a0, 1
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller
        sll     a3, a0, 1
        bgtu    a3, a2, L(not_greater)  // Comparison with NaN is invalid.
        sll     a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(not_greater)  // Comparison with NaN is invalid.

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        or      a2, a0, a1
        sll     a3, a2, 1
        beqz    a3, L(not_greater)
        bltz    a2, L(negative)

// We now have non-zero operands.
        sgtu    a0, a0, a1
        ret

L(negative):
        sgtu    a0, a1, a0
        ret

L(not_greater):
        li      a0, 0
        ret

#else

#error Bad configuration

#endif

END_FUNC __gtdf2

/*********************************************************************
*
*       __gesf2()
*
*  Function description
*    Greater than or equal, float.
*
*  Parameters
*    a0 - Left-hand operand.
*    a1 - Right-hand operand.
*
*  Return value
*    a0 - Return >= 0 if both operands are non-NaN and a0 >= a1
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__gesf2_##label

GLOBAL_FUNC __gesf2

#if __SEGGER_RTL_FP_ABI >= 2

        fge.s   a0, fa0, fa1
        addi    a0, a0, -1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFF000000
        sll     a3, a0, 1
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.
        sll     a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sll     a3, a2, 1
        beqz    a3, L(zero)             // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sltu    a0, a0, a1
        neg     a0, a0
        ret

// Either negative, generate truth.
L(negative):
        sltu    a0, a1, a0
        neg     a0, a0
        ret

L(nan):
        li      a0, -1
        ret

L(zero):
        li      a0, 0
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFFFFFFFFFF000000
        sllw    a3, a0, 1
        bgtu    a3, a2, L(not_greater_eq) // Comparison with NaN is invalid.
        sllw    a3, a1, 1                 // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(not_greater_eq) // Comparison with NaN is invalid.

// Incoming 32-bit arguments have undefined high 32 bits, so extend to
// 64 bits as the compares below use require 64 valid bits.
        sext.w  a0, a0
        sext.w  a1, a1

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sllw    a3, a2, 1
        beqz    a3, L(greater_eq)       // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sltu    a0, a0, a1
        neg     a0, a0
        ret

// Either negative, generate truth.
L(negative):
        sltu    a0, a1, a0
        neg     a0, a0
        ret

L(not_greater_eq):
        li      a0, -1
        ret

L(greater_eq):
        li      a0, 1
        ret

#else

#error Bad configuration

#endif

END_FUNC __gesf2

/*********************************************************************
*
*       __gedf2()
*
*  Function description
*    Greater equal, double.
*
*  Parameters
*    a1:a0 / a0 - Left-hand operand.
*    a3:a2 / a1 - Right-hand operand.
*
*  Return value
*    a0 - Return >= 0 if both operands are non-NaN and a1:a0 >= a3:a2
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__gedf2_##label

GLOBAL_FUNC __gedf2

#if __SEGGER_RTL_FP_ABI >= 2

        fge.d   a0, fa0, fa1
        addi    a0, a0, -1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a5, 0xFFE00000
        sll     a4, a1, 1               // cast out sign bits
        bgtu    a4, a5, L(not_greater_equal)     // Is lhs NaN?
        sll     a4, a3, 1
        bgtu    a4, a5, L(not_greater_equal)     // Is rhs NaN?

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        or      a4, a1, a3
        sll     a5, a4, 1
        or      a5, a5, a0
        or      a5, a5, a2              // Z iff x == (+0.0 or -0.0) and y == (+0.0 or -0.0)
        beqz    a5, L(greater_equal)
        bltz    a4, L(negative)

// We now have non-zero operands.
        bgtu    a1, a3, L(greater_equal)
        bne     a1, a3, L(not_greater_equal)
        bgeu    a0, a2, L(greater_equal)
L(not_greater_equal):
        li      a0, -1
        ret

L(negative):
        bgtu    a3, a1, L(greater_equal)
        bne     a3, a1, L(not_greater_equal)
        bltu    a2, a0, L(not_greater_equal)
L(greater_equal):
        li      a0, 1
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Need to see if any operand is NaN which is an invalid comparison.
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller
        sll     a3, a0, 1
        bgtu    a3, a2, L(not_greater_eq) // Comparison with NaN is invalid.
        sll     a3, a1, 1                 // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(not_greater_eq) // Comparison with NaN is invalid.

// OK, now get down to business of the compare.  We use the fact that all floating
// point values are monotonic when viewed as 32-bit integers so all we need to do is
// a straight integer compare based on whether both are positive or either are negative.
        or      a2, a0, a1
        sll     a3, a2, 1
        beqz    a3, L(greater_eq)       // LHS and RHS are both +-0.
        bltz    a2, L(negative)

// Both positive, generate truth value.
        sltu    a0, a0, a1
        neg     a0, a0
        ret

// Either negative, generate truth.
L(negative):
        sltu    a0, a1, a0
        neg     a0, a0
        ret

L(not_greater_eq):
        li      a0, -1
        ret

L(greater_eq):
        li      a0, 1
        ret

#else

#error Bad configuration

#endif

END_FUNC __gedf2

/*********************************************************************
*
*       __eqsf2()
*
*  Function description
*    Equal, float.
*
*  Prototype
*    int __eqsf2(float x, float y);
*
*  Parameters
*    a0 - Left-hand operand.
*    a1 - Right-hand operand.
*
*  Return value
*    a0 - Return == 0 if both operands are non-NaN and a0 == a1
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__eqsf2_##label

GLOBAL_FUNC __eqsf2
ALIAS_LABEL __nesf2

#if __SEGGER_RTL_FP_ABI >= 1

        feq.s   a0, fa0, fa1
        xor     a0, a0, 1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFF000000
        sll     a3, a0, 1
        bgtu    a3, a2, L(one)          // Comparison with NaN is invalid.
        sll     a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(one)          // Comparison with NaN is invalid.

// OK, now get down to business of the compare.
        or      a2, a0, a1
        sll     a2, a2, 1
        beqz    a2, L(zero)             // LHS and RHS are both +-0.

// Generate truth value.
        sub     a0, a0, a1
        snez    a0, a0
        ret

// Return ordered and "equal" status (both operands are zero).
L(zero):
        li      a0, 0
        ret

// Return unordered or "unequal" status.
L(one):
        li      a0, 1
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Need to see if any operand is NaN which is an invalid comparison.
// isnan(x) where x is a 32-bit unsigned is (x & 0x7FFFFFFF) > 0x7F800000.
// However, if we ditch the sign bit using a shift left, then the above can
// be rewritten (x << 1) > 0xFF000000.
        li      a2, 0xFFFFFFFFFF000000
        sllw    a3, a0, 1
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.
        sllw    a3, a1, 1               // If lhs isn't NaN, see if rhs is NaN
        bgtu    a3, a2, L(nan)          // Comparison with NaN is invalid.

// OK, now get down to business of the compare.
        or      a2, a0, a1
        sllw    a2, a2, 1
        beqz    a2, L(zero)             // LHS and RHS are both +-0.

// Generate truth value.
        subw    a0, a0, a1
        snez    a0, a0
        ret

// Return ordered and "equal" status (both operands are zero).
L(zero):
        li      a0, 0
        ret

// Return unordered or "unequal" status.
L(nan):
        li      a0, 1
        ret

#else

#error Bad configuration

#endif

SET_SIZE __nesf2
END_FUNC __eqsf2

/*********************************************************************
*
*       __eqdf2()
*
*  Function description
*    Equal, double.
*
*  Prototype
*    int __eqsd2(double x, double y);
*
*  Parameters
*    a1:a0 / a0 - Left-hand operand.
*    a3:a2 / a1 - Right-hand operand.
*
*  Return value
*    a0 - Return == 0 if both operands are non-NaN and a1:a0 == a3:a2
*         (GNU three-way boolean).
*/

#undef L
#define L(label) .L__eqdf2_##label

GLOBAL_FUNC __eqdf2
ALIAS_LABEL __nedf2

#if __SEGGER_RTL_FP_ABI >= 2

        feq.d   a0, fa0, fa1
        xor     a0, a0, 1
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a5, 0xFFE00000
        sll     a4, a1, 1               // cast out sign bits
        bgtu    a4, a5, L(not_equal)    // Is lhs NaN?
        sll     a4, a3, 1
        bgtu    a4, a5, L(not_equal)    // Is rhs NaN?

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        or      a4, a1, a3
        sll     a4, a4, 1
        or      a4, a4, a0
        or      a4, a4, a2              // Z iff x == (+0.0 or -0.0) or (y == +0.0 or -0.0)
        beqz    a4, L(equal)

// We now have non-zero operands.
        xor     a0, a0, a2
        xor     a1, a1, a3
        or      a0, a0, a1
        snez    a0, a0
        ret

L(equal):
        li      a0, 0
        ret
L(not_equal):
        li      a0, 1
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// First check for Inf and NaNs.  If we have Inf or NaN then handle the non-common case
// out of line.
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller
        sll     a3, a0, 1               // cast out sign bits
        bgtu    a3, a2, L(not_equal)    // Is lhs NaN?
        sll     a3, a1, 1
        bgtu    a3, a2, L(not_equal)    // Is rhs NaN?

// Ok, now get down to business of the compare.  A wrinkle is that +0 and -0
// compare equal, so deal with that now.
        or      a3, a0, a1
        sll     a3, a3, 1
        beqz    a3, L(equal)

// We now have non-zero operands.
        sub     a0, a0, a1
        snez    a0, a0
        ret

L(equal):
        li      a0, 0
        ret
L(not_equal):
        li      a0, 1
        ret

#else

#error Bad configuration

#endif

SET_SIZE __nedf2
END_FUNC __eqdf2

/*********************************************************************
*
*       __fixsfsi()
*
*  Function description
*    Convert float to int.
*
*  Prototype
*    int __fixsfsi(float x);
*
*  Parameters
*    a0 - x - Floating value to convert.
*
*  Return value
*    a0 - Integerized value.
*/

#undef L
#define L(label) .L__fixsfsi_##label

GLOBAL_FUNC __fixsfsi

#if __SEGGER_RTL_FP_ABI >= 1

        fcvt.w.s a0, fa0, rtz
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Save sign of input.
        sra     a5, a0, 31

// Extract biased exponent to a1, magnitude to a2.
        sll     a2, a0, 1
        srl     a1, a2, 24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -127
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Pre-load constant.
        li      a3, 0x80000000

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a4, 31
        sub     a4, a4, a1
        blez    a4, L(overflow_result)

// Shift off exponent bits.
        sll     a0, a0, 8

// Set hidden bit.
        or      a0, a0, a3

// Integerize with rounding towards zero.
        srl     a0, a0, a4

// Apply sign of input to two's complement output.
        xor     a0, a0, a5
        sub     a0, a0, a5
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result based on sign.
// NaNs are converted to INT_MAX irrespective.
L(overflow_result):
        not     a4, a3
        li      a1, 0xFF000000
        bleu    a2, a1, L(not_nan)      // NaN if > 0xFF000000
        and     a0, a0, a4              // NaN, clear sign bit to truncate to max +ve integer
L(not_nan):
        and     a0, a0, a3              // Isolate sign
        bltz    a0, L(done)
        mv      a0, a4                  // If positive, saturate to +ve maximum
L(done):
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Save sign.
        sraw    a3, a0, 31

// Move off sign.  
        sll     a1, a0, 32+1

// Extract biased exponent to a1.
        srl     a1, a1, 32+24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x7F
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x1F
        sub     a1, a2, a1
        blez    a1, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a0, a0, 32+8

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 63
#else
        sll     a2, a2, 63              // li a2, 0x8000000000000000...but faster
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a1
        srl     a0, a0, 32

// Apply sign of input to integerized output.
        xor     a0, a0, a3
        sub     a0, a0, a3
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet or signed or not.
L(overflow_result):
        li      a2, 0xFFFFFFFFFF000000  // li a2, 0xFFE0000000000000...
        sllw    a3, a0, 1
        bleu    a3, a2, L(not_nan)

// Clear sign bit of NaN.
        li      a0, 0

// Isolate sign bit and return INT_MAX or INT_MIN.
L(not_nan):
        srlw    a0, a0, 31              // a0 = 0x00000000'00000000 or 0xFFFFFFFF'FFFFFFFF
        sllw    a0, a0, 31              // a0 = 0x00000000'00000000 or 0xFFFFFFFF'80000000
        bnez    a0, L(done)
        li      a0, -1
        srl     a0, a0, 33              // a0 = 0x00000000'7FFFFFFF
L(done):
        ret

#else

#error Bad configuration

#endif

END_FUNC __fixsfsi

/*********************************************************************
*
*       __fixdfsi()
*
*  Function description
*    Convert double to int.
*
*  Parameters
*    a1:a0 / a0 - Floating value to convert :: soft and single ABI.
*    fa0        - Floating value to convert :: double ABI.
*
*  Return value
*    a0 - Integerized value.
*/

#undef L
#define L(label) .L__fixdfsi_##label

GLOBAL_FUNC __fixdfsi

#if __SEGGER_RTL_FP_ABI >= 2

        fcvt.w.d a0, fa0, rtz
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Extract sign to a3.
        sra     a3, a1, 31

// Extract biased exponent to a2.
        BFOZ    a2, a1, 30, 20

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a2, a2, -0x3FF
        bltz    a2, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a4, 31
        sub     a4, a4, a2
        blez    a4, L(overflow_result)

// Shift off exponent bits and recombine significand.
        srl     a0, a0, 21
        sll     a1, a1, 11
        or      a0, a0, a1

// Set hidden bit.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 31
#else
        li      a1, 0x80000000
        or      a0, a0, a1
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a4

// Apply sign.
        xor     a0, a0, a3
        sub     a0, a0, a3
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet.
L(overflow_result):
        snez    a0, a0                  // Fold in significand bits from low-order to
        or      a1, a1, a0              // ensure that a NaN is correctly detected (if high significand bits are zero)

// If NaN input, sign is always considered positive.
        sll     a1, a1, 1
        li      a0, 0xFFE00000
        bleu    a1, a0, L(not_nan)
        li      a3, 0

// Saturate to INT_MAX or INT_MIN depending upon sign.
L(not_nan):
        li      a0, 0x7FFFFFFF
        xor     a0, a0, a3
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Save sign.
        sra     a3, a0, 63

// Move off sign.  
        sll     a1, a0, 1

// Extract biased exponent to a1.
        srl     a1, a1, 53

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x3FF
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x1F
        sub     a1, a2, a1
        blez    a1, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a0, a0, 11

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 63
#else
        sll     a2, a2, 63              // li a2, 0x8000000000000000...but faster
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a1
        srl     a0, a0, 32

// Apply sign of input to integerized output.
        xor     a0, a0, a3
        sub     a0, a0, a3
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet or signed or not.
L(overflow_result):
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller
        sll     a3, a0, 1
        bleu    a3, a2, L(not_nan)

// Clear sign bit of NaN.
        li      a0, 0

// Isolate sign bit and return INT_MAX or INT_MIN.
L(not_nan):
        srl     a0, a0, 63
        sllw    a0, a0, 31
        bnez    a0, L(done)
        addi    a0, a0, -1
        srlw    a0, a0, 1
L(done):
        ret

#else

#error Bad configuration

#endif

END_FUNC __fixdfsi

/*********************************************************************
*
*       __fixsfdi()
*
*  Function description
*    Convert single to long long.
*
*  Parameters
*    a0  - Float value to convert :: soft ABI.
*    fa0 - Float value to convert :: single and double ABI.
*
*  Return value
*    a1:a0 / a0 - Integerized value.
*/

#undef L
#define L(label) .L__fixsfdi_##label

GLOBAL_FUNC __fixsfdi

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 1
        fmv.x.w a0, fa0
#endif

// Extract biased exponent to a2, magnitude to a3.
        sll     a4, a0, 1
        srl     a2, a4, 24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a2, a2, -127
        bltz    a2, L(zero_result)      // 2^(-1) and below are always zero.

// Save sign of operand.
        mv      t0, a0

// Break shifting into two sets: one shifts by less than 32 places, the other by more.
        li      t1, 31
        bgt     a2, t1, L(long_shift)

// Value is less than 2^32 so shift is by fewer than 32 bits.
// Compute number of bits to shift significand by to align correctly.
        sub     a2, t1, a2

// Shift off exponent bits.
        sll     a0, a0, 8

// Set hidden bit.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 31
#else
        li      a3, 0x80000000
        or      a0, a0, a3
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a2

// Set the high 32 bits of the result correctly.
        li      a1, 0                   // +ve, zero extend

// Conditionally negate.
        j       L(optional_negate)

// Value is >= 2^32 so shift is by 32 bits or more.
// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
L(long_shift):
        li      a3, 63
        sub     a3, a3, a2
        blez    a3, L(overflow_result)

// Shift off exponent bits.
        sll     a0, a0, 8

// Set hidden bit.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 31
#else
        li      a2, 0x80000000
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        neg     a2, a3
        srl     a1, a0, a3
        sll     a0, a0, a2

// Return as operand was positive.
L(optional_negate):
        bgez    t0, L(done)
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        li      a3, 0
        li      a2, 0
        sub64   a0, a2, a0
#else
        snez    a2, a0
        neg     a0, a0
        neg     a1, a1
        sub     a1, a1, a2
#endif
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        li      a1, 0
        ret

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet.
L(overflow_result):
        li      a2, 0xFF000000
        bleu    a4, a2, L(not_nan)
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bclri   a0, a0, 31
#else
        BFOZ    a0, a0, 30, 0           // If NaN, clear sign bit to truncate to max +ve 64-bit integer
#endif
L(not_nan):
        li      a3, 0x80000000
        and     a1, a0, a3              // Isolate sign and set maximum -ve 64-bit integer
        li      a0, 0
        bltz    a1, L(done)
        not     a1, a3                  // If positive, saturate to +ve maximum
        li      a0, -1
L(done):
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

// ISA can do this directly.
        fcvt.l.s a0, fa0, rtz
        ret

#else

#if __SEGGER_RTL_FP_ABI >= 1
        fmv.x.w a0, fa0
#endif

// Save sign.
        sraw    a3, a0, 31

// Move off sign.  
        sll     a1, a0, 32+1

// Extract biased exponent to a1.
        srl     a1, a1, 32+24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x7F
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x3F
        sub     a1, a2, a1
        blez    a1, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a0, a0, 32+8

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 63
#else
        sll     a2, a2, 63              // li a2, 0x8000000000000000...but faster
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a1

// Apply sign of input to integerized output.
        xor     a0, a0, a3
        sub     a0, a0, a3
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet or signed or not.
L(overflow_result):
        li      a2, 0xFFFFFFFFFF000000  // li a2, 0xFFE0000000000000...
        sllw    a3, a0, 1
        bleu    a3, a2, L(not_nan)

// Clear sign bit of NaN.
        li      a0, 0

// Isolate sign bit and return INT_MAX or INT_MIN.
L(not_nan):
        srlw    a0, a0, 31
        sll     a0, a0, 63
        bnez    a0, L(done)
        addi    a0, a0, -1
        srl     a0, a0, 1
L(done):
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __fixsfdi

/*********************************************************************
*
*       __fixdfdi()
*
*  Function description
*    Convert double to long.
*
*  Parameters
*    a1:a0 / a0 - Double value to convert :: soft and single ABI.
*    fa0        - Double value to convert        :: double ABI.
*
*  Return value
*    a1:a0 / a0 - Integerized value.
*/

#undef L
#define L(label) .L__fixdfdi_##label

GLOBAL_FUNC __fixdfdi

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 2

// If value is positive, use unsigned conversion.
        fcvt.d.w fa1, zero
        flt.d    a0, fa0, fa1
        bnez     a0, L(negative)
        tail     __fixunsdfdi

// Negative, need to be a bit more careful.
// This aligns with RV64D what FCVT.LU.D will do.

// Work wth magnitude.
L(negative):
        fabs.d    fa0, fa0

// Load constant 2^64.
       .set       __SEGGER_RTL_2pow64_REQUIRED, 1
        lui       a2,  %hi(__SEGGER_RTL_2pow64)
        fld       fa1, %lo(__SEGGER_RTL_2pow64)(a2)

// If NaN, return max.
        feq.d     a0, fa0, fa0
        beqz      a0, L(max)

// If greater than or equal to 2^64, return MAX_UINT.
        fge.d     a0, fa0, fa1
        bnez      a0, L(max)

// Load constants 2^32 and 2^-32.
       .set       __SEGGER_RTL_2pow32_REQUIRED, 1
       .set       __SEGGER_RTL_2powNeg32_REQUIRED, 1
        lui       a2,  %hi(__SEGGER_RTL_2pow32)
        fld       fa1, %lo(__SEGGER_RTL_2pow32)(a2)
        lui       a2,  %hi(__SEGGER_RTL_2powNeg32)
        fld       fa2, %lo(__SEGGER_RTL_2powNeg32)(a2)

// Get upper 32 bits of integerized value by scaling input by 2^-32.
        fmul.d    fa2, fa2, fa0
        fcvt.wu.d a1, fa2, rtz

// Get lower 32 bits of integerized value by subtracting integerized
// high part from argument.
        fcvt.d.wu fa2, a1
        fnmsub.d  fa2, fa2, fa1, fa0
        fcvt.wu.d a0, fa2, rtz

// Result is negative, so negate outging value.
        neg       a0, a0
        neg       a1, a1
        seqz      a2, a0
        sub       a0, a0, a2
        ret

// Return MAX_INT.
L(max):
        li        a0, -1
        srl       a1, a0, 1
        ret

#else

// For RV32 there is no instruction to move a double-floating register into
// the integer register file directly, so this must be done through memory.
//
#if __SEGGER_RTL_FP_ABI >= 2
        addi    sp, sp, -STACK_ALIGN(8)
        fsd     fa0, 0(sp)
        lw      a0, 0(sp)
        lw      a1, 4(sp)
        addi    sp, sp, +STACK_ALIGN(8)
#endif

// Save sign.
        mv      t1, a1

// Extract exponent to a3.
        BFOZ    a3, a1, 30, 20

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a3, a3, -0x3FF
        bltz    a3, L(zero_result)             // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x3F
        sub     a2, a2, a3
        blez    a2, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a1, a1, 11
        srl     a3, a0, 21
        or      a1, a1, a3
        sll     a0, a0, 11

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a1, a1, 31
#else
        li      a3, 0x80000000
        or      a1, a1, a3
#endif

// Integerize with rounding towards zero.  This divides into two
// classes of shift, one with 32 or more bits, one with less than 32.
        add     a5, a2, -32
        bltz    a5, L(full_shift)
        srl     a0, a1, a5
        li      a1, 0
        j       L(apply_sign)

// Full shift right of a1:a0 by a2 bits.
L(full_shift):
        sll     a5, a1, 1
        li      a4, 31
        sub     a4, a4, a2
        sll     a5, a5, a4
        srl     a0, a0, a2
        or      a0, a0, a5
        srl     a1, a1, a2

// Apply sign of input to integerized output.
L(apply_sign):
        bgez    t1, L(positive)
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        li      a2, 0
        li      a3, 0
        sub64   a0, a2, a0
#else
        snez    a2, a0
        neg     a0, a0
        neg     a1, a1
        sub     a1, a1, a2
#endif
L(positive):
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        li      a1, 0
        ret

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet or signed or not.
L(overflow_result):
        snez    a0, a0                  // Fold in significand bits from low-order
        or      a1, a1, a0

// Check for NaN.
        li      a2, 0xFFE00000
        sll     a3, a1, 1
        bleu    a3, a2, L(not_nan)

// Clear sign bit of NaN.
        li      a1, 0

// Isolate sign bit and return INT_MAX or INT_MIN.
L(not_nan):
        li      a0, 0
        srl     a1, a1, 31
        sll     a1, a1, 31
        bnez    a1, L(done)             // Set maximum -ve value, a1:a0 is 0x80000000:0x00000000.
        li      a0, -1                  // Set maximum +ve value, a1:a1 is 0x????????:0xFFFFFFFF.
        srl     a1, a0, 1               // Set maximum +ve value, a1:a1 is 0x7FFFFFFF:0xFFFFFFFF.
L(done):
        ret

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

// ISA can do this directly.
        fcvt.l.d a0, fa0, rtz
        ret

#else

// Save sign.
        sra     a3, a0, 63

// Move off sign.  
        sll     a1, a0, 1

// Extract biased exponent to a1.
        srl     a1, a1, 53

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x3FF
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x3F
        sub     a1, a2, a1
        blez    a1, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a0, a0, 11

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 63
#else
        sll     a2, a2, 63              // li a2, 0x8000000000000000...but faster
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a1

// Apply sign of input to integerized output.
        xor     a0, a0, a3
        sub     a0, a0, a3
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet or signed or not.
L(overflow_result):
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller
        sll     a3, a0, 1
        bleu    a3, a2, L(not_nan)

// Clear sign bit of NaN.
        li      a0, 0

// Isolate sign bit and return INT_MAX or INT_MIN.
L(not_nan):
        srl     a0, a0, 63
        sll     a0, a0, 63
        bnez    a0, L(done)
        addi    a0, a0, -1
        srl     a0, a0, 1
L(done):
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __fixdfdi

/*********************************************************************
*
*       __fixunssfsi()
*
*  Function description
*    Convert float to unsigned.
*
*  Parameters
*    a0 - Float value to convert.
*
*  Return value
*    a0 - Integerized value.
*/

#undef L
#define L(label) .L__fixunssfsi_##label

GLOBAL_FUNC __fixunssfsi

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 1

        fcvt.wu.s a0, fa0, rtz
        ret

#else

// Move off sign.  If negative (not the common case) truncate to 0.
        blez    a0, L(zero_result)

// Extract exponent.
        BFOZ    a1, a0, 30, 23

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x7F
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
        neg     a1, a1
        add     a1, a1, 0x1F
        bltz    a1, L(max_result)

// Shift off exponent bits.
        sll     a0, a0, 8

// Set hidden bit.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 31
#else
        li      a3, 0x80000000
        or      a0, a0, a3
#endif

// Integerize with rounding towards zero and return.
        srl     a0, a0, a1              // Truncate argument
        ret

// If argument is >= 2^32, saturate to UINT_MAX.
L(max_result):
        li      a0, -1
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 1

        fcvt.wu.s a0, fa0, rtz
        ret

#else

// If negative (not the common case) truncate to 0.
        sext.w  a0, a0                  // Incoming argument has undefined high 32 bits
        blez    a0, L(zero_result)

// Extract biased exponent to a1.
        srl     a1, a0, 23

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x7F
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x1F
        sub     a1, a2, a1
        bltz    a1, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a0, a0, 8

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 63
#else
        sll     a2, a2, 31
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        srlw    a0, a0, a1
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// If argument is >= 2^32, saturate to UINT_MAX.
L(overflow_result):
        li      a0, -1
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __fixunssfsi

/*********************************************************************
*
*       __fixunsdfsi()
*
*  Function description
*    Convert double to unsigned.
*
*  Parameters
*    a1:a0 - Double value to convert.
*
*  Return value
*    a0 - Integerized value.
*/

#undef L
#define L(label) .L__fixunsdfsi_##label

GLOBAL_FUNC __fixunsdfsi

#if __SEGGER_RTL_FP_ABI >= 2

        fcvt.wu.d a0, fa0, rtz
        ret


#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// If negative (not the common case) truncate to 0.
        bltz    a1, L(zero_result)

// Extract biased exponent to a2.
        srl     a2, a1, 20

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a2, a2, -0x3FF
        bltz    a2, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a4, 31
        sub     a4, a4, a2
        bltz    a4, L(overflow_result)

// Shift off exponent bits and recombine significand.
        srl     a0, a0, 21
        sll     a1, a1, 11
        or      a0, a0, a1

// Set hidden bit.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 31
#else
        li      a3, 0x80000000
        or      a0, a0, a3
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a4
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result.
L(overflow_result):
        li      a0, -1
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// If negative (not the common case) truncate to 0.
        bltz    a0, L(zero_result)

// Extract biased exponent to a1.
        srl     a1, a0, 52

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x3FF
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x1F
        sub     a1, a2, a1
        bltz    a1, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a0, a0, 11

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 63
#else
        sll     a2, a2, 63              // li a2, 0x8000000000000000...but faster
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a1
        sra     a0, a0, 32
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result based on sign.  NaNs are converted to INT_MAX
// irrespective of whether they are signaling or quiet or signed or not.
L(overflow_result):
        li      a0, -1
        ret

#else

#error Bad configuration

#endif

END_FUNC __fixunsdfsi

/*********************************************************************
*
*       __fixunssfdi()
*
*  Function description
*    Convert float to unsigned long long.
*
*  Prototype
*    unsigned long long __fixunssfdi(float x);
*
*  Parameters
*    a0  - Float value to convert :: soft ABI.
*    fa0 - Float value to convert :: single and double ABI.
*
*  Return value
*    a1:a0 / a0 - Integerized value.
*/

#undef L
#define L(label) .L__fixunssfdi_##label

GLOBAL_FUNC __fixunssfdi

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 2

        fcvt.d.s fa0, fa0
        tail     __fixunsdfdi

#else

#if __SEGGER_RTL_FP_ABI == 1
        fmv.x.w a0, fa0
#endif

// Move off sign.  If negative (not the common case) truncate to 0.
        bltz    a0, L(zero_result)
        sll     a2, a0, 1

// Extract biased exponent to a2.
        srl     a2, a2, 24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a2, a2, -0x7F
        bltz    a2, L(zero_result)      // (1,2]*2^(-1) and below are truncated to zero.

// Pre-load constant.
        li      a3, 0x80000000

// Break shifting into two sets: one shifts by less than 32 places, the other by more.
        li      t0, 32
        bge     a2, t0, L(long_shift)

// Value is less than 2^32 so shift is by fewer than 32 bits.
// Compute number of bits to shift significand by to align correctly.
        neg     a2, a2
        add     a2, a2, 0x1F

// Shift off exponent bits.
        sll     a0, a0, 8

// Set hidden bit.
        or      a0, a0, a3

// Integerize with rounding towards zero.
        srl     a0, a0, a2

// Set the high 32 bits of the result and return.
        li      a1, 0 
        ret

// Value is >= 2^32 so shift is by 32 bits or more.
// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
L(long_shift):
        neg     a2, a2
        add     a2, a2, 0x3F
        bltz    a2, L(overflow_result)

// Shift off exponent bits.
        sll     a1, a0, 8

// Set hidden bit.
        or      a1, a1, a3

// Integerize with rounding towards zero.
        li      a0, 0
        beqz    a2, L(shift_32)
        neg     a3, a2
        sll     a0, a1, a3
        srl     a1, a1, a2
L(shift_32):
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        li      a1, 0
        ret

// Argument is outside the integer range so saturate result based on sign.
L(overflow_result):
        li      a0, -1                  // Saturate to +ve maximum
        li      a1, -1
        ret

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

// ISA can do this directly.
        fcvt.lu.s a0, fa0, rtz
        ret

#else

#if __SEGGER_RTL_FP_ABI == 1
        fmv.x.w a0, fa0
#endif

// Move off sign.  If negative (not the common case) truncate to 0.
        sext.w  a0, a0                  // Upper 32 bits of incoming float are undefined
        bltz    a0, L(zero_result)

// Extract biased exponent to a2.
        sll     a1, a0, 1
        srl     a1, a1, 24

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x7F
        bltz    a1, L(zero_result)      // (1,2]*2^(-1) and below are truncated to zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x3F
        sub     a1, a2, a1
        bltz    a1, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a0, a0, 32+8

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 63
#else
        sll     a2, a2, 63              // li a2, 0x8000000000000000...but faster
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a1
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate result based on sign.
L(overflow_result):
        li      a0, -1                  // Saturate to +ve maximum
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __fixunssfdi

/*********************************************************************
*
*       __fixunsdfdi()
*
*  Function description
*    Convert double to unsigned long.
*
*  Parameters
*    a1:a0 / a0 - Floating value to convert :: soft and single ABI.
*    fa0        - Floating value to convert        :: double ABI.
*
*  Return value
*    a1:a0 / a0 - Integerized value.
*/

#undef L
#define L(label) .L__fixunsdfdi_##label

GLOBAL_FUNC __fixunsdfdi

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 2

// This aligns with RV64D FCVT.LU.D will do.

// Load constant 2^64.
       .set       __SEGGER_RTL_2pow64_REQUIRED, 1
        lui       a2,  %hi(__SEGGER_RTL_2pow64)
        fld       fa1, %lo(__SEGGER_RTL_2pow64)(a2)

// If less than zero, truncate to zero.  Use sign injection to capture negative NaN.
        fcvt.d.w  fa2, zero
        fsgnj.d   fa1, fa1, fa0
        flt.d     a0, fa1, fa2
        bnez      a0, L(zero)

// If NaN, return max.
        feq.d     a0, fa0, fa0
        beqz      a0, L(max)

// If greater than or equal to 2^64, return MAX_UINT.
        fge.d     a0, fa0, fa1
        bnez      a0, L(max)

// Load constants 2^32 and 2^-32.
       .set       __SEGGER_RTL_2pow32_REQUIRED, 1
       .set       __SEGGER_RTL_2powNeg32_REQUIRED, 1
        lui       a2,  %hi(__SEGGER_RTL_2pow32)
        fld       fa1, %lo(__SEGGER_RTL_2pow32)(a2)
        lui       a2,  %hi(__SEGGER_RTL_2powNeg32)
        fld       fa2, %lo(__SEGGER_RTL_2powNeg32)(a2)

// Get upper 32 bits of integerized value by scaling input by 2^-32.
        fmul.d    fa2, fa2, fa0
        fcvt.wu.d a1, fa2, rtz

// Get lower 32 bits of integerized value by subtracting integerized
// high part from argument.
        fcvt.d.wu fa2, a1
        fnmsub.d  fa2, fa2, fa1, fa0
        fcvt.wu.d a0, fa2, rtz
        ret

// Return 0.
L(zero):
        li        a0, 0
        li        a1, 0
        ret

// Return MAX_UINT.
L(max):
        li        a0, -1
        li        a1, -1
        ret

#else

// If negative (not the common case) truncate to 0.
        bltz    a1, L(zero_result)

// Move off sign.  
        sll     a3, a1, 1

// Extract biased exponent to a2, magnitude to a3.
        srl     a3, a3, 21

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a3, a3, -0x3FF
        bltz    a3, L(zero_result)             // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x3F
        sub     a2, a2, a3
        bltz    a2, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a1, a1, 11
        srl     a3, a0, 21
        or      a1, a1, a3
        sll     a0, a0, 11

// Set hidden bit at bit 63.
        li      a3, 0x80000000
        or      a1, a1, a3

// Integerize with rounding towards zero.  This divides into two
// classes of shift, one with 32 or more bits, one with less than 32.
        add     a5, a2, -32
        bltz    a5, L(full_shift)
        srl     a0, a1, a5
        li      a1, 0
        ret

// Full shift right of a1:a0 by a2 bits.
L(full_shift):
        sll     a5, a1, 1
        li      a4, 31
        sub     a4, a4, a2
        sll     a5, a5, a4
        srl     a0, a0, a2
        or      a0, a0, a5
        srl     a1, a1, a2
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        li      a1, 0
        ret

// Argument is outside the integer range so saturate to maximum.
L(overflow_result):
        li      a1, -1
        li      a0, -1
        ret

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

// ISA can do this directly.
        fcvt.lu.d a0, fa0, rtz
        ret

#else

// If negative (not the common case) truncate to 0.
        bltz    a0, L(zero_result)

// Extract biased exponent to a1.
        srl     a1, a0, 52

// Adjust exponent to zero-based power of two.  Early out for small numbers.
        add     a1, a1, -0x3FF
        bltz    a1, L(zero_result)      // 2^(-1) and below are always zero.

// Compute number of bits to shift significand by to align correctly.
// If there is overflow, which is not the common case, go to saturate the result.
        li      a2, 0x3F
        sub     a1, a2, a1
        bltz    a1, L(overflow_result)

// Shift off exponent bits and align significand to bit 62.
        sll     a0, a0, 11

// Set hidden bit at bit 63.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 63
#else
        sll     a2, a2, 63              // li a2, 0x8000000000000000...but faster
        or      a0, a0, a2
#endif

// Integerize with rounding towards zero.
        srl     a0, a0, a1
        ret

// Return zero, magnitude of argument is smaller than 1.0.
L(zero_result):
        li      a0, 0
        ret

// Argument is outside the integer range so saturate to maximum.
L(overflow_result):
        li      a0, -1
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __fixunsdfdi

/*********************************************************************
*
*       __floatsisf()
*
*  Function description
*    Convert int to float.
*
*  Parameters
*    a0 - Integer value to convert.
*
*  Return value
*    a0 - Floating value.
*/

#undef L
#define L(label) .L__floatsisf_##label

GLOBAL_FUNC __floatsisf

#if __SEGGER_RTL_FP_ABI >= 1

        fcvt.s.w fa0, a0
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Move sign bit of operand into place just above exponent work area.
        srl     a2, a0, 31
        sll     a2, a2, 8

// Set a2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the binary point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// of one here.
        add     a2, a2, 31+0x7F-1

// Floating 0 to 0.0 is easy.
        beqz    a0, L(done)

// Set a1 = abs(a0).
        sra     a3, a0, 31               // a3 is sign copied to all bits, -1/0
        xor     a1, a0, a3               // Conditional complement...
        sub     a1, a1, a3               // ...and conditional add one, i.e. negate if -ve

// Normalize.
        NORM32D a1, a2, a3              // Normalize a1, adjust count a2 down by number of shifts to normalize, use a3 as a temporary

// Move exponent into final position.
L(normalized):
        sll     a2, a2, 23

// Move significand into position.
        srl     a0, a1, 8

// Scrub off upper bits of significand and prepare to round
// on lower order bits only.
        sll     a1, a1, 23
        sltz    t1, a1
        LEAH    a1, t1, a1

// 0x rounds down, 1 may need to round up or down...
        bgez    a1, L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        add     a1, a1, a1
        snez    a1, a1
        add     a0, a0, a1

// Insert exponent and break-tie flag.  Overflow gets nicely handled
L(round_down):
        add     a0, a0, a2
L(done):
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_OPTIMIZE < 0

        tail    __floatdisf

#else

// Floating zero.
        beqz    a0, L(done)

// Preserve sign in a2.
	sltz    a2, a0

// Calculate magnitude of argument.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        neg     a3, a2
        xor     a0, a0, a2
        sub     a0, a0, a2
#else
        beqz    a2, L(positive)
        neg     a0, a0
L(positive):
#endif

// Move sign bit into position adjacent to developing exponent.
        sll     a2, a2, 8

// Full 64-bit normalization.  Note that either a0 is positive
// or a0 is 0x8000000000000000.  In latter case, it doesn't
// matter that the shift count for one of the shifts is 64 because
// the value shifted is always zero, so specify "positive flow"
// to the normalization macro.
        add     a2, a2, 31+0x7F-1       // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.
        NORM32D a0, a2, a3              // Normalize a0, initialize and adjust count a3 down by number of shifts to normalize, use a2, a4 as a temporaries


// Move exponent into final position.
L(normalized):
        sll     a2, a2, 23

// Scrub off upper bits of significand and isolate bits to be discarded.
        sll     a1, a0, 24

// Move significand into position.
        srl     a0, a0, 32+8

// Round based on discarded bits from significand.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltz    a5, a1                  // Isolate most significant discarded bit
        add     a0, a0, a5              // Round up or not
        li      a5, 1                   // li a5, 0x8000000000000000...
        sll     a5, a5, 63              // ...but faster
        sub     a1, a1, a5
        seqz    a1, a1
        ANDNx   a0, a0, a1
#else
        bgez    a1, L(no_rounding)
        add     a0, a0, 1
        sll     a1, a1, 1
        bnez    a1, L(no_rounding)      // No tie
        and     a0, a0, ~1
#endif

// Move significand into position and combine with sign+exponent.
L(no_rounding):
        add     a0, a0, a2

L(done):
#if defined(__riscv_float_abi_single) || defined(__riscv_float_abi_double)
        fmv.w.x fa0, a0
#endif
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __floatsisf

/*********************************************************************
*
*       __floatsidf()
*
*  Function description
*    Convert int to double.
*
*  Parameters
*    a0 - Integer value to convert.
*
*  Return value
*    a1:a0 / a0 - Floating value.
*/

#undef L
#define L(label) .L__floatsidf_##label

GLOBAL_FUNC __floatsidf

#if __SEGGER_RTL_FP_ABI >= 2

        fcvt.d.w fa0, a0
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Save sign of input.
        sra     a1, a0, 31

// Floating zero is easy as long as a1 is initialized.
        beqz    a0, L(zero)

// Compute absolute value of input.
        xor     a0, a0, a1
        sub     a0, a0, a1

// Set a2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the radix point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// by one here.  Normalize.
        li      a2, 0x41D
        NORM32D a0, a2, a3              // Normalize a0, adjust count a2 down by number of shifts to normalize, use a3 as a temporary

// Move exponent into final position.
L(normalized):
        sll     a2, a2, 20              // Insert exponent

// Move significand into final position.
        srl     a3, a0, 11
        sll     a0, a0, 21

// Combine exponent and sign with significand.
        sll     a1, a1, 31 
        add     a2, a2, a3
        add     a1, a1, a2
L(zero):
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Floating zero is easy as long as a1 is initialized.
        beqz    a0, L(zero)

// Save sign of input.
        sra     a1, a0, 63

// Compute absolute value of input.
        xor     a0, a0, a1
        sub     a0, a0, a1

// Set a2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the radix point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// by one here.  Normalize.
        li      a2, 0x41D
        NORM32D a0, a2, a3              // Normalize a0, adjust count a2 down by number of shifts to normalize, use a3 as a temporary

// Move exponent into final position.
L(normalized):
        sll     a2, a2, 52              // Insert exponent

// Move significand into final position.
        srl     a0, a0, 11

// Combine exponent and sign with significand.
        sll     a1, a1, 63
        add     a0, a0, a1
        add     a0, a0, a2
L(zero):
        ret

#else

#error Bad configuration

#endif

END_FUNC __floatsidf

/*********************************************************************
*
*       __floatdisf()
*
*  Function description
*    Convert long long to float.
*
*  Parameters
*    a1:a0 / a0 - Integer value to convert.
*
*  Return value
*    a0  - Floating value :: soft ABI.
*    fa0 - Floating value :: single and double ABI.
*/

#undef L
#define L(label) .L__floatdisf_##label

GLOBAL_FUNC __floatdisf

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_OPTIMIZE < 0

// Save sign.
        srl     t0, a1, 31
        sll     t0, t0, 31

// Conditionally negate input, a1:a0 = abs(a1:a0).
        bgez    a1, L(positive)
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        li      a2, 0
        li      a3, 0
        sub64   a0, a2, a0
#else
        snez    a3, a0
        neg     a0, a0
        neg     a1, a1
        sub     a1, a1, a3
#endif

// Set a2 to the exponent of 2^63, i.e. 63.  However, we don't knock off the
// integer bit before the decimal point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
L(positive):
        li      a2, 63+0x7F-1

// If the value to float is less than 2^32, i.e. the high word is zero, then
// we can simply float a 32-bit.
        bnez    a1, L(normalize)
        beqz    a0, L(done)             // 0 -> 0.0

// Quick shift by 32.
        mv      a1, a0
        li      a0, 0
        add     a2, a2, -32

// Iteratively normalize.
L(normalize):
        NORM64D a1, a0, a2, a3, a4      // Normalize a1:a0, adjust count a2 down by number of shifts to normalize, use a3, a4 as a temporaries

// Move exponent into final position.
L(normalized):
        sll     a2, a2, 23

// Compress (fold) bits from lower half only if they are non-zero.
        snez    a0, a0
        or      a1, a1, a0

// Move significand into position.
        srl     a0, a1, 8

// Scrub off upper bits of significand and prepare to round
// on lower order bits only.
        sll     a1, a1, 23
        sltz    t1, a1
        LEAH    a1, t1, a1

// 0x rounds down, 1 may need to round up or down...
        bgez    a1, L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        add     a1, a1, a1
        snez    a1, a1
        add     a0, a0, a1

// Insert exponent and sign.  Overflow gets nicely handled
L(round_down):
        add     a0, a0, a2
        add     a0, a0, t0
L(done):
#if defined(__riscv_float_abi_single) || defined(__riscv_float_abi_double)
        fmv.w.x fa0, a0
#endif
        ret

#else

// Preserve sign in a2 and test it.
	srl     a2, a1, 31
        sll     a2, a2, 8
        beqz    a2, L(positive)

// Form magnitude of argument by negating it.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        li      a4, 0
        li      a5, 0
        sub64   a0, a4, a0
#else
        snez    a3, a0
        neg     a0, a0
        neg     a1, a1
        sub     a1, a1, a3
#endif

// Divide into two flows based on magnitide.
L(positive):
        beqz    a1, L(high_word_zero)   // Top 32 bits are zero, so shift quickly.

// Full 64-bit normalization.  Note that either a1:a0 is positive
// or a1:a0 is 0x80000000:0x00000000.  In latter case, it doesn't
// matter that the shift count for one of the shifts is 32 because
// the value shifted is always zero, so specify "positive flow"
// to the normalization macro.
        add     a2, a2, 63+0x7F-1       // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.
        NORM64D a1, a0, a2, a3, a4,, pos  // Normalize a1:a0, initialize and adjust count a3 down by number of shifts to normalize, use a2, a4 as a temporaries

// Compress (fold) bits from lower half only if they are non-zero.
L(normalized):
        snez    a0, a0
        or      a1, a1, a0

// Move exponent into final position.
L(round_and_pack):
        sll     a2, a2, 23

// Move significand into position.
        srl     a0, a1, 8

// Scrub off upper bits of significand and prepare to round
// on lower order bits only.
        sll     a1, a1, 23
        sltz    t1, a1
        LEAH    a1, t1, a1

// 0x rounds down, 1 may need to round up or down...
        bgez    a1, L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        add     a1, a1, a1
        snez    a1, a1
        add     a0, a0, a1

// Insert exponent and sign.  Overflow gets nicely handled
L(round_down):
        add     a0, a0, a2
L(done):
#if defined(__riscv_float_abi_single) || defined(__riscv_float_abi_double)
        fmv.w.x fa0, a0
#endif
        ret

// Normalization.
L(high_word_zero):
        beqz    a0, L(done)

// Set exponent.
        add     a2, a2, 31+0x7F-1

#if __SEGGER_RTL_CORE_HAS_CLZ || __SEGGER_RTL_CORE_HAS_CLZ32
// This is a specialized version where normalization ends up
// in a different register.
        CLZ     a3, a0
        sub     a2, a2, a3
        sll     a1, a0, a3
#else
        NORM32D a0, a2, a3              // Normalize a0, adjust count a2 down by number of shifts to normalize, use a3 as a temporary
        mv      a1, a0
#endif
        li      a0, 0
        j       L(round_and_pack)

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

// ISA can do this directly.
        fcvt.s.l fa0, a0
        ret

#else

// Floating zero.
        beqz    a0, L(done)

// Preserve sign in a2.
	sltz    a2, a0

// Calculate magnitude of argument.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        neg     a3, a2
        xor     a0, a0, a2
        sub     a0, a0, a2
#else
        beqz    a2, L(positive)
        neg     a0, a0
L(positive):
#endif

// Move sign bit into position adjacent to developing exponent.
        sll     a2, a2, 8

// Full 64-bit normalization.  Note that either a0 is positive
// or a0 is 0x8000000000000000.  In latter case, it doesn't
// matter that the shift count for one of the shifts is 64 because
// the value shifted is always zero, so specify "positive flow"
// to the normalization macro.
        add     a2, a2, 63+0x7F-1       // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.
        NORM64D a0, a2, a3              // Normalize a0, initialize and adjust count a3 down by number of shifts to normalize, use a2, a4 as a temporaries


// Move exponent into final position.
L(normalized):
        sll     a2, a2, 23

// Scrub off upper bits of significand and isolate bits to be discarded.
        sll     a1, a0, 24

// Move significand into position.
        srl     a0, a0, 32+8

// Round based on discarded bits from significand.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltz    a5, a1                  // Isolate most significant discarded bit
        add     a0, a0, a5              // Round up or not
        li      a5, 1                   // li a5, 0x8000000000000000...
        sll     a5, a5, 63              // ...but faster
        sub     a1, a1, a5
        seqz    a1, a1
        ANDNx   a0, a0, a1
#else
        bgez    a1, L(no_rounding)
        add     a0, a0, 1
        sll     a1, a1, 1
        bnez    a1, L(no_rounding)      // No tie
        and     a0, a0, ~1
#endif

// Move significand into position and combine with sign+exponent.
L(no_rounding):
        add     a0, a0, a2

L(done):
#if defined(__riscv_float_abi_single) || defined(__riscv_float_abi_double)
        fmv.w.x fa0, a0
#endif
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __floatdisf

/*********************************************************************
*
*       __floatdidf()
*
*  Function description
*    Convert long long to double.
*
*  Parameters
*    a1:a0 - Integer value to convert.
*
*  Return value
*    a1:a0 - Floating value :: soft and single abi.
*    fa0   - Floating value :: double abi.
*/

#undef L
#define L(label) .L__floatdidf_##label

GLOBAL_FUNC __floatdidf

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 2

// Float both halves of the 64-bit value independently
// then add them together, scaled.

// Load constant 2^32 to floating register.
       .set       __SEGGER_RTL_2pow32_REQUIRED, 1
        lui       a2,  %hi(__SEGGER_RTL_2pow32)
        fld       fa2, %lo(__SEGGER_RTL_2pow32)(a2)

// Float high and low parts.
        fcvt.d.w  fa1, a1               // fa1 = High part
        fcvt.d.wu fa0, a0               // fa0 = Low part

// Scale and sum.
        fmadd.d   fa0, fa1, fa2, fa0    // fa1*2^32 + fa0 <=> High*2^32 + Low
        ret

#elif __SEGGER_RTL_OPTIMIZE < 0

// Floating 0 to 0.0 is easy.
        or      a2, a0, a1
        beqz    a2, L(done)

// Preserve sign of input with exponent.
        srl     a3, a1, 31
        sll     a3, a3, 11

// Compute absolute value of input.
        beqz    a3, L(input_positive)
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        li      a4, 0
        li      a5, 0
        sub64   a0, a4, a0
#else
        snez    a2, a0
        neg     a0, a0
        neg     a1, a1
        sub     a1, a1, a2
#endif

// Set a3 to the exponent of 2^63, i.e. 63.  However, we don't knock off the
// integer bit before the binary point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
L(input_positive):
        add     a3, a3, 0x43D           // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.  43D = 63+0x3FF-1

// Divide into fast and slow paths.
        bnez    a1, L(no_quick_shift)
        mv      a1, a0
        li      a0, 0
        add     a3, a3, -32             // Adjust exponent

// Quick test for 16-bit normalization.
L(no_quick_shift):
        li      a2, 0x10000
        bgeu    a1, a2, L(clockwork_shifts)
        add     a3, a3, -16
        sll     a1, a1, 16
        srl     a2, a0, 16
        or      a1, a1, a2
        sll     a0, a0, 16

// Do normlization slowly, a bit at a time.
L(clockwork_shifts):

// If already normalized, don't need to do any shifting.
        bltz    a1, L(normalized)

// Normalize by clockwork.
L(normalize):
        add     a3, a3, -1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a0
#else
        sltz    a2, a0
        add     a0, a0, a0
        LEAH    a1, a2, a1
#endif
        bgez    a1, L(normalize)

// Now need to round and combine exponent and sign.
L(normalized):

// Save copy of low bits, we need them to decide on rounding.
        mv      t1, a0

// Move significand into position, right shift a1:a0 by 11 bits
        srl     a0, a0, 11
        sll     a2, a1, 21
        or      a0, a0, a2
        srl     a1, a1, 11

// Move result's sign and exponent into position (see below for insertion).
        sll     a3, a3, 20

// Now round.  Shift low-order bits rounding bits into high order.
        sll     a2, t1, 20              // Recover low bits
        sltz    a4, a2                  // a2 = value of bit 21
        add     a2, a2, a2              // Correct shift
        sltz    a5, a2                  // shift out...
        LEAH    a2, a4, a2              // shift and shift in
        beqz    a5, L(no_rounding)
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        snez    a4, a2
        li      a5, 0
        add64   a0, a0, a4
#else
        snez    a4, a2
        add     a0, a0, a4              //    shift in...
        sltu    a4, a0, a4              // ...carry out
        add     a1, a1, a4              //    shift in...
#endif

L(no_rounding):
        add     a1, a1, a3              // Insert sign and exponent (see above)

// All done.
L(done):
        ret

#else

// Move sign bit to adjoin exponent.
        srl     a3, a1, 31
        sll     a3, a3, 11
        beqz    a3, L(positive)

// Form magnitude of argument by negating it.
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        li      a4, 0
        li      a5, 0
        sub64   a0, a4, a0
#else
        snez    a2, a0
        neg     a0, a0
        neg     a1, a1
        sub     a1, a1, a2
#endif

// Normalization.  Divide into two flows.
L(positive):
        beqz    a1, L(high_word_zero)

// Full 64-bit normalization.  Note that either a1:a0 is positive
// or a1:a0 is 0x80000000:0x00000000.  In latter case, it doesn't
// matter that the shift count for one of the shifts is 32 because
// the value shifted is always zero, so specify "positive flow"
// to the normalization macro.
        add     a3, a3, 0x43D
        NORM64D a1, a0, a3, a2, a4,, pos  // Normalize a1:a0, initialize and adjust count a3 down by number of shifts to normalize, use a2, a4 as a temporaries

// Shift low-order bits, we need them to decide on rounding.
L(normalized):
        sll     a2, a0, 20

// Move significand into position, right shift a1:a0 by 11 bits
        srl     a0, a0, 11
        sll     a4, a1, 21
        or      a0, a0, a4
        srl     a1, a1, 11

// Move result's sign and exponent into position (see below for insertion).
        sll     a3, a3, 20

// Now round.  Recover low bits and shift low-order bits rounding bits into high order.
// Shift low-order bits rounding bits into high order.
        sltz    a4, a2                  // a4 = value of bit 21
        add     a2, a2, a2              // Correct shift
        sltz    a5, a2                  // Rounding gate: a5 is 0 if no rounding required, 1 is rounding required
        LEAH    a2, a4, a2              // Shift and add carry in
        snez    a2, a2
        and     a2, a2, a5              // Combine rounding using gate
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a2
#else
        add     a0, a0, a2              //    shift in...
        sltu    a5, a0, a2              // ...carry out
        add     a1, a1, a5              //    shift in...
        add     a1, a1, a3              // Insert sign and exponent (see above)
#endif

// All done.
L(done):
        ret

L(high_word_zero):
        beqz    a0, L(done)

// Set exponent and normalize.
        add     a3, a3, 0x41D
        NORM32D a0, a3, a2              // Normalize a0, adjust count a3 down by number of shifts to normalize, use a2 as a temporary

// 32 bits fit into a double exactly with no rounding.
        srl     a1, a0, 11              // Move significand into position
        sll     a0, a0, 21
        sll     a3, a3, 20
        add     a1, a1, a3
        ret

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

// ISA can do this directly

        fcvt.d.l  fa0, a0
        ret

#else

// Floating 0 is zero.
        beqz    a0, L(done)

// Move sign bit to adjoin exponent.
        sltz    a3, a0
        sll     a3, a3, 11

// Compute magnitude of a0.
        sra     a1, a0, 63
        xor     a0, a0, a1
        sub     a0, a0, a1

// Full 64-bit normalization.
        add     a3, a3, 0x43D
        NORM64D a0, a3, a2              // Normalize a0, initialize and adjust count a3 down by number of shifts to normalize

// Shift low-order bits, we need them to decide on rounding.
L(normalized):
        sll     a2, a0, 53

// Move significand into position.
        srl     a0, a0, 11

// Now round.  Recover low bits and shift low-order bits rounding bits into high order.
// Shift low-order bits rounding bits into high order.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltz    a5, a2                  // Isolate most significant discarded bit
        add     a0, a0, a5              // Round up or not
        li      a5, 1                   // li a5, 0x8000000000000000...
        sll     a5, a5, 63              // ...but faster
        sub     a2, a2, a5
        seqz    a2, a2
        ANDNx   a0, a0, a2
#else
        bgez    a2, L(no_rounding)
        add     a0, a0, 1
        sll     a2, a2, 1
        bnez    a2, L(no_rounding)      // No tie
        and     a0, a0, ~1
#endif

// Combine significand with sign+exponent.
L(no_rounding):
        sll     a3, a3, 52              // Move result's sign and exponent into position
        add     a0, a0, a3

// All done.
L(done):
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __floatdidf

/*********************************************************************
*
*       __floatunsisf()
*
*  Function description
*    Convert unsigned to float.
*
*  Parameters
*    a0 - Unsigned value to convert.
*
*  Return value
*    a0 - Float value.
*/

#undef L
#define L(label) .L__floatunsisf_##label

GLOBAL_FUNC __floatunsisf

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 1

        fcvt.s.wu fa0, a0
        ret

#else

// Floating 0 to 0.0 is easy.
        beqz    a0, L(done)

// Set a2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the decimal point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.  Normalize.
        li      a2, 31+0x7F-1
        NORM32D a0, a2, a3              // Normalize a0, adjust count a2 down by number of shifts to normalize, use a3 as a temporary

// Move exponent into final position.
L(normalized):
        sll     a2, a2, 23

// Scrub off upper bits of significand and prepare to round
// on lower order bits only.
        sll     a1, a0, 23
        srl     a0, a0, 8               // Move significand into position.
        sltz    t1, a1
        LEAH    a1, t1, a1

// 0x rounds down, 1 may need to round up or down...
        bgez    a1, L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        add     a1, a1, a1
        snez    a1, a1
        add     a0, a0, a1

// Insert exponent and break-tie flag.  Overflow gets nicely handled
L(round_down):
        add     a0, a0, a2
L(done):
        ret

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 1

        fcvt.s.wu fa0, a0
        ret

#elif __SEGGER_RTL_OPTIMIZE < 0

        sll     a0, a0, 32
        srl     a0, a0, 32
        tail    __floatundisf

#else

// Floating zero.
        beqz    a0, L(done)

// Move sign bit into position adjacent to developing exponent.
        sll     a2, a2, 8

// Full 64-bit normalization.  Note that either a0 is positive
// or a0 is 0x8000000000000000.  In latter case, it doesn't
// matter that the shift count for one of the shifts is 64 because
// the value shifted is always zero, so specify "positive flow"
// to the normalization macro.
        li      a2, 31+0x7F-1           // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.
        NORM32D a0, a2, a3              // Normalize a0, initialize and adjust count a3 down by number of shifts to normalize, use a2, a4 as a temporaries


// Move exponent into final position.
L(normalized):
        sll     a2, a2, 23

// Scrub off upper bits of significand and isolate bits to be discarded.
        sll     a1, a0, 24

// Move significand into position.
        srl     a0, a0, 32+8

// Round based on discarded bits from significand.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltz    a5, a1                  // Isolate most significant discarded bit
        add     a0, a0, a5              // Round up or not
        li      a5, 1                   // li a5, 0x8000000000000000...
        sll     a5, a5, 63              // ...but faster
        sub     a1, a1, a5
        seqz    a1, a1
        ANDNx   a0, a0, a1
#else
        bgez    a1, L(no_rounding)
        add     a0, a0, 1
        sll     a1, a1, 1
        bnez    a1, L(no_rounding)      // No tie
        and     a0, a0, ~1
#endif

// Move significand into position and combine with exponent.
L(no_rounding):
        add     a0, a0, a2

L(done):
#if defined(__riscv_float_abi_single) || defined(__riscv_float_abi_double)
        fmv.w.x fa0, a0
#endif
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __floatunsisf

/*********************************************************************
*
*       __floatunsidf()
*
*  Function description
*    Convert unsigned to double.
*
*  Parameters
*    a0 - Unsigned value to convert.
*
*  Return value
*    a1:a0 - Double value.
*/

#undef L
#define L(label) .L__floatunsidf_##label

GLOBAL_FUNC __floatunsidf

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 2

        fcvt.d.wu fa0, a0
        ret

#else

// Floating zero is easy.
        beqz    a0, L(zero)

// Set a2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the radix point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// by one here.
        li      a2, 0x41D
        NORM32D a0, a2, a3              // Normalize a0, adjust count a2 down by number of shifts to normalize, use a3 as a temporary

// Move exponent into final position.
L(normalized):
        sll     a2, a2, 20              // Insert exponent

// Move significand into final position.
        srl     a3, a0, 11
        sll     a0, a0, 21

// Combine exponent with significand.
        add     a1, a3, a2
        ret

// Duplicate zero to high order.
L(zero):
        mv      a1, a0
        ret

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

        fcvt.d.wu fa0, a0
        ret

#else

// Floating zero is easy as long as a1 is initialized.
        beqz    a0, L(zero)

// Set a2 to the exponent of 2^31, i.e. 31.  However, we don't knock off the
// integer bit before the radix point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// by one here.  Normalize.
        li      a2, 0x41D
        NORM32D a0, a2, a3              // Normalize a0, adjust count a2 down by number of shifts to normalize, use a3 as a temporary

// Move exponent into final position.
L(normalized):
        sll     a2, a2, 52              // Insert exponent

// Move significand into final position.
        srl     a0, a0, 11

// Combine exponent with significand.
        add     a0, a0, a2
L(zero):
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __floatunsidf

/*********************************************************************
*
*       __floatundisf()
*
*  Function description
*    Convert unsigned long long to float.
*
*  Parameters
*    a1:a0 / a0 - Unsigned long long value to convert.
*
*  Return value
*    a0 - Float value.
*/

#undef L
#define L(label) .L__floatundisf_##label

GLOBAL_FUNC __floatundisf

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 1

// If x less than 2^32, use FPU to directly float (and do rounding).
        bnez      a1, L(big)
        fcvt.s.wu fa0, a0
        ret

// X is larger than 2^32, defer to software.

L(big):

#endif

#if __SEGGER_RTL_OPTIMIZE < 0

// Set a2 to the exponent of 2^63, i.e. 63.  However, we don't knock off the
// integer bit before the decimal point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
        li      a2, 63+0x7F-1

// If the value to float is less than 2^32, i.e. the high word is zero, then
// we can simply float a 32-bit.
        bnez    a1, L(high_nonzero)
        beqz    a0, L(done)              // 0 -> 0.0

// Quick shift by 32.
        mv      a1, a0
        li      a0, 0
        add     a2, a2, -32
        j       L(high_nonzero)

// Iteratively normalize; anything faster turns out rather ugly.
L(normalize):
        add     a2, a2, -1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a0
#else
        sltz    a3, a0
        add     a0, a0, a0
        LEAH    a1, a3, a1
#endif
L(high_nonzero):
        bgez    a1, L(normalize)

// Move exponent into final position.
L(normalized):
        sll     a2, a2, 23

// Compress (fold) bits from lower half only if they are non-zero.
        snez    a0, a0
        or      a1, a1, a0

// Move significand into position.
        srl     a0, a1, 8

// Scrub off upper bits of significand and prepare to round
// on lower order bits only.
        sll     a1, a1, 23
        sltz    t1, a1
        LEAH    a1, t1, a1

// 0x rounds down, 1 may need to round up or down...
        bgez    a1, L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        add     a1, a1, a1
        snez    a1, a1
        add     a0, a0, a1

// Insert exponent and break-tie flag.  Overflow gets nicely handled
L(round_down):
        add     a0, a0, a2
L(done):
#if __SEGGER_RTL_FP_ABI >= 1
        fmv.w.x fa0, a0
#endif
        ret

#else

// Divide into two flows based on magnitide.
        beqz    a1, L(high_word_zero)   // Top 32 bits are zero, so shift quickly.

// Use full 64-bit normalization. Subtract an extra '1' as
// from the initialization as this compensates for the '1' added by
// the hidden bit which we do not remove.  
        NORM64D a1, a0, a2, a3, a4, 63+0x7F-1  // Normalize a1:a0, initialize and adjust count a2 down by number of shifts to normalize, use a3, a4 as a temporaries

// Compress (fold) bits from lower half only if they are non-zero.
L(normalized):
        snez    a0, a0
        or      a1, a1, a0

// Move exponent into final position.
L(round_and_pack):
        sll     a2, a2, 23

// Move significand into position.
        srl     a0, a1, 8

// Scrub off upper bits of significand and prepare to round
// on lower order bits only.
        sll     a1, a1, 23
        sltz    t1, a1
        LEAH    a1, t1, a1

// 0x rounds down, 1 may need to round up or down...
        bgez    a1, L(round_down)

// Ok, fraction is 1xxxx, with bits moved off it only ties if it is
// exactly zero, i.e. 100000000 shifted = carry set with result 0.
// For anything else there is no tie and no need to break it.
        add     a1, a1, a1
        snez    a1, a1
        add     a0, a0, a1

// Insert exponent and sign.  Overflow gets nicely handled
L(round_down):
        add     a0, a0, a2
L(done):
#if __SEGGER_RTL_FP_ABI >= 1
        fmv.w.x fa0, a0
#endif
        ret

// Normalization.
L(high_word_zero):
        beqz    a0, L(done)

// Set exponent and normalize.
        li      a2, 31+0x7F-1
        NORM32D a0, a2, a3              // Normalize a0, adjust count a2 down by number of shifts to normalize, use a3 as a temporary

L(hwz_normalized):
        mv      a1, a0
        li      a0, 0
        j       L(round_and_pack)

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

// ISA can do this directly.
        fcvt.s.lu fa0, a0
        ret

#else

// Floating zero.
        beqz    a0, L(done)

// Full 64-bit normalization.  Note that either a0 is positive
// or a0 is 0x8000000000000000.  In latter case, it doesn't
// matter that the shift count for one of the shifts is 64 because
// the value shifted is always zero, so specify "positive flow"
// to the normalization macro.
        li      a2, 63+0x7F-1           // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.
        NORM64D a0, a2, a3              // Normalize a0, initialize and adjust count a3 down by number of shifts to normalize, use a2, a4 as a temporaries


// Move exponent into final position.
L(normalized):
        sll     a2, a2, 23

// Scrub off upper bits of significand and isolate bits to be discarded.
        sll     a1, a0, 24

// Move significand into position.
        srl     a0, a0, 32+8

// Round based on discarded bits from significand.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltz    a5, a1                  // Isolate most significant discarded bit
        add     a0, a0, a5              // Round up or not
        li      a5, 1                   // li a5, 0x8000000000000000...
        sll     a5, a5, 63              // ...but faster
        sub     a1, a1, a5
        seqz    a1, a1
        ANDNx   a0, a0, a1
#else
        bgez    a1, L(no_rounding)
        add     a0, a0, 1
        sll     a1, a1, 1
        bnez    a1, L(no_rounding)      // No tie
        and     a0, a0, ~1
#endif

// Move significand into position and combine with exponent.
L(no_rounding):
        add     a0, a0, a2

L(done):
#if defined(__riscv_float_abi_single) || defined(__riscv_float_abi_double)
        fmv.w.x fa0, a0
#endif
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __floatundisf

/*********************************************************************
*
*       __floatundidf()
*
*  Function description
*    Convert unsigned long long to double.
*
*  Parameters
*    a1:a0 / a0 - Unsigned long long value to convert.
*
*  Return value
*    a1:a0 / a0 - Double value :: soft or single ABI.
*    fa0        - Double value :: double ABI.
*/

#undef L
#define L(label) .L__floatundidf_##label

GLOBAL_FUNC __floatundidf

#if __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI >= 2

// Float both halves of the 64-bit value independently
// then add them together, scaled.

// Load constant 2^32 to floating register.
       .set       __SEGGER_RTL_2pow32_REQUIRED, 1
        lui       a2,  %hi(__SEGGER_RTL_2pow32)
        fld       fa2, %lo(__SEGGER_RTL_2pow32)(a2)

// Float high and low parts.
        fcvt.d.wu fa1, a1
        fcvt.d.wu fa0, a0

// Scale and sum.
        fmadd.d   fa0, fa1, fa2, fa0
        ret

#else

#if __SEGGER_RTL_RT_OPTIMIZE < 0

// Floating 0 to 0.0 is easy.
        or      a2, a0, a1
        beqz    a2, L(done)

// Set a3 to the exponent of 2^63, i.e. 63.  However, we don't knock off the
// integer bit before the binary point and elect to add in the exponent rather
// than use a logical or when combining.  So, we need to take account of this
// extra one bit that appears when we add in the exponent, so make an adjustment
// one one here.
        li      a3, 0x43D               // Subtract an extra '1' as this compensates
                                        // for the '1' added by the hidden bit which
                                        // we do not remove.  43D = 63+0x3FF-1

// Divide into fast and slow paths.
        bnez    a1, L(no_quick_shift)
        mv      a1, a0
        li      a0, 0
        add     a3, a3, -32            // Adjust exponent

// Quick test for 16-bit normalization.
L(no_quick_shift):
        li      a2, 0x10000
        bgeu    a1, a2, L(clockwork_shifts)
        add     a3, a3, -16
        sll     a1, a1, 16
        srl     a2, a0, 16
        or      a1, a1, a2
        sll     a0, a0, 16

// Do normlization slowly, a bit at a time.
L(clockwork_shifts):

// If already normalized, don't need to do any shifting.
        bltz    a1, L(normalized)

// Normalize by clockwork.
L(normalize):
        add     a3, a3, -1
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a0
#else
        sltz    a2, a0
        add     a0, a0, a0
        LEAH    a1, a2, a1
#endif
        bgez    a1, L(normalize)

// Now need to round and combine exponent and sign.
L(normalized):

// Save copy of low bits, we need them to decide on rounding.
        mv      t1, a0

// Move significand into position, right shift a1:a0 by 11 bits
        srl     a0, a0, 11
        sll     a2, a1, 21
        or      a0, a0, a2
        srl     a1, a1, 11

// Move result's sign and exponent into position (see below for insertion).
        sll     a3, a3, 20

// Now round.  Recover low bits.
        mv      a2, t1

// Shift low-order bits rounding bits into high order.
        sll     a2, a2, 20
        sltz    a4, a2                  // a2 = value of bit 21
        add     a2, a2, a2              // Correct shift
        sltz    a5, a2                  //    shift out...
        LEAH    a2, a4, a2              //    shift and shift in
        beqz    a5, L(no_rounding)
        snez    a2, a2
        add     a0, a0, a2              //    shift in...
        sltu    a5, a0, a2              // ...carry out
        add     a1, a1, a5              //    shift in...

L(no_rounding):
        add     a1, a1, a3              // Insert sign and exponent (see above)

// All done.
L(done):
        ret

#else

// Divide normalization into two flows.
        beqz    a1, L(high_word_zero)

// Full 64-bit normalization.
        NORM64D a1, a0, a3, a2, a4, 0x43D  // Normalize a1:a0, initialize and adjust count a3 down by number of shifts to normalize, use a2, a4 as a temporaries

// Shift low-order bits, we need them to decide on rounding.
        sll     a2, a0, 20

// Move significand into position, right shift a1:a0 by 11 bits
        srl     a0, a0, 11
        sll     a4, a1, 21
        or      a0, a0, a4
        srl     a1, a1, 11

// Move result's sign and exponent into position (see below for insertion).
        sll     a3, a3, 20

// Now round.  Recover low bits and shift low-order bits rounding bits into high order.
        sltz    a4, a2                  // a4 = value of bit 21
        add     a2, a2, a2              // Correct shift
        sltz    a5, a2                  // Rounding gate: a5 is 0 if no rounding required, 1 is rounding required
        LEAH    a2, a4, a2              // Shift and add carry in
        snez    a2, a2
        and     a2, a2, a5              // Combine rounding using gate
#if __SEGGER_RTL_CORE_HAS_ISA_SIMD
        add64   a0, a0, a2
#else
        add     a0, a0, a2              //    shift in...
        sltu    a5, a0, a2              // ...carry out
        add     a1, a1, a5              //    shift in...
        add     a1, a1, a3              // Insert sign and exponent (see above)
#endif

// All done.
L(done):
        ret

L(high_word_zero):
        beqz    a0, L(done)

// Set exponent and normalize.
        li      a3, 0x41D
        NORM32D a0, a3, a2              // Normalize a0, adjust count a3 down by number of shifts to normalize, use a2 as a temporary

// 32 bits fit into a double exactly with no rounding.
        srl     a1, a0, 11             // Move significand into position
        sll     a0, a0, 21
        sll     a3, a3, 20
        add     a1, a1, a3
        ret

#endif

#endif

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI >= 2

// ISA can do this directly
        fcvt.d.lu fa0, a0
        ret

#else

// Floating 0 is zero.
        beqz    a0, L(done)

// Full 64-bit normalization.
        li      a3, 0x43D
        NORM64D a0, a3, a2              // Normalize a0, initialize and adjust count a3 down by number of shifts to normalize

// Shift low-order bits, we need them to decide on rounding.
L(normalized):
        sll     a2, a0, 53

// Move significand into position.
        srl     a0, a0, 11

// Now round.  Recover low bits and shift low-order bits rounding bits into high order.
// Shift low-order bits rounding bits into high order.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sltz    a5, a2                  // Isolate most significant discarded bit
        add     a0, a0, a5              // Round up or not
        li      a5, 1                   // li a5, 0x8000000000000000...
        sll     a5, a5, 63              // ...but faster
        sub     a2, a2, a5
        seqz    a2, a2
        ANDNx   a0, a0, a2
#else
        bgez    a2, L(no_rounding)
        add     a0, a0, 1
        sll     a2, a2, 1
        bnez    a2, L(no_rounding)      // No tie
        and     a0, a0, ~1
#endif

// Combine significand with sign+exponent.
L(no_rounding):
        sll     a3, a3, 52
        add     a0, a0, a3

// All done.
L(done):
        ret

#endif

#else

#error Bad configuration

#endif

END_FUNC __floatundidf

/*********************************************************************
*
*       __extendsfdf2()
*
*  Function description
*    Extend float to double.
*
*  Parameters
*    a0  - Float value to extend :: soft ABI.
*    fa0 - Float value to extend :: single ABI.
*
*  Return value
*    a1:a0 / a0 - Double value :: soft ABI.
*    fa0        - Double value :: double ABI.
*/

#undef L
#define L(label) .L__extendsfdf2_##label

GLOBAL_FUNC __extendsfdf2

#if __SEGGER_RTL_FP_ABI >= 2

        fcvt.d.s fa0, fa0
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

#if __SEGGER_RTL_FP_ABI == 1
        fmv.x.w a0, fa0
#endif

// Shift off sign leaving exponent in high order byte--we need to compare magnitude.
        sll     t0, a0, 1

// Flush subnormal to zero.
        li      a3, 0x01000000
        bgtu    a3, t0, L(subnormal)

// Align single precision significand to its double precision position.
        srl     a1, t0, 4

// Re-bias to new exponent.
        li      a2, 0x38000000
        add     a1, a1, a2

// Extract sign from single precision value and move to double precision placement.
        srl     a3, a0, 31
        sll     a3, a3, 31
        or      a1, a1, a3

// Move lower 4 bits of 23-bit significand to lower 4 bits of double significand.
        sll     a0, a0, 29

// Handle Inf and NaN off hot trace.
        li      a3, 0xFF000000
        bgeu    t0, a3, L(inf_nan)

// Normal is converted, done.
        ret

// Input is Inf or NaN.  Adjust to generate 64-bit Inf/NaN exponent.
L(inf_nan):
        li      a2, 0x7ff00000-0x47f00000
        add     a1, a1, a2
        beq     t0, a3, L(ret)

// Ensure it's a quiet NaN.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a1, a1, 19
#else
        li      a2, 0x00080000
        or      a1, a1, a2
#endif
L(ret):
        ret

// Zero or subnormal input: flush to signed zero.
L(subnormal):
        srl     a1, a0, 31             // Copy sign to double precision position
        sll     a1, a1, 31
        li      a0, 0                  // Zero significand low-order bits
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

#if __SEGGER_RTL_FP_ABI == 1
        fmv.x.w a0, fa0
#endif

// Shift off sign leaving exponent in high order byte--we need to compare magnitude.
        sllw    t0, a0, 1

// Extract sign to bit 63.
        srl     a4, a0, 31
        sll     a4, a4, 63

// Flush subnormal to zero.
        li      a3, 0x01000000
        bgtu    a3, t0, L(subnormal)

// Align single precision significand to its double precision position.
        sll     a0, t0, 32
        srl     a0, a0, 4

// Re-bias to new exponent.
        li      a2, 7                   // li a2, (0x7ff00000-0x47f00000) << 32...
        slli    a2, a2, 59              // ...but more efficient
        add     a0, a0, a2

// Insert sign.
        or      a0, a0, a4

// Handle Inf and NaN off hot trace.
        li      a3, 0xFFFFFFFFFF000000
        bgeu    t0, a3, L(inf_nan)

// Normal is converted, done.
        ret

// Input is Inf or NaN.  Adjust to generate 64-bit Inf/NaN exponent.
L(inf_nan):
        add     a0, a0, a2
        beq     t0, a3, L(ret)

// Ensure it's a quiet NaN.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a1, a1, 19
#else
        li      a2, 1                   // li a2, 0x0008000000000000...
        sll     a2, a2, 51              // ...but more efficient
        or      a0, a0, a2
#endif
L(ret):
        ret

// Zero or subnormal input: flush to signed zero.
L(subnormal):
        mv      a0, a4
        ret

#else

#error Bad configuration

#endif

END_FUNC __extendsfdf2

/*********************************************************************
*
*       __truncdfsf2()
*
*  Function description
*    Truncate double to float.
*
*  Parameters
*    a1:a0 / a0 - Double value to truncate.
*
*  Return value
*    a0  - Float value :: soft ABI.
*    fa0 - Float value :: single ABI.
*/

#undef L
#define L(label) .L__truncdfsf2_##label

GLOBAL_FUNC __truncdfsf2

#if __SEGGER_RTL_FP_ABI >= 2

        fcvt.s.d fa0, fa0
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Extract exponent.
        BFOZ    a3, a1, 30, 20

// Inf or Nan require special treatment.
        li      a2, 0x7FF
        beq     a3, a2, L(inf_nan)

// Compute exponent biased to single-precision.
        add     a3, a3, -0x380

// If the exponent underflowed or input is subnormal or zero, need a signed zero.
        blez    a3, L(underflow)

// If we can't represent this in single precision, overflow to Inf.
        li      a2, 0xFF
        bgeu    a3, a2, L(inf)

// Move exponent into dp position.
        sll     a3, a3, 23

// Isolate sign into high bit of a2.
        srl     a2, a1, 31
        sll     a2, a2, 31

// Copy argument's sign bit to result's sign bit.  a3 now contains
// the sign bit plus result's exponent and zero significand.
        or      a3, a3, a2

// Shift dp significand to align at bit 31 including low-order part.
        sll     a1, a1, 12
        srl     a2, a0, 20
        or      a1, a1, a2

// Move significand into position for sp result.
        srl     a1, a1, 9

// Use low-order bits to break tie in rounding.
#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5
        bfoz    a4, a0, 29, 29
        bfoz    a5, a0, 28, 28
        sll     a2, a0, 4
        add     a2, a2, a4
#else
        sll     a2, a0, 2                // Shift by two
        sltz    a4, a2                   // Compute carry before shift
        add     a2, a2, a2               // Shift by three complete with a4=carry out
        sltz    a5, a2                   // Compute carry before addition
        LEAH    a2, a4, a2               // Complete add with carry in
#endif
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        snez    a4, a2                   // Branch-free
        and     a4, a4, a5
        add     a1, a1, a4
#else
        beqz    a2, L(no_round_tie)      // Small
        add     a1, a1, a5
#endif

// Rounding done; insert exponent and return.  NOTE: "add" is required,
// "or" will not work as rounding above "overflows" to adjust the exponent!
L(no_round_tie):
        add     a0, a1, a3
#if __SEGGER_RTL_FP_ABI == 1
        fmv.w.x fa0, a0
#endif
        ret

// Can't represent the argument in single precision as it underflowed.
// Generate a correctly-signed zero from the argument in a1 into a0.
L(underflow):
        srl     a0, a1, 31
        sll     a0, a0, 31
#if __SEGGER_RTL_FP_ABI == 1
        fmv.w.x fa0, a0
#endif
        ret

// Distinguish between Inf and NaN.  We do process non-EABI NaNs where the
// low-order bits are also taken into account.
L(inf_nan):
        sll     a3, a1, 12
        or      a3, a3, a0
        bnez    a3, L(nan)

// Can't represent the argument in single precision as it overflowed or.
// the input is Inf.  Generate a correctly-signed zero from the argument
// in a1 into a0.
L(inf):
        srl     a1, a1, 31            // Extract sign to lsb of a1
        sll     a1, a1, 31            // Make room for 0xFF bits.
        li      a0, 0x7F800000        // +Inf
        or      a0, a0, a1            // Apply sign to inf
        li      a1, 0
#if __SEGGER_RTL_FP_ABI == 1
        fmv.w.x fa0, a0
#endif
        ret

// We now know the input is a NaN.  Convert the 64-bit NaN to a 32-bit NaN
// preserving the information in the significand bits.
L(nan):

// Generally useful constant 0x80000000 to manipulate sign bit.
        li      a3, 0x80000000

// Preserve sign bit of NaN in a2.
        and     a2, a1, a3              // Isolate sign bit

// Shift significand bits from dp to sp position.
        sll     a1, a1, 3               // Move significand digits
        srl     a0, a0, 32-3            // Wedge in lower bits
        or      a0, a0, a1

// Copy original sign of NaN to sp position.
        BFOZ    a0, a0, 30, 0           // Clear result sign bit
        or      a0, a0, a2              // Copy in sign bit

// Ensure that the NaN indicator is set.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 22
#else
        srl     a3, a3, 9               // a3=0x00400000 == 1<<22
        or      a0, a0, a3
#endif

#if __SEGGER_RTL_FP_ABI == 1
        fmv.w.x fa0, a0
#endif
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Extract exponent.
        sll     a3, a0, 1
        srl     a3, a3, 53

// Inf or Nan require special treatment.
        li      a2, 0x7FF
        beq     a3, a2, L(inf_nan)

// Compute exponent biased to single-precision.
        add     a3, a3, -0x380

// If the exponent underflowed or input is subnormal or zero, need a signed zero.
        blez    a3, L(underflow)

// If we can't represent this in single precision, overflow to Inf.
        li      a2, 0xFF
        bgeu    a3, a2, L(inf)

// Move exponent into sp position.
        sll     a3, a3, 23

// Isolate sign into high bit of a2.
        sltz    a2, a0
        sll     a2, a2, 31

// Copy argument's sign bit to result's sign bit.  a3 now contains
// the sign bit plus result's exponent and zero significand.
        or      a3, a3, a2

// Move dp significand into sp position in a1.
        sll     a1, a0, 12
        srl     a1, a1, 32+9

// Use low-order bits to break tie in rounding.
#if __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
        sll     a0, a0, 35              // Discarded bits aligned to high-order word
        sltz    a5, a0                  // Isolate most significant discarded bit
        add     a1, a1, a5              // Round up or not
        li      a5, 1                   // li a5, 0x8000000000000000...
        sll     a5, a5, 63              // ...but faster
        sub     a0, a0, a5
        seqz    a0, a0
        ANDNx   a1, a1, a0
#else
        sll     a0, a0, 35
        bgez    a0, L(no_rounding)
        add     a1, a1, 1
        sll     a0, a0, 1
        bnez    a0, L(no_rounding)      // No tie
        and     a1, a1, ~1
#endif

// Rounding done; insert exponent and return.  NOTE: "add" is required,
// "or" will not work as rounding above "overflows" to adjust the exponent!
L(no_rounding):
        add     a0, a1, a3
#if __SEGGER_RTL_FP_ABI == 1
        fmv.w.x fa0, a0
#endif
        ret

// Can't represent the argument in single precision as it underflowed.
// Generate a correctly-signed zero from the argument in a1 into a0.
L(underflow):
        srl     a0, a0, 63
        sll     a0, a0, 31
#if __SEGGER_RTL_FP_ABI == 1
        fmv.w.x fa0, a0
#endif
        ret

// Distinguish between Inf and NaN.  We do process non-EABI NaNs where the
// low-order bits are also taken into account.
L(inf_nan):
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller
        sll     a3, a0, 1
        bgtu    a3, a2, L(nan)

// Can't represent the argument in single precision as it overflowed or.
// the input is Inf.  Generate a correctly-signed zero from the argument
// in a1 into a0.
L(inf):
        srl     a1, a0, 63            // Extract sign to lsb of a1
        sll     a1, a1, 31            // Make room for 0xFF bits.
        li      a0, 0x7F800000        // +Inf
        or      a0, a0, a1            // Apply sign to inf
#if __SEGGER_RTL_FP_ABI == 1
        fmv.w.x fa0, a0
#endif
        ret

// We now know the input is a NaN.  Convert the 64-bit NaN to a 32-bit NaN
// preserving the information in the significand bits.
L(nan):

// Preserve sign bit of NaN in a3.
        srl     a3, a0, 63
        sll     a3, a3, 31

// Shift significand bits from dp to sp position.
        sll     a0, a0, 4               // Move significand digits
        srl     a0, a0, 33

// Insert sign.
        or      a0, a0, a3

// Ensure that the NaN indicator is set.
#if __SEGGER_RTL_CORE_HAS_BSET_BCLR_BINV_BEXT
        bseti   a0, a0, 22
#else
        li      a1, 1<<22
        or      a0, a0, a1
#endif

#if __SEGGER_RTL_FP_ABI == 1
        fmv.w.x fa0, a0
#endif
        ret

#else

#error Bad configuration

#endif

END_FUNC __truncdfsf2

/*********************************************************************
*
*       __unordsf2()
*
*  Function description
*    Unordered test, float.
*
*  Parameters
*    a0 - Argument #1.
*    a1 - Argument #2.
*
*  Return value
*    a0 - 1 if unordered, 0 if ordered.
*/

GLOBAL_FUNC __unordsf2

#if __SEGGER_RTL_FP_ABI >= 1

        fclass.s a0, fa0
        fclass.s a1, fa1
        or      a0, a0, a1
        andi    a0, a0, __SEGGER_RTL_RV_QNAN | __SEGGER_RTL_RV_SNAN
        snez    a0, a0
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Set up Inf/NaN value (shifted).
        li      a2, 0xFF000000

// Generate flags per operand.
        sll     a0, a0, 1
        sgtu    a0, a0, a2
        sll     a1, a1, 1
        sgtu    a1, a1, a2

// Combine.
        or      a0, a0, a1
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Set up Inf/NaN value (shifted).
        li      a2, 0xFFFFFFFFFF000000

// Generate flags per operand.
        sllw    a0, a0, 1
        sgtu    a0, a0, a2
        sllw    a1, a1, 1
        sgtu    a1, a1, a2

// Combine.
        or      a0, a0, a1
        ret

#else

#error Bad configuration

#endif

END_FUNC __unordsf2

/*********************************************************************
*
*       __unorddf2()
*
*  Function description
*    Unordered test, double.
*
*  Parameters
*    a0 - Argument #1.
*    a1 - Argument #2.
*
*  Return value
*    a0 - 1 if unordered, 0 if ordered.
*/

GLOBAL_FUNC __unorddf2

#if __SEGGER_RTL_FP_ABI >= 2

        fclass.d a0, fa0
        fclass.d a1, fa1
        or      a0, a0, a1
        andi    a0, a0, __SEGGER_RTL_RV_QNAN | __SEGGER_RTL_RV_SNAN
        snez    a0, a0
        ret

#elif __SEGGER_RTL_TYPESET == 32

//
// RV32
//

// Set up Inf/NaN value, shifted.
        li      a5, 0xFFE00000

// Compress significand digits and lose sign bit.
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        snez    a0, a0                  // Fold low-order NaN indicator into high order...
        LEAH    a1, a0, a1              // ...and discard sign
#else
        sll     a1, a1, 1
#endif
        sgtu    a0, a1, a5              // True if NaN.

// Compress significand digits and lose sign bit.
#if __SEGGER_RTL_NAN_FORMAT == __SEGGER_RTL_NAN_FORMAT_IEEE
        snez    a2, a2                  // Fold low-order NaN indicator into high order...
        LEAH    a3, a2, a3              // ...and discard sign
#else
        sll     a3, a3, 1
#endif
        sgtu    a2, a3, a5              // True if NaN.

// Combine.
        or      a0, a0, a2
        ret

#elif __SEGGER_RTL_TYPESET == 64

//
// RV64
//

// Set up Inf/NaN value (shifted).
        li      a2, -1                  // li a2, 0xFFE0000000000000...
        slli    a2, a2, 53              // ...but smaller

// Generate flags per operand.
        sll     a0, a0, 1
        sgtu    a0, a0, a2
        sll     a1, a1, 1
        sgtu    a1, a1, a2

// Combine.
        or      a0, a0, a1
        ret

#else

#error Bad configuration

#endif

END_FUNC __unorddf2

/*********************************************************************
*
*       Local data
*
**********************************************************************
*/

// 2^64

.if __SEGGER_RTL_2pow64_REQUIRED
LOCAL_RODATA __SEGGER_RTL_2pow64, 8
       .word    0x00000000
       .word    0x43F00000
END_RODATA __SEGGER_RTL_2pow64
.endif

// 2^32

.if __SEGGER_RTL_2pow32_REQUIRED
LOCAL_RODATA __SEGGER_RTL_2pow32, 8
       .word    0x00000000
       .word    0x41F00000
END_RODATA __SEGGER_RTL_2pow32
.endif

// 2^-32

.if __SEGGER_RTL_2powNeg32_REQUIRED
LOCAL_RODATA __SEGGER_RTL_2powNeg32, 8
       .word    0x00000000
       .word    0x3DF00000
END_RODATA __SEGGER_RTL_2powNeg32
.endif

// Reciprocal table for single divide.

.if __SEGGER_RTL_fdiv_reciprocal_table_REQUIRED
LOCAL_RODATA __SEGGER_RTL_fdiv_reciprocal_table, 4

// 6 bits in, 32 bits out
       .word    0x820FE040, 0x860FA233, 0x890F6604, 0x8D0F2B9D
       .word    0x900EF2EB, 0x930EBBDB, 0x970E865B, 0x990E525A
       .word    0x9C0E1FC8, 0x9F0DEE96, 0xA20DBEB6, 0xA40D901B
       .word    0xA60D62B8, 0xA90D3681, 0xAB0D0B6A, 0xAD0CE169
       .word    0xAF0CB872, 0xB10C907E, 0xB30C6981, 0xB50C4373
       .word    0xB70C1E4C, 0xB80BFA03, 0xBA0BD691, 0xBC0BB3EE
       .word    0xBD0B9214, 0xBF0B70FC, 0xC00B509E, 0xC10B30F6
       .word    0xC30B11FD, 0xC40AF3AE, 0xC50AD603, 0xC70AB8F7
       .word    0xC80A9C85, 0xC90A80A8, 0xCA0A655C, 0xCB0A4A9D
       .word    0xCC0A3066, 0xCD0A16B3, 0xCE09FD81, 0xCF09E4CB
       .word    0xD009CC8E, 0xD109B4C7, 0xD2099D72, 0xD309868D
       .word    0xD3097013, 0xD4095A02, 0xD5094458, 0xD6092F11
       .word    0xD7091A2B, 0xD70905A4, 0xD808F178, 0xD908DDA5
       .word    0xD908CA2A, 0xDA08B703, 0xDB08A430, 0xDB0891AC
       .word    0xDC087F78, 0xDC086D90, 0xDD085BF3, 0xDE084AA0
       .word    0xDE083993, 0xDF0828CC, 0xDF081849, 0xE0080808

END_RODATA __SEGGER_RTL_fdiv_reciprocal_table
.endif

// Bipartite reciprocal tables for double divide.

.if __SEGGER_RTL_ddiv_reciprocal_table_REQUIRED
LOCAL_RODATA __SEGGER_RTL_ddiv_reciprocal_table, 2

// 7-bits-in, 16-bits out table TH, each entry is floor(2^16/(1.<b1..b7> + 2^-7))
       .hword   0xFE00, 0xFC0C, 0xFA20, 0xF83C, 0xF660, 0xF488, 0xF2B8, 0xF0F0
       .hword   0xEF2C, 0xED70, 0xEBBC, 0xEA0C, 0xE864, 0xE6C0, 0xE524, 0xE38C
       .hword   0xE1FC, 0xE070, 0xDEE8, 0xDD64, 0xDBE8, 0xDA74, 0xD900, 0xD794
       .hword   0xD628, 0xD4C4, 0xD368, 0xD20C, 0xD0B4, 0xCF64, 0xCE14, 0xCCCC
       .hword   0xCB84, 0xCA44, 0xC904, 0xC7CC, 0xC698, 0xC564, 0xC434, 0xC30C
       .hword   0xC1E4, 0xC0C0, 0xBFA0, 0xBE80, 0xBD68, 0xBC50, 0xBB3C, 0xBA2C
       .hword   0xB920, 0xB814, 0xB70C, 0xB608, 0xB508, 0xB408, 0xB30C, 0xB214
       .hword   0xB11C, 0xB02C, 0xAF38, 0xAE4C, 0xAD60, 0xAC74, 0xAB8C, 0xAAA8
       .hword   0xA9C8, 0xA8E8, 0xA808, 0xA72C, 0xA654, 0xA57C, 0xA4A8, 0xA3D4
       .hword   0xA304, 0xA234, 0xA168, 0xA0A0, 0x9FD8, 0x9F10, 0x9E4C, 0x9D88
       .hword   0x9CC8, 0x9C08, 0x9B4C, 0x9A90, 0x99D4, 0x991C, 0x9868, 0x97B4
       .hword   0x9700, 0x964C, 0x95A0, 0x94F0, 0x9444, 0x9398, 0x92F0, 0x9248
       .hword   0x91A0, 0x90FC, 0x9058, 0x8FB8, 0x8F14, 0x8E78, 0x8DD8, 0x8D3C
       .hword   0x8CA0, 0x8C08, 0x8B70, 0x8AD8, 0x8A40, 0x89AC, 0x8918, 0x8888
       .hword   0x87F4, 0x8764, 0x86D8, 0x8648, 0x85BC, 0x8534, 0x84A8, 0x8420
       .hword   0x8398, 0x8310, 0x828C, 0x8208, 0x8184, 0x8100, 0x8080, 0x8000
       
// 7-bits-in, 8-bits out table TL, each entry is floor(2^8/(1.<b1..b7> + 2^-7)^2)
       .byte    0xFC, 0xF8, 0xF4, 0xF0, 0xEC, 0xE8, 0xE6, 0xE2, 0xDE, 0xDC, 0xD8, 0xD4, 0xD2, 0xD0, 0xCC, 0xCA
       .byte    0xC6, 0xC4, 0xC2, 0xBE, 0xBC, 0xBA, 0xB6, 0xB4, 0xB2, 0xB0, 0xAE, 0xAC, 0xAA, 0xA8, 0xA4, 0xA2
       .byte    0xA0, 0x9E, 0x9C, 0x9A, 0x9A, 0x98, 0x96, 0x94, 0x92, 0x90, 0x8E, 0x8C, 0x8C, 0x8A, 0x88, 0x86
       .byte    0x84, 0x84, 0x82, 0x80, 0x80, 0x7E, 0x7C, 0x7A, 0x7A, 0x78, 0x76, 0x76, 0x74, 0x74, 0x72, 0x70
       .byte    0x70, 0x6E, 0x6E, 0x6C, 0x6C, 0x6A, 0x68, 0x68, 0x66, 0x66, 0x64, 0x64, 0x62, 0x62, 0x60, 0x60
       .byte    0x60, 0x5E, 0x5E, 0x5C, 0x5C, 0x5A, 0x5A, 0x58, 0x58, 0x58, 0x56, 0x56, 0x54, 0x54, 0x54, 0x52
       .byte    0x52, 0x52, 0x50, 0x50, 0x4E, 0x4E, 0x4E, 0x4C, 0x4C, 0x4C, 0x4A, 0x4A, 0x4A, 0x4A, 0x48, 0x48
       .byte    0x48, 0x46, 0x46, 0x46, 0x44, 0x44, 0x44, 0x44, 0x42, 0x42, 0x42, 0x42, 0x40, 0x40, 0x40, 0x40

END_RODATA __SEGGER_RTL_ddiv_reciprocal_table
.endif

#endif

/*************************** End of file ****************************/
