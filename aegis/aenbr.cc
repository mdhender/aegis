//
//	aegis - project change supervisor
//	Copyright (C) 1995-2008 Peter Miller
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
#include <libaegis/ael/change/changes.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/verbose.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aenbr.h>


static void
new_branch_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -New_BRanch [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_BRanch -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_BRanch -Help\n", progname);
    quit(1);
}


static void
new_branch_help(void)
{
    help("aenbr", new_branch_usage);
}


static void
new_branch_list(void)
{
    trace(("new_branch_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(new_branch_usage);
    list_changes(cid, 0);
    trace(("}\n"));
}


static void
new_branch_main(void)
{
    string_ty	    *project_name;
    project_ty	    *pp;
    project_ty	    *bp;
    long	    change_number;
    user_ty::pointer up;
    string_ty	    *devdir;
    const char      *output_filename;

    trace(("new_branch_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    devdir = 0;
    output_filename = 0;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(new_branch_usage);
	    continue;

	case arglex_token_branch:
	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_branch
	    (
		&project_name,
		&change_number,
		new_branch_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, new_branch_usage);
	    continue;

	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, new_branch_usage);
	    if (devdir)
	    {
		duplicate_option_by_name
		(
		    arglex_token_directory,
		    new_branch_usage
		);
	    }

	    //
	    // To cope with automounters, directories are stored as
	    // given, or are derived from the home directory in the
	    // passwd file.  Within aegis, pathnames have their
	    // symbolic links resolved, and any comparison of paths
	    // is done on this "system idea" of the pathname.
	    //
	    devdir = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(new_branch_usage);
	    break;

	case arglex_token_output:
	    if (output_filename)
		duplicate_option(new_branch_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_output, new_branch_usage);
		// NOTREACHED

	    case arglex_token_string:
		output_filename = arglex_value.alv_string;
		break;

	    case arglex_token_stdio:
		output_filename = "";
		break;
	    }
	    break;

	case arglex_token_reason:
	    if (reason)
		duplicate_option(new_branch_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_string(arglex_token_reason, new_branch_usage);
		// NOTREACHED

	    case arglex_token_string:
	    case arglex_token_number:
		reason = str_from_c(arglex_value.alv_string);
		break;
	    }
	    break;
	}
	arglex();
    }
    if (change_number && output_filename)
    {
	mutually_exclusive_options
	(
	    arglex_token_change,
	    arglex_token_output,
	    new_branch_usage
	);
    }

    //
    // locate project data
    //
    if (!project_name)
	fatal_intl(0, i18n("no project name"));
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // make sure this branch of the project is still active
    //
    if (!change_is_a_branch(pp->change_get()))
	project_fatal(pp, 0, i18n("branch completed"));

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // Lock the project state file and the user state file.  There
    // is no need to lock the project state file of the new branch
    // as that file does not exist.
    //
    pp->pstate_lock_prepare();
    lock_take();

    //
    // it is an error if
    // the user is not an administrator for the project.
    //
    if (!project_administrator_query(pp, up->name()))
	project_fatal(pp, 0, i18n("not an administrator"));

    //
    // Add another row to the change table.
    //
    if (!change_number)
	change_number = project_next_change_number(pp, 0);
    else
    {
	if (project_change_number_in_use(pp, change_number))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", magic_zero_decode(change_number));
	    project_fatal(pp, scp, i18n("branch $number used"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }

    //
    // create the new branch
    //
    bp = project_new_branch(pp, up, change_number, devdir, reason);

    //
    // If there is an output option,
    // write the change number to the file.
    //
    if (output_filename)
    {
	string_ty	*content;

	content = str_format("%ld", magic_zero_decode(change_number));
	if (*output_filename)
	{
	    string_ty	    *fn;

	    fn = str_from_c(output_filename);
            user_ty::become scoped(up);
	    file_from_string(fn, content, 0644);
	    str_free(fn);
	}
	else
	    cat_string_to_stdout(content);
	str_free(content);
    }

    //
    // Write out the various files
    //
    change_cstate_write(bp->change_get());
    pp->pstate_write();

    //
    // release the locks
    //
    commit();
    lock_release();

    //
    // verbose success message
    //
    project_verbose_new_branch_complete(bp);
    project_free(bp);
    trace(("}\n"));
}


void
new_branch(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, new_branch_help, 0 },
	{ arglex_token_list, new_branch_list, 0 },
    };

    trace(("new_branch()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_branch_main);
    trace(("}\n"));
}
