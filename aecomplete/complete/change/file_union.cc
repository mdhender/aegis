//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <aecomplete/complete/change/file_union.h>
#include <aecomplete/complete/private.h>
#include <libaegis/dir_stack.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <aecomplete/shell.h>
#include <common/str_list.h>


struct complete_change_file_union_ty
{
    complete_ty     inherited;
    change::pointer cp;
    int             baserel;
    int             regular_ok;
    int             special_ok;
    int             source_ok;
};


static void
destructor(complete_ty *cp)
{
    complete_change_file_union_ty *this_thing;

    this_thing = (complete_change_file_union_ty *)cp;
    change_free(this_thing->cp);
}


static void
perform(complete_ty *cp, shell_ty *sh)
{
    complete_change_file_union_ty *this_thing;
    string_ty       *prefix;
    string_ty       *prefix_dir;
    string_ty       *prefix_ent;
    size_t          j;
    string_list_ty  search_path;
    string_list_ty  names;
    string_ty       *base = 0;

    //
    // Get the change search path.
    //
    this_thing = (complete_change_file_union_ty *)cp;
    change_search_path_get(this_thing->cp, &search_path, 1);

    if (this_thing->baserel)
	base = str_from_c("");
    else
    {
	string_ty       *cwd;
	string_ty       *tmp;

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
		    base = str_format("%s/", tmp->str_text);
		    str_free(tmp);
	       	}
	       	else
		    base = tmp;
	       	break;
	    }
	}
	if (j >= search_path.nstrings)
	{
	    //
	    // They are in a weird place, assume base relative.
	    //
	    base = str_from_c("");
	}
	str_free(cwd);
    }

    //
    // Break the prefix into directory part and filename part.
    //
    prefix = str_catenate(base, shell_prefix_get(sh));
    prefix_dir = os_dirname_relative(prefix);
    prefix_ent = os_entryname_relative(prefix);

    //
    // Read the directory (the stack union) for the file names.
    //
    os_become_orig();
    dir_stack_readdir(&search_path, prefix_dir, &names);
    os_become_undo();

    //
    // Troll the names.
    //
    for (j = 0; j < names.nstrings; ++j)
    {
	string_ty       *name;
	struct stat     st;
	string_ty       *path;
	string_ty       *relfn;
	string_ty       *s;

	//
	// Make sure we like the name.
	//
	name = names.string[j];
	if (!str_leading_prefix(name, prefix_ent))
	    continue;

	//
	// Ignore project source files
	// and change source files.
	//
	path = os_path_cat(prefix_dir, name);
	if (!this_thing->source_ok)
	{
	    fstate_src_ty   *src;

	    src =
                project_file_find(this_thing->cp->pp, path, view_path_extreme);
	    if (src)
	    {
		str_free(path);
		continue;
	    }
	    src = change_file_find(this_thing->cp, path, view_path_first);
	    if (src)
	    {
		str_free(path);
		continue;
	    }
	}

	//
	// Find out what it is.
	// (Ignore if can't stat.)
	//
	s = dir_stack_find(&search_path, 0, path, &st, 0, 1);
	if (!s)
	    continue;
	str_free(s);

	//
	// Remove the base portion before emitting.
	//
	relfn =
	    str_n_from_c
	    (
		path->str_text + base->str_length,
		path->str_length - base->str_length
	    );

	//
	// Be selective about what we print.
	//
	if (S_ISDIR(st.st_mode))
	{
	    //
	    // We emit directories with and without a slash, so that Bash
	    // doesn't think we are finished, and put a space after the
	    // argument if there is only one directory in the answer.
	    //
	    shell_emit(sh, relfn);
	    s = str_format("%s/", relfn->str_text);
	    shell_emit(sh, s);
	    str_free(s);
	}
	else if (S_ISREG(st.st_mode))
	{
	    if (this_thing->regular_ok)
		shell_emit(sh, relfn);
	}
	else
	{
	    if (this_thing->special_ok)
		shell_emit(sh, relfn);
	}
	str_free(relfn);
	str_free(path);
    }
    str_free(prefix_ent);
    str_free(prefix_dir);
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_change_file_union_ty),
    "change file union",
};


complete_ty *
complete_change_file_union(change::pointer cp, int baserel, int dir_only)
{
    complete_ty     *result;
    complete_change_file_union_ty *this_thing;

    result = complete_new(&vtbl);
    this_thing = (complete_change_file_union_ty *)result;
    this_thing->cp = cp;
    this_thing->baserel = !!baserel;
    this_thing->regular_ok = !dir_only;
    this_thing->special_ok = 0;
    this_thing->source_ok = 0;
    return result;
}
