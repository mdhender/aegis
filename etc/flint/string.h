/*
 * Minimum correct ANSI C header.
 */
#ifndef ETC_FLINT_STRING_H
#define ETC_FLINT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TYPEDEF_SIZE_T
#define TYPEDEF_SIZE_T
typedef unsigned long size_t;
#endif

#ifndef NULL
#define NULL 0
#endif

void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memchr(const void *, int, size_t);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, size_t);
char *strcat(char *, const char *);
char *strncat(char *, const char *, size_t);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
int strcoll(const char *, const char *);
size_t strxfrm(char *, const char *, size_t);
char *strchr(const char *, int);
char *strrchr(const char *, int);
size_t strcspn(const char *, const char *);
size_t strspn(const char *, const char *);
char *strpbrk(const char *, const char *);
char *strstr(const char *, const char *);
char *strtok(char *, const char *);
size_t strlen(const char *);
char *strerror(int);
char *strsignal(int);
char *index(const char *, int);
char *rindex(const char *, int);

#ifdef __cplusplus
}
#endif

#endif /* ETC_FLINT_STRING_H */
