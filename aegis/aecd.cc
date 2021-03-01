//
//	aegis - project change supervisor
//	Copyright (C) 1991-1997, 1999-2004 Peter Miller;
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
// MANIFEST: functions to change directory or determine paths
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/libintl.h>

#include <aecd.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <change/branch.h>
#include <error.h>
#include <help.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


static void
change_directory_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Change_Directory [ <option>... ][ <subdir> ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Change_Directory -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Change_Directory -Help\n", progname);
    quit(1);
}


static void
change_directory_help(void)
{
    help("aecd", change_directory_usage);
}


static void
change_directory_list(void)
{
    string_ty	    *project_name;

    trace(("change_directory_list()\n{\n"));
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(change_directory_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, change_directory_usage);
	    continue;
	}
	arglex();
    }
    list_changes_in_state_mask
    (
	project_name,
	(
	    (1 << cstate_state_being_developed)
	|
	    (1 << cstate_state_awaiting_review)
	|
	    (1 << cstate_state_being_reviewed)
	|
	    (1 << cstate_state_awaiting_integration)
	|
	    (1 << cstate_state_being_integrated)
	)
    );
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
change_directory_main(void)
{
    sub_context_ty  *scp;
    const char      *subdir =	    0;
    int		    devdir =	    0;
    cstate_ty	    *cstate_data;
    string_ty	    *d;
    int		    baseline =	    0;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    int		    trunk;
    int		    grandparent;
    const char      *branch;

    trace(("change_directory_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    trunk = 0;
    grandparent = 0;
    branch = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(change_directory_usage);
	    continue;

	case arglex_token_string:
	    if (subdir)
		fatal_intl(0, i18n("too many dir"));
	    subdir = arglex_value.alv_string;
	    if (!*subdir || *subdir == '/')
		fatal_intl(0, i18n("dir must be rel"));
	    break;

	case arglex_token_development_directory:
	    if (devdir)
		duplicate_option(change_directory_usage);
	    if (baseline)
	    {
		bad_combo:
		mutually_exclusive_options
		(
		    arglex_token_baseline,
		    arglex_token_development_directory,
		    change_directory_usage
		);
	    }
	    devdir = 1;
	    break;

	case arglex_token_baseline:
	    if (baseline)
		duplicate_option(change_directory_usage);
	    if (devdir)
		goto bad_combo;
	    baseline = 1;
	    break;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change_with_branch
	    (
		&project_name,
		&change_number,
		&branch,
		change_directory_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, change_directory_usage);
	    continue;

	case arglex_token_branch:
	    if (branch)
		duplicate_option(change_directory_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_number
		(
		    arglex_token_branch,
		    change_directory_usage
		);

	    case arglex_token_number:
	    case arglex_token_string:
		branch = arglex_value.alv_string;
		break;

	    case arglex_token_stdio:
		branch = "";
		break;
	    }
	    break;

	case arglex_token_trunk:
	    if (trunk)
		duplicate_option(change_directory_usage);
	    ++trunk;
	    break;

	case arglex_token_grandparent:
	    if (grandparent)
		duplicate_option(change_directory_usage);
	    ++grandparent;
	    break;
	}
	arglex();
    }

    //
    // reject illegal combinations of options
    //
    if (grandparent)
    {
	if (branch)
	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
		arglex_token_grandparent,
		change_directory_usage
	    );
	}
	if (trunk)
	{
	    mutually_exclusive_options
	    (
		arglex_token_trunk,
		arglex_token_grandparent,
		change_directory_usage
	    );
	}
	branch = "..";
    }
    if (trunk)
    {
	if (branch)
	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
		arglex_token_trunk,
		change_directory_usage
	    );
	}
	branch = "";
    }

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // locate the other branch
    //
    if (branch)
	pp = project_find_branch(pp, branch);

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // figure out where to go
    //
    if (baseline)
    {
	if (change_number)
	{
	    mutually_exclusive_options
	    (
		arglex_token_baseline,
		arglex_token_change,
		change_directory_usage
	    );
	}
	d = project_baseline_path_get(pp, 0);
	cp = 0;
    }
    else
    {
	//
	// locate change data
	//
	if (!change_number)
	    change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	cstate_data = change_cstate_get(cp);
	switch (cstate_data->state)
	{
	default:
	    change_fatal(cp, 0, i18n("bad cd, no dir"));

	case cstate_state_being_integrated:
	    if (!devdir)
	    {
		d = change_integration_directory_get(cp, 0);
		break;
	    }
	    d = change_development_directory_get(cp, 0);
	    break;

	case cstate_state_awaiting_review:
	    change_verbose(cp, 0, i18n("remember to use the aerb command"));
	    // fall through...

	case cstate_state_awaiting_integration:
	case cstate_state_being_reviewed:
	case cstate_state_being_developed:
	    d = change_development_directory_get(cp, 0);
	    break;
	}
    }

    //
    // Add in the extra path elements as necessary.
    // Flatten it out if they go up the tree (etc).
    //
    if (subdir)
    {
	string_ty	*tmp;

	tmp = str_format("%s/%s", d->str_text, subdir);
	user_become(up);
	d = os_pathname(tmp, 0);
	user_become_undo();
	str_free(tmp);
    }

    //
    // print out the path
    //	    (do NOT free d)
    //
    printf("%s\n", d->str_text);
    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", d);
    if (!cp)
	project_verbose(pp, scp, i18n("change directory $filename complete"));
    else
    {
	change_verbose(cp, scp, i18n("change directory $filename complete"));
	change_free(cp);
    }
    sub_context_delete(scp);

    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


void
change_directory(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, change_directory_help, },
	{arglex_token_list, change_directory_list, },
    };

    trace(("change_directory()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), change_directory_main);
    trace(("}\n"));
}
