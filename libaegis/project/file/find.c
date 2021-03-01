/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
project_file_find(project_ty *pp, string_ty *file_name,
    view_path_ty as_view_path)
{
    project_ty      *ppp;
    change_ty       *cp;
    fstate_src      src_data;

    trace(("project_file_find(pp = %8.8lX, file_name = \"%s\")\n{\n",
	(long)pp, file_name->str_text));
    for (ppp = pp; ppp; ppp = ppp->parent)
    {
	cp = project_change_get(ppp);
	src_data = change_file_find(cp, file_name);
	if (src_data)
	{
	    trace(("%s \"%s\"\n", file_action_ename(src_data->action),
		src_data->file_name->str_text));
	    switch (as_view_path)
	    {
	    case view_path_none:
		/*
		 * This means that transparent files are returned as
		 * transparent, no processing or filtering of the list
		 * is performed.
		 */
		break;

	    case view_path_extreme:
		/*
		 * This means that transparent files are resolved (the
		 * underlying file is shown), and removed files are
		 * omitted from the result.
		 */
		if
		(
		    src_data->deleted_by
		||
		    src_data->action == file_action_remove
		)
		{
		    trace(("return NULL;\n"));
		    trace(("}\n"));
		    return 0;
		}
		/* fall through... */

	    case view_path_simple:
		/*
		 * This means that transparent files are resolved
		 * (the underlying file is shown), but removed files
		 * are retained.
		 */
		if (src_data->action == file_action_transparent)
		    continue;

		/*
		 * Remember if the file has been removed.
		 * (Takes precedence over the next two.)
		 */
		if
		(
		    src_data->deleted_by
		||
		    src_data->action == file_action_remove
		)
		    break;

		/*
		 * These two are transparent, for locking purposes.
		 */
		if (src_data->about_to_be_copied_by)
		    continue;
		if (src_data->about_to_be_created_by)
		    continue;
		break;
	    }
    	    trace(("return %8.8lX;\n", (long)src_data));
    	    trace(("}\n"));
    	    return src_data;
	}
    }
    trace(("return NULL;\n"));
    trace(("}\n"));
    return 0;
}
