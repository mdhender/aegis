//
// aegis - project change supervisor
// Copyright (C) 1994-1999, 2001-2008, 2011, 2012 Peter Miller
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

#include <common/gettime.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/project/history.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file/event.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>

#include <aegis/aedn.h>


static void
delta_name_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Delta_Name [ <option>... ][ <delta_number> ] <string>...\n",
        progname
    );
    fprintf(stderr, "       %s -Delta_Name -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Delta_Name -Help\n", progname);
    quit(1);
}


static void
delta_name_help(void)
{
    help("aedn", delta_name_usage);
}


static void
delta_name_list(void)
{
    trace(("delta_name_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(delta_name_usage);
    list_project_history(cid, 0);
    trace(("}\n"));
}


static void
delta_name_main(void)
{
    trace(("delta_name_main()\n{\n"));
    arglex();
    bool stomp = false;
    change_identifier cid;
    nstring new_name;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(delta_name_usage);
            continue;

        case arglex_token_overwriting:
            if (stomp)
                duplicate_option(delta_name_usage);
            stomp = true;
            break;

        case arglex_token_string:
            if (!new_name.empty())
            {
                error_intl(0, i18n("too many delta names"));
                delta_name_usage();
            }
            new_name = arglex_value.alv_string;
            break;

        case arglex_token_delta:
        case arglex_token_number:
        case arglex_token_change:
        case arglex_token_delta_from_change:
        case arglex_token_delta_date:
        case arglex_token_project:
            cid.command_line_parse(delta_name_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(delta_name_usage);
            break;
        }
        arglex();
    }
    cid.command_line_check(delta_name_usage);
    if (new_name.empty())
    {
        error_intl(0, i18n("no delta name"));
        delta_name_usage();
    }

    //
    // If the user did not nominate a specific change set, then we take
    // the head of the branch (most recent integration) as the change
    // the label is to be attached to.
    //
    if (!cid.set())
        cid.set_delta_from_branch_head();

    //
    // lock the project file
    //
    cid.get_pp()->pstate_lock_prepare();
    lock_take();

    //
    // it is an error if the user is not a project administrator
    //
    if (!project_administrator_query(cid.get_pp(), cid.get_up()->name()))
        project_fatal(cid.get_pp(), 0, i18n("not an administrator"));

    //
    // find the delta number
    //
    long delta_number = cid.get_cp()->delta_number_get();
    if (delta_number <= 0)
    {
        change_fatal(cid.get_cp(), 0, i18n("aedn bad state"));
    }

    //
    // Make sure the name has not been used already.
    //
    if (!stomp)
    {
        long other =
            project_history_delta_by_name(cid.get_pp(), new_name.get_ref(), 1);
        if (other && other != delta_number)
        {
            sub_context_ty sc;
            sc.var_set_string("Name", new_name);
            sc.var_set_long("Number", delta_number);
            sc.var_optional("Number");
            sc.var_set_long("Other", other);
            sc.var_optional("Other");
            project_fatal(cid.get_pp(), &sc, i18n("delta $name in use"));
            // NOTREACHED
        }
    }
    project_history_delta_name_delete(cid.get_pp(), new_name.get_ref());

    //
    // add the name to the selected history entry
    //
    project_history_delta_name_add
    (
        cid.get_pp(),
        delta_number,
        new_name.get_ref()
    );

    //
    // If the history label command is defined,
    // label each of the project files.
    //
    pconf_ty *pconf_data = project_pconf_get(cid.get_pp());
    if (pconf_data->history_label_command)
    {
        time_t delta_date =
            project_history_delta_to_timestamp(cid.get_pp(), delta_number);
        project_file_roll_forward historian(cid.get_pp(), delta_date, 0);
        for (size_t j = 0;; j++)
        {
            fstate_src_ty *src = cid.get_pp()->file_nth(j, view_path_simple);
            if (!src)
                break;
            file_event *fep = historian.get_last(src->file_name);
            if (!fep)
            {
                //
                // File not yet created at this delta.
                //
                continue;
            }
            src = fep->get_src();
            assert(src);
            switch (src->action)
            {
            case file_action_remove:
                //
                // File removed before this delta.
                //
                continue;

            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
                break;
            }

            //
            // Label everything else.
            //
            change_run_history_label_command
            (
                fep->get_change(),
                src,
                new_name.get_ref()
            );
        }
    }

    //
    // release the locks
    //
    cid.get_pp()->pstate_write();
    commit();
    lock_release();

    //
    // verbose success message
    //
    {
        sub_context_ty sc;
        sc.var_set_string("Name", new_name);
        sc.var_optional("Name");
        sc.var_set_long("Number", delta_number);
        sc.var_optional("Number");
        project_verbose(cid.get_pp(), &sc, i18n("delta name complete"));
    }
    trace(("}\n"));
}


void
delta_name_assignment(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, delta_name_help, 0 },
        { arglex_token_list, delta_name_list, 0 },
    };

    trace(("delta_name_assignment()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), delta_name_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
