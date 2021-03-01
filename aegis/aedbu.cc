//
//      aegis - project change supervisor
//      Copyright (C) 1991-1999, 2001-2007 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/common.h>
#include <libaegis/dir.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aedbu.h>


static void
develop_begin_undo_usage(void)
{
    const char      *progname;

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
    string_ty       *project_name;

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
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, develop_begin_undo_usage);
            continue;
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
    string_ty       *project_name;
    long            change_number;
    project_ty      *pp;
    user_ty::pointer admin_up;
    user_ty::pointer up;
    change::pointer cp;
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    string_ty       *usr_name;

    trace(("develop_begin_undo_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    usr_name = 0;
    string_ty *reason = 0;
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
            user_ty::delete_file_argument(develop_begin_undo_usage);
            break;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                develop_begin_undo_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, develop_begin_undo_usage);
            continue;

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
            user_ty::lock_wait_argument(develop_begin_undo_usage);
            break;

	case arglex_token_reason:
	    if (reason)
	    duplicate_option(develop_begin_undo_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_string
		(
	    	    arglex_token_reason,
	    	    develop_begin_undo_usage
		);
		// NOTREACHED

	    case arglex_token_string:
	    case arglex_token_number:
		reason = str_from_c(arglex_value.alv_string);
		break;
	    }
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
    if (usr_name)
    {
        admin_up = user_ty::create();
        up = user_ty::create(nstring(usr_name));
        if (up->name() == admin_up->name())
        {
            //
            // If the user specified themselves, silently
            // ignore such sillyness.
            //
            up = admin_up;
            admin_up.reset();
        }

        //
        // If a user name was specified on the command line,
        // check that the executing user is a project
        // administrator.
        //
        if (admin_up && !project_administrator_query(pp, admin_up->name()))
            project_fatal(pp, 0, i18n("not an administrator"));
    }
    else
    {
        admin_up.reset();
        up = user_ty::create();
    }

    //
    // locate change data
    //
    if (!change_number)
        change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Make sure it is a change we are manipulating, not a branch.
    //
    if (change_is_a_branch(cp))
        change_fatal(cp, 0, i18n("use aenbru instead"));

    //
    // Take an advisory write lock on the appropriate row of the change
    // table.  Take an advisory write lock on the appropriate row of the
    // user table.  Block until can get both simultaneously.
    //
    pp->pstate_lock_prepare();
    change_cstate_lock_prepare(cp);
    up->ustate_lock_prepare();
    lock_take();
    cstate_data = cp->cstate_get();

    //
    // Race condition: check that the admin_up is still a project
    // administrator now that we have the project lock.
    //
    if
    (
        admin_up
    &&
        !project_administrator_query(pp, admin_up->name())
    )
        project_fatal(pp, 0, i18n("not an administrator"));

    //
    // It is an error if the change is not in the being developed state.
    // It is an error if the change is not assigned to the current user.
    //
    if (cstate_data->state != cstate_state_being_developed)
        change_fatal(cp, 0, i18n("bad dbu state"));
    if (nstring(change_developer_name(cp)) != up->name())
        change_fatal(cp, 0, i18n("not developer"));

    //
    // It is an error if an administrator is nuking the change and
    // the change has any files.  Mostly, because it is expected
    // that this is used to undo forced aedb's.
    //
    if (admin_up && change_file_nth(cp, 0L, view_path_first))
        change_fatal(cp, 0, i18n("aedbu, has files"));

    //
    // add to history for state change
    //
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_develop_begin_undo;
    if (admin_up)
    {
        string_ty *r2 =
            str_format
            (
                "Forced by administrator %s.",
                admin_up->name().quote_c().c_str()
            );
	if (reason)
	{
	    string_ty *r1 = reason;
	    reason = str_format("%s\n%s", r1->str_text, r2->str_text);
	    str_free(r1);
	    str_free(r2);
	}
	else
	    reason = r2;
    }
    history_data->why = reason;

    //
    // Send the change to the awaiting-development state.
    // Clear the build-time field.
    // Clear the test-time field.
    // Clear the test-baseline-time field.
    // Clear the src field.
    //
    cstate_data->state = cstate_state_awaiting_development;
    change_build_times_clear(cp);
    change_file_remove_all(cp);

    //
    // Remove the change from the list of assigned changes in the user
    // change table (in the user row).
    //
    up->own_remove(pp, change_number);

    //
    // remove the development directory
    //
    nstring dd(change_development_directory_get(cp, 1));
    if (up->delete_file_query(dd, true, -1))
    {
        change_verbose(cp, 0, i18n("remove development directory"));
        user_ty::become scoped(up);
        commit_rmdir_tree_errok(dd);
    }

    //
    // clear development directory field
    //
    change_development_directory_clear(cp);

    //
    // Write the change table row.
    // Write the user table row.
    // Release advisory locks.
    //
    change_cstate_write(cp);
    pp->pstate_write();
    up->ustate_write();
    commit();
    lock_release();

    //
    // run the notification command
    //
    change_run_develop_begin_undo_command(cp, up);

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("develop begin undo complete"));
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
develop_begin_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, develop_begin_undo_help, 0 },
        { arglex_token_list, develop_begin_undo_list, 0 },
    };

    trace(("develop_begin_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), develop_begin_undo_main);
    trace(("}\n"));
}
