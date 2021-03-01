//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1997-1999, 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/project/administrato.h>
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

#include <aegis/aera.h>


static void
remove_administrator_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Remove_Administrator [ <option>... ] <username>...\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Remove_Administrator -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Remove_Administrator -Help\n", progname);
    quit(1);
}


static void
remove_administrator_help(void)
{
    help("aera", remove_administrator_usage);
}


static void
remove_administrator_list(void)
{
    trace(("remove_administrator_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(remove_administrator_usage);
    list_administrators(cid, 0);
    trace(("}\n"));
}


static void
remove_administrator_inner(project_ty *pp, string_list_ty *wlp, int strict)
{
    user_ty::pointer up;
    size_t          j;

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
    // check the users are actually administrators
    //
    for (j = 0; j < wlp->nstrings; ++j)
    {
	nstring name(wlp->string[j]);
	if (!project_administrator_query(pp, name))
	{
	    if (!strict)
		continue;
            sub_context_ty sc;
	    sc.var_set_string("Name", name);
	    project_fatal
	    (
		pp,
		&sc,
		i18n("user \"$name\" is not an administrator")
	    );
	    // NOTREACHED
	}
	project_administrator_remove(pp, name);
    }

    //
    // make sure there will always be at least one administrator
    //
    if (project_administrator_nth(pp, 0) == 0)
	project_fatal(pp, 0, i18n("not enough admin"));

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
	project_verbose(pp, scp, i18n("remove administrator $name complete"));
	sub_context_delete(scp);
    }
}


static void
remove_administrator_main(void)
{
    string_ty	    *s1;
    string_ty	    *project_name;
    project_ty	    *pp;
    int             recursive;

    trace(("remove_administrator_main()\n{\n"));
    arglex();
    string_list_ty wl;
    project_name = 0;
    recursive = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(remove_administrator_usage);
	    continue;

	case arglex_token_project_recursive:
	    recursive = 1;
	    break;

	case arglex_token_user:
	    if (arglex() != arglex_token_string)
	    {
		option_needs_name
		(
		    arglex_token_user,
		    remove_administrator_usage
		);
	    }
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
	    arglex_parse_project(&project_name, remove_administrator_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(remove_administrator_usage);
	    break;
	}
	arglex();
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no user names"));
	remove_administrator_usage();
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
	    project_ty      *branch;

	    branch = project_alloc(pl.string[j]);
	    branch->bind_existing();
	    remove_administrator_inner(branch, &wl, 0);
	    project_free(branch);
	}
    }
    else
    {
	remove_administrator_inner(pp, &wl, 1);
    }
    project_free(pp);
    trace(("}\n"));
}


void
remove_administrator(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, remove_administrator_help, 0 },
	{ arglex_token_list, remove_administrator_list, 0 },
    };

    trace(("remove_administrator()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), remove_administrator_main);
    trace(("}\n"));
}
