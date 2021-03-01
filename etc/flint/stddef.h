/*
 * Minimum correct ANSI C header.
 */
#ifndef ETC_FLINT_STDDEF_H
#define ETC_FLINT_STDDEF_H

#ifndef TYPEDEF_PTRDIFF_T
#define TYPEDEF_PTRDIFF_T
typedef long ptrdiff_t;
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

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#endif /* ETC_FLINT_STDDEF_H */
