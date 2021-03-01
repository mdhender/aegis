//
//	aegis - project change supervisor
//	Copyright (C) 1998-2004 Peter Miller;
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
// MANIFEST: clean a change development directory
//

#include <ac/errno.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/time.h>
#include <ac/unistd.h>

#include <aeclean.h>
#include <ael/change/files.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <change/file.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <glue.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <now.h>
#include <os.h>
#include <progname.h>
#include <project/file.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


static void
clean_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -CLEan [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -CLEan -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -CLEan -Help\n", progname);
    quit(1);
}


static void
clean_help(void)
{
    help("aeclean", clean_usage);
}


static void
clean_list(void)
{
    string_ty	    *project_name;
    long	    change_number;

    trace(("clean_list()\n{\n"));
    project_name = 0;
    change_number = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(clean_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, clean_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, clean_usage);
	    continue;
	}
	arglex();
    }

    list_change_files(project_name, change_number, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


struct clean_info_ty
{
    string_ty	    *dd;
    change_ty	    *cp;
    int		    minimum;
    user_ty	    *up;
    int		    verbose;
    int             touch;
};


static void
clean_out_the_garbage(void *p, dir_walk_message_ty msg, string_ty *path,
    struct stat *st)
{
    clean_info_ty   *sip;
    string_ty	    *s1;
    int		    delete_me;
    int             touch_me;

    sip = (clean_info_ty *)p;
    switch (msg)
    {
    case dir_walk_dir_before:
	break;

    case dir_walk_dir_after:
	//
	// Try to remove each directory.  This makes directories
	// where we deleted everything else go away, which is
	// usually what is desired.  It isn't an error if the
	// rmdir fails because it isn't empty, so quietly ignore
	// those errors.
	//
	// Exception: Don't remove the development directory!
	//
	if (str_equal(path, sip->dd))
	    break;
	if
	(
	    glue_rmdir(path->str_text)
	&&
	    errno != ENOTEMPTY
	&&
	    //
	    // Some brain-dead Unix implementations return
	    // EEXIST when they mean ENOTEMPTY.  Sigh.
	    //
	    errno != EEXIST
	)
	{
	    sub_context_ty  *scp;
	    int             errno_old;

	    errno_old = errno;
	    scp = sub_context_new();
	    sub_errno_setx(scp, errno_old);
	    sub_var_set_string(scp, "File_Name", path);
	    error_intl(scp, i18n("warning: rmdir $filename: $errno"));
	    sub_context_delete(scp);
	}
	break;

    case dir_walk_special:
    case dir_walk_symlink:
	//
	// This can't be a change source file, so always
	// delete it.
	//
	// Taking notice of their delete file preference, of course.
	//
	s1 = os_below_dir(sip->dd, path);
	user_become_undo();
	if (sip->verbose)
	    error_raw("rm %S", s1);
	delete_me = user_delete_file_query(sip->up, s1, 0);
	user_become(sip->up);
	if (delete_me)
	    os_unlink_errok(path);
	str_free(s1);
	break;

    case dir_walk_file:
	//
	// leave difference files alone
	//
	if
	(
	    path->str_length > 2
	&&
	    path->str_text[path->str_length - 2] == ','
	&&
	    path->str_text[path->str_length - 1] == 'D'
	)
	    break;

	//
	// Find the pathname relative to the development directory.
	//
	s1 = os_below_dir(sip->dd, path);
	assert(s1);

	//
	// don't delete change files
	//
	delete_me = 1;
	touch_me = 0;
	user_become_undo();
	if (change_file_find(sip->cp, s1, view_path_first))
	{
	    delete_me = 0;
	    touch_me = sip->touch;
	}

	//
	// The minimum option says to leave regular files
	// which obscure project files (though aecp -ro
	// would have been more appropriate).
	//
	if
	(
	    sip->minimum
	&&
	    project_file_find(sip->cp->pp, s1, view_path_extreme)
	)
	    delete_me = 0;

	//
	// Take notice of their delete file preference.
	//
	if (delete_me)
	{
	    if (sip->verbose)
		error_raw("rm %S", s1);
	    delete_me = user_delete_file_query(sip->up, s1, 0);
	}
	user_become(sip->up);

	//
	// delete the file
	//
	if (delete_me)
	    os_unlink_errok(path);
	else if (touch_me)
	    os_mtime_set_errok(path, now());
	str_free(s1);
	break;
    }
}


static void
clean_main(void)
{
    sub_context_ty  *scp;
    string_ty	    *dd;
    cstate_ty       *cstate_data;
    size_t	    j;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    log_style_ty    log_style;
    user_ty	    *up;
    int		    mergeable_files;
    int		    diffable_files;
    string_list_ty  wl_nf;
    string_list_ty  wl_nt;
    string_list_ty  wl_cp;
    string_list_ty  wl_rm;
    string_list_ty  wl_mt;
    fstate_src_ty   *p_src_data;
    fstate_src_ty   *c_src_data;
    pconf_ty        *pconf_data;
    int		    minimum;
    clean_info_ty   info;
    int             touch;

    trace(("clean_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    log_style = log_style_snuggle_default;
    minimum = 0;
    touch = -1;
    info.verbose = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(clean_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, clean_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, clean_usage);
	    continue;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(clean_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(clean_usage);
	    break;

	case arglex_token_minimum:
	    if (minimum)
		duplicate_option(clean_usage);
	    minimum = 1;
	    break;

	case arglex_token_keep:
	    info.verbose = 1;
	    // fall through...

	case arglex_token_interactive:
	case arglex_token_keep_not:
	    user_delete_file_argument(clean_usage);
	    break;

	case arglex_token_touch:
	    if (touch >= 0)
		duplicate_option(clean_usage);
	    touch = 1;
	    break;

	case arglex_token_touch_not:
	    if (touch >= 0)
		duplicate_option(clean_usage);
	    touch = 0;
	    break;
	}
	arglex();
    }

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
    // lock on the appropriate row of the change table.
    //
    change_cstate_lock_prepare(cp);
    lock_take();

    //
    // It is an error if the change is not in the being_developed state.
    // It is an error if the change is not assigned to the current user.
    //
    cstate_data = change_cstate_get(cp);
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad clean state"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));

    dd = change_development_directory_get(cp, 0);
    os_throttle();

    //
    // open the log file
    //
    log_open(change_logfile_get(cp), up, log_style);

    //
    // look for files which need to be merged
    //
    mergeable_files = 0;
    diffable_files = 0;
    for (j = 0;; ++j)
    {
	string_ty	*path;
	string_ty	*path_d;
	int		exists;
	int		ignore;

	//
	// find the relevant change src data
	//
	c_src_data = change_file_nth(cp, j, view_path_first);
	if (!c_src_data)
	    break;

	//
	// generated files are not merged
	// created or deleted files are not merged
	//
	switch (c_src_data->usage)
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
	switch (c_src_data->action)
	{
	case file_action_create:
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
	p_src_data =
	    project_file_find(pp, c_src_data->file_name, view_path_extreme);
	if (!p_src_data)
	    continue;

	//
	// check for files which need to be merged
	//
	if (!change_file_up_to_date(pp, c_src_data))
	{
	    //
	    // this one needs merging
	    //
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", c_src_data->file_name);
	    change_verbose
	    (
		cp,
		scp,
		i18n("warning: file \"$filename\" needs merge")
	    );
	    sub_context_delete(scp);
	    ++mergeable_files;
	    continue;
	}

	//
	// the removed half of a move is not differenced
	//
	switch (c_src_data->action)
	{
	case file_action_create:
	case file_action_modify:
	    break;

	case file_action_remove:
	    if
	    (
		c_src_data->move
	    &&
		change_file_find(cp, c_src_data->move, view_path_first)
	    )
		continue;
	    break;

	case file_action_insulate:
	case file_action_transparent:
	    break;
	}

	//
	// build various paths
	//
	path = change_file_path(cp, c_src_data->file_name);
	assert(path);
	trace_string(path->str_text);

	//
	// make sure the change sourec file even exists
	//
	user_become(up);
	exists = os_exists(path);
	user_become_undo();
	if (!exists)
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", c_src_data->file_name);
	    change_error(cp, scp, i18n("file \"$filename\" does not exist"));
	    sub_context_delete(scp);
	    str_free(path);
	    ++diffable_files;
	    continue;
	}
	path_d = str_format("%s,D", path->str_text);
	trace_string(path_d->str_text);

	//
	// Check the file's fingerprint.  This will zap
	// the other timestamps if the fingerprint has
	// changed.
	//
	switch (c_src_data->action)
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
	    change_file_fingerprint_check(cp, c_src_data);
	    break;
	}

	//
	// See if we need to diff the file
	//
	user_become(up);
	ignore = change_fingerprint_same(c_src_data->diff_file_fp, path_d, 0);
	user_become_undo();
	if (!ignore)
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", c_src_data->file_name);
	    change_verbose
	    (
		cp,
		scp,
		i18n("warning: file \"$filename\" needs diff")
	    );
	    sub_context_delete(scp);
	    ++diffable_files;
	}
	str_free(path);
	str_free(path_d);
    }
    if (mergeable_files)
    {
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", mergeable_files);
	sub_var_optional(scp, "Number");
	change_verbose(cp, scp, i18n("warning: mergable files"));
	sub_context_delete(scp);
    }
    if (diffable_files)
    {
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", diffable_files);
	sub_var_optional(scp, "Number");
	change_verbose(cp, scp, i18n("warning: diffable files"));
	sub_context_delete(scp);
    }

    //
    // now walk the change directory tree,
    // looking for files to throw away
    //
    info.cp = cp;
    info.minimum = minimum;
    info.touch = (touch != 0);
    info.up = up;
    info.dd = change_development_directory_get(cp, 1);
    user_become(up);
    dir_walk(info.dd, clean_out_the_garbage, &info);
    user_become_undo();

    //
    // create the symbolic links again, if required
    //
    pconf_data = change_pconf_get(cp, 0);
    if
    (
	pconf_data->create_symlinks_before_build
    &&
	!pconf_data->remove_symlinks_after_build
    )
	change_create_symlinks_to_baseline(cp, pp, up, minimum);

    //
    // Re-run the change file command and the project file command,
    // if defined, as these usually manipulate information used by
    // the build tool.
    //
    string_list_constructor(&wl_nf);
    string_list_constructor(&wl_nt);
    string_list_constructor(&wl_cp);
    string_list_constructor(&wl_rm);
    string_list_constructor(&wl_mt);
    for (j = 0;; ++j)
    {
	c_src_data = change_file_nth(cp, j, view_path_first);
	if (!c_src_data)
	    break;
	switch (c_src_data->action)
	{
	case file_action_create:
	    switch (c_src_data->usage)
	    {
	    case file_usage_test:
	    case file_usage_manual_test:
		string_list_append(&wl_nt, c_src_data->file_name);
		break;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_build:
		string_list_append(&wl_nf, c_src_data->file_name);
		break;
	    }
	    break;

	case file_action_modify:
	case file_action_insulate:
	    string_list_append(&wl_cp, c_src_data->file_name);
	    break;

	case file_action_transparent:
	    string_list_append(&wl_mt, c_src_data->file_name);
	    break;

	case file_action_remove:
	    string_list_append(&wl_rm, c_src_data->file_name);
	    break;
	}
    }
    if (wl_nf.nstrings)
	change_run_new_file_command(cp, &wl_nf, up);
    if (wl_nt.nstrings)
	change_run_new_test_command(cp, &wl_nt, up);
    if (wl_cp.nstrings)
	change_run_copy_file_command(cp, &wl_cp, up);
    if (wl_rm.nstrings)
	change_run_remove_file_command(cp, &wl_rm, up);
    if (wl_mt.nstrings)
	change_run_make_transparent_command(cp, &wl_mt, up);
    string_list_destructor(&wl_nf);
    string_list_destructor(&wl_nt);
    string_list_destructor(&wl_cp);
    string_list_destructor(&wl_rm);
    string_list_destructor(&wl_mt);
    cstate_data->project_file_command_sync = 0;
    change_run_project_file_command(cp, up);

    //
    // If the change row (or change file table) changed,
    // write it out.
    // Release advisory lock.
    //
    change_cstate_write(cp);
    commit();
    lock_release();

    //
    // Run the develop begin command, because we have probably
    // removed its side effects.  This is specifically run outside
    // the locks.
    //
    change_run_develop_begin_command(cp, up);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("clean complete"));

    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
clean(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, clean_help, },
	{arglex_token_list, clean_list, },
    };

    trace(("clean()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), clean_main);
    trace(("}\n"));
}
