/*
 * Minimum correct ANSI C header.
 */
#ifndef	ETC_FLINT_STDLIB_H
#define	ETC_FLINT_STDLIB_H

#ifndef TYPEDEF_SIZE_T
#define TYPEDEF_SIZE_T
typedef unsigned long size_t;
#endif

#ifndef TYPEDEF_WCHAR_T
#define TYPEDEF_WCHAR_T
typedef unsigned long wchar_t;
#endif

#ifndef NULL
#define NULL 0
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define RAND_MAX 32767

extern int _mb_cur_max(void);
#define MB_CUR_MAX _mb_cur_max()

typedef struct
  {
    int quot;			/* Quotient.  */
    int rem;			/* Remainder.  */
  } div_t;

typedef struct
  {
    long int quot;		/* Quotient.  */
    long int rem;		/* Remainder.  */
  } ldiv_t;

double atof(const char *);
int atoi(const char *);
long int atol(const char *);
double strtod(const char *, char **);
long int strtol(const char *, char **, int);
unsigned long int strtoul(const char *, char **, int);
void *malloc(size_t);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);
void *bsearch(const void *, const void *, size_t, size_t,
    int(*)(const void *, const void *));
void qsort(void *, size_t, size_t,
    int(*)(const void *, const void *));
int abs(int);
long labs(long);
div_t div(int, int);
ldiv_t ldiv(long, long);
int rand(void);
void srand(unsigned);
void abort(void);
int atexit(void (*)(void));
void exit(int);
char *getenv(const char *);
int system(const char *);
int mblen(const char *, size_t);
int mbtowc(wchar_t *, const char *, size_t);
int wctomb(char *, wchar_t);
size_t mbstowcs(wchar_t *, const char *, size_t);
size_t wcstombs(char *, const wchar_t *, size_t);

#endif /* ETC_FLINT_STDLIB_H */
