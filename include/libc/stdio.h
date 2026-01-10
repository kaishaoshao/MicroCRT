#ifndef __STDLIB_H__
#define __STDLIB_H__

//  file end or error number
#undef  EOF
#define EOF (-1)

// NULL ptr
#undef  NULL
#define NULL ((void *)0)

// Buffer size
#define BUFSIZ 8192

typedef struct _IO_FILE FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

// --- File operation ---
FILE *fopen(const char *filename, const char *mode);
int   fclose(FILE *stream);
int   fflush(FILE *stream);

// --- Format input/output ----
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t n, const char *format, ...);

int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);

// --- Character input/output ---
int getchar(void);
int fgetc(FILE *stream);

int putchar(int character);
int fputc(int character, FILE *stream);

// --- String input/output ---
char *fgets(char *str, int n, FILE *stream);
char *gets(char *str);

int   fputs(const char *str, FILE *stream);

// --- Binary (direct) input/output ---
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

// --- File location ---
int  fseek(FILE *stream, long offset,int origin);
long ftell(FILE *stream);
void rewind(FILE *stream);

// --- Error handling ---
void perror(const char *s);
int  feof(FILE *stream);
int  ferror(FILE *stream);

#endif // __STDLIB_H__
