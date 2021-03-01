//
// aegis - project change supervisor
// Copyright (C) 1999, 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
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

#include <common/str_list.h>
#include <libaegis/change/env_set.h>
#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
change_run_development_build_command(change::pointer cp, user_ty::pointer up,
    string_list_ty *partial)
{
    sub_context_ty  *scp;
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    string_ty       *dd;

    assert(cp->reference_count >= 1);
    assert(cp->is_being_developed());
    pconf_data = change_pconf_get(cp, 1);
    assert(pconf_data);
    the_command = pconf_data->development_build_command;
    if (!the_command)
    {
        assert(pconf_data->errpos);
        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", pconf_data->errpos);
        sub_var_set_charstar(scp, "FieLD_Name", "build_command");
        change_fatal
        (
            cp,
            scp,
            i18n("$filename: contains no \"$field_name\" field")
        );
        // NOTREACHED
        sub_context_delete(scp);
    }

    //
    // place the partial file name into the File_List variable
    //
    scp = sub_context_new();
    if (partial->nstrings)
    {
        string_ty       *s;

        s = partial->unsplit();
        sub_var_set_string(scp, "File_List", s);
        str_free(s);
        sub_var_append_if_unused(scp, "File_List");
    }
    else
    {
        sub_var_set_charstar(scp, "File_List", "");
        sub_var_optional(scp, "File_List");
    }

    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);

    dd = change_development_directory_get(cp, 0);
    change_env_set(cp, 1);
    user_ty::become scope(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
    str_free(the_command);
}


// vim: set ts=8 sw=4 et :
