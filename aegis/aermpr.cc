//
//	aegis - project change supervisor
//	Copyright (C) 1993-1999, 2001-2006 Peter Miller;
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
// MANIFEST: functions to implement remove project
//

#include <common/ac/stdio.h>

#include <libaegis/ael/project/projects.h>
#include <aegis/aermpr.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change.h>
#include <libaegis/commit.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <libaegis/project.h>
#include <libaegis/project/active.h>
#include <libaegis/project/history.h>
#include <common/quit.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/user.h>


static void
remove_project_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -ReMove_PRoject [ <option>... ]\n", progname);
    fprintf
    (
	stderr,
	"       %s -ReMove_PRoject -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -ReMove_PRoject -Help\n", progname);
    quit(1);
}


static void
remove_project_help(void)
{
    help("aermpr", remove_project_usage);
}


static void
remove_project_list(void)
{
    arglex();
    while (arglex_token != arglex_token_eoln)
	generic_argument(remove_project_usage);
    list_projects(0, 0, 0);
}


static void
remove_project_main(void)
{
    long	    nerr;
    string_ty	    *project_name;
    project_ty	    *pp;
    user_ty	    *up;
    int		    still_exists;

    trace(("remove_project_main()\n{\n"));
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(remove_project_usage);
	    continue;

	case arglex_token_keep:
	case arglex_token_interactive:
	case arglex_token_keep_not:
	    user_delete_file_argument(remove_project_usage);
	    break;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, remove_project_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(remove_project_usage);
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
	remove_project_usage();
    }
    pp = project_alloc(project_name);
    pp->bind_existing();

    //
    // Make sure it is a top-level project we are talking about,
    // not merely a branch.
    //
    if (!pp->is_a_trunk())
	project_fatal(pp, 0, i18n("use aenbru instead"));

    //
    // make sure it's not an alias
    //
    if (gonzo_alias_to_actual(project_name))
	fatal_project_alias_exists(project_name);
    str_free(project_name);

    //
    // see if the user already deleted it
    //
    os_become_orig();
    still_exists = os_exists(pp->pstate_path_get());
    os_become_undo();

    //
    // locate user data
    //
    up = user_executing(still_exists ? pp : (project_ty *)0);

    //
    // lock the project
    //
    pp->pstate_lock_prepare();
    gonzo_gstate_lock_prepare_new();
    lock_take();

    //
    // avoid reading any project state information
    // if it is already gone
    //
    if (!still_exists)
	goto nuke;

    //
    // it is an error if any of the changes are active
    // or any changes on any of the branches
    // (project_active reports the error itself)
    //
    project_active_check(pp, 1);

    //
    // it is an error if the current user is not an administrator
    //
    nerr = 0;
    if (!project_administrator_query(pp, user_name(up)))
    {
	project_error(pp, 0, i18n("not an administrator"));
	nerr++;
    }
    if (nerr)
	quit(1);

    //
    // remove the project directory
    //
    if (user_delete_file_query(up, pp->home_path_get(), 1))
    {
	project_verbose(pp, 0, i18n("remove project directory"));
	project_become(pp);
	commit_rmdir_tree_errok(pp->home_path_get());
	project_become_undo();
    }

    //
    // tell gonzo to forget about this project
    //
    nuke:
    gonzo_project_delete(pp);
    gonzo_gstate_write();

    //
    // release the locks
    //
    commit();
    lock_release();

    //
    // verbose success message
    //
    project_verbose(pp, 0, i18n("remove project complete"));

    //
    // clean up and go home
    //
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
remove_project(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, remove_project_help, },
	{arglex_token_list, remove_project_list, },
    };

    trace(("remove_project()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), remove_project_main);
    trace(("}\n"));
}
