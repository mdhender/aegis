/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to remove project aliases
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <ael/project/aliases.h>
#include <aerpa.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <gonzo.h>
#include <gonzo.h>
#include <help.h>
#include <lock.h>
#include <progname.h>
#include <project.h>
#include <project/history.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


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
    while (arglex_token != arglex_token_eoln)
	generic_argument(project_alias_remove_usage);
    list_project_aliases(0, 0, 0);
}


static void
project_alias_remove_main(void)
{
    string_ty	    *project_name;
    project_ty	    *pp;
    user_ty	    *up;
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
	    user_delete_file_argument(project_alias_remove_usage);
	    break;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
	    {
		option_needs_name
		(
		    arglex_token_project,
		    project_alias_remove_usage
		);
	    }
	    /* fall through... */

	case arglex_token_string:
	    if (project_name)
	    {
		duplicate_option_by_name
		(
		    arglex_token_project,
		    project_alias_remove_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(project_alias_remove_usage);
	    break;
	}
	arglex();
    }

    /*
     * locate project data
     */
    if (!project_name)
    {
	error_intl(0, i18n("no project name"));
	project_alias_remove_usage();
    }
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * make sure it's an alias
     */
    if (!gonzo_alias_to_actual(project_name))
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "Name", project_name);
	project_fatal(pp, scp, i18n("project alias $name exists not"));
	/* NOTREACHED */
    }

    /*
     * lock gstate
     */
    gonzo_gstate_lock_prepare_new();
    lock_take();

    /*
     * it is an error if the current user is not an administrator
     */
    if (!project_administrator_query(pp, user_name(up)))
	project_fatal(pp, 0, i18n("not an administrator"));

    /*
     * remove the project alias
     */
    gonzo_alias_delete(project_name);
    gonzo_gstate_write();

    /*
     * release the locks
     */
    commit();
    lock_release();

    /*
     * verbose success message
     */
    scp = sub_context_new();
    sub_var_set_string(scp, "Name", project_name);
    project_verbose(pp, scp, i18n("remove project alias $name complete"));
    sub_context_delete(scp);

    /*
     * clean up and go home
     */
    project_free(pp);
    user_free(up);
    str_free(project_name);
    trace(("}\n"));
}


void
project_alias_remove(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, project_alias_remove_help, },
	{arglex_token_list, project_alias_remove_list, },
    };

    trace(("project_alias_remove()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), project_alias_remove_main);
    trace(("}\n"));
}
