#ifndef __STRING_H__
#define __STRING_H__

void  *memcpy(void *s1, const void *s2, size_t n);
void  *mempcpy(void *s1, const void *s2, size_t n);
void  *memccpy(void *s1, const void *s2, int count, size_t n);
void  *memmove(void *s1, const void *s2, size_t n);
void  *memchr(const void *s, int c, size_t n);
void  *memrchr(const void *s, int c, size_t n);
void  *memmem(const void *s1, size_t n1, const void *s2, size_t n2);
void  *memset(void *s, int c, size_t n);
int    memcmp(const void *s1, const void *s2, size_t n);
size_t memlcp(const void *s1, const void *s2, size_t n);

char  *strcpy(char *s1, const char *s2);
char  *strpcpy(char *s1, const char *s2);
char  *strncpy(char *s1, const char *s2, size_t n);
char  *stpncpy(char *s1, const char *s2, size_t n);
char  *strcat(char *s1, const char *s2);
char  *strncat(char *s1, const char *s2, size_t n);
int    strcmp(const char *s1, const char *s2);
int    strncmp(const char *s1, const char *s2, size_t n);
int    strcasecmp(const char *s1, const char *s2);
int    strncasecmp(const char *s1, const char *s2, size_t n);
int    strcoll(const char *s1, const char *s2);
char  *strchr(const char *s, int c);
char  *strnchr(const char *s, size_t n, int c);
size_t strlcpy(char *s1, const char *s2, size_t n);
size_t strlcat(char *s1, const char *s2, size_t n);
size_t strcspn(const char *s1, const char *s2);
size_t strspn(const char *s1, const char *s2);
char  *strpbrk(const char *s1, const char *s2);
char  *strrchr(const char *s, int c);
char  *strstr(const char *s1, const char *s2);
char  *strnstr(const char *s1, const char *s2, size_t n);
char  *strcasestr(const char *s1, const char *s2);
char  *strncasestr(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t n);
size_t strxfrm(char *s1, const char *s2, size_t n);
char  *strtok(char *s1, const char *s2);
char  *strtok_r(char *s1, const char *s2, char **lasts);
char  *strdup(const char *s);
char  *strndup(const char *s, size_t n);
char  *strerror(int errnum);

#endif // __STRING_H__
