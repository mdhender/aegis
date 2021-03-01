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

#include <error.h> /* for assert */
#include <change/file.h>
#include <os/isa/path_prefix.h>
#include <str_list.h>
#include <trace.h>


void
change_file_directory_query(cp, file_name, result_in, result_out)
	change_ty	*cp;
	string_ty	*file_name;
	string_list_ty	*result_in;
	string_list_ty	*result_out;
{
	fstate		fstate_data;
	int		j;
	fstate_src	src_data;

	trace(("change_file_dir(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	assert(result_in);
	string_list_constructor(result_in);
	if (result_out)
		string_list_constructor(result_out);
	fstate_data = change_fstate_get(cp);
	assert(fstate_data->src);
	for (j = 0; j < fstate_data->src->length; ++j)
	{
		src_data = fstate_data->src->list[j];
		if (src_data->about_to_be_created_by && !src_data->deleted_by)
			continue;
		if (src_data->usage == file_usage_build)
			continue;
		if (os_isa_path_prefix(file_name, src_data->file_name))
		{
			if (!src_data->deleted_by)
				string_list_append(result_in, src_data->file_name);
			else if (result_out)
				string_list_append(result_out, src_data->file_name);
		}
	}
	trace((/*{*/"}\n"));
}
