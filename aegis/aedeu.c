/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to implement develop end undo
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <aedeu.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <change.h>
#include <change/branch.h>
#include <change/develop_direct/read_write.h>
#include <change/file.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <progname.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void
develop_end_undo_usage(void)
{
    char	    *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -Undo_Develop_End [ <option>... ]\n", progname);
    fprintf
    (
	stderr,
	"       %s -Undo_Develop_End -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Undo_Develop_End -Help\n", progname);
    quit(1);
}


static void
develop_end_undo_help(void)
{
    help("aedeu", develop_end_undo_usage);
}


static void
develop_end_undo_list(void)
{
    string_ty	    *project_name;

    trace(("develop_end_undo_list()\n{\n"));
    project_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(develop_end_undo_usage);
	    continue;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, develop_end_undo_usage);
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    develop_end_undo_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;
	}
	arglex();
    }
    list_changes_in_state_mask
    (
	project_name,
	(
	    (1 << cstate_state_awaiting_review)
	|
	    (1 << cstate_state_being_reviewed)
	|
	    (1 << cstate_state_awaiting_integration)
	)
    );
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
develop_end_undo_main(void)
{
    cstate	    cstate_data;
    cstate_history  history_data;
    int		    j;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    user_ty	    *up_admin;

    trace(("develop_end_undo_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(develop_end_undo_usage);
	    continue;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
	    {
		option_needs_number
		(
		    arglex_token_change,
		    develop_end_undo_usage
		);
	    }
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
	    {
		duplicate_option_by_name
		(
		    arglex_token_change,
		    develop_end_undo_usage
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
		option_needs_name(arglex_token_project, develop_end_undo_usage);
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    develop_end_undo_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(develop_end_undo_usage);
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
    up_admin = 0;

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * lock the change for writing
     */
    project_pstate_lock_prepare(pp);
    change_cstate_lock_prepare(cp);
    user_ustate_lock_prepare(up);
    lock_take();
    cstate_data = change_cstate_get(cp);

    /*
     * Project administrators are allowed to undo end the development
     * of a branch, no matter who created it.
     */
    if
    (
	change_was_a_branch(cp)
    &&
	!str_equal(change_developer_name(cp), user_name(up))
    &&
	project_administrator_query(pp, user_name(up))
    )
    {
	up_admin = up;
	up = user_symbolic(pp, change_developer_name(cp));
    }

    /*
     * It is an error if the change is not in one of the 'being_reviewed'
     * or 'awaiting_integration' states.
     * It is an error if the current user did not develop the change.
     */
    if
    (
	cstate_data->state != cstate_state_awaiting_review
    &&
	cstate_data->state != cstate_state_being_reviewed
    &&
	cstate_data->state != cstate_state_awaiting_integration
    )
	change_fatal(cp, 0, i18n("bad deu state"));
    if (!str_equal(change_developer_name(cp), user_name(up)))
	change_fatal(cp, 0, i18n("was not developer"));

    /*
     * Change the state.
     * Add to the change's history.
     */
    cstate_data->state = cstate_state_being_developed;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_develop_end_undo;
    if (up_admin)
    {
	history_data->why =
	    str_format("Forced by administrator \"%S\".", user_name(up_admin));
    }

    /*
     * add it back into the user's change list
     */
    user_own_add(up, project_name_get(pp), change_number);

    /*
     * go through the files in the change and unlock them
     * in the baseline
     */
    for (j = 0;; ++j)
    {
	fstate_src	c_src_data;
	fstate_src	p_src_data;

	c_src_data = change_file_nth(cp, j);
	if (!c_src_data)
	    break;
	p_src_data =
	    project_file_find(pp, c_src_data->file_name, view_path_none);
	if (!p_src_data)
	{
	    /* this is really a corrupted file */
	    continue;
	}
	p_src_data->locked_by = 0;

	/*
	 * Remove the file if it is about_to_be_created
	 * by the change we are rescinding.
	 */
	if
	(
	    p_src_data->about_to_be_created_by
	||
	    p_src_data->about_to_be_copied_by
	)
	{
	    assert(!p_src_data->about_to_be_created_by ||
		p_src_data->about_to_be_created_by==change_number);
	    assert(!p_src_data->about_to_be_copied_by ||
		p_src_data->about_to_be_copied_by==change_number);
	    if (p_src_data->deleted_by)
	    {
		assert(!p_src_data->about_to_be_copied_by);
		p_src_data->about_to_be_created_by = 0;
	    }
	    else
		project_file_remove(pp, c_src_data->file_name);
	}
    }

    /*
     * Make the development directory writable again.
     */
    if
    (
	!change_was_a_branch(cp)
    &&
	project_protect_development_directory_get(pp)
    )
	change_development_directory_chmod_read_write(cp);

    /*
     * write out the data and release the locks
     */
    change_cstate_write(cp);
    project_pstate_write(pp);
    user_ustate_write(up);
    commit();
    lock_release();

    /*
     * run the notify command
     */
    change_run_develop_end_undo_notify_command(cp);

    /*
     * verbose success message
     */
    change_verbose(cp, 0, i18n("develop end undo complete"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
develop_end_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, develop_end_undo_help, },
	{arglex_token_list, develop_end_undo_list, },
    };

    trace(("develop_end_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), develop_end_undo_main);
    trace(("}\n"));
}
