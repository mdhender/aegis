//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <common/trace.h>


string_ty *
change_run_history_query_command(change::pointer cp, fstate_src_ty *src)
{
    sub_context_ty  *scp;
    string_ty	    *hp;
    pconf_ty        *pconf_data;
    string_ty	    *the_command;
    string_ty	    *result;
    string_ty	    *hfn;

    //
    // Ask the history file what its edit number is.  We use this
    // method because the string returned is essentially random,
    // between different history programs.  Only ever executed in
    // the "being integrated" state.  Current directory will be
    // set to the base of the history tree.  All of the
    // substitutions described in aesub(5) are available.  In
    // addition
    //
    // ${History}
    //	absolute path of history file
    //
    trace(("change_run_history_query_command(cp = %8.8lX, "
        "src->file_name = \"%s\")\n{\n", (long)cp, src->file_name->str_text));
    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 1);
    hfn = project_history_filename_get(cp->pp, src);
    scp = sub_context_new();
    sub_var_set_string(scp, "History", hfn);
    str_free(hfn);
    the_command = pconf_data->history_query_command;
    if (!the_command)
    {
	sub_context_ty	*scp2;

	assert(pconf_data->errpos);
	scp2 = sub_context_new();
	sub_var_set_string(scp2, "File_Name", pconf_data->errpos);
	sub_var_set_charstar(scp2, "FieLD_Name", "history_query_command");
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

    change_env_set(cp, 0);
    hp = cp->pp->history_path_get();
    project_become(cp->pp);
    result = os_execute_slurp(the_command, OS_EXEC_FLAG_NO_INPUT, hp);
    project_become_undo(cp->pp);
    str_free(the_command);
    if (!result->str_length)
	fatal_intl(0, i18n("history_query_command return empty"));
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}
