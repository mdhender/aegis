//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002-2004 Peter Miller;
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

#include <change.h>
#include <change/test/batch.h>
#include <change/test/batch_fake.h>
#include <change/test/run_list.h>
#include <pconf.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>
#include <trace.h>
#include <user.h>


batch_result_list_ty *
project_test_run_list(project_ty *pp, string_list_ty *wlp, user_ty *up,
    int progress_flag)
{
    change_ty	    *cp;
    batch_result_list_ty *result;

    //
    // create a fake change,
    // so can set environment variables
    // for the test
    //
    trace(("project_run_test_list(pp = %08lX, wlp = %08lX, up = %08lX, "
	"progress_flag = %d)\n{\n",(long)pp, (long)wlp, (long)up,
	progress_flag));
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
	    0, // not baseline, positive!
	    progress_flag
	);
    change_free(cp);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


static batch_result_list_ty *
change_test_run_list_inner(change_ty *cp, string_list_ty *wlp, user_ty *up,
    int bl, int current, int total)
{
    pconf_ty        *pconf_data;
    batch_result_list_ty *result;

    trace(("change_test_run_list_inner(cp = %08lX, wlp = %08lX, up = %08lX, "
	"bl = %d, current = %d, total = %d)\n{\n", (long)cp, (long)wlp,
	(long)up, bl, current, total));
    pconf_data = change_pconf_get(cp, 1);
    if (wlp->nstrings == 0)
    {
	trace(("mark\n"));
	result = batch_result_list_new();
    }
    else if (pconf_data->batch_test_command)
    {
	trace(("mark\n"));
	result = change_test_batch(cp, wlp, up, bl, current, total);
    }
    else
    {
	trace(("mark\n"));
	result = change_test_batch_fake(cp, wlp, up, bl, current, total);
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
change_test_run_list(change_ty *cp, string_list_ty *wlp, user_ty *up, int bl,
    int progress_flag)
{
    batch_result_list_ty *result;
    size_t	    multiple;
    size_t	    j;
    int		    persevere;

    multiple = 100;
    if (wlp->nstrings <= multiple)
    {
	return
	    change_test_run_list_inner
	    (
		cp,
		wlp,
		up,
		bl,
		0,
		progress_flag ? wlp->nstrings : 0
	    );
    }
    trace(("change_test_run_list(cp = %08lX, wlp = %08lX, up = %08lX, "
	"bl = %d, progress_flag = %d)\n{\n", (long)cp, (long)wlp, (long)up, bl,
	progress_flag));
    result = batch_result_list_new();
    persevere = user_persevere_preference(up, 1);
    for (j = 0; j < wlp->nstrings; j += multiple)
    {
	size_t		end;
	string_list_ty	wl2;
	size_t		k;
	batch_result_list_ty *result2;

	end = j + multiple;
	if (end > wlp->nstrings)
	    end = wlp->nstrings;
	string_list_constructor(&wl2);
	for (k = j; k < end; ++k)
	    string_list_append(&wl2, wlp->string[k]);
	result2 =
	    change_test_run_list_inner
	    (
		cp,
		&wl2,
		up,
		bl,
		j,
		progress_flag ? wlp->nstrings : 0
	    );
	string_list_destructor(&wl2);
	batch_result_list_append_list(result, result2);
	batch_result_list_delete(result2);

	//
	// Don't keep going if the user asked us not to.
	//
	if (!persevere && result->fail_count)
	    break;
    }
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
