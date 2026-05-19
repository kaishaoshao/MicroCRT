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

#include "stdlib.h"
#include "__SEGGER_RTL_Int.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

// Require STACKSIZE >= log2(address-space-size)
#define STACKSIZE         32
#define SUBDIVISION_LIMIT 10

static __SEGGER_RTL_ALWAYS_INLINE void exchange(char *p1, char *p2, size_t size) {
  do {
    char temp = *p1;
    *p1++ = *p2;
    *p2++ = temp;
    size--;
  }
  while (size != 0);
}

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_partition_sort()
*
*/
static void __SEGGER_RTL_partition_sort(void *base, size_t nmemb, size_t size,
                                        int (*compar)(const void *, const void *)) {
  char * l;
  char * r;
  char * pivot;
  int    s1, s2;
  int    stackpointer = 1;
  void * basestack[STACKSIZE];
  int    sizestack[STACKSIZE];
  //
  basestack[0] = base;
  sizestack[0] = nmemb;
  //
  while (stackpointer) {
    char *b = (char *) basestack[--stackpointer];
    int n = sizestack[stackpointer];
    //
    // Sort interval.
    //
    for (;;) {
      int halfn = (n >> 1) * size;  // index of middle in the list
      char *bmid = b + halfn;
      char *btop;
      if ((n & 1) == 0) {
        halfn -= size;
      }
      btop = bmid + halfn;
      //
      // Sort first, middle and last items in the segment into order.
      //
      if (compar(b, bmid) > 0) {
        exchange(b, bmid, size);
      }
      if (compar(bmid, btop) > 0) {
        if (compar(b, btop) > 0) {
          exchange(b, btop, size);
        }
        exchange(bmid, btop, size);
      }
      //
      // Now have the first and last elements in place with a useful pivot.
      //
      r = btop - size;
      exchange(bmid, r, size);
      l = b;
      pivot = r;

      // Sweep inwards partitioning elements on the basis of the pivot.
      for (;;) {
        do  {
          l += size; 
        } while (compar(l, pivot) < 0);
        do {
          r -= size; 
        } while (compar(r, pivot) > 0);
        if (r <= l) {
          break;
        }
        exchange(l, r, size);
      }
      //
      // Move the pivot value to the middle of the array.
      //
      exchange(l, pivot, size);
      //
      // s1 and s2 get the sizes of the sublists.
      //
      s1 = ((l - b) / size) - 1;
      s2 = n - s1 - 1;
      //
      // Only subdivide meaningful sublists.
      //
      if (s1 > s2) {
        if (s2 > SUBDIVISION_LIMIT) {
          basestack[stackpointer] = b;
          sizestack[stackpointer++] = s1;
          b = l;
          n = s2;
        } else if (s1 > SUBDIVISION_LIMIT) {
          n = s1;
        } else {
          break;
        }
      } else {
        if (s1 > SUBDIVISION_LIMIT) {
          basestack[stackpointer] = l;
          sizestack[stackpointer++] = s2;
          n = s1;
        } else if (s2 > SUBDIVISION_LIMIT) {
          b = l;
          n = s2;
        } else {
          break;
        }
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
*       qsort()
*
*  Function description
*    Sort array.
*
*  Parameters
*    base    - Pointer to the start of the array.
*    nmemb   - Number of array elements.
*    sz      - Number of characters per array element.
*    compare - Pointer to element comparison function.
*
*  Additional information
*    Sorts the array pointed to by base using the compare function. The
*    array should have nmemb elements of sz bytes. The compare function
*    should return a negative value if the first parameter is less than
*    the second parameter, zero if the parameters are equal, and a
*    positive value if the first parameter is greater than the second
*    parameter.
*
*  Thread safety
*    Safe.
*/
void __SEGGER_RTL_PUBLIC_API qsort(void *base, size_t nmemb, size_t sz,
                                   int (*compare)(const void *elem1, const void *elem2)) {
  char * b;
  char * endp;
  size_t j;
  //
  // Trivially sorted.
  //
  if (sz == 0) {
    return;
  }
  //
  // Meaningful (non-trivial) sort.
  //
  if (nmemb > SUBDIVISION_LIMIT) {
    __SEGGER_RTL_partition_sort(base, nmemb, sz, compare);
  }
  //
  // Insertion sort of array.
  //
  b = (char *)base;
  endp = b + (nmemb-1)*sz;
  while (b < endp) {
    char *b1 = b;
    b += sz;
    //
    // Find where to insert this item.
    //
    while (b1 >= (char *)base && compare(b1, b) > 0) {
      b1 -= sz;
    }
    b1 += sz;
    //
    // Exchange.
    //
    for (j = 0; j < sz; ++j) {
      char *by;
      char *bx = b + j;
      char save = *bx;
      for (by = bx - sz; by >= b1; by -= sz) {
        by[sz] = *by;
      }
      by[sz] = save;
    }
  }
}

/*********************************************************************
*
*       bsearch()
*
*  Function description
*    Search sorted array.
*
*  Parameters
*    key     - Pointer to object to search for.
*    base    - Pointer to the start of the array.
*    nmemb   - Number of array elements.
*    sz      - Number of characters per array element.
*    compare - Pointer to element comparison function.
*
*  Return value
*    == NULL - Key not found.
*    != NULL - Pointer to found object.
*
*  Additional information
*    Searches the array pointed to by base for the specified key
*    and returns a pointer to the first entry that matches, or null
*    if no match. The array should have nmemb elements of sz bytes
*    and be sorted by the same algorithm as the compare function.
*    
*    The compare function should return a negative value if the
*    first parameter is less than second parameter, zero if the
*    parameters are equal, and a positive value if the first parameter
*    is greater than the second parameter.
*
*  Thread safety
*    Safe.
*/
void * __SEGGER_RTL_PUBLIC_API bsearch(const void *key, const void *base,
                                       size_t nmemb, size_t sz,
                                       int (*compare)(const void *elem1, const void *elem2)) {
  void * midp;
  int    midn;
  int    c;
  //
  for (;;) {
    if (nmemb == 0) {
      return NULL;
    } else if (nmemb == 1) {
      //
      // If there is only one item left in the array it is the only one that
      // needs to be looked at.
      //
      if (compare(key, base) == 0) {
        return (void *)base;
      } else {
        return NULL;
      }
    } else {
      //
      // Compute index of middle item.
      //
      midn = nmemb >> 1;
      //
      // Form pointer to element.
      //
      midp = (char *)base + midn*sz;
      //
      // Compare keys.
      //
      c = compare(key, midp);
      if (c == 0) {
        //
        // Key found.
        //
        return midp;
      } else if (c < 0) {
        //
        // Key is to the left.
        //
        nmemb = midn;
      } else {
        //
        // Key is to the right.
        //
        base = (char *)midp + sz; // exclude midpoint
        nmemb = nmemb - midn - 1;
      }
    }
  }
}

/*************************** End of file ****************************/
