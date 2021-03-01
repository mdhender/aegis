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
 * MANIFEST: functions to manipulate sets
 */

#include <arglex2.h>
#include <change.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>


void
change_integration_directory_set(cp, s)
	change_ty	*cp;
	string_ty	*s;
{
	cstate		cstate_data;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("change_integration_directory_set(cp = %8.8lX, s = \"%s\")\n{\n"
		/*}*/, cp, s->str_text));
	assert(cp->reference_count >= 1);
	if (cp->integration_directory_resolved)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_directory));
		fatal_intl(scp, i18n("duplicate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	cp->integration_directory_unresolved = str_copy(s);
	change_become(cp);
	cp->integration_directory_resolved = os_pathname(s, 1);
	change_become_undo();
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->integration_directory)
	{
		string_ty	*dir;

		dir = os_below_dir(project_Home_path_get(cp->pp), s);
		if (dir)
		{
			if (!dir->str_length)
			{
				assert(0);
				str_free(dir);
				dir = str_from_c(".");
			}
			cstate_data->integration_directory = dir;
		}
		else
			cstate_data->integration_directory = str_copy(s);
	}
	trace((/*{*/"}\n"));
}
