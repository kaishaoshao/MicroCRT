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
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stddef.h"
#include "assert.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#if __SEGGER_RTL_TYPESET == 64
  #define HEAP_CHUNK_SIZE  16   /* Must be a power of two. */
#else
  #define HEAP_CHUNK_SIZE   8   /* Must be a power of two. */
#endif

#define ADDADR(x,n)            ((void *)((unsigned char *)(x)+(n)))
#define SUBADR(x,n)            ((void *)((unsigned char *)(x)-(n)))
#define ALIGN_UP(num, align)   (((num) + ((align) - 1)) & ~((align) - 1))

#define UserDataToBlockHdr(x)  ((__SEGGER_RTL_BLOCK_t *)SUBADR(x, sizeof(__SEGGER_RTL_BLOCK_t)))
#define BlockHdrToUserData(x)  (ADDADR(x, sizeof(__SEGGER_RTL_BLOCK_t)))

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

typedef union __SEGGER_RTL_BLOCK_t {
  struct {
    union __SEGGER_RTL_BLOCK_t * next;
    size_t                       size;
  } Free;
  struct {
    size_t                       size;
  } Used;
  struct {
    max_align_t                  max_align;
  } Aligner;
} __SEGGER_RTL_BLOCK_t;

typedef struct {
  __SEGGER_RTL_BLOCK_t * pFreeList;  // Pointer to free list.
} HEAP;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static HEAP __SEGGER_RTL_heap_globals;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_free_regular()
*
*  Function description
*    Free allocated memory for reuse, incoming pointer is regular,
*    nonaligned pointer.
*
*  Parameters
*    ptr - Pointer to object to free.
*/
static void __SEGGER_RTL_free_regular(void *ptr) {
  __SEGGER_RTL_BLOCK_t * pBlock;
  __SEGGER_RTL_BLOCK_t * rover;
  __SEGGER_RTL_BLOCK_t * prev;
  size_t                 size;
  //
  if (ptr == NULL) {
    return;
  }
  //
  // Adjust backwards to actual memory block origin.
  //
  pBlock = UserDataToBlockHdr(ptr);
  //
  // Recover size.
  //
  size = pBlock->Used.size;
  //
  if (ptr) {
    //
    // Deallocated onto full/empty heap?
    //
    if (__SEGGER_RTL_heap_globals.pFreeList == NULL) {
      //
      // Yes, add this floating block to the heap.
      //
      __SEGGER_RTL_heap_globals.pFreeList = pBlock;
      __SEGGER_RTL_heap_globals.pFreeList->Free.next = NULL;
      __SEGGER_RTL_heap_globals.pFreeList->Free.size = size;
    } else {
      //
      // Search for block adjacent to pBlock, but with lower address.
      //
      rover = __SEGGER_RTL_heap_globals.pFreeList;
      prev  = NULL;
      while (rover && rover < pBlock) {
        prev = rover; rover = rover->Free.next;
      }
      //
      // Return block to free list.
      //
      pBlock->Free.next = rover;
      pBlock->Free.size = size;
      if (prev) {
        prev->Free.next = pBlock;
      } else {
        __SEGGER_RTL_heap_globals.pFreeList = pBlock;
      }
      //
      // See if can join with block to right.
      //
      if (rover && (ADDADR(pBlock, size) == rover)) {
        pBlock->Free.size += rover->Free.size;
        pBlock->Free.next  = rover->Free.next;
      }
      //
      // See if can join with block to left.
      //
      if (prev && ADDADR(prev, prev->Free.size) == pBlock) {
        prev->Free.size += pBlock->Free.size;
        prev->Free.next  = pBlock->Free.next;
      }
    }
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_alloc()
*
*  Function description
*    Allocate space for single object.
*
*  Parameters
*    sz - Number of characters to allocate for the object.
*
*  Return value
*    Returns a null pointer if the space for the object cannot be
*    allocated from free memory; if space for the object can be allocated,
*    __SEGGER_RTL_alloc() returns a pointer to the start of the allocated
*    space.
*/
void * __SEGGER_RTL_alloc(size_t sz) {
  __SEGGER_RTL_BLOCK_t * p;
  __SEGGER_RTL_BLOCK_t * q;
  __SEGGER_RTL_BLOCK_t * best;
  size_t                 best_size;    
  //
  // Round up to a multiple of HEAP_CHUNK_SIZE and account for header.
  //
  sz = (sz + sizeof(__SEGGER_RTL_BLOCK_t) + HEAP_CHUNK_SIZE-1) & (0u-HEAP_CHUNK_SIZE);
  //
  // Find first block that could satisfy request.
  //
  p = __SEGGER_RTL_heap_globals.pFreeList;
  q = NULL;
  //
  // No best-fit block.
  //
  best_size = ~(size_t)0;
  best      = NULL;
  //
  // Search for best-fit block.
  //
  while (p) {
    //
    // Check whether block is correct size.
    //
    if (p->Free.size == sz) {
      //
      // Exact fit - remove block from the free list.
      //
      if (q) {
        q->Free.next = p->Free.next;
      } else {
        __SEGGER_RTL_heap_globals.pFreeList = p->Free.next;
      }
      //
      // Short-cut return.
      //
      p->Used.size = sz;
      return BlockHdrToUserData(p);
      //
    } else if (p->Free.size > sz && p->Free.size < best_size) {  /* Check whether block can be split. */
      best      = p;
      best_size = p->Free.size;
    }
    //
    // Consider next block.
    //
    q = p;
    p = p->Free.next;
  }
  //
  // Block must be split; take free memory from top of block.
  //
  if (best) {
    best->Free.size -= sz;
    p = (__SEGGER_RTL_BLOCK_t *)ADDADR(best, best_size - sz);
    p->Used.size = sz;
    return BlockHdrToUserData(p);
  } else {
    return NULL;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_aligned_alloc()
*
*  Function description
*    Allocate space for aligned single object.
*
*  Parameters
*    align - Alignment of object.
*    sz    - Number of characters to allocate for the object.
*
*  Additional information
*    Allocates space for an object whose size is specified by sz,
*    whose alignment is align, and whose value is indeterminate.
*
*  Return value
*    Returns a null pointer if the space for the object cannot be
*    allocated from free memory or the alignment cannot be satisfied;
*    if space for the object can be allocated, __SEGGER_RTL_alloc()
*    returns a pointer to the start of the allocated, aligned space.
*/
void * __SEGGER_RTL_aligned_alloc(size_t align, size_t sz) {
  __SEGGER_RTL_BLOCK_t * pBlock;
  void                 * pUserAligned;
  void                 * mem;
  //
  pBlock = NULL;
  //
  if ((align & (align - 1)) == 0 && align > 0 && sz) {
    //
    // Find a block containing an address that satisfies the
    // alignment requirement.
    //
    mem = __SEGGER_RTL_alloc(sz + sizeof(__SEGGER_RTL_BLOCK_t) + (align - 1));
    if (mem) {
      //
      // Align pointer within the allocated block.
      //
      pUserAligned = (void *)ALIGN_UP(((uintptr_t)mem + sizeof(__SEGGER_RTL_BLOCK_t)), align);
      //
      // Store the allocated pointer behind the aligned pointer and mark it
      // odd so free() can differentiate.
      //
      pBlock = UserDataToBlockHdr(pUserAligned);
      pBlock->Used.size = (size_t)(mem) | 1u;
    }
  }
  //
  return pBlock ? BlockHdrToUserData(pBlock) : NULL;
}

/*********************************************************************
*
*       __SEGGER_RTL_free()
*
*  Function description
*    Free allocated memory for reuse.
*
*  Parameters
*    ptr - Pointer to object to free.
*/
void __SEGGER_RTL_free(void *ptr) {
  __SEGGER_RTL_BLOCK_t * pBlock;
  size_t sz;
  //
  // If input is null, nothing to free.
  //
  if (ptr == NULL) {
    return;
  }
  //
  pBlock = UserDataToBlockHdr(ptr);
  sz = pBlock->Used.size;
  if (sz & 1) {
    //
    // Stored value is not a size, but a pointer the the
    // origin of the block.
    //
    ptr = (void *)(sz & ~(size_t)1);
  }
  //
  __SEGGER_RTL_free_regular(ptr);
}

/*********************************************************************
*
*       __SEGGER_RTL_realloc()
*
*  Function description
*    Resize or allocate memory space.
*
*  Parameters
*    ptr - Pointer to resize, or NULL to allocate.
*    sz  - New size of object.
*
*  Additional information
*    Deallocates the old object pointed to by ptr and returns a
*    pointer to a new object that has the size specified by sz.
*    The contents of the new object is identical to that of the
*    old object prior to deallocation, up to the lesser of the
*    new and old sizes. Any bytes in the new object beyond the
*    size of the old object have indeterminate values.
*    
*    If ptr is a null pointer, realloc() behaves like malloc()
*    for the specified size. If memory for the new object cannot
*    be allocated, the old object is not deallocated and its value
*    is unchanged.
*
*    If ptr does not match a pointer earlier returned by calloc(),
*    malloc(), or realloc(), or if the space has been deallocated
*    by a call to free() or realloc(), the behavior is undefined.
*
*  Return value
*    Returns a pointer to the new object (which may have the same
*    value as a pointer to the old object), or a null pointer if
*    the new object could not be allocated.
*/
void * __SEGGER_RTL_realloc(void *ptr, size_t sz) {
  __SEGGER_RTL_BLOCK_t * pBlock;
  void                 * pNew;
  void                 * mem;
  size_t                 existing_size;
  //
  // Allocate area of requested size.
  //
  pNew = __SEGGER_RTL_alloc(sz);
  //
  // If no memory, keep existing block and indicate failure to reallocate.
  //
  if (pNew == NULL) {
    return pNew;
  }
  //
  // Copy existing block if we were given one.
  //
  if (ptr) {
    //
    // Recover size.
    //
    pBlock = UserDataToBlockHdr(ptr);
    existing_size = pBlock->Used.size;
    if (existing_size & 1) {
      //
      // Pointer is hiding before block, get size of entire
      // allocated block.
      //
      mem = (void *)(existing_size & ~(size_t)1);  // Pointer to allocated block.
      pBlock = UserDataToBlockHdr(mem);
      existing_size  = pBlock->Used.size;          // Entire size of allocated block.
      //
      // Calculate valid size beyond pointer.
      //
      existing_size = (size_t)(ADDADR(mem, existing_size) - (uintptr_t)(ptr));
    }
    //
    // Limit size to copy.
    //
    if (existing_size < sz) {
      sz = existing_size;
    }
    //
    // Make copy of data.
    //
    (memcpy)(pNew, ptr, sz);
    //
    // And we're done with the original block.
    //
    __SEGGER_RTL_free(ptr);
  }
  //
  // Return newly allocated block.
  //
  return pNew;
}

/*********************************************************************
*
*       __SEGGER_RTL_init_heap()
*
*  Function description
*    Initializes the heap.
*
*  Parameters
*    pMem - Pointer to correctly-aligned heap memory to manage.
*    size - Size of managed area in bytes.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_init_heap(void *pMem, size_t size) {
  //
#if defined(static_assert)
  static_assert(sizeof(size_t) == sizeof(void *), "__SEGGER_RTL_init_heap");
#else
  assert(sizeof(size_t) == sizeof(void *));
#endif
  //
  if (size >= HEAP_CHUNK_SIZE) {
    __SEGGER_RTL_heap_globals.pFreeList            = pMem;
    __SEGGER_RTL_heap_globals.pFreeList->Free.next = NULL;
    __SEGGER_RTL_heap_globals.pFreeList->Free.size = size;
  }
}

/*************************** End of file ****************************/
