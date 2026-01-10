#ifndef __STDLIB_H__
#define __STDLIB_H__

typedef unsigned long size_t;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define RAND_MAX 32767

// --- String conversion ---
double atof(const char *str);
int    atoi(const char *str);
long   atol(const char *str);
double strtod(const char *str, char **endptr);
long   strtol(const char *str, char **endptr, int base);

// --- Dynamic Memory management ---
void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t new_size);
void  free(void *ptr);

// --- Process control ---
void  abort(void);
void  exit(int status);
int   atexit(void (*func)(void));
int   system(const char *command);
char *getenv(const char *name);

// --- Algorithm (Search and Sorting) ---
void *bsearch(const void *key, const void *base, size_t nitems, size_t size,
              int (*compar)(const void *, const void *));
void  qsort(void *base, size_t nitems, size_t size,
           int (*compar)(const void *, const void *));

// --- Integer operation ---
int  abs(int n);
long labs(long n);

#endif // __STDLIB_H__
