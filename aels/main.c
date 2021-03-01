/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate mains
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex2.h>
#include <change.h>
#include <env.h>
#include <error.h>
#include <help.h>
#include <language.h>
#include <list.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <r250.h>
#include <stack.h>
#include <sub.h>
#include <user.h>
#include <zero.h>

enum
{
	arglex_token_attr_hide = ARGLEX2_MAX,
	arglex_token_attr_show,
	arglex_token_dot_files_hide,
	arglex_token_dot_files_show,
	arglex_token_group_hide,
	arglex_token_group_show,
	arglex_token_long,
	arglex_token_mode_hide,
	arglex_token_mode_show,
	arglex_token_recursive,
	arglex_token_size_hide,
	arglex_token_size_show,
	arglex_token_user_hide,
	arglex_token_user_show,
	arglex_token_when_hide,
	arglex_token_when_show,
	ARGLEX3_MAX
};

static arglex_table_ty argtab[] =
{
	{ "-Hide_Attributes",	arglex_token_attr_hide,		},
	{ "-Show_Attributes",	arglex_token_attr_show,		},
	{ "-Hide_Dot_Files",	arglex_token_dot_files_hide,	},
	{ "-ALL",		arglex_token_dot_files_show,	},
	{ "-Show_Dot_Files",	arglex_token_dot_files_show,	},
	{ "-Hide_Group",	arglex_token_group_hide,	},
	{ "-Show_Group",	arglex_token_group_show,	},
	{ "-LOng",		arglex_token_long,		},
	{ "-Hide_Mode",		arglex_token_mode_hide,		},
	{ "-Show_Mode",		arglex_token_mode_show,		},
	{ "-RECursive",		arglex_token_recursive,		},
	{ "-Hide_Size",		arglex_token_size_hide,		},
	{ "-Show_Size",		arglex_token_size_show,		},
	{ "-Hide_User",		arglex_token_user_hide,		},
	{ "-Show_User",		arglex_token_user_show,		},
	{ "-Hide_When",		arglex_token_when_hide,		},
	{ "-Show_When",		arglex_token_when_show,		},
	ARGLEX_END_MARKER
};


static void list_usage _((void));

static void
list_usage()
{
	char		*progname;

	progname = progname_get();
	fprintf(stderr, "usage: %s [ <option>... ] width height\n", progname);
	fprintf(stderr, "       %s -Help\n", progname);
	quit(1);
}


static void list_help _((void));

static void
list_help()
{
	help((char *)0, list_usage);
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	long		change_number;
	string_ty	*project_name;
	string_list_ty	name;
	string_ty	*s;
	project_ty	*pp;
	change_ty	*cp;
	int		baseline;
	user_ty		*up;
	int		based;
	size_t		j;

	r250_init();
	os_become_init_mortal();
	arglex2_init3(argc, argv, argtab);
	str_initialize();
	env_initialize();
	language_init();

	if (arglex() == arglex_token_help)
		list_help();

	project_name = 0;
	change_number = 0;
	baseline = 0;
	string_list_constructor(&name);
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(list_usage);
			continue;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
			{
				option_needs_number
				(
					arglex_token_change,
					list_usage
				);
				/*NOTREACHED*/
			}
			/* fall through... */

		case arglex_token_number:
			if (change_number)
			{
				duplicate_option_by_name
				(
					arglex_token_change,
					list_usage
				);
				/*NOTREACHED*/
			}
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set_long(scp, "Number", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
			option_needs_name(arglex_token_project, list_usage);
			if (project_name)
			{
				duplicate_option_by_name
				(
					arglex_token_project,
					list_usage
				);
				/*NOTREACHED*/
			}
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_string:
			s = str_from_c(arglex_value.alv_string);
			string_list_append(&name, s);
			str_free(s);
			break;

		case arglex_token_base_relative:
		case arglex_token_current_relative:
			user_relative_filename_preference_argument(list_usage);
			break;

		case arglex_token_baseline:
			if (baseline)
				duplicate_option(list_usage);
			baseline = 1;
			break;

		case arglex_token_recursive:
			if (recursive_flag)
				duplicate_option(list_usage);
			recursive_flag = 1;
			break;

		case arglex_token_long:
			if (long_flag)
				duplicate_option(list_usage);
			long_flag = 1;
			break;

		case arglex_token_dot_files_show:
			show_dot_files = 1;
			break;

		case arglex_token_dot_files_hide:
			show_dot_files = 0;
			break;

		case arglex_token_mode_show:
			mode_flag = 1;
			break;

		case arglex_token_mode_hide:
			mode_flag = 0;
			break;

		case arglex_token_attr_show:
			attr_flag = 1;
			break;

		case arglex_token_attr_hide:
			attr_flag = 0;
			break;

		case arglex_token_user_show:
			user_flag = 1;
			break;

		case arglex_token_user_hide:
			user_flag = 0;
			break;

		case arglex_token_group_show:
			group_flag = 1;
			break;

		case arglex_token_group_hide:
			group_flag = 0;
			break;

		case arglex_token_size_show:
			size_flag = 1;
			break;

		case arglex_token_size_hide:
			size_flag = 0;
			break;

		case arglex_token_when_show:
			when_flag = 1;
			break;

		case arglex_token_when_hide:
			when_flag = 0;
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

	if (baseline)
	{
		if (change_number)
		{
			mutually_exclusive_options
			(
				arglex_token_branch,
				arglex_token_change,
				list_usage
			);
		}

		stack_from_project(pp);

		up = 0;
		cp = 0;
	}
	else
	{
		cstate		cstate_data;

		/*
		 * locate user data
		 */
		up = user_executing(pp);

		/*
		 * locate change data
		 */
		if (!change_number)
			change_number = user_default_change(up);
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
		cstate_data = change_cstate_get(cp);

		if (cstate_data->state == cstate_state_completed)
		{
			/*
			 * Get the search path from the project.
			 */
			stack_from_project(pp);

			up = 0;
			cp = 0;
		}
		else
		{
			/*
			 * It is an error if the change is not in the
			 * being_developed state (if it does not have a
			 * directory).
			 */
			if (cstate_data->state < cstate_state_being_developed)
				change_fatal(cp, 0, i18n("bad aels state"));

			/*
			 * Get the search path from the change.
			 */
			stack_from_change(cp);
		}
	}

	/*
	 * resolve the path of each path
	 * 1.	the absolute path of the file name is obtained
	 * 2.	if the file is inside the development directory, ok
	 * 3.	if the file is inside the baseline, ok
	 * 4.	if neither, error
	 */
	based =
		(
			up != 0
		&&
			user_relative_filename_preference
			(
				up,
				uconf_relative_filename_preference_current
			)
		==
			uconf_relative_filename_preference_base
		);
	if (!name.nstrings)
	{
		os_become_orig();
		string_list_append(&name, os_curdir());
		os_become_undo();
	}
	for (j = 0; j < name.nstrings; ++j)
	{
		string_ty	*s0;
		string_ty	*s1;
		string_ty	*s2;

		s0 = name.string[j];
		if (s0->str_text[0] == '/' || !based)
			s1 = str_copy(s0);
		else
			s1 = str_format("%S/%S", stack_nth(0), s0);
		str_free(s0);
		s2 = stack_relative(s1);
		if (!s2)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "File_Name", s1);
			if (cp)
				change_fatal(cp, scp, i18n("$filename unrelated"));
			project_fatal(pp, scp, i18n("$filename unrelated"));
			/* NOTREACHED */
		}
		assert(s2);
		str_free(s1);
		name.string[j] = s2;
	}

	/*
	 * emit the listing
	 */
	list(&name, pp, cp);

	/*
	 * report success
	 */
	exit(0);
	return 0;
}
