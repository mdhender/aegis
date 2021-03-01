//
// aegis - project change supervisor
// Copyright (C) 1991-1997, 1999-2008, 2012 Peter Miller
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
#include <common/ac/libintl.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aecd.h>


static void
change_directory_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Change_Directory [ <option>... ][ <subdir> ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Change_Directory -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Change_Directory -Help\n", progname);
    quit(1);
}


static void
change_directory_help(void)
{
    help("aecd", change_directory_usage);
}


static void
change_directory_list(void)
{
    trace(("change_directory_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(change_directory_usage);
    list_changes_in_state_mask
    (
        cid,
        (
            (1 << cstate_state_being_developed)
        |
            (1 << cstate_state_awaiting_review)
        |
            (1 << cstate_state_being_reviewed)
        |
            (1 << cstate_state_awaiting_integration)
        |
            (1 << cstate_state_being_integrated)
        )
    );
    trace(("}\n"));
}


static void
change_directory_main(void)
{
    const char      *subdir =       0;
    cstate_ty       *cstate_data;

    trace(("change_directory_main()\n{\n"));
    arglex();
    change_identifier cid;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(change_directory_usage);
            continue;

        case arglex_token_string:
            if (subdir)
                fatal_intl(0, i18n("too many dir"));
            subdir = arglex_value.alv_string;
            if (!*subdir || *subdir == '/')
                fatal_intl(0, i18n("dir must be rel"));
            break;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_development_directory:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(change_directory_usage);
            continue;
        }
        arglex();
    }
    cid.command_line_check(change_directory_usage);

    //
    // figure out where to go
    //
    string_ty *d = 0;
    cstate_data = cid.get_cp()->cstate_get();
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
    case cstate_state_completed:
#ifndef DEBUG
    default:
#endif
        change_fatal(cid.get_cp(), 0, i18n("bad cd, no dir"));

    case cstate_state_being_integrated:
        if (!cid.get_devdir())
            d = change_integration_directory_get(cid.get_cp(), 0);
        else
            d = change_development_directory_get(cid.get_cp(), 0);
        break;

    case cstate_state_awaiting_review:
        change_verbose
        (
            cid.get_cp(),
            0,
            i18n("remember to use the aerb command")
        );
        // fall through...

    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_being_developed:
        d = change_development_directory_get(cid.get_cp(), 0);
        break;
    }
    assert(d);

    //
    // Add in the extra path elements as necessary.
    // Flatten it out if they go up the tree (etc).
    //
    if (subdir)
    {
        string_ty       *tmp;

        tmp = str_format("%s/%s", d->str_text, subdir);
        user_ty::become scoped(cid.get_up());
        d = os_pathname(tmp, 0);
        str_free(tmp);
    }

    //
    // print out the path
    //      (do NOT free d)
    //
    printf("%s\n", d->str_text);
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", d);
    change_verbose
    (
        cid.get_cp(),
        scp,
        i18n("change directory $filename complete")
    );
    sub_context_delete(scp);
    trace(("}\n"));
}


void
change_directory(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, change_directory_help, 0 },
        { arglex_token_list, change_directory_list, 0 },
    };

    trace(("change_directory()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), change_directory_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
