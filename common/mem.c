/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate dynamic memory
 */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <main.h>
#include <mem.h>
#include <error.h>


/*
 *  NAME
 *	mem_alloc - allocate memory
 *
 *  SYNOPSIS
 *	char *mem_alloc(size_t n);
 *
 *  DESCRIPTION
 *	mem_alloc uses malloc to allocate the required sized chunk of memory.
 *	If any error is returned from malloc() an fatal diagnostic is issued.
 *
 *  CAVEAT
 *	It is the responsibility of the caller to ensure that the space is
 *	freed when finished with, by a call to free().
 */

char *
mem_alloc(n)
	size_t		n;
{
	char		*cp;

	if (n < 1)
		n = 1;
	errno = 0;
	cp = malloc(n);
	if (!cp)
	{
		if (!errno)
			errno = ENOMEM;
		nfatal("malloc");
	}
	return cp;
}


/*
 *  NAME
 *	mem_alloc_clear - allocate and clear memory
 *
 *  SYNOPSIS
 *	char *mem_alloc_clear(size_t n);
 *
 *  DESCRIPTION
 *	mem_alloc_clear uses malloc to allocate the required sized chunk of memory.
 *	If any error is returned from malloc() an fatal diagnostic is issued.
 *	The memory is zeroed befor it is returned.
 *
 *  CAVEAT
 *	It is the responsibility of the caller to ensure that the space is
 *	freed when finished with, by a call to free().
 */

char *
mem_alloc_clear(n)
	size_t		n;
{
	char		*cp;

	cp = mem_alloc(n);
	memset(cp, 0, n);
	return cp;
}


/*
 *  NAME
 *	enlarge - enlarges dynamic arrays
 *
 *  SYNOPSIS
 *	char *enlarge(size_t *length, char **base, size_t size);
 *
 *  DESCRIPTION
 *	Enlarge is used to append more space onto the end of dynamically
 *	allocated arrays.
 *	If any error is returned from the memory allocator,
 *	a fatal diagnostic is issued.
 *
 *  RETURNS
 *	A pointer to the element added.
 *
 *  CAVEAT
 *	Because it uses realloc, pointers into the array may be invalid after
 *	a call to enlarge(); only use indexing.
 *
 *	The new space is not zeroed.
 *
 *	It is the responsibility of the caller to ensure that the array is
 *	freed when finished with, by a call to free().
 */

char *
enlarge(length, base, size)
	size_t		*length;
	char		**base;
	size_t		size;
{
	char		*result;

	if (*length)
		mem_change_size(base, (*length + 1) * size);
	else
		*base = mem_alloc(size);
	result = *base + (*length)++ * size;
	return result;
}


void
mem_change_size(cpp, n)
	char		**cpp;
	size_t		n;
{
	char		*cp;

	cp = *cpp;
	if (n < 1)
		n = 1;
	errno = 0;
	cp = realloc(cp, n);
	if (!cp)
	{
		if (!errno)
			errno = ENOMEM;
		nfatal("realloc");
	}
	*cpp = cp;
}


void
mem_free(cp)
	char		*cp;
{
	free(cp);
}


char *
mem_copy_string(s)
	char		*s;
{
	char		*cp;

	cp = mem_alloc(strlen(s) + 1);
	strcpy(cp, s);
	return cp;
}
