/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2000 Peter Miller;
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
 * MANIFEST: functions to manipulate sources
 */

#include <change/file.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <trace.h>


string_ty *
change_file_source(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	string_ty	*result;
	cstate		cstate_data;
	fstate_src	src;

	/*
	 * For changes which are not somewhere between `being developed'
	 * and `being integrated' look in the project only.
	 */
	trace(("change_file_source(cp = %08lX, file_name = \"%s\")\n{\n"/*}*/,
		(long)cp, file_name->str_text));
	cstate_data = change_cstate_get(cp);
	if
	(
		cstate_data->state == cstate_state_awaiting_development
	||
		cstate_data->state == cstate_state_completed
	)
		goto projfile;

	/*
	 * see if the file is in the change
	 */
	src = change_file_find(cp, file_name);
	if (src && !src->about_to_be_copied_by)
	{
		result = change_file_path(cp, file_name);
		assert(result);
		trace(("return \"%S\";\n", result->str_text));
		trace((/*{*/"}\n"));
		return result;
	}

	/*
	 * If the change is being integrated, and the file would be in
	 * the branch's baseline, then the path to it is in the
	 * integration directory.
	 */
	if (cstate_data->state == cstate_state_being_integrated)
	{
		change_ty	*pcp;

		pcp = project_change_get(cp->pp);
		src = change_file_find(pcp, file_name);
		if (src && !src->about_to_be_copied_by)
		{
			string_ty	*id;

			id = change_integration_directory_get(cp, 0);
			result = os_path_cat(id, file_name);
			assert(result);
			trace(("return \"%S\";\n", result->str_text));
			trace((/*{*/"}\n"));
			return result;
		}
	}

	/*
	 * see if the file is in the project
	 */
	projfile:
	if (project_file_find(cp->pp, file_name))
	{
		result = project_file_path(cp->pp, file_name);
		assert(result);
		trace(("return \"%S\";\n", result->str_text));
		trace((/*{*/"}\n"));
		return result;
	}

	/*
	 * no such file
	 */
	trace(("return NULL;\n"));
	trace((/*{*/"}\n"));
	return 0;
}
