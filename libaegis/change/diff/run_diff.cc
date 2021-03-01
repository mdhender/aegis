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
#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
change_run_diff_command(change::pointer cp, user_ty::pointer up,
    string_ty *original, string_ty *input_file_name, string_ty *output_filename)
{
    sub_context_ty  *scp;
    pconf_ty        *pconf_data;
    string_ty       *the_command;

    //
    // Run the diff_command.
    // All of the substitutions described in aesub(5) are available.
    // In addition
    //
    // ${Original}
    //  absolute path of original file copied from the baseline
    //  usually, but not always
    //
    // ${Input}
    //  absolute path of current file in the development directory
    //  usually, but not always
    //
    // ${Output}
    //  absolute path of file in which to write the difference listing
    //  usually in the development diretcory
    //
    trace
    ((
        "change_run_diff_command(cp = %p, up = %p, original = \"%s\", "
            "input_file_name = \"%s\", output_filename = \"%s\")\n{\n",
        cp,
        up.get(),
        original->str_text,
        input_file_name->str_text,
        output_filename->str_text
    ));
    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 1);
    string_ty *dd = 0;
    switch (cp->cstate_get()->state)
    {
    case cstate_state_being_developed:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
        dd = change_development_directory_get(cp, 0);
        break;

    case cstate_state_being_integrated:
        dd = change_integration_directory_get(cp, 0);
        break;

    case cstate_state_awaiting_development:
    case cstate_state_completed:
#ifndef DEBUG
    default:
#endif
        dd = os_tmpdir();
        break;
    }
    assert(dd);
    scp = sub_context_new();
    sub_var_set_string(scp, "ORiginal", original);
    sub_var_set_string(scp, "Input", input_file_name);
    sub_var_set_string(scp, "Output", output_filename);
    the_command = pconf_data->diff_command;
    if (!the_command)
    {
        sub_context_ty  *scp2;

        assert(pconf_data->errpos);
        scp2 = sub_context_new();
        sub_var_set_string(scp2, "File_Name", pconf_data->errpos);
        sub_var_set_charstar(scp2, "FieLD_Name", "diff_command");
        change_fatal
        (
            cp,
            scp2,
            i18n("$filename: contains no \"$field_name\" field")
        );
        // NOTREACHED
        sub_context_delete(scp2);
    }
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);
    trace_string(the_command->str_text);
    change_env_set(cp, 0);
    user_ty::become scoped(up);
    if (os_exists(output_filename))
        os_unlink(output_filename);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
    str_free(the_command);
    str_free(dd);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
