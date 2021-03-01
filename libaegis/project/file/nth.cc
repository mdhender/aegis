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

#include <libaegis/project/file.h>
#include <common/error.h> // for assert
#include <common/str_list.h>
#include <common/trace.h>


fstate_src_ty *
project_ty::file_nth(size_t n, view_path_ty as_view_path)
{
    trace(("project_ty::file_nth(this = %8.8lX, n = %ld)\n{\n", (long)this,
	(long)n));
    // do not delete wlp, it's cached
    string_list_ty *wlp = file_list_get(as_view_path);

    fstate_src_ty *src_data = 0;
    if (n < wlp->nstrings)
    {
	trace(("%s\n", wlp->string[n]->str_text));
	src_data = project_file_find(this, wlp->string[n], as_view_path);

	//
	// If this assert fails, it means thato project_ty::file_list_get and
	// project_file_find are interpreting the view path differently.
	//
	assert(src_data);
    }
    trace(("return %8.8lX;\n", (long)src_data));
    trace(("}\n"));
    return src_data;
}
