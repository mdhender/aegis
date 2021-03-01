//
//      aegis - project change supervisor
//      Copyright (C) 1991-1999, 2001-2006 Peter Miller;
//      All rights reserved.
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
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions for implementing integrate begin undo
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>

#include <aegis/aeibu.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/commit.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <common/progname.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <common/quit.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


static void
integrate_begin_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Integrate_Begin_Undo [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Integrate_Begin_Undo -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Integrate_Begin_Undo -Help\n", progname);
    quit(1);
}


static void
integrate_begin_undo_help(void)
{
    help("aeibu", integrate_begin_undo_usage);
}


static void
integrate_begin_undo_list(void)
{
    string_ty       *project_name;

    trace(("integrate_begin_undo_list()\n{\n"));
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(integrate_begin_undo_usage);
            continue;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, integrate_begin_undo_usage);
            continue;
        }
        arglex();
    }
    list_changes_in_state_mask
    (
        project_name,
        1 << cstate_state_being_integrated
    );
    if (project_name)
        str_free(project_name);
    trace(("}\n"));
}


static void
integrate_begin_undo_main(void)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    string_ty       *dir;
    string_ty       *project_name;
    project_ty      *pp;
    long            change_number;
    change_ty       *cp;
    user_ty         *up;
    size_t          j;

    trace(("integrate_begin_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(integrate_begin_undo_usage);
            continue;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                integrate_begin_undo_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, integrate_begin_undo_usage);
            continue;

        case arglex_token_keep:
        case arglex_token_interactive:
        case arglex_token_keep_not:
            user_delete_file_argument(integrate_begin_undo_usage);
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_lock_wait_argument(integrate_begin_undo_usage);
            break;

	case arglex_token_reason:
	    if (reason)
		duplicate_option(integrate_begin_undo_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_string
		(
	    	    arglex_token_reason,
	    	    integrate_begin_undo_usage
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
        project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

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
    // lock the change for writing
    //
    pp->pstate_lock_prepare();
    change_cstate_lock_prepare(cp);
    user_ustate_lock_prepare(up);
    lock_take();
    cstate_data = change_cstate_get(cp);

    //
    // it is an error if the change is not in the 'being_integrated' state.
    // it is an error if user is not the integrator (or proj admin)
    //
    if (cstate_data->state != cstate_state_being_integrated)
        change_fatal(cp, 0, i18n("bad ibu state"));
    if
    (
        !str_equal(change_integrator_name(cp), user_name(up))
    &&
        !project_administrator_query(pp, user_name(up))
    )
        change_fatal(cp, 0, i18n("not integrator"));

    //
    // Change the state.
    // Add to the change's history.
    //
    cstate_data->state = cstate_state_awaiting_integration;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_integrate_begin_undo;
    history_data->why = reason;

    //
    // clear the idiff times
    //
    for (j = 0;; ++j)
    {
        fstate_src_ty   *src_data;

        src_data = change_file_nth(cp, j, view_path_first);
        if (!src_data)
            break;
        if (src_data->idiff_file_fp)
        {
            fingerprint_type.free(src_data->idiff_file_fp);
            src_data->idiff_file_fp = 0;
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

    //
    // remove it from the user's change list
    //
    user_own_remove(up, project_name_get(pp), change_number);

    //
    // Note that the project has no current integration
    //
    project_current_integration_set(pp, 0);
    dir = str_copy(change_integration_directory_get(cp, 1));
    change_integration_directory_clear(cp);
    change_build_times_clear(cp);
    cstate_data->delta_number = 0;
    if (cstate_data->delta_uuid)
    {
        str_free(cstate_data->delta_uuid);
        cstate_data->delta_uuid = 0;
    }

    //
    // Complain if they are in the integration directory,
    // because the rmdir at the end can't then run to completion.
    //
    os_become_orig();
    if (os_below_dir(dir, os_curdir()))
        change_fatal(cp, 0, i18n("leave int dir"));
    os_become_undo();

    //
    // remove the integration directory on success
    //
    if (user_delete_file_query(up, dir, true, true))
    {
        change_verbose(cp, 0, i18n("remove integration directory"));
        project_become(pp);
        commit_rmdir_tree_errok(dir);
        project_become_undo();
    }

    //
    // write out the data and release the locks
    //
    change_cstate_write(cp);
    user_ustate_write(up);
    pp->pstate_write();
    str_free(dir);
    commit();
    lock_release();

    //
    // run the notification command
    //
    change_run_integrate_begin_undo_command(cp);

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("integrate begin undo complete"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
integrate_begin_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        {arglex_token_help, integrate_begin_undo_help, },
        {arglex_token_list, integrate_begin_undo_list, },
    };

    trace(("integrate_begin_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), integrate_begin_undo_main);
    trace(("}\n"));
}
