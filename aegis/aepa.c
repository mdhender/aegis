/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to list and modify project attributes
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <aepa.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <io.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <pattr.h>
#include <project.h>
#include <project/pattr/edit.h>
#include <project/pattr/get.h>
#include <project/pattr/set.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void project_attributes_usage _((void));

static void
project_attributes_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s -Project_Attributes -File <attr-file> [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Project_Attributes -Edit [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Project_Attributes -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -Project_Attributes -Help\n", progname);
	quit(1);
}


static void project_attributes_help _((void));

static void
project_attributes_help()
{
	help("aepa", project_attributes_usage);
}


static void project_attributes_list _((void));

static void
project_attributes_list()
{
	pattr		pattr_data;
	string_ty	*project_name;
	project_ty	*pp;

	trace(("project_attributes_list()\n{\n"/*}*/));
	project_name = 0;
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(project_attributes_usage);
			continue;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, project_attributes_usage);
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				duplicate_option_by_name(arglex_token_project, project_attributes_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	pattr_data = (pattr)pattr_type.alloc();
	project_pattr_get(pp, pattr_data);
	pattr_write_file((char *)0, pattr_data, 0);
	pattr_type.free(pattr_data);
	project_free(pp);
	trace((/*{*/"}\n"));
}


static void check_permissions _((project_ty *, user_ty *));

static void
check_permissions(pp, up)
	project_ty	*pp;
	user_ty		*up;
{
	/*
	 * it is an error if the user is not an administrator
	 */
	if (!project_administrator_query(pp, user_name(up)))
		project_fatal(pp, 0, i18n("not an administrator"));
}


static void project_attributes_main _((void));

static void
project_attributes_main()
{
	sub_context_ty	*scp;
	pattr		pattr_data = 0;
	string_ty	*project_name;
	project_ty	*pp;
	edit_ty		edit;
	user_ty		*up;

	trace(("project_attributes_main()\n{\n"/*}*/));
	project_name = 0;
	edit = edit_not_set;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(project_attributes_usage);
			continue;

		case arglex_token_string:
			scp = sub_context_new();
			sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_file));
			error_intl(scp, i18n("warning: use $name option"));
			sub_context_delete(scp);
			if (pattr_data)
				fatal_intl(0, i18n("too many files"));
			goto read_input_file;

		case arglex_token_file:
			if (pattr_data)
				duplicate_option(project_attributes_usage);
			if (arglex() != arglex_token_string)
				option_needs_file(arglex_token_file, project_attributes_usage);
			read_input_file:
			os_become_orig();
			pattr_data = pattr_read_file(arglex_value.alv_string);
			os_become_undo();
			assert(pattr_data);
			break;

		case arglex_token_project:
			if (project_name)
				duplicate_option(project_attributes_usage);
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, project_attributes_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;

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
			user_lock_wait_argument(project_attributes_usage);
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
		sub_var_set(scp, "Name1", "%s", arglex_token_name(arglex_token_file));
		sub_var_set(scp, "Name2", "%s", arglex_token_name(arglex_token_edit));
		error_intl(scp, i18n("warning: no $name1, assuming $name2"));
		sub_context_delete(scp);
		edit = edit_foreground;
	}
	if (edit != edit_not_set && !pattr_data)
		pattr_data = (pattr)pattr_type.alloc();

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
	 * edit the attributes
	 */
	if (edit != edit_not_set)
	{
		/*
		 * make sure they are allowed to,
		 * to avoid a wasted edit
		 */
		check_permissions(pp, up);

		/*
		 * copy things from project
		 */
		project_pattr_get(pp, pattr_data);

		/*
		 * edit them
		 */
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%S", project_name_get(pp));
		io_comment_append(scp, "Project $name");
		sub_context_delete(scp);
		project_pattr_edit(&pattr_data, edit);
	}

	/*
	 * take project lock
	 */
	project_pstate_lock_prepare(pp);
	lock_take();

	/*
	 * make sure they are allowed to
	 * (even if edited, it could have changed while editing)
	 */
	check_permissions(pp, up);

	/*
	 * copy the attributes across
	 */
	project_pattr_set(pp, pattr_data);

	pattr_type.free(pattr_data);
	project_pstate_write(pp);
	commit();
	lock_release();
	project_verbose(pp, 0, i18n("project attributes complete"));
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
project_attributes()
{
	trace(("project_attributes()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		project_attributes_main();
		break;

	case arglex_token_help:
		project_attributes_help();
		break;

	case arglex_token_list:
		project_attributes_list();
		break;
	}
	trace((/*{*/"}\n"));
}
