
#ifndef ETC_FLINT_WCHAR_H
#define ETC_FLINT_WCHAR_H


#ifndef TYPEDEF_FILE
#define TYPEDEF_FILE
typedef struct FILE FILE;
#endif

#ifndef TYPEDEF_SIZE_T
#define TYPEDEF_SIZE_T
typedef unsigned long size_t;
#endif

#ifndef TYPEDEF_WCHAR_T
#define TYPEDEF_WCHAR_T
typedef unsigned long wchar_t;
#endif

#ifndef TYPEDEF_WINT_T
#define TYPEDEF_WINT_T
typedef unsigned long wint_t;
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TYPEDEF_MBSTATE_T
#define TYPEDEF_MBSTATE_T
typedef struct mbstate_t mbstate_t;
#endif

#ifndef WCHAR_MIN
# define WCHAR_MIN ((wchar_t) 0)
# define WCHAR_MAX (~WCHAR_MIN)
#endif

#ifndef WEOF
# define WEOF (0xffffffffu)
#endif

/* #include <wctype.h> */

struct tm;

wchar_t *wcscpy(wchar_t *, const wchar_t *);
wchar_t *wcsncpy(wchar_t *, const wchar_t *, size_t);

wchar_t *wcscat(wchar_t *, const wchar_t *);
wchar_t *wcsncat(wchar_t *, const wchar_t *, size_t);

int wcscmp(const wchar_t *, const wchar_t *);
int wcsncmp(const wchar_t *, const wchar_t *, size_t);

int wcscasecmp(const wchar_t *, const wchar_t *);

int wcsncasecmp(const wchar_t *, const wchar_t *, size_t);

int wcscoll(const wchar_t *, const wchar_t *);
size_t wcsxfrm(wchar_t *, const wchar_t *, size_t);

wchar_t *wcsdup(const wchar_t *);

wchar_t *wcschr(const wchar_t *, wchar_t);
wchar_t *wcsrchr(const wchar_t *, wchar_t);

size_t wcscspn(const wchar_t *, const wchar_t *);
size_t wcsspn(const wchar_t *, const wchar_t *);
wchar_t *wcspbrk(const wchar_t *, const wchar_t *);
wchar_t *wcsstr(const wchar_t *, const wchar_t *);

wchar_t *wcstok(wchar_t *, const wchar_t *, wchar_t **);

size_t wcslen(const wchar_t *);

wchar_t *wmemchr(const wchar_t *, wchar_t, size_t);

int wmemcmp(const wchar_t *, const wchar_t *, size_t);

wchar_t *wmemcpy(wchar_t *, const wchar_t *, size_t);

wchar_t *wmemmove(wchar_t *, const wchar_t *, size_t);

wchar_t *wmemset(wchar_t *, wchar_t, size_t);

wint_t btowc(int);

int wctob(wint_t);

int mbsinit(const mbstate_t *);

size_t mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);

size_t wcrtomb(char *, wchar_t, mbstate_t *);

size_t mbrlen(const char *, size_t, mbstate_t *);

size_t mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *);

size_t wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *);

int wcwidth(wint_t);

int wcswidth(const wchar_t *, size_t);

double wcstod(const wchar_t *, wchar_t **);

long int wcstol(const wchar_t *, wchar_t **, int);

unsigned long int wcstoul(const wchar_t *, wchar_t **, int);

#endif /* ETC_FLINT_WCHAR_H */
