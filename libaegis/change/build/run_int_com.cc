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

#include <libaegis/change/env_set.h>
#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
change_run_build_command(change::pointer cp)
{
    sub_context_ty  *scp;
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    string_ty       *id;

    assert(cp->reference_count >= 1);
    assert(cp->cstate_data);
    assert(cp->cstate_data->state == cstate_state_being_integrated);
    pconf_data = change_pconf_get(cp, 1);
    assert(pconf_data);
    the_command = pconf_data->build_command;
    assert(the_command);
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

    scp = sub_context_new();
    sub_var_set_charstar(scp, "File_List", "");
    sub_var_optional(scp, "File_List");
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);

    id = change_integration_directory_get(cp, 0);
    change_env_set(cp, 1);
    project_become(cp->pp);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, id);
    project_become_undo(cp->pp);
    str_free(the_command);
}


// vim: set ts=8 sw=4 et :
