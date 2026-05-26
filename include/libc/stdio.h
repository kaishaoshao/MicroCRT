#ifndef __STDIO_H__
#define __STDIO_H__

#include <stddef.h>
#include <stdint.h>

typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_copy(dest, src) __builtin_va_copy(dest, src)

#ifndef __MICROCRT_UNGETC_SIZE
#define __MICROCRT_UNGETC_SIZE 2
#endif

#if __MICROCRT_UNGETC_SIZE == 4
typedef uint32_t __ungetc_t;
#else
typedef uint16_t __ungetc_t;
#endif

struct __file {
  __ungetc_t unget;
  uint8_t flags;
#define __SRD 0x0001
#define __SWR 0x0002
#define __SERR 0x0004
#define __SEOF 0x0008
#define __SCLOSE 0x0010
#define __SEXT 0x0020
#define __SBUF 0x0040
#define __SWIDE 0x0080
  int (*put)(char, struct __file *);
  int (*get)(struct __file *);
  int (*flush)(struct __file *);
};

typedef struct __file __FILE;
typedef __FILE FILE;

extern FILE *const stdin;
extern FILE *const stdout;
extern FILE *const stderr;

#define _FDEV_SETUP_RW (__SRD | __SWR)

#define __IO_VARIANT_DOUBLE  'd'
#define __IO_VARIANT_FLOAT   'f'
#define __IO_VARIANT_LLONG   'l'
#define __IO_VARIANT_INTEGER 'i'
#define __IO_VARIANT_MINIMAL 'm'

#ifndef __IO_DEFAULT
#define __IO_DEFAULT __IO_VARIANT_DOUBLE
#endif

#ifndef _PICOLIBC_PRINTF
#define _PICOLIBC_PRINTF __IO_DEFAULT
#endif

#if _PICOLIBC_PRINTF == __IO_VARIANT_FLOAT
static __inline uint32_t __printf_float(float f)
{
  union {
    float f;
    uint32_t u;
  } u = { .f = f };
  return u.u;
}
#define printf_float(x) __printf_float(x)
#else
#define printf_float(x) ((double)(x))
#endif

#if __cplusplus >= 201703L || __STDC_VERSION__ > 201710L
#define __fallthrough [[fallthrough]]
#elif defined(__has_attribute)
#if __has_attribute(__fallthrough__)
#define __fallthrough __attribute__((__fallthrough__))
#else
#define __fallthrough do { } while (0)
#endif
#else
#define __fallthrough do { } while (0)
#endif

#undef EOF
#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t n, const char *format, ...);
int vprintf(const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t n, const char *format, va_list ap);
int iprintf(const char *format, ...);
int viprintf(const char *format, va_list ap);
int fiprintf(FILE *stream, const char *format, ...);
int vfiprintf(FILE *stream, const char *format, va_list ap);
int siprintf(char *str, const char *format, ...);
int vsiprintf(char *str, const char *format, va_list ap);
int printf_full(const char *format, ...);
int fprintf_full(FILE *stream, const char *format, ...);
int sprintf_full(char *str, const char *format, ...);
int snprintf_full(char *str, size_t n, const char *format, ...);
int vprintf_full(const char *format, va_list ap);
int vfprintf_full(FILE *stream, const char *format, va_list ap);
int vsprintf_full(char *str, const char *format, va_list ap);
int vsnprintf_full(char *str, size_t n, const char *format, va_list ap);

int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int vfscanf(FILE *stream, const char *format, va_list ap);

int getchar(void);
int fgetc(FILE *stream);
int putchar(int character);
int fputc(int character, FILE *stream);
int putc(int character, FILE *stream);

char *fgets(char *str, int n, FILE *stream);
char *gets(char *str);
int fputs(const char *str, FILE *stream);
int puts(const char *str);

size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *stream);
int fflush(FILE *stream);
int fseek(FILE *stream, long offset, int origin);
long ftell(FILE *stream);
void rewind(FILE *stream);

void perror(const char *s);
int feof(FILE *stream);
int ferror(FILE *stream);

#ifdef __cplusplus
}
#endif

#endif // __STDIO_H__
