/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to implement the 'aegis -Change_Owner' command
 */

#include <ac/stdio.h>
#include <ac/string.h>
#include <ac/libintl.h>

#include <aechown.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <error.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <commit.h>
#include <cstate.h>
#include <file.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/history.h>
#include <pstate.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void
change_owner_usage(void)
{
    char	    *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -Change_Owner [ <option>... ]\n", progname);
    fprintf
    (
	stderr,
	"       %s -Change_Owner -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Change_Owner -Help\n", progname);
    quit(1);
}


static void
change_owner_help(void)
{
    help("aechown", change_owner_usage);
}


static void
change_owner_list(void)
{
    string_ty	    *project_name;

    trace(("change_owner_list()\n{\n"));
    project_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(change_owner_usage);
	    continue;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, change_owner_usage);
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    change_owner_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;
	}
	arglex();
    }
    list_changes_in_state_mask(project_name, 1 << cstate_state_being_developed);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
change_owner_main(void)
{
    sub_context_ty  *scp;
    string_ty	    *project_name;
    long	    change_number;
    project_ty	    *pp;
    user_ty	    *up;
    user_ty	    *up1;
    user_ty	    *up2;
    change_ty	    *cp;
    cstate	    cstate_data;
    cstate_history  history_data;
    string_ty	    *new_developer;
    string_ty	    *devdir;
    string_ty	    *old_dd;
    size_t	    j;
    pconf	    pconf_data;

    trace(("change_owner_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    new_developer = 0;
    devdir = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(change_owner_usage);
	    continue;

	case arglex_token_keep:
	case arglex_token_interactive:
	case arglex_token_keep_not:
	    user_delete_file_argument(change_owner_usage);
	    break;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, change_owner_usage);
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
	    {
		duplicate_option_by_name
		(
		    arglex_token_change,
		    change_owner_usage
		);
	    }
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_project:
	    if (project_name)
		duplicate_option(change_owner_usage);
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, change_owner_usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_directory:
	    if (devdir)
		duplicate_option(change_owner_usage);
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, change_owner_usage);
	    devdir = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_user:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_user, change_owner_usage);
	    /* fall through... */

	case arglex_token_string:
	    if (new_developer)
		duplicate_option_by_name(arglex_token_user, change_owner_usage);
	    new_developer = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(change_owner_usage);
	    break;
	}
	arglex();
    }
    if (!change_number)
	fatal_intl(0, i18n("no change number"));
    if (!new_developer)
	fatal_intl(0, i18n("no user name"));

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * it is an error if the named user is not a developer
     */
    if (!project_developer_query(pp, new_developer))
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "Target", new_developer);
	project_fatal(pp, scp, i18n("$target not developer"));
	/* NOTREACHED */
	sub_context_delete(scp);
    }

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * locate change data
     */
    assert(change_number);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * It is an error if the change is not in the 'being developed' state
     */
    cstate_data = change_cstate_get(cp);
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad chown state"));

    /*
     * Get details of the two users involved.
     */
    up1 = user_symbolic(pp, change_developer_name(cp));
    trace(("up1 = %08lX\n", (long)up1));
    up2 = user_symbolic(pp, new_developer);
    trace(("up2 = %08lX\n", (long)up2));

    /*
     * It is an error if the executing user is not a project
     * administrator.  However, developers are allowed to chown it
     * to themselves, because this is a common way of moving the
     * development directory of a change.
     */
    if (str_equal(change_developer_name(cp), new_developer))
	change_verbose(cp, 0, i18n("warning: no chown"));
    else if (!project_administrator_query(pp, user_name(up)))
	project_fatal(pp, 0, i18n("not an administrator"));

    /*
     * Take an advisory write lock on the appropriate row of the change
     * table.  Take an advisory write lock on the appropriate row of the
     * user table.  Block until can get both simultaneously.
     */
    project_pstate_lock_prepare(pp);
    change_cstate_lock_prepare(cp);
    user_ustate_lock_prepare(up1);
    user_ustate_lock_prepare(up2);
    lock_take();
    cstate_data = change_cstate_get(cp);

    /*
     * These could have changed, check again:
     * It is an error if the change is not in the being developed state.
     * It is an error if the change is already being developed by the
     * named user.
     */
    if (!str_equal(change_developer_name(cp), user_name(up1)))
	change_fatal(cp, 0, i18n("sync error, try again"));
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("no branch chown"));

    /*
     * add to history for state change
     */
    history_data = change_history_new(cp, up1);
    history_data->what = cstate_history_what_develop_begin_undo;
    history_data->why =
	str_format("Forced by administrator \"%S\".", user_name(up));
    history_data = change_history_new(cp, up2);
    history_data->what = cstate_history_what_develop_begin;
    history_data->why =
	str_format("Forced by administrator \"%S\".", user_name(up));

    /*
     * Clear the build-time field.
     * Clear the test-time field.
     * Clear the test-baseline-time field.
     * Clear the src field.
     */
    change_build_times_clear(cp);

    /*
     * Remove the change from the list of assigned changes in the user
     * change table (in the user row).
     */
    user_own_remove(up1, project_name_get(pp), change_number);
    user_own_add(up2, project_name_get(pp), change_number);

    /*
     * Create the change directory.
     */
    if (!devdir)
    {
	scp = sub_context_new();
	devdir = change_development_directory_template(cp, up2);
	sub_var_set_string(scp, "File_Name", devdir);
	change_verbose(cp, scp, i18n("development directory \"$filename\""));
	sub_context_delete(scp);
    }
    assert(cstate_data->development_directory);
    old_dd = cstate_data->development_directory;
    cstate_data->development_directory = devdir;

    /*
     * Create the development directory.
     */
    user_become(up2);
    os_mkdir(devdir, 02755);
    undo_rmdir_errok(devdir);
    user_become_undo();

    /*
     * Make sure fstate read in so that it does not do so during
     * the loop (otherwise multiple user permissions set).
     */
    change_file_nth(cp, 0);

    /*
     * copy change files across
     *	    (even the removed files)
     */
    change_verbose(cp, 0, i18n("copy change source files"));
    user_become(up2);
    for (j = 0;; ++j)
    {
	string_ty	*s1;
	fstate_src	src_data;

	/*
	 * copy the file across
	 */
	src_data = change_file_nth(cp, j);
	if (!src_data)
	    break;
	s1 = str_format("%S/%S", old_dd, src_data->file_name);
	if (os_exists(s1))
	{
	    string_ty	    *s2;

	    s2 = str_format("%S/%S", devdir, src_data->file_name);
	    os_mkdir_between(devdir, src_data->file_name, 02755);
	    copy_whole_file(s1, s2, 0);
	    str_free(s2);
	}
	str_free(s1);

	/*
	 * clear the file time stamps
	 */
	if (src_data->file_fp)
	{
	    fingerprint_type.free(src_data->file_fp);
	    src_data->file_fp = 0;
	}
	if (src_data->diff_file_fp)
	{
	    fingerprint_type.free(src_data->diff_file_fp);
	    src_data->diff_file_fp = 0;
	}
	if (src_data->architecture_times)
	{
	    fstate_src_architecture_times_list_type.free
	    (
		src_data->architecture_times
	    );
	    src_data->architecture_times = 0;
	}
    }
    user_become_undo();

    /*
     * remove the old development directory
     */
    if (user_delete_file_query(up, old_dd, 1))
    {
	change_verbose(cp, 0, i18n("remove old development directory"));
	user_become(up1);
	commit_rmdir_tree_errok(old_dd);
	user_become_undo();
    }
    str_free(old_dd);

    /*
     * Write the change table row.
     * Write the user table rows.
     * Release advisory locks.
     */
    change_cstate_write(cp);
    project_pstate_write(pp);
    user_ustate_write(up1);
    user_ustate_write(up2);
    commit();
    lock_release();

    /*
     * run the notification commands
     */
    change_run_develop_begin_undo_command(cp, up1);
    change_run_develop_begin_command(cp, up2);
    change_run_forced_develop_begin_notify_command(cp, up);

    /*
     * if symlinks are being used to pander to dumb DMT,
     * and they are not removed after each build,
     * create them now, rather than waiting for the first build.
     * This will present a more uniform interface to the developer.
     */
    pconf_data = change_pconf_get(cp, 0);
    if
    (
	pconf_data->create_symlinks_before_build
    &&
	!pconf_data->remove_symlinks_after_build
    )
	change_create_symlinks_to_baseline(cp, pp, up2, 0);

    /*
     * verbose success message
     */
    scp = sub_context_new();
    sub_var_set_string(scp, "ORiginal", user_name(up1));
    sub_var_optional(scp, "ORiginal");
    sub_var_set_string(scp, "Target", user_name(up2));
    sub_var_optional(scp, "Target");
    change_verbose(cp, scp, i18n("chown complete"));
    sub_context_delete(scp);

    /*
     * clean up and go home
     */
    change_free(cp);
    project_free(pp);
    user_free(up);
    user_free(up1);
    user_free(up2);
    trace(("}\n"));
}


void
change_owner(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, change_owner_help, },
	{arglex_token_list, change_owner_list, },
    };

    trace(("change_owner()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), change_owner_main);
    trace(("}\n"));
}
