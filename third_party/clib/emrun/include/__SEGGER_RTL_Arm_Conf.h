/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef __SEGGER_RTL_ARM_CONF_H
#define __SEGGER_RTL_ARM_CONF_H

/*********************************************************************
*
*       Applicability checks
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

//
// NOTE: Not all compilers define __ARM_ACLE so this is disabled for now
//
#if 0 && !defined(__ARM_ACLE)
  #error This configuration file expects and ACLE-conforming compiler for configuration!
#endif
#if !defined(__ARM_ARCH_ISA_ARM) && !defined(__ARM_ARCH_ISA_THUMB)
  #error This configuration file expects __ARM_ARCH_ISA_ARM
  #error or __ARM_ARCH_ISA_THUMB to be defined!
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define __SEGGER_RTL_ISA_T16                    0
#define __SEGGER_RTL_ISA_T32                    1
#define __SEGGER_RTL_ISA_ARM                    2

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#if !defined(__SEGGER_RTL_NO_BUILTIN)
  #if defined(__clang__)
    #define __SEGGER_RTL_NO_BUILTIN
  #elif defined(__GNUC__)
    #define __SEGGER_RTL_NO_BUILTIN             \
        __attribute__((optimize("-fno-tree-loop-distribute-patterns")))
  #else
  #endif
#endif
 
#if defined(__GNUC__) || defined(__clang__)
  #if defined(__has_builtin)
    #if __has_builtin(__builtin_unreachable)
      #define __SEGGER_RTL_UNREACHABLE()        __builtin_unreachable()
    #endif
  #endif
#endif

#if defined(__thumb__) && !defined(__thumb2__)
  #define __SEGGER_RTL_TARGET_ISA               __SEGGER_RTL_ISA_T16
#elif defined(__thumb2__)
  #define __SEGGER_RTL_TARGET_ISA               __SEGGER_RTL_ISA_T32
#else
  #define __SEGGER_RTL_TARGET_ISA               __SEGGER_RTL_ISA_ARM
#endif

//
// GCC and clang on ARM default to include the Arm AEABI
// with assembly speedups (2).  Define this to 1 to use the
// C implementation.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_INCLUDE_AEABI_API        2
#endif

//
// GCC on Arm doesn't use the AEABI naming convention for half-float.
// Define this to 1 to force inclusion for this retrograde compiler.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_INCLUDE_GNU_FP16_API     1
#endif

//
// External compilers or user code might use GNU libgcc API.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_INCLUDE_GNU_API        1
#endif

//
// Configuration of assembly-optimized or C string functions.
//
#define __SEGGER_RTL_STRING_ASM                 1

//
// Configuration of byte order.
//
#if defined(__ARM_BIG_ENDIAN) && (__ARM_BIG_ENDIAN == 1)
  #define __SEGGER_RTL_BYTE_ORDER               (+1)
#else
  #define __SEGGER_RTL_BYTE_ORDER               (-1)
#endif

//
// Configuration of unaligned accesses.
//
#if defined(__ARM_FEATURE_UNALIGNED)
  #define __SEGGER_RTL_UNALIGNED_RW_FAULTS      0
#else
  #define __SEGGER_RTL_UNALIGNED_RW_FAULTS      1
#endif

//
// Configuration of "32-bit" problematic format prefix.
// Test here for true GCC and not for compilers masquerading
// as GCC.
//
// The issue is that the compiler-defined types __INT32_TYPE__
// and __UINT32_TYPE__ are defined as "[unsigned] long int" for
// GCC and "[unsigned] int" for clang.  This sets the problematic
// print/scan type specifier correctly for the target compiler.
//
#if !defined(__clang__) && defined(__GNUC_PATCHLEVEL__)
  #define __SEGGER_RTL_PRI_32_PREFIX            "l"
#endif

//
// Configuration of typeset.
//
#define __SEGGER_RTL_TYPESET                    32
#define __SEGGER_RTL_ADDRSIZE                   32
//
// Configuration of 16-bit floating type.
//
#if defined(__clang__)
  #define __SEGGER_RTL_FLOAT16                  _Float16
#elif defined(__GNUC_PATCHLEVEL__) && !defined(__cplusplus)
  #define __SEGGER_RTL_FLOAT16                  _Float16
#endif

//
// Configuration of maximal type alignment
//
#define __SEGGER_RTL_MAX_ALIGN                  8

//
// Configuration of floating-point ABI.
//
#if defined(__ARM_PCS_VFP) && (__ARM_PCS_VFP == 1)
  //
  // PCS uses hardware registers for passing parameters.  For VFP
  // with only single-precision operations, parameters are still
  // passed in floating registers.
  //
  #define __SEGGER_RTL_FP_ABI                   2
  //
#elif defined(__ARM_PCS) && (__ARM_PCS == 1)
  //
  // PCS is standard integer PCS.
  //
  #define __SEGGER_RTL_FP_ABI                   0
  //
#else
  #error Unable to determine floating-point ABI used
#endif

//
// Configuration of floating-point hardware.
//
#if defined(__ARM_FP) && (__ARM_FP & 0x08)
  #define __SEGGER_RTL_FP_HW                    2
#elif defined(__ARM_FP) && (__ARM_FP & 0x04)
  #define __SEGGER_RTL_FP_HW                    1
#else
  #define __SEGGER_RTL_FP_HW                    0
#endif

// Clang gets __ARM_FP wrong for the T16 target ISA indicating
// that floating-point instructions exist in this ISA -- which
// they don't.  Patch that definition up here.
#if __ARM_ARCH_ISA_THUMB == 1
  #undef  __SEGGER_RTL_FP_HW
  #define __SEGGER_RTL_FP_HW                    0
  #undef  __SEGGER_RTL_FP_ABI
  #define __SEGGER_RTL_FP_ABI                   0
#endif

//
// Configuration of full or Arm AEABI NaNs.
//
#if __SEGGER_RTL_FP_ABI == 0
  #define __SEGGER_RTL_NAN_FORMAT               __SEGGER_RTL_NAN_FORMAT_IEEE
#endif

//
// Configuration of floating constant selection.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_FLT_SELECT(HEX, DEC)     HEX
#endif

//
// Configuration of types for atomics.  The default selected by emRun
// would use the GCC/Clang compiler-defined __UINT32_TYPE__ which
// expands to 'unsigned long' for Arm.  This doesn't match the 32-bit
// type 'unsigned' which is required for the atomic prototypes, so
// override the default here.
//
#define __SEGGER_RTL_ATOMIC_U32                 unsigned

//
// Configuration of multi-register clear capability for v8.1 Mainline.
// Disabled for now awaiting compilers to conform to ACLE 2022Q4
//
#if 0
  #define __SEGGER_RTL_CORE_HAS_CLRM            1
#else
  #define __SEGGER_RTL_CORE_HAS_CLRM            0
#endif

//
// Configuration of conditional increment.
// Disabled for now awaiting compilers to conform to ACLE 2022Q4
//
#if 0
  #define __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV  1
#else
  #define __SEGGER_RTL_CORE_HAS_CSINC_CSNEG_CSINV  0
#endif

//
// Configuration of long shifts.
//
#if (__ARM_ARCH >= 8) && defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE != 0)
  #define __SEGGER_RTL_CORE_HAS_LSLL_LSRL_ASRL  1
#else
  #define __SEGGER_RTL_CORE_HAS_LSLL_LSRL_ASRL  0
#endif

//
// Configuration of multiply-add capability.
//
#if __SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T16
  #define __SEGGER_RTL_CORE_HAS_MLA             1
#else
  #define __SEGGER_RTL_CORE_HAS_MLA             0
#endif

//
// Configuration of multiply-subtract capability.
//
#if (__ARM_ARCH >= 7) && (__SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T16)
  #define __SEGGER_RTL_CORE_HAS_MLS             1
#else
  #define __SEGGER_RTL_CORE_HAS_MLS             0
#endif

//
// Configuration of multiplication capability.
//
// In the ARM Architecture Reference Manual, DDI 01001, Arm states
// the following for SMULL and UMULL:
//
//   "Specifying the same register for either <RdHi> and <Rm>,
//   or <RdLo> and <Rm>, was previously described as producing
//   UNPREDICTABLE results. There is no restriction in ARMv6, and
//   it is believed all relevant ARMv4 and ARMv5 implementations
//   do not require this restriction either, because high
//   performance multipliers read all their operands prior to
//   writing back any results."
//
// Unfortunately, the GNU assembler enforces this restriction which
// means that assembly-level inserts will not work for ARMv4 and
// ARMv5 even though there is no indication that they fail in
// practice.
//
#if __SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16
  //
  // T16 ISA has no extended multiplication at all.
  //
  #define __SEGGER_RTL_CORE_HAS_EXT_MUL         0
  //
#elif __ARM_ARCH >= 6
  //
  // ARMv6 and above have no restrictions on their input
  // and output registers, so assembly-level inserts with
  // constraints to guide the compiler are acceptable.
  //
  #define __SEGGER_RTL_CORE_HAS_EXT_MUL         1
  //
#elif (__ARM_ARCH == 5) && defined(__clang__)
  //
  // Take Arm at its word and disable restrictions on input
  // and output registers.
  //
  #define __SEGGER_RTL_CORE_HAS_EXT_MUL         1
  //
#else 
  //
  // ARMv5TE and lower have restrictions on their input
  // and output registers, therefore do not enable extended
  // multiply inserts.
  //
  #define __SEGGER_RTL_CORE_HAS_EXT_MUL         0
  //
#endif

#if __SEGGER_RTL_CORE_HAS_EXT_MUL && (defined(__GNUC__) || defined(__clang__))
  //
  // v6+DSP and v7E-M has SMMUL instruction, others do not.
  //
  // Benchmarking using GCC and Clang/LLVM shows that using SMULL results in faster
  // code than SMMUL.  Using SMMUL results in marginally smaller code because there
  // is less register pressure (no requirement to allocate a bit-bucket register).
  //
  // Given this, we disable the detection of SMMUL and use SMULL always.
  //
  #if 0 && (__ARM_ARCH >= 6) && defined(__ARM_FEATURE_DSP) && (__ARM_FEATURE_DSP == 1)
    #define __SEGGER_RTL_SMULL_HI(x0, x1)                      \
      ({ long __hi;                                            \
         __asm__(  "smmul %0, %1, %2"                          \
                 : "=r"(__hi)                                  \
                 : "r"((unsigned)(x0)), "r"((unsigned)(x1)) ); \
         __hi;                                                 \
      })
  #else
    #define __SEGGER_RTL_SMULL_HI(x0, x1)                      \
      ({ long __trash, __hi;                                   \
         __asm__(  "smull %0, %1, %2, %3"                      \
                 : "=r"(__trash), "=r"(__hi)                   \
                 : "r"((unsigned)(x0)), "r"((unsigned)(x1)) ); \
         __hi;                                                 \
      })
  #endif

  #define __SEGGER_RTL_SMULL(lo, hi, x0, x1)                 \
    do {                                                     \
      __asm__(  "smull %0, %1, %2, %3"                       \
              : "=r"(lo), "=r"(hi)                           \
              : "r"((unsigned)(x0)), "r"((unsigned)(x1)) );  \
    } while (0)

  #define __SEGGER_RTL_SMLAL(lo, hi, x0, x1)                 \
    do {                                                     \
      __asm__(  "smlal %0, %1, %2, %3"                       \
              : "+r"(lo), "+r"(hi)                           \
              : "r"((unsigned)(x0)), "r"((unsigned)(x1)) );  \
    } while (0)

  #define __SEGGER_RTL_UMULL_HI(x0, x1)                      \
    ({ unsigned long __trash, __hi;                          \
       __asm__(  "umull %0, %1, %2, %3"                      \
               : "=r"(__trash), "=r"(__hi)                   \
               : "r"((unsigned)(x0)), "r"((unsigned)(x1)) ); \
       __hi;                                                 \
    })

  #define __SEGGER_RTL_UMULL(lo, hi, x0, x1)                 \
    do {                                                     \
      __asm__(  "umull %0, %1, %2, %3"                       \
              : "=r"(lo), "=r"(hi)                           \
              : "r"((unsigned)(x0)), "r"((unsigned)(x1)) );  \
    } while (0)

#if 1
  #define __SEGGER_RTL_UMULL_X(x, y)                         \
    ((__SEGGER_RTL_U64)(__SEGGER_RTL_U32)(x) *               \
                       (__SEGGER_RTL_U32)(y))
#else
  #define __SEGGER_RTL_UMULL_X(x, y)                         \
    ({                                                       \
      unsigned __lo, __hi;                                   \
      __asm__(  "umull %0, %1, %2, %3"                       \
              : "=r"(__lo), "=r"(__hi)                       \
              : "r"((unsigned)(x)), "r"((unsigned)(y)) );    \
      ((__SEGGER_RTL_U64)__hi << 32) + __lo;                 \
    })
#endif

  #define __SEGGER_RTL_UMLAL(lo, hi, x0, x1)                 \
    do {                                                     \
      __asm__("umlal %0, %1, %2, %3"                         \
              : "+r"(lo), "+r"(hi)                           \
              : "r"((unsigned)(x0)), "r"((unsigned)(x1)) );  \
    } while (0)

#endif

//
// Configuration of static branch probability.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_UNLIKELY(X)              __builtin_expect((X), 0)
  #define __SEGGER_RTL_LIKELY(X)                __builtin_expect((X), 1)
#endif

//
// Configuration of thread-local storage
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_THREAD                   __thread
#endif

//
// Configuration of inlining.
//
#if defined(__GNUC__) || defined(__clang__)
  #ifndef   __SEGGER_RTL_NEVER_INLINE
    //
    // Clang doesn't know noclone...
    //
    #if defined(__clang__)
      #define __SEGGER_RTL_NEVER_INLINE         __attribute__((__noinline__))
    #else
      #define __SEGGER_RTL_NEVER_INLINE         __attribute__((__noinline__, __noclone__))
    #endif
  #endif
  //
  #ifndef   __SEGGER_RTL_ALWAYS_INLINE
    #define __SEGGER_RTL_ALWAYS_INLINE          __inline__ __attribute__((__always_inline__))
  #endif
  //
  #ifndef   __SEGGER_RTL_REQUEST_INLINE
    #define __SEGGER_RTL_REQUEST_INLINE         __inline__
  #endif
  //
#endif

//
// Configuration of public APIs.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_PUBLIC_API               __attribute__((__weak__)) 
#endif

//
// Using these builtins requires that the library is compiled
// with -fno-math-errno.
//
#if (__SEGGER_RTL_FP_HW >= 1) && (defined(__GNUC__) || defined(__clang__))
  #define __SEGGER_RTL_FLOAT32_ABS(__m)\
  ({\
    float __d;\
    __asm__ __volatile__("vabs.f32 %[d], %[m]"\
                          : [d] "=t"(__d)\
                          : [m] "t"(__m));\
    __d;\
  })

#endif
#if (__SEGGER_RTL_FP_HW >= 2) && (defined(__GNUC__) || defined(__clang__))
  #define __SEGGER_RTL_FLOAT64_ABS(__m)\
  ({\
    double __d;\
    __asm__ __volatile__("vabs.f64 %P[d], %P[m]"\
                          : [d] "=w"(__d)\
                          : [m] "w"(__m));\
    __d;\
  })
#endif

#if (__SEGGER_RTL_FP_HW >= 1) && (defined(__GNUC__) || defined(__clang__))
  #define __SEGGER_RTL_FLOAT32_SQRT(__m)\
  ({\
    float __d;\
    __asm__ __volatile__("vsqrt.f32 %[d], %[m]"\
                         : [d] "=t"(__d)\
                         : [m] "t"(__m));\
    __d;\
  })
#endif
#if (__SEGGER_RTL_FP_HW >= 2) && (defined(__GNUC__) || defined(__clang__))
  #define __SEGGER_RTL_FLOAT64_SQRT(__m)\
  ({\
    double __d;\
    __asm__ __volatile__("vsqrt.f64 %P[d], %P[m]"\
                         : [d] "=w"(__d)\
                         : [m] "w"(__m));\
    __d;\
  })
#endif

//
// Configuration of CLZ support.
//
#if defined(__ARM_FEATURE_CLZ) && (__ARM_FEATURE_CLZ == 1)
  #define __SEGGER_RTL_CORE_HAS_CLZ             1
#else
  #define __SEGGER_RTL_CORE_HAS_CLZ             0
#endif

// Clang gets __ARM_FEATURE_CLZ wrong for v8M.Baseline, indicating
// that CLZ is available in this ISA  -- which it isn't.  Patch that
// definition up here.
#if (__ARM_ARCH == 8) && (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16)
  #undef  __SEGGER_RTL_CORE_HAS_CLZ
  #define __SEGGER_RTL_CORE_HAS_CLZ             0
#endif

// Clang gets __ARM_FEATURE_CLZ wrong for v6 Thumb, indicating
// that CLZ is available in this ISA  -- which it isn't.  Patch that
// definition up here.
#if (__ARM_ARCH == 6) //&& (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16)
  #undef  __SEGGER_RTL_CORE_HAS_CLZ
  #define __SEGGER_RTL_CORE_HAS_CLZ             0
#endif

// GCC gets __ARM_FEATURE_CLZ wrong for v5TE //compiling for Thumb,
// indicating that CLZ is available in this ISA -- which it isn't.
// Patch that definition up here.
#if (__ARM_ARCH == 5) //&& (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T16)
  #undef  __SEGGER_RTL_CORE_HAS_CLZ
  #define __SEGGER_RTL_CORE_HAS_CLZ             0
#endif

#if __SEGGER_RTL_CORE_HAS_CLZ && !defined(__SEGGER_RTL_EXCLUDE_STATIC_FUNCTIONS)
  //
  // For ACLE-conforming C compilers that declare the architecture or profile
  // has a CLZ instruction, use that CLZ instruction.  However, __builtin_clz(0)
  // is undefined for GCC and SEGGER-CC which contradicts Arm's requrement in
  // ACLE that __builtin_clz(0) == 32.  Arrange to cover that discrepancy here
  // and hope that the compiler is smart enough to eliminate the zero test.
  //
  #define __SEGGER_RTL_CLZ_U32(X)               __SEGGER_RTL_CLZ_U32_safe_inline(X)
  //
  static __SEGGER_RTL_ALWAYS_INLINE int  __SEGGER_RTL_CLZ_U32_safe_inline(unsigned x) {
    return x == 0 ? 32 : __builtin_clz(x);
  }
#endif

//
// Configuration of bit reversal capability.
//
#if defined(__GNUC__) || defined(__clang__)
  #if defined(__has_builtin)
    #if __has_builtin(__builtin_arm_rbit) && (__ARM_ARCH != 4) && (__ARM_ARCH != 5) && (__ARM_ARCH != 6)
      #define __SEGGER_RTL_BITREV_U32(X)        __builtin_arm_rbit(X)
    #endif
  #endif
#endif

//
// For Arm architectures using GNU C or clang, the SEGGER RTL offers
// optimized versions written in GNU-compatible assembly language.
// Selection of them is made here.
//
#if defined(__SEGGER_RTL_COMPILING_LIBRARY) && __SEGGER_RTL_COMPILING_LIBRARY
  #if defined(__GNUC__) || defined(__clang__)
    #if __SEGGER_RTL_STRING_ASM
      #define strcpy(x, y)                      __SEGGER_RTL_HIDE(strcpy)(x, y)
      #define strcmp(x, y)                      __SEGGER_RTL_HIDE(strcmp)(x, y)
      #define strchr(x, y)                      __SEGGER_RTL_HIDE(strchr)(x, y)
//    #define strlen(x)                         __SEGGER_RTL_HIDE(strlen)(x)  We use C-level code for this now
      #define memcpy(x, y, z)                   __SEGGER_RTL_HIDE(memcpy)(x, y, z)
      #define memset(x, y, z)                   __SEGGER_RTL_HIDE(memset)(x, y, z)
    #endif
    //
    // __clzsi2 and __clzdi2 are provided by intasmops_arm.s
    //
    #define __clzsi2(x)                         __SEGGER_RTL_HIDE(__clzsi2)(x)
    #define __clzdi2(x)                         __SEGGER_RTL_HIDE(__clzdi2)(x)
  #endif
#endif


/*********************************************************************
*
*       Configuration of core features.
*
**********************************************************************
*/

#if defined(__ARM_FEATURE_SIMD32) && __ARM_FEATURE_SIMD32
  #define __SEGGER_RTL_CORE_HAS_ISA_SIMD32      1
#else
  #define __SEGGER_RTL_CORE_HAS_ISA_SIMD32      0
#endif

#if (__SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T16) && defined(__ARM_FEATURE_SIMD32) && __ARM_FEATURE_SIMD32
  #define __SEGGER_RTL_CORE_HAS_UQADD_UQSUB     1
#else
  #define __SEGGER_RTL_CORE_HAS_UQADD_UQSUB     0
#endif

#if defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
  #define __SEGGER_RTL_CORE_HAS_REV             1
#else
  #define __SEGGER_RTL_CORE_HAS_REV             0
#endif

#if defined(__ARM_ARCH) && (__ARM_ARCH >= 6) && (__SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_T16) && defined(__ARM_FEATURE_DSP) && (__ARM_FEATURE_DSP == 1)
  #define __SEGGER_RTL_CORE_HAS_PKHTB_PKHBT     1
#else
  #define __SEGGER_RTL_CORE_HAS_PKHTB_PKHBT     0
#endif

#if (__ARM_ARCH >= 7) && (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32)
  #define __SEGGER_RTL_CORE_HAS_ADDW_SUBW       1   // ARMv8A/R only has ADDW in Thumb mode
#else
  #define __SEGGER_RTL_CORE_HAS_ADDW_SUBW       0
#endif

#if __ARM_ARCH >= 7
  #define __SEGGER_RTL_CORE_HAS_MOVW_MOVT       1
#else
  #define __SEGGER_RTL_CORE_HAS_MOVW_MOVT       0
#endif

#if defined(__ARM_FEATURE_IDIV) && __ARM_FEATURE_IDIV
  #define __SEGGER_RTL_CORE_HAS_IDIV            1
#else
  #define __SEGGER_RTL_CORE_HAS_IDIV            0
#endif

#if (__ARM_ARCH >= 7) && (__SEGGER_RTL_TARGET_ISA != __SEGGER_RTL_ISA_ARM)
  #define __SEGGER_RTL_CORE_HAS_CBZ_CBNZ        1
#else
  #define __SEGGER_RTL_CORE_HAS_CBZ_CBNZ        0
#endif

#if (__ARM_ARCH >= 7) && (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32)
  #define __SEGGER_RTL_CORE_HAS_TBB_TBH         1
#else
  #define __SEGGER_RTL_CORE_HAS_TBB_TBH         0
#endif

#if __ARM_ARCH >= 6
  #define __SEGGER_RTL_CORE_HAS_UXT_SXT         1
#else
  #define __SEGGER_RTL_CORE_HAS_UXT_SXT         0
#endif

#if (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32) || (__ARM_ARCH >= 7)
  #define __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX     1
#else
  #define __SEGGER_RTL_CORE_HAS_BFC_BFI_BFX     0
#endif

#if __ARM_ARCH >= 5
  #define __SEGGER_RTL_CORE_HAS_BLX_REG         1
#else
  #define __SEGGER_RTL_CORE_HAS_BLX_REG         0
#endif

#if (__ARM_ARCH >= 6) && (__SEGGER_RTL_TARGET_ISA == __SEGGER_RTL_ISA_T32)
  #define __SEGGER_RTL_CORE_HAS_LONG_SHIFT      1
#else
  #define __SEGGER_RTL_CORE_HAS_LONG_SHIFT      0
#endif

#ifndef __SEGGER_RTL_FAST_CODE_SECTION
  #if defined(__GNUC__) || defined(__clang__)
    #define __SEGGER_RTL_FAST_CODE_SECTION(X)   __attribute__((__section__(".fast." X)))
  #endif
#endif

#ifndef   __SEGGER_RTL_USE_FPU_FOR_IDIV
  #define __SEGGER_RTL_USE_FPU_FOR_IDIV         0
#endif

//
// GCC and clang provide a built-in support for _Complex.
//
#if defined(__GNUC__) || defined(__clang__)
  #ifndef   __SEGGER_RTL_FLOAT32_C_COMPLEX
    #define __SEGGER_RTL_FLOAT32_C_COMPLEX      float _Complex
  #endif
  #ifndef   __SEGGER_RTL_FLOAT64_C_COMPLEX
    #define __SEGGER_RTL_FLOAT64_C_COMPLEX      double _Complex
  #endif
  #ifndef   __SEGGER_RTL_LDOUBLE_C_COMPLEX
    #define __SEGGER_RTL_LDOUBLE_C_COMPLEX      long double _Complex
  #endif
#endif

#ifndef   __SEGGER_RTL_PREFER_BRANCH_FREE_CODE
  #define __SEGGER_RTL_PREFER_BRANCH_FREE_CODE  1
#endif

//
// GCC and clang provide a built-in va_list.
//
#if defined(__GNUC__) || defined(__clang__)
  #define __SEGGER_RTL_VA_LIST                  __builtin_va_list
#endif

//
// ARM C library ABI requires low-level assert function to be __aeabi_assert
//
#define __SEGGER_RTL_X_assert                   __aeabi_assert

//
// ARM C library ABI defines how to interrogate errno
//
#define __SEGGER_RTL_X_errno_addr               __aeabi_errno_addr

#if defined(__SEGGER_RTL_EXCLUDE_STATIC_FUNCTIONS)
  #define __SEGGER_RTL_P2I(X)                   (X)
  #define __SEGGER_RTL_I2P(X)                   (X)
#else
  #define __SEGGER_RTL_P2I(X)                   ((unsigned)(X))
  #define __SEGGER_RTL_I2P(X)                   ((void *)(X))
#endif

#if __SEGGER_RTL_UNALIGNED_RW_FAULTS
  #define __SEGGER_RTL_ALIGN_PTR(X)             __SEGGER_RTL_I2P(__SEGGER_RTL_P2I(X) & ~3u)
  #define __SEGGER_RTL_ALIGN_REM(X)             (__SEGGER_RTL_P2I(X) & 3u)
#else
  #define __SEGGER_RTL_ALIGN_PTR(X)             (void *)(X)
  #define __SEGGER_RTL_ALIGN_REM(X)             0
  #define __SEGGER_RTL_UNALIGNED_ATTR           __attribute__((__aligned__(1)))
#endif

//
// Configuration of atomic-lock-free query.
//
#ifndef   __SEGGER_RTL_ATOMIC_IS_LOCK_FREE
  #define __SEGGER_RTL_ATOMIC_IS_LOCK_FREE(S,P) __SEGGER_RTL_atomic_is_lock_free(S, P)
#endif

#if !defined(__SEGGER_RTL_EXCLUDE_STATIC_FUNCTIONS)

#define __SEGGER_RTL_WORD                       unsigned
#define __SEGGER_RTL_ZBYTE_CHECK(X)             __SEGGER_RTL_ZBYTE_CHECK_func(X)
#define __SEGGER_RTL_ZBYTE_INDEX(X)             __SEGGER_RTL_ZBYTE_INDEX_func(X)
#define __SEGGER_RTL_DIFF_INDEX(X, Y)           __SEGGER_RTL_DIFF_INDEX_func(X, Y)
#define __SEGGER_RTL_DIFF_BYTE(X, N)            __SEGGER_RTL_DIFF_BYTE_func(X, N)
#define __SEGGER_RTL_BYTE_PATTERN(X)            __SEGGER_RTL_BYTE_PATTERN_func(X)
#define __SEGGER_RTL_FILL_HEAD(A, W, C)         __SEGGER_RTL_FILL_HEAD_func(A, W, C)
#define __SEGGER_RTL_FILL_TAIL(N, W, C)         __SEGGER_RTL_FILL_TAIL_func(N, W, C)
#define __SEGGER_RTL_RD_WORD(A)                 __SEGGER_RTL_RD_WORD_func(A)
#define __SEGGER_RTL_WR_WORD(A, W)              __SEGGER_RTL_WR_WORD_func(A, W)
#define __SEGGER_RTL_WR_PARTIAL_WORD(A, W, N)   __SEGGER_RTL_WR_PARTIAL_WORD_func(A, W, N)

static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_WR_PARTIAL_WORD_func(char *addr, __SEGGER_RTL_WORD w, int n) {
  switch (n) {
  default:  addr[3] = __SEGGER_RTL_BYTE_ORDER > 0 ? w : w >> 24;          //FALLTHRU
  case 3:   addr[2] = __SEGGER_RTL_BYTE_ORDER > 0 ? w >> 8 : w >> 16;     //FALLTHRU
  case 2:   addr[1] = __SEGGER_RTL_BYTE_ORDER > 0 ? w >> 16 : w >> 8;     //FALLTHRU
  case 1:   addr[0] = __SEGGER_RTL_BYTE_ORDER > 0 ? w >> 24 : w;          //FALLTHRU
  case 0:   ;
  }
}

static __SEGGER_RTL_ALWAYS_INLINE void __SEGGER_RTL_WR_WORD_func(char *addr, __SEGGER_RTL_WORD w) {
  __SEGGER_RTL_WR_PARTIAL_WORD_func(addr, w, 4);
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_RD_WORD_func(const void *addr) {
  const unsigned char *pAddr = (const unsigned char *)addr;
  //
  return pAddr[0] * (__SEGGER_RTL_BYTE_ORDER > 0 ? 0x1000000u : 0x1u) +
         pAddr[1] * (__SEGGER_RTL_BYTE_ORDER > 0 ? 0x10000u : 0x100u) +
         pAddr[2] * (__SEGGER_RTL_BYTE_ORDER > 0 ? 0x100u : 0x10000u) +
         pAddr[3] * (__SEGGER_RTL_BYTE_ORDER > 0 ? 0x1u : 0x1000000u);
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_BYTE_PATTERN_func(unsigned x) {
  return x * 0x01010101uL;
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_FILL_HEAD_func(const void *pOrigin, __SEGGER_RTL_WORD Word, unsigned Standin) {
  __SEGGER_RTL_WORD Mask;
  __SEGGER_RTL_WORD Fill;
  //
  (void)pOrigin;
  //
  Fill   = __SEGGER_RTL_BYTE_PATTERN(Standin);
  //
#if __SEGGER_RTL_BYTE_ORDER > 0
  //
  Mask   = 0xFFFFFFFFu;
  Mask >>= 8 * __SEGGER_RTL_ALIGN_REM(pOrigin);
  //
  return (Word & Mask) | (Fill & ~Mask);
  //
#else
  //
  Mask   = 0xFFFFFFFFu;
  Mask <<= 8 * __SEGGER_RTL_ALIGN_REM(pOrigin);
  //
  return (Word & Mask) | (Fill & ~Mask);
  //
#endif
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_WORD __SEGGER_RTL_FILL_TAIL_func(unsigned n, __SEGGER_RTL_WORD Word, unsigned Standin) {
  __SEGGER_RTL_WORD Mask;
  __SEGGER_RTL_WORD Fill;
  //
#if __SEGGER_RTL_BYTE_ORDER > 0
  //
  if (n >= 4) {
    return Word;
  } else {
    Fill = __SEGGER_RTL_BYTE_PATTERN(Standin);
    Mask = 0xFFFFFFFFu >> (8 * n);
    //
    return (Fill & Mask) | (Word & ~Mask);
  }
  //
#else
  //
  if (n >= 4) {
    return Word;
  } else {
    Fill = __SEGGER_RTL_BYTE_PATTERN(Standin);
    Mask = 0xFFFFFFFFu << (8 * n);
    //
    return (Fill & Mask) | (Word & ~Mask);
  }
  //
#endif
}

static __SEGGER_RTL_ALWAYS_INLINE unsigned __SEGGER_RTL_ZBYTE_CHECK_func(unsigned x) {
  //
#if __SEGGER_RTL_CORE_HAS_UQADD_UQSUB
  //
  return __builtin_arm_uqsub8(0x01010101, x);
  //
#else
  //
  return ((x-0x01010101u) & ~x & 0x80808080u);
  //
#endif
}

static __SEGGER_RTL_ALWAYS_INLINE unsigned __SEGGER_RTL_DIFF_INDEX_func(__SEGGER_RTL_WORD x, __SEGGER_RTL_WORD y) {
  //
#if __SEGGER_RTL_CORE_HAS_CLZ && (__SEGGER_RTL_BYTE_ORDER < 0) && __has_builtin(__builtin_arm_rbit)
  //
  return __SEGGER_RTL_CLZ_U32(__builtin_arm_rbit(x^y)) / 8;
  //
#elif __SEGGER_RTL_CORE_HAS_CLZ && (__SEGGER_RTL_BYTE_ORDER < 0) && __has_builtin(__builtin_bswap32)
  //
  return __SEGGER_RTL_CLZ_U32(__builtin_bswap32(x^y)) / 8;
  //
#elif __SEGGER_RTL_CORE_HAS_CLZ && (__SEGGER_RTL_BYTE_ORDER > 0)
  //
  return __SEGGER_RTL_CLZ_U32(x^y) / 8;
  //
#elif __SEGGER_RTL_BYTE_ORDER > 0
  //
  if ((x & 0xFF000000uL) != (y & 0xFF000000uL)) { return 0; }
  if ((x & 0x00FF0000uL) != (y & 0x00FF0000uL)) { return 1; }
  if ((x & 0x0000FF00uL) != (y & 0x0000FF00uL)) { return 2; }
  if ((x & 0x000000FFuL) != (y & 0x000000FFuL)) { return 3; }
  return 4;
  //
#else
  //
  if ((x & 0x000000FFuL) != (y & 0x000000FFuL)) { return 0; }
  if ((x & 0x0000FF00uL) != (y & 0x0000FF00uL)) { return 1; }
  if ((x & 0x00FF0000uL) != (y & 0x00FF0000uL)) { return 2; }
  if ((x & 0xFF000000uL) != (y & 0xFF000000uL)) { return 3; }
  return 4;
  //
#endif
}

static __SEGGER_RTL_ALWAYS_INLINE unsigned __SEGGER_RTL_DIFF_BYTE_func(__SEGGER_RTL_WORD x, int Index) {
#if __SEGGER_RTL_BYTE_ORDER > 0
  return (x >> (24 - 8*Index)) & 0xFF;
#else
  return (x >> (8*Index)) & 0xFF;
#endif
}

static __SEGGER_RTL_ALWAYS_INLINE unsigned __SEGGER_RTL_ZBYTE_INDEX_func(__SEGGER_RTL_WORD x) {
  //
#if __SEGGER_RTL_CORE_HAS_UQADD_UQSUB && (__SEGGER_RTL_BYTE_ORDER > 0)
  //
  return __SEGGER_RTL_CLZ_U32(__SEGGER_RTL_ZBYTE_CHECK(x)) / 8;
  //
#elif __SEGGER_RTL_CORE_HAS_UQADD_UQSUB && (__SEGGER_RTL_BYTE_ORDER < 0) && __has_builtin(__builtin_arm_rbit) && (__ARM_ARCH != 6)
  //
  return __SEGGER_RTL_CLZ_U32(__builtin_arm_rbit(__SEGGER_RTL_ZBYTE_CHECK(x))) / 8;
  //
#elif __SEGGER_RTL_CORE_HAS_UQADD_UQSUB && (__SEGGER_RTL_BYTE_ORDER < 0) && __has_builtin(__builtin_bswap32) && (__ARM_ARCH != 6)
  //
  return __SEGGER_RTL_CLZ_U32(__builtin_bswap32(__SEGGER_RTL_ZBYTE_CHECK(x))) / 8;
  //
#elif !__SEGGER_RTL_CORE_HAS_UQADD_UQSUB && __SEGGER_RTL_CORE_HAS_CLZ && (__SEGGER_RTL_BYTE_ORDER > 0) && __has_builtin(__builtin_arm_rbit)
  //
  return __SEGGER_RTL_CLZ_U32(__builtin_arm_rbit(__SEGGER_RTL_ZBYTE_CHECK(__builtin_arm_rbit(x)))) / 8;
  //
#elif !__SEGGER_RTL_CORE_HAS_UQADD_UQSUB && __SEGGER_RTL_CORE_HAS_CLZ && (__SEGGER_RTL_BYTE_ORDER > 0) && __has_builtin(__builtin_bswap32)
  //
  return __SEGGER_RTL_CLZ_U32(__builtin_bswap32(__SEGGER_RTL_ZBYTE_CHECK(__builtin_bswap32(x)))) / 8;
  //
#elif !__SEGGER_RTL_CORE_HAS_UQADD_UQSUB && __SEGGER_RTL_CORE_HAS_CLZ && (__SEGGER_RTL_BYTE_ORDER < 0) && __has_builtin(__builtin_bswap32)
  //
  return __SEGGER_RTL_CLZ_U32(__builtin_bswap32(__SEGGER_RTL_ZBYTE_CHECK(x))) / 8;
  //
#elif __SEGGER_RTL_BYTE_ORDER > 0
  //
  if ((x & 0xFF000000uL) == 0) { return 0; }
  if ((x & 0x00FF0000uL) == 0) { return 1; }
  if ((x & 0x0000FF00uL) == 0) { return 2; }
  if ((x & 0x000000FFuL) == 0) { return 3; }
  return 4;
  //
#else
  //
  if ((x & 0x000000FFuL) == 0) { return 0; }
  if ((x & 0x0000FF00uL) == 0) { return 1; }
  if ((x & 0x00FF0000uL) == 0) { return 2; }
  if ((x & 0xFF000000uL) == 0) { return 3; }
  return 4;
  //
#endif
}

static __SEGGER_RTL_ALWAYS_INLINE __SEGGER_RTL_BOOL __SEGGER_RTL_atomic_is_lock_free(unsigned size, const volatile void *ptr) {
  switch (size) {
  case 1:  return 1;                          // Bytes always lock-free
  case 2:  return ((unsigned)ptr & 1) == 0;   // Misaligned half-words are not lock free
  case 4:  return ((unsigned)ptr & 3) == 0;   // Misaligned words are not lock free
  default: return 0;                          // 8-byte objects (e.g. doubles, long long) are not lock free.
  }
}

#endif

//
// Configuration of signals according to AEABI portability.
//

#define __SEGGER_RTL_SIGNAL_MAX 6

#ifndef __SEGGER_RTL_EXCLUDE_STATIC_FUNCTIONS
extern const int __aeabi_SIGABRT;
extern const int __aeabi_SIGINT;
extern const int __aeabi_SIGILL;
extern const int __aeabi_SIGFPE;
extern const int __aeabi_SIGSEGV;
extern const int __aeabi_SIGTERM;

void __aeabi_SIG_DFL(int sig);
void __aeabi_SIG_IGN(int sig);
void __aeabi_SIG_ERR(int sig);
#endif

#define __SEGGER_RTL_SIGNAL_SIGABRT             __aeabi_SIGABRT
#define __SEGGER_RTL_SIGNAL_SIGFPE              __aeabi_SIGFPE
#define __SEGGER_RTL_SIGNAL_SIGILL              __aeabi_SIGILL
#define __SEGGER_RTL_SIGNAL_SIGINT              __aeabi_SIGINT
#define __SEGGER_RTL_SIGNAL_SIGSEGV             __aeabi_SIGSEGV
#define __SEGGER_RTL_SIGNAL_SIGTERM             __aeabi_SIGTERM

#define __SEGGER_RTL_SIGNAL_SIG_DFL             __aeabi_SIG_DFL
#define __SEGGER_RTL_SIGNAL_SIG_ERR             __aeabi_SIG_ERR
#define __SEGGER_RTL_SIGNAL_SIG_IGN             __aeabi_SIG_IGN

#ifdef __cplusplus
}
#endif

#endif

/*************************** End of file ****************************/
