//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#include <common/trace.h>
#include <libaegis/project.h>
#include <libaegis/os.h>


string_ty *
project_ty::baseline_path_get(bool resolve)
{
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    trace(("project_baseline_path_get(this = %08lX)\n{\n", (long)this));
    if (!baseline_path_unresolved)
    {
	string_ty *dd = project_top_path_get(this, 0);
	baseline_path_unresolved = str_format("%s/baseline", dd->str_text);
    }
    string_ty *result = 0;
    if (!resolve)
	result = baseline_path_unresolved;
    else
    {
	if (!baseline_path)
	{
	    project_become(this);
	    baseline_path = os_pathname(baseline_path_unresolved, 1);
	    project_become_undo(this);
	}
	result = baseline_path;
    }
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}
