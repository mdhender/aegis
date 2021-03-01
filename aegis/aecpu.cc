//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2004 Peter Miller;
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
// MANIFEST: functions to impliment copy file undo
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>
#include <ac/libintl.h>

#include <aecpu.h>
#include <ael/change/files.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <commit.h>
#include <change/branch.h>
#include <change/file.h>
#include <error.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <quit.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <str_list.h>


static void
copy_file_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -CoPy_file_Undo [ <option>... ] <filename>...\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -CoPy_file_Undo -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -CoPy_file_Undo -Help\n", progname);
    quit(1);
}


static void
copy_file_undo_help(void)
{
    help("aecpu", copy_file_undo_usage);
}


static void
copy_file_undo_list(void)
{
    string_ty	    *project_name;
    long	    change_number;

    trace(("copy_file_undo_list()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(copy_file_undo_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		copy_file_undo_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, copy_file_undo_usage);
	    continue;
	}
	arglex();
    }
    list_change_files(project_name, change_number, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
copy_file_undo_main(void)
{
    sub_context_ty  *scp;
    size_t	    j;
    size_t	    k;
    string_ty	    *s1;
    string_ty	    *s2;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    log_style_ty    log_style;
    user_ty	    *up;
    int		    config_seen;
    int             unchanged;
    int		    number_of_errors;
    string_list_ty  search_path;
    int		    based;
    string_ty	    *base;
    int             tests_uncopied;

    trace(("copy_file_undo_main()\n{\n"));
    arglex();
    string_list_ty wl;
    project_name = 0;
    change_number = 0;
    log_style = log_style_append_default;
    unchanged = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(copy_file_undo_usage);
	    continue;

	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, copy_file_undo_usage);
	    goto get_file_names;

	case arglex_token_file:
	    if (arglex() != arglex_token_string)
		option_needs_files(arglex_token_file, copy_file_undo_usage);
	    // fall through...

	case arglex_token_string:
	    get_file_names:
	    s2 = str_from_c(arglex_value.alv_string);
	    wl.push_back(s2);
	    str_free(s2);
	    break;

	case arglex_token_keep:
	case arglex_token_interactive:
	case arglex_token_keep_not:
	    user_delete_file_argument(copy_file_undo_usage);
	    break;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		copy_file_undo_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, copy_file_undo_usage);
	    continue;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(copy_file_undo_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_unchanged:
	    if (unchanged)
		duplicate_option(copy_file_undo_usage);
	    unchanged = 1;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(copy_file_undo_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_relative_filename_preference_argument(copy_file_undo_usage);
	    break;

	case arglex_token_symbolic_links:
	case arglex_token_symbolic_links_not:
	    user_symlink_pref_argument(copy_file_undo_usage);
	    break;
	}
	arglex();
    }
    if (!unchanged && !wl.nstrings)
	fatal_intl(0, i18n("no file names"));

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

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
    // lock the change file
    //
    change_cstate_lock_prepare(cp);
    lock_take();

    log_open(change_logfile_get(cp), up, log_style);

    //
    // It is an error if the change is not in the in_development state.
    // It is an error if the change is not assigned to the current user.
    //
    if (!change_is_being_developed(cp))
	change_fatal(cp, 0, i18n("bad cp undo state"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad cp undo branch"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));

    //
    // If no files were named and the -unchanged option was used,
    // add all of the modified files in the change.
    // It is an error if there are none.
    //
    if (!wl.nstrings)
	wl.push_back(change_development_directory_get(cp, 1));

    //
    // resolve the path of each file
    // 1.   the absolute path of the file name is obtained
    // 2.   if the file is inside the development directory, ok
    // 3.   if the file is inside the baseline, ok
    // 4.   if neither, error
    //
    config_seen = 0;
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

    string_list_ty wl2;
    number_of_errors = 0;
    for (j = 0; j < wl.nstrings; ++j)
    {
	string_list_ty	wl_in;

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
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", wl.string[j]);
	    change_error(cp, scp, i18n("$filename unrelated"));
	    sub_context_delete(scp);
	    ++number_of_errors;
	    continue;
	}
	change_file_directory_query(cp, s2, &wl_in, 0);
	if (wl_in.nstrings)
	{
	    int		    used;

	    //
	    // If the user named a directory, add all of the
	    // source files in this change in that
	    // directory, provided they were added using
	    // the aecp command.
	    //
	    used = 0;
	    for (k = 0; k < wl_in.nstrings; ++k)
	    {
		string_ty	*s3;
		fstate_src_ty   *src_data;

		s3 = wl_in.string[k];
		src_data = change_file_find(cp, s3, view_path_first);
		assert(src_data);
		if (src_data)
		{
		    switch (src_data->action)
		    {
		    case file_action_modify:
		    case file_action_insulate:
			if (wl2.member(s3))
			{
			    scp = sub_context_new();
			    sub_var_set_string(scp, "File_Name", s3);
			    change_error(cp, scp, i18n("too many $filename"));
			    sub_context_delete(scp);
			    ++number_of_errors;
			}
			else
			    wl2.push_back(s3);
			if (change_file_is_config(cp, s3))
			    ++config_seen;
			++used;
			break;

		    case file_action_create:
		    case file_action_remove:
		    case file_action_transparent:
			break;
		    }
		}
	    }
	    if (!used)
	    {
		scp = sub_context_new();
		if (s2->str_length)
		    sub_var_set_string(scp, "File_Name", s2);
		else
		    sub_var_set_charstar(scp, "File_Name", ".");
		sub_var_set_long(scp, "Number", (long)wl_in.nstrings);
		sub_var_optional(scp, "Number");
		change_error
		(
		    cp,
		    scp,
		    i18n("directory $filename contains no relevant files")
		);
		sub_context_delete(scp);
		++number_of_errors;
	    }
	}
	else
	{
	    if (wl2.member(s2))
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_error(cp, scp, i18n("too many $filename"));
		sub_context_delete(scp);
		++number_of_errors;
	    }
	    else
		wl2.push_back(s2);
	    if (change_file_is_config(cp, s2))
		++config_seen;
	}
	str_free(s2);
    }
    wl = wl2;

    //
    // ensure that each file
    // 1. is already part of the change
    // 2. is being modified by this change
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	fstate_src_ty   *src_data;

	s1 = wl.string[j];
	src_data = change_file_find(cp, s1, view_path_first);
	if (!src_data)
	{
	    scp = sub_context_new();
	    src_data = change_file_find_fuzzy(cp, s1);
	    sub_var_set_string(scp, "File_Name", s1);
	    if (src_data)
	    {
		sub_var_set_string(scp, "Guess", src_data->file_name);
		change_error(cp, scp, i18n("no $filename, closest is $guess"));
	    }
	    else
		change_error(cp, scp, i18n("no $filename"));
	    sub_context_delete(scp);
	    ++number_of_errors;
	    continue;
	}
	switch (src_data->action)
	{
	case file_action_modify:
	case file_action_insulate:
	    break;

	case file_action_create:
	case file_action_remove:
	case file_action_transparent:
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s1);
	    change_error(cp, scp, i18n("bad cp undo $filename"));
	    sub_context_delete(scp);
	    ++number_of_errors;
	    continue;
	}
	if (config_seen && src_data->file_fp)
	{
	    fingerprint_type.free(src_data->file_fp);
	    src_data->file_fp = 0;
	}
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no file names"));
	++number_of_errors;
    }
    if (number_of_errors)
    {
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", number_of_errors);
	sub_var_optional(scp, "Number");
	change_fatal(cp, scp, i18n("no files uncopied"));
	sub_context_delete(scp);
    }

    //
    // Remove each file from the development directory,
    // if it still exists.
    // Remove the difference file, too.
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	int		exists;
        int             blf_unlink = 0;

	s1 = wl.string[j];
	s2 = change_file_path(cp, s1);
	assert(s2);
	user_become(up);
	exists = os_exists(s2);
	user_become_undo();

	//
	// skip the changed files
	// if the user asked us to work on unchanged files
	//
	if (unchanged && exists)
	{
	    string_ty	    *blf;
	    int		    different;
	    fstate_src_ty   *src_data;
	    fstate_src_ty   *psrc_data;

	    //
	    // Leave cross branch merges alone, even if they
	    // don't change anything.  The information is
	    // about the merge, not the file contents.
	    //
	    src_data = change_file_find(cp, s1, view_path_first);
	    if (!src_data)
		goto not_this_one;
	    if (src_data->edit_origin_new)
	    {
		assert(src_data->edit_origin_new->revision);
		goto not_this_one;
	    }
	    switch (src_data->action)
	    {
	    case file_action_modify:
	    case file_action_insulate:
		break;

	    case file_action_create:
	    case file_action_remove:
	    case file_action_transparent:
		goto not_this_one;
	    }

	    //
	    // The file could have vanished from under us,
	    // so make sure this is sensable.
	    //
	    psrc_data = project_file_find(pp, s1, view_path_extreme);
	    if (!psrc_data)
		goto not_this_one;

	    //
	    // Compare the files.
	    //
	    blf = project_file_path(pp, s1);
	    assert(blf);
	    {
		os_become_orig();
		int blf_exists = os_exists(blf);
		os_become_undo();
		assert(blf_exists);
		if (!blf_exists)
		    blf = project_file_version_path(pp, psrc_data, &blf_unlink);
	    }

            user_become(up);
	    different = files_are_different(s2, blf);
	    user_become_undo();
            if (blf_unlink)
            {
                os_become_orig();
                os_unlink(blf);
                os_become_undo();
            }
            str_free(blf);
	    if (different)
	    {
		not_this_one:
		wl.remove(s1);
		--j;
		goto next;
	    }
	}

	//
	// delete the file if it exists
	// and the users wants us to
	//
	if (exists && user_delete_file_query(up, s1, 0))
	{
	    //
            // This is not as robust in the face of
	    // errors as using commit.  Its merit
	    // is its simplicity.
	    //
	    // It plays nice with the change_maintain_symlinks_to_basline
	    // call, below.
	    //
            // Also, the rename-and-delete shenanigans
	    // take a long time over NFS, and users
	    // expect this to be fast.
	    //
	    user_become(up);
	    os_unlink_errok(s2);
	    user_become_undo();
	}

	//
	// always delete the difference file
	//
	user_become(up);
	s1 = str_format("%s,D", s2->str_text);
	if (os_exists(s1))
	    commit_unlink_errok(s1);
	str_free(s1);

	//
	// always delete the backup merge file
	//
	s1 = str_format("%s,B", s2->str_text);
	if (os_exists(s1))
	    commit_unlink_errok(s1);
	str_free(s1);
	user_become_undo();
	next:
	str_free(s2);
    }

    //
    // Remove each file from the change file,
    // and write it back out.
    //
    tests_uncopied = 0;
    for (j = 0; j < wl.nstrings; ++j)
    {
	fstate_src_ty   *psrc_data;

	change_file_remove(cp, wl.string[j]);

	//
	// See if there are any tests were uncopied by this action.
	//
	psrc_data = project_file_find(pp, wl.string[j], view_path_simple);
	switch (psrc_data->usage)
	{
	case file_usage_source:
	case file_usage_config:
	case file_usage_build:
	    break;

	case file_usage_test:
	case file_usage_manual_test:
	    tests_uncopied = 1;
	    break;
	}
    }

    if (tests_uncopied)
    {
	int             more_tests;

	more_tests = 0;
	change_force_regression_test_exemption_undo(cp);
	for (j = 0; ; ++j)
	{
	    fstate_src_ty   *src_data;

	    src_data = change_file_nth(cp, j, view_path_first);
	    if (!src_data)
	       	break;
	    switch (src_data->usage)
	    {
	    case file_usage_test:
	    case file_usage_manual_test:
		more_tests = 1;
		break;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_build:
		continue;
	    }
	    break;
	}
	if (!more_tests)
	    change_rescind_test_exemption_undo(cp);
    }

    //
    // Only clear the various timestamps if we actually did something.
    //
    // This is especially important for aedist -receive, which
    // automagtically calls aecpu -unch to make sure it supresses change
    // sets we already have.  But we don't want to clear the UUID if we
    // didn't remove any files from the change set.
    //
    if (wl.nstrings > 0)
    {
	//
	// the number of files changed,
	// so stomp on the validation fields.
	//
	change_build_times_clear(cp);

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
	change_run_copy_file_undo_command(cp, &wl, up);
	change_run_project_file_command(cp, up);
    }

    //
    // Repair symlinks (etc) to the baseline.
    //
    change_maintain_symlinks_to_baseline(cp, up);

    //
    // release the locks
    //
    change_cstate_write(cp);
    commit();
    lock_release();

    //
    // verbose success message
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", wl.string[j]);
	change_verbose(cp, scp, i18n("copy file undo $filename complete"));
	sub_context_delete(scp);
    }

    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}


void
copy_file_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, copy_file_undo_help, },
	{arglex_token_list, copy_file_undo_list, },
    };

    trace(("copy_file_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), copy_file_undo_main);
    trace(("}\n"));
}
