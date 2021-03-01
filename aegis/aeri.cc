//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1997-1999, 2001-2006 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to implement removal of integrators
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <libaegis/ael/project/integrators.h>
#include <aegis/aeri.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/project.h>
#include <libaegis/commit.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/user.h>


static void
remove_integrator_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Remove_Integrator [ <option>... ] <username>...\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Remove_Integrator -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Remove_Integrator -Help\n", progname);
    quit(1);
}


static void
remove_integrator_help(void)
{
    help("aeri", remove_integrator_usage);
}


static void
remove_integrator_list(void)
{
    string_ty	    *project_name;

    trace(("remove_integrator_list()\n{\n"));
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(remove_integrator_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, remove_integrator_usage);
	    continue;
	}
	arglex();
    }
    list_integrators(project_name, 0, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
remove_integrator_inner(project_ty *pp, string_list_ty *wlp, int strict)
{
    size_t          j;
    user_ty         *up;

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // lock the project for change
    //
    pp->pstate_lock_prepare();
    lock_take();

    //
    // check they are allowed to do this
    //
    if (!project_administrator_query(pp, user_name(up)))
	project_fatal(pp, 0, i18n("not an administrator"));

    //
    // check they they are OK users
    //
    for (j = 0; j < wlp->nstrings; ++j)
    {
	string_ty	*name;

	name = wlp->string[j];
	if (!project_integrator_query(pp, name))
	{
	    sub_context_ty  *scp;

	    if (!strict)
		continue;
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    project_fatal(pp, scp, i18n("user \"$name\" is not an integrator"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
	project_integrator_remove(pp, name);
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
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", wlp->string[j]);
	project_verbose(pp, scp, i18n("remove integrator $name complete"));
	sub_context_delete(scp);
    }
    user_free(up);
}


static void
remove_integrator_main(void)
{
    string_ty	    *s1;
    string_ty	    *project_name;
    project_ty	    *pp;
    int             recursive;

    trace(("remove_integrator_main()\n{\n"));
    arglex();
    string_list_ty wl;
    project_name = 0;
    recursive = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(remove_integrator_usage);
	    continue;

	case arglex_token_project_recursive:
	    recursive = 1;
	    break;

	case arglex_token_user:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_user, remove_integrator_usage);
	    // fall through...

	case arglex_token_string:
	    s1 = str_from_c(arglex_value.alv_string);
	    if (wl.member(s1))
	    {
		sub_context_ty	*scp;

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
	    arglex_parse_project(&project_name, remove_integrator_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(remove_integrator_usage);
	    break;
	}
	arglex();
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no user names"));
	remove_integrator_usage();
    }

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    if (recursive)
    {
	string_list_ty pl;
	pp->list_inner(pl);
	for (size_t j = 0; j < pl.nstrings; ++j)
	{
	    project_ty *branch;
	    branch = project_alloc(pl.string[j]);
	    branch->bind_existing();
	    remove_integrator_inner(branch, &wl, 0);
	    project_free(branch);
	}
    }
    else
    {
	remove_integrator_inner(pp, &wl, 1);
    }
    project_free(pp);
    trace(("}\n"));
}


void
remove_integrator(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, remove_integrator_help, },
	{arglex_token_list, remove_integrator_list, },
    };

    trace(("remove_integrator()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), remove_integrator_main);
    trace(("}\n"));
}
