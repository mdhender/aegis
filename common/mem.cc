//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1999, 2002-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stddef.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>
#include <common/ac/errno.h>
#include <common/ac/new.h>

#include <common/mem.h>
#include <common/error.h>

#ifdef DMALLOC
#undef new
#undef delete
#include <dmalloc.h>
#endif


#ifdef _AIX

//
// Yet another AIX stupidity:
// malloc does not guarantee that the space is available in swap.
//

#include <common/ac/signal.h>
#include <setjmp.h>

static jmp_buf  aix_bungy;


//
// Catch SIGDANGER and longjmp to aix_touch.
//

static void
aix_danger(int n)
{
    longjmp(aix_bungy, 1);
}

//
// Touch the pages that cover [p, p+nbytes-1].
//

static int
aix_touch(void *vp, size_t nbytes)
{
    char            *p;
    char            *endp;
    int             pgsize;
    volatile char   c;
    void            (*oldsig)(int);

    oldsig = signal(SIGDANGER, aix_danger);
    if (setjmp(aix_bungy))
    {
	signal(SIGDANGER, oldsig);
	return -1;
    }

    //
    // A load is enough to cause the
    // allocation of the paging space
    //
    p = vp;
    pgsize = getpagesize();
    endp = p + nbytes;
    while (p < endp)
    {
	c = *(volatile char *)p;
	p += pgsize;
    }

    //
    // restore the signal handler
    //
    signal(SIGDANGER, oldsig);
    return 0;
}

#endif

void *
mem_alloc(size_t n)
{
    if (n < 1)
	n = 1;
    int old_errno = errno;
    errno = 0;
    void *cp = malloc(n);
    if (!cp)
    {
	if (!errno)
	    errno = ENOMEM;
	nfatal("malloc(%ld)", (long)n);
    }
#ifdef _AIX
    //
    // watch out for AIX stupidity
    //
    if (aix_touch(cp, n))
    {
	errno = ENOMEM;
	nfatal("malloc(%ld)", (long)n);
    }
#endif
    errno = old_errno;
    return cp;
}

#ifdef DMALLOC

void *
dmem_alloc(const char* file, int line, size_t n)
{
    if (n < 1)
	n = 1;
    int old_errno = errno;
    errno = 0;
    void *cp = dmalloc_malloc(file, line, n, DMALLOC_FUNC_MALLOC, 0, 0);
    if (!cp)
    {
	if (!errno)
	    errno = ENOMEM;
	nfatal("malloc(%ld)", (long)n);
    }
#ifdef _AIX
    //
    // watch out for AIX stupidity
    //
    if (aix_touch(cp, n))
    {
	errno = ENOMEM;
	nfatal("malloc(%ld)", (long)n);
    }
#endif
    errno = old_errno;
    return cp;
}

#endif // DMALLOC


void *
mem_alloc_clear(size_t n)
{
    void *cp = mem_alloc(n);
    memset(cp, 0, n);
    return cp;
}


#ifdef DMALLOC

void *
dmem_alloc_clear(const char *file, int line, size_t n)
{
    void *cp = dmem_alloc(file, line, n);
    memset(cp, 0, n);
    return cp;
}

#endif // DMALLOC


void
mem_free(void *cp)
{
    free(cp);
}


#ifdef DMALLOC

void
dmem_free(const char *file, int line, void *cp)
{
    dmalloc_free(file, line, cp, DMALLOC_FUNC_FREE);
}

#endif // DMALLOC


char *
mem_copy_string(const char *s, size_t len)
{
    char *cp = (char *)mem_alloc(len + 1);
    if (len)
	memcpy(cp, s, len);
    cp[len] = 0;
    return cp;
}


#ifdef DMALLOC

char *
dmem_copy_string(const char *file, int line, const char *s, size_t len)
{
    char *cp = (char *)dmem_alloc(file, line, len + 1);
    if (len)
	memcpy(cp, s, len);
    cp[len] = 0;
    return cp;
}

#endif // DMALLOC


char *
mem_copy_string(const char *s)
{
    return mem_copy_string(s, (s ? strlen(s) : 0));
}

#ifdef DMALLOC

char *
dmem_copy_string(const char *file, int line, const char *s)
{
    return dmem_copy_string(file, line, s, (s ? strlen(s) : 0));
}

#endif // DMALLOC


void *
operator new(size_t nbytes)
    THROW_BAD_ALLOC
{
    return mem_alloc(nbytes);
}


#ifdef DMALLOC

void *
operator new(size_t nbytes, const char *file, int line)
    THROW_BAD_ALLOC
{
    return dmem_alloc(file, line, nbytes);
}

#endif // DMALLOC


void *
operator new[](size_t nbytes)
    THROW_BAD_ALLOC
{
    return mem_alloc(nbytes);
}


#ifdef DMALLOC

void *
operator new[](size_t nbytes, const char *file, int line)
    THROW_BAD_ALLOC
{
    return dmem_alloc(file, line, nbytes);
}

#endif // DMALLOC


void
operator delete(void *ptr)
    throw()
{
    if (ptr)
        mem_free(ptr);
}


#ifdef DMALLOC

void
operator delete(void *ptr, const char *file, int line)
    throw()
{
    if (ptr)
        dmem_free(file, line, ptr);
}

#endif // DMALLOC


void
operator delete[](void *ptr)
    throw()
{
    if (ptr)
        mem_free(ptr);
}


#ifdef DMALLOC

void
operator delete[](void *ptr, const char *file, int line)
    throw()
{
    if (ptr)
        dmem_free(file, line, ptr);
}

#endif // DMALLOC


#if HAVE_HEADER_NEW || HAVE_NEW_H


void *
operator new(size_t nbytes, std::nothrow_t &)
    throw()
{
    return mem_alloc(nbytes);
}


#ifdef DMALLOC

void *
operator new(size_t nbytes, std::nothrow_t &, const char *file, int line)
    throw()
{
    return dmem_alloc(file, line, nbytes);
}

#endif // DMALLOC


void *
operator new[](size_t nbytes, std::nothrow_t &)
    throw()
{
    return mem_alloc(nbytes);
}


void
operator delete(void *ptr, std::nothrow_t const &)
    throw()
{
    if (ptr)
        mem_free(ptr);
}


#ifdef DMALLOC

void
operator delete(void *ptr, std::nothrow_t const &, const char *file, int line)
    throw()
{
    if (ptr)
        dmem_free(file, line, ptr);
}

#endif // DMALLOC


void
operator delete[](void *ptr, std::nothrow_t const &)
    throw()
{
    if (ptr)
        mem_free(ptr);
}


#ifdef DMALLOC

void
operator delete[](void *ptr, std::nothrow_t const &, const char *file, int line)
    throw()
{
    if (ptr)
        dmem_free(file, line, ptr);
}

#endif // DMALLOC

#endif
