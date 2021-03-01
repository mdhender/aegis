//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
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

#include <common/fstrcmp.h>
#include <libaegis/project/file.h>
#include <common/str_list.h>
#include <common/trace.h>


fstate_src_ty *
project_ty::file_find_fuzzy(string_ty *file_name, view_path_ty as_view_path)
{
    //
    // This is used to find names when project_file_find does not.
    // Deleted and almost created files are thus ignored.
    //
    trace(("project_ty::file_find_fuzzy(this = %8.8lX, fn = \"%s\")\n{\n",
	(long)this, file_name->str_text));

    //
    // get the merged list of file names
    // (do not free wlp, it's cached)
    //
    string_list_ty *wlp = file_list_get(as_view_path);

    //
    // find the closest name
    // that actually exists
    //
    fstate_src_ty *best_src = 0;
    double best_weight = 0.6;
    for (size_t j = 0; j < wlp->nstrings; ++j)
    {
	string_ty *tmp = wlp->string[j];
	double weight = fstrcmp(file_name->str_text, tmp->str_text);
	if (weight > best_weight)
	{
	    fstate_src_ty *src_data =
		project_file_find(this, tmp, as_view_path);
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
