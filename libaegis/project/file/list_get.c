/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate list_gets
 */

#include <change/file.h>
#include <error.h> /* for assert */
#include <project/file.h>
#include <project/file/list_get.h>
#include <str_list.h>
#include <symtab_iter.h>
#include <trace.h>


string_list_ty *
project_file_list_get(project_ty *pp, view_path_ty as_view_path)
{
    trace(("project_file_list_get(pp = %8.8lX)\n{\n"/*}*/, (long)pp));
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
    if (!pp->file_list[as_view_path])
    {
	project_ty	*ppp;
	change_ty	*cp;
	long		j;
	fstate_src	fsp;
	string_list_ty	*wlp;
	symtab_ty	*tmp;
	symtab_iterator	*tmpi;
	string_ty	*key;
	void		*data;

	/*
	 * Drop all the files into a symbol table.
	 * This has O(1) insertion times.
	 *
	 * We are using the symbol table to mimic the actions of
	 * project_file_find, but for every project file in parallel.
	 * It is essential that this function does exactly what the
	 * project_file_find function does.
	 */
	tmp = symtab_alloc(100);
	for (ppp = pp; ppp; ppp = ppp->parent)
	{
	    cp = project_change_get(ppp);
	    for (j = 0; ; ++j)
	    {
		fsp = change_file_nth(cp, j);
		if (!fsp)
	    	    break;

		/*
		 * If we already have a "hit" for this file, ignore any
		 * deeper records.  This mimics the way project_file_find
		 * stops as soon as it finds a match.
		 *
		 * This has O(1) query times.
		 */
		if (symtab_query(tmp, fsp->file_name))
		    continue;

		switch (as_view_path)
		{
		case view_path_none:
		    /*
		     * This means that transparent files are returned
		     * as transparent, no processing or filtering of
		     * the list is performed.
		     */
		    break;

		case view_path_simple:
		case view_path_extreme:
		    /*
		     * These cases both mean that transparent files
		     * are resolved (the underlying file is shown).
		     */
		    if (fsp->action == file_action_transparent)
			continue;

		    /*
		     * For now, we keep removed files in the symbol table.
		     * (Takes precedence over the next two.)
		     */
		    if (fsp->deleted_by || fsp->action == file_action_remove)
			break;

		    /*
		     * These two are transparent, for locking purposes.
		     */
		    if (fsp->about_to_be_created_by)
			continue;
		    if (fsp->about_to_be_copied_by)
			continue;
		    break;
		}
		symtab_assign(tmp, fsp->file_name, fsp);
	    }
	}

	/*
	 * Walk the symbol table to build the file name list.
	 * This has O(1) query times.
	 */
	tmpi = symtab_iterator_new(tmp);
	wlp = string_list_new();
	while (symtab_iterator_next(tmpi, &key, &data))
	{
	    switch (as_view_path)
	    {
	    case view_path_none:
	    case view_path_simple:
		break;

	    case view_path_extreme:
		/*
		 * This means that transparent files are resolved (the
		 * underlying file is shown), but removed files are
		 * omitted from the result.
		 */
		fsp = symtab_query(tmp, key);
		assert(fsp);
		if
		(
		    fsp
		&&
		    (fsp->deleted_by || fsp->action == file_action_remove)
		)
		    continue;
		break;
	    }
	    string_list_append(wlp, key);
	}
	symtab_iterator_delete(tmpi);
	symtab_free(tmp);

	/*
	 * Ensure that the file name list is in lexicographical
	 * order, otherwise the users can see the joins (and it's
	 * harder to find the files in a listing).
	 * (C locale)
	 *
	 * Overall performance is O(n) for file discovery,
	 * plus O(n log n) for the qsort, where n is the number
	 * of files.
	 */
	string_list_sort(wlp);
	pp->file_list[as_view_path] = wlp;
    }
    trace(("return %8.8lX;\n", (long)pp->file_list[as_view_path]));
    trace((/*{*/"}\n"));
    return pp->file_list[as_view_path];
}
