#include "__SEGGER_RTL_Int.h"
#include "atomic_locking.c"

static int en;

void  __SEGGER_RTL_X_heap_lock(void)
{  
  en = __atomic_lock();
}

void  __SEGGER_RTL_X_heap_unlock(void)
{
  __atomic_unlock(en);
}
