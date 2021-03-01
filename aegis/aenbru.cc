//
// aegis - project change supervisor
// Copyright (C) 1995, 1999, 2001-2009, 2011, 2012 Peter Miller
// Copyright (C) 2008 Walter Franzini
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

#include <common/ac/assert.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/active.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aenbru.h>


static void
new_branch_undo_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -New_Branch_Undo <change_number> [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -New_Branch_Undo -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -New_Branch_Undo -Help\n", progname);
    quit(1);
}


static void
new_branch_undo_help(void)
{
    help("aenbru", new_branch_undo_usage);
}


static void
new_branch_undo_list(void)
{
    trace(("new_branch_undo_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(new_branch_undo_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_developed);
    trace(("}\n"));
}


static string_ty *
branch_changes_path_get(change::pointer cp)
{
    project         *pp;
    string_ty       *result;

    assert(cp->is_a_branch());
    pp = cp->pp->bind_branch(change_copy(cp));
    result = str_copy(pp->changes_path_get());
    project_free(pp);
    return result;
}


static void
new_branch_undo_main(void)
{
    string_ty       *project_name;
    long            change_number;
    project         *pp;
    user_ty::pointer up;
    change::pointer cp;
    string_ty       *s;

    trace(("new_branch_undo_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(new_branch_undo_usage);
            continue;

        case arglex_token_keep:
        case arglex_token_interactive:
        case arglex_token_keep_not:
            user_ty::delete_file_argument(new_branch_undo_usage);
            break;

        case arglex_token_change:
        case arglex_token_branch:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_branch
            (
                &project_name,
                &change_number,
                new_branch_undo_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, new_branch_undo_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(new_branch_undo_usage);
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
        project_name = n.get_ref_copy();
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();
    if (!project_administrator_query(pp, up->name()))
        project_fatal(pp, 0, i18n("not an administrator"));

    //
    // locate change data
    //
    if (!change_number)
        change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Make sure we are talking about a branch.
    //
    if (!cp->is_a_branch())
        change_fatal(cp, 0, i18n("use aedbu instead"));

    //
    // Make sure the branch is not active.
    //      (project_active reports the error itself)
    //
    project_active_check_branch(cp, 0);

    //
    // Take an advisory write lock on the appropriate row of the change
    // table.  Take an advisory write lock on the appropriate row of the
    // user table.  Block until can get both simultaneously.
    //
    gonzo_gstate_lock_prepare_new();
    pp->pstate_lock_prepare();
    change_cstate_lock_prepare(cp);
    up->ustate_lock_prepare();
    lock_take();

    //
    // Race condition: check that the up is still a project
    // administrator now that we have the project lock.
    //
    if (!project_administrator_query(pp, up->name()))
        project_fatal(pp, 0, i18n("not an administrator"));

    //
    // It is an error if the change is not in the being developed state.
    // It is an error if the change is not assigned to the current user.
    //
    if (!cp->is_being_developed())
        change_fatal(cp, 0, i18n("bad dbu state"));

    //
    // Remove the change from the list of assigned changes in the user
    // change table (in the user row).
    //
    up->own_remove(pp, change_number);

    //
    // remove the development directory
    //
    nstring dd(change_top_path_get(cp, 1));
    if (up->delete_file_query(dd, true, true))
    {
        change_verbose(cp, 0, i18n("remove development directory"));
        user_ty::become scoped(pp->get_user());
        commit_rmdir_tree_errok(dd);
    }

    //
    // tell the project to forget this change
    //
    project_change_delete(pp, change_number);

    //
    // delete the change state file of the branch
    // and the state files of all subordinate changes
    //
    project_become(pp);
    commit_unlink_errok(cp->cstate_filename_get());
    commit_unlink_errok(change_fstate_filename_get(cp));
    commit_rmdir_tree_errok(branch_changes_path_get(cp));
    project_become_undo(pp);

    //
    // Remove aliases of this branch.
    //      (Punctuation?)
    //
    s = str_format("%s.%ld", project_name_get(pp).c_str(), change_number);
    gonzo_alias_delete(s);
    str_free(s);

    //
    // Update change table row (and change history table).
    // Update user table row.
    // Release advisory write locks.
    //
    pp->pstate_write();
    gonzo_gstate_write();
    commit();
    lock_release();

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("new branch undo complete"));
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
new_branch_undo(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, new_branch_undo_help, 0 },
        { arglex_token_list, new_branch_undo_list, 0 },
    };

    trace(("new_branch_undo()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_branch_undo_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
