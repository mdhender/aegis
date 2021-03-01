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
 * MANIFEST: functions to manipulate finds
 */

#include <change/file.h>
#include <project/file.h>
#include <trace.h>


fstate_src
project_file_find(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	project_ty	*ppp;
	change_ty	*cp;
	fstate_src	src_data;

	trace(("project_file_find(pp = %8.8lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)pp, file_name->str_text));
	for (ppp = pp; ppp; ppp = ppp->parent)
	{
		cp = project_change_get(ppp);
		src_data = change_file_find(cp, file_name);
		if (src_data)
		{
			trace(("return %8.8lX;\n", (long)src_data));
			trace((/*{*/"}\n"));
			return src_data;
		}
	}
	trace(("return NULL;\n"));
	trace((/*{*/"}\n"));
	return 0;
}
