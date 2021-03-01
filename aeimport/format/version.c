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
 * MANIFEST: functions to manipulate versions
 */

#include <error.h> /* for assert */
#include <format/version.h>
#include <format/version_list.h>
#include <mem.h>


format_version_ty *
format_version_new()
{
	format_version_ty *fvp;

	fvp = mem_alloc(sizeof(format_version_ty));
	fvp->filename_physical = 0;
	fvp->filename_logical = 0;
	fvp->edit = 0;
	fvp->when = 0;
	fvp->who = 0;
	fvp->description = 0;
	fvp->before = 0;
	string_list_constructor(&fvp->tag);
	fvp->after = 0;
	fvp->after_branch = 0;
	fvp->dead = 0;
	return fvp;
}


void
format_version_delete(fvp)
	format_version_ty *fvp;
{
	if (fvp->filename_physical)
	{
		str_free(fvp->filename_physical);
		fvp->filename_physical = 0;
	}
	if (fvp->filename_logical)
	{
		str_free(fvp->filename_logical);
		fvp->filename_logical = 0;
	}
	if (fvp->edit)
	{
		str_free(fvp->edit);
		fvp->edit = 0;
	}
	fvp->when = 0;
	if (fvp->who)
	{
		str_free(fvp->who);
		fvp->who = 0;
	}
	if (fvp->description)
	{
		str_free(fvp->description);
		fvp->description = 0;
	}
	string_list_destructor(&fvp->tag);
	fvp->before = 0;
	if (fvp->after)
	{
		format_version_delete(fvp->after);
		fvp->after = 0;
	}
	if (fvp->after_branch)
	{
		format_version_list_delete(fvp->after_branch, 1);
		fvp->after_branch = 0;
	}
	fvp->dead = 0;
	mem_free(fvp);
}


#ifdef DEBUG

void
format_version_validate(fvp)
	format_version_ty *fvp;
{
	assert(fvp);
	if (fvp->filename_physical)
		assert(str_validate(fvp->filename_physical));
	if (fvp->filename_logical)
		assert(str_validate(fvp->filename_logical));
	if (fvp->edit)
		assert(str_validate(fvp->edit));
	if (fvp->who)
		assert(str_validate(fvp->who));
	if (fvp->description)
		assert(str_validate(fvp->description));
	assert(string_list_validate(&fvp->tag));
	if (fvp->after)
		format_version_validate(fvp->after);
	if (fvp->after_branch)
		format_version_list_validate(fvp->after_branch);
}

#endif
