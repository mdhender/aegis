/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate files
 */

#include <change.h>
#include <change/file.h>
#include <complete/change/file.h>
#include <complete/private.h>
#include <os.h>
#include <shell.h>
#include <str_list.h>


typedef struct complete_change_file_ty complete_change_file_ty;
struct complete_change_file_ty
{
    complete_ty     inherited;
    change_ty       *cp;
    int             baserel;
    int             usage_mask;
    int             action_mask;
};


static void
destructor(complete_ty *cp)
{
    complete_change_file_ty *this_thing;

    this_thing = (complete_change_file_ty *)cp;
    change_free(this_thing->cp);
}


static void
perform(complete_ty *cop, shell_ty *sh)
{
    complete_change_file_ty *this_thing;
    string_ty       *base = 0;
    string_list_ty  candidate;
    string_ty       *prefix;
    size_t          j;

    /*
     * We need to figure the base of the file names, in case the user
     * is completing their project file name from within a directory,
     * and we must give answers strictly in that context.
     */
    this_thing = (complete_change_file_ty *)cop;
    if (this_thing->baserel)
	base = str_from_c("");
    else
    {
	string_list_ty  search_path;
	string_ty       *cwd;
	string_ty       *tmp;

	change_search_path_get(this_thing->cp, &search_path, 1);

	os_become_orig();
	cwd = os_curdir();
	os_become_undo();

	for (j = 0; j < search_path.nstrings; ++j)
	{
	    tmp = os_below_dir(search_path.string[j], cwd);
	    if (tmp)
	    {
		if (tmp->str_length)
		{
		    base = str_format("%S/", tmp);
		    str_free(tmp);
		}
		else
		    base = tmp;
		break;
	    }
	}
	if (j >= search_path.nstrings)
	{
	    /*
	     * They are in a weird place, assume base relative.
	     */
	    base = str_from_c("");
	}
	str_free(cwd);
    }

    /*
     * Troll the complete list of change files.
     */
    string_list_constructor(&candidate);
    prefix = str_catenate(base, shell_prefix_get(sh));
    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src;
	string_ty       *relfn;

	src = change_file_nth(this_thing->cp, j);
	if (!src)
	    break;

	/*
	 * Ignore files that aren't appropriate.
	 */
	if (!(this_thing->action_mask & (1 << src->action)))
	    continue;
	if (!(this_thing->usage_mask & (1 << src->usage)))
	    continue;

	/*
	 * Ignore files that don't match the prefix.
	 */
	if (!str_leading_prefix(src->file_name, prefix))
	    continue;

	/*
	 * We have a completion candidate.
	 * Remove the base portion before emitting
	 */
	relfn =
	    str_n_from_c
	    (
	       	src->file_name->str_text + base->str_length,
	       	src->file_name->str_length - base->str_length
	    );
        string_list_append(&candidate, relfn);
	str_free(relfn);
    }

    /*
     * Emit the list of files.
     */
    shell_emit_file_list(sh, &candidate);
    string_list_destructor(&candidate);
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_change_file_ty),
    "change file",
};


complete_ty *
complete_change_file(change_ty *cp, int baserel, int usage_mask,
    int action_mask)
{
    complete_ty     *result;
    complete_change_file_ty *this_thing;

    result = complete_new(&vtbl);
    this_thing = (complete_change_file_ty *)result;
    this_thing->cp = cp;
    this_thing->baserel = !!baserel;
    this_thing->usage_mask = usage_mask ? usage_mask : ~0;
    this_thing->action_mask = action_mask ? action_mask : ~0;
    return result;
}
