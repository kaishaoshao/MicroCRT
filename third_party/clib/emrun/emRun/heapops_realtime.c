/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Purpose: emRun interface to SEGGER real-time O(1) memory allocator.

*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Int.h"
#include "__SEGGER_RTL_RTHEAP.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static __SEGGER_RTL_RTHEAP_CONTEXT __SEGGER_RTL_HeapContext;

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
  return __SEGGER_RTL_RTHEAP_Alloc(&__SEGGER_RTL_HeapContext, sz);
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
  return __SEGGER_RTL_RTHEAP_AllocAligned(&__SEGGER_RTL_HeapContext, sz, align);
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
  return __SEGGER_RTL_RTHEAP_Free(&__SEGGER_RTL_HeapContext, ptr);
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
  return __SEGGER_RTL_RTHEAP_Realloc(&__SEGGER_RTL_HeapContext, ptr, sz);
}

/*********************************************************************
*
*       __SEGGER_RTL_init_heap()
*
*  Function description
*    Initializes the heap.
*
*  Parameters
*    ptr  - Pointer to correctly-aligned heap memory to manage.
*    size - Size of managed area in bytes.
*/
void __SEGGER_RTL_PUBLIC_API __SEGGER_RTL_init_heap(void *ptr, size_t size) {
  __SEGGER_RTL_RTHEAP_Init(&__SEGGER_RTL_HeapContext, ptr, size);
}

/*************************** End of file ****************************/
