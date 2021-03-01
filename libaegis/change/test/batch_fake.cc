//
//	aegis - project change supervisor
//	Copyright (C) 2000-2005 Peter Miller;
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
// MANIFEST: functions to manipulate batch_fakes
//

#include <ac/stdio.h>

#include <change.h>
#include <change/file.h>
#include <change/test/batch_fake.h>
#include <error.h> // for assert
#include <fstate.h>
#include <project.h>
#include <project/file.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


batch_result_list_ty *
change_test_batch_fake(change_ty *cp, string_list_ty *wlp, user_ty *up,
    bool baseline_flag, int current, int total, time_t time_limit)
{
    size_t	    j;
    string_ty	    *dir;
    int		    (*run_test_command)(change_ty *, user_ty *, string_ty *,
			string_ty *, int, int);
    cstate_ty       *cstate_data;
    batch_result_list_ty *result;
    int		    persevere;

    //
    // which command
    //
    trace(("change_test_batch_fake(cp = %08lX, wlp = %08lX, up = %08lX, "
	"baseline_flag = %d, current = %d, total = %d, time_limit = %ld)\n{\n",
	(long)cp, (long)wlp, (long)up, baseline_flag, current, total,
	(long)time_limit));
    cstate_data = change_cstate_get(cp);
    run_test_command = change_run_test_command;
    if (cstate_data->state != cstate_state_being_integrated)
	run_test_command = change_run_development_test_command;
    persevere = user_persevere_preference(up, 1);

    //
    // directory depends on the state of the change
    //
    // During long tests the automounter can unmount the
    // directory referenced by the ``dir'' variable.
    // To minimize this, it is essential that they are
    // unresolved, and thus always trigger the automounter.
    //
    dir = project_baseline_path_get(cp->pp, 0);
    if (!baseline_flag && !cp->bogus)
    {
	switch (cstate_data->state)
	{
	case cstate_state_awaiting_development:
	    assert(0);

	case cstate_state_completed:
	    break;

	case cstate_state_being_integrated:
	    dir = change_integration_directory_get(cp, 0);
	    break;

	case cstate_state_being_developed:
	case cstate_state_awaiting_review:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
	    dir = change_development_directory_get(cp, 0);
	    break;
	}
    }

    //
    // allocate a place to store the results
    //
    result = batch_result_list_new();

    //
    // one command per file
    //
    for (j = 0; j < wlp->nstrings; ++j)
    {
	string_ty	*fn;
	string_ty	*fn_abs;
	fstate_src_ty   *src_data;
	int		inp;
	int		exit_status;
	sub_context_ty	*scp;

	//
	// display progress message if requested
	//
	if (total > 0)
	{
	    scp = sub_context_new();
	    sub_var_set_long(scp, "Current", current + j + 1);
	    sub_var_set_long(scp, "Total", total);
	    change_error(cp, scp, i18n("test $current of $total"));
	    sub_context_delete(scp);
	}

	//
	// resolve the file name
	//
	fn = wlp->string[j];
	src_data = change_file_find(cp, fn, view_path_first);
	if (src_data)
	{
	    fn_abs = change_file_path(cp, fn);
	}
	else
	{
	    src_data = project_file_find(cp->pp, fn, view_path_simple);
	    assert(src_data);
	    fn_abs = project_file_path(cp->pp, fn);
	}
	assert(fn_abs);

	//
	// figure the command execution flags
	//
	inp = 0;
	switch (src_data->usage)
	{
	case file_usage_source:
	case file_usage_config:
	case file_usage_build:
	case file_usage_test:
	    break;

	case file_usage_manual_test:
	    inp = 1;
	    break;
	}

	//
	// run the command
	//
	exit_status = run_test_command(cp, up, fn_abs, dir, inp, baseline_flag);

	//
	// remember what happened
	//
	batch_result_list_append(result, fn, exit_status, 0);

	//
	// verbose progress message
	//
	switch (exit_status)
	{
	case 1:
	    if (baseline_flag)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", fn);
		change_verbose(cp, scp, i18n("$filename baseline fail, good"));
		sub_context_delete(scp);
		result->pass_count++;
	    }
	    else
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", fn);
		change_verbose(cp, scp, i18n("$filename fail"));
		sub_context_delete(scp);
		result->fail_count++;
	    }
	    break;

	case 0:
	    if (baseline_flag)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", fn);
		change_verbose
		(
		    cp,
		    scp,
		    i18n("$filename baseline pass, not good")
		);
		sub_context_delete(scp);
		result->fail_count++;
	    }
	    else
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", fn);
		change_verbose(cp, scp, i18n("$filename pass"));
		sub_context_delete(scp);
		result->pass_count++;
	    }
	    break;

	default:
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", fn);
	    change_verbose(cp, scp, i18n("$filename no result"));
	    sub_context_delete(scp);
	    result->no_result_count++;
	    break;
	}
	str_free(fn_abs);

	//
	// don't persevere if the user asked us not to
	//
	if (!persevere && (result->fail_count || result->no_result_count))
	    break;

	//
        // If we have been given a time limit, and that time has passed,
        // do not continue testing.
	//
	if (time_limit)
	{
	    time_t now;
	    time(&now);
	    if (now >= time_limit)
		break;
	}
    }

    //
    // all done
    //
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
