/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Purpose: Real-time O(1) memory allocator.

*/

#ifndef __SEGGER_RTL_RTHEAP_CONFDEFAULTS_H
#define __SEGGER_RTL_RTHEAP_CONFDEFAULTS_H

#include "__SEGGER_RTL_RTHEAP_Conf.h"

#ifndef __SEGGER_RTL_RTHEAP_ADDRSIZE
  #error __SEGGER_RTL_RTHEAP_ADDRSIZE must be configured!
#endif

#if (__SEGGER_RTL_RTHEAP_ADDRSIZE != 32) && (__SEGGER_RTL_RTHEAP_ADDRSIZE != 64)
  #error __SEGGER_RTL_RTHEAP_ADDRSIZE can only be 32 or 64
#endif

//
// Default block alignment.
//
#if !defined(__SEGGER_RTL_RTHEAP_ALIGN_BITS) && (__SEGGER_RTL_RTHEAP_ADDRSIZE == 64)
  #define __SEGGER_RTL_RTHEAP_ALIGN_BITS             3   // Align modulo 8
#elif !defined(__SEGGER_RTL_RTHEAP_ALIGN_BITS) && (__SEGGER_RTL_RTHEAP_ADDRSIZE == 32)
  #define __SEGGER_RTL_RTHEAP_ALIGN_BITS             2   // Align modulo 4
#endif

//
// Managed memory size.
//
#if !defined(__SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS) && (__SEGGER_RTL_RTHEAP_ADDRSIZE == 64)
  #define __SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS      32
#endif
#if !defined(__SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS) && (__SEGGER_RTL_RTHEAP_ADDRSIZE == 32)
  #define __SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS      30
#endif

//
// Enforce hard limits.
//
#if (__SEGGER_RTL_RTHEAP_ADDRSIZE == 64) && (__SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS > 32)
  #error Invalid configuration of managed heap size: too big!
#endif
#if (__SEGGER_RTL_RTHEAP_ADDRSIZE == 32) && (__SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS > 30)
  #error Invalid configuration of managed heap size: too big!
#endif

//
// Number of bits in the secondary index.  Typically 4 or 5.
//
#ifndef   __SEGGER_RTL_RTHEAP_L2_INDEX_BITS
  #define __SEGGER_RTL_RTHEAP_L2_INDEX_BITS          5
#endif

//
// Total number of bits in the L2 index (including redundant bits subsumed into small blocks).
//
#define __SEGGER_RTL_RTHEAP_L2_MAX_BITS              (__SEGGER_RTL_RTHEAP_L2_INDEX_BITS + __SEGGER_RTL_RTHEAP_ALIGN_BITS)

//
// Number of supported bits in L1 index.
//
#define __SEGGER_RTL_RTHEAP_L1_INDEX_BITS            (__SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS - __SEGGER_RTL_RTHEAP_L2_MAX_BITS + 1)

//
// Blocks <= this size are subsumed into a single free list.
//
#define __SEGGER_RTL_RTHEAP_L2_MAX_SZ                (1u << __SEGGER_RTL_RTHEAP_L2_MAX_BITS)

//
// Size of L2 indexes.
//
#define __SEGGER_RTL_RTHEAP_L2_INDEX_SZ              (1 << __SEGGER_RTL_RTHEAP_L2_INDEX_BITS)

//
// Minimal alignment size for blocks.
//
#define __SEGGER_RTL_RTHEAP_ALIGN_SZ                 (1 << __SEGGER_RTL_RTHEAP_ALIGN_BITS)

//
// Simple static check of constants.
//
#if __SEGGER_RTL_RTHEAP_ALIGN_SZ != (__SEGGER_RTL_RTHEAP_L2_MAX_SZ >> __SEGGER_RTL_RTHEAP_L2_INDEX_BITS)
  #error Bad configuration!
#endif

//
// Configuration of aligned units.
//
#ifndef   __SEGGER_RTL_RTHEAP_ALIGNED_UNIT
  #define __SEGGER_RTL_RTHEAP_ALIGNED_UNIT           __SEGGER_RTL_RTHEAP_ALIGNED_UNIT_t
  #define __SEGGER_RTL_RTHEAP_ALIGNED_UNIT_REPL(X)   ((X) * 0x01010101u)
  typedef unsigned                         __SEGGER_RTL_RTHEAP_ALIGNED_UNIT_t;
#endif

//
// Configuration of inlining.
//
#ifndef   __SEGGER_RTL_RTHEAP_INLINE
  #define __SEGGER_RTL_RTHEAP_INLINE
#endif

//
// Debugging level:
//   0: No checks, no traps.
//   1: Check incoming API parameters for validity.
//   2: As (1) but include internal checks for correct operation.
//   3: As (2) but include memory initialization.
//
#ifndef   __SEGGER_RTL_RTHEAP_DEBUG_LEVEL
  #define __SEGGER_RTL_RTHEAP_DEBUG_LEVEL            0
#endif

//
// Enforce bailout when checks enabled.
//
#if (__SEGGER_RTL_RTHEAP_DEBUG_LEVEL > 0) && !defined(__SEGGER_RTL_RTHEAP_PANIC)
  #error __SEGGER_RTL_RTHEAP_PANIC must be defined!
#endif

//
// Default size type.
//
#ifndef   __SEGGER_RTL_RTHEAP_SIZE
  #include <stddef.h>
  #define __SEGGER_RTL_RTHEAP_SIZE                   size_t
#endif

//
// Default fill values indicating heap memory state.
//
#ifndef   __SEGGER_RTL_RTHEAP_FILL_ALLOCATED
  #define __SEGGER_RTL_RTHEAP_FILL_ALLOCATED         0xAA
#endif
#ifndef   __SEGGER_RTL_RTHEAP_FILL_FREED
  #define __SEGGER_RTL_RTHEAP_FILL_FREED             0xDD
#endif
#ifndef   __SEGGER_RTL_RTHEAP_FILL_UNTOUCHED
  #define __SEGGER_RTL_RTHEAP_FILL_UNTOUCHED         0xBB
#endif

//
// Default bit searches.
//
#ifndef   __SEGGER_RTL_RTHEAP_FFS
  #define __SEGGER_RTL_RTHEAP_FFS                    __SEGGER_RTL_RTHEAP_FFS_func
  #define __SEGGER_RTL_RTHEAP_FFS_SYNTHESIZED
#endif
#ifndef   __SEGGER_RTL_RTHEAP_FLS
  #define __SEGGER_RTL_RTHEAP_FLS                    __SEGGER_RTL_RTHEAP_FLS_func
  #define __SEGGER_RTL_RTHEAP_FLS_SYNTHESIZED
#endif
#if (__SEGGER_RTL_RTHEAP_ADDRSIZE == 64) && !defined(__SEGGER_RTL_RTHEAP_FLSL)
  #define __SEGGER_RTL_RTHEAP_FLSL                   __SEGGER_RTL_RTHEAP_FLSL_func
  #define __SEGGER_RTL_RTHEAP_FLSL_SYNTHESIZED
#endif
#if (__SEGGER_RTL_RTHEAP_ADDRSIZE == 32) && !defined(__SEGGER_RTL_RTHEAP_FLSL)
  #define __SEGGER_RTL_RTHEAP_FLSL                   __SEGGER_RTL_RTHEAP_FLS
#endif

#ifndef __SEGGER_RTL_RTHEAP_MEMMOVE
  #define __SEGGER_RTL_RTHEAP_MEMMOVE(T, F, L)       __SEGGER_RTL_RTHEAP_MEMMOVE_func(T, F, L)
  #define __SEGGER_RTL_RTHEAP_MEMMOVE_SYNTHESIZED
#endif

#if !defined(__SEGGER_RTL_RTHEAP_MEMSET)
  #define __SEGGER_RTL_RTHEAP_MEMSET(A, V, L)        __SEGGER_RTL_RTHEAP_MEMSET_func(A, V, L)
  #define __SEGGER_RTL_RTHEAP_MEMSET_SYNTHESIZED
#endif

#endif

/*************************** End of file ****************************/
