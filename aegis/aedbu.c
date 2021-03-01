/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to implement develop begin undo
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aedbu.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <dir.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void
develop_begin_undo_usage(void)
{
    char	    *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Develop_Begin_Undo <change_number> [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Develop_Begin_Undo -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Develop_Begin_Undo -Help\n", progname);
    quit(1);
}


static void
develop_begin_undo_help(void)
{
    help("aedbu", develop_begin_undo_usage);
}


static void
develop_begin_undo_list(void)
{
    string_ty	    *project_name;

    trace(("develop_begin_undo_list()\n{\n"));
    project_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(develop_begin_undo_usage);
	    continue;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
	    {
		option_needs_name
		(
		    arglex_token_project,
		    develop_begin_undo_usage
		);
	    }
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    develop_begin_undo_usage
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
develop_begin_undo_main(void)
{
    string_ty	    *project_name;
    long	    change_number;
    project_ty	    *pp;
    user_ty	    *admin_up;
    user_ty	    *up;
    change_ty	    *cp;
    cstate	    cstate_data;
    cstate_history  history_data;
    string_ty	    *dd;
    string_ty	    *usr_name;

    trace(("develop_begin_undo_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    usr_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(develop_begin_undo_usage);
	    continue;

	case arglex_token_keep:
	case arglex_token_interactive:
	case arglex_token_keep_not:
	    user_delete_file_argument(develop_begin_undo_usage);
	    break;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
	    {
		option_needs_number
		(
		    arglex_token_change,
		    develop_begin_undo_usage
		);
	    }
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
	    {
		duplicate_option_by_name
		(
		    arglex_token_change,
		    develop_begin_undo_usage
		);
	    }
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
	    {
		option_needs_name
		(
		    arglex_token_project,
		    develop_begin_undo_usage
		);
	    }
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    develop_begin_undo_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_user:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_user, develop_begin_undo_usage);
	    if (usr_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_user,
		    develop_begin_undo_usage
		);
	    }
	    usr_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(develop_begin_undo_usage);
	    break;
	}
	arglex();
    }

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
    if (usr_name)
    {
	admin_up = user_executing(pp);
	up = user_symbolic(pp, usr_name);
	if (str_equal(user_name(up), user_name(admin_up)))
	{
	    /*
	     * If the user specified themselves, silently
	     * ignore such sillyness.
	     */
	    user_free(up);
	    up = admin_up;
	    admin_up = 0;
	}

	/*
	 * If a user name was specified on the command line,
	 * check that the executing user is a project
	 * administrator.
	 */
	if (admin_up && !project_administrator_query(pp, user_name(admin_up)))
	    project_fatal(pp, 0, i18n("not an administrator"));
    }
    else
    {
	admin_up = 0;
	up = user_executing(pp);
    }

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * Make sure it is a change we are manipulating, not a branch.
     */
    if (change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("use aenbru instead"));

    /*
     * Take an advisory write lock on the appropriate row of the change
     * table.  Take an advisory write lock on the appropriate row of the
     * user table.  Block until can get both simultaneously.
     */
    project_pstate_lock_prepare(pp);
    change_cstate_lock_prepare(cp);
    user_ustate_lock_prepare(up);
    lock_take();
    cstate_data = change_cstate_get(cp);

    /*
     * Race condition: check that the admin_up is still a project
     * administrator now that we have the project lock.
     */
    if (admin_up && !project_administrator_query(pp, user_name(admin_up)))
	project_fatal(pp, 0, i18n("not an administrator"));

    /*
     * It is an error if the change is not in the being developed state.
     * It is an error if the change is not assigned to the current user.
     */
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad dbu state"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("not developer"));

    /*
     * It is an error if an administrator is nuking the change and
     * the change has any files.  Mostly, because it is expected
     * that this is used to undo forced aedb's.
     */
    if (admin_up && change_file_nth(cp, 0L))
	change_fatal(cp, 0, i18n("aedbu, has files"));

    /*
     * add to history for state change
     */
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_develop_begin_undo;
    if (admin_up)
    {
	history_data->why =
	    str_format("Forced by administrator \"%S\".", user_name(admin_up));
    }

    /*
     * Send the change to the awaiting-development state.
     * Clear the build-time field.
     * Clear the test-time field.
     * Clear the test-baseline-time field.
     * Clear the src field.
     */
    cstate_data->state = cstate_state_awaiting_development;
    change_build_times_clear(cp);
    change_file_remove_all(cp);

    /*
     * Remove the change from the list of assigned changes in the user
     * change table (in the user row).
     */
    user_own_remove(up, project_name_get(pp), change_number);

    /*
     * remove the development directory
     */
    dd = change_development_directory_get(cp, 1);
    if (user_delete_file_query(up, dd, 1))
    {
	change_verbose(cp, 0, i18n("remove development directory"));
	user_become(up);
	commit_rmdir_tree_errok(dd);
	user_become_undo();
    }

    /*
     * clear development directory field
     */
    change_development_directory_clear(cp);

    /*
     * Write the change table row.
     * Write the user table row.
     * Release advisory locks.
     */
    change_cstate_write(cp);
    project_pstate_write(pp);
    user_ustate_write(up);
    commit();
    lock_release();

    /*
     * run the notification command
     */
    change_run_develop_begin_undo_command(cp, up);

    /*
     * verbose success message
     */
    change_verbose(cp, 0, i18n("develop begin undo complete"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    if (admin_up)
	user_free(admin_up);
    trace(("}\n"));
}


void
develop_begin_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, develop_begin_undo_help, },
	{arglex_token_list, develop_begin_undo_list, },
    };

    trace(("develop_begin_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), develop_begin_undo_main);
    trace(("}\n"));
}
