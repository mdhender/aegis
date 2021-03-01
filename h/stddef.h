/*
 * MANIFEST: minimal stddef.h substitute
 */

#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef NULL
#define NULL 0
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef long ptrdiff_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long size_t;
#endif

#ifndef ofsetof
#define offsetof(type, identifier) \
	(size_t)((char *)&((type *)0)->identifier - (char *)0)
#endif

#endif /* _STDDEF_H */
