//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
change_integration_directory_set(change::pointer cp, string_ty *s)
{
    cstate_ty       *cstate_data;

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    trace(("change_integration_directory_set(cp = %p, s = \"%s\")\n{\n",
        cp, s->str_text));
    assert(cp->reference_count >= 1);
    if (cp->integration_directory_resolved)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_charstar
        (
            scp,
            "Name",
            arglex_token_name(arglex_token_directory)
        );
        fatal_intl(scp, i18n("duplicate $name option"));
        // NOTREACHED
        sub_context_delete(scp);
    }
    cp->integration_directory_unresolved = str_copy(s);
    change_become(cp);
    cp->integration_directory_resolved = os_pathname(s, 1);
    change_become_undo(cp);
    cstate_data = cp->cstate_get();
    if (!cstate_data->integration_directory)
    {
        string_ty       *dir;

        dir = os_below_dir(project_Home_path_get(cp->pp), s);
        if (dir)
        {
            if (!dir->str_length)
            {
                assert(0);
                str_free(dir);
                dir = str_from_c(".");
            }
            cstate_data->integration_directory = dir;
        }
        else
            cstate_data->integration_directory = str_copy(s);
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
