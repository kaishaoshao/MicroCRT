#include <stdio.h>

typedef __SIZE_TYPE__ ssize_t;
ssize_t _write(int __fd, const void *__buf, size_t __nbyte);
ssize_t _read(int __fd, void *__buf, size_t __nbyte);

#define FDEV_SETUP_STREAM(__put, __get, __flush, __flags)                      \
  {                                                                            \
      .flags = (__flags),                                                      \
      .put = (__put),                                                          \
      .get = (__get),                                                          \
      .flush = (__flush),                                                      \
  }

static int
mculib_getc(FILE *file)
{
    (void) file;
    char c = 0;

    _read(0, &c, 1);
    return c;
}

static int
mculib_stdout_putc(char c, FILE *file)
{
    (void) file;
    _write(1, &c, 1);
    return (int) c;
}

static int
mculib_stderr_putc(char c, FILE *file)
{
    (void) file;
    _write(2, &c, 1);
    return (int) c;
}

static int
mculib_flush(FILE *file)
{
    (void) file;
    return 0;
}

static FILE __stdin =
    FDEV_SETUP_STREAM(NULL, mculib_getc, NULL, __SRD);
static FILE __stdout =
    FDEV_SETUP_STREAM(mculib_stdout_putc, NULL, mculib_flush, __SWR);
static FILE __stderr =
    FDEV_SETUP_STREAM(mculib_stderr_putc, NULL, mculib_flush, __SWR);

FILE *const stdin = &__stdin;
FILE *const stdout = &__stdout;
FILE *const stderr = &__stderr;
