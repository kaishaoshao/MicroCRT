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

#if !defined(__aarch64__)

@ gnu_syntax

#include "asmdefs_arm.ah"

#if __SEGGER_RTL_STRING_ASM

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       strcmp()
*
*  Function description
*    Compare strings.
*
*  Parameters
*    r0 - Pointer to string #1.
*    r1 - Pointer to string #2.
*
*  Return value
*    r0 - Result.
*/

#undef L
#define L(label) .Lstrcmp_##label

ARM_GLOBAL_FUNC strcmp

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

1:
        ldrb    r2, [r0]
        adds    r0, r0, #1
        ldrb    r3, [r1]
        adds    r1, r1, #1
        subs    r2, r2, r3
        bne     L(ret)
        cmp     r3, #0                  // Is the character a null terminator?
        bne     1b                      // If not, keep comparing.
L(ret):
        movs    r0, r2
        bx      lr

#elif __SEGGER_RTL_OPTIMIZE < 0

1:
        ldrb    r2, [r0], #1
        ldrb    r3, [r1], #1
        subs    r2, r2, r3
        bne     L(ret)
        cmp     r3, #0                  // Is the character a null terminator?
        bne     1b                      // If not, keep comparing.
L(ret):
        mov     r0, r2
        bx      lr

#else

#if __SEGGER_RTL_ALIGN_REM(~0u) != 0
        orrs    r3, r0, r1
        lsls    r3, r3, #30
        bne     L(unaligned)
#endif

#if __SEGGER_RTL_CORE_HAS_UQADD_UQSUB

        li      ip, 0x01010101

L(wordstrcmp):
        ldr     r2, [r0], #4            // r4 = *source (source is a word ptr)
        ldr     r3, [r1], #4
        cmp     r2, r3
        bne     L(bytestrcmp)           // different
        uqsub8  r2, ip, r2
#if __SEGGER_RTL_OPTIMIZE >= 0
        CBNZx   r2, L(bytestrcmp)

        ldr     r2, [r0], #4            // r4 = *source (source is a word ptr)
        ldr     r3, [r1], #4
        cmp     r2, r3
        bne     L(bytestrcmp)           // different
        uqsub8  r2, ip, r2
        CBNZx   r2, L(bytestrcmp)

        ldr     r2, [r0], #4            // r4 = *source (source is a word ptr)
        ldr     r3, [r1], #4
        cmp     r2, r3
        bne     L(bytestrcmp)           // different
        uqsub8  r2, ip, r2
        CBNZx   r2, L(bytestrcmp)

        ldr     r2, [r0], #4            // r4 = *source (source is a word ptr)
        ldr     r3, [r1], #4
        cmp     r2, r3
        bne     L(bytestrcmp)           // different
        uqsub8  r2, ip, r2
#endif
        cmp     r2, #0
        beq     L(wordstrcmp)

L(bytestrcmp):

#else

        push    {r4-r6}

        li      r6, 0x01010101
        lsls    r3, r6, #7              // r3 := 0x80808080

L(wordstrcmp):
        ldr     r4, [r0], #4
        ldr     r5, [r1], #4
        cmp     r4, r5
        bne     L(bytestrcmp)

// Now, either these are the same words or one of them has a zero in.
        subs    r5, r4, r6
        bics    r5, r5, r4
        ands    r5, r5, r3
        beq     L(wordstrcmp)

// Reposition to start of word
L(bytestrcmp):
        pop     {r4-r6}

#endif

        subs    r0, r0, #4
        subs    r1, r1, #4

L(unaligned):
        ldrb    r2, [r0], #1            // Read from and increment source pointer.
        ldrb    r3, [r1], #1            // Store to and increment destination pointer.
        subs    r2, r2, r3
        bne     L(ret)
        cmp     r3, #0                  // Is the character a null terminator?
        bne     L(unaligned)            // If not, keep comparing.

L(ret):
        mov     r0, r2
        bx      lr

#endif

END_FUNC strcmp

/*********************************************************************
*
*       strcpy()
*
*  Function description
*    Copy string.
*
*  Parameters
*    r0 - String to copy to.
*    r1 - String to copy.
*
*  Return value
*    r0 - Copy of r0 on entry.
*/

#undef L
#define L(label) .Lstrcpy_##label

ARM_GLOBAL_FUNC strcpy

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

//
// Code for 16-bit Thumb instruction set
//
        movs    r2, #0
1:
        ldrb    r3, [r1, r2]
        strb    r3, [r0, r2]
        adds    r2, r2, #1
        cmp     r3, #0
        bne     1b
        bx      lr

#elif __SEGGER_RTL_OPTIMIZE < 0

//
// Universal code when optimizing balanced or for size.
//
        mov     r2, r0
1:
        ldrb    r3, [r1], #1
        strb    r3, [r2], #1
        cmp     r3, #0
        bne     1b
        bx      lr

#else

//
// Universal code when optimizing for speed.
//
        mov     r2, r0

#if __SEGGER_RTL_ALIGN_REM(~0u) != 0
        orrs    r3, r0, r1
        lsls    r3, r3, #30
        bne     L(unaligned)
#endif

        push    {r4-r7}

        li      r7, 0x01010101
        lsls    r3, r7, #7              // r3 := 0x80808080

L(wordstrcpy):
        ldr     r4, [r1], #4            // r4 = *source (source is a word ptr)
        subs    r5, r4, r7
        bics    r5, r5, r4
        ands    r5, r5, r3
        bne     L(bytestrcpy)

        str     r4, [r2], #4            // *dest++ = r4  
        b       L(wordstrcpy)

L(bytestrcpy):
        subs    r1, r1, #4              // src--
        pop     {r4-r7}
L(unaligned):
        ldrb    r3, [r1], #1
        strb    r3, [r2], #1
        cmp     r3, #0
        bne     L(unaligned)
        bx      lr

#endif

END_FUNC strcpy

#undef L
#define L(label) .Lstrlen_##label

#if 0

// Tne C compiler generates pretty good code independent of this...

/*********************************************************************
*
*       strlen()
*
*  Function description
*    Calculate length of string.
*
*  Parameters
*    r0 - Pointer to zero-terminated string.
*
*  Return value
*    r0 - Length of the string.
*/
ARM_GLOBAL_FUNC strlen

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

//
// Code for 16-bit Thumb instruction set
//
        adds    r1, r0, #1
L(find_end):
        ldrb    r2, [r0]
        adds    r0, r0, #1
        cmp     r2, #0
        bne     L(find_end)
        subs    r0, r0, r1
        bx      lr

#elif __SEGGER_RTL_OPTIMIZE < 0

//
// Universal code when optimizing for size.
//
        add     r1, r0, #1
L(find_end):
        ldrb    r2, [r0], #1
        cmp     r2, #0
        bne     L(find_end)
        subs    r0, r0, r1
        bx      lr

#else

//
// Universal code when optimizing balanced or for speed.
//
        adds    r1, r0, #1                // Record starting pointer position

        lsls    r2, r0, #30               // Isolate low order 2 bits, smaller than tst r0, #3
        beq     L(aligned)

        ldrb    r2, [r0], #1
        CBZx    r2, L(done)

        lsls    r2, r0, #30
        beq     L(aligned)

        ldrb    r2, [r0], #1
        CBZx    r2, L(done)

        lsls    r2, r0, #30
        beq     L(aligned)

        ldrb    r2, [r0], #1
        CBZx    r2, L(done)

L(aligned):

#if __SEGGER_RTL_CORE_HAS_UQADD_UQSUB

        li      ip, 0x01010101
L(wordstrlen):
        ldr     r2, [r0], #4
        uqsub8  r3, ip, r2
        CBNZx   r3, L(bytestrlen)
        ldr     r2, [r0], #4
        uqsub8  r3, ip, r2
        CBNZx   r3, L(bytestrlen)
        ldr     r2, [r0], #4
        uqsub8  r3, ip, r2
        CBNZx   r3, L(bytestrlen)
        ldr     r2, [r0], #4
        uqsub8  r3, ip, r2
        cmp     r3, #0
        beq     L(wordstrlen)

L(bytestrlen):

#else

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32

L(wordstrlen):
        ldr     r2, [r0], #4
#if __SEGGER_RTL_CORE_HAS_REV && (__SEGGER_RTL_BYTE_ORDER > 0)
        rev     r2, r2
#endif
        subs    r3, r2, #0x01010101
        bics    r3, r3, r2
        ands    r3, r3, #0x80808080
        beq     L(wordstrlen)

#else

        li      ip, 0x01010101
L(wordstrlen):
        ldr     r2, [r0], #4
#if __SEGGER_RTL_CORE_HAS_REV && (__SEGGER_RTL_BYTE_ORDER > 0)
        rev     r2, r2
#endif
        subs    r3, r2, ip
        bics    r3, r3, r2
        ands    r3, r3, ip, lsl #7
        beq     L(wordstrlen)

#endif

#if __SEGGER_RTL_CORE_HAS_CLZ && __SEGGER_RTL_CORE_HAS_REV
        rev     r3, r3
#endif

#endif

#if __SEGGER_RTL_CORE_HAS_CLZ

// At this point r3 has nonzero values in all byte positions that are
// not 0x00 in the loaded word, and 0x00 in all byte positions that
// are 0x00.  We can use this to quickly find which byte contains
// the terminating zero.

        clz     r3, r3
        add     r0, r0, r3, lsr #3
        subs    r0, r0, #4-1

#else

        subs    r0, r0, #4

L(unaligned):
        ldrb    r2, [r0], #1
        cmp     r2, #0
        bne     L(unaligned)

#endif

L(done):
        subs    r0, r0, r1
        bx      lr

#endif

END_FUNC strlen

#endif

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
*    s - r0 - String to search.
*    c - r1 - Character to search for.
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
ARM_GLOBAL_FUNC strchr

#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16

//
// Code for 16-bit Thumb instruction set
//
        UXTBs   r1, r1
        subs    r0, r0, #1
L(search):
        adds    r0, r0, #1
        ldrb    r2, [r0]
        cmp     r2, r1
        beq     L(found)
        cmp     r2, #0
        bne     L(search)
        movs    r0, #0
L(found):
        bx      lr

#elif __SEGGER_RTL_OPTIMIZE < 0

//
// Universal code when optimizing balanced or for size.
//
        UXTBs   r1, r1
        subs    r0, r0, #1
L(search):
        adds    r0, r0, #1
        ldrb    r2, [r0]
        cmp     r2, r1
        beq     L(found)
        cmp     r2, #0
        bne     L(search)
        movs    r0, #0
L(found):
        bx      lr

#else

//
// Head loop, try to align and also check for string.
//
        UXTBs   r1, r1
L(search):
        lsls    r2, r0, #30             // 16-bit opcode, tst r0, #3 is a 32-bit opcode
        beq     L(aligned)
        ldrb    r2, [r0], #1
        cmp     r2, r1
        beq     L(found)
        cmp     r2, #0
        bne     L(search)
        movs    r0, #0
        bx      lr
L(found):
        subs    r0, r0, #1
        bx      lr

#if __SEGGER_RTL_CORE_HAS_UQADD_UQSUB

L(aligned):

        li      r2, 0x01010101
        muls    r1, r1, r2

L(wordsearch):
        ldr     ip, [r0], #4            // Load string 4 bytes at a time
        uqsub8  r3, r2, ip
        CBNZx   r3, L(bytesearch)
        eors    ip, ip, r1              // Generate zero bytes where candidates match
        uqsub8  r3, r2, ip
#if __SEGGER_RTL_OPTIMIZE > 0
        CBNZx   r3, L(bytesearch)

        ldr     ip, [r0], #4
        uqsub8  r3, r2, ip
        CBNZx   r3, L(bytesearch)
        eors    ip, ip, r1
        uqsub8  r3, r2, ip
        CBNZx   r3, L(bytesearch)

        ldr     ip, [r0], #4
        uqsub8  r3, r2, ip
        CBNZx   r3, L(bytesearch)
        eors    ip, ip, r1
        uqsub8  r3, r2, ip
        CBNZx   r3, L(bytesearch)

        ldr     ip, [r0], #4
        uqsub8  r3, r2, ip
        CBNZx   r3, L(bytesearch)
        eors    ip, ip, r1
        uqsub8  r3, r2, ip
#endif
        cmp     r3, #0
        beq     L(wordsearch)

L(bytesearch):

#else

L(aligned):

        push    {r4-r6}

        li      r2, 0x01010101
        lsls    r3, r2, #7              // r3 := 0x80808080
        muls    r1, r2, r1              // NOTE: GNU Assembler complains for ARMv4 if rd = rm.

L(wordsearch):
        ldr     r4, [r0], #4            // Load string 4 bytes at a time
        subs    r5, r4, r2              // Test to see if this contains a null, in which case we need to examine byte by byte
        bics    r5, r5, r4
        eors    r4, r4, r1              // Generate zero bytes where candidates match
        subs    r6, r4, r2              // Test to see if this contains a null, in which case we need to examine byte by byte
        bics    r6, r6, r4
        orrs    r6, r6, r5
        ands    r6, r6, r3

#if __SEGGER_RTL_OPTIMIZE <= 0
        beq     L(wordsearch)           // Didn't match any bytes
#else
        bne     L(bytesearch)           // Didn't match any bytes
        ldr     r4, [r0], #4            // Load string 4 bytes at a time
        subs    r5, r4, r2              // Test to see if this contains a null, in which case we need to examine byte by byte
        bics    r5, r5, r4
        eors    r4, r4, r1              // Generate zero bytes where candidates match
        subs    r6, r4, r2              // Test to see if this contains a null, in which case we need to examine byte by byte
        bics    r6, r6, r4
        orrs    r6, r6, r5
        ands    r6, r6, r3
        beq     L(wordsearch)           // Didn't match any bytes
#endif

L(bytesearch):
        pop     {r4-r6}

#endif

        subs    r0, r0, #4
        UXTBs   r1, r1
L(tailsearch):
        ldrb    r2, [r0], #1
        cmp     r2, r1
        beq     L(tailfound)
        cmp     r2, #0
        bne     L(tailsearch)
        movs    r0, #0
        bx      lr

L(tailfound):
        subs    r0, r0, #1
        bx      lr

#endif

END_FUNC strchr

#undef L
#define L(label) .Lmemcpy_##label

#if (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16) || (__SEGGER_RTL_OPTIMIZE < 0)

ARM_GLOBAL_FUNC memcpy
ALIAS_LABEL     __aeabi_memcpy
ALIAS_LABEL     __aeabi_memcpy4
ALIAS_LABEL     __aeabi_memcpy8

        CBZx    r2, L(memcpy_end)

L(memcpy_loop):
        subs    r2, r2, #1
        ldrb    r3, [r1, r2]
        strb    r3, [r0, r2]
        bne     L(memcpy_loop)

L(memcpy_end):
        bx      lr

#else

ARM_GLOBAL_FUNC memcpy
ALIAS_LABEL     __aeabi_memcpy

        CBZx    r2, L(done)

// Copy initial destination pointer for result.
        mov     r12, r0

// If pointers can never be aligned, carry out a simple byte copy.
        eors    r3, r0, r1
        lsls    r3, r3, #30
        bne     L(byte_copy_loop)

// Copy until byte pointers are aligned.
L(word_align_check):
        lsls    r3, r0, #30
        beq     L(aligned)
L(word_align):
        ldrb    r3, [r1], #1
        strb    r3, [r0], #1
        subs    r2, r2, #1
        bne     L(word_align_check)
        b       L(memcpy_end)

ALIAS_LABEL __aeabi_memcpy4
ALIAS_LABEL __aeabi_memcpy8

// Copy initial destination pointer for result
        mov     r12, r0

// If there aren't enough bytes for one block copy, carry out a word copy
L(aligned):
        subs    r2, #32
        bcc     L(word_copy)

// Carry out aligned block copy
        push    {r4-r10}
L(aligned_block_copy_loop):
        ldmia   r1!, {r3-r10}
        stmia   r0!, {r3-r10}
        subs    r2, r2, #32
        bcs     L(aligned_block_copy_loop)
        pop     {r4-r10}
  
// Carry out word copy
L(word_copy):
// If there aren't enough bytes for a word copy, carry out a byte copy
        adds    r2, #28
        bmi     L(byte_copy)

L(word_copy_loop):
        ldr     r3, [r1], #4
        str     r3, [r0], #4
        subs    r2, r2, #4
        bhs     L(word_copy_loop)

        // Carry out byte copy.
L(byte_copy):
        adds    r2, #4
        beq     L(memcpy_end)

L(byte_copy_loop):
        ldrb    r3, [r1], #1
        strb    r3, [r0], #1
        subs    r2, r2, #1
        bne     L(byte_copy_loop)

L(memcpy_end):
        // Restore initial destination pointer and return as result
        mov     r0, r12
L(done):
        bx      lr
#endif

END_FUNC memcpy

#undef L
#define L(label) .L__aeabi_memset_##label

ARM_GLOBAL_FUNC __aeabi_memclr
ALIAS_LABEL     __aeabi_memclr8
ALIAS_LABEL     __aeabi_memclr4

        movs    r2, #0

ALIAS_LABEL __aeabi_memset8
ALIAS_LABEL __aeabi_memset4
ALIAS_LABEL __aeabi_memset

// Swap operands into memset() parameter order.
        movs    r3, r1
        movs    r1, r2
        movs    r2, r3

ALIAS_LABEL memset

#if (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16) || (__SEGGER_RTL_OPTIMIZE < 0)

        CBZx    r2, L(memset_end)
L(memset_loop):
        subs    r2, r2, #1
        strb    r1, [r0, r2]
        bne     L(memset_loop)

L(memset_end):
        bx      lr

#else

        mov     r12, r0                 // Copy initial destination pointer for result.
        CBZx    r2, L(memset_end)

L(unaligned_byte_set_loop):
        lsls    r3, r0, #30
        beq     L(fast_set)

        strb    r1, [r0], #1
        subs    r2, r2, #1
        bne     L(unaligned_byte_set_loop)
        b       L(memset_end)

L(fast_set):
        UXTBs   r1, r1
        orr     r1, r1, r1, lsl #8
        orr     r1, r1, r1, lsl #16
        cmp     r2, #32                 // Check if there is enough for a block write.
        blo     L(word_set)             // If not, move onto the word write code.
        subs    r2, r2, #32             // Decrement counter by block size.
        push    {r4-r9}                 // Store registers we are using for writing.
        mov     r3, r1                  // Duplicate the memset value into all the 
        mov     r4, r1                  // registers we are to use for writing.
        mov     r5, r1
        mov     r6, r1
        mov     r7, r1
        mov     r8, r1
        mov     r9, r1
L(fast_set_loop):
        stmia   r0!, {r1, r3-r9}        // Carry out block write.
        subs    r2, r2, #32             // Decrement counter by block size
        bhs     L(fast_set_loop)        // Keep going while counter >= 0
        pop     {r4-r9}                 // Restore registers used for writing.
        adds    r2, r2, #32             // Correct the counter.
        beq     L(memset_end)           // If there is nothing else to do, finish.

L(word_set):
        cmp     r2, #4                  // Check if there is enough for word write
        blo     L(byte_set_loop)        // If not, move onto the byte write code.

L(word_set_loop):                       // Carry out byte write for remaining bytes.
        str     r1, [r0], #4
        subs    r2, r2, #4
        beq     L(memset_end)           // Nothing more to write
        cmp     r2, #4
        bhs     L(word_set_loop)

L(byte_set_loop):                       // Carry out byte write for remaining bytes.
        strb    r1, [r0], #1
        subs    r2, r2, #1
        bne     L(byte_set_loop)

L(memset_end):
        mov     r0, r12                 // Return destination pointer.
        bx      lr
#endif

END_FUNC __aeabi_memclr

#undef L
#define L(label) .L__aeabi_memmove_##label

ARM_GLOBAL_FUNC __aeabi_memmove
ALIAS_LABEL     __aeabi_memmove4
ALIAS_LABEL     __aeabi_memmove8

// If zero length, done
        CBZx    r2, L(done)

// Determine copy order.
        cmp     r0, r1
#if __SEGGER_RTL_OPTIMIZE >= 0
        beq     L(done)                 // copy to self, no operation
#endif
        bcs     L(copy_from_end)        // destination >= source, copy backwards from end

// Copy from start address
        adds    r0, r0, r2
        adds    r1, r1, r2
        rsbs    r2, r2, #0
L(copy_loop):
        ldrb    r3, [r1, r2]
        strb    r3, [r0, r2]
        adds    r2, r2, #1
        bne     L(copy_loop)
L(done):
        bx      lr

// Copy from end address
L(copy_from_end):
        subs    r2, r2, #1
        ldrb    r3, [r1, r2]
        strb    r3, [r0, r2]
        bne     L(copy_from_end)
        bx      lr

@ gnu_syntax

#endif

#endif

       .end

/*************************** End of file ****************************/
