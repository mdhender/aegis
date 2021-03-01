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
 * MANIFEST: list interesting things about changes and projects
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <arglex2.h>
#include <col.h>
#include <change.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <list.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>


typedef struct table_ty table_ty;
struct table_ty
{
	char	*name;
	char	*description;
	void	(*func)_((string_ty *, long));
};


static	table_ty	table[] =
{
	{
		"Administrators",
		"List the administrators of a project",
		list_administrators,
	},
	{
		"Change_Details",
		"List all information about a change in large layout form.",
		list_change_details,
	},
	{
		"Change_Files",
		"List all files in a change",
		list_change_files,
	},
	{
		"Change_History",
		"List the history of a change",
		list_change_history,
	},
	{
		"Changes",
		"List the changes of a project",
		list_changes,
	},
	{
		"Developers",
		"List the developers of a project",
		list_developers,
	},
	{
		"Integrators",
		"List the integrators of a project",
		list_integrators,
	},
	{
		"List_List",
		"List all lists available",
		list_list_list,
	},
	{
		"Project_Files",
		"List all files in the baseline of a project",
		list_project_files,
	},
	{
		"Project_History",
		"List the integration history of a project",
		list_project_history,
	},
	{
		"Projects",
		"List all projects",
		list_projects,
	},
	{
		"Reviewers",
		"List the reviewers of a project",
		list_reviewers,
	},
	{
		"Users_Changes",
		"List of changes owned by the current user",
		list_user_changes,

	},
	{
		"Version",
		"List version of a project or change",
		list_version,
	},
};


static void list_usage _((void));

static void
list_usage()
{
	char		*progname;

	progname = option_get_progname();
	fprintf(stderr, "usage: %s -List [ <option>... ] <listname>\n", progname);
	fprintf(stderr, "       %s -List -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -List -Help\n", progname);
	quit(1);
}


static void list_help _((void));

static void
list_help()
{
	static char *text[] =
	{
"NAME",
"	%s -List - list (possibly) interesting things",
"",
"SYNOPSIS",
"	%s -List [ <option>... ] <list-name>",
"	%s -List -List [ <option>... ]",
"	%s -List -Help",
"",
"DESCRIPTION",
"	The %s -List command is used to list information.",
"	There are a number of possible list-names, as follows",
"	(abbreviations as for command line options):",
"",
"	Administrators",
"		List the administrators of a project.",
"",
"	Change_Details",
"		List full information about a change in large",
"		format.",
"",
"	Change_Files",
"		List all files in a change.",
"",
"	Change_History",
"		List the history of a change.",
"",
"	Changes",
"		List the changes of a project.",
"",
"	Developers",
"		List the developers of a project.",
"",
"	Integrators",
"		List the integrators of a project.",
"",
"	List_List",
"		List all lists available.",
"",
"	Project_Files",
"		List all files in the baseline of a project.",
"",
"	Project_History",
"		List the integration history of a project.",
"",
"	Projects",
"		List all projects.",
"",
"	Reviewers",
"		List the reviewers of a project.",
"",
"	Users_Changes",
"		List of changes owned by the current user.",
"",
"	Version",
"		List version of a project or change.",
"",
"	Most of these lists are available from other %s",
"	functions.  Many %s functions provide more specific",
"	lists.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Change <number>",
"		This option may be used to specify a particular",
"		change within a project.  When no -Change option",
"		is specified, the AEGIS_CHANGE environment",
"		variable is consulted.  If that does not exist,",
"		the user's $HOME/.%src file is examined for a",
"		default change field (see aeuconf(5) for more",
"		information).  If that does not exist, when the",
"		user is only working on one change within a",
"		project, that is the default change number.",
"		Otherwise, it is an error.",
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
"		interest.  When no -Project option is specified,",
"		the AEGIS_PROJECT environment variable is",
"		consulted.  If that does not exist, the user's",
"		$HOME/.%src file is examined for a default",
"		project field (see aeuconf(5) for more",
"		information).  If that does not exist, when the",
"		user is only working on changes within a single",
"		project, the project name defaults to that",
"		project.  Otherwise, it is an error.",
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
"	-Page_Length <number>",
"		This option may be used to set the page length of",
"		listings.  The default, in order of preference,",
"		is obtained from the system, from the LINES",
"		environment variable, or set to 24 lines.",
"",
"	-Page_Width <number>",
"		This option may be used to set the page width of",
"		listings and error messages.  The default, in",
"		order of preference, is obtained from the system,",
"		from the COLS environment variable, or set to 79",
"		characters.",
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
"	csh%%	alias ael '%s -l \\!* -v'",
"	sh$	ael(){%s -l $* -v}",
"",
"ERRORS",
"	It is an error if the list name given is unknown.",
"",
"EXIT STATUS",
"	The %s command will exit with a status of 1 on any",
"	error.  The %s command will only exit with a status of",
"	0 if there are no errors.",
"",
"COPYRIGHT",
"	%C",
"",
"AUTHOR",
"	%A",
	};

	help(text, SIZEOF(text), list_usage);
}


static void list_list _((void));

static void
list_list()
{
	trace(("list_list()\n{\n"/*}*/));
	arglex();
	while (arglex_token != arglex_token_eoln)
		generic_argument(list_usage);
	list_list_list(0, 0);
	trace((/*{*/"}\n"));
}


static void list_main _((void));

static void
list_main()
{
	char		*listname;
	table_ty		*tp;
	int		j;
	table_ty		*hit[SIZEOF(table)];
	int		nhit;
	string_ty	*s1;
	string_ty	*s2;
	string_ty	*project_name;
	long		change_number;

	trace(("list_main()\n{\n"/*}*/));
	listname = 0;
	project_name = 0;
	change_number = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(list_usage);
			continue;

		case arglex_token_string:
			if (listname)
				fatal("too many lists named");
			listname = arglex_value.alv_string;
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				list_usage();
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				fatal("duplicate -Change option");
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				list_usage();
			if (project_name)
				fatal("duplicate -Project option");
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!listname)
		fatal("no list name given");

	nhit = 0;
	for (tp = table; tp < ENDOF(table); ++tp)
	{
		if (arglex_compare(tp->name, listname))
			hit[nhit++] = tp;
	}
	switch (nhit)
	{
	case 0:
		fatal("list name \"%s\" unknown", listname);

	case 1:
		hit[0]->func(project_name, change_number);
		break;

	default:
		s1 = str_from_c(hit[0]->name);
		for (j = 1; j < nhit; ++j)
		{
			s2 = str_format("%S, %s", s1, hit[j]->name);
			str_free(s1);
			s1 = s2;
		}
		fatal("list name \"%s\" ambiguous (%s)", listname, s1->str_text);
	}
	if (project_name)
		str_free(project_name);
	trace((/*{*/"}\n"));
}


void
list()
{
	trace(("list()\n{\n"/*}*/));
	switch (arglex())
	{
	default:
		list_main();
		break;

	case arglex_token_help:
		list_help();
		break;

	case arglex_token_list:
		list_list();
		break;
	}
	trace((/*{*/"}\n"));
}


static int name_cmp _((const void *, const void *));

static int
name_cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	string_ty	*a;
	string_ty	*b;

	a = *(string_ty **)va;
	b = *(string_ty **)vb;
	return strcmp(a->str_text, b->str_text);
}



void
list_projects(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	wlist		name;

	trace(("list_projects()\n{\n"/*}*/));
	if (project_name)
		fatal("inappropriate -Project option");
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * list the projects
	 */
	gonzo_project_list(&name);
	if (!name.wl_nwords)
		verbose("there are no projects");
	else
	{
		int	name_col;
		int	dir_col;
		int	desc_col;
		int	j;

		qsort
		(
			name.wl_word,
			name.wl_nwords,
			sizeof(name.wl_word[0]),
			name_cmp
		);
		col_open((char *)0);
		col_title("List of Projects", (char *)0);
		name_col = col_create(0, 15);
		dir_col = col_create(16, 39);
		desc_col = col_create(40, 0);
		col_heading(name_col, "Project\n---------");
		col_heading(dir_col, "Directory\n-----------");
		col_heading(desc_col, "Description\n-------------");
		for (j = 0; j < name.wl_nwords; ++j)
		{
			project_ty	*pp;
			int		err;

			pp = project_alloc(name.wl_word[j]);
			project_bind_existing(pp);
			col_puts(name_col, project_name_get(pp)->str_text);
			col_puts(dir_col, project_home_path_get(pp)->str_text);
			os_become_orig();
			err = os_readable(project_pstate_path_get(pp));
			os_become_undo();
			if (err)
				col_puts(desc_col, strerror(err));
			else
			{
				col_puts
				(
					desc_col,
					project_pstate_get(pp)->description->
						str_text
				);
			}
			project_free(pp);
			col_eoln();
		}
		col_close();
	}
	trace((/*{*/"}\n"));
}


void
list_project_files(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	pstate		pstate_data;
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;

	/*
	 * locate project data
	 */
	trace(("list_project_files()\n{\n"/*}*/));
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * locate change data
	 */
	if (change_number)
	{
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
	}
	else
		cp = 0;

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->src);
	if (pstate_data->src->length)
	{
		int	usage_col = -1;
		int	edit_col = -1;
		int	file_name_col = -1;
		int	j;

		if (!option_get_terse())
		{
			string_ty	*line1;

			col_open((char *)0);
			if (change_number)
				line1 = str_format("Project \"%S\"  Change %ld", project_name, change_number);
			else
				line1 = str_format("Project \"%S\"", project_name);
			col_title(line1->str_text, "List of Project's Files");
			str_free(line1);

			usage_col = col_create(0, 7);
			edit_col = col_create(8, 15);
			file_name_col = col_create(16, 0);
			col_heading(usage_col, "Type\n-------");
			col_heading(edit_col, "Edit\n-------");
			col_heading(file_name_col, "File Name\n-----------");
		}
		for (j = 0; j < pstate_data->src->length; ++j)
		{
			pstate_src	src_data;

			src_data = pstate_data->src->list[j];
			if
			(
				(
					src_data->about_to_be_created_by
				||
					src_data->deleted_by
				)
			&&
				!option_get_verbose()
			)
				continue;
			if (cp && change_src_find(cp, src_data->file_name))
				continue;
			if (option_get_terse())
			{
				assert(src_data->file_name);
				printf("%s\n", src_data->file_name->str_text);
			}
			else
			{
				col_puts
				(
					usage_col,
					file_usage_ename(src_data->usage)
				);
				if (src_data->edit_number)
				{
					col_puts
					(
						edit_col,
						src_data->edit_number->str_text
					);
				}
				assert(src_data->file_name);
				col_puts
				(
					file_name_col,
					src_data->file_name->str_text
				);
				if (src_data->about_to_be_created_by)
				{
					col_bol(file_name_col);
					col_printf
					(
						file_name_col,
					   "About to be created by change %ld.",
						src_data->about_to_be_created_by
					);
				}
				if (src_data->deleted_by)
				{
					col_bol(file_name_col);
					col_printf
					(
						file_name_col,
						"Deleted by change %ld.",
						src_data->deleted_by
					);
				}
				if (src_data->locked_by)
				{
					col_bol(file_name_col);
					col_printf
					(
						file_name_col,
						"Locked by change %ld.",
						src_data->locked_by
					);
				}
				col_eoln();
			}
		}
		if (!option_get_terse())
			col_close();
	}
	else
		project_verbose(pp, "no files");
	project_free(pp);
	if (cp)
		change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
list_administrators(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	pstate		pstate_data;
	project_ty	*pp;

	trace(("list_administrators()\n{\n"/*}*/));
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->administrator);
	if (option_get_terse())
	{
		int	j;

		for (j = 0; j < pstate_data->administrator->length; ++j)
		{
			printf
			(
				"%s\n",
				pstate_data->administrator->list[j]->str_text
			);
		}
	}
	else if (pstate_data->administrator->length)
	{
		int		login_col;
		int		name_col;
		int		j;
		string_ty	*line1;

		col_open((char *)0);
		line1 = str_format("Project \"%S\"", project_name);
		col_title(line1->str_text, "List of Administrators");
		str_free(line1);

		login_col = col_create(0, 15);
		name_col = col_create(16, 0);
		col_heading(login_col, "User\n------");
		col_heading(name_col, "Full Name\n-----------");
		for (j = 0; j < pstate_data->administrator->length; ++j)
		{
			string_ty	*logname;

			logname = pstate_data->administrator->list[j];
			col_puts(login_col, logname->str_text);
			col_puts(name_col, user_full_name(logname));
			col_eoln();
		}
		col_close();
	}
	else
		project_verbose(pp, "no administrators");
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_change_files(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	cstate		cstate_data;
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;

	/*
	 * locate project data
	 */
	trace(("list_change_files()\n{\n"/*}*/));
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

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
	assert(cstate_data->src);
	if (cstate_data->src->length)
	{
		int	usage_col = -1;
		int	action_col = -1;
		int	edit_col = -1;
		int	file_name_col = -1;
		int	j;

		if (!option_get_terse())
		{
			string_ty	*line1;

			col_open((char *)0);
			line1 = str_format("Project \"%S\"  Change %ld", project_name, change_number);
			col_title(line1->str_text, "List of Change's Files");
			str_free(line1);

			usage_col = col_create(0, 7);
			action_col = col_create(8, 15);
			edit_col = col_create(16, 23);
			file_name_col = col_create(24, 0);
			col_heading(usage_col, "Type\n-------");
			col_heading(action_col, "Action\n-------");
			col_heading(edit_col, "Edit\n-------");
			col_heading(file_name_col, "File Name\n-----------");
		}
		for (j = 0; j < cstate_data->src->length; ++j)
		{
			cstate_src	src_data;

			src_data = cstate_data->src->list[j];
			assert(src_data->file_name);
			if (option_get_terse())
			{
				if (src_data->action != file_action_remove)
					printf("%s\n", src_data->file_name->str_text);
			}
			else
			{
				col_puts
				(
					usage_col,
					file_usage_ename(src_data->usage)
				);
				col_puts
				(
					action_col,
					file_action_ename(src_data->action)
				);
				if (src_data->edit_number)
				{
					pstate_src	psrc_data;

					col_puts
					(
						edit_col,
						src_data->edit_number->str_text
					);
					if
					(
						cstate_data->state
					==
						cstate_state_being_developed
					)
						psrc_data =
							project_src_find
							(
								pp,
							     src_data->file_name
							);
					else
						psrc_data = 0;
					if
					(
						psrc_data
					&&
						psrc_data->edit_number
					&&
						!str_equal
						(
							src_data->edit_number,
							psrc_data->edit_number
						)
					)
					{
						col_printf
						(
							edit_col,
							" (%s)",
						psrc_data->edit_number->str_text
						);
					}
				}
				col_puts
				(
					file_name_col,
					src_data->file_name->str_text
				);
				col_eoln();
			}
		}
		if (!option_get_terse())
			col_close();
	}
	else
		change_verbose(cp, "no files");
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
}

#define HOURS_PER_WORKING_DAY 7.5
#define SECONDS_PER_WORKING_DAY (long)(HOURS_PER_WORKING_DAY * 60L * 60L)
#define SECONDS_PER_DAY (24L * 60L * 60L)
#define ELAPSED_TIME_THRESHOLD (10L * 60L) /* ten minutes */

static double working_days _((time_t, time_t));

static double
working_days(start, finish)
	time_t		start;
	time_t		finish;
{
	time_t		duration;
	double		working_days_frac;
	long		working_days;
	long		ndays;
	long		nweeks;
	struct tm 	*tm;

	/*
	 * elapsed time in seconds
	 */
	trace(("working_days(start = %ld, finish = %ld)\n{\n"/*}*/, start,
		finish));
	trace(("start = %s", ctime(&start)));
	trace(("finish = %s", ctime(&finish)));
	duration = finish - start;
	if (duration < 0)
		duration = 0;
	trace(("duration = %ld;\n", duration));

	/*
	 * determine the number of whole calendar days
	 */
	ndays =
		(
			(duration + (SECONDS_PER_DAY - SECONDS_PER_WORKING_DAY))
		/
			SECONDS_PER_DAY
		);
	trace(("ndays = %ld;\n", ndays));

	/*
	 * determine the fractional part
	 */
	working_days_frac =
		(
			(double)(duration - ndays * SECONDS_PER_DAY)
		/
			SECONDS_PER_WORKING_DAY
		);
	trace(("working_days_frac = %g;\n", working_days_frac));

	/*
	 * 5 working days per working week
	 */
	nweeks = ndays / 7;
	working_days = 5 * nweeks;
	ndays -= nweeks * 7;
	working_days += ndays;
	trace(("working_days = %ld;\n", working_days));

	/*
	 * give credit for working over the weekend
	 */
	tm = localtime(&start);
	ndays += tm->tm_wday;
	trace(("ndays = %ld;\n", ndays));
	if (ndays >= 7 && tm->tm_wday != 6)
		working_days--;
	if (ndays >= 8)
		working_days--;
	if (working_days < 0)
		working_days = 0;
	trace(("working_days = %ld;\n", working_days));

	/*
	 * done
	 */
	working_days_frac += working_days;
	if (working_days_frac < 0)
		working_days_frac = 0;
	trace(("return %.10g;\n", working_days_frac));
	trace((/*{*/"}\n"));
	return working_days_frac;
}


void
list_change_history(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	cstate		cstate_data;
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;

	/*
	 * locate project data
	 */
	trace(("list_change_history()\n{\n"/*}*/));
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

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
	assert(cstate_data->src);
	if (cstate_data->history->length)
	{
		int		what_col;
		int		when_col;
		int		who_col;
		int		why_col;
		int		j;
		string_ty	*line1;

		col_open((char *)0);
		line1 = str_format("Project \"%S\"  Change %ld", project_name, change_number);
		col_title(line1->str_text, "History");
		str_free(line1);

		what_col = col_create(0, 15);
		when_col = col_create(16, 31);
		who_col = col_create(32, 39);
		why_col = col_create(40, 0);
		col_heading(what_col, "What\n------");
		col_heading(when_col, "When\n------");
		col_heading(who_col, "Who\n-----");
		col_heading(why_col, "Comment\n---------");
		for (j = 0; j < cstate_data->history->length; ++j)
		{
			cstate_history	history_data;
			time_t		t;

			history_data = cstate_data->history->list[j];
			col_puts
			(
				what_col,
				cstate_history_what_ename(history_data->what)
			);
			t = history_data->when;
			col_puts(when_col, ctime(&t));
			col_puts(who_col, history_data->who->str_text);
			if (history_data->why)
				col_puts(why_col, history_data->why->str_text);
			if (history_data->what != cstate_history_what_integrate_pass)
			{
				time_t	finish;

				if (j + 1 < cstate_data->history->length)
					finish = cstate_data->history->list[j + 1]->when;
				else
					time(&finish);
				if (finish - t >= ELAPSED_TIME_THRESHOLD)
				{
					col_bol(why_col);
					col_printf
					(
						why_col,
						"Elapsed time: %5.3f days.\n",
						working_days(t, finish)
					);
				}
			}
			col_eoln();
		}
		col_close();
	}
	else
		change_verbose(cp, "no history");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
list_changes_in_state_mask(project_name, state_mask)
	string_ty	*project_name;
	int		state_mask;
{
	pstate		pstate_data;
	int		n;
	int		number_col = 0;
	int		description_col = 0;
	int		state_col = 0;
	int		j;
	project_ty	*pp;

	/*
	 * locate project data
	 */
	trace(("list_changes_in_state_mask(state_mask = 0x%X)\n{\n"/*}*/, state_mask));
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	pstate_data = project_pstate_get(pp);
	n = 0;
	for (j = 0; j < pstate_data->change->length; ++j)
	{
		cstate		cstate_data;
		long		change_number;
		change_ty	*cp;

		change_number = pstate_data->change->list[j];
		if (option_get_terse())
		{
			printf("%ld\n", change_number);
			continue;
		}
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
		cstate_data = change_cstate_get(cp);
		if (state_mask & (1 << cstate_data->state))
		{
			if (!n)
			{
				string_ty	*line1;

				col_open((char *)0);
				line1 = str_format("Project \"%S\"", project_name);
				col_title(line1->str_text, "List of Changes");
				str_free(line1);

				number_col = col_create(0, 7);
				state_col = col_create(8, 23);
				description_col = col_create(24, 0);
				col_heading(number_col, "Change\n-------");
				col_heading(state_col, "State\n-------");
				col_heading(description_col, "Description\n-------------");
			}
			col_printf(number_col, "%4ld", change_number);
			col_puts
			(
				state_col,
				cstate_state_ename(cstate_data->state)
			);
			if (cstate_data->brief_description)
				col_puts
				(
					description_col,
					cstate_data->brief_description->str_text
				);
			col_eoln();
			++n;
		}
		change_free(cp);
	}
	if (n)
		col_close();
	else
		project_verbose(pp, "no changes");
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_changes(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	trace(("list_changes()\n{\n"/*}*/));
	if (change_number)
		fatal("inappropriate -Change option");
	list_changes_in_state_mask(project_name, ~0);
	trace((/*{*/"}\n"));
}


void
list_list_list(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	int		name_col;
	int		desc_col;
	table_ty		*tp;

	trace(("list_list_list()\n{\n"/*}*/));
	if (project_name)
		fatal("inappropriate -Project option");
	if (change_number)
		fatal("inappropriate -Change option");
	col_open((char *)0);
	col_title("List of Lists", (char *)0);
	name_col = col_create(0, 15);
	desc_col = col_create(16, 0);
	col_heading(name_col, "Name\n------");
	col_heading(desc_col, "Description\n-------------");
	for (tp = table; tp < ENDOF(table); ++tp)
	{
		col_puts(name_col, tp->name);
		col_puts(desc_col, tp->description);
		col_eoln();
	}
	col_close();
	trace((/*{*/"}\n"));
}


void
list_developers(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	pstate		pstate_data;
	project_ty	*pp;

	trace(("list_developers()\n{\n"/*}*/));
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->developer);
	if (option_get_terse())
	{
		int	j;

		for (j = 0; j < pstate_data->developer->length; ++j)
		{
			printf
			(
				"%s\n",
				pstate_data->developer->list[j]->str_text
			);
		}
	}
	else if (pstate_data->developer->length)
	{
		int		login_col;
		int		name_col;
		int		j;
		string_ty	*line1;

		col_open((char *)0);
		line1 = str_format("Project \"%S\"", project_name);
		col_title(line1->str_text, "List of Developers");
		str_free(line1);

		login_col = col_create(0, 15);
		name_col = col_create(16, 0);
		col_heading(login_col, "User\n------");
		col_heading(name_col, "Full Name\n-----------");
		for (j = 0; j < pstate_data->developer->length; ++j)
		{
			string_ty	*logname;

			logname = pstate_data->developer->list[j];
			col_puts(login_col, logname->str_text);
			col_puts(name_col, user_full_name(logname));
			col_eoln();
		}
		col_close();
	}
	else
		project_verbose(pp, "no developers");
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_integrators(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	pstate		pstate_data;
	project_ty	*pp;

	trace(("list_integrators()\n{\n"/*}*/));
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->integrator);
	if (option_get_terse())
	{
		int	j;

		for (j = 0; j < pstate_data->integrator->length; ++j)
		{
			printf
			(
				"%s\n",
				pstate_data->integrator->list[j]->str_text
			);
		}
	}
	else if (pstate_data->integrator->length)
	{
		int		login_col;
		int		name_col;
		int		j;
		string_ty	*line1;

		col_open((char *)0);
		line1 = str_format("Project \"%S\"", project_name);
		col_title(line1->str_text, "List of Integrators");
		str_free(line1);

		login_col = col_create(0, 15);
		name_col = col_create(16, 0);
		col_heading(login_col, "User\n------");
		col_heading(name_col, "Full Name\n-----------");
		for (j = 0; j < pstate_data->integrator->length; ++j)
		{
			string_ty	*logname;

			logname = pstate_data->integrator->list[j];
			col_puts(login_col, logname->str_text);
			col_puts(name_col, user_full_name(logname));
			col_eoln();
		}
		col_close();
	}
	else
		project_verbose(pp, "no integrators");
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_reviewers(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	pstate		pstate_data;
	project_ty	*pp;

	trace(("list_reviewers()\n{\n"/*}*/));
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->reviewer);
	if (option_get_terse())
	{
		int	j;

		for (j = 0; j < pstate_data->reviewer->length; ++j)
		{
			printf
			(
				"%s\n",
				pstate_data->reviewer->list[j]->str_text
			);
		}
	}
	else if (pstate_data->reviewer->length)
	{
		int		login_col;
		int		name_col;
		int		j;
		string_ty	*line1;

		col_open((char *)0);
		line1 = str_format("Project \"%S\"", project_name);
		col_title(line1->str_text, "List of Reviewers");
		str_free(line1);

		login_col = col_create(0, 15);
		name_col = col_create(16, 0);
		col_heading(login_col, "User\n------");
		col_heading(name_col, "Full Name\n-----------");
		for (j = 0; j < pstate_data->reviewer->length; ++j)
		{
			string_ty	*logname;

			logname = pstate_data->reviewer->list[j];
			col_puts(login_col, logname->str_text);
			col_puts(name_col, user_full_name(logname));
			col_eoln();
		}
		col_close();
	}
	else
		project_verbose(pp, "no reviewers");
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_project_history(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	pstate		pstate_data;
	int		n;
	int		delta_col = 0;
	int		date_col = 0;
	int		change_col = 0;
	int		description_col = 0;
	int		j;
	project_ty	*pp;

	trace(("list_project_history()\n{\n"/*}*/));
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	pstate_data = project_pstate_get(pp);
	n = 0;
	for (j = 0; j < pstate_data->history->length; ++j)
	{
		cstate		cstate_data;
		pstate_history	history_data;
		time_t		t;
		change_ty	*cp;

		history_data = pstate_data->history->list[j];
		if (option_get_terse())
		{
			printf("%ld\n", history_data->change_number);
			continue;
		}
		cp = change_alloc(pp, history_data->change_number);
		change_bind_existing(cp);
		cstate_data = change_cstate_get(cp);
		if (!n)
		{
			string_ty	*line1;

			col_open((char *)0);
			line1 = str_format("Project \"%S\"", project_name);
			col_title(line1->str_text, "History");
			str_free(line1);

			delta_col = col_create(0, 7);
			date_col = col_create(8, 23);
			change_col = col_create(24, 31);
			description_col = col_create(32, 0);
			col_heading(delta_col, "Delta\n-------");
			col_heading(date_col, "Date and Time\n---------------");
			col_heading(change_col, "Change\n-------");
			col_heading(description_col, "Description\n-------------");
		}
		col_printf(delta_col, "%4ld", history_data->delta_number);
		t = cstate_data->history->list[cstate_data->history->length - 1]->when;
		col_puts(date_col, ctime(&t));
		col_printf(change_col, "%4ld", history_data->change_number);
		assert(cstate_data->brief_description);
		col_puts
		(
			description_col,
			cstate_data->brief_description->str_text
		);
		col_eoln();
		++n;
		change_free(cp);
	}
	if (n)
		col_close();
	else
		project_verbose(pp, "no integrations");
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_version(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	pstate		pstate_data;
	project_ty	*pp;
	cstate		cstate_data;
	change_ty	*cp;
	user_ty		*up;

	/*
	 * locate project data
	 */
	trace(("list_version()\n{\n"/*}*/));
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

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

	pstate_data = project_pstate_get(pp);
	cstate_data = change_cstate_get(cp);
	if (option_get_terse())
	{
		if (cstate_data->state == cstate_state_being_developed)
		{
			printf
			(
				"%ld.%ld.C%3.3ld\n",
				pstate_data->version_major,
				pstate_data->version_minor,
				change_number
			);
		}
		else
		{
			printf
			(
				"%ld.%ld.D%3.3ld\n",
				pstate_data->version_major,
				pstate_data->version_minor,
				cstate_data->delta_number
			);
		}
	}
	else
	{
		printf("version_major = %ld;\n", pstate_data->version_major);
		printf("version_minor = %ld;\n", pstate_data->version_minor);
		if (cstate_data->state == cstate_state_being_developed)
			printf("change_number = %ld;\n", change_number);
		else
		{
			printf
			(
				"delta_number = %ld;\n",
				cstate_data->delta_number
			);
		}
		if (pstate_data->version_previous)
		{
			printf
			(
				"version_previous = \"%s\";\n",
				pstate_data->version_previous->str_text
			);
		}
	}
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
list_change_details(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	int		j;
	int		head_col;
	int		body_col;
	project_ty	*pp;
	change_ty	*cp;
	cstate		cstate_data;
	user_ty		*up;
	string_ty	*line1;

	/*
	 * locate project data
	 */
	trace(("list_change_details()\n{\n"/*}*/));
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

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

	/*
	 * identification
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\", Change %ld", project_name, change_number);
	col_title(line1->str_text, "Change Details");
	str_free(line1);

	head_col = col_create(0, 0);
	body_col = col_create(8, 0);
	col_puts(head_col, "NAME");
	col_eoln();
	col_printf(body_col, "Project \"%s\"", project_name->str_text);
	if (cstate_data->delta_number)
		col_printf(body_col, ", Delta %ld", cstate_data->delta_number);
	if (cstate_data->state < cstate_state_completed || option_get_verbose())
		col_printf(body_col, ", Change %ld", change_number);
	col_puts(body_col, ".");
	col_eoln();

	/*
	 * synopsis
	 */
	col_need(5);
	col_puts(head_col, "SUMMARY");
	col_eoln();
	col_puts(body_col, cstate_data->brief_description->str_text);
	col_eoln();

	/*
	 * description
	 */
	col_need(5);
	col_puts(head_col, "DESCRIPTION");
	col_eoln();
	col_puts(body_col, cstate_data->description->str_text);
	if (cstate_data->test_exempt || cstate_data->test_baseline_exempt)
	{
		col_bol(body_col);
		col_puts(body_col, "\n");
		if (cstate_data->test_exempt)
		{
			col_puts
			(
				body_col,
				"This change is exempt from testing.  "
			);
		}
		if (cstate_data->test_baseline_exempt)
		{
			col_puts
			(
				body_col,
		      "This change is exempt from testing against the baseline."
			);
		}
	}
	col_eoln();

	/*
	 * cause
	 */
	col_need(5);
	col_puts(head_col, "CAUSE");
	col_eoln();
	col_printf
	(
		body_col,
		"This change was caused by %s.",
		change_cause_ename(cstate_data->cause)
	);
	col_eoln();

	/*
	 * state
	 */
	if
	(
		option_get_verbose()
	&&
		cstate_data->state != cstate_state_completed
	)
	{
		col_need(5);
		col_puts(head_col, "STATE");
		col_eoln();
		col_printf
		(
			body_col,
			"This change is in '%s' state.",
			cstate_state_ename(cstate_data->state)
		);
		col_eoln();
	}

	/*
	 * files
	 */
	col_need(5);
	col_puts(head_col, "FILES");
	col_eoln();
	if (cstate_data->src->length)
	{
		int	usage_col;
		int	action_col;
		int	edit_col;
		int	file_name_col;

		usage_col = col_create(8, 15);
		action_col = col_create(16, 23);
		edit_col = col_create(24, 31);
		file_name_col = col_create(32, 0);
		col_heading(usage_col, "Type\n-------");
		col_heading(action_col, "Action\n-------");
		col_heading(edit_col, "Edit\n-------");
		col_heading(file_name_col, "File Name\n-----------");
		for (j = 0; j < cstate_data->src->length; ++j)
		{
			cstate_src	src_data;
	
			src_data = cstate_data->src->list[j];
			assert(src_data->file_name);
			col_puts(usage_col, file_usage_ename(src_data->usage));
			col_puts
			(
				action_col,
				file_action_ename(src_data->action)
			);
			if (src_data->edit_number)
			{
				pstate_src	psrc_data;
	
				col_puts
				(
					edit_col,
					src_data->edit_number->str_text
				);
				if (cstate_data->state == cstate_state_being_developed)
					psrc_data =
						project_src_find
						(
							pp,
						     src_data->file_name
						);
				else
					psrc_data = 0;
				if
				(
					psrc_data
				&&
					psrc_data->edit_number
				&&
					!str_equal
					(
						src_data->edit_number,
						psrc_data->edit_number
					)
				)
				{
					col_printf
					(
						edit_col,
						" (%s)",
						psrc_data->edit_number->str_text
					);
				}
			}
			col_puts(file_name_col, src_data->file_name->str_text);
			col_eoln();
		}
		col_heading(usage_col, (char *)0);
		col_heading(action_col, (char *)0);
		col_heading(edit_col, (char *)0);
		col_heading(file_name_col, (char *)0);
	}
	else
	{
		col_printf(body_col, "This change has no files.");
		col_eoln();
	}

	/*
	 * history
	 */
	col_need(5);
	col_puts(head_col, "HISTORY");
	col_eoln();
	if (option_get_verbose())
	{
		int	what_col;
		int	when_col;
		int	who_col;
		int	why_col;

		what_col = col_create(8, 23);
		when_col = col_create(24, 39);
		who_col = col_create(40, 47);
		why_col = col_create(48, 0);
		col_heading(what_col, "What\n------");
		col_heading(when_col, "When\n------");
		col_heading(who_col, "Who\n-----");
		col_heading(why_col, "Comment\n---------");
		for (j = 0; j < cstate_data->history->length; ++j)
		{
			cstate_history	history_data;
			time_t		t;

			history_data = cstate_data->history->list[j];
			col_puts
			(
				what_col,
				cstate_history_what_ename(history_data->what)
			);
			t = history_data->when;
			col_puts(when_col, ctime(&t));
			col_puts(who_col, history_data->who->str_text);
			if (history_data->why)
				col_puts(why_col, history_data->why->str_text);
			if (history_data->what != cstate_history_what_integrate_pass)
			{
				time_t	finish;

				if (j + 1 < cstate_data->history->length)
					finish = cstate_data->history->list[j + 1]->when;
				else
					time(&finish);
				if (finish - t >= ELAPSED_TIME_THRESHOLD)
				{
					col_bol(why_col);
					col_printf
					(
						why_col,
						"Elapsed time: %5.3f days.\n",
						working_days(t, finish)
					);
				}
			}
			col_eoln();
		}
		col_heading(what_col, (char *)0);
		col_heading(when_col, (char *)0);
		col_heading(who_col, (char *)0);
		col_heading(why_col, (char *)0);
	}
	else
	{
		if (cstate_data->state >= cstate_state_being_developed)
		{
			col_printf
			(
				body_col,
				"Developed by %s.",
				change_developer_name(cp)->str_text
			);
		}
		if (cstate_data->state >= cstate_state_awaiting_integration)
		{
			col_printf
			(
				body_col,
				"  Reviewed by %s.",
				change_reviewer_name(cp)->str_text
			);
		}
		if (cstate_data->state >= cstate_state_being_integrated)
		{
			col_printf
			(
				body_col,
				"  Integrated by %s.",
				change_integrator_name(cp)->str_text
			);
		}
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
list_user_changes(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	user_ty		*up;
	int		project_col;
	int		change_col;
	int		state_col;
	int		description_col;
	string_ty	*s;
	wlist		name;
	long		j;

	trace(("list_user_changes()\n{\n"/*}*/));
	if (project_name)
		fatal("inappropriate -Project option");
	if (change_number)
		fatal("inappropriate -Change option");


	/*
	 * get the list of projects
	 */
	gonzo_project_list(&name);
	if (!name.wl_nwords)
	{
		verbose("there are no projects");
		goto done;
	}

	/*
	 * sort the list so the listing is pretty
	 */
	qsort
	(
		name.wl_word,
		name.wl_nwords,
		sizeof(name.wl_word[0]),
		name_cmp
	);

	/*
	 * open listing
	 */
	col_open((char *)0);
	up = user_executing((project_ty *)0);
	s = str_format("Owned by %S <%S>", up->full_name, user_name(up));
	user_free(up);
	col_title("List of Changes", s->str_text);
	str_free(s);

	/*
	 * create the columns
	 */
	project_col = col_create(0, 10);
	change_col = col_create(11, 17);
	state_col = col_create(18, 28);
	description_col = col_create(29, 0);
	col_heading(project_col, "Project\n----------");
	col_heading(change_col, "Change\n------");
	col_heading(state_col, "State\n----------");
	col_heading(description_col, "Description\n-------------");

	/*
	 * for each project, see if the current user
	 * is working on any of them.
	 */
	for (j = 0; j < name.wl_nwords; ++j)
	{
		project_ty	*pp;
		int		err;
		long		n;

		/*
		 * locate the project,
		 * and make sure we are allowed to look at it
		 */
		pp = project_alloc(name.wl_word[j]);
		project_bind_existing(pp);
		os_become_orig();
		err = os_readable(project_pstate_path_get(pp));
		os_become_undo();
		if (err)
		{
			project_free(pp);
			continue;
		}

		/*
		 * bind a user to that project
		 */
		up = user_executing(pp);

		/*
		 * for each change within this project the user
		 * is working on emit a line of information
		 */
		for (n = 0; ; ++n)
		{
			change_ty	*cp;
			cstate		cstate_data;

			if (!user_own_nth(up, n, &change_number))
				break;
			
			/*
			 * locate change data
			 */
			cp = change_alloc(pp, change_number);
			change_bind_existing(cp);
	
			/*
			 * emit the info
			 */
			col_puts(project_col, project_name_get(pp)->str_text);
			col_printf(change_col, "%4ld", change_number);
			cstate_data = change_cstate_get(cp);
			col_puts
			(
				state_col,
				cstate_state_ename(cstate_data->state)
			);
			if (cstate_data->brief_description)
			{
				col_puts
				(
					description_col,
					cstate_data->brief_description->str_text
				);
			}
			col_eoln();
	
			/*
			 * release change and project
			 */
			change_free(cp);
		}

		/*
		 * free user and project
		 */
		user_free(up);
		project_free(pp);
	}

	/*
	 * clean up and go home
	 */
	col_close();
	done:
	trace((/*{*/"}\n"));
}
