/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to list and modify project attributes
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <aepa.h>
#include <arglex2.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <pattr.h>
#include <project.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void project_attributes_usage _((void));

static void
project_attributes_usage()
{
	char		*progname;

	progname = option_progname_get();
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
	static char *text[] =
	{
#include <../man1/aepa.h>
	};

	help(text, SIZEOF(text), project_attributes_usage);
}


static void pattr_copy _((pattr, pstate));

static void
pattr_copy(a, s)
	pattr	a;
	pstate	s;
{
	if (!a->description && s->description)
		a->description = str_copy(s->description);
	if (!a->owner_name && s->owner_name)
		a->owner_name = str_copy(s->owner_name);
	if (!a->group_name && s->group_name)
		a->group_name = str_copy(s->group_name);
	if
	(
		!a->default_development_directory
	&&
		s->default_development_directory
	)
		a->default_development_directory =
			str_copy(s->default_development_directory);

	if (!(a->mask & pattr_developer_may_review_mask))
		a->developer_may_review = s->developer_may_review;
	if (!(a->mask & pattr_developer_may_integrate_mask))
		a->developer_may_integrate = s->developer_may_integrate;
	if (!(a->mask & pattr_reviewer_may_integrate_mask))
		a->reviewer_may_integrate = s->reviewer_may_integrate;
	if (!(a->mask & pattr_developers_may_create_changes_mask))
		a->developers_may_create_changes =
			s->developers_may_create_changes;
	if (!(a->mask & pattr_umask_mask))
		a->umask = s->umask;
	if (!(a->mask & pattr_default_test_exemption_mask))
		a->default_test_exemption = s->default_test_exemption;
	
	if (!a->copyright_years && s->copyright_years)
	{
		size_t		j;

		a->copyright_years = pattr_copyright_years_list_type.alloc();
		for (j = 0; j < s->copyright_years->length; ++j)
		{
			long		*year_p;
			type_ty		*type_p;

			year_p =
				pattr_copyright_years_list_type.list_parse
				(
					a->copyright_years,
					&type_p
				);
			assert(type_p = &integer_type);
			*year_p = s->copyright_years->list[j];
		}
	}

	if
	(
		!a->forced_develop_begin_notify_command
	&&
		s->forced_develop_begin_notify_command
	)
		a->forced_develop_begin_notify_command =
			str_copy(s->forced_develop_begin_notify_command);
	if (!a->develop_end_notify_command && s->develop_end_notify_command)
		a->develop_end_notify_command =
			str_copy(s->develop_end_notify_command);
	if
	(
		!a->develop_end_undo_notify_command
	&&
		s->develop_end_undo_notify_command
	)
		a->develop_end_undo_notify_command =
			str_copy(s->develop_end_undo_notify_command);
	if
	(
		!a->review_pass_notify_command
	&&
		s->review_pass_notify_command
	)
		a->review_pass_notify_command =
			str_copy(s->review_pass_notify_command);
	if
	(
		!a->review_pass_undo_notify_command
	&&
		s->review_pass_undo_notify_command
	)
		a->review_pass_undo_notify_command =
			str_copy(s->review_pass_undo_notify_command);
	if
	(
		!a->review_fail_notify_command
	&&
		s->review_fail_notify_command
	)
		a->review_fail_notify_command =
			str_copy(s->review_fail_notify_command);
	if
	(
		!a->integrate_pass_notify_command
	&&
		s->integrate_pass_notify_command
	)
		a->integrate_pass_notify_command =
			str_copy(s->integrate_pass_notify_command);
	if
	(
		!a->integrate_fail_notify_command
	&&
		s->integrate_fail_notify_command
	)
		a->integrate_fail_notify_command =
			str_copy(s->integrate_fail_notify_command);
}


static void project_attributes_list _((void));

static void
project_attributes_list()
{
	pattr		pattr_data;
	pstate		pstate_data;
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
				project_attributes_usage();
			/* fall through... */

		case arglex_token_string:
			if (project_name)
				fatal("duplicate -Project option");
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

	pstate_data = project_pstate_get(pp);
	pattr_data = (pattr)pattr_type.alloc();
	pattr_copy(pattr_data, pstate_data);
	pattr_write_file((char *)0, pattr_data);
	pattr_type.free(pattr_data);
	project_free(pp);
	trace((/*{*/"}\n"));
}


static void pattr_edit _((pattr *));

static void
pattr_edit(dp)
	pattr		*dp;
{
	pattr		d;
	string_ty	*filename;
	string_ty	*msg;
	
	/*
	 * write attributes to temporary file
	 */
	d = *dp;
	assert(d);
	filename = os_edit_filename(1);
	os_become_orig();
	pattr_write_file(filename->str_text, d);
	pattr_type.free(d);

	/*
	 * error message to issue if anything goes wrong
	 */
	msg = str_format("attributes text left in the \"%S\" file", filename);
	undo_message(msg);
	str_free(msg);

	/*
	 * edit the file
	 */
	os_edit(filename);

	/*
	 * read it in again
	 */
	d = pattr_read_file(filename->str_text);
	commit_unlink_errok(filename);
	os_become_undo();
	str_free(filename);
	*dp = d;
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
	{
		project_fatal
		(
			pp,
		     "attributes may only be changed by a project administrator"
		);
	}
}


static void project_attributes_main _((void));

static void
project_attributes_main()
{
	pattr		pattr_data = 0;
	pstate		pstate_data;
	string_ty	*project_name;
	project_ty	*pp;
	string_ty	*s;
	int		edit;
	user_ty		*up;

	trace(("project_attributes_main()\n{\n"/*}*/));
	project_name = 0;
	edit = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(project_attributes_usage);
			continue;

		case arglex_token_string:
			error
			(
"warning: please use the -File option when specifying an attributes file, \
the unadorned form is now obsolescent"
			);
			if (pattr_data)
				fatal("too many files named");
			goto read_input_file;

		case arglex_token_file:
			if (pattr_data)
				goto duplicate;
			if (arglex() != arglex_token_string)
			{
				error
				(
				 "the -File option requires a filename argument"
				);
				project_attributes_usage();
			}
			read_input_file:
			os_become_orig();
			pattr_data = pattr_read_file(arglex_value.alv_string);
			os_become_undo();
			assert(pattr_data);
			break;

		case arglex_token_project:
			if (project_name)
				goto duplicate;
			if (arglex() != arglex_token_string)
				project_attributes_usage();
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_edit:
			if (edit)
			{
				duplicate:
				fatal
				(
					"duplicate %s option",
					arglex_value.alv_string
				);
			}
			edit++;
			break;
		}
		arglex();
	}
	if (!edit && !pattr_data)
	{
		error("warning: no -File specified, assuming -Editr desired");
		++edit;
	}
	if (edit && !pattr_data)
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
	if (edit)
	{
		/*
		 * make sure they are allowed to,
		 * to avoid a wasted edit
		 */
		check_permissions(pp, up);

		/*
		 * copy things from project
		 */
		pstate_data = project_pstate_get(pp);
		pattr_copy(pattr_data, pstate_data);

		/*
		 * edit them
		 */
		pattr_edit(&pattr_data);
	}

	/*
	 * take project lock
	 */
	project_pstate_lock_prepare(pp);
	lock_take();
	pstate_data = project_pstate_get(pp);

	/*
	 * make sure they are allowed to
	 * (even if edited, it could have changed while editing)
	 */
	check_permissions(pp, up);

	/*
	 * copy the attributes across
	 */
	if (pattr_data->description)
	{
		if (pstate_data->description)
			str_free(pstate_data->description);
		pstate_data->description = str_copy(pattr_data->description);
	}

	if (pattr_data->owner_name)
	{
		if (!user_uid_check(pattr_data->owner_name))
		{
			fatal
			(
				"user \"%s\" is too privileged",
				pattr_data->owner_name->str_text
			);
		}
		if (pstate_data->owner_name)
			str_free(pstate_data->owner_name);
		pstate_data->owner_name = str_copy(pattr_data->owner_name);
	}

	if (pattr_data->group_name)
	{
		if (!user_gid_check(pattr_data->group_name))
		{
			fatal
			(
				"group \"%s\" is too privileged",
				pattr_data->group_name->str_text
			);
		}
		if (pstate_data->group_name)
			str_free(pstate_data->group_name);
		pstate_data->group_name = str_copy(pattr_data->group_name);
	}

	if (pattr_data->mask & pattr_developer_may_review_mask)
		pstate_data->developer_may_review =
			pattr_data->developer_may_review;
	if (pattr_data->mask & pattr_developer_may_integrate_mask)
		pstate_data->developer_may_integrate =
			pattr_data->developer_may_integrate;
	if (pattr_data->mask & pattr_reviewer_may_integrate_mask)
		pstate_data->reviewer_may_integrate =
			pattr_data->reviewer_may_integrate;
	if (pattr_data->mask & pattr_developers_may_create_changes_mask)
		pstate_data->developers_may_create_changes =
			pattr_data->developers_may_create_changes;

	/*
	 * only some combos work,
	 * umask is basically for the "other" permissions
	 */
	if (pattr_data->mask & pattr_umask_mask)
		pstate_data->umask = (pattr_data->umask & 5) | 022;
	
	if (pattr_data->mask & pattr_default_test_exemption_mask)
		pstate_data->default_test_exemption =
			pattr_data->default_test_exemption;

	if (pattr_data->copyright_years)
	{
		size_t		j;

		if (pstate_data->copyright_years)
			pstate_copyright_years_list_type.free
			(
				pstate_data->copyright_years
			);
		pstate_data->copyright_years =
			pstate_copyright_years_list_type.alloc();
		for (j = 0; j < pattr_data->copyright_years->length; ++j)
		{
			long		*year_p;
			type_ty		*type_p;
			
			year_p =
				pstate_copyright_years_list_type.list_parse
				(
					pstate_data->copyright_years,
					&type_p
				);
			assert(type_p == &integer_type);
			*year_p = pattr_data->copyright_years->list[j];
		}
	}

	if (pattr_data->forced_develop_begin_notify_command)
	{
		if (pstate_data->forced_develop_begin_notify_command)
			str_free(pstate_data->forced_develop_begin_notify_command);
		pstate_data->forced_develop_begin_notify_command =
			str_copy(pattr_data->forced_develop_begin_notify_command);
	}

	if (pattr_data->develop_end_notify_command)
	{
		if (pstate_data->develop_end_notify_command)
			str_free(pstate_data->develop_end_notify_command);
		pstate_data->develop_end_notify_command =
			str_copy(pattr_data->develop_end_notify_command);
	}

	if (pattr_data->develop_end_undo_notify_command)
	{
		if (pstate_data->develop_end_undo_notify_command)
			str_free(pstate_data->develop_end_undo_notify_command);
		pstate_data->develop_end_undo_notify_command =
			str_copy(pattr_data->develop_end_undo_notify_command);
	}

	if (pattr_data->review_pass_notify_command)
	{
		if (pstate_data->review_pass_notify_command)
			str_free(pstate_data->review_pass_notify_command);
		pstate_data->review_pass_notify_command =
			str_copy(pattr_data->review_pass_notify_command);
	}

	if (pattr_data->review_pass_undo_notify_command)
	{
		if (pstate_data->review_pass_undo_notify_command)
			str_free(pstate_data->review_pass_undo_notify_command);
		pstate_data->review_pass_undo_notify_command =
			str_copy(pattr_data->review_pass_undo_notify_command);
	}

	if (pattr_data->review_fail_notify_command)
	{
		if (pstate_data->review_fail_notify_command)
			str_free(pstate_data->review_fail_notify_command);
		pstate_data->review_fail_notify_command =
			str_copy(pattr_data->review_fail_notify_command);
	}

	if (pattr_data->integrate_pass_notify_command)
	{
		if (pstate_data->integrate_pass_notify_command)
			str_free(pstate_data->integrate_pass_notify_command);
		pstate_data->integrate_pass_notify_command =
			str_copy(pattr_data->integrate_pass_notify_command);
	}

	if (pattr_data->integrate_fail_notify_command)
	{
		if (pstate_data->integrate_fail_notify_command)
			str_free(pstate_data->integrate_fail_notify_command);
		pstate_data->integrate_fail_notify_command =
			str_copy(pattr_data->integrate_fail_notify_command);
	}

	if (pattr_data->default_development_directory)
	{
		if (pstate_data->default_development_directory)
			str_free(pstate_data->default_development_directory);
		s = pattr_data->default_development_directory;
		if (!s->str_length)
			pstate_data->default_development_directory = 0;
		else
		{
			if (s->str_text[0] != '/')
			{
				fatal
				(
	"default development directory must be specified as an absolute path"
				);
			}
			pstate_data->default_development_directory =
				str_copy(s);
		}
	}

	pattr_type.free(pattr_data);
	project_pstate_write(pp);
	commit();
	lock_release();
	project_verbose(pp, "attributes changed");
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
