#ifndef _MICROCRT_PRINTF_BRIDGE_H_
#define _MICROCRT_PRINTF_BRIDGE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct __file_str {
    struct __file file;
    char         *pos;
    char         *end;
    size_t        size;
    bool          alloc;
};

int __file_str_get(FILE *stream);
int __file_wstr_get(FILE *stream);
int __file_str_put(char c, FILE *stream);
int __file_str_put_alloc(char c, FILE *stream);

#define FDEV_STRING_WRITE_END(_s, _n) (((int)(_n) < 0) ? NULL : ((_n) ? (_s) + (_n) - 1 : (_s)))

#define FDEV_SETUP_STRING_WRITE(_s, _end)                                    \
    {                                                                        \
        .file = { .flags = __SWR, .put = __file_str_put, __LOCK_INIT_NONE }, \
        .pos = (_s),                                                         \
        .end = (_end),                                                       \
    }

#define FDEV_SETUP_STRING_ALLOC()                                                  \
    {                                                                              \
        .file = { .flags = __SWR, .put = __file_str_put_alloc, __LOCK_INIT_NONE }, \
        .pos = NULL,                                                               \
        .end = NULL,                                                               \
        .size = 0,                                                                 \
        .alloc = false,                                                            \
    }

#define FDEV_SETUP_STRING_ALLOC_BUF(_buf, _size)                                   \
    {                                                                              \
        .file = { .flags = __SWR, .put = __file_str_put_alloc, __LOCK_INIT_NONE }, \
        .pos = _buf,                                                               \
        .end = (char *)(_buf) + (_size),                                           \
        .size = _size,                                                             \
        .alloc = false,                                                            \
    }

#ifdef __STDIO_LOCKING
void __flockfile_init(FILE *f);
#define __LOCK_NONE      ((_LOCK_RECURSIVE_T)(uintptr_t)1)
#define __LOCK_INIT_NONE .lock = __LOCK_NONE
#else
#define __LOCK_INIT_NONE
#endif

#define __funlock_return(f, v) \
    do {                       \
        __funlockfile(f);      \
        return (v);            \
    } while (0)

static inline void
__flockfile(FILE *f)
{
    (void)f;
#ifdef __STDIO_LOCKING
    if (!f->lock)
        __flockfile_init(f);
    if (f->lock != __LOCK_NONE)
        __lock_acquire_recursive(f->lock);
#endif
}

static inline void
__funlockfile(FILE *f)
{
    (void)f;
#ifdef __STDIO_LOCKING
    if (f->lock != __LOCK_NONE)
        __lock_release_recursive(f->lock);
#endif
}

#endif
