/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate version_lists
 */

#include <ac/stdlib.h>

#include <format/version_list.h>
#include <mem.h>


format_version_list_ty *
format_version_list_new()
{
	format_version_list_ty *fvlp;

	fvlp = mem_alloc(sizeof(format_version_list_ty));
	fvlp->item = 0;
	fvlp->length = 0;
	fvlp->maximum = 0;
	return fvlp;
}


void
format_version_list_delete(fvlp, delmore)
	format_version_list_ty *fvlp;
	int		delmore;
{
	size_t		j;

	if (delmore)
	{
		for (j = 0; j < fvlp->length; ++j)
			format_version_delete(fvlp->item[j]);
	}
	if (fvlp->item)
		mem_free(fvlp->item);
	fvlp->item = 0;
	fvlp->length = 0;
	fvlp->maximum = 0;
	mem_free(fvlp);
}


#ifdef DEBUG

void
format_version_list_validate(fvlp)
	format_version_list_ty *fvlp;
{
	size_t		j;

	for (j = 0; j < fvlp->length; ++j)
		format_version_validate(fvlp->item[j]);
}

#endif


void
format_version_list_append(fvlp, fvp)
	format_version_list_ty *fvlp;
	format_version_ty *fvp;
{
	if (fvlp->length >= fvlp->maximum)
	{
		size_t		nbytes;

		fvlp->maximum = fvlp->maximum * 2 + 8;
		nbytes = fvlp->maximum * sizeof(fvlp->item[0]);
		fvlp->item = mem_change_size(fvlp->item, nbytes);
	}
	fvlp->item[fvlp->length++] = fvp;
}


static int cmp _((const void *, const void *));

static int
cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	format_version_ty *a;
	format_version_ty *b;

	a = *(format_version_ty **)va;
	b = *(format_version_ty **)vb;
	if (a->when < b->when)
		return -1;
	if (a->when > b->when)
		return 1;
	return 0;
}


void
format_version_list_sort_by_date(fvlp)
	format_version_list_ty *fvlp;
{
	qsort(fvlp->item, fvlp->length, sizeof(fvlp->item[0]), cmp);
}
