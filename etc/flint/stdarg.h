/*
 * Minimum correct ANSI C header.
 */
#ifndef ETC_FLINT_STDARG_H
#define ETC_FLINT_STDARG_H

#define va_start(AP, LASTARG) ((AP) = (void *)(&(LASTARG) + 1))

typedef void *va_list;
#ifndef TYPEDEF_VA_ALIST
#define TYPEDEF_VA_ALIST
typedef void *va_alist;
#endif /* TYPEDEF_VA_ALIST */

#define va_end(AP) ((void)0)

#define va_arg(AP, TYPE) \
    (((AP) = (void *)((TYPE *)(AP) + 1)), ((TYPE *)(AP))[-1])

#endif /* ETC_FLINT_STDARG_H */
