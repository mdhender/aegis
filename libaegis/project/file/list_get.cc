//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/str_list.h>
#include <common/symtab_iter.h>
#include <common/trace.h>
#include <libaegis/change/file.h>
#include <libaegis/project/file.h>


string_list_ty *
project_ty::file_list_get(view_path_ty as_view_path)
{
    trace(("project_ty::file_list_get(this = %8.8lX, as_view_path = %s)\n{\n",
	(long)this, view_path_ename(as_view_path)));
#ifdef DEBUG
    switch (as_view_path)
    {
    case view_path_none:
    case view_path_simple:
    case view_path_extreme:
	break;

    default:
	assert(0);
	as_view_path = view_path_simple;
	break;
    }
#endif
    if (!file_list[as_view_path])
    {
	project_ty	*ppp;
	change::pointer cp;
	long		j;
	fstate_src_ty   *fsp;
	string_list_ty	*wlp;
	symtab_iterator	*tmpi;
	string_ty	*key;
	void		*data;

	//
	// Drop all the files into a symbol table.
	// This has O(1) insertion times.
	//
	// We are using the symbol table to mimic the actions of
	// project_file_find, but for every project file in parallel.
	// It is essential that this function does exactly what the
	// project_file_find function does.
	//
	symtab_ty *tmp = new symtab_ty(100);
	trace(("tmp = %08lX\n", (long)tmp));
	assert(tmp->valid());
	for (ppp = this; ppp; ppp = (ppp->is_a_trunk() ? 0 : ppp->parent_get()))
	{
	    trace(("project \"%s\"\n", project_name_get(ppp)->str_text));
	    cp = ppp->change_get();
	    trace(("mark\n"));
	    for (j = 0; ; ++j)
	    {
		trace(("mark\n"));
		fsp = change_file_nth(cp, j, view_path_first);
		if (!fsp)
	    	    break;
		trace(("%s \"%s\"\n", file_action_ename(fsp->action),
		    fsp->file_name->str_text));

		//
		// If we already have a "hit" for this file, ignore any
		// deeper records.  This mimics the way project_file_find
		// stops as soon as it finds a match.
		//
		// This has O(1) query times.
		//
		trace(("tmp = %08lX\n", (long)tmp));
		if (tmp->query(fsp->file_name))
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
			    continue;
			// should be file_action_transparent
			assert(!fsp->about_to_be_copied_by);
			if (fsp->about_to_be_copied_by)
			    continue;
			break;
		    }
		    break;
		}
		trace(("tmp = %08lX\n", (long)tmp));
		tmp->assign(fsp->file_name, fsp);
	    }
	    if (as_view_path == view_path_first)
	    {
		//
		// This means that transparent and removed files
		// are returned.  No project file union, either.
		//
		break;
	    }
	}

	//
	// Walk the symbol table to build the file name list.
	// This has O(n) running time.
	//
	trace(("tmp = %08lX\n", (long)tmp));
	assert(tmp->valid());
	tmpi = symtab_iterator_new(tmp);
	wlp = new string_list_ty();
	while (symtab_iterator_next(tmpi, &key, &data))
	{
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
		trace(("tmp = %08lX\n", (long)tmp));
		fsp = (fstate_src_ty *)tmp->query(key);
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
	trace(("tmp = %08lX\n", (long)tmp));
	assert(tmp->valid());
	delete tmp;

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
	file_list[as_view_path] = wlp;
    }
    trace(("return %8.8lX;\n", (long)file_list[as_view_path]));
    trace(("}\n"));
    return file_list[as_view_path];
}
