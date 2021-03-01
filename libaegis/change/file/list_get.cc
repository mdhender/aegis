//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate list_gets
//

#include <change/branch.h>
#include <change/file/list_get.h>
#include <error.h> // for assert
#include <str_list.h>
#include <project.h>
#include <symtab.h>
#include <symtab_iter.h>
#include <trace.h>


string_list_ty *
change_file_list_get(change_ty *cp, view_path_ty as_view_path)
{
    trace(("change_file_list_get(cp = %8.8lX, as_view_path = %s)\n{\n",
	(long)cp, view_path_ename(as_view_path)));
    if (!cp->file_list[as_view_path])
    {
	size_t          j;
	string_list_ty	*wlp;
	symtab_ty	*tmp;
	symtab_ty	*xpar;
	symtab_iterator	*tmpi;
	string_ty	*key;
	void		*data;
	int             top_level;
	change_ty       *cp2;

	//
	// Drop all the files into a symbol table.
	// This has O(1) insertion times.
	//
	// We are using the symbol table to mimic the actions of
	// change_file_find, but for every change and project file in
	// parallel.  It is essential that this function does exactly
	// what the change_file_find function does.
	//
	tmp = symtab_alloc(100);
	xpar = symtab_alloc(100);
	top_level = !change_is_a_branch(cp);
	cp2 = cp;
	for (;;)
	{
	    fstate_ty       *fs;

	    trace(("project \"%s\": change %ld\n",
		project_name_get(cp2->pp)->str_text, cp2->number));
	    if (cp2->bogus)
		goto next;
	    fs = change_fstate_get(cp2);
	    trace(("fs->src->length = %ld\n", (long)fs->src->length));
	    for (j = 0; j < fs->src->length; ++j)
	    {
		fstate_src_ty   *fsp;

		fsp = fs->src->list[j];
		assert(fsp);
		trace(("%ld: %s \"%s\"\n", (long)j,
		    file_action_ename(fsp->action), fsp->file_name->str_text));

		//
		// If there is a top-level transparent file,
		// it hides the next instance of the file.
		//
		if (symtab_query(xpar, fsp->file_name))
		{
		    symtab_delete(tmp, fsp->file_name);
		    symtab_delete(xpar, fsp->file_name);
		    // go onto the next file
		    continue;
		}

		//
		// If we already have a "hit" for this file, ignore any
		// deeper records.  This mimics the way change_file_find
		// stops as soon as it finds a match.
		//
		// This has O(1) query times.
		//
		if (symtab_query(tmp, fsp->file_name))
		    continue;

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
			    symtab_assign(xpar, fsp->file_name, fsp);
			// go onto the next file
			continue;

		    case file_action_remove:
			//
			// For now, we keep removed files in the symbol table.
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
			{
			    // go onto the next file
			    continue;
			}
			// should be file_action_transparent
			assert(!fsp->about_to_be_copied_by);
			if (fsp->about_to_be_copied_by)
			{
			    // go onto the next file
			    continue;
			}
			break;
		    }
		    break;
		}
		symtab_assign(tmp, fsp->file_name, fsp);
	    }

	    next:
	    if (as_view_path == view_path_first)
		break;
	    if (cp2->number == TRUNK_CHANGE_NUMBER)
		break;
	    cp2 = project_change_get(cp2->pp);
	    top_level = 0;
	}

	//
	// Walk the symbol table to build the file name list.
	// This has O(1) query times.
	//
	tmpi = symtab_iterator_new(tmp);
	wlp = new string_list_ty();
	while (symtab_iterator_next(tmpi, &key, &data))
	{
	    fstate_src_ty   *fsp;

	    switch (as_view_path)
	    {
	    case view_path_first:
	    case view_path_none:
	    case view_path_simple:
		break;

	    case view_path_extreme:
		//
		// This means that transparent files are resolved (the
		// underlying file is shown), but removed files are
		// omitted from the result.
		//
		fsp = (fstate_src_ty *)symtab_query(tmp, key);
		assert(fsp);
		if (!fsp)
		    break;
		switch (fsp->action)
		{
		case file_action_remove:
		    continue;

		case file_action_create:
		case file_action_modify:
		case file_action_insulate:
		case file_action_transparent:
#ifndef DEBUG
		default:
#endif
		    // should be file_action_remove
		    assert(!fsp->deleted_by);
		    if (fsp->deleted_by)
			continue;
		    break;
		}
		break;
	    }
	    wlp->push_back(key);
	}
	symtab_iterator_delete(tmpi);
	symtab_free(tmp);
	symtab_free(xpar);

	//
	// Ensure that the file name list is in lexicographical
	// order, otherwise the users can see the joins (and it's
	// harder to find the files in a listing).
	// (C locale)
	//
	// Overall performance is O(n) for file discovery,
	// plus O(n log n) for the qsort, where n is the number
	// of files.
	//
	wlp->sort();
	cp->file_list[as_view_path] = wlp;
    }
    trace(("return %8.8lX;\n", (long)cp->file_list[as_view_path]));
    trace(("}\n"));
    return cp->file_list[as_view_path];
}
