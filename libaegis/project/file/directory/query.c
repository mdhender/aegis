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
 * MANIFEST: functions to manipulate querys
 */

#include <change/file.h>
#include <error.h> /* for assert */
#include <project/file.h>
#include <str_list.h>
#include <trace.h>


void
project_file_directory_query(pp, file_name, result_in, result_out)
	project_ty	*pp;
	string_ty	*file_name;
	string_list_ty	*result_in;
	string_list_ty	*result_out;
{
	project_ty	*ppp;
	change_ty	*cp;
	string_list_ty	wl_in;
	string_list_ty	wl_out;
	string_list_ty	exclude;

	trace(("project_file_dir(pp = %8.8lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)pp, file_name->str_text));
	assert(result_in);
	string_list_constructor(result_in);
	string_list_constructor(&exclude);
	if (result_out)
		string_list_constructor(result_out);
	else
		result_out = &exclude;
	for (ppp = pp; ppp; ppp = ppp->parent)
	{
		cp = project_change_get(ppp);
		change_file_directory_query(cp, file_name, &wl_in, &wl_out);
		string_list_remove_list(&wl_in, result_out);
		string_list_append_list_unique(result_in, &wl_in);
		string_list_append_list_unique(result_out, &wl_out);
		string_list_destructor(&wl_in);
		string_list_destructor(&wl_out);
	}
	string_list_destructor(&exclude);
	trace((/*{*/"}\n"));
}
