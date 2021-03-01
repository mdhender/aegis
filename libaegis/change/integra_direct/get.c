/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate gets
 */

#include <change.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>


string_ty *
change_integration_directory_get(change_ty *cp, int resolve)
{
	string_ty	*result;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("change_integration_directory_get(cp = %08lX)\n{\n", (long)cp));
	assert(cp->reference_count >= 1);
	if (!cp->integration_directory_unresolved)
	{
		cstate		cstate_data;
		string_ty	*dir;

		cstate_data = change_cstate_get(cp);
		dir = cstate_data->integration_directory;
		if (!dir)
			change_fatal(cp, 0, i18n("no int dir"));
		if (dir->str_text[0] == '/')
			cp->integration_directory_unresolved = str_copy(dir);
		else
		{
			cp->integration_directory_unresolved =
				os_path_cat(project_Home_path_get(cp->pp), dir);
		}
	}
	if (!resolve)
		result = cp->integration_directory_unresolved;
	else
	{
		if (!cp->integration_directory_resolved)
		{
			change_become(cp);
			cp->integration_directory_resolved =
				os_pathname
				(
					cp->integration_directory_unresolved,
					1
				);
			change_become_undo();
		}
		result = cp->integration_directory_resolved;
	}
	trace(("result = \"%s\"\n", result->str_text));
	trace(("}\n"));
	return result;
}
