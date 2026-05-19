/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Purpose: malloc()-only heap allocator.

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

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

typedef struct {
  char   * pOrigin;
  size_t   Remaining;
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
  size_t   aligned_sz;
  char   * ptr;
  //
  aligned_sz = (sz + HEAP_CHUNK_SIZE) & (0u - HEAP_CHUNK_SIZE);
  if (aligned_sz < sz) {
    return NULL;  // sz too big, overflowed
  }
  if (__SEGGER_RTL_heap_globals.Remaining < aligned_sz) {
    return NULL;  // out of memory
  }
  ptr = __SEGGER_RTL_heap_globals.pOrigin + __SEGGER_RTL_heap_globals.Remaining;
  __SEGGER_RTL_heap_globals.Remaining -= aligned_sz;
  return ptr;
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
  //
  // Aligned allocation not supported.
  //
  __SEGGER_RTL_USE_PARA(align);
  __SEGGER_RTL_USE_PARA(sz);
  //
  return NULL;
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
  //
  // Free not supported.
  //
  __SEGGER_RTL_USE_PARA(ptr);
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
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_init_heap(void *ptr, size_t size) {
  __SEGGER_RTL_heap_globals.pOrigin   = ptr;
  __SEGGER_RTL_heap_globals.Remaining = size;
}

/*************************** End of file ****************************/
