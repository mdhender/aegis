//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002-2008 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/math.h>

#include <common/error.h> // for assert
#include <common/nstring.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/change/test/batch_fake.h>
#include <libaegis/change/test/batch.h>
#include <libaegis/change/test/run_list.h>
#include <libaegis/pconf.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>


batch_result_list_ty *
project_test_run_list(project_ty *pp, string_list_ty *wlp, user_ty::pointer up,
    bool progress_flag, time_t time_limit,
    const nstring_list &variable_assignments)
{
    change::pointer cp;
    batch_result_list_ty *result;

    //
    // create a fake change,
    // so can set environment variables
    // for the test
    //
    trace(("project_run_test_list(pp = %08lX, wlp = %08lX, up = %08lX, "
	"progress_flag = %d, time_limit = %ld)\n{\n",(long)pp, (long)wlp,
	(long)up.get(), progress_flag, (long)time_limit));
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
change_test_run_list_inner(change::pointer cp, string_list_ty *wlp,
    user_ty::pointer up, bool baseline_flag, int current, int total,
    time_t time_limit, const nstring_list &variable_assignments,
    const long *countdown)
{
    trace(("change_test_run_list_inner(cp = %08lX, wlp = %08lX, up = %08lX, "
	"baseline_flag = %d, current = %d, total = %d, time_limit = %ld, "
        "countdown = %08lX)\n{\n", (long)cp, (long)wlp, (long)up.get(),
        baseline_flag, current, total, (long)time_limit, (long)countdown));
    pconf_ty *pconf_data = change_pconf_get(cp, 1);
    batch_result_list_ty *result = 0;
    if (wlp->nstrings == 0)
    {
	result = batch_result_list_new();
    }
    else if (pconf_data->batch_test_command)
    {
	result =
	    change_test_batch
	    (
		cp,
		wlp,
		up,
		baseline_flag,
		current,
		total,
		variable_assignments,
                countdown
	    );
    }
    else
    {
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
		variable_assignments,
                countdown
	    );
    }
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static double
calculate_average_elapsed(change::pointer cp, const nstring &attr_name)
{
    double sum = 0;
    long n = 0;
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty *src = cp->pp->file_nth(j, view_path_extreme);
        if (!src)
            break;

        if (src->usage != file_usage_test)
        {
            //
            // we don't bother with the timings for manual tests,
            // they will have human induced oscillations.
            //
            continue;
        }

        double secs =
            attributes_list_find_real(src->attribute, attr_name.c_str(), -1);
        if (secs > 0)
        {
            sum += secs;
            ++n;
        }
    }
    if (n == 0)
        return 60;
    sum /= n;
    if (sum < 1e-3)
        sum = 1e-3;
    return sum;
}


static void
calculate_eta(change::pointer cp, string_list_ty *wlp, long *countdown)
{
    nstring arch_name(change_architecture_name(cp, 0));
    if (arch_name.empty())
        arch_name = "unspecified";
    nstring attr_name = "test/" + arch_name + "/elapsed";
    double average_elapsed = -1;
    double eta = 0;
    for (size_t nn = 0; nn < wlp->nstrings; ++nn)
    {
        size_t n = wlp->nstrings - 1 - nn;
        string_ty *fn = wlp->string[n];
        fstate_src_ty *src = change_file_find(cp, fn, view_path_simple);
        double secs = -1;
        assert(src);
        if (src)
        {
            secs =
                attributes_list_find_real
                (
                    src->attribute,
                    attr_name.c_str(),
                    -1
                );
        }
        if (secs < 0)
        {
            if (average_elapsed < 0)
                average_elapsed = calculate_average_elapsed(cp, attr_name);
            assert(average_elapsed > 0);
            secs = average_elapsed;
        }
        assert(secs > 0);
        eta += secs;
        countdown[n] = long(ceil(eta));
    }
}


//
// The above function is the real one, this one simply breaks things up
// if it looks like the batch test command line may get too long.
//

batch_result_list_ty *
change_test_run_list(change::pointer cp, string_list_ty *wlp,
    user_ty::pointer up, bool baseline_flag, bool progress_flag,
    time_t time_limit, const nstring_list &variable_assignments)
{
    //
    // Build an ETA based on the past performance of the tests on this
    // architecture.
    //
    long *countdown = new long [wlp->nstrings];
    calculate_eta(cp, wlp, countdown);

    //
    // We limit ourselves to commands with at most 100 tests, because
    // some Unix implementations have very short command lines and can't
    // cope with more.
    //
    size_t multiple = 100;
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
        batch_result_list_ty *result =
	    change_test_run_list_inner
	    (
		cp,
		wlp,
		up,
		baseline_flag,
		0, // start
		(progress_flag ? wlp->nstrings : 0),
		time_limit,
		variable_assignments,
                countdown
	    );
        delete [] countdown;
        return result;
    }
    trace(("change_test_run_list(cp = %08lX, wlp = %08lX, up = %08lX, "
	"baseline_flag = %d, progress_flag = %d, time_limit = %ld)\n{\n",
	(long)cp, (long)wlp, (long)up.get(), baseline_flag, progress_flag,
	(long)time_limit));
    batch_result_list_ty *result = batch_result_list_new();
    int persevere = up->persevere_preference(true);
    for (size_t j = 0; j < wlp->nstrings; j += multiple)
    {
	size_t end = j + multiple;
	if (end > wlp->nstrings)
	    end = wlp->nstrings;
	string_list_ty wl2;
	for (size_t k = j; k < end; ++k)
	    wl2.push_back(wlp->string[k]);
	batch_result_list_ty *result2 =
	    change_test_run_list_inner
	    (
		cp,
		&wl2,
		up,
		baseline_flag,
		j,
		(progress_flag ? wlp->nstrings : 0),
		time_limit,
		variable_assignments,
                countdown + j
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
    delete [] countdown;
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
