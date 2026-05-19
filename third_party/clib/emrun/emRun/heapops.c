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

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#ifdef __DISABLE_INTERRUPTS_LOCKING
#include "heapops_atomic_locking.c"
#else

#define __SEGGER_RTL_X_heap_lock __heap_lock
#define __SEGGER_RTL_X_heap_unlock __heap_unlock


/*********************************************************************
*
*       __SEGGER_RTL_X_heap_lock()
*
*  Function description
*    Lock heap.
*
*  Additional information
*    This function is called to lock access to the heap before
*    allocation or deallocation is processed.  This is only
*    required for multitasking systems where heap operations
*    may possibly be called called from different threads.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_heap_lock(void) {
  /* Pass */
}

/*********************************************************************
*
*       __SEGGER_RTL_X_heap_unlock()
*
*  Function description
*    Unlock heap.
*
*  Additional information
*    This function is called to unlock access to the heap after
*    allocation or deallocation has completed.  This is only
*    required for multitasking systems where heap operations
*    may possibly be called called from different threads.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_X_heap_unlock(void) {
  /* Pass */
}
#endif
/*********************************************************************
*
*       malloc()
*
*  Function description
*    Allocate space for single object.
*
*  Parameters
*    sz - Number of characters to allocate for the object.
*
*  Additional information
*    Allocates space for an object whose size is specified by sz
*    and whose value is indeterminate.
*
*  Return value
*    Returns a null pointer if the space for the object cannot be
*    allocated from free memory; if space for the object can be allocated,
*    malloc() returns a pointer to the start of the allocated space.
*
*  Thread safety
*    Safe [if configured].
*/
void * __SEGGER_RTL_PUBLIC_API malloc(size_t sz) {
  void *ptr;
  //
  __SEGGER_RTL_X_heap_lock();
  ptr = __SEGGER_RTL_alloc(sz);
  __SEGGER_RTL_X_heap_unlock();
  //
  return ptr;
}

/*********************************************************************
*
*       aligned_alloc()
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
*    if space for the object can be allocated, aligned_alloc() returns
*    a pointer to the start of the allocated, aligned space.
*
*  Thread safety
*    Safe [if configured].
*/
void * __SEGGER_RTL_PUBLIC_API aligned_alloc(size_t align, size_t sz) {
  void *ptr;
  //
  __SEGGER_RTL_X_heap_lock();
  ptr = __SEGGER_RTL_aligned_alloc(align, sz);
  __SEGGER_RTL_X_heap_unlock();
  //
  return ptr;
}

/*********************************************************************
*
*       free()
*
*  Function description
*    Free allocated memory for reuse.
*
*  Parameters
*    ptr - Pointer to object to free.
*
*  Additional information
*    Causes the space pointed to by ptr to be deallocated, that
*    is, made available for further allocation. If ptr is a null
*    pointer, no action occurs.
*
*    If ptr does not match a pointer earlier returned by calloc(),
*    malloc(), or realloc(), or if the space has been deallocated
*    by a call to free() or realloc(), the behavior is undefined.
*
*  Thread safety
*    Safe [if configured].
*/
void __SEGGER_RTL_PUBLIC_API free(void *ptr) {
  __SEGGER_RTL_X_heap_lock();
  __SEGGER_RTL_free(ptr);
  __SEGGER_RTL_X_heap_unlock();
}

/*********************************************************************
*
*       realloc()
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
*
*  Thread safety
*    Safe [if configured].
*/
void * __SEGGER_RTL_PUBLIC_API realloc(void *ptr, size_t sz) {
  __SEGGER_RTL_X_heap_lock();
  ptr = __SEGGER_RTL_realloc(ptr, sz);
  __SEGGER_RTL_X_heap_unlock();
  return ptr;
}

/*********************************************************************
*
*       calloc()
*
*  Function description
*    Allocate space for multiple objects and zero them.
*
*  Parameters
*    nobj - Number of objects to allocate.
*    sz   - Number of characters to allocate per object.
*
*  Additional information
*    Allocates space for an array of nobj objects, each of
*    whose size is sz. The space is initialized to all zero
*    bits.
*
*  Return value
*    Returns a null pointer if the space for the object cannot be
*    allocated from free memory; if space for the object can be allocated,
*    calloc() returns a pointer to the start of the allocated space.
*
*  Thread safety
*    Safe [if configured].
*/
void * __SEGGER_RTL_PUBLIC_API calloc(size_t nobj, size_t sz) {
  void   * m;
  size_t   arrsize;
  //
  arrsize = nobj * sz;
  //
  if (arrsize < nobj || arrsize < sz) {
    return NULL;
  }
  //
  __SEGGER_RTL_X_heap_lock();
  m = __SEGGER_RTL_alloc(arrsize);
  __SEGGER_RTL_X_heap_unlock();
  if (m && arrsize > 0) {
    (memset)(m, 0, arrsize);
  }
  //
  return m;
}

/*************************** End of file ****************************/
