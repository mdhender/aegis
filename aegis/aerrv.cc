//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1997-1999, 2001-2004 Peter Miller;
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
// MANIFEST: functions to implement remove reviewer
//

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <ael/project/reviewers.h>
#include <aerrv.h>
#include <arglex2.h>
#include <arglex/project.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


static void
remove_reviewer_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Remove_Reviewer [ <option>... ] <username>...\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Remove_Reviewer -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Remove_Reviewer -Help\n", progname);
    quit(1);
}


static void
remove_reviewer_help(void)
{
    help("aerrv", remove_reviewer_usage);
}


static void
remove_reviewer_list(void)
{
    string_ty	    *project_name;

    trace(("remove_reviewer_list()\n{\n"));
    project_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(remove_reviewer_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, remove_reviewer_usage);
	    continue;
	}
	arglex();
    }
    list_reviewers(project_name, 0, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
remove_reviewer_inner(project_ty *pp, string_list_ty *wlp, int strict)
{
    user_ty         *up;
    size_t          j;

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // lock the project for change
    //
    project_pstate_lock_prepare(pp);
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
	if (!project_reviewer_query(pp, name))
	{
	    sub_context_ty  *scp;

	    if (!strict)
		continue;
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    project_fatal(pp, scp, i18n("user \"$name\" is not a reviewer"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
	project_reviewer_remove(pp, name);
    }

    //
    // write out and release lock
    //
    project_pstate_write(pp);
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
	project_verbose(pp, scp, i18n("remove reviewer $name complete"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    user_free(up);
}


static void
remove_reviewer_main(void)
{
    string_list_ty  wl;
    string_ty	    *s1;
    string_ty	    *project_name;
    project_ty	    *pp;
    int             recursive;

    trace(("remove_reviewer_main()\n{\n"));
    arglex();
    project_name = 0;
    recursive = 0;
    string_list_constructor(&wl);
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(remove_reviewer_usage);
	    continue;

	case arglex_token_project_recursive:
	    recursive = 1;
	    break;

	case arglex_token_user:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_user, remove_reviewer_usage);
	    // fall through...

	case arglex_token_string:
	    s1 = str_from_c(arglex_value.alv_string);
	    if (string_list_member(&wl, s1))
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "Name", s1);
		fatal_intl(scp, i18n("too many user $name"));
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    string_list_append(&wl, s1);
	    str_free(s1);
	    break;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, remove_reviewer_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(remove_reviewer_usage);
	    break;
	}
	arglex();
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no user names"));
	remove_reviewer_usage();
    }

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    if (recursive)
    {
	string_list_ty  pl;
	size_t          j;

	string_list_constructor(&pl);
	project_list_inner(&pl,pp);
	for (j = 0; j < pl.nstrings; ++j)
	{
	    project_ty      *branch;

	    branch = project_alloc(pl.string[j]);
	    project_bind_existing(branch);
	    remove_reviewer_inner(branch, &wl, 0);
	    project_free(branch);
	}
	string_list_destructor(&pl);
    }
    else
    {
	remove_reviewer_inner(pp, &wl, 1);
    }
    project_free(pp);
    trace(("}\n"));
}


void
remove_reviewer(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, remove_reviewer_help, },
	{arglex_token_list, remove_reviewer_list, },
    };

    trace(("remove_reviewer()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), remove_reviewer_main);
    trace(("}\n"));
}
