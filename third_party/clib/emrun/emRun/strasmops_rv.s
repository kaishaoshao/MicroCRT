/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#include "asmdefs_rv.ah"
#include "__SEGGER_RTL_ConfDefaults.h"

#if __SEGGER_RTL_STRING_ASM

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       memset()
*
*  Function description
*    Set memory to character.
*
*  Parameters
*    a0 - Pointer to destination object.
*    a1 - Character to copy.
*    a2 - Length of destination object in characters.
*
*  Return value
*    Returns s.
*
*  Additional information
*    Copies the value of c (converted to an unsigned char) 
*    into each of the first n characters of the object pointed to by s.
*
*  Thread safety
*    Safe.
*/

#undef L
#define L(label) .Lmemset_##label

GLOBAL_FUNC memset

#if __SEGGER_RTL_OPTIMIZE <= -2

        beqz    a2, L(memset_end)
        mv      a3, a0                  // Working address
        add     a2, a2, a0              // One byte past end of destination object
L(memset_loop):
        sb      a1, (a3)
        addi    a3, a3, 1
        bne     a3, a2, L(memset_loop)

L(memset_end):
        ret

#else

        mv      a4, a0                  // Copy initial destination pointer for result.
        beqz    a2, L(memset_end)

L(unaligned_byte_set_loop):
        slli    a3, a0, 30
        beqz    a3, L(fast_set)

        sb      a1, (a0)
        addi    a0, a0, 1
        addi    a2, a2, -1
        bnez    a2, L(unaligned_byte_set_loop)
        j       L(memset_end)

L(fast_set):
        andi    a1, a1, 0xFF
        slli    a3, a1, 8
        or      a1, a1, a3
        slli    a3, a1, 16
        or      a1, a1, a3

        li      a3, 32
        bltu    a2, a3, L(word_set)
L(fast_set_loop):
        sw      a1, (a0)
        sw      a1, 4(a0)
        sw      a1, 8(a0)
        sw      a1, 12(a0)
        sw      a1, 16(a0)
        sw      a1, 20(a0)
        sw      a1, 24(a0)
        sw      a1, 28(a0)
        add     a0, a0, a3
        sub     a2, a2, a3
        bgeu    a2, a3, L(fast_set_loop)
        beqz    a2, L(memset_end)

L(word_set):
        li      a3, 4
        bltu    a2, a3, L(byte_set_loop)

L(word_set_loop):
        sw      a1, (a0)
        add     a0, a0, a3
        sub     a2, a2, a3
        bgeu    a2, a3, L(word_set_loop)
        beqz    a2, L(memset_end)

L(byte_set_loop):
        sb      a1, (a0)
        addi    a0, a0, 1
        addi    a2, a2, -1
        bnez    a2, L(byte_set_loop)

L(memset_end):
        mv      a0, a4
        ret
#endif

END_FUNC memset

/*********************************************************************
*
*       memcpy()
*
*  Function description
*    Copy memory.
*
*  Parameters
*    a0 - Pointer to destination object.
*    a1 - Pointer to source object.
*    a2 - Number of characters to copy.
*
*  Return value
*    Returns a pointer to the destination object.
*
*  Additional information
*    Copies n characters from the object pointed to by s2 into the
*    object pointed to by s1. The behavior of memcpy() is undefined
*    if copying takes place between objects that overlap.
*/

#undef L
#define L(label) .Lmemcpy_##label

GLOBAL_FUNC memcpy

        beqz    a2, L(done)

// Copy initial destination pointer for result.
        mv      a5, a0

#if __SEGGER_RTL_OPTIMIZE >= 0

// If pointers can never be aligned, carry out a simple byte copy.
        xor     a3, a0, a1
        slli    a3, a3, 30
        bnez    a3, L(byte_copy)

// Copy until byte pointers are aligned.
        sll     a3, a0, 30
        beqz    a3, L(aligned)
L(word_align):
        lb      a3, (a1)
        sb      a3, (a0)
        addi    a1, a1, 1
        addi    a0, a0, 1
        addi    a2, a2, -1
        beqz    a2, L(memcpy_end)
        slli    a3, a0, 30
        bnez    a3, L(word_align)

// If there aren't enough bytes for one block copy, carry out a word copy
L(aligned):
        li      a3, 32
        bltu    a2, a3, L(word_copy)

// Carry out aligned block copy
L(aligned_block_copy_loop):
        lw      a4, (a1)
        sw      a4, (a0)
        lw      a4, 4(a1)
        sw      a4, 4(a0)
        lw      a4, 8(a1)
        sw      a4, 8(a0)
        lw      a4, 12(a1)
        sw      a4, 12(a0)
        lw      a4, 16(a1)
        sw      a4, 16(a0)
        lw      a4, 20(a1)
        sw      a4, 20(a0)
        lw      a4, 24(a1)
        sw      a4, 24(a0)
        lw      a4, 28(a1)
        sw      a4, 28(a0)
        add     a0, a0, a3
        add     a1, a1, a3
        sub     a2, a2, a3
        bgeu    a2, a3, L(aligned_block_copy_loop)
  
// Carry out word copy
L(word_copy):
        beqz    a2, L(memcpy_end)
        li      a3, 4
        bltu    a2, a3, L(byte_copy)

L(word_copy_loop):
        lw      a4, (a1)
        sw      a4, (a0)
        add     a0, a0, a3
        add     a1, a1, a3
        sub     a2, a2, a3
        bgeu    a2, a3, L(word_copy_loop)

#endif

L(byte_copy):
        beqz    a2, L(memcpy_end)

L(byte_copy_loop):
        lb      a4, (a1)
        sb      a4, (a0)
        addi    a1, a1, 1
        addi    a0, a0, 1
        addi    a2, a2, -1
        bnez    a2, L(byte_copy_loop)

L(memcpy_end):
        mv      a0, a5
L(done):
        ret

END_FUNC memcpy

/*********************************************************************
*
*       strcpy()
*
*  Function description
*    String copy.
*
*  Parameters
*    a0 - String to copy to.
*    a1 - String to copy.
*
*  Return value
*    Returns the value of a0.
*
*  Additional information
*    Copies the string pointed to by a1 (including the terminating 
*    null character) into the array pointed to by a0. The behavior of strcpy()
*    is undefined if copying takes place between objects that overlap.
*/

#undef L
#define L(label) .Lstrcpy_##label

GLOBAL_FUNC strcpy

#if __SEGGER_RTL_OPTIMIZE < 0

        mv      a2, a0

L(bytestrcpy):
        lb      a3, (a1)
        addi    a1, a1, 1
        sb      a3, (a2)
        addi    a2, a2, 1
        bnez    a3, L(bytestrcpy)

        ret

#elif __SEGGER_RTL_CORE_HAS_ISA_SIMD

//
// If the core has the P extension, use the DSP instructions to
// accelerate finding a zero byte in a 32-bit word.
//
        mv      a2, a0

        or      a3, a0, a1
        andi    a3, a3, 3
        bnez    a3, L(bytestrcpy)
        
L(wordstrcpy):
        lw      a4, (a1)
        ucmple8 a3, a4, zero
        bnez    a3, L(bytestrcpy)
        sw      a4, (a2)

        lw      a4, 4(a1)
        ucmple8 a3, a4, zero
        bnez    a3, L(bytestrcpy_4)
        sw      a4, 4(a2)

        lw      a4, 8(a1)
        ucmple8 a3, a4, zero
        bnez    a3, L(bytestrcpy_8)
        sw      a4, 8(a2)

        lw      a4, 12(a1)
        ucmple8 a3, a4, zero
        bnez    a3, L(bytestrcpy_12)
        sw      a4, 12(a2)

        addi    a1, a1, 16
        addi    a2, a2, 16
        j       L(wordstrcpy)

L(bytestrcpy_12):
        addi    a2, a2, 4

L(bytestrcpy_8):
        addi    a2, a2, 4

L(bytestrcpy_4):
        addi    a2, a2, 4

// Form bitmask
        bitrevi a3, a3, 31
        clz32   a5, a3
        bitrevi a3, a3, 31
        neg     a5, a5
        add     a5, a5, 31
        sll     a3, a3, a5
        sra     a3, a3, a5
        sll     a3, a3, 8

        lw      a5, 0(a2)               // Get word at destination
        and     a5, a5, a3              // Set bytes that must be altered to zero
        not     a3, a3                  // Flip mask
        and     a4, a4, a3              // Set bytes that must not be altered to zero
        or      a4, a4, a5              // Combine
        sw      a4, 0(a2)               // Store modified bytes
        ret

#if 0 // alternate implementation

        and     a3, a4, 0xFF
        sb      a3, 0(a2)
        beqz    a3, L(done)

        srl     a3, a4, 8
        sb      a3, 1(a2)
        and     a3, a3, 0xFF
        beqz    a3, L(done)

        srl     a3, a4, 16
        sb      a3, 2(a2)
        and     a3, a3, 0xFF
        beqz    a3, L(done)

        sb      zero, 3(a2)
L(done):
        ret

#endif

L(bytestrcpy):
        lb      a3, (a1)
        addi    a1, a1, 1
        sb      a3, (a2)
        addi    a2, a2, 1
        bnez    a3, L(bytestrcpy)

        ret

#else

        mv      a2, a0

#if __SEGGER_RTL_OPTIMIZE >= 0
        andi    a3, a0, 3
        bnez    a3, L(bytestrcpy)
        andi    a3, a1, 3
        bnez    a3, L(bytestrcpy)
        
        li      t0, -0x01010101
        li      a3, 0x80808080

L(wordstrcpy):
        lw      a4, (a1)

        xori    a5, a4, -1
        add     t1, a4, t0
        and     a5, a5, t1
        and     a5, a5, a3
        bnez    a5, L(bytestrcpy)

        sw      a4, (a2)
        addi    a1, a1, 4
        addi    a2, a2, 4
        j       L(wordstrcpy)
#endif

L(bytestrcpy):
        lb      a3, (a1)
        addi    a1, a1, 1
        sb      a3, (a2)
        addi    a2, a2, 1
        bnez    a3, L(bytestrcpy)

        ret
#endif

END_FUNC strcpy

#undef L
#define L(label) .Lstrlen_##label

/*********************************************************************
*
*       strlen()
*
*  Function description
*    Calculate length of string.
*
*  Parameters
*    a0 - Pointer to zero-terminated string.
*
*  Return value
*    a0 - Length of the string.
*/
GLOBAL_FUNC strlen

#if __SEGGER_RTL_OPTIMIZE < 0

// Universal code when optimizing balanced or for size.
        add     a1, a0, 1
L(compare):
        lbu     a2, (a0)
        add     a0, a0, 1
        bnez    a2, L(compare)
        sub     a0, a0, a1
        ret

#else

// Record starting pointer position.
        mv      a1, a0

// Iteratively work towards pointer alignment.
        and     a3, a0, 3
        beqz    a3, L(aligned)
        lbu     a2, (a0)
        beqz    a2, L(done)

        add     a0, a0, 1
        and     a3, a0, 3
        beqz    a3, L(aligned)
        lbu     a2, (a0)
        beqz    a2, L(done)

        add     a0, a0, 1
        and     a3, a0, 3
        beqz    a3, L(aligned)
        lbu     a2, (a0)
        beqz    a2, L(done)

        add     a0, a0, 1

#if __SEGGER_RTL_CORE_HAS_ISA_ANDES_V5

L(aligned):
        lw      a4, (a0)
        ffb     a5, a4, zero
        add     a0, a0, 4
        beqz    a5, L(aligned)
        add     a0, a0, a5
L(done):
        sub     a0, a0, a1
        ret

#else

#if __SEGGER_RTL_CORE_HAS_ISA_SIMD

L(aligned):
        addi    a0, a0, -16

L(wordstrlen):
        addi    a0, a0, 16

        lw      a4, (a0)
        ucmple8 a5, a4, zero
        bnez    a5, L(tail)

        lw      a4, 4(a0)
        ucmple8 a5, a4, zero
        bnez    a5, L(bytestrlen_4)

        lw      a4, 8(a0)
        ucmple8 a5, a4, zero
        bnez    a5, L(bytestrlen_8)

        lw      a4, 12(a0)
        ucmple8 a5, a4, zero
        beqz    a5, L(wordstrlen)

        addi    a0, a0, 4
L(bytestrlen_8):
        addi    a0, a0, 4
L(bytestrlen_4):
        addi    a0, a0, 4

L(tail):
#if __SEGGER_RTL_BYTE_ORDER < 0
        bitrevi a5, a5, 31
#endif
        clz32   a5, a5
        srl     a5, a5, 3
        add     a0, a0, a5
        sub     a0, a0, a1
        ret

#else

#if __SEGGER_RTL_OPTIMIZE <= 0

L(aligned):
        li      a2, 0x01010101
        sll     a3, a2, 7

L(wordstrlen):
        lw      a4, (a0)
        add     a0, a0, 4
        sub     a5, a4, a2
        not     a4, a4
        and     a5, a5, a4
        and     a5, a5, a3
        beqz    a5, L(wordstrlen)
        add     a0, a0, -4

#else

L(aligned):
        li      a2, 0x01010101
        sll     a3, a2, 7

L(wordstrlen):
        lw      a4, (a0)
        sub     a5, a4, a2
        not     a4, a4
        and     a5, a5, a4
        and     a5, a5, a3
        bnez    a5, L(contain0_1)

        lw      a4, 4(a0)
        sub     a5, a4, a2
        not     a4, a4
        and     a5, a5, a4
        and     a5, a5, a3
        bnez    a5, L(contain0_2)

        lw      a4, 8(a0)
        sub     a5, a4, a2
        not     a4, a4
        and     a5, a5, a4
        and     a5, a5, a3
        bnez    a5, L(contain0_3)

        lw      a4, 12(a0)
        sub     a5, a4, a2
        not     a4, a4
        and     a5, a5, a4
        and     a5, a5, a3
        add     a0, a0, 16
        beqz    a5, L(wordstrlen)

        add     a0, a0, -16+4

L(contain0_3):
        add     a0, a0, 4

L(contain0_2):
        add     a0, a0, 4

L(contain0_1):

#endif

#endif

// a5 now contains nonzero bytes (actually, 0x80) where zero bytes are in the string
#if __SEGGER_RTL_BYTE_ORDER > 0
  #error big-endian mode not supported
#endif
        sll     a4, a5, 24
        bnez    a4, L(done)
        addi    a0, a0, 1
        sll     a4, a5, 16
        bnez    a4, L(done)
        addi    a0, a0, 1
        sll     a4, a5, 8
        bnez    a4, L(done)
        addi    a0, a0, 1
L(done):
        sub     a0, a0, a1
        ret

#endif

#endif

END_FUNC strlen

#undef L
#define L(label) .Lstrchr_##label

/*********************************************************************
*
*       strchr()
*
*  Function description
*    Find character within string, forward.
*
*  Parameters
*    s - a0 - String to search.
*    c - a1 - Character to search for.
*
*  Return value
*    Returns a pointer to the located character, or a null pointer 
*    if c does not occur in the string.
*
*  Additional information
*    Locates the first occurrence of c (converted to a char)
*    in the string pointed to by s. The terminating null character 
*    is considered to be part of the string.
*/
GLOBAL_FUNC strchr

#if __SEGGER_RTL_OPTIMIZE < 0

        and     a1, a1, 0xFF
        addi    a0, a0, -1
L(search):
        add     a0, a0, 1
        lbu     a2, (a0)
        beq     a2, a1, L(found)
        bnez    a2, L(search)
        li      a0, 0
L(found):
        ret

#elif __SEGGER_RTL_CORE_HAS_ISA_SIMD

// Head loop, try to align and also check for string.
        andi    a1, a1, 255
L(search):
        andi    a2, a0, 3
        beqz    a2, L(aligned)
        lbu     a2, (a0)
        addi    a0, a0, 1
        beqz    a2, L(notfound)
        bne     a2, a1, L(search)
        addi    a0, a0, -1
        ret

L(aligned):
        sll     a3, a1, 8
        or      a3, a3, a1
        sll     a1, a3, 16
        or      a1, a1, a3

L(wordsearch):
        lw      a4, (a0)                // Load string 4 bytes at a time
        addi    a0, a0, 4
        ucmple8 a3, a4, zero            // Test for terminating zero in loaded word
        xor     a2, a4, a1              // Test for matching character in loaded word
        ucmple8 a2, a2, zero
        bnez    a3, L(bytesearch)
        beqz    a2, L(wordsearch)

L(bytesearch):
        addi    a0, a0, -4
        and     a1, a1, 0xFF
        or      a2, a2, a3              // Combine, now have bytes set if different or match
#if __SEGGER_RTL_BYTE_ORDER < 0
        bitrevi a2, a2, 31              // Get bytes in right order for CLZ
#endif
        clz32   a5, a2                  // Find bit index of first zero or match
        srl     a4, a4, a5              // Extract byte
        and     a4, a4, 0xFF
        bne     a4, a1, L(notfound)
        srl     a5, a5, 3               // Form byte index of match
        add     a0, a0, a5
        ret

L(notfound):
        li      a0, 0
        ret

#else

// Head loop, try to align and also check for string.
        andi    a1, a1, 255
L(search):
        andi    a2, a0, 3
        beqz    a2, L(aligned)
        lbu     a2, (a0)
        addi    a0, a0, 1
        beq     a2, a1, L(found)
        bnez    a2, L(search)
        li      a0, 0
        ret

L(found):
        addi    a0, a0, -1
        ret

L(aligned):
        li      a2, 0x01010101
#ifdef __riscv_mul
        mul     a1, a1, a2
#else
        sll     a3, a1, 8
        or      a3, a3, a1
        sll     a1, a3, 16
        or      a1, a1, a3
#endif
        sll     a3, a2, 7

L(wordsearch):
        lw      a4, (a0)                // Load string 4 bytes at a time
        addi    a0, a0, 4
        not     t0, a4
        sub     a5, a4, a2              // Test to see if this contains a null, in which case we need to examine byte by byte
        and     a5, a5, t0
        xor     a4, a4, a1              // Generate zero bytes where candidates match
        sub     t0, a4, a2              // Test to see if this contains a null, in which case we need to examine byte by byte
        not     a4, a4
        and     t0, t0, a4
        or      t0, t0, a5
        and     t0, t0, a3

#if __SEGGER_RTL_OPTIMIZE <= 0
        beqz    t0, L(wordsearch)       // Didn't match any bytes
#else
        bnez    t0, L(bytesearch)       // Didn't match any bytes
        lw      a4, (a0)                // Load string 4 bytes at a time
        addi    a0, a0, 4
        not     t0, a4
        sub     a5, a4, a2              // Test to see if this contains a null, in which case we need to examine byte by byte
        and     a5, a5, t0
        xor     a4, a4, a1              // Generate zero bytes where candidates match
        sub     t0, a4, a2              // Test to see if this contains a null, in which case we need to examine byte by byte
        not     a4, a4
        and     t0, t0, a4
        or      t0, t0, a5
        and     t0, t0, a3
        beqz    t0, L(wordsearch)       // Didn't match any bytes
#endif

L(bytesearch):
        addi    a0, a0, -4
        andi    a1, a1, 0xFF
L(tailsearch):
        lbu     a2, (a0)
        add     a0, a0, 1
        beq     a2, a1, L(tailfound)
        bnez    a2, L(tailsearch)
        li      a0, 0
        ret
L(tailfound):
        addi    a0, a0, -1
        ret

#endif

END_FUNC strchr

/*********************************************************************
*
*       strcmp()
*
*  Function description
*    Compare strings.
*
*  Parameters
*    a0 - Pointer to string #1.
*    a1 - Pointer to string #2.
*
*  Return value
*    Returns an integer greater than, equal to, or less than zero, 
*    if the null-terminated array pointed to by a0 is greater than, 
*    equal to, or less than the null-terminated array pointed to by a1.
*/

#undef L
#define L(label) .Lstrcmp_##label

GLOBAL_FUNC strcmp

#if __SEGGER_RTL_OPTIMIZE < 0

L(unaligned):
        lbu     a2, (a0)
        addi    a0, a0, 1
        lbu     a3, (a1)
        addi    a1, a1, 1
        bne     a2, a3, L(ret)
        bnez    a2, L(unaligned)

L(ret):
        sub     a0, a2, a3
        ret

#elif __SEGGER_RTL_CORE_HAS_ISA_SIMD

        or      a2, a0, a1
        andi    a2, a2, 3
        bnez    a2, L(unaligned)
        
L(wordcmp):
        lw      a4, (a0)
        lw      a5, (a1)
        bne     a4, a5, L(diff)
        ucmple8 a2, a4, zero
        bnez    a2, L(diff)
        ucmple8 a2, a5, zero
#if __SEGGER_RTL_OPTIMIZE <= 0
        addi    a0, a0, 4
        addi    a1, a1, 4
        beqz    a2, L(wordcmp)
#else
        bnez    a2, L(diff)
        
        lw      a4, 4(a0)
        lw      a5, 4(a1)
        bne     a4, a5, L(diff)
        ucmple8 a2, a4, zero
        bnez    a2, L(diff)
        ucmple8 a2, a5, zero
        bnez    a2, L(diff)

        lw      a4, 8(a0)
        lw      a5, 8(a1)
        bne     a4, a5, L(diff)
        ucmple8 a2, a4, zero
        bnez    a2, L(diff)
        ucmple8 a2, a5, zero
        bnez    a2, L(diff)

        lw      a4, 12(a0)
        lw      a5, 12(a1)
        bne     a4, a5, L(diff)
        ucmple8 a2, a4, zero
        bnez    a2, L(diff)
        ucmple8 a2, a5, zero
        addi    a0, a0, 16
        addi    a1, a1, 16
        beqz    a2, L(wordcmp)

#endif

L(diff):

#if __SEGGER_RTL_BYTE_ORDER > 0
        bitrevi a4, a4, 31
        bitrevi a5, a5, 31
#endif
        sll     a2, a4, 24
        sll     a3, a5, 24
        srl     a0, a2, 24
        bne     a2, a3, L(result)
        beqz    a0, L(result)

        sll     a2, a4, 16
        sll     a3, a5, 16
        srl     a0, a2, 24
        bne     a2, a3, L(result)
        beqz    a0, L(result)

        sll     a2, a4, 8
        sll     a3, a5, 8
        srl     a0, a2, 24
        bne     a2, a3, L(result)
        beqz    a0, L(result)

        mv      a2, a4
        mv      a3, a5
        srl     a0, a2, 24

L(result):
        srl     a3, a3, 24
        sub     a0, a0, a3
        ret

L(unaligned):
        lbu     a2, (a0)
        addi    a0, a0, 1
        lbu     a3, (a1)
        addi    a1, a1, 1
        bne     a2, a3, L(ret)
        bnez    a2, L(unaligned)

L(ret):
        sub     a0, a2, a3
        ret

#else
        or      a2, a0, a1
        andi    a2, a2, 3
        bnez    a2, L(unaligned)
        
        li      t0, -0x01010101
        li      a3,  0x80808080

L(wordcmp):
        lw      a4, (a0)
        lw      a5, (a1)
        bne     a4, a5, L(unaligned)
        add     a4, a4, t0
        not     a5, a5
        and     a5, a5, a4
        and     a5, a5, a3
#if __SEGGER_RTL_OPTIMIZE <= 0
        addi    a0, a0, 4
        addi    a1, a1, 4
        beqz    a5, L(wordcmp)
        addi    a0, a0, -4
        addi    a1, a1, -4
#else
        bnez    a5, L(unaligned)

        lw      a4, 4(a0)
        lw      a5, 4(a1)
        bne     a4, a5, L(exit_4)
        add     a4, a4, t0
        not     a5, a5
        and     a5, a5, a4
        and     a5, a5, a3
        bnez    a5, L(exit_4)

        lw      a4, 8(a0)
        lw      a5, 8(a1)
        bne     a4, a5, L(exit_8)
        add     a4, a4, t0
        not     a5, a5
        and     a5, a5, a4
        and     a5, a5, a3
        bnez    a5, L(exit_8)

        lw      a4, 12(a0)
        lw      a5, 12(a1)
        bne     a4, a5, L(exit_12)
        add     a4, a4, t0
        not     a5, a5
        and     a5, a5, a4
        and     a5, a5, a3
        addi    a0, a0, 16
        addi    a1, a1, 16
        beqz    a5, L(wordcmp)

        addi    a0, a0, -16
        addi    a1, a1, -16

L(exit_12):
        addi    a0, a0, 4
        addi    a1, a1, 4
L(exit_8):
        addi    a0, a0, 4
        addi    a1, a1, 4
L(exit_4):
        addi    a0, a0, 4
        addi    a1, a1, 4
#endif

L(unaligned):
        lbu     a2, (a0)
        addi    a0, a0, 1
        lbu     a3, (a1)
        addi    a1, a1, 1
        bne     a2, a3, L(ret)
        bnez    a2, L(unaligned)

L(ret):
        sub     a0, a2, a3
        ret

#endif

END_FUNC strcmp

#endif

/*************************** End of file ****************************/
