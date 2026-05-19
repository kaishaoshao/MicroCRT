/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Purpose: Real-time O(1) memory allocator.

*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "__SEGGER_RTL_RTHEAP.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define HEAP_FLAG_FREE                  (1 << 0u)
#define HEAP_ALIGNED(X, A)              (__SEGGER_RTL_RTHEAP_AlignPtr(X, A) == (X))
#define HEAP_ISPOW2(X)                  (((X) & ((X)-1)) == 0)
#define HEAP_PTRDIFF(X, Y)              ((const char *)(X) - (const char *)(Y))
#define HEAP_DATA_OFFSET                offsetof(__SEGGER_RTL_RTHEAP_BLOCK, u.Used.aData)
#define HEAP_MAX_HEAP_SIZE              (((__SEGGER_RTL_RTHEAP_SIZE)1 << __SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS) - HEAP_DATA_OFFSET)
#define HEAP_IO(...)                    if (pIoFunc) pIoFunc(__VA_ARGS__)
#define HEAP_CK(X, ...)                 if (!(X)) { ++TotalErrorCnt; ++LocalErrorCnt; if (pIoFunc) pIoFunc(__VA_ARGS__); }


#if __SEGGER_RTL_RTHEAP_DEBUG_LEVEL >= 1
  #define HEAP_API_CHECK(X)             if (!(X)) __SEGGER_RTL_RTHEAP_PANIC();
#endif
#if __SEGGER_RTL_RTHEAP_DEBUG_LEVEL >= 2
  #define HEAP_INT_CHECK(X)             if (!(X)) __SEGGER_RTL_RTHEAP_PANIC();
#endif
#if __SEGGER_RTL_RTHEAP_DEBUG_LEVEL >= 3
  #define HEAP_INT_SET(A, B, L)         __SEGGER_RTL_RTHEAP_MEMSET(A, B, L)
#endif

#ifndef   HEAP_API_CHECK
  #define HEAP_API_CHECK(X)
#endif
#ifndef   HEAP_INT_CHECK
  #define HEAP_INT_CHECK(X)
#endif
#ifndef   HEAP_INT_SET
  #define HEAP_INT_SET(A, B, L)
#endif

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

typedef struct {
  int L1Index;
  int L2Index;
} __SEGGER_RTL_RTHEAP_INDEX;

struct __SEGGER_RTL_RTHEAP_BLOCK_HEADER_t {
  __SEGGER_RTL_RTHEAP_BLOCK     * pBlockBefore;  // Pointer to block immediately before this block
  __SEGGER_RTL_RTHEAP_SIZE        SizeAndFlags;  // Size of block (excluding header) and embedded block flags
  union {
    struct {
      __SEGGER_RTL_RTHEAP_BLOCK * pNextFree;     // Pointer to next free block; only valid if this block is free
      __SEGGER_RTL_RTHEAP_BLOCK * pPrevFree;     // Pointer to previous free block; only valid if this block is free
    } Free;
    struct {
      char aData[1];
    } Used;
  } u;
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

#if defined(__SEGGER_RTL_RTHEAP_MEMSET_SYNTHESIZED)
/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_MEMSET_func()
*
*  Function description
*    Set memory.
*
*  Parameters
*    pMem - Pointer to destination, aligned modulo __SEGGER_RTL_RTHEAP_ALIGN.
*    Byte - Byte to fill with.
*    Len  - Number of bytes to set, zero modulo __SEGGER_RTL_RTHEAP_ALIGN.
*/
static void __SEGGER_RTL_RTHEAP_MEMSET_func(void *pMem, unsigned Byte, __SEGGER_RTL_RTHEAP_SIZE Len) {
  __SEGGER_RTL_RTHEAP_ALIGNED_UNIT * pWord;
  __SEGGER_RTL_RTHEAP_ALIGNED_UNIT   Word;
  __SEGGER_RTL_RTHEAP_SIZE           n;
  //
  Byte &= 0xFF;  // Reduce to byte value
  //
  pWord = (__SEGGER_RTL_RTHEAP_ALIGNED_UNIT *)pMem;
  Word  = __SEGGER_RTL_RTHEAP_ALIGNED_UNIT_REPL(Byte);
  //
  for (n = Len / sizeof(__SEGGER_RTL_RTHEAP_ALIGNED_UNIT); n >= 8; n -= 8) {
    *pWord++ = Word; *pWord++ = Word; *pWord++ = Word; *pWord++ = Word;
    *pWord++ = Word; *pWord++ = Word; *pWord++ = Word; *pWord++ = Word;
  }
  while (n > 0) {
    *pWord++ = Word;
    n       -= 1;
  }
}
#endif

#if defined(__SEGGER_RTL_RTHEAP_MEMMOVE_SYNTHESIZED)
/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_MEMMOVE_func()
*
*  Function description
*    Move memory.
*
*  Parameters
*    pTo   - Pointer to destination, aligned modulo __SEGGER_RTL_RTHEAP_ALIGN.
*    pFrom - Pointer to source, aligned modulo __SEGGER_RTL_RTHEAP_ALIGN.
*    Len   - Number of bytes to move, zero modulo __SEGGER_RTL_RTHEAP_ALIGN.
*
*  Additional information
*   pFrom and pTo are guaranteed not to overlap or to overlap with
*   pTo less than pFrom.  As such, memory must be copied in increasing
*   address order.  Do not be tempted to use memcpy() or its
*   specialized relatives for this.
*/
static void __SEGGER_RTL_RTHEAP_MEMMOVE_func(__SEGGER_RTL_RTHEAP_ALIGNED_UNIT * pTo,
                                             __SEGGER_RTL_RTHEAP_ALIGNED_UNIT * pFrom,
                                             __SEGGER_RTL_RTHEAP_SIZE           Len) {
  __SEGGER_RTL_RTHEAP_SIZE n;
  //
  for (n = Len / sizeof(__SEGGER_RTL_RTHEAP_ALIGNED_UNIT); n >= 8; n -= 8) {
    *pTo++ = *pFrom++; *pTo++ = *pFrom++; *pTo++ = *pFrom++; *pTo++ = *pFrom++;
    *pTo++ = *pFrom++; *pTo++ = *pFrom++; *pTo++ = *pFrom++; *pTo++ = *pFrom++;
  }
  while (n > 0) {
    *pTo++ = *pFrom++;
    n     -= 1;
  }
}
#endif

#if defined(__SEGGER_RTL_RTHEAP_FLS_SYNTHESIZED)
/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_FLS_func()
*
*  Function description
*    Find bit index of last bit set.
*
*  Parameters
*    Word - Value to search, nonzero.
*
*  Return value
*    Bit index of least significant bit set, 0 through 31.
*/
static __SEGGER_RTL_RTHEAP_INLINE int __SEGGER_RTL_RTHEAP_FLS_func(unsigned Word) {
  int n;
  //
  HEAP_INT_CHECK(Word != 0);
  //
  n = 31;
  if ((Word >> 16) == 0) { Word <<= 16; n -= 16; }
  if ((Word >> 24) == 0) { Word <<=  8; n -=  8; }
  if ((Word >> 28) == 0) { Word <<=  4; n -=  4; }
  if ((Word >> 30) == 0) { Word <<=  2; n -=  2; }
  if ((Word >> 31) == 0) {              n -=  1; }
  //
  return n;
}
#endif

#if defined(__SEGGER_RTL_RTHEAP_FFS_SYNTHESIZED)
/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_FFS_func()
*
*  Function description
*    Find bit index of most significant bit set.
*
*  Parameters
*    Word - Value to search, nonzero.
*
*  Return value
*    Bit index of most significant bit set, 0 through 31.
*/
static __SEGGER_RTL_RTHEAP_INLINE int __SEGGER_RTL_RTHEAP_FFS_func(unsigned Word) {
  return __SEGGER_RTL_RTHEAP_FLS(Word & (0u - Word));
}
#endif

#if defined(__SEGGER_RTL_RTHEAP_FLSL_SYNTHESIZED)
/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_FLSL_func()
*
*  Function description
*    Find bit index of most significant bit set, 64-bit word.
*
*  Parameters
*    Word - Value to search, nonzero.
*
*  Return value
*    Bit index of most significant bit set, 0 through 31.
*/
static __SEGGER_RTL_RTHEAP_INLINE int __SEGGER_RTL_RTHEAP_FLSL_func(__SEGGER_RTL_RTHEAP_SIZE Word) {
  unsigned Hi;
  //
  Hi = (unsigned)(Word >> 32);
  if (Hi) {
    return 32 + __SEGGER_RTL_RTHEAP_FLS(Hi);
  } else {
    return __SEGGER_RTL_RTHEAP_FLS((unsigned)(Word & 0xFFFFFFFFu));
  }
}
#endif

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_AdjustUp()
*
*  Function description
*    Adjust to power-of-two alignment, rounding up.
*
*  Parameters
*    Size  - Block size.
*    Align - Alignment requirement, a power of two.
*
*  Return value
*    The smallest value that is zero modulo Align and greater than
*    or equal to Size.
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_SIZE __SEGGER_RTL_RTHEAP_AdjustUp(__SEGGER_RTL_RTHEAP_SIZE Size, __SEGGER_RTL_RTHEAP_SIZE Align) {
  return (Size + (Align - 1)) & (0u - Align);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_AdjustDown()
*
*  Function description
*    Adjust to power-of-two alignment, rounding down.
*
*  Parameters
*    Size  - Block size.
*    Align - Alignment requirement, a power of two.
*
*  Return value
*    The largest value that is zero modulo Align and less than
*    or equal to Size.
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_SIZE __SEGGER_RTL_RTHEAP_AdjustDown(__SEGGER_RTL_RTHEAP_SIZE Size, __SEGGER_RTL_RTHEAP_SIZE Align) {
  return Size - (Size & (Align - 1));
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_AlignPtr()
*
*  Function description
*    Adjust pointer to power-of-two alignment, rounding up.
*
*  Parameters
*    pVoid - Pointer.
*    Align - Alignment requirement, a power of two.
*
*  Return value
*    The smallest value that is zero modulo Align with an address
*    greater than or equal to pVoid.
*/
static __SEGGER_RTL_RTHEAP_INLINE void * __SEGGER_RTL_RTHEAP_AlignPtr(void *pVoid, __SEGGER_RTL_RTHEAP_SIZE Align) {
  return (void *)(((__SEGGER_RTL_RTHEAP_SIZE)pVoid + (Align - 1)) & (0u - Align));
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_ConstrainSize()
*
*  Function description
*    Adjust block size according to internal constraints.
*
*  Parameters
*    Size  - Block size in octets.
*    Align - Alignment requirement for block size.
*
*  Return value
*    A value that is zero modulo Align.
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_SIZE __SEGGER_RTL_RTHEAP_ConstrainSize(__SEGGER_RTL_RTHEAP_SIZE Size, __SEGGER_RTL_RTHEAP_SIZE Align) {
  Size = __SEGGER_RTL_RTHEAP_AdjustUp(Size, Align);
  return Size < sizeof(__SEGGER_RTL_RTHEAP_BLOCK) ? sizeof(__SEGGER_RTL_RTHEAP_BLOCK) : Size;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_ToBlock()
*
*  Function description
*    Return block associated with pointer.
*
*  Parameters
*    pVoid - Previously-allocated pointer.
*
*  Return value
*    Block associated with pointer.
*
*  Additional information
*    The pointer must have been allocated using this allocator. 
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_BLOCK * __SEGGER_RTL_RTHEAP_ToBlock(void *pVoid) {
  return (__SEGGER_RTL_RTHEAP_BLOCK *)((char *)pVoid - HEAP_DATA_OFFSET);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_ToData()
*
*  Function description
*    Return data associated with block.
*
*  Parameters
*    pBlock - Pointer to block.
*
*  Return value
*    Data associated with block.
*/
static __SEGGER_RTL_RTHEAP_INLINE void * __SEGGER_RTL_RTHEAP_ToData(__SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  return pBlock->u.Used.aData;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_GetBlockSize()
*
*  Function description
*    Return total size of block including metadata.
*
*  Parameters
*    pBlock - Pointer to block.
*
*  Return value
*    The total size of the block, including metadata.  This value will
*    be zero modulo __SEGGER_RTL_RTHEAP_ALIGN.
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_SIZE __SEGGER_RTL_RTHEAP_GetBlockSize(__SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  return pBlock->SizeAndFlags & ~HEAP_FLAG_FREE;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_IsSentinelBlock()
*
*  Function description
*    Is this the sentinel block?
*
*  Parameters
*    pBlock - Pointer to block.
*
*  Return value
*    == 0 - Not sentinel.
*    != 0 - Sentinel.
*
*  Additional information
*    The sentinel block at the end of the heap is the only block
*    created with size zero (and the free flag clear).
*/
static __SEGGER_RTL_RTHEAP_INLINE int __SEGGER_RTL_RTHEAP_IsSentinelBlock(__SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  return pBlock->SizeAndFlags == 0;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_IsBlockFree()
*
*  Function description
*    Is this block marked free?
*
*  Parameters
*    pBlock - Pointer to block.
*
*  Return value
*    == 0 - Not free.
*    != 0 - Free.
*/
static __SEGGER_RTL_RTHEAP_INLINE int __SEGGER_RTL_RTHEAP_IsBlockFree(__SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  return (int)(pBlock->SizeAndFlags & HEAP_FLAG_FREE);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_MarkFree()
*
*  Function description
*    Mark block as free.
*
*  Parameters
*    pBlock - Pointer to block.
*/
static __SEGGER_RTL_RTHEAP_INLINE void __SEGGER_RTL_RTHEAP_MarkFree(__SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  pBlock->SizeAndFlags |= HEAP_FLAG_FREE;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_MarkUsed()
*
*  Function description
*    Mark block as used.
*
*  Parameters
*    pBlock - Pointer to block.
*/
static __SEGGER_RTL_RTHEAP_INLINE void __SEGGER_RTL_RTHEAP_MarkUsed(__SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  pBlock->SizeAndFlags &= ~HEAP_FLAG_FREE;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_SkipBlock()
*
*  Function description
*    Calculate following block address.
*
*  Parameters
*    pBlock - Pointer to source block.
*    Size   - Total size of block pointed to by pBlock.
*
*  Return value
*    Pointer to block block starting at pBlock of size Size bytes.
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_BLOCK *__SEGGER_RTL_RTHEAP_SkipBlock(__SEGGER_RTL_RTHEAP_BLOCK *pBlock, __SEGGER_RTL_RTHEAP_SIZE Size) {
  return (__SEGGER_RTL_RTHEAP_BLOCK *)((char *)pBlock + Size);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_GetBlockAfter()
*
*  Function description
*    Get location of immediately-following block.
*
*  Parameters
*    pBlock - Pointer to block.
*
*  Return value
*    Pointer to the block that immediately follows pBlock.
*
*  Additional information
*    Cannot be used on the final block in the list.
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_BLOCK *__SEGGER_RTL_RTHEAP_GetBlockAfter(__SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  HEAP_INT_CHECK(!__SEGGER_RTL_RTHEAP_IsSentinelBlock(pBlock));
  //
  return __SEGGER_RTL_RTHEAP_SkipBlock(pBlock, __SEGGER_RTL_RTHEAP_GetBlockSize(pBlock));
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_GetBlockBefore()
*
*  Function description
*    Get location of immediately-before block.
*
*  Parameters
*    pBlock - Pointer to block.
*
*  Return value
*    Pointer to the block that immediately before pBlock.
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_BLOCK *__SEGGER_RTL_RTHEAP_GetBlockBefore(__SEGGER_RTL_RTHEAP_BLOCK* pBlock) {
  return pBlock->pBlockBefore;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_FixBackinkTo()
*
*  Function description
*    Fix back-link to block.
*
*  Parameters
*    pBlock - Pointer to block.
*
*  Additional information
*    The block immediately following pBlock has its back-link pointed
*    to pBlock, according to the size of pBlock.
*/
static __SEGGER_RTL_RTHEAP_INLINE void __SEGGER_RTL_RTHEAP_FixBackinkTo(__SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  __SEGGER_RTL_RTHEAP_GetBlockAfter(pBlock)->pBlockBefore = pBlock;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_MakeIndex()
*
*  Function description
*    Create a mapping index.
*
*  Parameters
*    L1Index - L1 index.
*    L2Index - L2 index.
*
*  Return value
*    An index initialized with the given values.
*/
static __SEGGER_RTL_RTHEAP_INLINE __SEGGER_RTL_RTHEAP_INDEX __SEGGER_RTL_RTHEAP_MakeIndex(int L1Index, int L2Index) {
  __SEGGER_RTL_RTHEAP_INDEX Index;
  //
  Index.L1Index = L1Index;
  Index.L2Index = L2Index;
  //
  return Index;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_Map()
*
*  Function description
*    Calculate mapping index associated with a block size.
*
*  Parameters
*    Size - Block size.
*
*  Return value
*    The mapping index corresponding the the given block size.
*/
static __SEGGER_RTL_RTHEAP_INDEX __SEGGER_RTL_RTHEAP_Map(__SEGGER_RTL_RTHEAP_SIZE Size) {
  int L1Index;
  int L2Index;
  //
  if (Size < __SEGGER_RTL_RTHEAP_L2_MAX_SZ) {
    //
    // Store small blocks in L1 list.
    //
    L1Index = 0;
    L2Index = (unsigned)Size / __SEGGER_RTL_RTHEAP_ALIGN_SZ;
    //
  } else {
    //
    // Store regular blocks in L2 list.
    //
    L1Index  = __SEGGER_RTL_RTHEAP_FLSL(Size);
    L2Index  = (int)(Size >> (L1Index - __SEGGER_RTL_RTHEAP_L2_INDEX_BITS)) - (1 << __SEGGER_RTL_RTHEAP_L2_INDEX_BITS);
    L1Index -= __SEGGER_RTL_RTHEAP_L2_MAX_BITS - 1;
  }
  //
  return __SEGGER_RTL_RTHEAP_MakeIndex(L1Index, L2Index);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_Pick()
*
*  Function description
*    Adjust index if indexed list is empty.
*
*  Parameters
*    pSelf - Pointer to allocator context.
*    Index - Mapping index corresponding to block size.
*
*  Return value
*    A mapping index of a block list that can satisfy the original
*    request.  If the L1Index member of the returned value is negative,
*    no there is no block list that can satisfy the request.
*/
static __SEGGER_RTL_RTHEAP_INDEX __SEGGER_RTL_RTHEAP_Pick(__SEGGER_RTL_RTHEAP_CONTEXT * pSelf,
                                                          __SEGGER_RTL_RTHEAP_INDEX     Index) {
  unsigned L1Map;
  unsigned L2Map;
  //
  // Search for a block in the list associated with the given index.
  //
  L2Map = pSelf->aL2Bitmap[Index.L1Index] & (~0u << Index.L2Index);
  if (L2Map == 0u) {
    //
    // No block exists. Search in the next largest first-level list.
    //
    L1Map = pSelf->L1Bitmap & (~0u << (Index.L1Index + 1));
    if (L1Map == 0u) {
      //
      // No free blocks available, memory has been exhausted.
      //
      Index.L1Index = -1;  // Indicate no solution
      return Index;
    }
    //
    Index.L1Index = __SEGGER_RTL_RTHEAP_FFS(L1Map);
    L2Map = pSelf->aL2Bitmap[Index.L1Index];
  }
  //
  HEAP_INT_CHECK(L2Map != 0);
  //
  Index.L2Index = __SEGGER_RTL_RTHEAP_FFS(L2Map);
  return Index;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_TakeBlockWithIndex()
*
*  Function description
*    Take a block from the free block list; the index of the
*    block is provided and does not require calculation.
*
*  Parameters
*    pSelf  - Pointer to allocator context.
*    pBlock - Pointer to block being removed.
*    Index  - Index of block being removed.
*/
static void __SEGGER_RTL_RTHEAP_TakeBlockWithIndex(__SEGGER_RTL_RTHEAP_CONTEXT * pSelf,
                                                   __SEGGER_RTL_RTHEAP_BLOCK   * pBlock,
                                                   __SEGGER_RTL_RTHEAP_INDEX     Index) {
  __SEGGER_RTL_RTHEAP_BLOCK *pPrev;
  __SEGGER_RTL_RTHEAP_BLOCK *pNext;
  //
  pPrev = pBlock->u.Free.pPrevFree;
  pNext = pBlock->u.Free.pNextFree;
  //
  // Remove block from doubly-linked list.
  //
  if (pNext) { pNext->u.Free.pPrevFree = pPrev; }
  if (pPrev) { pPrev->u.Free.pNextFree = pNext; }
  //
  // If this block is the head of the free list, set new head.
  //
  if (pSelf->aBlocks[Index.L1Index][Index.L2Index] == pBlock) {
    pSelf->aBlocks[Index.L1Index][Index.L2Index] = pNext;
    //
    // If the list is empty, reflect change in allocation bitmaps.
    //
    if (pNext == NULL) {
      //
      // Update L2 bitmap. If L2 bitmap is empty, L1 bitmap must follow.
      //
      if ((pSelf->aL2Bitmap[Index.L1Index] &= ~(1u << Index.L2Index)) == 0) {
        pSelf->L1Bitmap &= ~(1u << Index.L1Index);
      }
    }
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_TakeBlock()
*
*  Function description
*    Take a block from the free block list; the index of the
*    block must be calculated.
*
*  Parameters
*    pSelf  - Pointer to allocator context.
*    pBlock - Pointer to block being removed.
*/
static void __SEGGER_RTL_RTHEAP_TakeBlock(__SEGGER_RTL_RTHEAP_CONTEXT * pSelf,
                                          __SEGGER_RTL_RTHEAP_BLOCK   * pBlock) {
  __SEGGER_RTL_RTHEAP_TakeBlockWithIndex(pSelf,
                                         pBlock,
                                         __SEGGER_RTL_RTHEAP_Map(__SEGGER_RTL_RTHEAP_GetBlockSize(pBlock)));
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_GiveBlock()
*
*  Function description
*    Give a block to the free block list; the index of the
*    block must be calculated.
*
*  Parameters
*    pSelf  - Pointer to allocator context.
*    pBlock - Pointer to block being returned.
*/
static void __SEGGER_RTL_RTHEAP_GiveBlock(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf, __SEGGER_RTL_RTHEAP_BLOCK *pBlock) {
  __SEGGER_RTL_RTHEAP_BLOCK * pHead;
  __SEGGER_RTL_RTHEAP_INDEX   Index;
  //
  HEAP_INT_CHECK(pBlock != NULL);                                                      // Inserted block can't be null
  HEAP_INT_CHECK(HEAP_ALIGNED(__SEGGER_RTL_RTHEAP_ToData(pBlock), __SEGGER_RTL_RTHEAP_ALIGN_SZ)); // Block must be correctly aligned
  //
  // Get index.
  //
  Index = __SEGGER_RTL_RTHEAP_Map(__SEGGER_RTL_RTHEAP_GetBlockSize(pBlock));
  //
  // Maintain doubly-linked list, inserting block at head of list.
  //
  pHead                                        = pSelf->aBlocks[Index.L1Index][Index.L2Index];
  pSelf->aBlocks[Index.L1Index][Index.L2Index] = pBlock;
  pBlock->u.Free.pNextFree                     = pHead;
  pBlock->u.Free.pPrevFree                     = NULL;
  if (pHead != NULL) { pHead->u.Free.pPrevFree = pBlock; }
  //
  // Maintain bitmaps, indicate there is at least one free block in the corresponding list.
  //
  pSelf->L1Bitmap                             |= 1u << Index.L1Index;
  pSelf->aL2Bitmap[Index.L1Index]             |= 1u << Index.L2Index;
  //
  // If required, zap deallocated memory.
  //
  HEAP_INT_SET(pBlock+1,
               __SEGGER_RTL_RTHEAP_FILL_FREED,
               __SEGGER_RTL_RTHEAP_GetBlockSize(pBlock) - sizeof(__SEGGER_RTL_RTHEAP_BLOCK));
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_SplitBlock()
*
*  Function description
*    Split block into head and tail blocks.
*
*  Parameters
*    pBlock - Pointer to block being split.
*    Size   - Requested size of head block, including metadata.
*
*  Return value
*    == NULL - Block cannot be split.
*    != NULL - Pointer to the tail block, marked as free.
*
*  Additional information
*    The block flags of the head block are unchanged.
*/
static __SEGGER_RTL_RTHEAP_BLOCK *__SEGGER_RTL_RTHEAP_SplitBlock(__SEGGER_RTL_RTHEAP_BLOCK *pBlock, __SEGGER_RTL_RTHEAP_SIZE Size) {
  __SEGGER_RTL_RTHEAP_BLOCK * pTailBlock;
  __SEGGER_RTL_RTHEAP_SIZE    TailSize;
  //
  HEAP_INT_CHECK(Size % __SEGGER_RTL_RTHEAP_ALIGN_SZ == 0);
  //
  // See if a split is possible.  The user data area within the current block
  // must be large enough to contain a full block header with the requested user
  // data.
  //
  if (__SEGGER_RTL_RTHEAP_GetBlockSize(pBlock) < Size + sizeof(__SEGGER_RTL_RTHEAP_BLOCK)) {
    return NULL;
  }
  //
  // Calculate tail block size.
  //
  TailSize = __SEGGER_RTL_RTHEAP_GetBlockSize(pBlock) - Size;
  //
  // Reduce size of head block.  Block might be used or free,
  // therefore essential to preserve flags.
  //
  pBlock->SizeAndFlags = Size | (pBlock->SizeAndFlags & HEAP_FLAG_FREE);
  //
  // Create the tail block.
  //
  pTailBlock = __SEGGER_RTL_RTHEAP_SkipBlock(pBlock, Size);
  pTailBlock->SizeAndFlags = TailSize | HEAP_FLAG_FREE;
  pTailBlock->pBlockBefore = pBlock;
  __SEGGER_RTL_RTHEAP_FixBackinkTo(pTailBlock);
  //
  return pTailBlock;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_JoinBlocks()
*
*  Function description
*    Join two adjacent blocks irrespective of state.
*
*  Parameters
*    pBlock - Pointer to block.
*    pAfter - Pointer to immediately-following block.
*/
static void __SEGGER_RTL_RTHEAP_JoinBlocks(__SEGGER_RTL_RTHEAP_BLOCK *pBlock,
                                           __SEGGER_RTL_RTHEAP_BLOCK *pAfter) {
  pBlock->SizeAndFlags += __SEGGER_RTL_RTHEAP_GetBlockSize(pAfter);
  __SEGGER_RTL_RTHEAP_FixBackinkTo(pBlock);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_MergeWithBefore()
*
*  Function description
*    Try to merge a block with its left neighbor.
*
*  Parameters
*    pSelf  - Pointer to allocator context.
*    pBlock - Pointer to block.
*
*  Return value
*    Pointer to merged block or original block is not merged.
*/
static __SEGGER_RTL_RTHEAP_BLOCK *__SEGGER_RTL_RTHEAP_MergeWithBefore(__SEGGER_RTL_RTHEAP_CONTEXT * pSelf,
                                                                      __SEGGER_RTL_RTHEAP_BLOCK   * pBlock) {
  __SEGGER_RTL_RTHEAP_BLOCK *pBefore;
  //
  pBefore = __SEGGER_RTL_RTHEAP_GetBlockBefore(pBlock);
  if (__SEGGER_RTL_RTHEAP_IsBlockFree(pBefore)) {
    __SEGGER_RTL_RTHEAP_TakeBlock(pSelf, pBefore);
    __SEGGER_RTL_RTHEAP_JoinBlocks(pBefore, pBlock);
    pBlock = pBefore;
  }
  //
  return pBlock;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_MergeWithAfter()
*
*  Function description
*    Try to merge a block with its right neighbor.
*
*  Parameters
*    pSelf  - Pointer to allocator context.
*    pBlock - Pointer to block.
*/
static void __SEGGER_RTL_RTHEAP_MergeWithAfter(__SEGGER_RTL_RTHEAP_CONTEXT * pSelf,
                                               __SEGGER_RTL_RTHEAP_BLOCK   * pBlock) {
  __SEGGER_RTL_RTHEAP_BLOCK *pAfter;
  //
  pAfter = __SEGGER_RTL_RTHEAP_GetBlockAfter(pBlock);
  if (__SEGGER_RTL_RTHEAP_IsBlockFree(pAfter)) {
    __SEGGER_RTL_RTHEAP_TakeBlock(pSelf, pAfter);
    __SEGGER_RTL_RTHEAP_JoinBlocks(pBlock, pAfter);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_TrimBlock()
*
*  Function description
*    Try to merge a block with its right neighbor.
*
*  Parameters
*    pSelf  - Pointer to allocator context.
*    pBlock - Pointer to block to trim.
*    Size   - Requested size of trimmed block, including metadata.
*/
static void __SEGGER_RTL_RTHEAP_TrimBlock(__SEGGER_RTL_RTHEAP_CONTEXT * pSelf,
                                          __SEGGER_RTL_RTHEAP_BLOCK   * pBlock,
                                          __SEGGER_RTL_RTHEAP_SIZE      Size) {
  __SEGGER_RTL_RTHEAP_BLOCK *pExcess;
  //
  if ((pExcess = __SEGGER_RTL_RTHEAP_SplitBlock(pBlock, Size)) != NULL) {
    __SEGGER_RTL_RTHEAP_MergeWithAfter(pSelf, pExcess);
    __SEGGER_RTL_RTHEAP_GiveBlock(pSelf, pExcess);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_TakeFreeBlock()
*
*  Function description
*    Take a free block from the heap.
*
*  Parameters
*    pSelf - Pointer to allocator context.
*    Size  - Requested size of block, including metadata.
*
*  Return value
*    != NULL - Pointer to free block of at least Size bytes.
*    == NULL - No free block of given size.
*/
static __SEGGER_RTL_RTHEAP_BLOCK *__SEGGER_RTL_RTHEAP_TakeFreeBlock(__SEGGER_RTL_RTHEAP_CONTEXT * pSelf,
                                                                    __SEGGER_RTL_RTHEAP_SIZE      Size) {
  __SEGGER_RTL_RTHEAP_BLOCK * pBlock;
  __SEGGER_RTL_RTHEAP_INDEX   Index;
  //
  // Calculate appropriate free-list index according to size.
  //
  Index = __SEGGER_RTL_RTHEAP_Pick(pSelf, __SEGGER_RTL_RTHEAP_Map(Size));
  //
  // Check if we satisfied this request.
  //
  if (Index.L1Index >= 0) {
    //
    // Take the front of the free list.
    //
    pBlock = pSelf->aBlocks[Index.L1Index][Index.L2Index];
    __SEGGER_RTL_RTHEAP_TakeBlockWithIndex(pSelf, pBlock, Index);  // Optimized path: index is already calculated
    //
  } else {
    //
    // No block satisfies this request.
    //
    pBlock = NULL;
  }
  //
  return pBlock;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_WalkHeap()
*
*  Function description
*    Visit heap block and check bitmaps.
*
*  Parameters
*    pSelf   - Pointer to allocator context.
*    pIoFunc - Pointer to function for I/O.  If NULL, no I/O.
*
*  Return value
*    == 0 - Heap ok, no errors.
*    !- 0 - Heap in error, number of detected errors.
*/
static __SEGGER_RTL_RTHEAP_INLINE int __SEGGER_RTL_RTHEAP_WalkHeap(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf, __SEGGER_RTL_RTHEAP_IO_FUNC *pIoFunc) {
  __SEGGER_RTL_RTHEAP_BLOCK * pBlock;
  __SEGGER_RTL_RTHEAP_SIZE    Size;
  int                         LastUsed;
  int                         ThisUsed;
  int                         L1Index;
  int                         L2Index;
  int                         LocalErrorCnt;
  int                         TotalErrorCnt;
  //
  HEAP_IO("\n*** HEAP BLOCK REPORT ***\n\n");
  //
  TotalErrorCnt = 0;
  LocalErrorCnt = 0;
  LastUsed      = 1;
  //
  for (pBlock = pSelf->pOrigin; !__SEGGER_RTL_RTHEAP_IsSentinelBlock(pBlock); pBlock = __SEGGER_RTL_RTHEAP_GetBlockAfter(pBlock)) {
    Size  = __SEGGER_RTL_RTHEAP_GetBlockSize(pBlock);
    //
    HEAP_IO("%s block at %06x-%06x  size=%u",
            __SEGGER_RTL_RTHEAP_IsBlockFree(pBlock) ? "Free" : "Used",
            (unsigned)HEAP_PTRDIFF(pBlock, pSelf->pOrigin),
            (unsigned)HEAP_PTRDIFF(pBlock, pSelf->pOrigin) + (unsigned)Size - 1,
            (unsigned)Size);
    //
    if (pBlock == pSelf->pOrigin) {
      HEAP_CK(pBlock->pBlockBefore == NULL, " ...origin block should have null back-link");
    } else {
      HEAP_CK(__SEGGER_RTL_RTHEAP_GetBlockSize(pBlock) % __SEGGER_RTL_RTHEAP_ALIGN_SZ == 0, " ...block is misaligned");
      HEAP_CK(__SEGGER_RTL_RTHEAP_GetBlockSize(pBlock) >= sizeof(__SEGGER_RTL_RTHEAP_BLOCK), " ...block is too small");
      HEAP_CK(__SEGGER_RTL_RTHEAP_GetBlockAfter(pBlock)->pBlockBefore == pBlock, " ...successor block isn't back-linked to this block");
    }
    //
    ThisUsed = !__SEGGER_RTL_RTHEAP_IsBlockFree(pBlock);
    if (pBlock != pSelf->pOrigin && !LastUsed) {
      HEAP_CK(ThisUsed, " ...predecessor and this block cannot both be free");
    }
    LastUsed = ThisUsed;
    //
    HEAP_IO("\n");
  }
  //
  Size  = __SEGGER_RTL_RTHEAP_GetBlockSize(pBlock);
  //
  HEAP_IO("Mark block at %06x         size=%u\n",
          (unsigned)HEAP_PTRDIFF(pBlock, pSelf->pOrigin),
          (unsigned)Size);
  //
  HEAP_IO(LocalErrorCnt == 0 ? "\nNo errors\n\n" : "\nErrors found ***\n\n");
  //
  HEAP_IO("*** BITMAP ACCURACY REPORT ***\n");
  //
  for (L1Index = 0; L1Index < __SEGGER_RTL_RTHEAP_L1_INDEX_BITS; ++L1Index) {
    for (L2Index = 0; L2Index < __SEGGER_RTL_RTHEAP_L2_INDEX_SZ; ++L2Index) {
      __SEGGER_RTL_RTHEAP_INDEX Index;
      int                       L1Map;
      int                       L2Map;
      int                       L2Bitmap;
      //
      L1Map    = pSelf->L1Bitmap & (1u << L1Index);
      L2Bitmap = pSelf->aL2Bitmap[L1Index];
      L2Map    = L2Bitmap & (1u << L2Index);
      pBlock   = pSelf->aBlocks[L1Index][L2Index];
      //
      if (L1Map == 0) {
        HEAP_CK(L2Map == 0, "empty L1 map entry but corresponding L2 map is not empty\n");
      }
      if (L2Map == 0) {
        HEAP_CK(pBlock == NULL, "empty L2 map but free list is not empty\n");
      } else {
        //
        // Must be at least one free block.
        //
        HEAP_CK(pBlock != NULL, "map indicates nonempty list but corresponding free list is empty\n");
        //
        // Iterate over block list.
        //
        for (; pBlock != NULL; pBlock = pBlock->u.Free.pNextFree) {
          Index = __SEGGER_RTL_RTHEAP_Map(__SEGGER_RTL_RTHEAP_GetBlockSize(pBlock));
          HEAP_CK(__SEGGER_RTL_RTHEAP_IsBlockFree(pBlock), "block is free list but not flagged as free\n");
          HEAP_CK(Index.L1Index == L1Index && Index.L2Index == L2Index, "block-size index error, block assigned to incorrect list\n");
        }
      }
    }
  }
  //
  HEAP_IO(LocalErrorCnt == 0 ? "\nNo errors\n\n" : "\nErrors found ***\n\n");
  //
  return TotalErrorCnt;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_Init()
*
*  Function description
*    Initialize heap.
*
*  Parameters
*    pSelf   - Pointer to allocator context.
*    pMem    - Pointer to correctly-aligned memory for heap blocks.
*    MemSize - Number of bytes.
*
*  Additional information
*    pMem must point to memory that is correctly aligned for storage
*    of heap blocks (a multiple of __SEGGER_RTL_RTHEAP_ALIGN).  MemSize must be
*    able to hold at least three minimum-size blocks.
*/
void __SEGGER_RTL_RTHEAP_Init(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf, void *pMem, __SEGGER_RTL_RTHEAP_SIZE MemSize) {
  __SEGGER_RTL_RTHEAP_BLOCK * pBeginBlock;
  __SEGGER_RTL_RTHEAP_BLOCK * pHeapBlock;
  __SEGGER_RTL_RTHEAP_BLOCK * pEndBlock;
  __SEGGER_RTL_RTHEAP_SIZE    HeapSize;
  //
  // Memory block used for the heap must be correctly aligned.
  // Need a minimum of three (begin, heap, end) blocks.
  //
  HEAP_API_CHECK(HEAP_ALIGNED(pMem, __SEGGER_RTL_RTHEAP_ALIGN_SZ));
  HEAP_API_CHECK(MemSize >= 3*sizeof(__SEGGER_RTL_RTHEAP_BLOCK));
  //
  // If we've been given too much memory, we must trim it or mapping
  // indexes will be invalid causing writes outside bitmap arrays.
  //
  if (MemSize > HEAP_MAX_HEAP_SIZE) {
    MemSize = HEAP_MAX_HEAP_SIZE;
  }
  //
  // Clear context.
  //
  __SEGGER_RTL_RTHEAP_MEMSET(pSelf, 0, sizeof(*pSelf));
  pSelf->pOrigin = pMem;
  //
  HeapSize = __SEGGER_RTL_RTHEAP_AdjustDown(MemSize, sizeof(__SEGGER_RTL_RTHEAP_BLOCK));
  //
  // Clear heap memory.
  //
  HEAP_INT_SET(pMem, __SEGGER_RTL_RTHEAP_FILL_UNTOUCHED, HeapSize);
  //
  // Create an empty block block at the start of the heap.
  // This helps as there is always a "before" block that is
  // used, so every workable block in the heap has an immediate
  // predecessor.
  //
  pBeginBlock               = pMem;
  pBeginBlock->SizeAndFlags = sizeof(__SEGGER_RTL_RTHEAP_BLOCK);
  pBeginBlock->pBlockBefore = NULL;
  //
  // Create a block covering the heap, excluding the two sentinels.
  //
  pHeapBlock               = __SEGGER_RTL_RTHEAP_GetBlockAfter(pBeginBlock);
  pHeapBlock->SizeAndFlags = (HeapSize - 2*sizeof(__SEGGER_RTL_RTHEAP_BLOCK)) | HEAP_FLAG_FREE;
  pHeapBlock->pBlockBefore = pBeginBlock;
  //
  // Create end block.
  //
  pEndBlock               = __SEGGER_RTL_RTHEAP_GetBlockAfter(pHeapBlock);
  pEndBlock->SizeAndFlags = 0;  // Size zero, in use
  pEndBlock->pBlockBefore = pHeapBlock;
  //
  // Place free heap block into heap management structure.
  //
  __SEGGER_RTL_RTHEAP_GiveBlock(pSelf, pHeapBlock);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_Alloc()
*
*  Function description
*    Allocate memory.
*
*  Parameters
*    pSelf - Pointer to allocator context.
*    Size  - Number of bytes to allocate.
*
*  Return value
*    == NULL - Cannot allocate block or Size is zero.
*    != NULL - Pointer to correctly aligned new block.
*/
void * __SEGGER_RTL_RTHEAP_Alloc(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf, __SEGGER_RTL_RTHEAP_SIZE Size) {
  __SEGGER_RTL_RTHEAP_BLOCK *pBlock;
  __SEGGER_RTL_RTHEAP_BLOCK *pExcess;
  //
  // Allocating a zero size in C returns an implementation-defined value.
  // This implementation elects to return a null pointer.  For requests that
  // exceed the manageable heap or block size, handle them here to avoid
  // unsigned overflows when constraining the size.
  //
  if (Size == 0) {
    return NULL;
  } else if (Size >= HEAP_MAX_HEAP_SIZE) {
    return NULL;
  }
  //
  // Include block metadata in size calculation.
  //
  Size = __SEGGER_RTL_RTHEAP_ConstrainSize(Size + HEAP_DATA_OFFSET, __SEGGER_RTL_RTHEAP_ALIGN_SZ);
  //
  // Find block that can satisfy request.
  //
  pBlock = __SEGGER_RTL_RTHEAP_TakeFreeBlock(pSelf, Size);
  //
  if (pBlock == NULL) {
    //
    // No block satisfies this request.
    //
    return NULL;
    //
  } else {
    //
    // Split off any tail that is surplus to requirements and return it
    // to the free list.  The excess doesn't need merging as the split
    // block is free and the following block used.
    //
    if ((pExcess = __SEGGER_RTL_RTHEAP_SplitBlock(pBlock, Size)) != NULL) {
      __SEGGER_RTL_RTHEAP_GiveBlock(pSelf, pExcess);
    }
    //
    // Mark allocated block as used.
    //
    __SEGGER_RTL_RTHEAP_MarkUsed(pBlock);
    //
    // Zap allocated data if required.
    //
    HEAP_INT_SET(__SEGGER_RTL_RTHEAP_ToData(pBlock),
                 __SEGGER_RTL_RTHEAP_FILL_ALLOCATED,
                 Size - HEAP_DATA_OFFSET);
    //
    // Return a pointer to user data.
    //
    return __SEGGER_RTL_RTHEAP_ToData(pBlock);
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_AllocAligned()
*
*  Function description
*    Allocate memory.
*
*  Parameters
*    pSelf - Pointer to allocator context.
*    Size  - Number of bytes to allocate.
*    Align - Minimum alignment for returned pointer; must be
*            a power of two.
*
*  Return value
*    == NULL - Cannot allocate block or Size is zero.
*    != NULL - Pointer to correctly aligned new block.
*/
void * __SEGGER_RTL_RTHEAP_AllocAligned(__SEGGER_RTL_RTHEAP_CONTEXT * pSelf,
                                        __SEGGER_RTL_RTHEAP_SIZE      Size,
                                        __SEGGER_RTL_RTHEAP_SIZE      Align) {
  __SEGGER_RTL_RTHEAP_BLOCK * pBlock;
  __SEGGER_RTL_RTHEAP_BLOCK * pRetain;
  __SEGGER_RTL_RTHEAP_SIZE    SplitPos;
  //
  // Alignment is constrained to powers of two.
  //
  HEAP_API_CHECK(HEAP_ISPOW2(Align));
  //
  // Allocating a zero size in C returns an implementation-defined value.
  // This implementation elects to return a null pointer.  For requests
  // that exceed the manageable heap or block size, handle them here to
  // avoid unsigned overflows when constraining the size.
  //
  if (Size == 0) {
    return NULL;
  } else if (Size >= HEAP_MAX_HEAP_SIZE) {
    return NULL;
  }
  //
  Size = __SEGGER_RTL_RTHEAP_ConstrainSize(Size, __SEGGER_RTL_RTHEAP_ALIGN_SZ);
  //
  // Impose minimal alignment.
  //
  if (Align < __SEGGER_RTL_RTHEAP_ALIGN_SZ) {
    //
    // Requested alignment is satisfied by minimal block alignment.
    //
    Align  = __SEGGER_RTL_RTHEAP_ALIGN_SZ;
    pBlock = __SEGGER_RTL_RTHEAP_TakeFreeBlock(pSelf, Size);
    if (pBlock == NULL) {
      return NULL;
    }
  } else {
    //
    // Requested alignment needs to be satisfied by overallocation.
    //
    pBlock = __SEGGER_RTL_RTHEAP_TakeFreeBlock(pSelf, Size + 2*sizeof(__SEGGER_RTL_RTHEAP_BLOCK) + Align);
    if (pBlock == NULL) {
      return NULL;
    }
    //
    // Move the pointer into alignment and compute split position.
    //
    SplitPos = HEAP_PTRDIFF(__SEGGER_RTL_RTHEAP_ToBlock(__SEGGER_RTL_RTHEAP_AlignPtr(__SEGGER_RTL_RTHEAP_ToData(pBlock+1), Align)),
                            pBlock);
    //
    // Return unused data before aligned block.
    //
    pRetain = __SEGGER_RTL_RTHEAP_SplitBlock(pBlock, SplitPos);  // Split must succeed
    __SEGGER_RTL_RTHEAP_GiveBlock(pSelf, pBlock);
    pBlock = pRetain;
    //
    // Size needs to include metadata.
    //
    Size += HEAP_DATA_OFFSET;
  }
  //
  // Return any unused data after aligned block.
  //
  __SEGGER_RTL_RTHEAP_MarkUsed(pBlock);
  __SEGGER_RTL_RTHEAP_TrimBlock(pSelf, pBlock, Size);
  //
  HEAP_INT_CHECK(HEAP_ALIGNED(__SEGGER_RTL_RTHEAP_ToData(pBlock), Align));
  HEAP_INT_CHECK(__SEGGER_RTL_RTHEAP_GetBlockSize(pBlock) >= Size);
  //
  // Zap allocated data if required.
  //
  HEAP_INT_SET(__SEGGER_RTL_RTHEAP_ToData(pBlock),
               __SEGGER_RTL_RTHEAP_FILL_ALLOCATED,
               Size - HEAP_DATA_OFFSET);
  //
  return __SEGGER_RTL_RTHEAP_ToData(pBlock);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_Realloc()
*
*  Function description
*    Resize allocated memory.
*
*  Parameters
*    pSelf - Pointer to allocator context.
*    pMem  - Pointer to existing block to reallocate; can be NULL.
*    Size  - Number of bytes to allocate.
*
*  Return value
*    == NULL - Cannot allocate block or Size is zero.
*    != NULL - Pointer to correctly aligned reallocated block.
*
*  Additional information
*    The allocator attempts to reallocate the memory without moving,
*    if possible.  If this is not possible, it will try to reallocate
*    without unduly increasing memory pressure.
*/
void * __SEGGER_RTL_RTHEAP_Realloc(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf, void *pMem, __SEGGER_RTL_RTHEAP_SIZE Size) {
  __SEGGER_RTL_RTHEAP_BLOCK * pBlock;
  __SEGGER_RTL_RTHEAP_BLOCK * pAdjunct;
  __SEGGER_RTL_RTHEAP_SIZE    BlockSize;
  __SEGGER_RTL_RTHEAP_SIZE    JoinedSize;
  void                      * pNew;
  //
  if (pMem == NULL) {
    //
    // For realloc(NULL, 0)        elect to return a NULL pointer.
    //     realloc(NULL, nonzero)  equivalent to malloc(nonzero).
    //
    return Size == 0 ? NULL : __SEGGER_RTL_RTHEAP_Alloc(pSelf, Size);
    //
  } else if (Size >= HEAP_MAX_HEAP_SIZE) {
    //
    // For requests that exceed the manageable heap or block size,
    // handle them here to avoid unsigned overflows when constraining
    // the size.
    //
    return NULL;
    //
  } else {
    //
    // Allocated pointers are always returned correctly aligned: check
    // validity of incoming pointer.  If this is validated, check block
    // is in use.  This is a cheap test and traps double-dispose.
    //
    HEAP_API_CHECK(HEAP_ALIGNED(pMem, __SEGGER_RTL_RTHEAP_ALIGN_SZ));
    HEAP_API_CHECK(!__SEGGER_RTL_RTHEAP_IsBlockFree(__SEGGER_RTL_RTHEAP_ToBlock(pMem)));
    //
    pBlock    = __SEGGER_RTL_RTHEAP_ToBlock(pMem);
    BlockSize = __SEGGER_RTL_RTHEAP_GetBlockSize(pBlock);
    Size      = __SEGGER_RTL_RTHEAP_ConstrainSize(Size + HEAP_DATA_OFFSET,
                                             __SEGGER_RTL_RTHEAP_ALIGN_SZ);
    //
    // Same size or smaller block, shrinking?
    //
    if (Size <= BlockSize) {
      if (Size < BlockSize) {
        __SEGGER_RTL_RTHEAP_TrimBlock(pSelf, pBlock, Size);
      }
      return __SEGGER_RTL_RTHEAP_ToData(pBlock);
    }
    //
    // Expanding forward. If the block after is free and we joined these
    // blocks, would that be able to satisfy the request without moving
    // the data?
    //
    pAdjunct = __SEGGER_RTL_RTHEAP_GetBlockAfter(pBlock);
    if (__SEGGER_RTL_RTHEAP_IsBlockFree(pAdjunct)) {
      JoinedSize = BlockSize + __SEGGER_RTL_RTHEAP_GetBlockSize(pAdjunct);
      if (JoinedSize >= Size) {
        __SEGGER_RTL_RTHEAP_MergeWithAfter(pSelf, pBlock);
        __SEGGER_RTL_RTHEAP_TrimBlock(pSelf, pBlock, Size);
        return __SEGGER_RTL_RTHEAP_ToData(pBlock);
      }
    }
    //
    // Expanding backward. If the block before is free and we joined these
    // blocks, would that be able to satisfy the request by sliding the
    // data into it?
    //
    pAdjunct = __SEGGER_RTL_RTHEAP_GetBlockBefore(pBlock);
    if (__SEGGER_RTL_RTHEAP_IsBlockFree(pAdjunct)) {
      JoinedSize = BlockSize + __SEGGER_RTL_RTHEAP_GetBlockSize(pAdjunct);
      if (JoinedSize >= Size) {
        __SEGGER_RTL_RTHEAP_MergeWithBefore(pSelf, pBlock);
        __SEGGER_RTL_RTHEAP_MarkUsed(pAdjunct);
        __SEGGER_RTL_RTHEAP_MEMMOVE(__SEGGER_RTL_RTHEAP_ToData(pAdjunct), __SEGGER_RTL_RTHEAP_ToData(pBlock), BlockSize);
        __SEGGER_RTL_RTHEAP_TrimBlock(pSelf, pAdjunct, Size);
        return __SEGGER_RTL_RTHEAP_ToData(pAdjunct);
      }
    }
    //
    // Exhausted possibilities of expanding the current
    // block in-place, only option is to allocate and copy.
    //
    pNew = __SEGGER_RTL_RTHEAP_Alloc(pSelf, Size);
    if (pNew != NULL) {
      __SEGGER_RTL_RTHEAP_MEMMOVE(pNew, pMem, BlockSize - HEAP_DATA_OFFSET);  // Know BlockSize < Size
      __SEGGER_RTL_RTHEAP_Free(pSelf, pMem);
    }
    //
    return pNew;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_Free()
*
*  Function description
*    Free memory.
*
*  Parameters
*    pSelf - Pointer to allocator context.
*    pMem  - Pointer to memory to returned; must have been allocted
*            and not previously freed.  Can be NULL.
*/
void __SEGGER_RTL_RTHEAP_Free(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf, void *pMem) {
  __SEGGER_RTL_RTHEAP_BLOCK *pBlock;
  //
  // Perfectly valid to free a null pointer.
  //
  if (pMem == NULL) {
    return;
  }
  //
  // Allocated pointers are always returned correctly aligned: check
  // validity of incoming pointer.
  //
  HEAP_API_CHECK(HEAP_ALIGNED(pMem, __SEGGER_RTL_RTHEAP_ALIGN_SZ));
  HEAP_API_CHECK(!__SEGGER_RTL_RTHEAP_IsBlockFree(__SEGGER_RTL_RTHEAP_ToBlock(pMem)));
  //
  // Merge with any free blocks before and after and return to free list.
  //
  pBlock = __SEGGER_RTL_RTHEAP_ToBlock(pMem);
  __SEGGER_RTL_RTHEAP_MarkFree(pBlock);
  __SEGGER_RTL_RTHEAP_MergeWithAfter(pSelf, pBlock);
  __SEGGER_RTL_RTHEAP_GiveBlock(pSelf, __SEGGER_RTL_RTHEAP_MergeWithBefore(pSelf, pBlock));
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_IsEmptyHeap()
*
*  Function description
*    Is heap empty?
*
*  Parameters
*    pSelf - Pointer to allocator context.
*
*  Return value
*    == 0 - Heap is not empty.
*    != 0 - Heap is empty.
*/
int __SEGGER_RTL_RTHEAP_IsEmptyHeap(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf) {
  return __SEGGER_RTL_RTHEAP_IsBlockFree(&pSelf->pOrigin[1]) &&
         __SEGGER_RTL_RTHEAP_GetBlockSize(__SEGGER_RTL_RTHEAP_GetBlockAfter(&pSelf->pOrigin[1])) == 0;
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_CheckHeap()
*
*  Function description
*    Check heap integrity.
*
*  Parameters
*    pSelf - Pointer to allocator context.
*
*  Return value
*    == 0 - Heap ok, no errors.
*    !- 0 - Heap in error, number of detected errors.
*/
int __SEGGER_RTL_RTHEAP_CheckHeap(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf) {
  return __SEGGER_RTL_RTHEAP_WalkHeap(pSelf, NULL);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_DumpHeap()
*
*  Function description
*    Dump heap data.
*
*  Parameters
*    pSelf   - Pointer to allocator context.
*    pIoFunc - Pointer to formatted I/O function.
*
*  Return value
*    == 0 - Heap ok, no errors.
*    !- 0 - Heap in error, number of detected errors.
*/
int __SEGGER_RTL_RTHEAP_DumpHeap(__SEGGER_RTL_RTHEAP_CONTEXT *pSelf, __SEGGER_RTL_RTHEAP_IO_FUNC *pIoFunc) {
  return __SEGGER_RTL_RTHEAP_WalkHeap(pSelf, pIoFunc);
}

/*********************************************************************
*
*       __SEGGER_RTL_RTHEAP_CheckConfig()
*
*  Function description
*    Cheap check of heap configuration.
*
*  Parameters
*    pIoFunc - Pointer to function for I/O.  If NULL, no I/O.
*
*  Return value
*    == 0 - Self-test passed.
*    !- 0 - Self-test failed.
*/
int __SEGGER_RTL_RTHEAP_CheckConfig(__SEGGER_RTL_RTHEAP_IO_FUNC *pIoFunc) {
  __SEGGER_RTL_RTHEAP_INDEX Index;
  __SEGGER_RTL_RTHEAP_SIZE  Size;
  int                       LocalErrorCnt;
  int                       TotalErrorCnt;
  //
  LocalErrorCnt = 0;
  TotalErrorCnt = 0;
  //
  if (pIoFunc) {
    pIoFunc("__SEGGER_RTL_RTHEAP_SMALL_BLOCK_SZ    = %u\n", __SEGGER_RTL_RTHEAP_L2_MAX_SZ);
    pIoFunc("__SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS = %u\n", __SEGGER_RTL_RTHEAP_L1_INDEX_MAX_BITS);
    pIoFunc("__SEGGER_RTL_RTHEAP_ALIGN_BITS        = %u\n", __SEGGER_RTL_RTHEAP_ALIGN_BITS);
    pIoFunc("__SEGGER_RTL_RTHEAP_L2_MAX_BITS       = %u\n", __SEGGER_RTL_RTHEAP_L2_MAX_BITS);
    pIoFunc("__SEGGER_RTL_RTHEAP_L1_INDEX_BITS     = %u\n", __SEGGER_RTL_RTHEAP_L1_INDEX_BITS);
    pIoFunc("__SEGGER_RTL_RTHEAP_L2_INDEX_BITS     = %u\n", __SEGGER_RTL_RTHEAP_L2_INDEX_BITS);
  }
  //
  Size  = 0;
  Index = __SEGGER_RTL_RTHEAP_Map(Size);
  HEAP_CK(Index.L1Index < __SEGGER_RTL_RTHEAP_L1_INDEX_BITS, "expected L1 index within bounds");
  //
  Size  = HEAP_MAX_HEAP_SIZE;
  Index = __SEGGER_RTL_RTHEAP_Map(Size);
  HEAP_CK(Index.L1Index < __SEGGER_RTL_RTHEAP_L1_INDEX_BITS, "expected L1 index within bounds");
  //
  Size  = HEAP_MAX_HEAP_SIZE+HEAP_DATA_OFFSET-1;
  Index = __SEGGER_RTL_RTHEAP_Map(Size);
  HEAP_CK(Index.L1Index < __SEGGER_RTL_RTHEAP_L1_INDEX_BITS, "expected L1 index within bounds");
  //
  Size  = HEAP_MAX_HEAP_SIZE + HEAP_DATA_OFFSET;
  Index = __SEGGER_RTL_RTHEAP_Map(Size);
  HEAP_CK(Index.L1Index >= __SEGGER_RTL_RTHEAP_L1_INDEX_BITS, "expected L1 index out of bounds");
  //
  __SEGGER_RTL_USE_PARA(LocalErrorCnt);
  //
  return TotalErrorCnt;
}

/*************************** End of file ****************************/
