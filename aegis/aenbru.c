/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to perform new branch undo
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ael/change/by_state.h>
#include <aenbru.h>
#include <arglex2.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <col.h>
#include <commit.h>
#include <dir.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/active.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void new_branch_undo_usage _((void));

static void
new_branch_undo_usage()
{
    char	    *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -New_Branch_Undo <change_number> [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -New_Branch_Undo -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -New_Branch_Undo -Help\n", progname);
    quit(1);
}


static void new_branch_undo_help _((void));

static void
new_branch_undo_help()
{
    help("aenbru", new_branch_undo_usage);
}


static void new_branch_undo_list _((void));

static void
new_branch_undo_list()
{
    string_ty	    *project_name;

    trace(("new_branch_undo_list()\n{\n"));
    project_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_branch_undo_usage);
	    continue;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, new_branch_undo_usage);
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    new_branch_undo_usage
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


static string_ty *branch_changes_path_get _((change_ty *));

static string_ty *
branch_changes_path_get(cp)
    change_ty	    *cp;
{
    project_ty	    *pp;
    string_ty	    *result;

    assert(change_is_a_branch(cp));
    pp = project_bind_branch(cp->pp, change_copy(cp));
    result = str_copy(project_changes_path_get(pp));
    project_free(pp);
    return result;
}


static void new_branch_undo_main _((void));

static void
new_branch_undo_main()
{
    string_ty	    *project_name;
    long	    change_number;
    project_ty	    *pp;
    user_ty	    *up;
    change_ty	    *cp;
    cstate	    cstate_data;
    string_ty	    *dd;
    string_ty	    *s;

    trace(("new_branch_undo_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_branch_undo_usage);
	    continue;

	case arglex_token_keep:
	case arglex_token_interactive:
	case arglex_token_no_keep:
	    user_delete_file_argument(new_branch_undo_usage);
	    break;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, new_branch_undo_usage);
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
	    {
		duplicate_option_by_name
		(
		    arglex_token_change,
		    new_branch_undo_usage
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
		option_needs_name(arglex_token_project, new_branch_undo_usage);
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    new_branch_undo_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(new_branch_undo_usage);
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
    up = user_executing(pp);
    if (!project_administrator_query(pp, user_name(up)))
	project_fatal(pp, 0, i18n("not an administrator"));

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * Make sure we are talking about a branch.
     */
    if (!change_is_a_branch(cp))
	change_fatal(cp, 0, i18n("use aedbu instead"));

    /*
     * Make sure the branch is not active.
     *	    (project_active reports the error itself)
     */
    project_active_check_branch(cp, 0);

    /*
     * Take an advisory write lock on the appropriate row of the change
     * table.  Take an advisory write lock on the appropriate row of the
     * user table.  Block until can get both simultaneously.
     */
    gonzo_gstate_lock_prepare_new();
    project_pstate_lock_prepare(pp);
    change_cstate_lock_prepare(cp);
    user_ustate_lock_prepare(up);
    lock_take();
    cstate_data = change_cstate_get(cp);

    /*
     * Race condition: check that the up is still a project
     * administrator now that we have the project lock.
     */
    if (!project_administrator_query(pp, user_name(up)))
	project_fatal(pp, 0, i18n("not an administrator"));

    /*
     * It is an error if the change is not in the being developed state.
     * It is an error if the change is not assigned to the current user.
     */
    if (cstate_data->state != cstate_state_being_developed)
	change_fatal(cp, 0, i18n("bad dbu state"));

    /*
     * Remove the change from the list of assigned changes in the user
     * change table (in the user row).
     */
    user_own_remove(up, project_name_get(pp), change_number);

    /*
     * remove the development directory
     */
    dd = change_top_path_get(cp, 1);
    if (user_delete_file_query(up, dd, 1))
    {
	change_verbose(cp, 0, i18n("remove development directory"));
	project_become(pp);
	commit_rmdir_tree_errok(dd);
	user_become_undo();
    }

    /*
     * tell the project to forget this change
     */
    project_change_delete(pp, change_number);

    /*
     * delete the change state file of the branch
     * and the state files of all subordinate changes
     */
    project_become(pp);
    commit_unlink_errok(change_cstate_filename_get(cp));
    commit_unlink_errok(change_fstate_filename_get(cp));
    commit_rmdir_tree_errok(branch_changes_path_get(cp));
    project_become_undo();

    /*
     * Remove aliases of this branch.
     *	    (Punctuation?)
     */
    s = str_format("%S.%ld", project_name_get(pp), change_number);
    gonzo_alias_delete(s);
    str_free(s);

    /*
     * Update change table row (and change history table).
     * Update user table row.
     * Release advisory write locks.
     */
    project_pstate_write(pp);
    gonzo_gstate_write();
    commit();
    lock_release();

    /*
     * verbose success message
     */
    change_verbose(cp, 0, i18n("new branch undo complete"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
new_branch_undo()
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, new_branch_undo_help, },
	{arglex_token_list, new_branch_undo_list, },
    };

    trace(("new_branch_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_branch_undo_main);
    trace(("}\n"));
}
