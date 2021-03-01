//
//	aegis - project change supervisor
//	Copyright (C) 1998-2008 Peter Miller
//	Copyright (C) 2006 Walter Franzini;
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

#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/time.h>
#include <common/ac/unistd.h>

#include <aegis/aeclean.h>
#include <common/error.h>
#include <common/gmatch.h>
#include <common/now.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/files.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/glue.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


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
    trace(("clean_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(clean_usage);
    list_change_files(cid, 0);
    trace(("}\n"));
}


struct clean_info_ty
{
    string_ty       *dd;
    change::pointer cp;
    bool            minimum;
    user_ty::pointer up;
    bool            verbose;
    bool            touch;
    const work_area_style_ty *style;
    pconf_clean_exceptions_list_ty  *exceptions;

    clean_info_ty() :
	dd(0),
        cp(0),
        minimum(false),
	verbose(false),
        touch(false),
        style(0)
    {
    }
};


static void
clean_out_the_garbage(void *p, dir_walk_message_ty msg, string_ty *path,
    const struct stat *)
{
    clean_info_ty   *sip;
    string_ty	    *s1;
    bool            delete_me;
    bool            touch_me;

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
	// Find the pathname relative to the development directory.
	//
	s1 = os_below_dir(sip->dd, path);

	//
        // Try not to delete anything the user's development directory
        // style wants to keep around.
	//
	delete_me = true;
	sip->up->become_end();
	if
	(
	    !sip->style->during_build_only
	&&
	    sip->style->source_file_symlink
	&&
	    project_file_find(sip->cp->pp, s1, view_path_extreme)
	)
	{
	    delete_me = false;
	}

	//
	// This can't be a change source file, so always
	// delete it.
	//
	// Taking notice of their delete file preference, of course.
	//
	if (sip->verbose)
	    error_raw("rm %S", s1);
	if (delete_me)
	    delete_me = sip->up->delete_file_query(nstring(s1), false, true);
	sip->up->become_begin();
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
	&&
	    change_diff_required(sip->cp)
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
	delete_me = true;
	touch_me = false;
	sip->up->become_end();
	if (change_file_find(sip->cp, s1, view_path_first))
	{
	    delete_me = false;
	    touch_me = sip->touch;
	}

	//
	// Take note of the development directory style.
	//
	if
	(
	    delete_me
	&&
	    sip->style->during_build_only
	&&
	    (
		sip->style->source_file_link
	    ||
		sip->style->source_file_copy
	    )
	&&
	    project_file_find(sip->cp->pp, s1, view_path_extreme)
	)
	{
	    delete_me = false;
	    touch_me = false;
	}

	//
	// The minimum option says to leave regular files
	// which obscure project files (though aecp -ro
	// would have been more appropriate).
	//
	if
	(
	    delete_me
	&&
	    sip->minimum
	&&
	    project_file_find(sip->cp->pp, s1, view_path_extreme)
	)
	    delete_me = false;

	//
	// Take into account clean_exceptions;
	//
	if (delete_me && sip->exceptions)
	{
	    for (size_t i = 0; i < sip->exceptions->length; ++i)
	    {
		if (!gmatch(sip->exceptions->list[i]->str_text,
			    path->str_text))
		    continue;

		delete_me = false;
		break;
	    }
	}

	//
	// Take notice of their delete file preference.
	//
	if (delete_me)
	{
	    if (sip->verbose)
		error_raw("rm %S", s1);
	    delete_me = sip->up->delete_file_query(nstring(s1), false, true);
	}
	sip->up->become_begin();

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
    cstate_ty       *cstate_data;
    size_t	    j;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change::pointer cp;
    log_style_ty    log_style;
    user_ty::pointer up;
    int		    mergeable_files;
    int		    diffable_files;
    fstate_src_ty   *p_src_data;
    fstate_src_ty   *c_src_data;
    clean_info_ty   info;
    int             touch;

    trace(("clean_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    log_style = log_style_snuggle_default;
    bool minimum = false;
    touch = -1;
    info.verbose = false;
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
	    user_ty::lock_wait_argument(clean_usage);
	    break;

	case arglex_token_minimum:
	    if (minimum)
		duplicate_option(clean_usage);
	    minimum = true;
	    break;

	case arglex_token_keep:
	    info.verbose = true;
	    // fall through...

	case arglex_token_interactive:
	case arglex_token_keep_not:
	    user_ty::delete_file_argument(clean_usage);
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
    // lock on the appropriate row of the change table.
    //
    change_cstate_lock_prepare(cp);
    lock_take();

    //
    // It is an error if the change is not in the being_developed state.
    // It is an error if the change is not assigned to the current user.
    //
    cstate_data = cp->cstate_get();
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad clean state"));
    if (nstring(change_developer_name(cp)) != up->name())
	change_fatal(cp, 0, i18n("not developer"));

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
	// make sure the change source file even exists
	//
	up->become_begin();
	exists = os_exists(path);
	up->become_end();
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
	if (change_diff_required(cp))
	{
	    path_d = str_format("%s,D", path->str_text);
	    trace_string(path_d->str_text);
	    up->become_begin();
	    ignore =
		change_fingerprint_same(c_src_data->diff_file_fp, path_d, 0);
	    up->become_end();
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
	    str_free(path_d);
	}
	str_free(path);
    }
    if (mergeable_files)
    {
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", mergeable_files);
	sub_var_optional(scp, "Number");
	change_verbose(cp, scp, i18n("warning: mergable files"));
	sub_context_delete(scp);
    }
    if (diffable_files && change_diff_required(cp))
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
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data->development_directory_style);
    work_area_style_ty style = *pconf_data->development_directory_style;
    if (minimum)
    {
	style.derived_file_link = false;
	style.derived_file_symlink = false;
	style.derived_file_copy = false;
    }
    info.cp = cp;
    info.minimum = minimum;
    info.touch = (touch != 0);
    info.up = up;
    info.dd = change_development_directory_get(cp, 1);
    info.style = &style;
    info.exceptions = pconf_data->clean_exceptions;
    up->become_begin();
    dir_walk(info.dd, clean_out_the_garbage, &info);
    up->become_end();

    //
    // Nuke the build time stamps.  This lets aepromptcmd and aefinish
    // operate correctly, and it surprizes users less, too.
    //
    if (change_build_required(cp))
    {
	change_build_times_clear(cp);
	change_test_times_clear(cp);
    }

    //
    // create the symbolic links again, if required
    //
    if (!pconf_data->development_directory_style->during_build_only)
    {
	change_create_symlinks_to_baseline(cp, up, style);
    }

    //
    // Re-run the change file command and the project file command,
    // if defined, as these usually manipulate information used by
    // the build tool.
    //
    string_list_ty wl_nf;
    string_list_ty wl_nt;
    string_list_ty wl_cp;
    string_list_ty wl_rm;
    string_list_ty wl_mt;
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
		wl_nt.push_back(c_src_data->file_name);
		break;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_build:
		wl_nf.push_back(c_src_data->file_name);
		break;
	    }
	    break;

	case file_action_modify:
	case file_action_insulate:
	    wl_cp.push_back(c_src_data->file_name);
	    break;

	case file_action_transparent:
	    wl_mt.push_back(c_src_data->file_name);
	    break;

	case file_action_remove:
	    wl_rm.push_back(c_src_data->file_name);
	    break;
	}
    }

    // remember that we are about to do it
    cp->run_project_file_command_done();

    //
    // If the change row (or change file table) changed,
    // write it out.
    // Release advisory lock.
    //
    change_cstate_write(cp);
    commit();
    lock_release();

    if (wl_nf.nstrings)
	cp->run_new_file_command(&wl_nf, up);
    if (wl_nt.nstrings)
	cp->run_new_test_command(&wl_nt, up);
    if (wl_cp.nstrings)
	cp->run_copy_file_command(&wl_cp, up);
    if (wl_rm.nstrings)
	cp->run_remove_file_command(&wl_rm, up);
    if (wl_mt.nstrings)
	cp->run_make_transparent_command(&wl_mt, up);
    cp->run_project_file_command(up);

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
    trace(("}\n"));
}


void
clean(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, clean_help, 0 },
	{ arglex_token_list, clean_list, 0 },
    };

    trace(("clean()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), clean_main);
    trace(("}\n"));
}
