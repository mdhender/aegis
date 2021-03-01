//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1999, 2002-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate dynamic memory
//

#include <ac/stddef.h>
#include <ac/string.h>
#include <ac/stdlib.h>
#include <ac/errno.h>

#include <mem.h>
#include <error.h>


#ifdef _AIX

//
// Yet another AIX stupidity:
// malloc does not guarantee that the space is available in swap.
//

#include <ac/signal.h>
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
    void            *cp;
    int             old_errno;

    if (n < 1)
	n = 1;
    old_errno = errno;
    errno = 0;
    cp = malloc(n);
    if (!cp)
    {
	if (!errno)
	    errno = ENOMEM;
	nfatal("malloc");
    }
#ifdef _AIX
    //
    // watch out for AIX stupidity
    //
    if (aix_touch(cp, n))
    {
	errno = ENOMEM;
	nfatal("malloc");
    }
#endif
    errno = old_errno;
    return cp;
}


void *
mem_alloc_clear(size_t n)
{
    void            *cp;

    cp = mem_alloc(n);
    memset(cp, 0, n);
    return cp;
}


void *
mem_change_size(void *cp, size_t n)
{
    int         old_errno;

    if (n < 1)
	n = 1;
    old_errno = errno;
    errno = 0;
    if (!cp)
	cp = malloc(n);
    else
	cp = realloc(cp, n);
    if (!cp)
    {
	if (!errno)
	    errno = ENOMEM;
	nfatal("realloc");
    }

    errno = old_errno;
    return cp;
}


void
mem_free(void *cp)
{
    free(cp);
}


char *
mem_copy_string(const char *s, size_t len)
{
    char *cp = (char *)mem_alloc(len + 1);
    if (len)
	memcpy(cp, s, len);
    cp[len] = 0;
    return cp;
}


char *
mem_copy_string(const char *s)
{
    return mem_copy_string(s, (s ? strlen(s) : 0));
}
