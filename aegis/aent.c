/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to implement new test
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/fcntl.h>
#include <ac/unistd.h>

#include <ael/project/files.h>
#include <aent.h>
#include <arglex2.h>
#include <change/branch.h>
#include <change/file.h>
#include <col.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <str_list.h>


static void new_test_usage _((void));

static void
new_test_usage()
{
    char	    *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -New_Test [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_Test -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_Test -Help\n", progname);
    quit(1);
}


static void new_test_help _((void));

static void
new_test_help()
{
    help("aent", new_test_usage);
}


static void new_test_list _((void));

static void
new_test_list()
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
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, new_test_usage);
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
		duplicate_option_by_name(arglex_token_change, new_test_usage);
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, new_test_usage);
	    if (project_name)
		duplicate_option_by_name(arglex_token_project, new_test_usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;
	}
	arglex();
    }
    list_project_files(project_name, change_number);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void new_test_main _((void));

static void
new_test_main()
{
    cstate	    cstate_data;
    string_ty	    *s1;
    string_ty	    *s2;
    string_ty	    *dd;
    fstate_src	    src_data;
    int		    manual_flag;
    int		    automatic_flag;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    long	    n;
    string_list_ty  wl;
    size_t	    j;
    size_t	    k;
    int		    nerrs;
    log_style_ty    log_style;
    char	    *output;
    int		    use_template;

    trace(("new_test_main()\n{\n"));
    arglex();
    manual_flag = 0;
    automatic_flag = 0;
    project_name = 0;
    change_number = 0;
    log_style = log_style_append_default;
    string_list_constructor(&wl);
    output = 0;
    use_template = -1;
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
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, new_test_usage);
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
		duplicate_option_by_name(arglex_token_change, new_test_usage);
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, new_test_usage);
	    if (project_name)
		duplicate_option_by_name(arglex_token_project, new_test_usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_file:
	    if (arglex() != arglex_token_string)
		new_test_usage();
	    /* fall through... */

	case arglex_token_string:
	    s2 = str_from_c(arglex_value.alv_string);
	    string_list_append(&wl, s2);
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
		/* NOTREACHED */

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

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * lock the change state file
     * and the project state file for the test number
     */
    project_pstate_lock_prepare_top(pp);
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    log_open(change_logfile_get(cp), up, log_style);

    /*
     * It is an error if the change is not in the in_development state.
     * It is an error if the change is not assigned to the current user.
     */
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad nt state"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad nf branch"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));

    nerrs = 0;
    if (wl.nstrings)
    {
	string_list_ty	wl2;
	string_list_ty	search_path;
	int		based;
	string_ty	*base;

	/*
	 * Search path for resolving filenames.
	 */
	change_search_path_get(cp, &search_path, 1);

	/*
	 * Find the base for relative filenames.
	 */
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

	/*
	 * make sure each file named is unique
	 */
	string_list_constructor(&wl2);
	for (j = 0; j < wl.nstrings; ++j)
	{
	    s1 = wl.string[j];
	    if (s1->str_text[0] == '/')
		s2 = str_copy(s1);
	    else
		s2 = str_format("%S/%S", base, s1);
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
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    if (string_list_member(&wl2, s2))
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_error(cp, scp, i18n("too many $filename"));
		sub_context_delete(scp);
		++nerrs;
	    }
	    else
		string_list_append(&wl2, s2);
	    str_free(s2);
	}
	string_list_destructor(&search_path);
	string_list_destructor(&wl);
	wl = wl2;

	/*
	 * ensure that each file
	 * 1. is not already part of the change
	 * 2. is not already part of the baseline
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
	    s1 = wl.string[j];
	    if (change_file_find(cp, s1))
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s1);
		change_error(cp, scp, i18n("file $filename dup"));
		sub_context_delete(scp);
		++nerrs;
	    }
	    else
	    {
		src_data = project_file_find(pp, s1);
		if
		(
		    src_data
		&&
		    !src_data->deleted_by
		&&
		    !src_data->about_to_be_created_by
		)
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
	/*
	 * Invent a new test file name.
	 *
	 * Try 1000 times, as users could, conceivably, use
	 * files of the same name.
	 */
	s1 = 0;
	for (j = 0; j < 1000; ++j)
	{
	    n = project_next_test_number_get(pp);
	    s1 = change_new_test_filename_get(cp, n, !manual_flag);
	    if (!change_file_find(cp, s1) && !project_file_find(pp, s1))
		break;
	    s1 = 0;
	}
	if (!s1)
	    fatal_intl(0, i18n("all test numbers used"));
	string_list_append(&wl, s1);
	str_free(s1);
    }

    /*
     * check that each filename is OK
     */
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
	e = change_filename_check(cp, file_name, 1);
	if (e)
	{
	    sub_context_ty  *scp;

	    /*
	     * no internationalization if the error string
	     * required, this is done inside the
	     * change_filename_check function.
	     */
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
	/* NOTREACHED */
	sub_context_delete(scp);
    }

    /*
     * Create each file in the development directory.
     * Create any necessary directories along the way.
     */
    dd = change_development_directory_get(cp, 1);

    /*
     * create the files
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	change_file_template(cp, wl.string[j], up, use_template);
    }

    /*
     * Add the files to the change.
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	s1 = wl.string[j];
	src_data = change_file_new(cp, s1);
	src_data->action = file_action_create;
	if (manual_flag)
	    src_data->usage = file_usage_manual_test;
	else
	    src_data->usage = file_usage_test;
    }

    /*
     * the number of files changed, or the version did,
     * so stomp on the validation fields.
     */
    change_build_times_clear(cp);

    /*
     * The change now has at least one test, so cancel any testing
     * exemption.  (Cancel the baseline test exemption if this is
     * not the first change on this branch.)
     */
    cstate_data->test_exempt = 0;
    if (project_change_nth(pp, 0L, &n) && n != change_number)
	cstate_data->test_baseline_exempt = 0;

    /*
     * update the copyright years
     */
    change_copyright_years_now(cp);

    /*
     * run the change file command
     * and the project file command if necessary
     */
    change_run_new_test_command(cp, &wl, up);
    change_run_project_file_command(cp, up);

    /*
     * If there is an output option,
     * write the change number to the file.
     */
    if (output)
    {
	string_ty	*content;

	content = wl2str(&wl, 0, wl.nstrings, "\n");
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

    /*
     * update the state files
     * release the locks
     */
    project_pstate_write_top(pp);
    change_cstate_write(cp);
    commit();
    lock_release();

    /*
     * verbose success message
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	s1 = wl.string[j];
	sub_var_set_string(scp, "File_Name", s1);
	change_verbose(cp, scp, i18n("new test $filename complete"));
	sub_context_delete(scp);
    }
    string_list_destructor(&wl);
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}


void
new_test()
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
