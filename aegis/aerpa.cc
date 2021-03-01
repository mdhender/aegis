//
// aegis - project change supervisor
// Copyright (C) 1999, 2001-2008, 2011, 2012 Peter Miller
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

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/project/aliases.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/gonzo.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aerpa.h>


static void
project_alias_remove_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Remove_Project_Alias [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Remove_Project_Alias -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Remove_Project_Alias -Help\n", progname);
    quit(1);
}


static void
project_alias_remove_help(void)
{
    help("aerpa", project_alias_remove_usage);
}


static void
project_alias_remove_list(void)
{
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(project_alias_remove_usage);
    list_project_aliases(cid, 0);
}


static void
project_alias_remove_main(void)
{
    string_ty       *project_name;
    project         *pp;
    user_ty::pointer up;
    sub_context_ty  *scp;

    trace(("project_alias_remove_main()\n{\n"));
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(project_alias_remove_usage);
            continue;

        case arglex_token_keep:
        case arglex_token_interactive:
        case arglex_token_keep_not:
            user_ty::delete_file_argument(project_alias_remove_usage);
            break;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, project_alias_remove_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(project_alias_remove_usage);
            break;
        }
        arglex();
    }

    //
    // locate project data
    //
    if (!project_name)
    {
        error_intl(0, i18n("no project name"));
        project_alias_remove_usage();
    }
    pp = project_alloc(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // make sure it's an alias
    //
    if (!gonzo_alias_to_actual(project_name))
    {
        scp = sub_context_new();
        sub_var_set_string(scp, "Name", project_name);
        project_fatal(pp, scp, i18n("project alias $name exists not"));
        // NOTREACHED
    }

    //
    // lock gstate
    //
    gonzo_gstate_lock_prepare_new();
    lock_take();

    //
    // it is an error if the current user is not an administrator
    //
    if (!project_administrator_query(pp, up->name()))
        project_fatal(pp, 0, i18n("not an administrator"));

    //
    // remove the project alias
    //
    gonzo_alias_delete(project_name);
    gonzo_gstate_write();

    //
    // release the locks
    //
    commit();
    lock_release();

    //
    // verbose success message
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "Name", project_name);
    project_verbose(pp, scp, i18n("remove project alias $name complete"));
    sub_context_delete(scp);

    //
    // clean up and go home
    //
    project_free(pp);
    str_free(project_name);
    trace(("}\n"));
}


void
project_alias_remove(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, project_alias_remove_help, 0 },
        { arglex_token_list, project_alias_remove_list, 0 },
    };

    trace(("project_alias_remove()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), project_alias_remove_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
