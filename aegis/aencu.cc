//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2008 Peter Miller
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
#include <common/ac/time.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/cattr.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/common.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aeca.h>
#include <aegis/aencu.h>


static void
new_change_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -New_Change_Undo [ <option>... ]\n", progname);
    fprintf
    (
	stderr,
	"       %s -New_Change_Undo -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -New_Change_Undo -Help\n", progname);
    quit(1);
}


static void
new_change_undo_help(void)
{
    help("aencu", new_change_undo_usage);
}


static void
new_change_undo_list(void)
{
    trace(("new_change_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(new_change_undo_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_awaiting_development);
    trace(("}\n"));
}


static void
new_change_undo_main(void)
{
    string_ty	    *project_name;
    long	    change_number;
    project_ty	    *pp;
    user_ty::pointer up;
    change::pointer cp;
    cstate_ty	    *cstate_data;

    trace(("new_change_undo_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_change_undo_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		new_change_undo_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, new_change_undo_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(new_change_undo_usage);
	    break;
	}
	arglex();
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

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    // The change number must be given on the command line,
    // even if there is only one appropriate change.
    // The is the "least surprizes" principle at work,
    // even though we could sometimes work this out for ourself.
    //
    if (!change_number)
	change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Take an advisory write lock on the project state
    // and the change state.
    //
    pp->pstate_lock_prepare();
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = cp->cstate_get();

    //
    // Extract the appropriate row of the change table.
    // It is an error if the change is not in the
    // awaiting_development state.
    //
    if (cstate_data->state != cstate_state_awaiting_development)
	change_fatal(cp, 0, i18n("bad ncu state"));
    if
    (
	!project_administrator_query(pp, up->name())
    &&
	nstring(change_creator_name(cp)) != up->name()
    )
	project_fatal(pp, 0, i18n("not an administrator"));

    //
    // tell the project to forget this change
    //
    project_change_delete(pp, change_number);

    //
    // delete the change state file
    //
    project_become(pp);
    commit_unlink_errok(change_cstate_filename_get(cp));
    commit_unlink_errok(change_fstate_filename_get(cp));
    project_become_undo(pp);

    //
    // Update change table row (and change history table).
    // Update user table row.
    // Release advisory write locks.
    //
    pp->pstate_write();
    commit();
    lock_release();

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("new change undo complete"));
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
new_change_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, new_change_undo_help, 0 },
	{ arglex_token_list, new_change_undo_list, 0 },
    };

    trace(("new_change_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_change_undo_main);
    trace(("}\n"));
}
