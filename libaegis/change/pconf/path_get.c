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
 * MANIFEST: functions to manipulate path_gets
 */

#include <change.h>
#include <change/file.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <trace.h>


string_ty *
change_pconf_path_get(change_ty *cp)
{
    static string_ty *file_name;

    trace(("change_pconf_path_get(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    if (!cp->pconf_path)
    {
	if (!file_name)
	    file_name = str_from_c(THE_CONFIG_FILE);
	cp->pconf_path = change_file_source(cp, file_name);
	if (!cp->pconf_path)
	{
	    cstate	    cstate_data;
	    string_ty	    *dir;

	    /*
	     * The file does not yet exist in either the
	     * change or the project.  Fake it.
	     */
	    cstate_data = change_cstate_get(cp);
	    switch (cstate_data->state)
	    {
	    case cstate_state_being_developed:
	    case cstate_state_awaiting_review:
	    case cstate_state_being_reviewed:
	    case cstate_state_awaiting_integration:
		dir = change_development_directory_get(cp, 0);
		if (!dir)
		{
		    /*
		     * Development directory may not
		     * be set yet, particularly if
		     * called by aeib.	Use the
		     * baseline instead.
		     */
		    dir = project_baseline_path_get(cp->pp, 0);
		}
		break;

	    case cstate_state_being_integrated:
		dir = change_integration_directory_get(cp, 0);
		break;

	    default:
		dir = project_baseline_path_get(cp->pp, 0);
		break;
	    }
	    trace(("dir = \"%s\"\n", (dir ? dir->str_text : "")));
	    cp->pconf_path = os_path_cat(dir, file_name);
	    trace(("cp->pconf_path = \"%s\"\n", cp->pconf_path->str_text));
	}
    }
    trace(("return \"%s\";\n", cp->pconf_path->str_text));
    trace(("}\n"));
    return cp->pconf_path;
}
