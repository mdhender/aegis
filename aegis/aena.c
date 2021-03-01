/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1995, 1997-1999, 2001, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to implement new administrator
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <aena.h>
#include <ael/project/administrato.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <str_list.h>


static void new_administrator_usage _((void));

static void
new_administrator_usage()
{
    char	    *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -New_Administrator [ <option>... ] <username>...\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -New_Administrator -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -New_Administrator -Help\n", progname);
    quit(1);
}


static void new_administrator_help _((void));

static void
new_administrator_help()
{
    help("aena", new_administrator_usage);
}


static void new_administrator_list _((void));

static void
new_administrator_list()
{
    string_ty	    *project_name;

    trace(("new_administrator_list()\n{\n"));
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_administrator_usage);
	    continue;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
	    {
		option_needs_name
		(
		    arglex_token_project,
		    new_administrator_usage
		);
	    }
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    new_administrator_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;
	}
	arglex();
    }
    list_administrators(project_name, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void new_administrator_main _((void));

static void
new_administrator_main()
{
    string_list_ty  wl;
    string_ty	    *s1;
    int		    j;
    string_ty	    *project_name;
    project_ty	    *pp;
    user_ty	    *up;

    trace(("new_administrator_main()\n{\n"));
    arglex();
    string_list_constructor(&wl);
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_administrator_usage);
	    continue;

	case arglex_token_user:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_user, new_administrator_usage);
	    /* fall through... */

	case arglex_token_string:
	    s1 = str_from_c(arglex_value.alv_string);
	    if (string_list_member(&wl, s1))
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "Name", s1);
		error_intl(scp, i18n("too many user $name"));
		sub_context_delete(scp);
		new_administrator_usage();
	    }
	    string_list_append(&wl, s1);
	    str_free(s1);
	    break;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
	    {
		option_needs_name
		(
		    arglex_token_project,
		    new_administrator_usage
		);
	    }
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    new_administrator_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(new_administrator_usage);
	    break;
	}
	arglex();
    }
    if (!wl.nstrings)
    {
	error_intl(0, i18n("no user names"));
	new_administrator_usage();
    }

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * lock the project for change
     */
    project_pstate_lock_prepare(pp);
    lock_take();

    /*
     * check they are allowed to do this
     */
    if (!project_administrator_query(pp, user_name(up)))
	project_fatal(pp, 0, i18n("not an administrator"));

    /*
     * check they they are OK users
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	user_ty		*candidate;

	/*
	 * make sure the user isn't already there
	 */
	candidate = user_symbolic(pp, wl.string[j]);
	if (project_administrator_query(pp, user_name(candidate)))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", user_name(candidate));
	    project_fatal(pp, scp, i18n("$name already administrator"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}

	/*
	 * make sure the user exists
	 *	(should we check s/he is in the project's group?)
	 * this is to avoid security holes
	 */
	if (!user_uid_check(user_name(candidate)))
	    fatal_user_too_privileged(user_name(candidate));

	/*
	 * add it to the list
	 */
	project_administrator_add(pp, user_name(candidate));
	user_free(candidate);
    }

    /*
     * write out and release lock
     */
    project_pstate_write(pp);
    commit();
    lock_release();

    /*
     * verbose success message
     */
    for (j = 0; j < wl.nstrings; ++j)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", wl.string[j]);
	project_verbose(pp, scp, i18n("new administrator $name complete"));
	sub_context_delete(scp);
    }
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
new_administrator()
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, new_administrator_help, },
	{arglex_token_list, new_administrator_list, },
    };

    trace(("new_administrator()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_administrator_main);
    trace(("}\n"));
}
