/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2000, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate paths
 */

#include <change/file.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project/file.h>
#include <str_list.h>
#include <symtab.h>
#include <trace.h>


string_ty *
change_file_path(change_ty *cp, string_ty *file_name)
{
    cstate_ty       *cstate_data;
    fstate_src_ty   *src_data;
    int             j;
    string_ty       *result;

    trace(("change_file_path(cp = %08lX, file_name = \"%s\")\n{\n",
	(long)cp, file_name->str_text));
    change_fstate_get(cp);
    assert(cp->fstate_stp);
    result = 0;
    src_data = symtab_query(cp->fstate_stp, file_name);
    if (!src_data)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    if (src_data->about_to_be_copied_by)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    /*
     * Files which are built could be any where in the change search
     * path.  Go hunting for such files.
     */
    switch (src_data->usage)
    {
	string_list_ty	search_path;

    case file_usage_source:
    case file_usage_config:
    case file_usage_test:
    case file_usage_manual_test:
	break;

    case file_usage_build:
	change_search_path_get(cp, &search_path, 0);
	assert(search_path.nstrings >= 1);
	result = 0;
	os_become_orig();
	for (j = 0; j < search_path.nstrings; ++j)
	{
	    result = os_path_cat(search_path.string[j], file_name);
	    if (os_exists(result))
	       	break;
	    str_free(result);
	    result = 0;
	}
	os_become_undo();
	if (j >= search_path.nstrings)
	    result = os_path_cat(search_path.string[0], file_name);
	assert(result);
	string_list_destructor(&search_path);
	trace(("return \"%s\";\n", result->str_text));
	trace(("}\n"));
	return result;
    }

    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    default:
	result = project_file_path(cp->pp, file_name);
	break;

    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
	result =
    	    os_path_cat(change_development_directory_get(cp, 0), file_name);
	break;

    case cstate_state_being_integrated:
	result =
    	    os_path_cat(change_integration_directory_get(cp, 0), file_name);
	break;
    }

    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}
