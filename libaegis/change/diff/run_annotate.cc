//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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
#include <libaegis/user.h>


void
change_run_annotate_diff_command(change::pointer cp, user_ty::pointer up,
    string_ty *original, string_ty *input_file_name, string_ty *output_filename,
    string_ty *index_name, const char *diff_option)
{
    sub_context_ty  *scp;
    pconf_ty        *pconf_data;
    string_ty	    *the_command;

    //
    // The difference of 2 files, to be read by aeannotate.
    // (This isn't the same as diff_command, because it's aimed at
    // aeannotate, not at humans.)
    //
    // All of the substitutions described in aesub(5) are available.
    // In addition
    //
    // ${Original}
    //	    absolute path of original file copied from the baseline
    //	    usually, but not always
    //
    // ${Input}
    //	    absolute path of current file in the development directory
    //	    usually, but not always
    //
    // ${Output}
    //	    absolute path of file in which to write the
    //	    patch difference listing usually in the development
    //	    diretcory
    //
    // ${INDex}
    //	    The project-relative path of file being differenced.
    //	    This is the name to be used to index the file within the
    //	    patch, to ensure that patch(1) applies the patch to the
    //	    correct file.  (You don't want it applying the patch to
    //	    any of the absolute paths mentioned above, especially
    //	    as they could well be temporary files created for the
    //	    purpose of generating the patch.)
    //
    // ${OPTion}
    //      Extra diff options supplied on the aeannotate command line.
    //
    trace(("change_run_annotate_diff_command(cp = %8.8lX, up = %8.8lX, "
	"original = \"%s\", input_file_name = \"%s\", "
        "output_filename = \"%s\")\n{\n", (long)cp, (long)up.get(),
        original->str_text, input_file_name->str_text,
        output_filename->str_text));
    if (!diff_option)
	diff_option = "";
    assert(cp->reference_count>=1);
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
    if (original)
        sub_var_set_string(scp, "ORiginal", original);
    else
        sub_var_set_charstar(scp, "ORiginal", "/dev/null");
    sub_var_set_string(scp, "Input", input_file_name);
    sub_var_set_string(scp, "Output", output_filename);
    sub_var_set_string(scp, "INDex", index_name);
    sub_var_optional(scp, "INDex");
    if (!diff_option)
        diff_option = 0;
    sub_var_set_charstar(scp, "OPTion", diff_option);
    the_command = pconf_data->annotate_diff_command;
    if (!the_command)
    {
	//
	// This one defaults, without user attention.
	// It's aimed at aeannotate, not at a human.
	//
	the_command =
	    str_from_c
	    (
		"set +e; "
		"$diff $option ${quote $original} ${quote $input} "
		    "> ${quote $output}; "
		"test $$? -le 1"
	    );
    }
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);
    trace_string(the_command);
    change_env_set(cp, 0);
    user_ty::become scoped(up);
    if (os_exists(output_filename))
	os_unlink(output_filename);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_SILENT, dd);
    str_free(the_command);
    trace(("}\n"));
}
