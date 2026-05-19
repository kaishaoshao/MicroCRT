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

#include "__SEGGER_RTL_Int.h"
#include "string.h"
#include "stdint.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

// Clang and SEGGER-CC both reject definitions of built-in functions
// in order to provide their implementation.  This insane-looking set
// of pragmas works around that limitation, enabling all functions to
// be written in C rather than assembly language.

#if defined(__clang__) || defined(__SEGGER_CC__)
  //
  #pragma redefine_extname   __sync_synchronize_c              __sync_synchronize
  #define                    __sync_synchronize                __sync_synchronize_c
  //
  #pragma redefine_extname   __sync_fetch_and_add_1_c          __sync_fetch_and_add_1
  #pragma redefine_extname   __sync_fetch_and_add_2_c          __sync_fetch_and_add_2
  #pragma redefine_extname   __sync_fetch_and_add_4_c          __sync_fetch_and_add_4
  #pragma redefine_extname   __sync_fetch_and_add_8_c          __sync_fetch_and_add_8
  #pragma redefine_extname   __sync_fetch_and_add_16_c         __sync_fetch_and_add_16
  #define                    __sync_fetch_and_add_1            __sync_fetch_and_add_1_c
  #define                    __sync_fetch_and_add_2            __sync_fetch_and_add_2_c
  #define                    __sync_fetch_and_add_4            __sync_fetch_and_add_4_c
  #define                    __sync_fetch_and_add_8            __sync_fetch_and_add_8_c
  #define                    __sync_fetch_and_add_16           __sync_fetch_and_add_16_c
  //
  #pragma redefine_extname   __sync_fetch_and_sub_1_c          __sync_fetch_and_sub_1
  #pragma redefine_extname   __sync_fetch_and_sub_2_c          __sync_fetch_and_sub_2
  #pragma redefine_extname   __sync_fetch_and_sub_4_c          __sync_fetch_and_sub_4
  #pragma redefine_extname   __sync_fetch_and_sub_8_c          __sync_fetch_and_sub_8
  #pragma redefine_extname   __sync_fetch_and_sub_16_c         __sync_fetch_and_sub_16
  #define                    __sync_fetch_and_sub_1            __sync_fetch_and_sub_1_c
  #define                    __sync_fetch_and_sub_2            __sync_fetch_and_sub_2_c
  #define                    __sync_fetch_and_sub_4            __sync_fetch_and_sub_4_c
  #define                    __sync_fetch_and_sub_8            __sync_fetch_and_sub_8_c
  #define                    __sync_fetch_and_sub_16           __sync_fetch_and_sub_16_c
  //
  #pragma redefine_extname   __sync_fetch_and_and_1_c          __sync_fetch_and_and_1
  #pragma redefine_extname   __sync_fetch_and_and_2_c          __sync_fetch_and_and_2
  #pragma redefine_extname   __sync_fetch_and_and_4_c          __sync_fetch_and_and_4
  #pragma redefine_extname   __sync_fetch_and_and_8_c          __sync_fetch_and_and_8
  #pragma redefine_extname   __sync_fetch_and_and_16_c         __sync_fetch_and_and_16
  #define                    __sync_fetch_and_and_1            __sync_fetch_and_and_1_c
  #define                    __sync_fetch_and_and_2            __sync_fetch_and_and_2_c
  #define                    __sync_fetch_and_and_4            __sync_fetch_and_and_4_c
  #define                    __sync_fetch_and_and_8            __sync_fetch_and_and_8_c
  #define                    __sync_fetch_and_and_16           __sync_fetch_and_and_16_c
  //
  #pragma redefine_extname   __sync_fetch_and_or_1_c           __sync_fetch_and_or_1
  #pragma redefine_extname   __sync_fetch_and_or_2_c           __sync_fetch_and_or_2
  #pragma redefine_extname   __sync_fetch_and_or_4_c           __sync_fetch_and_or_4
  #pragma redefine_extname   __sync_fetch_and_or_8_c           __sync_fetch_and_or_8
  #pragma redefine_extname   __sync_fetch_and_or_16_c          __sync_fetch_and_or_16
  #define                    __sync_fetch_and_or_1             __sync_fetch_and_or_1_c
  #define                    __sync_fetch_and_or_2             __sync_fetch_and_or_2_c
  #define                    __sync_fetch_and_or_4             __sync_fetch_and_or_4_c
  #define                    __sync_fetch_and_or_8             __sync_fetch_and_or_8_c
  #define                    __sync_fetch_and_or_16            __sync_fetch_and_or_16_c
  //
  #pragma redefine_extname   __sync_fetch_and_xor_1_c          __sync_fetch_and_xor_1
  #pragma redefine_extname   __sync_fetch_and_xor_2_c          __sync_fetch_and_xor_2
  #pragma redefine_extname   __sync_fetch_and_xor_4_c          __sync_fetch_and_xor_4
  #pragma redefine_extname   __sync_fetch_and_xor_8_c          __sync_fetch_and_xor_8
  #pragma redefine_extname   __sync_fetch_and_xor_16_c         __sync_fetch_and_xor_16
  #define                    __sync_fetch_and_xor_1            __sync_fetch_and_xor_1_c
  #define                    __sync_fetch_and_xor_2            __sync_fetch_and_xor_2_c
  #define                    __sync_fetch_and_xor_4            __sync_fetch_and_xor_4_c
  #define                    __sync_fetch_and_xor_8            __sync_fetch_and_xor_8_c
  #define                    __sync_fetch_and_xor_16           __sync_fetch_and_xor_16_c
  //
  #pragma redefine_extname   __sync_fetch_and_nand_1_c         __sync_fetch_and_nand_1
  #pragma redefine_extname   __sync_fetch_and_nand_2_c         __sync_fetch_and_nand_2
  #pragma redefine_extname   __sync_fetch_and_nand_4_c         __sync_fetch_and_nand_4
  #pragma redefine_extname   __sync_fetch_and_nand_8_c         __sync_fetch_and_nand_8
  #pragma redefine_extname   __sync_fetch_and_nand_16_c        __sync_fetch_and_nand_16
  #define                    __sync_fetch_and_nand_1           __sync_fetch_and_nand_1_c
  #define                    __sync_fetch_and_nand_2           __sync_fetch_and_nand_2_c
  #define                    __sync_fetch_and_nand_4           __sync_fetch_and_nand_4_c
  #define                    __sync_fetch_and_nand_8           __sync_fetch_and_nand_8_c
  #define                    __sync_fetch_and_nand_16          __sync_fetch_and_nand_16_c
  //
  #pragma redefine_extname   __sync_add_and_fetch_1_c          __sync_add_and_fetch_1
  #pragma redefine_extname   __sync_add_and_fetch_2_c          __sync_add_and_fetch_2
  #pragma redefine_extname   __sync_add_and_fetch_4_c          __sync_add_and_fetch_4
  #pragma redefine_extname   __sync_add_and_fetch_8_c          __sync_add_and_fetch_8
  #pragma redefine_extname   __sync_add_and_fetch_16_c         __sync_add_and_fetch_16
  #define                    __sync_add_and_fetch_1            __sync_add_and_fetch_1_c
  #define                    __sync_add_and_fetch_2            __sync_add_and_fetch_2_c
  #define                    __sync_add_and_fetch_4            __sync_add_and_fetch_4_c
  #define                    __sync_add_and_fetch_8            __sync_add_and_fetch_8_c
  #define                    __sync_add_and_fetch_16           __sync_add_and_fetch_16_c
  //
  #pragma redefine_extname   __sync_sub_and_fetch_1_c          __sync_sub_and_fetch_1
  #pragma redefine_extname   __sync_sub_and_fetch_2_c          __sync_sub_and_fetch_2
  #pragma redefine_extname   __sync_sub_and_fetch_4_c          __sync_sub_and_fetch_4
  #pragma redefine_extname   __sync_sub_and_fetch_8_c          __sync_sub_and_fetch_8
  #pragma redefine_extname   __sync_sub_and_fetch_16_c         __sync_sub_and_fetch_16
  #define                    __sync_sub_and_fetch_1            __sync_sub_and_fetch_1_c
  #define                    __sync_sub_and_fetch_2            __sync_sub_and_fetch_2_c
  #define                    __sync_sub_and_fetch_4            __sync_sub_and_fetch_4_c
  #define                    __sync_sub_and_fetch_8            __sync_sub_and_fetch_8_c
  #define                    __sync_sub_and_fetch_16           __sync_sub_and_fetch_16_c
  //
  #pragma redefine_extname   __sync_and_and_fetch_1_c          __sync_and_and_fetch_1
  #pragma redefine_extname   __sync_and_and_fetch_2_c          __sync_and_and_fetch_2
  #pragma redefine_extname   __sync_and_and_fetch_4_c          __sync_and_and_fetch_4
  #pragma redefine_extname   __sync_and_and_fetch_8_c          __sync_and_and_fetch_8
  #pragma redefine_extname   __sync_and_and_fetch_16_c         __sync_and_and_fetch_16
  #define                    __sync_and_and_fetch_1            __sync_and_and_fetch_1_c
  #define                    __sync_and_and_fetch_2            __sync_and_and_fetch_2_c
  #define                    __sync_and_and_fetch_4            __sync_and_and_fetch_4_c
  #define                    __sync_and_and_fetch_8            __sync_and_and_fetch_8_c
  #define                    __sync_and_and_fetch_16           __sync_and_and_fetch_16_c
  //
  #pragma redefine_extname   __sync_or_and_fetch_1_c           __sync_or_and_fetch_1
  #pragma redefine_extname   __sync_or_and_fetch_2_c           __sync_or_and_fetch_2
  #pragma redefine_extname   __sync_or_and_fetch_4_c           __sync_or_and_fetch_4
  #pragma redefine_extname   __sync_or_and_fetch_8_c           __sync_or_and_fetch_8
  #pragma redefine_extname   __sync_or_and_fetch_16_c          __sync_or_and_fetch_16
  #define                    __sync_or_and_fetch_1             __sync_or_and_fetch_1_c
  #define                    __sync_or_and_fetch_2             __sync_or_and_fetch_2_c
  #define                    __sync_or_and_fetch_4             __sync_or_and_fetch_4_c
  #define                    __sync_or_and_fetch_8             __sync_or_and_fetch_8_c
  #define                    __sync_or_and_fetch_16            __sync_or_and_fetch_16_c
  //
  #pragma redefine_extname   __sync_xor_and_fetch_1_c          __sync_xor_and_fetch_1
  #pragma redefine_extname   __sync_xor_and_fetch_2_c          __sync_xor_and_fetch_2
  #pragma redefine_extname   __sync_xor_and_fetch_4_c          __sync_xor_and_fetch_4
  #pragma redefine_extname   __sync_xor_and_fetch_8_c          __sync_xor_and_fetch_8
  #pragma redefine_extname   __sync_xor_and_fetch_16_c         __sync_xor_and_fetch_16
  #define                    __sync_xor_and_fetch_1            __sync_xor_and_fetch_1_c
  #define                    __sync_xor_and_fetch_2            __sync_xor_and_fetch_2_c
  #define                    __sync_xor_and_fetch_4            __sync_xor_and_fetch_4_c
  #define                    __sync_xor_and_fetch_8            __sync_xor_and_fetch_8_c
  #define                    __sync_xor_and_fetch_16           __sync_xor_and_fetch_16_c
  //
  #pragma redefine_extname   __sync_nand_and_fetch_1_c         __sync_nand_and_fetch_1
  #pragma redefine_extname   __sync_nand_and_fetch_2_c         __sync_nand_and_fetch_2
  #pragma redefine_extname   __sync_nand_and_fetch_4_c         __sync_nand_and_fetch_4
  #pragma redefine_extname   __sync_nand_and_fetch_8_c         __sync_nand_and_fetch_8
  #pragma redefine_extname   __sync_nand_and_fetch_16_c        __sync_nand_and_fetch_16
  #define                    __sync_nand_and_fetch_1           __sync_nand_and_fetch_1_c
  #define                    __sync_nand_and_fetch_2           __sync_nand_and_fetch_2_c
  #define                    __sync_nand_and_fetch_4           __sync_nand_and_fetch_4_c
  #define                    __sync_nand_and_fetch_8           __sync_nand_and_fetch_8_c
  #define                    __sync_nand_and_fetch_16          __sync_nand_and_fetch_16_c
  //
  #pragma redefine_extname   __sync_bool_compare_and_swap_1_c  __sync_bool_compare_and_swap_1
  #pragma redefine_extname   __sync_bool_compare_and_swap_2_c  __sync_bool_compare_and_swap_2
  #pragma redefine_extname   __sync_bool_compare_and_swap_4_c  __sync_bool_compare_and_swap_4
  #pragma redefine_extname   __sync_bool_compare_and_swap_8_c  __sync_bool_compare_and_swap_8
  #pragma redefine_extname   __sync_bool_compare_and_swap_16_c __sync_bool_compare_and_swap_16
  #define                    __sync_bool_compare_and_swap_1    __sync_bool_compare_and_swap_1_c
  #define                    __sync_bool_compare_and_swap_2    __sync_bool_compare_and_swap_2_c
  #define                    __sync_bool_compare_and_swap_4    __sync_bool_compare_and_swap_4_c
  #define                    __sync_bool_compare_and_swap_8    __sync_bool_compare_and_swap_8_c
  #define                    __sync_bool_compare_and_swap_16   __sync_bool_compare_and_swap_16_c
  //
  #pragma redefine_extname   __sync_val_compare_and_swap_1_c   __sync_val_compare_and_swap_1
  #pragma redefine_extname   __sync_val_compare_and_swap_2_c   __sync_val_compare_and_swap_2
  #pragma redefine_extname   __sync_val_compare_and_swap_4_c   __sync_val_compare_and_swap_4
  #pragma redefine_extname   __sync_val_compare_and_swap_8_c   __sync_val_compare_and_swap_8
  #pragma redefine_extname   __sync_val_compare_and_swap_16_c  __sync_val_compare_and_swap_16
  #define                    __sync_val_compare_and_swap_1     __sync_val_compare_and_swap_1_c
  #define                    __sync_val_compare_and_swap_2     __sync_val_compare_and_swap_2_c
  #define                    __sync_val_compare_and_swap_4     __sync_val_compare_and_swap_4_c
  #define                    __sync_val_compare_and_swap_8     __sync_val_compare_and_swap_8_c
  #define                    __sync_val_compare_and_swap_16    __sync_val_compare_and_swap_16_c
  //
  #pragma redefine_extname   __sync_lock_test_and_set_1_c      __sync_lock_test_and_set_1
  #pragma redefine_extname   __sync_lock_test_and_set_2_c      __sync_lock_test_and_set_2
  #pragma redefine_extname   __sync_lock_test_and_set_4_c      __sync_lock_test_and_set_4
  #pragma redefine_extname   __sync_lock_test_and_set_8_c      __sync_lock_test_and_set_8
  #pragma redefine_extname   __sync_lock_test_and_set_16_c     __sync_lock_test_and_set_16
  #define                    __sync_lock_test_and_set_1        __sync_lock_test_and_set_1_c
  #define                    __sync_lock_test_and_set_2        __sync_lock_test_and_set_2_c
  #define                    __sync_lock_test_and_set_4        __sync_lock_test_and_set_4_c
  #define                    __sync_lock_test_and_set_8        __sync_lock_test_and_set_8_c
  #define                    __sync_lock_test_and_set_16       __sync_lock_test_and_set_16_c
  //
  #pragma redefine_extname   __sync_lock_release_1_c           __sync_lock_release_1
  #pragma redefine_extname   __sync_lock_release_2_c           __sync_lock_release_2
  #pragma redefine_extname   __sync_lock_release_4_c           __sync_lock_release_4
  #pragma redefine_extname   __sync_lock_release_8_c           __sync_lock_release_8
  #pragma redefine_extname   __sync_lock_release_16_c          __sync_lock_release_16
  #define                    __sync_lock_release_1             __sync_lock_release_1_c
  #define                    __sync_lock_release_2             __sync_lock_release_2_c
  #define                    __sync_lock_release_4             __sync_lock_release_4_c
  #define                    __sync_lock_release_8             __sync_lock_release_8_c
  #define                    __sync_lock_release_16            __sync_lock_release_16_c
  //  
  #pragma redefine_extname   __atomic_load_c                   __atomic_load
  #define                    __atomic_load                     __atomic_load_c
  //
  #pragma redefine_extname   __atomic_store_c                  __atomic_store
  #define                    __atomic_store                    __atomic_store_c
  //
  #pragma redefine_extname   __atomic_is_lock_free_c           __atomic_is_lock_free
  #define                    __atomic_is_lock_free             __atomic_is_lock_free_c
  //
  #pragma redefine_extname   __atomic_load_1_c                 __atomic_load_1
  #pragma redefine_extname   __atomic_load_2_c                 __atomic_load_2
  #pragma redefine_extname   __atomic_load_4_c                 __atomic_load_4
  #pragma redefine_extname   __atomic_load_8_c                 __atomic_load_8
  #pragma redefine_extname   __atomic_load_16_c                __atomic_load_16
  #define                    __atomic_load_1                   __atomic_load_1_c
  #define                    __atomic_load_2                   __atomic_load_2_c
  #define                    __atomic_load_4                   __atomic_load_4_c
  #define                    __atomic_load_8                   __atomic_load_8_c
  #define                    __atomic_load_16                  __atomic_load_16_c
  //
  #pragma redefine_extname   __atomic_store_1_c                __atomic_store_1
  #pragma redefine_extname   __atomic_store_2_c                __atomic_store_2
  #pragma redefine_extname   __atomic_store_4_c                __atomic_store_4
  #pragma redefine_extname   __atomic_store_8_c                __atomic_store_8
  #pragma redefine_extname   __atomic_store_16_c               __atomic_store_16
  #define                    __atomic_store_1                  __atomic_store_1_c
  #define                    __atomic_store_2                  __atomic_store_2_c
  #define                    __atomic_store_4                  __atomic_store_4_c
  #define                    __atomic_store_8                  __atomic_store_8_c
  #define                    __atomic_store_16                 __atomic_store_16_c
  //
  #pragma redefine_extname   __atomic_exchange_c              __atomic_exchange
  #define                    __atomic_exchange                __atomic_exchange_c
  //
  #pragma redefine_extname   __atomic_compare_exchange_c      __atomic_compare_exchange
  #define                    __atomic_compare_exchange        __atomic_compare_exchange_c
#endif

#define SYNC_FETCH_AND_OP(PTR, VAL, TYPE, OP)                 \
  TYPE ret;                                                   \
  int  en;                                                    \
  en   = __SEGGER_RTL_ATOMIC_LOCK();                          \
  ret  = *(volatile TYPE *)(PTR);                             \
  *(volatile TYPE *)(PTR) = OP;                               \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define SYNC_OP_AND_FETCH(PTR, VAL, TYPE, OP)                 \
  TYPE ret;                                                   \
  int  en;                                                    \
  en   = __SEGGER_RTL_ATOMIC_LOCK();                          \
  ret  = *(volatile TYPE *)(PTR);                             \
  ret  = (TYPE)(OP);                                          \
  *(volatile TYPE *)(PTR) = ret;                              \
  ret  = *(volatile TYPE *)(PTR);                             \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define SYNC_BOOL_COMPARE_AND_SWAP(PTR, OLDVAL, NEWVAL, TYPE) \
  __SEGGER_RTL_BOOL ret;                                      \
  int               en;                                       \
  en = __SEGGER_RTL_ATOMIC_LOCK();                            \
  if (*(volatile TYPE *)ptr == (OLDVAL)) {                    \
    *(volatile TYPE *)ptr = (NEWVAL);                         \
    ret  = 1;                                                 \
  } else {                                                    \
    ret = 0;                                                  \
  }                                                           \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define SYNC_VAL_COMPARE_AND_SWAP(PTR, OLDVAL, NEWVAL, TYPE)  \
  int  en;                                                    \
  TYPE ret;                                                   \
  en = __SEGGER_RTL_ATOMIC_LOCK();                            \
  ret = *(volatile TYPE *)(PTR);                              \
  if (ret == (OLDVAL)) {                                      \
    *(volatile TYPE *)(PTR) = (NEWVAL);                       \
  }                                                           \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define SYNC_LOCK_TEST_AND_SET(PTR, VAL, TYPE)                \
  int  en;                                                    \
  TYPE ret;                                                   \
  en   = __SEGGER_RTL_ATOMIC_LOCK();                          \
  ret  = *(volatile TYPE *)(PTR);                             \
  *(volatile TYPE *)(PTR) = (VAL);                            \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define SYNC_LOCK_RELEASE(PTR, TYPE)                          \
  *(volatile TYPE *)(PTR) = 0;

#define ATOMIC_LOAD(PTR, MEMORDER, TYPE)                      \
  int  en;                                                    \
  TYPE ret;                                                   \
  __SEGGER_RTL_USE_PARA(MEMORDER);                            \
  en = __SEGGER_RTL_ATOMIC_LOCK();                            \
  ret = *(volatile TYPE *)(PTR);                              \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define ATOMIC_STORE(PTR, VAL, MEMORDER, TYPE)                \
  int  en;                                                    \
  __SEGGER_RTL_USE_PARA(MEMORDER);                            \
  en = __SEGGER_RTL_ATOMIC_LOCK();                            \
  *(volatile TYPE *)(PTR) = (VAL);                            \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);

#define ATOMIC_OP_FETCH(PTR, VAL, MEMORDER, TYPE, OP)         \
  TYPE ret;                                                   \
  int  en;                                                    \
  __SEGGER_RTL_USE_PARA(MEMORDER);                            \
  en  = __SEGGER_RTL_ATOMIC_LOCK();                           \
  ret = *(volatile TYPE *)(PTR);                              \
  ret = OP;                                                   \
  *(volatile TYPE *)(PTR) = ret;                              \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define ATOMIC_FETCH_AND_OP(PTR, VAL, MEMORDER, TYPE, OP)     \
  TYPE ret;                                                   \
  int  en;                                                    \
  __SEGGER_RTL_USE_PARA(MEMORDER);                            \
  en  = __SEGGER_RTL_ATOMIC_LOCK();                           \
  ret = *(volatile TYPE *)(PTR);                              \
  *(volatile TYPE *)(PTR) = (TYPE)(OP);                       \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define ATOMIC_EXCHANGE(PTR, VAL, MEMORDER, TYPE)             \
  TYPE ret;                                                   \
  int  en;                                                    \
  __SEGGER_RTL_USE_PARA(MEMORDER);                            \
  en  = __SEGGER_RTL_ATOMIC_LOCK();                           \
  ret = *(volatile TYPE *)(PTR);                              \
  *(volatile TYPE *)(PTR) = (VAL);                            \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

#define ATOMIC_COMPARE_EXCHANGE(PTR, EXPECTED, DESIRED, WEAK, SUCC, FAIL, TYPE) \
  __SEGGER_RTL_BOOL ret;                                      \
  int               en;                                       \
  TYPE              act;                                      \
  __SEGGER_RTL_USE_PARA(SUCC);                                \
  __SEGGER_RTL_USE_PARA(FAIL);                                \
  __SEGGER_RTL_USE_PARA(WEAK);                                \
  en  = __SEGGER_RTL_ATOMIC_LOCK();                           \
  act = *(volatile TYPE *)(PTR);                              \
  if (act == *(TYPE *)(EXPECTED)) {                           \
    *(volatile TYPE *)(PTR) = (DESIRED);                      \
    ret = 1;                                                  \
  } else {                                                    \
    *(TYPE *)(EXPECTED) = act;                                \
    ret = 0;                                                  \
  }                                                           \
  __SEGGER_RTL_ATOMIC_UNLOCK(en);                             \
  return ret;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __sync_synchronize()
*
*  Function description
*    Issue a full memory barrier.
*/
void __sync_synchronize(void) {
  __SEGGER_RTL_ATOMIC_SYNCHRONIZE();
}

/*********************************************************************
*
*       __sync_fetch_and_add_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U8 __sync_fetch_and_add_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U8, ret + value)
}

/*********************************************************************
*
*       __sync_fetch_and_add_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_add_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U16, ret + value)
}

/*********************************************************************
*
*       __sync_fetch_and_add_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_add_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret + value)
}

/*********************************************************************
*
*       __sync_fetch_and_add_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_add_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U64, ret + value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_fetch_and_add_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_add_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U128, ret + value)
}
#endif

/*********************************************************************
*
*       __sync_fetch_and_sub_1()
*
*  Function description
*    Atomic fetch with decrement, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U8 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_sub_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U8, ret - value)
}

/*********************************************************************
*
*       __sync_fetch_and_sub_2()
*
*  Function description
*    Atomic fetch with decrement, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_sub_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U16, ret - value)
}

/*********************************************************************
*
*       __sync_fetch_and_sub_4()
*
*  Function description
*    Atomic fetch with decrement, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_sub_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret - value)
}

/*********************************************************************
*
*       __sync_fetch_and_sub_8()
*
*  Function description
*    Atomic fetch with decrement, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_sub_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U64, ret - value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_fetch_and_sub_16()
*
*  Function description
*    Atomic fetch with decrement, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_sub_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U128, ret - value)
}
#endif

/*********************************************************************
*
*       __sync_fetch_and_and_1()
*
*  Function description
*    Atomic fetch with bitwise-and, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U8 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_and_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U8, ret & value)
}

/*********************************************************************
*
*       __sync_fetch_and_and_2()
*
*  Function description
*    Atomic fetch with bitwise-and, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_and_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U16, ret & value)
}

/*********************************************************************
*
*       __sync_fetch_and_and_4()
*
*  Function description
*    Atomic fetch with bitwise-and, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_and_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret & value)
}

/*********************************************************************
*
*       __sync_fetch_and_and_8()
*
*  Function description
*    Atomic fetch with bitwise-and, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_and_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U64, ret & value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_fetch_and_and_16()
*
*  Function description
*    Atomic fetch with bitwise-and, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_and_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U128, ret & value)
}
#endif

/*********************************************************************
*
*       __sync_fetch_and_or_1()
*
*  Function description
*    Atomic fetch with bitwise-or, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U8 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_or_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U8, ret | value)
}

/*********************************************************************
*
*       __sync_fetch_and_or_2()
*
*  Function description
*    Atomic fetch with bitwise-or, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_or_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U16, ret | value)
}

/*********************************************************************
*
*       __sync_fetch_and_or_4()
*
*  Function description
*    Atomic fetch with bitwise-or, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_or_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret | value)
}

/*********************************************************************
*
*       __sync_fetch_and_or_8()
*
*  Function description
*    Atomic fetch with bitwise-or, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_or_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U64, ret | value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_fetch_and_or_16()
*
*  Function description
*    Atomic fetch with bitwise-or, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_or_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U128, ret | value)
}
#endif

/*********************************************************************
*
*       __sync_fetch_and_xor_1()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U8 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_xor_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U8, ret ^ value)
}

/*********************************************************************
*
*       __sync_fetch_and_xor_2()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_xor_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U16, ret ^ value)
}

/*********************************************************************
*
*       __sync_fetch_and_xor_4()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_xor_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret ^ value)
}

/*********************************************************************
*
*       __sync_fetch_and_xor_8()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_xor_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U64, ret ^ value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_fetch_and_xor_16()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_xor_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U128, ret ^ value)
}
#endif

/*********************************************************************
*
*       __sync_fetch_and_nand_1()
*
*  Function description
*    Atomic fetch with bitwise-nand, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U8 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_nand_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U8, ~(ret & value))
}

/*********************************************************************
*
*       __sync_fetch_and_nand_2()
*
*  Function description
*    Atomic fetch with bitwise-nand, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_nand_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U16, ~(ret & value))
}

/*********************************************************************
*
*       __sync_fetch_and_nand_4()
*
*  Function description
*    Atomic fetch with bitwise-nand, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_nand_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_ATOMIC_U32, ~(ret & value))
}

/*********************************************************************
*
*       __sync_fetch_and_nand_8()
*
*  Function description
*    Atomic fetch with bitwise-nand, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_nand_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U64, ~(ret & value))
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_fetch_and_nand_16()
*
*  Function description
*    Atomic fetch with bitwise-nand, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_fetch_and_nand_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_FETCH_AND_OP(ptr, value, __SEGGER_RTL_U128, ~(ret & value))
}
#endif

/*********************************************************************
*
*       __sync_add_and_fetch_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __sync_add_and_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U8, ret + value)
}

/*********************************************************************
*
*       __sync_add_and_fetch_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_add_and_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U16, ret + value)
}

/*********************************************************************
*
*       __sync_add_and_fetch_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_add_and_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret + value)
}

/*********************************************************************
*
*       __sync_add_and_fetch_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_add_and_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U64, ret + value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_add_and_fetch_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_add_and_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U128, ret + value)
}
#endif

/*********************************************************************
*
*       __sync_sub_and_fetch_1()
*
*  Function description
*    Atomic fetch with decrement, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __sync_sub_and_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U8, ret - value)
}

/*********************************************************************
*
*       __sync_sub_and_fetch_2()
*
*  Function description
*    Atomic fetch with decrement, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_sub_and_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U16, ret - value)
}

/*********************************************************************
*
*       __sync_sub_and_fetch_4()
*
*  Function description
*    Atomic fetch with decrement, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_sub_and_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret - value)
}

/*********************************************************************
*
*       __sync_sub_and_fetch_8()
*
*  Function description
*    Atomic fetch with decrement, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_sub_and_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U64, ret - value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_sub_and_fetch_16()
*
*  Function description
*    Atomic fetch with decrement, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_sub_and_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U16, ret - value)
}
#endif

/*********************************************************************
*
*       __sync_and_and_fetch_1()
*
*  Function description
*    Atomic bitwise-and and fetch, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __sync_and_and_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U8, ret & value)
}

/*********************************************************************
*
*       __sync_and_and_fetch_2()
*
*  Function description
*    Atomic bitwise-and and fetch, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_and_and_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U8, ret & value)
}

/*********************************************************************
*
*       __sync_and_and_fetch_4()
*
*  Function description
*    Atomic bitwise-and and fetch, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_and_and_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret & value)
}

/*********************************************************************
*
*       __sync_and_and_fetch_8()
*
*  Function description
*    Atomic bitwise-and and fetch, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_and_and_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U64, ret & value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_and_and_fetch_16()
*
*  Function description
*    Atomic bitwise-and and fetch, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_and_and_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U128, ret & value)
}
#endif

/*********************************************************************
*
*       __sync_or_and_fetch_1()
*
*  Function description
*    Atomic bitwise-or and fetch, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __sync_or_and_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U8, ret | value)
}

/*********************************************************************
*
*       __sync_or_and_fetch_2()
*
*  Function description
*    Atomic bitwise-or and fetch, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_or_and_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U16, ret | value)
}

/*********************************************************************
*
*       __sync_or_and_fetch_4()
*
*  Function description
*    Atomic bitwise-or and fetch, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_or_and_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret | value)
}

/*********************************************************************
*
*       __sync_or_and_fetch_8()
*
*  Function description
*    Atomic bitwise-or and fetch, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_or_and_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U64, ret | value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_or_and_fetch_16()
*
*  Function description
*    Atomic bitwise-or and fetch, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_or_and_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U128, ret | value)
}
#endif

/*********************************************************************
*
*       __sync_xor_and_fetch_1()
*
*  Function description
*    Atomic bitwise-exclusive-or and fetch, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __sync_xor_and_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U8, ret ^ value)
}

/*********************************************************************
*
*       __sync_xor_and_fetch_2()
*
*  Function description
*    Atomic bitwise-exclusive-or and fetch, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_xor_and_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U16, ret ^ value)
}

/*********************************************************************
*
*       __sync_xor_and_fetch_4()
*
*  Function description
*    Atomic bitwise-exclusive-or and fetch, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_xor_and_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_ATOMIC_U32, ret ^ value)
}

/*********************************************************************
*
*       __sync_xor_and_fetch_8()
*
*  Function description
*    Atomic bitwise-exclusive-or and fetch, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_xor_and_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U64, ret ^ value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_xor_and_fetch_16()
*
*  Function description
*    Atomic bitwise-exclusive-or and fetch, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_xor_and_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U128, ret ^ value)
}
#endif

/*********************************************************************
*
*       __sync_nand_and_fetch_1()
*
*  Function description
*    Atomic bitwise-nand and fetch, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __sync_nand_and_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U8, ~(ret & value))
}

/*********************************************************************
*
*       __sync_nand_and_fetch_2()
*
*  Function description
*    Atomic bitwise-nand and fetch, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __SEGGER_RTL_PUBLIC_API __sync_nand_and_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U16, ~(ret & value))
}

/*********************************************************************
*
*       __sync_nand_and_fetch_4()
*
*  Function description
*    Atomic bitwise-nand and fetch, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __SEGGER_RTL_PUBLIC_API __sync_nand_and_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_ATOMIC_U32, ~(ret & value))
}

/*********************************************************************
*
*       __sync_nand_and_fetch_8()
*
*  Function description
*    Atomic bitwise-nand and fetch, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __SEGGER_RTL_PUBLIC_API __sync_nand_and_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U64, ~(ret & value))
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_nand_and_fetch_16()
*
*  Function description
*    Atomic bitwise-nand and fetch, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to update.
*    value - Value to add to object.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __SEGGER_RTL_PUBLIC_API __sync_nand_and_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_OP_AND_FETCH(ptr, value, __SEGGER_RTL_U128, ~(ret & value))
}
#endif

/*********************************************************************
*
*       __sync_bool_compare_and_swap_1()
*
*  Function description
*    Atomic compare and swap, 8-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_BOOL __sync_bool_compare_and_swap_1(volatile void *ptr, __SEGGER_RTL_U8 oldval, __SEGGER_RTL_U8 newval) {
  SYNC_BOOL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_U8)
}

/*********************************************************************
*
*       __sync_bool_compare_and_swap_2()
*
*  Function description
*    Atomic compare and swap, 16-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_BOOL __sync_bool_compare_and_swap_2(volatile void *ptr, __SEGGER_RTL_U16 oldval, __SEGGER_RTL_U16 newval) {
  SYNC_BOOL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_U16)
}

/*********************************************************************
*
*       __sync_bool_compare_and_swap_4()
*
*  Function description
*    Atomic compare and swap, 32-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_BOOL __sync_bool_compare_and_swap_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 oldval, __SEGGER_RTL_ATOMIC_U32 newval) {
  SYNC_BOOL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_ATOMIC_U32)
}

/*********************************************************************
*
*       __sync_bool_compare_and_swap_8()
*
*  Function description
*    Atomic compare and swap, 64-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_BOOL __sync_bool_compare_and_swap_8(volatile void *ptr, __SEGGER_RTL_U64 oldval, __SEGGER_RTL_U64 newval) {
  SYNC_BOOL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_U64)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_bool_compare_and_swap_16()
*
*  Function description
*    Atomic compare and swap, 128-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_BOOL __sync_bool_compare_and_swap_16(volatile void *ptr, __SEGGER_RTL_U128 oldval, __SEGGER_RTL_U128 newval) {
  SYNC_BOOL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_U128)
}
#endif

/*********************************************************************
*
*       __sync_val_compare_and_swap_1()
*
*  Function description
*    Atomic compare and swap, 8-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_U8 __sync_val_compare_and_swap_1(volatile void *ptr, __SEGGER_RTL_U8 oldval, __SEGGER_RTL_U8 newval) {
  SYNC_VAL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_U8)
}

/*********************************************************************
*
*       __sync_val_compare_and_swap_2()
*
*  Function description
*    Atomic compare and swap, 16-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_U16 __sync_val_compare_and_swap_2(volatile void *ptr, __SEGGER_RTL_U16 oldval, __SEGGER_RTL_U16 newval) {
  SYNC_VAL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_U16)
}

/*********************************************************************
*
*       __sync_val_compare_and_swap_4()
*
*  Function description
*    Atomic compare and swap, 32-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_ATOMIC_U32 __sync_val_compare_and_swap_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 oldval, __SEGGER_RTL_ATOMIC_U32 newval) {
  SYNC_VAL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_ATOMIC_U32)
}

/*********************************************************************
*
*       __sync_val_compare_and_swap_8()
*
*  Function description
*    Atomic compare and swap, 64-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_U64 __sync_val_compare_and_swap_8(volatile void *ptr, __SEGGER_RTL_U64 oldval, __SEGGER_RTL_U64 newval) {
  SYNC_VAL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_U64)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_val_compare_and_swap_16()
*
*  Function description
*    Atomic compare and swap, 128-bit object.
*
*  Parameters
*    ptr    - Pointer to object to update.
*    oldval - Expected value of object on entry.
*    newval - Value to attempt to write to object.
*
*  Return value
*    Nonzero if and only if newval was written to the object pointed
*    to by ptr.
*/
__SEGGER_RTL_U128 __sync_val_compare_and_swap_16(volatile void *ptr, __SEGGER_RTL_U128 oldval, __SEGGER_RTL_U128 newval) {
  SYNC_VAL_COMPARE_AND_SWAP(ptr, oldval, newval, __SEGGER_RTL_U128)
}
#endif

/*********************************************************************
*
*       __sync_lock_test_and_set_1()
*
*  Function description
*    Acquire lock, 8-bit object.
*
*  Parameters
*    ptr   - Pointer to object to be locked.
*    value - Value to be written to object.
*
*  Return value
*    The value of the object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U8 __sync_lock_test_and_set_1(volatile void *ptr, __SEGGER_RTL_U8 value) {
  SYNC_LOCK_TEST_AND_SET(ptr, value, __SEGGER_RTL_U8)
}

/*********************************************************************
*
*       __sync_lock_test_and_set_2()
*
*  Function description
*    Acquire lock, 16-bit object.
*
*  Parameters
*    ptr   - Pointer to object to be locked.
*    value - Value to be written to object.
*
*  Return value
*    The value of the object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U16 __sync_lock_test_and_set_2(volatile void *ptr, __SEGGER_RTL_U16 value) {
  SYNC_LOCK_TEST_AND_SET(ptr, value, __SEGGER_RTL_U16)
}

/*********************************************************************
*
*       __sync_lock_test_and_set_4()
*
*  Function description
*    Acquire lock, 32-bit object.
*
*  Parameters
*    ptr   - Pointer to object to be locked.
*    value - Value to be written to object.
*
*  Return value
*    The value of the object pointed to by ptr prior to update.
*/
__SEGGER_RTL_ATOMIC_U32 __sync_lock_test_and_set_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value) {
  SYNC_LOCK_TEST_AND_SET(ptr, value, __SEGGER_RTL_ATOMIC_U32)
}

/*********************************************************************
*
*       __sync_lock_test_and_set_8()
*
*  Function description
*    Acquire lock, 64-bit object.
*
*  Parameters
*    ptr   - Pointer to object to be locked.
*    value - Value to be written to object.
*
*  Return value
*    The value of the object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U64 __sync_lock_test_and_set_8(volatile void *ptr, __SEGGER_RTL_U64 value) {
  SYNC_LOCK_TEST_AND_SET(ptr, value, __SEGGER_RTL_U64)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_lock_test_and_set_16()
*
*  Function description
*    Acquire lock, 128-bit object.
*
*  Parameters
*    ptr   - Pointer to object to be locked.
*    value - Value to be written to object.
*
*  Return value
*    The value of the object pointed to by ptr prior to update.
*/
__SEGGER_RTL_U128 __sync_lock_test_and_set_16(volatile void *ptr, __SEGGER_RTL_U128 value) {
  SYNC_LOCK_TEST_AND_SET(ptr, value, __SEGGER_RTL_U128)
}
#endif

/*********************************************************************
*
*       __sync_lock_release_1()
*
*  Function description
*    Release lock, 8-bit object.
*
*  Parameters
*    ptr - Pointer to object to unlock.
*/
void __sync_lock_release_1(volatile void *ptr) {
  SYNC_LOCK_RELEASE(ptr, __SEGGER_RTL_U8)
}

/*********************************************************************
*
*       __sync_lock_release_2()
*
*  Function description
*    Release lock, 16-bit object.
*
*  Parameters
*    ptr - Pointer to object to unlock.
*/
void __sync_lock_release_2(volatile void *ptr) {
  SYNC_LOCK_RELEASE(ptr, __SEGGER_RTL_U16)
}

/*********************************************************************
*
*       __sync_lock_release_4()
*
*  Function description
*    Release lock, 32-bit object.
*
*  Parameters
*    ptr - Pointer to object to unlock.
*/
void __sync_lock_release_4(volatile void *ptr) {
  SYNC_LOCK_RELEASE(ptr, __SEGGER_RTL_ATOMIC_U32)
}

/*********************************************************************
*
*       __sync_lock_release_8()
*
*  Function description
*    Release lock, 64-bit object.
*
*  Parameters
*    ptr - Pointer to object to unlock.
*/
void __sync_lock_release_8(volatile void *ptr) {
  SYNC_LOCK_RELEASE(ptr, __SEGGER_RTL_U64)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __sync_lock_release_16()
*
*  Function description
*    Release lock, 128-bit object.
*
*  Parameters
*    ptr - Pointer to object to unlock.
*/
void __sync_lock_release_16(volatile void *ptr) {
  SYNC_LOCK_RELEASE(ptr, __SEGGER_RTL_U128)
}
#endif

/*********************************************************************
*
*       __atomic_load()
*
*  Function description
*    Generic atomic load.
*
*  Parameters
*    size     - Size of object to load.
*    ptr      - Pointer to object to load.
*    ret      - Pointer to where object is written.
*    memorder - Memory ordering (__ATOMIC_RELAXED, __ATOMIC_SEQ_CST,
*               or __ATOMIC_RELEASE).
*
*  Return value
*    Value loaded.
*/
void __SEGGER_RTL_PUBLIC_API __atomic_load(size_t size, const volatile void *ptr, void *ret, int memorder) {
  int en;
  //
  __SEGGER_RTL_USE_PARA(memorder);
  //
  en = __SEGGER_RTL_ATOMIC_LOCK();
  (memcpy)(ret, (const void *)ptr, size);
  __SEGGER_RTL_ATOMIC_UNLOCK(en);
}

/*********************************************************************
*
*       __atomic_store()
*
*  Function description
*    Generic atomic store.
*
*  Parameters
*    size     - Size of object to store.
*    ptr      - Pointer to where object is written.
*    val      - Pointer to object to store.
*    memorder - Memory ordering.
*/
void __SEGGER_RTL_PUBLIC_API __atomic_store(size_t size, volatile void *ptr, void *val, int memorder) {
  int en;
  //
  __SEGGER_RTL_USE_PARA(memorder);
  //
  en = __SEGGER_RTL_ATOMIC_LOCK();
  (memcpy)((void *)ptr, val, size);
  __SEGGER_RTL_ATOMIC_UNLOCK(en);
}


/*********************************************************************
*
*       __atomic_is_lock_free()
*
*  Function description
*    Inquire whether objects always generate lock-free atomic instructions.
*
*  Parameters
*    size - Size of object under consideration.
*    ptr  - Pointer to object under consideration.
*
*  Return value
*    True if objects of size bytes always generate lock-free atomic
*    instructions for the target architecture.
*
*  Additional information
*    ptr is an optional pointer to the object that may be used to determine
*    alignment.  A NULL pointer indicates typical alignment should be used.
*/
__SEGGER_RTL_BOOL __atomic_is_lock_free(size_t size, const volatile void *ptr) {
  return __SEGGER_RTL_ATOMIC_IS_LOCK_FREE(size, ptr);
}

/*********************************************************************
*
*       __atomic_load_1()
*
*  Function description
*    Atomic fetch, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to load.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr.
*/
__SEGGER_RTL_U8 __atomic_load_1(const volatile void *ptr, int memorder) {
  ATOMIC_LOAD(ptr, memorder, __SEGGER_RTL_U8)
}

/*********************************************************************
*
*       __atomic_load_2()
*
*  Function description
*    Atomic fetch, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to load.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr.
*/
__SEGGER_RTL_U16 __atomic_load_2(const volatile void *ptr, int memorder) {
  ATOMIC_LOAD(ptr, memorder, __SEGGER_RTL_U16)
}

/*********************************************************************
*
*       __atomic_load_4()
*
*  Function description
*    Atomic fetch, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to load.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_load_4(const volatile void *ptr, int memorder) {
  ATOMIC_LOAD(ptr, memorder, __SEGGER_RTL_ATOMIC_U32)
}

/*********************************************************************
*
*       __atomic_load_8()
*
*  Function description
*    Atomic fetch, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to load.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr.
*/
__SEGGER_RTL_U64 __atomic_load_8(const volatile void *ptr, int memorder) {
  ATOMIC_LOAD(ptr, memorder, __SEGGER_RTL_U64)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_load_16()
*
*  Function description
*    Atomic fetch, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to load.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr.
*/
__SEGGER_RTL_U128 __atomic_load_16(const volatile void *ptr, int memorder) {
  ATOMIC_LOAD(ptr, memorder, __SEGGER_RTL_U128)
}
#endif

/*********************************************************************
*
*       __atomic_store_1()
*
*  Function description
*    Atomic store, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to store to.
*    val      - Value to store.
*    memorder - Memory ordering.
*/
void __atomic_store_1(volatile void *ptr, __SEGGER_RTL_U8 val, int memorder) {
  ATOMIC_STORE(ptr, val, memorder, __SEGGER_RTL_U8)
}

/*********************************************************************
*
*       __atomic_store_2()
*
*  Function description
*    Atomic store, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to store to.
*    val      - Value to store.
*    memorder - Memory ordering.
*/
void __atomic_store_2(volatile void *ptr, __SEGGER_RTL_U16 val, int memorder) {
  ATOMIC_STORE(ptr, val, memorder, __SEGGER_RTL_U16)
}

/*********************************************************************
*
*       __atomic_store_4()
*
*  Function description
*    Atomic store, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to store to.
*    val      - Value to store.
*    memorder - Memory ordering.
*/
void __atomic_store_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 val, int memorder) {
  ATOMIC_STORE(ptr, val, memorder, __SEGGER_RTL_ATOMIC_U32)
}

/*********************************************************************
*
*       __atomic_store_8()
*
*  Function description
*    Atomic store, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to store to.
*    val      - Value to store.
*    memorder - Memory ordering.
*/
void __atomic_store_8(volatile void *ptr, __SEGGER_RTL_U64 val, int memorder) {
  ATOMIC_STORE(ptr, val, memorder, __SEGGER_RTL_U64)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_store_16()
*
*  Function description
*    Atomic store, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to store to.
*    val      - Value to store.
*    memorder - Memory ordering.
*/
void __atomic_store_16(volatile void *ptr, __SEGGER_RTL_U128 val, int memorder) {
  ATOMIC_STORE(ptr, val, memorder, __SEGGER_RTL_U128)
}
#endif

/*********************************************************************
*
*       __atomic_add_fetch_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __atomic_add_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U8, ret + value)
}

/*********************************************************************
*
*       __atomic_add_fetch_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __atomic_add_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U16, ret + value)
}

/*********************************************************************
*
*       __atomic_add_fetch_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_add_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret + value)
}

/*********************************************************************
*
*       __atomic_add_fetch_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __atomic_add_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U64, ret + value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_add_fetch_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __atomic_add_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U128, ret + value)
}
#endif

/*********************************************************************
*
*       __atomic_sub_fetch_1()
*
*  Function description
*    Atomic fetch with decrement, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __atomic_sub_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U8, ret - value)
}

/*********************************************************************
*
*       __atomic_sub_fetch_2()
*
*  Function description
*    Atomic fetch with decrement, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __atomic_sub_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U16, ret - value)
}

/*********************************************************************
*
*       __atomic_sub_fetch_4()
*
*  Function description
*    Atomic fetch with decrement, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_sub_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret - value)
}

/*********************************************************************
*
*       __atomic_sub_fetch_8()
*
*  Function description
*    Atomic fetch with decrement, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __atomic_sub_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U64, ret - value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_sub_fetch_16()
*
*  Function description
*    Atomic fetch with decrement, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __atomic_sub_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U128, ret - value)
}
#endif

/*********************************************************************
*
*       __atomic_and_fetch_1()
*
*  Function description
*    Atomic fetch with bitwise-and, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __atomic_and_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U8, ret & value)
}

/*********************************************************************
*
*       __atomic_and_fetch_2()
*
*  Function description
*    Atomic fetch with bitwise-and, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __atomic_and_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U16, ret & value)
}

/*********************************************************************
*
*       __atomic_and_fetch_4()
*
*  Function description
*    Atomic fetch with bitwise-and, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_and_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret & value)
}

/*********************************************************************
*
*       __atomic_and_fetch_8()
*
*  Function description
*    Atomic fetch with bitwise-and, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __atomic_and_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U64, ret & value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_and_fetch_16()
*
*  Function description
*    Atomic fetch with bitwise-and, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __atomic_and_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U128, ret & value)
}
#endif

/*********************************************************************
*
*       __atomic_or_fetch_1()
*
*  Function description
*    Atomic fetch with bitwise-or, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __atomic_or_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U8, ret | value)
}

/*********************************************************************
*
*       __atomic_or_fetch_2()
*
*  Function description
*    Atomic fetch with bitwise-or, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __atomic_or_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U16, ret | value)
}

/*********************************************************************
*
*       __atomic_or_fetch_4()
*
*  Function description
*    Atomic fetch with bitwise-or, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_or_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret | value)
}

/*********************************************************************
*
*       __atomic_or_fetch_8()
*
*  Function description
*    Atomic fetch with bitwise-or, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __atomic_or_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U64, ret | value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_or_fetch_16()
*
*  Function description
*    Atomic fetch with bitwise-or, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __atomic_or_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U128, ret | value)
}
#endif

/*********************************************************************
*
*       __atomic_xor_fetch_1()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __atomic_xor_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U8, ret ^ value)
}

/*********************************************************************
*
*       __atomic_xor_fetch_2()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __atomic_xor_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U16, ret ^ value)
}

/*********************************************************************
*
*       __atomic_xor_fetch_4()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_xor_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret ^ value)
}

/*********************************************************************
*
*       __atomic_xor_fetch_8()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __atomic_xor_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U64, ret ^ value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_xor_fetch_16()
*
*  Function description
*    Atomic fetch with bitwise-exclusive-or, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __atomic_xor_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U128, ret ^ value)
}
#endif

/*********************************************************************
*
*       __atomic_nand_fetch_1()
*
*  Function description
*    Atomic fetch with bitwise-nand, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U8 __atomic_nand_fetch_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U8, ~(ret & value))
}

/*********************************************************************
*
*       __atomic_nand_fetch_2()
*
*  Function description
*    Atomic fetch with bitwise-nand, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U16 __atomic_nand_fetch_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U16, ~(ret & value))
}

/*********************************************************************
*
*       __atomic_nand_fetch_4()
*
*  Function description
*    Atomic fetch with bitwise-nand, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_nand_fetch_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ~(ret & value))
}

/*********************************************************************
*
*       __atomic_nand_fetch_8()
*
*  Function description
*    Atomic fetch with bitwise-nand, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U64 __atomic_nand_fetch_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U64, ~(ret & value))
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_nand_fetch_16()
*
*  Function description
*    Atomic fetch with bitwise-nand, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr after the update.
*/
__SEGGER_RTL_U128 __atomic_nand_fetch_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_OP_FETCH(ptr, value, memorder, __SEGGER_RTL_U128, ~(ret & value))
}
#endif

/*********************************************************************
*
*       __atomic_fetch_add_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U8 __atomic_fetch_add_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U8, ret + value)
}

/*********************************************************************
*
*       __atomic_fetch_add_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U16 __atomic_fetch_add_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U16, ret + value)
}

/*********************************************************************
*
*       __atomic_fetch_add_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_fetch_add_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret + value)
}

/*********************************************************************
*
*       __atomic_fetch_add_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U64 __atomic_fetch_add_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U64, ret + value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_fetch_add_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U128 __atomic_fetch_add_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U128, ret + value)
}
#endif

/*********************************************************************
*
*       __atomic_fetch_sub_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U8 __atomic_fetch_sub_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U8, ret - value)
}

/*********************************************************************
*
*       __atomic_fetch_sub_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U16 __atomic_fetch_sub_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U16, ret - value)
}

/*********************************************************************
*
*       __atomic_fetch_sub_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_fetch_sub_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret - value)
}

/*********************************************************************
*
*       __atomic_fetch_sub_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U64 __atomic_fetch_sub_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U64, ret - value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_fetch_sub_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U128 __atomic_fetch_sub_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U128, ret - value)
}
#endif

/*********************************************************************
*
*       __atomic_fetch_and_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U8 __atomic_fetch_and_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U8, ret & value)
}

/*********************************************************************
*
*       __atomic_fetch_and_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U16 __atomic_fetch_and_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U16, ret & value)
}

/*********************************************************************
*
*       __atomic_fetch_and_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_fetch_and_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret & value)
}

/*********************************************************************
*
*       __atomic_fetch_and_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U64 __atomic_fetch_and_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U64, ret & value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_fetch_and_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U128 __atomic_fetch_and_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U128, ret & value)
}
#endif

/*********************************************************************
*
*       __atomic_fetch_or_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U8 __atomic_fetch_or_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U8, ret | value)
}

/*********************************************************************
*
*       __atomic_fetch_or_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U16 __atomic_fetch_or_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U16, ret | value)
}

/*********************************************************************
*
*       __atomic_fetch_or_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_fetch_or_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret | value)
}

/*********************************************************************
*
*       __atomic_fetch_or_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U64 __atomic_fetch_or_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U64, ret | value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_fetch_or_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U128 __atomic_fetch_or_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U128, ret | value)
}
#endif

/*********************************************************************
*
*       __atomic_fetch_xor_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U8 __atomic_fetch_xor_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U8, ret ^ value)
}

/*********************************************************************
*
*       __atomic_fetch_xor_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U16 __atomic_fetch_xor_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U16, ret ^ value)
}

/*********************************************************************
*
*       __atomic_fetch_xor_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_fetch_xor_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ret ^ value)
}

/*********************************************************************
*
*       __atomic_fetch_xor_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U64 __atomic_fetch_xor_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U64, ret ^ value)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_fetch_xor_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U128 __atomic_fetch_xor_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U128, ret ^ value)
}
#endif

/*********************************************************************
*
*       __atomic_fetch_nand_1()
*
*  Function description
*    Atomic fetch with increment, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U8 __atomic_fetch_nand_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U8, ~(ret & value))
}

/*********************************************************************
*
*       __atomic_fetch_nand_2()
*
*  Function description
*    Atomic fetch with increment, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U16 __atomic_fetch_nand_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U16, ~(ret & value))
}

/*********************************************************************
*
*       __atomic_fetch_nand_4()
*
*  Function description
*    Atomic fetch with increment, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_fetch_nand_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32, ~(ret & value))
}

/*********************************************************************
*
*       __atomic_fetch_nand_8()
*
*  Function description
*    Atomic fetch with increment, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U64 __atomic_fetch_nand_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U64, ~(ret & value))
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_fetch_nand_16()
*
*  Function description
*    Atomic fetch with increment, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to update.
*    value    - Value to add to object.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U128 __atomic_fetch_nand_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_FETCH_AND_OP(ptr, value, memorder, __SEGGER_RTL_U128, ~(ret & value))
}
#endif

/*********************************************************************
*
*       __atomic_exchange_1()
*
*  Function description
*    Atomic exchange, 8-bit object.
*
*  Parameters
*    ptr      - Pointer to object to exchange with.
*    value    - Value to write to object pointed to by ptr.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U8 __atomic_exchange_1(volatile void *ptr, __SEGGER_RTL_U8 value, int memorder) {
  ATOMIC_EXCHANGE(ptr, value, memorder, __SEGGER_RTL_U8)
}

/*********************************************************************
*
*       __atomic_exchange_2()
*
*  Function description
*    Atomic exchange, 16-bit object.
*
*  Parameters
*    ptr      - Pointer to object to exchange with.
*    value    - Value to write to object pointed to by ptr.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U16 __atomic_exchange_2(volatile void *ptr, __SEGGER_RTL_U16 value, int memorder) {
  ATOMIC_EXCHANGE(ptr, value, memorder, __SEGGER_RTL_U16)
}

/*********************************************************************
*
*       __atomic_exchange_4()
*
*  Function description
*    Atomic exchange, 32-bit object.
*
*  Parameters
*    ptr      - Pointer to object to exchange with.
*    value    - Value to write to object pointed to by ptr.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_ATOMIC_U32 __atomic_exchange_4(volatile void *ptr, __SEGGER_RTL_ATOMIC_U32 value, int memorder) {
  ATOMIC_EXCHANGE(ptr, value, memorder, __SEGGER_RTL_ATOMIC_U32)
}

/*********************************************************************
*
*       __atomic_exchange_8()
*
*  Function description
*    Atomic exchange, 64-bit object.
*
*  Parameters
*    ptr      - Pointer to object to exchange with.
*    value    - Value to write to object pointed to by ptr.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U64 __atomic_exchange_8(volatile void *ptr, __SEGGER_RTL_U64 value, int memorder) {
  ATOMIC_EXCHANGE(ptr, value, memorder, __SEGGER_RTL_U64)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_exchange_16()
*
*  Function description
*    Atomic exchange, 128-bit object.
*
*  Parameters
*    ptr      - Pointer to object to exchange with.
*    value    - Value to write to object pointed to by ptr.
*    memorder - Memory ordering.
*
*  Return value
*    Value of object pointed to by ptr prior to the update.
*/
__SEGGER_RTL_U128 __atomic_exchange_16(volatile void *ptr, __SEGGER_RTL_U128 value, int memorder) {
  ATOMIC_EXCHANGE(ptr, value, memorder, __SEGGER_RTL_U128)
}
#endif

/*********************************************************************
*
*       __atomic_exchange()
*
*  Function description
*    Atomic exchange, arbitrary-size object.
*
*  Parameters
*    size     - Size of the objects to exchange.
*    ptr      - Pointer to object to exchange with.
*    val      - Pointer to object to containing value to be written.
*    ret      - Pointer to object that receives the value pointed to by ptr.
*    memorder - Memory ordering.
*/
void __atomic_exchange(size_t size, void *ptr, void *val, void *ret, int memorder) {
  int en;
  //
  __SEGGER_RTL_USE_PARA(memorder);
  //
  en  = __SEGGER_RTL_ATOMIC_LOCK();                           \
  (memcpy)(ret, ptr, size);
  (memcpy)(ptr, val, size);
  __SEGGER_RTL_ATOMIC_UNLOCK(en);
}

/*********************************************************************
*
*       __atomic_compare_exchange_1()
*
*  Function description
*    Atomic compare and exchange, 8-bit object.
*
*  Parameters
*    ptr              - Pointer to object to exchange with.
*    expected         - Pointer to expected value of the object to exchange with.
*    desired          - Value to exchange with.
*    weak             - Flag indicating weak or strong variation.
*    success_memorder - Memory ordering to use if exchange proceeds.
*    failure_memorder - Memory ordering to use if exchange aborted.
*
*  Return value
*    Nonzero if the exchange completed, else zero.
*
*  Notes
*    GCC documents these functions with the type of expected as a pointer
*    to the type of the object.  However, gcc 12 (at least) requires that
*    expected is a pointer to void and issues a warning if it finds otherwise.
*
*    See https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins.
*/
__SEGGER_RTL_BOOL __atomic_compare_exchange_1(volatile void *ptr, void *expected, __SEGGER_RTL_U8 desired, __SEGGER_RTL_BOOL weak, int success_memorder, int failure_memorder) {
  ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired, weak, success_memorder, failure_memorder, __SEGGER_RTL_U8)
}

/*********************************************************************
*
*       __atomic_compare_exchange_2()
*
*  Function description
*    Atomic compare and exchange, 16-bit object.
*
*  Parameters
*    ptr              - Pointer to object to exchange with.
*    expected         - Pointer to expected value of the object to exchange with.
*    desired          - Value to exchange with.
*    weak             - Flag indicating weak or strong variation.
*    success_memorder - Memory ordering to use if exchange proceeds.
*    failure_memorder - Memory ordering to use if exchange aborted.
*
*  Return value
*    Nonzero if the exchange completed, else zero.
*
*  Notes
*    GCC documents these functions with the type of expected as a pointer
*    to the type of the object.  However, gcc 12 (at least) requires that
*    expected is a pointer to void and issues a warning if it finds otherwise.
*
*    See https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins.
*/
__SEGGER_RTL_BOOL __atomic_compare_exchange_2(volatile void *ptr, void *expected, __SEGGER_RTL_U16 desired, __SEGGER_RTL_BOOL weak, int success_memorder, int failure_memorder) {
  ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired, weak, success_memorder, failure_memorder, __SEGGER_RTL_U16)
}

/*********************************************************************
*
*       __atomic_compare_exchange_4()
*
*  Function description
*    Atomic compare and exchange, 32-bit object.
*
*  Parameters
*    ptr              - Pointer to object to exchange with.
*    expected         - Pointer to expected value of the object to exchange with.
*    desired          - Value to exchange with.
*    weak             - Flag indicating weak or strong variation.
*    success_memorder - Memory ordering to use if exchange proceeds.
*    failure_memorder - Memory ordering to use if exchange aborted.
*
*  Return value
*    Nonzero if the exchange completed, else zero.
*
*  Notes
*    GCC documents these functions with the type of expected as a pointer
*    to the type of the object.  However, gcc 12 (at least) requires that
*    expected is a pointer to void and issues a warning if it finds otherwise.
*
*    See https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins.
*/
__SEGGER_RTL_BOOL __atomic_compare_exchange_4(volatile void *ptr, void *expected, __SEGGER_RTL_ATOMIC_U32 desired, __SEGGER_RTL_BOOL weak, int success_memorder, int failure_memorder) {
  ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired, weak, success_memorder, failure_memorder, __SEGGER_RTL_ATOMIC_U32)
}

/*********************************************************************
*
*       __atomic_compare_exchange_8()
*
*  Function description
*    Atomic compare and exchange, 64-bit object.
*
*  Parameters
*    ptr              - Pointer to object to exchange with.
*    expected         - Pointer to expected value of the object to exchange with.
*    desired          - Value to exchange with.
*    weak             - Flag indicating weak or strong variation.
*    success_memorder - Memory ordering to use if exchange proceeds.
*    failure_memorder - Memory ordering to use if exchange aborted.
*
*  Return value
*    Nonzero if the exchange completed, else zero.
*
*  Notes
*    GCC documents these functions with the type of expected as a pointer
*    to the type of the object.  However, gcc 12 (at least) requires that
*    expected is a pointer to void and issues a warning if it finds otherwise.
*
*    See https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins.
*/
__SEGGER_RTL_BOOL __atomic_compare_exchange_8(volatile void *ptr, void *expected, __SEGGER_RTL_U64 desired, __SEGGER_RTL_BOOL weak, int success_memorder, int failure_memorder) {
  ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired, weak, success_memorder, failure_memorder, __SEGGER_RTL_U64)
}

#if defined(__SEGGER_RTL_U128)
/*********************************************************************
*
*       __atomic_compare_exchange_16()
*
*  Function description
*    Atomic compare and exchange, 128-bit object.
*
*  Parameters
*    ptr              - Pointer to object to exchange with.
*    expected         - Pointer to expected value of the object to exchange with.
*    desired          - Value to exchange with.
*    weak             - Flag indicating weak or strong variation.
*    success_memorder - Memory ordering to use if exchange proceeds.
*    failure_memorder - Memory ordering to use if exchange aborted.
*
*  Return value
*    Nonzero if the exchange completed, else zero.
*
*  Notes
*    GCC documents these functions with the type of expected as a pointer
*    to the type of the object.  However, gcc 12 (at least) requires that
*    expected is a pointer to void and issues a warning if it finds otherwise.
*
*    See https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html#g_t_005f_005fatomic-Builtins.
*/
__SEGGER_RTL_BOOL __atomic_compare_exchange_16(volatile void *ptr, void *expected, __SEGGER_RTL_U128 desired, __SEGGER_RTL_BOOL weak, int success_memorder, int failure_memorder) {
  ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired, weak, success_memorder, failure_memorder, __SEGGER_RTL_U128)
}
#endif

/*********************************************************************
*
*       __atomic_compare_exchange()
*
*  Function description
*    Atomic compare and exchange, arbitrary-size object.
*
*  Parameters
*    size             - Size of objects to exchange.
*    ptr              - Pointer to object to compare and exchange with.
*    expected         - Pointer to expected value of the object to exchange with.
*    desired          - Pointer to value to exchange with.
*    success_memorder - Memory ordering to use if exchange proceeds.
*    failure_memorder - Memory ordering to use if exchange aborted.
*
*  Return value
*    Nonzero if the exchange completed, else zero.
*/
__SEGGER_RTL_BOOL __atomic_compare_exchange(size_t size, void *ptr, void *expected, void *desired, int success_memorder, int failure_memorder) {
  int en;
  int ret;
  //
  __SEGGER_RTL_USE_PARA(success_memorder);
  __SEGGER_RTL_USE_PARA(failure_memorder);
  //
  en = __SEGGER_RTL_ATOMIC_LOCK();
  if ((memcmp)(ptr, expected, size) == 0) {
    (memcpy)(ptr, desired, size);
    ret = 1;
  } else {
    (memcpy)(expected, ptr, size);
    ret = 0;
  }
  //
  __SEGGER_RTL_ATOMIC_UNLOCK(en);
  //
  return ret;
}

/*************************** End of file ****************************/

#include "atomic_locking.c"

__SEGGER_RTL_BOOL __atomic_test_and_set(void *ptr, int memorder)
{
  int en;
  int ret;
  __SEGGER_RTL_USE_PARA(memorder);
  en = __SEGGER_RTL_ATOMIC_LOCK();
  if (*(char *)(ptr))
    {
      ret = 1;
    }
  else
    {
      *(char *)(ptr) = 1;
      ret = 0;
    }
  __SEGGER_RTL_ATOMIC_UNLOCK(en);
} 

void __atomic_clear(void *ptr, int memorder)
{  
  int en;
  __SEGGER_RTL_USE_PARA(memorder);
  en = __SEGGER_RTL_ATOMIC_LOCK();
  *(char *)(ptr) = 0;
  __SEGGER_RTL_ATOMIC_UNLOCK(en);
} 

int  __SEGGER_RTL_X_atomic_lock(void)
{  
  return __atomic_lock();
}

void  __SEGGER_RTL_X_atomic_unlock(int en)
{
  __atomic_unlock(en);
}
