//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


string_ty *
change_integration_directory_get(change::pointer cp, int resolve)
{
    nstring s = cp->integration_directory_get(!!resolve);
    //
    // We return a copy of the string_ty* so the result can be
    // free()ed.  This is to make the handling of strings more
    // uniform.
    //
    return str_copy(s.get_ref());
}


nstring
change::integration_directory_get(bool resolve)
{
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    trace(("change_integration_directory_get()\n{\n"));
    assert(reference_count >= 1);
    nstring result;
    if (!integration_directory_unresolved)
    {
        string_ty *dir = cstate_get()->integration_directory;
        if (!dir)
            change_fatal(this, 0, i18n("no int dir"));
        if (dir->str_text[0] == '/')
            integration_directory_unresolved = str_copy(dir);
        else
        {
            integration_directory_unresolved =
                os_path_cat(project_Home_path_get(pp), dir);
        }
    }
    if (!resolve)
        result = nstring(integration_directory_unresolved);
    else
    {
        if (!integration_directory_resolved)
        {
            change_become(this);
            integration_directory_resolved =
                os_pathname(integration_directory_unresolved, 1);
            change_become_undo(this);
        }
        result = nstring(integration_directory_resolved);
    }
    trace(("result = %s\n", result.quote_c().c_str()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
