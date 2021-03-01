//
//	aegis - project change supervisor
//	Copyright (C) 1991-2006 Peter Miller;
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
// MANIFEST: functions to implement new test
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/fcntl.h>
#include <common/ac/unistd.h>

#include <libaegis/ael/project/files.h>
#include <aegis/aent.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <common/error.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>
#include <common/uuidentifier.h>


static void
new_test_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -New_Test [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_Test -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_Test -Help\n", progname);
    quit(1);
}


static void
new_test_help(void)
{
    help("aent", new_test_usage);
}


static void
new_test_list(void)
{
    string_ty	    *project_name;
    long	    change_number;

    trace(("new_test_list()\n{\n"));
    project_name = 0;
    change_number = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_test_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, new_test_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, new_test_usage);
	    continue;
	}
	arglex();
    }
    list_project_files(project_name, change_number, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
new_test_main(void)
{
    cstate_ty	    *cstate_data;
    string_ty	    *s1;
    string_ty	    *s2;
    fstate_src_ty   *src_data;
    int		    manual_flag;
    int		    automatic_flag;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    long	    n;
    size_t	    j;
    size_t	    k;
    int		    nerrs;
    log_style_ty    log_style;
    const char      *output;
    int		    use_template;
    string_ty       *uuid;

    trace(("new_test_main()\n{\n"));
    arglex();
    manual_flag = 0;
    automatic_flag = 0;
    project_name = 0;
    change_number = 0;
    log_style = log_style_append_default;
    string_list_ty wl;
    output = 0;
    use_template = -1;
    uuid = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_test_usage);
	    continue;

	case arglex_token_manual:
	    if (manual_flag)
		duplicate_option(new_test_usage);
	    manual_flag = 1;
	    break;

	case arglex_token_automatic:
	    if (automatic_flag)
		duplicate_option(new_test_usage);
	    automatic_flag = 1;
	    break;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, new_test_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, new_test_usage);
	    continue;

	case arglex_token_file:
	    if (arglex() != arglex_token_string)
		new_test_usage();
	    // fall through...

	case arglex_token_string:
	    s2 = str_from_c(arglex_value.alv_string);
	    wl.push_back(s2);
	    str_free(s2);
	    break;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(new_test_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(new_test_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_relative_filename_preference_argument(new_test_usage);
	    break;

	case arglex_token_output:
	    if (output)
		duplicate_option(new_test_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_output, new_test_usage);
		// NOTREACHED

	    case arglex_token_string:
		output = arglex_value.alv_string;
		break;

	    case arglex_token_stdio:
		output = "";
		break;
	    }
	    break;

	case arglex_token_template:
	case arglex_token_template_not:
	    if (use_template >= 0)
		duplicate_option(new_test_usage);
	    use_template = (arglex_token == arglex_token_template);
	    break;

        case arglex_token_uuid:
            if (uuid)
                duplicate_option(new_test_usage);
            if (arglex () != arglex_token_string)
                option_needs_string(arglex_token_uuid, new_test_usage);
            s2 = str_from_c(arglex_value.alv_string);
            if (!universal_unique_identifier_valid(s2))
		option_needs_string(arglex_token_uuid, new_test_usage);
	    uuid = str_downcase(s2);
	    str_free(s2);
	    s2 = 0;
            break;
	}
	arglex();
    }
    if (change_number && output)
    {
	mutually_exclusive_options
	(
	    arglex_token_change,
	    arglex_token_output,
	    new_test_usage
	);
    }
    if (automatic_flag && manual_flag)
    {
	mutually_exclusive_options
	(
	    arglex_token_manual,
	    arglex_token_automatic,
	    new_test_usage
	);
    }
    if (!manual_flag && !automatic_flag)
	automatic_flag = 1;

    //
    //  It is an error if the -uuid switch is used and more that one
    //  file is nominated on the command line.
    //
    if (uuid && wl.nstrings > 1)
    {
	error_intl(0, i18n("too many files"));
	new_test_usage();
    }

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // lock the change state file
    // and the project state file for the test number
    //
    project_pstate_lock_prepare_top(pp);
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    log_open(change_logfile_get(cp), up, log_style);

    //
    // It is an error if the change is not in the in_development state.
    // It is an error if the change is not assigned to the current user.
    //
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad nt state"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad nf branch"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));

    //
    // It is an error if the UUID is already is use.
    //
    if (uuid)
    {
	fstate_src_ty   *src;

	src = change_file_find_uuid(cp, uuid, view_path_simple);
	if (src)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Other", src->file_name);
	    sub_var_optional(scp, "Other");
	    change_fatal(cp, scp, i18n("bad ca, uuid duplicate"));
	    // NOTREACHED
	}
    }

    nerrs = 0;
    if (wl.nstrings)
    {
	string_list_ty	search_path;
	int		based;
	string_ty	*base;

	//
	// Search path for resolving filenames.
	//
	change_search_path_get(cp, &search_path, 1);

	//
	// Find the base for relative filenames.
	//
	based =
	    (
		search_path.nstrings >= 1
	    &&
		(
		    user_relative_filename_preference
		    (
			up,
			uconf_relative_filename_preference_current
		    )
		==
		    uconf_relative_filename_preference_base
		)
	    );
	if (based)
	    base = search_path.string[0];
	else
	{
	    os_become_orig();
	    base = os_curdir();
	    os_become_undo();
	}

	//
	// make sure each file named is unique
	//
	string_list_ty wl2;
	for (j = 0; j < wl.nstrings; ++j)
	{
	    s1 = wl.string[j];
	    if (s1->str_text[0] == '/')
		s2 = str_copy(s1);
	    else
		s2 = os_path_join(base, s1);
	    user_become(up);
	    s1 = os_pathname(s2, 1);
	    user_become_undo();
	    str_free(s2);
	    s2 = 0;
	    for (k = 0; k < search_path.nstrings; ++k)
	    {
		s2 = os_below_dir(search_path.string[k], s1);
		if (s2)
		    break;
	    }
	    str_free(s1);
	    if (!s2)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", wl.string[j]);
		change_fatal(cp, scp, i18n("$filename unrelated"));
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    if (wl2.member(s2))
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_error(cp, scp, i18n("too many $filename"));
		sub_context_delete(scp);
		++nerrs;
	    }
	    else
		wl2.push_back(s2);
	    str_free(s2);
	}
	wl = wl2;

	//
	// ensure that each file
	// 1. is not already part of the change
	//        - except removed files
	// 2. is not already part of the baseline
	//
	for (j = 0; j < wl.nstrings; ++j)
	{
	    s1 = wl.string[j];
	    src_data = change_file_find(cp, s1, view_path_first);
	    if (src_data)
	    {
		sub_context_ty	*scp;

		switch (src_data->action)
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
		    scp = sub_context_new();
		    sub_var_set_string(scp, "File_Name", s1);
		    change_error(cp, scp, i18n("file $filename dup"));
		    sub_context_delete(scp);
		    ++nerrs;
		    break;
		}
	    }
	    else
	    {
		src_data = project_file_find(pp, s1, view_path_extreme);
		if (src_data)
		{
		    sub_context_ty  *scp;

		    scp = sub_context_new();
		    sub_var_set_string(scp, "File_Name", s1);
		    project_error(pp, scp, i18n("$filename in baseline"));
		    sub_context_delete(scp);
		    ++nerrs;
		}
	    }
	}
    }
    else
    {
	//
	// Invent a new test file name.
	//
	// Try 1000 times, as users could, conceivably, use
	// files of the same name.
	//
	s1 = 0;
	for (j = 0; j < 1000; ++j)
	{
	    n = project_next_test_number_get(pp);
	    s1 = change_new_test_filename_get(cp, n, !manual_flag);
	    if
	    (
		!change_file_find(cp, s1, view_path_first)
	    &&
		!project_file_find(pp, s1, view_path_extreme)
	    )
		break;
	    s1 = 0;
	}
	if (!s1)
	    fatal_intl(0, i18n("all test numbers used"));
	wl.push_back(s1);
	str_free(s1);
    }

    //
    // check that each filename is OK
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	string_ty	*e;
	string_ty	*file_name;
	string_ty	*other;

	file_name = wl.string[j];
	if (change_file_is_config(cp, file_name))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", file_name);
	    change_error(cp, scp, i18n("may not test $filename"));
	    sub_context_delete(scp);
	    ++nerrs;
	}
	other = change_file_directory_conflict(cp, file_name);
	if (other)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", file_name);
	    sub_var_set_string(scp, "File_Name2", other);
	    sub_var_optional(scp, "File_Name2");
	    change_error
	    (
		cp,
		scp,
		i18n("file $filename directory name conflict")
	    );
	    sub_context_delete(scp);
	    ++nerrs;
	    continue;
	}
	other = project_file_directory_conflict(pp, file_name);
	if (other)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", file_name);
	    sub_var_set_string(scp, "File_Name2", other);
	    sub_var_optional(scp, "File_Name2");
	    project_error
	    (
		pp,
		scp,
		i18n("file $filename directory name conflict")
	    );
	    sub_context_delete(scp);
	    ++nerrs;
	}
	e = change_filename_check(cp, file_name);
	if (e)
	{
	    sub_context_ty  *scp;

	    //
	    // no internationalization if the error string
	    // required, this is done inside the
	    // change_filename_check function.
	    //
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Message", e);
	    change_error(cp, scp, i18n("$message"));
	    sub_context_delete(scp);
	    ++nerrs;
	    str_free(e);
	}
    }
    if (nerrs)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", nerrs);
	sub_var_optional(scp, "Number");
	change_fatal(cp, scp, i18n("new test failed"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // create the files
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	change_file_template(cp, wl.string[j], up, use_template);
    }

    //
    // Add the files to the change.
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	s1 = wl.string[j];

	//
	// If the file is already in the change (we checked for this
	// earlier) then it must be being removed, and we are replacing
	// it, so we can change its type.
	//
	src_data = change_file_find(cp, s1, view_path_first);
	if (src_data)
	{
	    assert(src_data->action == file_action_remove);
	    if (src_data->uuid)
	    {
		if (uuid)
		    duplicate_option_by_name(arglex_token_uuid, new_test_usage);
		uuid = src_data->uuid;
		src_data->uuid = 0;
	    }
	    change_file_remove(cp, s1);
	}

	src_data = change_file_new(cp, s1);
	src_data->action = file_action_create;
	if (manual_flag)
	    src_data->usage = file_usage_manual_test;
	else
	    src_data->usage = file_usage_test;
	if (uuid)
	{
    	    assert(universal_unique_identifier_valid(uuid));
    	    src_data->uuid = uuid;
	    uuid = 0;
	}
    }

    //
    // the number of files changed, or the version did,
    // so stomp on the validation fields.
    //
    change_build_times_clear(cp);

    //
    // The change now has at least one test, so cancel any testing
    // exemption.  (Cancel the baseline test exemption if this is
    // not the first change on this branch.)
    //
    cstate_data->test_exempt = false;
    if (project_change_nth(pp, 0L, &n) && n != change_number)
	cstate_data->test_baseline_exempt = false;

    //
    // update the copyright years
    //
    change_copyright_years_now(cp);

    //
    // If the file manifest of the change is altered (e.g. by aenf, aenfu,
    // aecp, aecpu, etc), or the contents of any file is changed, the
    // UUID is cleared.  This is because it is no longer the same change
    // as was received by aedist or aepatch, and the UUID is invalidated.
    //
    change_uuid_clear(cp);

    //
    // run the change file command
    // and the project file command if necessary
    //
    change_run_new_test_command(cp, &wl, up);
    change_run_project_file_command(cp, up);

    //
    // If there is an output option,
    // write the change number to the file.
    //
    if (output)
    {
	string_ty *content = wl.unsplit("\n");
	if (*output)
	{
	    string_ty	    *fn;

	    fn = str_from_c(output);
	    user_become(up);
	    file_from_string(fn, content, 0644);
	    user_become_undo();
	    str_free(fn);
	}
	else
	    cat_string_to_stdout(content);
	str_free(content);
    }

    //
    // update the state files
    // release the locks
    //
    project_pstate_write_top(pp);
    change_cstate_write(cp);
    commit();
    lock_release();

    //
    // verbose success message
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	s1 = wl.string[j];
	sub_var_set_string(scp, "File_Name", s1);
	change_verbose(cp, scp, i18n("new test $filename complete"));
	sub_context_delete(scp);
    }
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}


void
new_test(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, new_test_help, },
	{arglex_token_list, new_test_list, },
    };

    trace(("new_test()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_test_main);
    trace(("}\n"));
}
