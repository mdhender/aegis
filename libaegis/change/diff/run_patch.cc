//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate run_patchs
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
change_run_patch_diff_command(change_ty *cp, user_ty  *up, string_ty *original,
    string_ty *input, string_ty *output, string_ty *index_name)
{
    sub_context_ty  *scp;
    pconf_ty        *pconf_data;
    string_ty	    *the_command;

    //
    // The difference of 2 files, to send around as a patch.
    // (This isn't the same as diff_command, because it's aimed at
    // GNU Patch, not at humans.)
    //
    // All of the substitutions described in aesub(5) are available.
    // In addition
    //
    // ${Original}
    //     absolute path of original file copied from the baseline
    //     usually, but not always
    //
    // ${Input}
    //     absolute path of current file in the development directory
    //     usually, but not always
    //
    // ${Output}
    //     absolute path of file in which to write the
    //     patch difference listing usually in the development
    //     diretcory
    //
    // ${INDex}
    //     The project-relative path of file being differenced.
    //     This is the name to be used to index the file within the
    //     patch, to ensure that patch(1) applies the patch to the
    //     correct file.  (You don't want it applying the patch to
    //     any of the absolute paths mentioned above, especially
    //     as they could well be temporary files created for the
    //     purpose of generating the patch.)
    //
    trace(("change_run_patch_diff_command(cp = %8.8lX, up = %8.8lX, "
	"original = \"%s\", input = \"%s\", output = \"%s\")\n{\n",
	(long)cp, (long)up, original->str_text, input->str_text,
	output->str_text));
    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 1);
    string_ty *dd = 0;
    switch (change_cstate_get(cp)->state)
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
    sub_var_set_string(scp, "Input", input);
    sub_var_set_string(scp, "Output", output);
    sub_var_set_string(scp, "INDex", index_name);
    sub_var_optional(scp, "INDex");
    the_command = pconf_data->patch_diff_command;
    if (!the_command)
    {
	//
	// This one defaults, without user attention.
	// It's aimed at GNU Patch, not at a human.
	//
	the_command =
	    str_from_c
	    (
		"set +e; "
		"$diff "
#ifdef HAVE_GNU_DIFF
		    "-u "
		    "--text "
		    "-L ${quote $index} -L ${quote $index} "
#else
		    "-c "
#endif
		    "${quote $original} ${quote $input} > ${quote $output}; "
		"test $$? -le 1"
	    );
    }
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);
    trace_string(the_command->str_text);
    change_env_set(cp, 0);
    user_become(up);
    if (os_exists(output))
	os_unlink(output);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, dd);
    user_become_undo();
    str_free(the_command);
    trace(("}\n"));
}
