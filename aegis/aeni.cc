//
// aegis - project change supervisor
// Copyright (C) 1991-1995, 1997-1999, 2001-2008, 2011, 2012 Peter Miller
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
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/project/integrators.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aeni.h>


static void
new_integrator_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -New_Integrator <username>... [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -New_Integrator -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -New_Integrator -Help\n", progname);
    quit(1);
}


static void
new_integrator_help(void)
{
    help("aeni", new_integrator_usage);
}


static void
new_integrator_list(void)
{
    trace(("new_integrator_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(new_integrator_usage);
    list_integrators(cid, 0);
    trace(("}\n"));
}


static void
new_integrator_inner(project *pp, string_list_ty *wlp, int strict)
{
    size_t          j;
    user_ty::pointer up;

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // lock the project for change
    //
    pp->pstate_lock_prepare();
    lock_take();

    //
    // check they are allowed to do this
    //
    if (!project_administrator_query(pp, up->name()))
        project_fatal(pp, 0, i18n("not an administrator"));

    //
    // check they they are OK users
    //
    for (j = 0; j < wlp->nstrings; ++j)
    {
        user_ty::pointer candidate;

        //
        // make sure the user isn't already there
        //
        candidate = user_ty::create(nstring(wlp->string[j]));
        if (project_integrator_query(pp, candidate->name()))
        {
            if (!strict)
                continue;
            sub_context_ty sc;
            sc.var_set_string("Name", candidate->name());
            project_fatal(pp, &sc, i18n("$name already integrator"));
            // NOTREACHED
        }

        //
        // make sure the user exists
        //      (should we check s/he is in the project's group?)
        // this is to avoid security holes
        //
        if (!candidate->check_uid())
            fatal_user_too_privileged(candidate->name());

        //
        // add it to the list
        //
        project_integrator_add(pp, candidate->name());
    }

    //
    // write out and release lock
    //
    pp->pstate_write();
    commit();
    lock_release();

    //
    // verbose success message
    //
    for (j = 0; j < wlp->nstrings; ++j)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "Name", wlp->string[j]);
        project_verbose(pp, scp, i18n("new integrator $name complete"));
        // NOTREACHED
        sub_context_delete(scp);
    }
}


static void
new_integrator_main(void)
{
    string_ty       *s1;
    string_ty       *project_name;
    project         *pp;
    int             recursive;

    trace(("new_integrator_main()\n{\n"));
    arglex();
    string_list_ty wl;
    project_name = 0;
    recursive = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(new_integrator_usage);
            continue;

        case arglex_token_project_recursive:
            recursive = 1;
            break;

        case arglex_token_user:
            if (arglex() != arglex_token_string)
                option_needs_name(arglex_token_user, new_integrator_usage);
            // fall through...

        case arglex_token_string:
            s1 = str_from_c(arglex_value.alv_string);
            if (wl.member(s1))
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_string(scp, "Name", s1);
                fatal_intl(scp, i18n("too many user $name"));
                // NOTREACHED
                sub_context_delete(scp);
            }
            wl.push_back(s1);
            str_free(s1);
            break;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, new_integrator_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(new_integrator_usage);
            break;
        }
        arglex();
    }
    if (!wl.nstrings)
    {
        error_intl(0, i18n("no user names"));
        new_integrator_usage();
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

    if (recursive)
    {
        string_list_ty pl;
        pp->list_inner(pl);
        for (size_t j = 0; j < pl.nstrings; ++j)
        {
            project      *branch;

            branch = project_alloc(pl.string[j]);
            branch->bind_existing();
            new_integrator_inner(branch, &wl, 0);
            project_free(branch);
        }
    }
    else
    {
        new_integrator_inner(pp, &wl, 1);
    }
    project_free(pp);
    trace(("}\n"));
}


void
new_integrator(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, new_integrator_help, 0 },
        { arglex_token_list, new_integrator_list, 0 },
    };

    trace(("new_integrator()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_integrator_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
