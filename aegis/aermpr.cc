//
//	aegis - project change supervisor
//	Copyright (C) 1993-1999, 2001-2008 Peter Miller
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

#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/ael/project/projects.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/active.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aermpr.h>


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
    change_identifier cid;
    cid.command_line_parse_rest(remove_project_usage);
    list_projects(cid, 0);
}


static void
remove_project_main(void)
{
    long	    nerr;
    string_ty	    *project_name;
    project_ty	    *pp;
    user_ty::pointer up;
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
	    user_ty::delete_file_argument(remove_project_usage);
	    break;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, remove_project_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(remove_project_usage);
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
    up = user_ty::create();

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
    if (!project_administrator_query(pp, up->name()))
    {
	project_error(pp, 0, i18n("not an administrator"));
	nerr++;
    }
    if (nerr)
	quit(1);

    //
    // remove the project directory
    //
    {
        nstring projdir(pp->home_path_get());
        if (up->delete_file_query(projdir, true, -1))
        {
            project_verbose(pp, 0, i18n("remove project directory"));
            user_ty::become scoped(pp->get_user());
            commit_rmdir_tree_errok(projdir);
        }
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
    trace(("}\n"));
}


void
remove_project(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, remove_project_help, 0 },
	{ arglex_token_list, remove_project_list, 0 },
    };

    trace(("remove_project()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), remove_project_main);
    trace(("}\n"));
}
