//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002-2006 Peter Miller;
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
// MANIFEST: functions to manipulate run_lists
//

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/test/batch_fake.h>
#include <libaegis/change/test/batch.h>
#include <libaegis/change/test/run_list.h>
#include <libaegis/pconf.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>


batch_result_list_ty *
project_test_run_list(project_ty *pp, string_list_ty *wlp, user_ty *up,
    bool progress_flag, time_t time_limit,
    const nstring_list &variable_assignments)
{
    change_ty	    *cp;
    batch_result_list_ty *result;

    //
    // create a fake change,
    // so can set environment variables
    // for the test
    //
    trace(("project_run_test_list(pp = %08lX, wlp = %08lX, up = %08lX, "
	"progress_flag = %d, time_limit = %ld)\n{\n",(long)pp, (long)wlp,
	(long)up, progress_flag, (long)time_limit));
    cp = change_alloc(pp, project_next_change_number(pp, 1));
    change_bind_new(cp);
    change_architecture_from_pconf(cp);
    cp->bogus = 1;

    //
    // do each of the tests
    //
    result =
	change_test_run_list
	(
	    cp,
	    wlp,
	    up,
	    false, // not baseline, positive!
	    progress_flag,
	    time_limit,
	    variable_assignments
	);
    change_free(cp);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static batch_result_list_ty *
change_test_run_list_inner(change_ty *cp, string_list_ty *wlp, user_ty *up,
    bool baseline_flag, int current, int total, time_t time_limit,
    const nstring_list &variable_assignments)
{
    pconf_ty        *pconf_data;
    batch_result_list_ty *result;

    trace(("change_test_run_list_inner(cp = %08lX, wlp = %08lX, up = %08lX, "
	"baseline_flag = %d, current = %d, total = %d, time_limit = %ld)\n{\n",
	(long)cp, (long)wlp, (long)up, baseline_flag, current, total,
	(long)time_limit));
    pconf_data = change_pconf_get(cp, 1);
    if (wlp->nstrings == 0)
    {
	trace(("mark\n"));
	result = batch_result_list_new();
    }
    else if (pconf_data->batch_test_command)
    {
	trace(("mark\n"));
	result =
	    change_test_batch
	    (
		cp,
		wlp,
		up,
		baseline_flag,
		current,
		total,
		variable_assignments
	    );
    }
    else
    {
	trace(("mark\n"));
	result =
	    change_test_batch_fake
	    (
		cp,
		wlp,
		up,
		baseline_flag,
		current,
		total,
		time_limit,
		variable_assignments
	    );
    }
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


//
// The above function is the real one, this one simply breaks things up
// if it looks like the batch test command line may get too long.
//

batch_result_list_ty *
change_test_run_list(change_ty *cp, string_list_ty *wlp, user_ty *up,
    bool baseline_flag, bool progress_flag, time_t time_limit,
    const nstring_list &variable_assignments)
{
    batch_result_list_ty *result;
    size_t	    multiple;
    size_t	    j;
    int		    persevere;

    //
    // We limit ourselves to commands with at most 100 tests, because
    // some Unix implementations have very short command lines and can't
    // cope with more.
    //
    multiple = 100;
    if (time_limit)
    {
	//
        // The batch_test_command is expected to run tests in parallel.
        // Even if it doesn't, we don't have the ability to tell it to
        // stop.  So, when we have a time limit, reduce the number of
        // tests run between checks to see if we have run out of time.
	//
	multiple = 12;
    }

    if (wlp->nstrings <= multiple)
    {
	return
	    change_test_run_list_inner
	    (
		cp,
		wlp,
		up,
		baseline_flag,
		0, // start
		(progress_flag ? wlp->nstrings : 0),
		time_limit,
		variable_assignments
	    );
    }
    trace(("change_test_run_list(cp = %08lX, wlp = %08lX, up = %08lX, "
	"baseline_flag = %d, progress_flag = %d, time_limit = %ld)\n{\n",
	(long)cp, (long)wlp, (long)up, baseline_flag, progress_flag,
	(long)time_limit));
    result = batch_result_list_new();
    persevere = user_persevere_preference(up, 1);
    for (j = 0; j < wlp->nstrings; j += multiple)
    {
	size_t		end;
	size_t		k;
	batch_result_list_ty *result2;

	end = j + multiple;
	if (end > wlp->nstrings)
	    end = wlp->nstrings;
	string_list_ty wl2;
	for (k = j; k < end; ++k)
	    wl2.push_back(wlp->string[k]);
	result2 =
	    change_test_run_list_inner
	    (
		cp,
		&wl2,
		up,
		baseline_flag,
		j,
		(progress_flag ? wlp->nstrings : 0),
		time_limit,
		variable_assignments
	    );
	batch_result_list_append_list(result, result2);
	batch_result_list_delete(result2);

	//
	// Don't keep going if the user asked us not to.
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
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
