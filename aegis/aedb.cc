//
//      aegis - project change supervisor
//      Copyright (C) 1991-1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions to implement develop begin
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <aedb.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <col.h>
#include <commit.h>
#include <common.h>
#include <dir.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/history.h>
#include <quit.h>
#include <rss.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


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
    string_ty       *project_name;

    trace(("develop_begin_list()\n{\n"));
    project_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(develop_begin_usage);
            continue;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, develop_begin_usage);
            continue;
        }
        arglex();
    }
    list_changes_in_state_mask
    (
        project_name,
        1 << cstate_state_awaiting_development
    );
    if (project_name)
        str_free(project_name);
    trace(("}\n"));
}


static void
develop_begin_main(void)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    string_ty       *devdir;
    string_ty       *project_name;
    project_ty      *pp;
    long            change_number;
    change_ty       *cp;
    user_ty         *up;
    pconf_ty        *pconf_data;
    string_ty       *usr;
    user_ty         *up2;
    log_style_ty    log_style;

    trace(("develop_begin_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    devdir = 0;
    usr = 0;
    log_style = log_style_create_default;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(develop_begin_usage);
            continue;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                develop_begin_usage
            );
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

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, develop_begin_usage);
            continue;

        case arglex_token_user:
            if (usr)
                duplicate_option(develop_begin_usage);
            if (arglex() != arglex_token_string)
                option_needs_name(arglex_token_user, develop_begin_usage);
            usr = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(develop_begin_usage);
            log_style = log_style_none;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_lock_wait_argument(develop_begin_usage);
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
    //      up = user to own the change
    //      up2 = administrator forcing
    //
    if (usr)
    {
        up = user_symbolic(pp, usr);
        up2 = user_executing(pp);
        if (up == up2)
        {
            user_free(up2);
            up2 = 0;
        }
        else if (!project_administrator_query(pp, user_name(up2)))
            project_fatal(pp, 0, i18n("not an administrator"));
    }
    else
    {
        up = user_executing(pp);
        up2 = 0;
    }

    //
    // Make sure the tests don't go too fast.
    //
    os_throttle();

    //
    // locate change data
    //
    // The change number must be given on the command line,
    // even if there is only one appropriate change.
    // The is the "least surprizes" principle at work,
    // even though we could sometimes work this out for ourself.
    //
    if (!change_number)
        fatal_intl(0, i18n("no change number"));
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Take an advisory write lock on the appropriate row of the change
    // table.  Take an advisory write lock on the appropriate row of the
    // user table.  The user table row may need to be created.
    // Block while can't get both simultaneously.
    //
    user_ustate_lock_prepare(up);
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    //
    // Extract the appropriate row of the change table.
    // It is an error if the change is not in the
    // awaiting_development state.
    //
    if (cstate_data->state != cstate_state_awaiting_development)
        change_fatal(cp, 0, i18n("bad db state"));
    if (!project_developer_query(pp, user_name(up)))
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        if (up2)
        {
            sub_var_set_string(scp, "User", user_name(up));
            sub_var_optional(scp, "User");
            sub_var_override(scp, "User");
        }
        project_fatal(pp, scp, i18n("not a developer"));
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
        devdir = change_development_directory_template(cp, up);
        sub_var_set_string(scp, "File_Name", devdir);
        change_verbose(cp, scp, i18n("development directory \"$filename\""));
        sub_context_delete(scp);
    }
    change_development_directory_set(cp, devdir);

    //
    // Set the change data to reflect the current user
    // as developer and move it to the being_developed state.
    // Append another entry to the change history.
    //
    cstate_data->state = cstate_state_being_developed;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_develop_begin;
    if (up2)
    {
        string_ty *r2 =
            str_format
            (
                "Forced by administrator \"%s\".",
                user_name(up2)->str_text
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
    user_become(up);
    os_mkdir(devdir, 02755);
    undo_rmdir_errok(devdir);
    user_become_undo();

    //
    // Update user change table to include this change in the list of
    // changes being developed by this user.
    //
    user_own_add(up, project_name_get(pp), change_number);

    //
    // Clear the time fields.
    //
    change_build_times_clear(cp);

    //
    // Update change table row (and change history table).
    // Update user table row.
    // Release advisory write locks.
    //
    change_cstate_write(cp);
    user_ustate_write(up);
    commit();
    lock_release();

    //
    // run the develop begin command
    //
    log_open(change_logfile_get(cp), up, log_style);
    change_run_develop_begin_command(cp, up);

    //
    // run the forced develop begin notify command
    //
    if (up2)
        change_run_forced_develop_begin_notify_command(cp, up2);

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // if symlinks are being used to pander to dumb DMT,
    // and they are not removed after each build,
    // create them now, rather than waiting for the first build.
    // This will present a more uniform interface to the developer.
    //
    pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data->development_directory_style);
    if (!pconf_data->development_directory_style->during_build_only)
    {
        work_area_style_ty style = *pconf_data->development_directory_style;
        change_create_symlinks_to_baseline(cp, up, style);
    }

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("develop begin complete"));
    change_free(cp);
    project_free(pp);
    user_free(up);
    if (up2)
        user_free(up2);
    trace(("}\n"));
}


void
develop_begin(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        {arglex_token_help, develop_begin_help, },
        {arglex_token_list, develop_begin_list, },
    };

    trace(("develop_begin()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), develop_begin_main);
    trace(("}\n"));
}
