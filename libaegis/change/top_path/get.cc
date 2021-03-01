//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <common/trace.h>


string_ty *
change_top_path_get(change::pointer cp, int resolve)
{
	string_ty	*result;

	//
	// To cope with automounters, directories are stored as given,
	// or are derived from the home directory in the passwd file.
	// Within aegis, pathnames have their symbolic links resolved,
	// and any comparison of paths is done on this "system idea"
	// of the pathname.
	//
	trace(("change_top_path_get(cp = %08lX)\n{\n", (long)cp));
	assert(cp->reference_count >= 1);
	if (!cp->top_path_unresolved)
	{
		cstate_ty       *cstate_data;
		string_ty	*dir;

		cstate_data = cp->cstate_get();
		dir = cstate_data->development_directory;
		if (!dir)
			change_fatal(cp, 0, i18n("no dev dir"));
		if (dir->str_text[0] == '/')
			cp->top_path_unresolved = str_copy(dir);
		else
		{
			cp->top_path_unresolved =
				os_path_cat(project_Home_path_get(cp->pp), dir);
		}
	}
	if (!resolve)
		result = cp->top_path_unresolved;
	else
	{
		if (!cp->top_path_resolved)
		{
			change_become(cp);
			cp->top_path_resolved =
				os_pathname
				(
					cp->top_path_unresolved,
					1
				);
			change_become_undo(cp);
		}
		result = cp->top_path_resolved;
	}
	trace(("result = \"%s\"\n", result->str_text));
	trace(("}\n"));
	return result;
}
