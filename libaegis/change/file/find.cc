//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate finds
//

#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <common/error.h> // for assert
#include <common/nstring.h>
#include <libaegis/project.h>
#include <common/symtab.h>
#include <common/trace.h>


fstate_src_ty *
change_file_find(change_ty *cp, const nstring &file_name,
    view_path_ty as_view_path)
{
    fstate_src_ty   *result;
    int             xpar;
    int             top_level;

    trace(("change_file_find(cp = %8.8lX, file_name = \"%s\", "
	"as_view_path = %s)\n{\n", (long)cp, file_name.c_str(),
	view_path_ename(as_view_path)));
    result = 0;
    xpar = 0;
    top_level = !change_is_a_branch(cp);
    for (;;)
    {
	fstate_src_ty   *fsp;

	trace(("project \"%s\": change %ld\n",
	    project_name_get(cp->pp)->str_text, cp->number));
	if (cp->bogus)
	    goto next;
	change_fstate_get(cp);
	assert(cp->fstate_stp);
       	fsp =
	    (fstate_src_ty *)symtab_query(cp->fstate_stp, file_name.get_ref());
	if (fsp)
	{
	    trace(("%s \"%s\" %s\n", file_action_ename(fsp->action),
		fsp->file_name->str_text,
		(fsp->edit && fsp->edit->revision ?
		fsp->edit->revision->str_text : "")));

	    //
	    // If there is a top-level transparent file,
	    // it hides the next instance of the file.
	    //
	    if (xpar)
	    {
		result = 0;
		xpar = 0;
		goto next;
	    }

	    //
	    // If we already have a "hit" for this file, ignore any
	    // deeper records.
	    //
	    if (result)
		break;

	    switch (as_view_path)
	    {
	    case view_path_first:
		//
		// This means that transparent and removed files
		// are returned.  No project file union, either.
		//
		break;

	    case view_path_none:
		//
		// This means that transparent files are returned
		// as transparent, no processing or filtering of
		// the list is performed.
		//
		break;

	    case view_path_simple:
	    case view_path_extreme:
		switch (fsp->action)
		{
		case file_action_transparent:
		    //
		    // These cases both mean that transparent files
		    // are resolved (the underlying file is shown).
		    //
		    if (top_level)
			xpar = 1;
		    goto next;

		case file_action_remove:
		    //
		    // For now, we keep removed files.
		    //
		    break;

		case file_action_create:
		case file_action_modify:
		case file_action_insulate:
#ifndef DEBUG
		default:
#endif
		    // should be file_action_remove
		    assert(!fsp->deleted_by);
		    if (fsp->deleted_by)
			break;
		    // should be file_action_transparent
		    assert(!fsp->about_to_be_created_by);
		    if (fsp->about_to_be_created_by)
			goto next;
		    // should be file_action_transparent
		    assert(!fsp->about_to_be_copied_by);
		    if (fsp->about_to_be_copied_by)
			goto next;
		    break;
		}
		break;
	    }
	    result = fsp;
	}

	next:
	if (as_view_path == view_path_first)
	    break;
	if (cp->number == TRUNK_CHANGE_NUMBER)
	    break;
	cp = cp->pp->change_get();
	top_level = 0;
    }

    if
    (
	result
    &&
	as_view_path == view_path_extreme
    &&
	result->action == file_action_remove
    )
	result = 0;

    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


fstate_src_ty *
change_file_find(change_ty *cp, string_ty *file_name, view_path_ty as_view_path)
{
    return change_file_find(cp, nstring(str_copy(file_name)), as_view_path);
}
