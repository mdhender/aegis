/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate nths
 */

#include <project/file.h>
#include <project/file/list_get.h>
#include <error.h> /* for assert */
#include <str_list.h>
#include <trace.h>


fstate_src
project_file_nth(pp, n)
	project_ty	*pp;
	size_t		n;
{
	string_list_ty	*wlp;
	fstate_src	src_data;

	trace(("project_file_nth(pp = %8.8lX, n = %ld)\n{\n"/*}*/,
		(long)pp, (long)n));
	wlp = project_file_list_get(pp);
	if (n < wlp->nstrings)
	{
		src_data = project_file_find(pp, wlp->string[n]);
		assert(src_data);
	}
	else
		src_data = 0;
	trace(("return %8.8lX;\n", (long)src_data));
	trace((/*{*/"}\n"));
	return src_data;
}
