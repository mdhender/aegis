//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
//	Copyright (C) 2006 Walter Franzini
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
#include <common/ac/stdlib.h>

#include <common/error.h> // for assert
#include <common/now.h>
#include <common/progname.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/architecture/find_variant.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/user.h>

#include <aefinish/finish.h>


static nstring progress_option;


void
finish_usage()
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>\n", progname);
    fprintf(stderr, "       %s --list [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


static bool
finish_merge(change_identifier &cid)
{
    project_ty *pp = cid.get_pp();
    change::pointer cp = cid.get_cp();
    if (cp->is_being_integrated())
	return false;

    //
    // look for files which need to be merged
    // We use the same test that aed(1) uses.
    //
    for (size_t j = 0; ; ++j)
    {
	//
	// find the relevant change src data
	//
	fstate_src_ty *src1_data = change_file_nth(cp, j, view_path_first);
	if (!src1_data)
	    break;

	//
	// generated files are not merged
	// created or deleted files are not merged
	//
	switch (src1_data->usage)
	{
	case file_usage_build:
	    // toss these ones
	    continue;

	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    // keep these ones
	    break;
	}
	switch (src1_data->action)
	{
	case file_action_create:
	    if (src1_data->move)
	    {
		// keep move destinations
		break;
	    }
	    // toss these ones
	    continue;

	case file_action_remove:
	    // toss these ones
	    continue;

	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
	    // keep these ones
	    break;
	}

	//
	// find the relevant baseline src data
	// note that someone may have deleted it from under you
	//
	// If the edit numbers match (is up to date)
	// then do not merge this one.
	//
	fstate_src_ty *src2_data =
	    project_file_find(pp, src1_data, view_path_extreme);
	if (!src2_data)
	    continue;
	if (change_file_up_to_date(pp, src1_data))
	    continue;

	//
	// At least one file needs to be merged.
	//
	nstring command = "aegis --diff --merge-only --project="
	    + nstring(project_name_get(pp)).quote_shell() + " --change="
	    + nstring::format("%ld", magic_zero_decode(cp->number))
	    + " --verbose";
	int flags = 0;
	nstring dd = nstring(change_development_directory_get(cp, 0));
	os_become_orig();
	os_execute(command, flags, dd);
	os_become_undo();

	//
	// Return true ("did something").
	//
	return true;
    }

    //
    // return false ("did nothing").
    //
    return false;
}


static bool
finish_diff(change_identifier &cid)
{
    project_ty *pp = cid.get_pp();
    change::pointer cp = cid.get_cp();

    //
    // Make sure that we terminate elegantly if no diff is required
    // for this project.
    //
    if (!change_diff_required(cp))
	return false;

    bool integrating = cp->is_being_integrated();

    //
    // check each file to see if it needs to be differenced
    //
    for (size_t j = 0; ; ++j)
    {
	trace(("j = %ld\n", (long)j));
	fstate_src_ty *src1_data = change_file_nth(cp, j, view_path_first);
	trace(("src1_data = %08lX\n", (long)src1_data));
	if (!src1_data)
	    break;

	//
	// find the relevant change src data
	//
	string_ty *s1 = src1_data->file_name;
	trace(("file name \"%s\"\n", s1->str_text));
	trace(("action = %s\n", file_action_ename(src1_data->action)));
	trace(("usage = %s\n", file_usage_ename(src1_data->usage)));

	//
	// locate the equivalent project file
	//
	fstate_src_ty *src2_data =
	    project_file_find(pp, src1_data, view_path_extreme);
	trace(("src2_data = %08lX\n", (long)src2_data));
	if (src2_data)
	{
	    trace(("action2 = %s\n", file_action_ename(src2_data->action)));
	    trace(("usage2 = %s\n", file_usage_ename(src2_data->usage)));
	}

	//
	// generated files are not differenced
	//
	switch (src1_data->usage)
	{
	case file_usage_build:
	    switch (src1_data->action)
	    {
	    case file_action_create:
		continue;

	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_remove:
	    case file_action_transparent:
		break;
	    }
	    continue;

	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    break;
	}

	switch (src1_data->action)
	{
	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    break;

	case file_action_remove:
	    //
	    // the removed half of a move is not differenced
	    //
	    if
	    (
		src1_data->move
	    &&
		change_file_find(cp, src1_data->move, view_path_first)
	    )
		continue;
	    break;

	case file_action_transparent:
	    //
	    // Transparent files are not differenced when integrating.
	    //
	    if (integrating)
		continue;
	    break;
	}

	//
	// build various paths
	//
	string_ty *path = change_file_path(cp, s1);
	assert(path);
	trace(("change file path \"%s\"\n", path->str_text));
	string_ty *path_d = str_format("%s,D", path->str_text);
	trace(("change file,D path \"%s\"\n", path_d->str_text));

	//
	// Check the file's fingerprint.  This will zap
	// the other timestamps if the fingerprint has
	// changed.
	//
	switch (src1_data->action)
	{
	case file_action_remove:
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    if (!integrating)
	    {
		change_file_fingerprint_check(cp, src1_data);
		assert(src1_data->file_fp);
		assert(src1_data->file_fp->youngest > 0);
		assert(src1_data->file_fp->oldest > 0);
	    }
	    break;
	}

	//
	// All change files have a difference file,
	// eventually, even files being removed and
	// created.  Do nothing if we can.
	//
	os_become_orig();
	bool ignore = true;
	if (integrating)
	{
	    ignore =
		change_fingerprint_same
		(
		    src1_data->idiff_file_fp,
		    path_d,
		    0
		);
	}
	else
	{
	    ignore =
		change_fingerprint_same
		(
		    src1_data->diff_file_fp,
		    path_d,
		    0
		);
	}
	os_become_undo();
	str_free(path);
	str_free(path_d);
	if (ignore)
	{
	    trace(("ignore\n"));
	    continue;
	}

	switch (src1_data->action)
	{
	case file_action_create:
	case file_action_remove:
	case file_action_transparent:
	case file_action_modify:
	    break;

	case file_action_insulate:
	    assert(!integrating);
	    if (integrating)
	    {
		trace(("ignore\n"));
		continue;
	    }
	    break;
	}

	//
	// At least one file needs to be differenced.
	//
	nstring command = "aegis --diff --no-merge --project="
	    + nstring(project_name_get(pp)).quote_shell() + " --change="
	    + nstring::format("%ld", magic_zero_decode(cp->number))
	    + " --verbose";
	int flags = 0;
	nstring dd = nstring(change_development_directory_get(cp, 0));
	os_become_orig();
	os_execute(command, flags, dd);
	os_become_undo();

	//
	// Return true ("did something").
	//
	return true;
    }

    //
    // Return false ("did nothing").
    //
    return false;
}


static time_t
calculate_youngest_file(change_identifier &cid)
{
    change::pointer cp = cid.get_cp();
    time_t youngest = 0;
    for (size_t j = 0;; ++j)
    {
	fstate_src_ty *c_src_data = change_file_nth(cp, j, view_path_first);
	if (!c_src_data)
	    break;
	trace(("file_name = \"%s\"\n", c_src_data->file_name->str_text));

	switch (c_src_data->action)
	{
	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    break;

	case file_action_remove:
	case file_action_transparent:
	    continue;
	}
	switch (c_src_data->usage)
	{
	case file_usage_build:
	    continue;

	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    break;
	}

	string_ty *path = change_file_path(cp, c_src_data->file_name);
	if (!c_src_data->file_fp)
	{
	    c_src_data->file_fp =
		(fingerprint_ty *)fingerprint_type.alloc();
	}
	assert(c_src_data->file_fp->youngest >= 0);
	assert(c_src_data->file_fp->oldest >= 0);
	os_become_orig();
	change_fingerprint_same(c_src_data->file_fp, path, 0);
	os_become_undo();
	assert(c_src_data->file_fp->youngest > 0);
	assert(c_src_data->file_fp->oldest > 0);

	if (!c_src_data->file_fp || !c_src_data->file_fp->youngest)
	    continue;

	if (c_src_data->file_fp->oldest > youngest)
	    youngest = c_src_data->file_fp->oldest;
    }
    return youngest;
}


static bool
finish_build(change_identifier &cid, time_t youngest)
{
    project_ty *pp = cid.get_pp();
    change::pointer cp = cid.get_cp();
    if (!change_build_required(cp))
	return false;

    //
    // verify that the youngest file is older than the build time
    // by architecture
    //
    cstate_architecture_times_ty *tp = change_find_architecture_variant(cp);
    if (!tp->build_time || youngest >= tp->build_time)
    {
        if (cp->is_being_developed())
        {
            //
            // If the project defines a develop_end_policy_command it is
            // a good idea to run it here, in case it requires work to
            // be done to the source files.  By doing it here you don't
            // spend all the time of building and testing (which could
            // be time consuming) only to find you forgot to update a
            // copyright notice.
            //
            change_run_develop_end_policy_command(cp, cid.get_up());
        }

	//
	// At least one file needs to be differenced.
	//
	nstring command = "aegis --build --project="
	    + nstring(project_name_get(pp)).quote_shell() + " --change="
	    + nstring::format("%ld", magic_zero_decode(cp->number))
	    + " --verbose";
	int flags = 0;
	nstring dd = nstring(change_development_directory_get(cp, 0));
	os_become_orig();
	os_execute(command, flags, dd);
	os_become_undo();

	//
	// Return true ("did something").
	//
	return true;
    }

    //
    // Return false ("did nothing").
    //
    return false;
}


static bool
finish_test(change_identifier &cid, time_t youngest)
{
    project_ty *pp = cid.get_pp();
    change::pointer cp = cid.get_cp();
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->test_exempt)
	return false;

    //
    // verify that the youngest file is older than the build time
    // by architecture
    //
    cstate_architecture_times_ty *tp = change_find_architecture_variant(cp);
    if (!tp->test_time || youngest >= tp->test_time)
    {
	//
	// At least one file needs to be tested.
	//
	nstring command = "aegis --test --project="
	    + nstring(project_name_get(pp)).quote_shell() + " --change="
	    + nstring::format("%ld", magic_zero_decode(cp->number))
	    + " --verbose" + progress_option;
	int flags = 0;
	nstring dd = nstring(change_development_directory_get(cp, 0));
	os_become_orig();
	os_execute(command, flags, dd);
	os_become_undo();

	//
	// Return true ("did something").
	//
	return true;
    }

    //
    // Return false ("did nothing").
    //
    return false;
}


static bool
finish_test_baseline(change_identifier &cid, time_t youngest)
{
    project_ty *pp = cid.get_pp();
    change::pointer cp = cid.get_cp();
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->test_baseline_exempt)
	return false;

    //
    // verify that the youngest file is older than the build time
    // by architecture
    //
    cstate_architecture_times_ty *tp = change_find_architecture_variant(cp);
    if (!tp->test_baseline_time || youngest >= tp->test_baseline_time)
    {
	//
	// At least one file needs to be tested.
	//
	nstring command = "aegis --test --baseline --project="
	    + nstring(project_name_get(pp)).quote_shell() + " --change="
	    + nstring::format("%ld", magic_zero_decode(cp->number))
	    + " --verbose" + progress_option;
	int flags = 0;
	nstring dd = nstring(change_development_directory_get(cp, 0));
	os_become_orig();
	os_execute(command, flags, dd);
	os_become_undo();

	//
	// Return true ("did something").
	//
	return true;
    }

    //
    // Return false ("did nothing").
    //
    return false;
}


static bool
finish_test_regression(change_identifier &cid, time_t youngest)
{
    project_ty *pp = cid.get_pp();
    change::pointer cp = cid.get_cp();
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->regression_test_exempt)
	return false;

    //
    // verify that the youngest file is older than the build time
    // by architecture
    //
    cstate_architecture_times_ty *tp = change_find_architecture_variant(cp);
    if (!tp->regression_test_time || youngest >= tp->regression_test_time)
    {
	//
	// At least one file needs to be tested.
	//
	nstring command = "aegis --test --regression --project="
	    + nstring(project_name_get(pp)).quote_shell() + " --change="
	    + nstring::format("%ld", magic_zero_decode(cp->number))
	    + " --verbose" + progress_option;
	int flags = 0;
	nstring dd = nstring(change_development_directory_get(cp, 0));
	os_become_orig();
	os_execute(command, flags, dd);
	os_become_undo();

	//
	// Return true ("did something").
	//
	return true;
    }

    //
    // Return false ("did nothing").
    //
    return false;
}


static void
work_loop(change_identifier &cid)
{
    //
    // This code is written in the form of a loop, because between each
    // command, we have to re-load the change state.  Other operations
    // can happen simultaneously, the most obvious one being that files
    // may be integrated into the baseline.
    //
    // Each time we re-load the change state, we have to re-calculate
    // what needs to be done.
    //
    for (;;)
    {
	now_clear();
	cid.invalidate_change_meta_data();

	if (finish_merge(cid))
	    continue;
	if (finish_diff(cid))
	    continue;
	time_t youngest = calculate_youngest_file(cid);
	if (finish_build(cid, youngest))
	    continue;
	if (finish_test(cid, youngest))
	    continue;
	if (finish_test_baseline(cid, youngest))
	    continue;
	if (finish_test_regression(cid, youngest))
	    continue;
	return;
    }
}


static void
finish_development(change_identifier &cid)
{
    work_loop(cid);

    project_ty *pp = cid.get_pp();
    change::pointer cp = cid.get_cp();
    nstring dd = nstring(change_development_directory_get(cp, 0));
    nstring command = "aegis --develop-end --project="
	+ nstring(project_name_get(pp)).quote_shell() + " --change="
	+ nstring::format("%ld", magic_zero_decode(cp->number))
	+ " --verbose";
    int flags = 0;
    os_become_orig();
    os_execute(command, flags, dd);
    os_become_undo();
}


static const char *
home()
{
    const char *cp = getenv("HOME");
    if (!cp)
	cp = "/";
    return cp;
}


static void
finish_integration(change_identifier &cid)
{
    work_loop(cid);

    project_ty *pp = cid.get_pp();
    change::pointer cp = cid.get_cp();
    nstring dir = home();
    nstring command = "aegis --integrate-pass --project="
	+ nstring(project_name_get(pp)).quote_shell() + " --change="
	+ nstring::format("%ld", magic_zero_decode(cp->number))
	+ " --verbose";
    int flags = 0;
    os_become_orig();
    os_execute(command, flags, dir);
    os_become_undo();
}


void
finish()
{
    trace(("finish()\n{\n"));
    change_identifier cid;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(finish_usage);
	    continue;

	case arglex_token_project:
	case arglex_token_string:
	case arglex_token_change:
	case arglex_token_number:
	    cid.command_line_parse(finish_usage);
	    continue;

	case arglex_token_progress:
            user_ty::progress_option_clear(finish_usage);
	    break;

	case arglex_token_progress_not:
            user_ty::progress_option_clear(finish_usage);
	    break;
	}
	arglex();
    }

    //
    // reject illegal combinations of options
    //
    cid.command_line_check(finish_usage);

    progress_option =
        (
            cid.get_up()->progress_get()
        ?
            " --progress"
        :
            " --no-progress"
        );

    cstate_ty *cstate_data = cid.get_cp()->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_awaiting_review:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_integration:
    case cstate_state_completed:
	fatal_raw("wrong state");
	// NOTREACHED

    case cstate_state_being_developed:
	finish_development(cid);
	break;

    case cstate_state_being_integrated:
	finish_integration(cid);
	break;
    }
    trace(("}\n"));
}
