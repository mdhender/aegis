/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
#include <stdlib.h>

#include <arglex2.h>
#include <commit.h>
#include <conf.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <pattr.h>
#include <pattr_ed.h>
#include <project.h>
#include <trace.h>
#include <user.h>


static void project_attributes_usage _((void));

static void
project_attributes_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -Project_Attributes [ <option>... ]\n", progname);
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
"NAME",
"	%s -Project_Attributes - modify the attributes of a",
"	project",
"",
"SYNOPSIS",
"	%s -Project_Attributes <attr-file> [ <option>... ]",
"	%s -Project_Attributes -List [ <option>... ]",
"	%s -Project_Attributes -Help",
"",
"DESCRIPTION",
"	The %s -Project_Attributes command is used to modify",
"	the attributes of a project.  The best method is to use",
"	the -List option to get a copy of the current attributes",
"	(redirect it into a file), edit those attributes as you",
"	want them, and then feed it back as the attr-file.  See",
"	aepattr(5) for more information.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Help",
"		This option may be used to obtain more",
"		information about how to use the %s program.",
"",
"	-List",
"		This option may be used to obtain a list of",
"		suitable subjects for this command.  The list may",
"		be more general than expected.",
"",
"	-Project <name>",
"		This option may be used to select the project of",
"		interest.  When no -Project option is specified, the",
"		AEGIS_PROJECT environment variable is consulted.  If",
"		that does not exist, the user's $HOME/.aegisrc file",
"		is examined for a default project field (see",
"		aeuconf(5) for more information).  If that does not",
"		exist, when the user is only working on changes",
"		within a single project, the project name defaults",
"		to that project.  Otherwise, it is an error.",
"",
"	-TERse",
"		This option may be used to cause listings to",
"		produce the bare minimum of information.  It is",
"		usually useful for shell scripts.",
"",
"	-Verbose",
"		This option may be used to cause %s to produce",
"		more output.  By default %s only produces",
"		output on errors.  When used with the -List",
"		option this option causes column headings to be",
"		added.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"RECOMMENDED ALIAS",
"	The recommended alias for this command is",
"	csh%%	alias aepa '%s -pa \\!* -v'",
"	sh$	aepa(){%s -pa $* -v}",
"",
"ERRORS",
"	It is an error if the current user is not an",
"	administrator of the specified project.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.	The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), project_attributes_usage);
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
	if (pstate_data->description)
		pattr_data->description = str_copy(pstate_data->description);
	if (pstate_data->owner_name)
		pattr_data->owner_name = str_copy(pstate_data->owner_name);
	if (pstate_data->group_name)
		pattr_data->group_name = str_copy(pstate_data->group_name);
	if (pstate_data->default_development_directory)
		pattr_data->default_development_directory =
			str_copy(pstate_data->default_development_directory);

	pattr_data->developer_may_review = pstate_data->developer_may_review;
	pattr_data->developer_may_integrate =
		pstate_data->developer_may_integrate;
	pattr_data->reviewer_may_integrate =
		pstate_data->reviewer_may_integrate;
	pattr_data->developers_may_create_changes =
		pstate_data->developers_may_create_changes;
	pattr_data->umask = pstate_data->umask;
	pattr_data->default_test_exemption =
		pstate_data->default_test_exemption;

	if (pstate_data->develop_end_notify_command)
		pattr_data->develop_end_notify_command =
			str_copy(pstate_data->develop_end_notify_command);
	if (pstate_data->develop_end_undo_notify_command)
		pattr_data->develop_end_undo_notify_command =
			str_copy(pstate_data->develop_end_undo_notify_command);
	if (pstate_data->review_pass_notify_command)
		pattr_data->review_pass_notify_command =
			str_copy(pstate_data->review_pass_notify_command);
	if (pstate_data->review_pass_undo_notify_command)
		pattr_data->review_pass_undo_notify_command =
			str_copy(pstate_data->review_pass_undo_notify_command);
	if (pstate_data->review_fail_notify_command)
		pattr_data->review_fail_notify_command =
			str_copy(pstate_data->review_fail_notify_command);
	if (pstate_data->integrate_pass_notify_command)
		pattr_data->integrate_pass_notify_command =
			str_copy(pstate_data->integrate_pass_notify_command);
	if (pstate_data->integrate_fail_notify_command)
		pattr_data->integrate_fail_notify_command =
			str_copy(pstate_data->integrate_fail_notify_command);
	pattr_write_file((char *)0, pattr_data);
	pattr_type.free(pattr_data);
	project_free(pp);
	trace((/*{*/"}\n"));
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

	trace(("project_attributes_main()\n{\n"/*}*/));
	project_name = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(project_attributes_usage);
			continue;

		case arglex_token_string:
			if (pattr_data)
				fatal("too many files named");
			os_become_orig();
			pattr_data = pattr_read_file(arglex_value.alv_string);
			os_become_undo();
			assert(pattr_data);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				project_attributes_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!pattr_data)
		fatal("no project attributes file named");

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	project_pstate_lock_prepare(pp);
	lock_take();
	pstate_data = project_pstate_get(pp);

	if (pattr_data->description)
	{
		if (pstate_data->description)
			str_free(pstate_data->description);
		pstate_data->description =str_copy(pattr_data->description);
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
		s = pstate_data->default_development_directory;
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
