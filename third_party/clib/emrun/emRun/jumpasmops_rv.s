/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#include "asmdefs_rv.ah"

.macro savew reg
        sw      \reg, buf_offset(a0)
       .set     buf_offset, buf_offset+4
.endm

.macro saved reg
        sd      \reg, buf_offset(a0)
       .set     buf_offset, buf_offset+8
.endm

.macro fsavew reg
        fsw     \reg, buf_offset(a0)
       .set     buf_offset, buf_offset+4
.endm

.macro fsaved reg
        fsd     \reg, buf_offset(a0)
       .set     buf_offset, buf_offset+8
.endm

.macro loadw reg
        lw      \reg, buf_offset(a0)
       .set     buf_offset, buf_offset+4
.endm

.macro loadd reg
        ld      \reg, buf_offset(a0)
       .set     buf_offset, buf_offset+8
.endm

.macro floadw reg
        flw     \reg, buf_offset(a0)
       .set     buf_offset, buf_offset+4
.endm

.macro floadd reg
        fld     \reg, buf_offset(a0)
       .set     buf_offset, buf_offset+8
.endm

GLOBAL_FUNC setjmp
       .set     buf_offset, 0
#ifdef __riscv_abi_rve
        savew   ra
        savew   sp
        savew   s0
        savew   s1
#elif __riscv_xlen == 32
        savew   ra
        savew   sp
        savew   s0
        savew   s1
        savew   s2
        savew   s3
        savew   s4
        savew   s5
        savew   s6
        savew   s7
        savew   s8
        savew   s9
        savew   s10
        savew   s11
#elif __riscv_xlen == 64
        saved   ra
        saved   sp
        saved   s0
        saved   s1
        saved   s2
        saved   s3
        saved   s4
        saved   s5
        saved   s6
        saved   s7
        saved   s8
        saved   s9
        saved   s10
        saved   s11
#endif
#if __riscv_flen == 32
        fsavew  fs0
        fsavew  fs1
        fsavew  fs2
        fsavew  fs3
        fsavew  fs4
        fsavew  fs5
        fsavew  fs6
        fsavew  fs7
        fsavew  fs8
        fsavew  fs9
        fsavew  fs10
        fsavew  fs11
#elif __riscv_flen == 64
       .if      buf_offset % 8
       .err     Buffer offset not aligned
       .endif
        fsaved  fs0
        fsaved  fs1
        fsaved  fs2
        fsaved  fs3
        fsaved  fs4
        fsaved  fs5
        fsaved  fs6
        fsaved  fs7
        fsaved  fs8
        fsaved  fs9
        fsaved  fs10
        fsaved  fs11
#endif
        mv   a0, zero
        ret
END_FUNC setjmp

GLOBAL_FUNC longjmp
       .set  buf_offset, 0
#ifdef __riscv_abi_rve
        loadw   ra
        loadw   sp
        loadw   s0
        loadw   s1
#elif __riscv_xlen == 32
        loadw   ra
        loadw   sp
        loadw   s0
        loadw   s1
        loadw   s2
        loadw   s3
        loadw   s4
        loadw   s5
        loadw   s6
        loadw   s7
        loadw   s8
        loadw   s9
        loadw   s10
        loadw   s11
#elif __riscv_xlen == 64
        loadd   ra
        loadd   sp
        loadd   s0
        loadd   s1
        loadd   s2
        loadd   s3
        loadd   s4
        loadd   s5
        loadd   s6
        loadd   s7
        loadd   s8
        loadd   s9
        loadd   s10
        loadd   s11
#endif
#if __riscv_flen == 32
        floadw  fs0
        floadw  fs1
        floadw  fs2
        floadw  fs3
        floadw  fs4
        floadw  fs5
        floadw  fs6
        floadw  fs7
        floadw  fs8
        floadw  fs9
        floadw  fs10
        floadw  fs11
#elif __riscv_flen == 64
       .if      buf_offset % 8
       .err     Buffer offset not aligned
       .endif
        floadd  fs0
        floadd  fs1
        floadd  fs2
        floadd  fs3
        floadd  fs4
        floadd  fs5
        floadd  fs6
        floadd  fs7
        floadd  fs8
        floadd  fs9
        floadd  fs10
        floadd  fs11
#endif
        seqz a0, a1
        add  a0, a0, a1
        ret

END_FUNC longjmp

#undef L
