//$(LICENSE_COMMENT)

#include <stdlib.h>

typedef void (*cxa_atexitfn)(void *);

#define NUM_CXA_ATEXITFNS 32

static cxa_atexitfn __cxa_atexitfns[NUM_CXA_ATEXITFNS];
static void *__cxa_atexitobjects[NUM_CXA_ATEXITFNS];
static unsigned __cxa_atexitfn_count;

static void
_execute_cxa_at_exit_fns(void)
{
  while (__cxa_atexitfn_count)
    {
      __cxa_atexitfns[__cxa_atexitfn_count-1](__cxa_atexitobjects[__cxa_atexitfn_count-1]);
      __cxa_atexitfn_count--;
    }
}

int __cxa_atexit(void (*) (void *), void *, void *)__attribute__((weak));
int
__cxa_atexit(void (*fn) (void *), void * arg, void *dso_handle)
{
  if (!__cxa_atexitfn_count)
    {
      if (atexit(_execute_cxa_at_exit_fns))
        return 1;
    }
  if (__cxa_atexitfn_count < NUM_CXA_ATEXITFNS)
    {
      __cxa_atexitfns[__cxa_atexitfn_count] = fn;
      __cxa_atexitobjects[__cxa_atexitfn_count] = arg;
      __cxa_atexitfn_count++;
      return 0;
    }
  return 1;   
}

#if defined(__CROSSWORKS_ARM)
int __aeabi_atexit(void *, void (*) (void *), void *)__attribute__((weak));
int
__aeabi_atexit(void *object, void (*destructor) (void *), void *dso_handle)
{
    return __cxa_atexit(destructor, object, dso_handle);
}
#endif
