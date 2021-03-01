//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <libaegis/project.h>
#include <libaegis/os.h>
#include <libaegis/project/history/uuid_trans.h>
#include <common/trace.h>


string_ty *
project_history_filename_get(project_ty *pp, fstate_src_ty *src)
{
    string_ty       *path;
    string_ty       *rel;
    string_ty       *result;

    trace(("project_history_filename_get(pp = %8.8lX, src=%8.8lX)\n{\n",
	(long)pp, (long)src));
    path = pp->history_path_get();
    trace(("path = \"%s\"\n", path->str_text));
    rel = project_history_uuid_translate(src);
    trace(("rel = \"%s\"\n", rel->str_text));
    result = os_path_cat(path, rel);
    str_free(rel);
    trace(("}\n"));
    return result;
}
