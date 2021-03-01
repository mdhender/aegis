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
 * MANIFEST: functions to manipulate find_fuzzys
 */

#include <fstrcmp.h>
#include <project/file.h>
#include <project/file/list_get.h>
#include <str_list.h>
#include <trace.h>


fstate_src_ty *
project_file_find_fuzzy(project_ty *pp, string_ty *file_name,
    view_path_ty as_view_path)
{
    fstate_src_ty   *src_data;
    string_list_ty  *wlp;
    size_t          j;
    fstate_src_ty   *best_src;
    double          best_weight;

    /*
     * This is used to find names when project_file_find does not.
     * Deleted and almost created files are thus ignored.
     */
    trace(("project_file_find_fuzzy(pp = %8.8lX, fn = \"%s\")\n{\n",
	(long)pp, file_name->str_text));

    /*
     * get the merged list of file names
     * (do not free wlp, it's cached)
     */
    wlp = project_file_list_get(pp, as_view_path);

    /*
     * find the closest name
     * that actually exists
     */
    best_src = 0;
    best_weight = 0.6;
    for (j = 0; j < wlp->nstrings; ++j)
    {
	string_ty	*name;
	double		weight;

	name = wlp->string[j];
	weight = fstrcmp(name->str_text, file_name->str_text);
	if (weight > best_weight)
	{
	    src_data = project_file_find(pp, name, as_view_path);
	    if (src_data)
	    {
	       	best_src = src_data;
	       	best_weight = weight;
	    }
	}
    }

    trace(("return %8.8lX;\n", (long)best_src));
    trace(("}\n"));
    return best_src;
}
