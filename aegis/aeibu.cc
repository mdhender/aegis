//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2001-2008, 2011, 2012 Peter Miller
// Copyright (C) 2008 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aeibu.h>


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
    trace(("integrate_begin_undo_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(integrate_begin_undo_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_integrated);
    trace(("}\n"));
}


static void
integrate_begin_undo_main(void)
{
    trace(("integrate_begin_main()\n{\n"));
    change_identifier cid;
    arglex();
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(integrate_begin_undo_usage);
            continue;

        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_string:
        case arglex_token_trunk:
            cid.command_line_parse(integrate_begin_undo_usage);
            continue;

        case arglex_token_keep:
        case arglex_token_interactive:
        case arglex_token_keep_not:
            user_ty::delete_file_argument(integrate_begin_undo_usage);
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(integrate_begin_undo_usage);
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
    cid.command_line_check(integrate_begin_undo_usage);

    //
    // lock the change for writing
    //
    cid.get_pp()->pstate_lock_prepare();
    change_cstate_lock_prepare(cid.get_cp());
    cid.get_up()->ustate_lock_prepare();
    lock_take();
    cstate_ty *cstate_data = cid.get_cp()->cstate_get();

    //
    // it is an error if the change is not in the 'being_integrated' state.
    // it is an error if user is not the integrator (or proj admin)
    //
    if (cstate_data->state != cstate_state_being_integrated)
        change_fatal(cid.get_cp(), 0, i18n("bad ibu state"));
    if
    (
        nstring(cid.get_cp()->integrator_name()) != cid.get_up()->name()
    &&
        !project_administrator_query(cid.get_pp(), cid.get_up()->name())
    )
        change_fatal(cid.get_cp(), 0, i18n("not integrator"));

    //
    // Change the state.
    // Add to the change's history.
    //
    cstate_data->state = cstate_state_awaiting_integration;
    cstate_history_ty *history_data =
        change_history_new(cid.get_cp(), cid.get_up());
    history_data->what = cstate_history_what_integrate_begin_undo;
    history_data->why = reason;

    //
    // clear the idiff times
    //
    for (size_t j = 0;; ++j)
    {
        fstate_src_ty   *src_data;

        src_data = change_file_nth(cid.get_cp(), j, view_path_first);
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
    cid.get_up()->own_remove(cid.get_pp(), cid.get_change_number());

    //
    // Note that the project has no current integration
    //
    project_current_integration_set(cid.get_pp(), 0);
    string_ty *dir =
        str_copy(change_integration_directory_get(cid.get_cp(), 1));
    change_integration_directory_clear(cid.get_cp());
    change_build_times_clear(cid.get_cp());
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
        change_fatal(cid.get_cp(), 0, i18n("leave int dir"));
    os_become_undo();

    //
    // remove the integration directory on success
    //
    if (cid.get_up()->delete_file_query(nstring(dir), true, true))
    {
        change_verbose(cid.get_cp(), 0, i18n("remove integration directory"));
        user_ty::become scoped(cid.get_pp()->get_user());
        commit_rmdir_tree_errok(dir);
    }

    //
    // write out the data and release the locks
    //
    cid.get_cp()->cstate_write();
    cid.get_up()->ustate_write();
    cid.get_pp()->pstate_write();
    str_free(dir);
    commit();
    lock_release();

    //
    // run the notification command
    //
    change_run_integrate_begin_undo_command(cid.get_cp());

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(cid.get_pp(), cid.get_cp());

    //
    // verbose success message
    //
    change_verbose(cid.get_cp(), 0, i18n("integrate begin undo complete"));
    trace(("}\n"));
}


void
integrate_begin_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, integrate_begin_undo_help, 0, },
        { arglex_token_list, integrate_begin_undo_list, 0, },
    };

    trace(("integrate_begin_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), integrate_begin_undo_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
