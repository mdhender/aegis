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

#include <common/error.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/help.h>
#include <libaegis/io.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/pattr.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/pattr/edit.h>
#include <libaegis/project/pattr/get.h>
#include <libaegis/project/pattr/set.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aepa.h>


static void
project_attributes_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Project_Attributes -File <attr-file> [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Project_Attributes -Edit [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Project_Attributes -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Project_Attributes -Help\n", progname);
    quit(1);
}


static void
project_attributes_help(void)
{
    help("aepa", project_attributes_usage);
}


static void
project_attributes_list(void)
{
    trace(("project_attributes_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(project_attributes_usage);
    if (cid.set())
        list_change_inappropriate();

    pattr_ty *pattr_data = (pattr_ty *)pattr_type.alloc();
    project_pattr_get(cid.get_pp(), pattr_data);
    pattr_write_file((string_ty *)0, pattr_data, 0);
    pattr_type.free(pattr_data);
    trace(("}\n"));
}


static void
check_permissions(project_ty *pp, user_ty::pointer up)
{
    //
    // it is an error if the user is not an administrator
    //
    if (!project_administrator_query(pp, up->name()))
	project_fatal(pp, 0, i18n("not an administrator"));
}


static void
change_existing_project_attributes(project_ty *pp, pattr_ty *pattr_data)
{
    user_ty::pointer up;

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // take project lock
    //
    pp->pstate_lock_prepare();
    lock_take();

    //
    // make sure they are allowed to
    // (even if edited, it could have changed while editing)
    //
    check_permissions(pp, up);

    //
    // copy the attributes across
    //
    project_pattr_set(pp, pattr_data);

    pp->pstate_write();
    commit();
    lock_release();
    project_verbose(pp, 0, i18n("project attributes complete"));
}


static void
project_attributes_main(void)
{
    string_ty	    *s;
    sub_context_ty  *scp;
    pattr_ty	    *pattr_data =    0;
    string_ty	    *project_name;
    project_ty	    *pp;
    edit_ty	    edit;
    int             recursive;

    trace(("project_attributes_main()\n{\n"));
    arglex();
    project_name = 0;
    edit = edit_not_set;
    recursive = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(project_attributes_usage);
	    continue;

	case arglex_token_project_recursive:
	    recursive = 1;
	    break;

	case arglex_token_string:
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name",
		arglex_token_name(arglex_token_file)
	    );
	    error_intl(scp, i18n("warning: use $name option"));
	    sub_context_delete(scp);
	    if (pattr_data)
		fatal_too_many_files();
	    goto read_input_file;

	case arglex_token_file:
	    if (pattr_data)
		duplicate_option(project_attributes_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, project_attributes_usage);
		// NOTREACHED

	    case arglex_token_string:
		read_input_file:
		os_become_orig();
		s = str_from_c(arglex_value.alv_string);
		pattr_data = pattr_read_file(s);
		str_free(s);
		os_become_undo();
		break;

	    case arglex_token_stdio:
		os_become_orig();
		pattr_data = pattr_read_file((string_ty *)0);
		os_become_undo();
		break;
	    }
	    assert(pattr_data);
	    break;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, project_attributes_usage);
	    continue;

	case arglex_token_edit:
	    if (edit == edit_foreground)
		duplicate_option(project_attributes_usage);
	    if (edit != edit_not_set)
	    {
		too_many_edits:
		mutually_exclusive_options
		(
		    arglex_token_edit,
		    arglex_token_edit_bg,
		    project_attributes_usage
		);
	    }
	    edit = edit_foreground;
	    break;

	case arglex_token_edit_bg:
	    if (edit == edit_background)
		duplicate_option(project_attributes_usage);
	    if (edit != edit_not_set)
		goto too_many_edits;
	    edit = edit_background;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(project_attributes_usage);
	    break;
	}
	arglex();
    }
    if (edit != edit_not_set && pattr_data)
    {
	mutually_exclusive_options
	(
	    (
		edit == edit_foreground
	    ?
		arglex_token_edit
	    :
		arglex_token_edit_bg
	    ),
	    arglex_token_file,
	    project_attributes_usage
	);
    }
    if (edit == edit_not_set && !pattr_data)
    {
	scp = sub_context_new();
	sub_var_set_charstar
	(
	    scp,
	    "Name1",
	    arglex_token_name(arglex_token_file)
	);
	sub_var_set_charstar
	(
	    scp,
	    "Name2",
	    arglex_token_name(arglex_token_edit)
	);
	error_intl(scp, i18n("warning: no $name1, assuming $name2"));
	sub_context_delete(scp);
	edit = edit_foreground;
    }
    if (edit != edit_not_set && !pattr_data)
	pattr_data = (pattr_ty *)pattr_type.alloc();

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
    // edit the attributes
    //
    if (edit != edit_not_set)
    {
        user_ty::pointer up;

        up = user_ty::create();

	//
	// make sure they are allowed to,
	// to avoid a wasted edit
	//
	check_permissions(pp, up);

	//
	// copy things from project
	//
	project_pattr_get(pp, pattr_data);

	//
	// edit them
	//
	scp = sub_context_new();
	sub_var_set_string(scp, "Name", project_name_get(pp));
	io_comment_append(scp, "Project $name");
	sub_context_delete(scp);
	project_pattr_edit(&pattr_data, edit);
    }

    if (recursive)
    {
	string_list_ty pl;
	pp->list_inner(pl);
	for (size_t j = 0; j < pl.nstrings; ++j)
	{
	    project_ty      *branch;

	    branch = project_alloc(pl.string[j]);
	    branch->bind_existing();
	    change_existing_project_attributes(branch, pattr_data);
	    project_free(branch);
	}
    }
    else
    {
	change_existing_project_attributes(pp,  pattr_data);
    }
    pattr_type.free(pattr_data);
    project_free(pp);
    trace(("}\n"));
}


void
project_attributes(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, project_attributes_help, 0 },
	{ arglex_token_list, project_attributes_list, 0 },
    };

    trace(("project_attributes()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), project_attributes_main);
    trace(("}\n"));
}
