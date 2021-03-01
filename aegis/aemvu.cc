//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
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

#include <common/error.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/files.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change.h>
#include <libaegis/commit.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/pconf.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>

#include <aegis/aemvu.h>


static void
move_file_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -MoVe_file_Undo [ <option>... ] <filename>...\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -MoVe_file_Undo -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -MoVe_file_Undo -Help\n", progname);
    quit(1);
}


static void
move_file_undo_help(void)
{
    help("aemvu", move_file_undo_usage);
}


static void
move_file_undo_list(void)
{
    trace(("move_file_undo_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(move_file_undo_usage);
    list_change_files(cid, 0);
    trace(("}\n"));
}


static void
move_file_undo_main(void)
{
    sub_context_ty  *scp;
    size_t	    j;
    size_t	    k;
    string_ty	    *s1;
    string_ty	    *s2;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change::pointer cp;
    log_style_ty    log_style;
    user_ty::pointer up;
    int		    config_seen;
    int		    number_of_errors;
    string_list_ty  search_path;
    int		    based;
    string_ty	    *base;

    trace(("move_file_undo_main()\n{\n"));
    string_list_ty wl;
    project_name = 0;
    change_number = 0;
    log_style = log_style_append_default;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(move_file_undo_usage);
	    continue;

	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, move_file_undo_usage);
	    goto get_file_names;

	case arglex_token_file:
	    if (arglex() != arglex_token_string)
		option_needs_files(arglex_token_file, move_file_undo_usage);
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
	    user_ty::delete_file_argument(move_file_undo_usage);
	    break;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		move_file_undo_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, move_file_undo_usage);
	    continue;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(move_file_undo_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(move_file_undo_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_ty::relative_filename_preference_argument
            (
                move_file_undo_usage
            );
	    break;

	case arglex_token_symbolic_links:
	case arglex_token_symbolic_links_not:
	    user_ty::symlink_pref_argument(move_file_undo_usage);
	    break;
	}
	arglex();
    }
    if (!wl.nstrings)
	fatal_intl(0, i18n("no file names"));

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
	change_number = up->default_change(pp);
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
    if (!cp->is_being_developed())
	change_fatal(cp, 0, i18n("bad cp undo state"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("bad cp undo branch"));
    if (nstring(change_developer_name(cp)) != up->name())
	change_fatal(cp, 0, i18n("not developer"));

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
		up->relative_filename_preference
		(
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
	up->become_begin();
	s1 = os_pathname(s2, 1);
	up->become_end();
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
	    // the aemv command.
	    //
	    used = 0;
	    for (k = 0; k < wl_in.nstrings; ++k)
	    {
		string_ty	*s3;
		fstate_src_ty   *src_data;

		s3 = wl_in.string[k];
		src_data = change_file_find(cp, s3, view_path_first);
		assert(src_data);
		if (src_data && src_data->move)
		{
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
    // 2. is being moved by this change
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
	if (src_data->move)
	{
	    //
	    // Add the other half of the move,
	    // if it isn't there already.
	    //
	    wl.push_back_unique(src_data->move);
	}
	else
	{
	    //
	    // If there is no "other half", then it
	    // wasn't added to the change as a file move,
	    // so complain.
	    //
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s1);
	    change_error(cp, scp, i18n("bad mv undo $filename"));
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

	s1 = wl.string[j];
	s2 = change_file_path(cp, s1);
	assert(s2);
	up->become_begin();
	exists = os_exists(s2);
	up->become_end();

	//
	// delete the file if it exists
	// and the user wants us to
	//
	if (exists && up->delete_file_query(nstring(s1), false, -1))
	{
	    //
	    // This is not as robust in the face of
	    // errors as using commit.  Its merit
	    // is its simplicity.
	    //
	    // It plays nice with the change_maintain_symlinks_to_baseline
	    // call below.
	    //
	    // Also, the rename-and-delete shenanigans
	    // take a long time over NFS, and users
	    // expect this to be fast.
	    //
            user_ty::become scoped(up);
	    os_unlink(s2);
	}

	//
	// always delete the difference file
	// and the merge backup file
	//
        user_ty::become scoped(up);
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
	str_free(s2);
    }

    //
    // Remove each file from the change file,
    // and write it back out.
    //
    string_list_ty wl_nfu;
    string_list_ty wl_ntu;
    string_list_ty wl_rmu;
    for (j = 0; j < wl.nstrings; ++j)
    {
	string_ty	*s;
	fstate_src_ty   *c_src;

	s = wl.string[j];
	c_src = change_file_find(cp, s, view_path_first);
	assert(c_src);
	if (!c_src)
	    continue;
	switch (c_src->action)
	{
	case file_action_create:
	    switch (c_src->usage)
	    {
	    case file_usage_test:
	    case file_usage_manual_test:
		wl_ntu.push_back(s);
		break;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_build:
		wl_nfu.push_back(s);
		break;
	    }
	    break;

	case file_action_modify:
	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    wl_rmu.push_back(s);
	    break;
	}
	change_file_remove(cp, s);
    }

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
    // Maintain the symlinks (etc) to the baseline.
    //
    bool undoing = true;
    change_maintain_symlinks_to_baseline(cp, up, undoing);

    // remember we are about to
    bool recent_integration = cp->run_project_file_command_needed();
    if (recent_integration)
        cp->run_project_file_command_done();

    //
    // release the locks
    //
    change_cstate_write(cp);
    commit();
    lock_release();

    //
    // run the change file command
    // and the project file command if necessary
    //
    if (wl_nfu.nstrings)
	cp->run_new_file_undo_command(&wl_nfu, up);
    if (wl_ntu.nstrings)
	cp->run_new_file_undo_command(&wl_ntu, up);
    if (wl_rmu.nstrings)
	cp->run_remove_file_undo_command(&wl_rmu, up);
    if (recent_integration)
        cp->run_project_file_command(up);

    //
    // verbose success message
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", wl.string[j]);
	change_verbose(cp, scp, i18n("move file undo $filename complete"));
	sub_context_delete(scp);
    }

    project_free(pp);
    change_free(cp);
    trace(("}\n"));
}


void
move_file_undo(void)
{
    trace(("move_file_undo()\n{\n"));
    switch (arglex())
    {
    default:
	move_file_undo_main();
	break;

    case arglex_token_help:
	move_file_undo_help();
	break;

    case arglex_token_list:
	move_file_undo_list();
	break;
    }
    trace(("}\n"));
}
