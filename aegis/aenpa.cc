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

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/project/aliases.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aenpa.h>


static void
project_alias_create_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -New_Project_Alias <name> [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -New_Project_Alias -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -New_Project_Alias -Help\n", progname);
    quit(1);
}


static void
project_alias_create_help(void)
{
    help("aenpa", project_alias_create_usage);
}


static void
project_alias_create_list(void)
{
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(project_alias_create_usage);
    list_project_aliases(cid, 0);
}


static void
project_alias_create_main(void)
{
    sub_context_ty  *scp;
    string_ty       *project_name[2];
    int             project_name_count;
    project         *pp;
    user_ty::pointer up;

    trace(("project_alias_create_main()\n{\n"));
    arglex();
    project_name_count = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(project_alias_create_usage);
            continue;

        case arglex_token_project:
            if (arglex() != arglex_token_string)
            {
                option_needs_name
                (
                    arglex_token_project,
                    project_alias_create_usage
                );
            }
            // fall through...

        case arglex_token_string:
            if (project_name_count >= 2)
                fatal_intl(0, i18n("too many proj name"));
            project_name[project_name_count++] =
                str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(project_alias_create_usage);
            break;
        }
        arglex();
    }
    if (project_name_count != 2)
        fatal_intl(0, i18n("no project name"));

    //
    // locate OLD project data
    //
    pp = project_alloc(project_name[0]);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // it is an error if the current user is not an administrator
    // of the old project.
    //
    if (!project_administrator_query(pp, up->name()))
        project_fatal(pp, 0, i18n("not an administrator"));

    //
    // Make sure the project alias is acceptable.
    //
    if (!gonzo_alias_acceptable(project_name[1]))
    {
        scp = sub_context_new();
        sub_var_set_string(scp, "Name", project_name[1]);
        fatal_intl(scp, i18n("bad alias $name"));
        // NOTREACHED
        sub_context_delete(scp);
    }

    //
    // take the relevant locks
    //
    gonzo_gstate_lock_prepare_new();
    lock_take();

    //
    // add the new alias
    //      it is an error if it is already in use
    //
    if (gonzo_alias_to_actual(project_name[1]))
        fatal_project_alias_exists(project_name[1]);
    if (gonzo_project_home_path_from_name(project_name[1]))
    {
        scp = sub_context_new();
        sub_var_set_string(scp, "Name", project_name[1]);
        fatal_intl(scp, i18n("project $name exists"));
        // NOTREACHED
        sub_context_delete(scp);
    }
    gonzo_alias_add(pp, project_name[1]);
    gonzo_gstate_write();

    //
    // release locks
    //
    commit();
    lock_release();

    //
    // verbose success message
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "Name", project_name[1]);
    project_verbose(pp, scp, i18n("new alias $name complete"));
    sub_context_delete(scp);
    str_free(project_name[0]);
    project_free(pp);
    str_free(project_name[1]);
    trace(("}\n"));
}


void
project_alias_create(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, project_alias_create_help, 0 },
        { arglex_token_list, project_alias_create_list, 0 },
    };

    trace(("project_alias_create()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), project_alias_create_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
