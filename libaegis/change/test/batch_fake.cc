//
//	aegis - project change supervisor
//	Copyright (C) 2000-2008 Peter Miller
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

#include <common/ac/stdio.h>

#include <common/error.h> // for assert
#include <common/format_elpsd.h>
#include <common/now.h>
#include <common/nstring/list.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/change/test/batch_fake.h>
#include <libaegis/fstate.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


batch_result_list_ty *
change_test_batch_fake(change::pointer cp, string_list_ty *wlp,
    user_ty::pointer up, bool baseline_flag, int current, int total,
    time_t time_limit, const nstring_list &variable_assignments,
    const long *remaining)
{
    size_t	    j;
    string_ty	    *dir;
    int (*run_test_command)(change::pointer , user_ty::pointer , string_ty *,
			string_ty *, int, int, const nstring_list &vars);
    cstate_ty       *cstate_data;
    batch_result_list_ty *result;
    int		    persevere;

    //
    // which command
    //
    trace(("change_test_batch_fake(cp = %08lX, wlp = %08lX, up = %08lX, "
	"baseline_flag = %d, current = %d, total = %d, time_limit = %ld, "
        "remaining = %08lX)\n{\n", (long)cp, (long)wlp, (long)up.get(),
        baseline_flag, current, total, (long)time_limit, (long)remaining));
    cstate_data = cp->cstate_get();
    run_test_command = change_run_test_command;
    if (cstate_data->state != cstate_state_being_integrated)
	run_test_command = change_run_development_test_command;
    persevere = up->persevere_preference(true);

    //
    // directory depends on the state of the change
    //
    // During long tests the automounter can unmount the
    // directory referenced by the "dir" variable.
    // To minimize this, it is essential that they are
    // unresolved, and thus always trigger the automounter.
    //
    dir = cp->pp->baseline_path_get();
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

	//
	// display progress message if requested
	//
	if (total > 0)
	{
	    sub_context_ty sc;
	    sc.var_set_long("Current", current + j + 1);
	    sc.var_set_long("Total", total);
            trace(("remaining = %ld\n", remaining[j]));
            sc.var_set_string("REMaining", format_elapsed(remaining[j]));
            sc.var_optional("REMaining");
	    change_error(cp, &sc, i18n("test $current of $total"));
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
        double t_begin = dtime();
	exit_status =
	    run_test_command
	    (
		cp,
		up,
		fn_abs,
		dir,
		inp,
		baseline_flag,
		variable_assignments
	    );
        trace(("exit_status = %d\n", exit_status));
        double t_end = dtime();
        double elapsed = t_end - t_begin;
        trace(("elapsed = %g\n", elapsed));

	//
	// remember what happened
	//
	batch_result_list_append(result, fn, exit_status, 0, elapsed);

	//
	// verbose progress message
	//
	switch (exit_status)
	{
	case 1:
	    if (baseline_flag)
	    {
                sub_context_ty sc;
		sc.var_set_string("File_Name", fn);
		change_verbose(cp, &sc, i18n("$filename baseline fail, good"));
		result->pass_count++;
	    }
	    else
	    {
                sub_context_ty sc;
		sc.var_set_string("File_Name", fn);
		change_verbose(cp, &sc, i18n("$filename fail"));
		result->fail_count++;
	    }
	    break;

	case 0:
	    if (baseline_flag)
	    {
                sub_context_ty sc;
		sc.var_set_string("File_Name", fn);
		change_verbose
		(
		    cp,
		    &sc,
		    i18n("$filename baseline pass, not good")
		);
		result->fail_count++;
	    }
	    else
	    {
                sub_context_ty sc;
		sc.var_set_string("File_Name", fn);
		change_verbose(cp, &sc, i18n("$filename pass"));
		result->pass_count++;
	    }
	    break;

	case 77:
            {
                // Note: the value 77 was chosen to be compatible with
                // other test systems.
                sub_context_ty sc;
                sc.var_set_string("File_Name", fn);
                change_verbose(cp, &sc, i18n("$filename skipped"));
                result->skip_count++;
            }
	    break;

	default:
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", fn);
                change_verbose(cp, &sc, i18n("$filename no result"));
                result->no_result_count++;
            }
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
	    time_t curr_time;
	    time(&curr_time);
	    if (curr_time >= time_limit)
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
