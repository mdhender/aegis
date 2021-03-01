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

#include <common/ac/assert.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/common.fmtgen.h>
#include <libaegis/dir.h>
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

#include <aegis/aedb.h>


static void
develop_begin_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Develop_Begin <change_number> [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Develop_Begin -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Develop_Begin -Help\n", progname);
    quit(1);
}


static void
develop_begin_help(void)
{
    help("aedb", develop_begin_usage);
}


static void
develop_begin_list(void)
{
    trace(("develop_begin_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(develop_begin_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_awaiting_development);
    trace(("}\n"));
}


static void
develop_begin_main(void)
{
    trace(("develop_begin_main()\n{\n"));
    change_identifier cid;
    arglex();
    string_ty *devdir = 0;
    nstring usr;
    log_style_ty log_style = log_style_create_default;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(develop_begin_usage);
            continue;

        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_string:
        case arglex_token_trunk:
            cid.command_line_parse(develop_begin_usage);
            continue;

        case arglex_token_directory:
            if (arglex() != arglex_token_string)
                option_needs_dir(arglex_token_directory, develop_begin_usage);
            if (devdir)
            {
                duplicate_option_by_name
                (
                    arglex_token_directory,
                    develop_begin_usage
                );
            }

            //
            // To cope with automounters, directories are stored as
            // given, or are derived from the home directory in the
            // passwd file.  Within aegis, pathnames have their
            // symbolic links resolved, and any comparison of paths
            // is done on this "system idea" of the pathname.
            //
            devdir = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_user:
            if (!usr.empty())
                duplicate_option(develop_begin_usage);
            if (arglex() != arglex_token_string)
                option_needs_name(arglex_token_user, develop_begin_usage);
            usr = arglex_value.alv_string;
            break;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(develop_begin_usage);
            log_style = log_style_none;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(develop_begin_usage);
            break;

        case arglex_token_reason:
            if (reason)
                duplicate_option(develop_begin_usage);
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_reason, develop_begin_usage);
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
    cid.command_line_check(develop_begin_usage);

    //
    // locate user data
    //
    //      up = user to own the change
    //      admin_user = administrator forcing (NULL if not forced)
    //
    user_ty::pointer up = cid.get_up();
    user_ty::pointer admin_user;
    if (!usr.empty())
    {
        user_ty::pointer usr_up = user_ty::create(usr);
        if (up != usr_up)
        {
            if (!project_administrator_query(cid.get_pp(), up->name()))
                project_fatal(cid.get_pp(), 0, i18n("not an administrator"));
            admin_user = up;
            up = usr_up;
        }
    }

    //
    // Make sure the tests don't go too fast.
    //
    os_throttle();

    //
    // The change number must be given on the command line,
    // even if there is only one appropriate change.
    // The is the "least surprizes" principle at work,
    // even though we could sometimes work this out for ourself.
    //
    if (!cid.set())
        fatal_intl(0, i18n("no change number"));

    //
    // Take an advisory write lock on the appropriate row of the change
    // table.  Take an advisory write lock on the appropriate row of the
    // user table.  The user table row may need to be created.
    // Block while can't get both simultaneously.
    //
    up->ustate_lock_prepare();
    change_cstate_lock_prepare(cid.get_cp());
    lock_take();

    //
    // Extract the appropriate row of the change table.
    // It is an error if the change is not in the
    // awaiting_development state.
    //
    if (!cid.get_cp()->is_awaiting_development())
        change_fatal(cid.get_cp(), 0, i18n("bad db state"));
    if (!project_developer_query(cid.get_pp(), up->name()))
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        if (admin_user)
        {
            sub_var_set_string(scp, "User", up->name());
            sub_var_optional(scp, "User");
            sub_var_override(scp, "User");
        }
        project_fatal(cid.get_pp(), scp, i18n("not a developer"));
        // NOTREACHED
        sub_context_delete(scp);
    }

    //
    // Work out the development directory.
    //
    // (Do this before the state advances to being developed,
    // otherwise it tries to find the config file in the as-yet
    // non-existant development directory.)
    //
    if (!devdir)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        devdir = change_development_directory_template(cid.get_cp(), up);
        sub_var_set_string(scp, "File_Name", devdir);
        change_verbose
        (
            cid.get_cp(),
            scp,
            i18n("development directory \"$filename\"")
        );
        sub_context_delete(scp);
    }
    change_development_directory_set(cid.get_cp(), os_canonify_dirname(devdir));

    //
    // Make sure the development directory is reachangle by the project
    // owner (used during aeib) and by the reviewers.  We look for
    // the directory 'x' bits, to make sure it is at least possible
    // traverse all of the directories in order to reach the development
    // directory.
    //
    project_become(cid.get_pp());
    os_check_path_traversable(devdir);
    project_become_undo(cid.get_pp());

    //
    // Set the change data to reflect the current user
    // as developer and move it to the being_developed state.
    // Append another entry to the change history.
    //
    cstate_ty *cstate_data = cid.get_cp()->cstate_get();
    cstate_data->state = cstate_state_being_developed;
    cstate_history_ty *history_data = change_history_new(cid.get_cp(), up);
    history_data->what = cstate_history_what_develop_begin;
    if (admin_user)
    {
        string_ty *r2 =
            str_format
            (
                "Forced by administrator %s.",
                admin_user->name().quote_c().c_str()
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
        history_data->why = reason;
    }

    //
    // Create the development directory.
    //
    up->become_begin();
    os_mkdir(devdir, 02755);
    undo_rmdir_errok(devdir);
    up->become_end();

    //
    // run the develop begin command
    //
    trace(("mark\n"));
    log_open(change_logfile_get(cid.get_cp()), up, log_style);
    cid.get_cp()->run_develop_begin_early_command(up);

    //
    // Update user change table to include this change in the list of
    // changes being developed by this user.
    //
    up->own_add(cid.get_pp(), cid.get_cp()->number);

    //
    // Clear the time fields.
    //
    change_build_times_clear(cid.get_cp());

    //
    // Update change table row (and change history table).
    // Update user table row.
    // Release advisory write locks.
    //
    trace(("mark\n"));
    cid.get_cp()->cstate_write();
    up->ustate_write();
    commit();
    lock_release();

    //
    // run the develop begin command
    //
    trace(("mark\n"));
    cid.get_cp()->run_develop_begin_command(up);

    //
    // run the forced develop begin notify command
    //
    trace(("mark\n"));
    if (admin_user)
        cid.get_cp()->run_forced_develop_begin_notify_command(admin_user);

    //
    // Update the RSS feed file if necessary.
    //
    trace(("mark\n"));
    rss_add_item_by_change(cid.get_pp(), cid.get_cp());

    //
    // if symlinks are being used to pander to dumb DMT,
    // and they are not removed after each build,
    // create them now, rather than waiting for the first build.
    // This will present a more uniform interface to the developer.
    //
    trace(("mark\n"));
    pconf_ty *pconf_data = change_pconf_get(cid.get_cp(), 0);
    assert(pconf_data->development_directory_style);
    if (!pconf_data->development_directory_style->during_build_only)
    {
        work_area_style_ty style = *pconf_data->development_directory_style;
        change_create_symlinks_to_baseline(cid.get_cp(), up, style);
    }

    //
    // verbose success message
    //
    trace(("mark\n"));
    change_verbose(cid.get_cp(), 0, i18n("develop begin complete"));
    trace(("}\n"));
}


void
develop_begin(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, develop_begin_help, 0 },
        { arglex_token_list, develop_begin_list, 0 },
    };

    trace(("develop_begin()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), develop_begin_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
