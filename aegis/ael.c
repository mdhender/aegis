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
 * MANIFEST: list interesting things about changes and projects
 *
 * If you add another list to this file,
 * don't forget to update man1/ael.1
 */

#include <stdio.h>
#include <ac/string.h>
#include <ac/stdlib.h>
#include <ac/time.h>

#include <ael.h>
#include <aer/func/now.h>
#include <arglex2.h>
#include <col.h>
#include <change.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <word.h>

#define ELAPSED_TIME_THRESHOLD (10L * 60L) /* ten minutes */


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
		"Default_Change",
		"List the default change for the current user",
		list_default_change,
	},
	{
		"Default_Project",
		"List the default project for the current user",
		list_default_project,
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
		"Locks",
		"List the active locks",
		list_locks,
	},
	{
		"Outstanding_Changes",
		"List the outstanding changes",
		list_outstanding_changes,
	},
	{
		"All_Outstanding_Changes",
		"List the outstanding changes for all projects",
		list_outstanding_changes_all,
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

	progname = option_progname_get();
	fprintf
	(
		stderr,
		"usage: %s -List [ <option>... ] <listname>\n",
		progname
	);
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
#include <../man1/ael.h>
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
		fatal
		(
			"list name \"%s\" ambiguous (%s)",
			listname,
			s1->str_text
		);
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
	int		name_col = 0;
	int		dir_col = 0;
	int		desc_col = 0;
	int		j;

	trace(("list_projects()\n{\n"/*}*/));
	if (project_name)
		fatal("inappropriate -Project option");
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * list the projects
	 */
	gonzo_project_list(&name);
	qsort
	(
		name.wl_word,
		name.wl_nwords,
		sizeof(name.wl_word[0]),
		name_cmp
	);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	col_title("List of Projects", (char *)0);
	name_col = col_create(0, 15);
	col_heading(name_col, "Project\n---------");
	if (!option_terse_get())
	{
		dir_col = col_create(16, 39);
		desc_col = col_create(40, 0);
		col_heading(dir_col, "Directory\n-----------");
		col_heading(desc_col, "Description\n-------------");
	}

	/*
	 * list each project
	 */
	for (j = 0; j < name.wl_nwords; ++j)
	{
		project_ty	*pp;
		int		err;

		pp = project_alloc(name.wl_word[j]);
		project_bind_existing(pp);

		os_become_orig();
		err = os_readable(project_pstate_path_get(pp));
		os_become_undo();

		col_puts(name_col, project_name_get(pp)->str_text);
		if (!option_terse_get())
		{
			col_puts(dir_col, project_home_path_get(pp)->str_text);
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
		}
		project_free(pp);
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	if (!name.wl_nwords)
		verbose("there are no projects");
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
	int		usage_col = 0;
	int		edit_col = 0;
	int		file_name_col = 0;
	int		j;
	string_ty	*line1;

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

	/*
	 * create the columns
	 */
	col_open((char *)0);
	if (change_number)
		line1 =
			str_format
			(
				"Project \"%S\"  Change %ld",
				project_name,
				change_number
			);
	else
		line1 = str_format("Project \"%S\"", project_name);
	col_title(line1->str_text, "List of Project's Files");
	str_free(line1);

	if (!option_terse_get())
	{
		usage_col = col_create(0, 7);
		edit_col = col_create(8, 15);
		col_heading(usage_col, "Type\n-------");
		col_heading(edit_col, "Edit\n-------");
	}
	file_name_col = col_create(16, 0);
	col_heading(file_name_col, "File Name\n-----------");

	/*
	 * list the project's files
	 */
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
			!option_verbose_get()
		)
			continue;
		if (cp && change_src_find(cp, src_data->file_name))
			continue;
		if (option_terse_get())
		{
			if (src_data->deleted_by)
				continue;
			if (src_data->about_to_be_created_by)
				continue;
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

	/*
	 * clean up and go home
	 */
	col_close();
	if (!pstate_data->src->length)
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
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;

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

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name);
	col_title(line1->str_text, "List of Administrators");
	str_free(line1);

	login_col = col_create(0, 15);
	col_heading(login_col, "User\n------");
	if (!option_terse_get())
	{
		name_col = col_create(16, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list project's administrators
	 */
	for (j = 0; j < pstate_data->administrator->length; ++j)
	{
		string_ty	*logname;

		logname = pstate_data->administrator->list[j];
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	if (!pstate_data->administrator->length)
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
	int		usage_col = 0;
	int		action_col = 0;
	int		edit_col = 0;
	int		file_name_col = 0;
	int		j;
	string_ty	*line1;

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

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 =
		str_format
		(
			"Project \"%S\"  Change %ld",
			project_name,
			change_number
		);
	col_title(line1->str_text, "List of Change's Files");
	str_free(line1);

	if (!option_terse_get())
	{
		usage_col = col_create(0, 7);
		action_col = col_create(8, 15);
		edit_col = col_create(16, 23);
		col_heading(usage_col, "Type\n-------");
		col_heading(action_col, "Action\n-------");
		col_heading(edit_col, "Edit\n-------");
	}
	file_name_col = col_create(24, 0);
	col_heading(file_name_col, "File Name\n-----------");

	/*
	 * list the change's files
	 */
	for (j = 0; j < cstate_data->src->length; ++j)
	{
		cstate_src	src_data;

		src_data = cstate_data->src->list[j];
		assert(src_data->file_name);
		if (option_terse_get())
		{
			if (src_data->action == file_action_remove)
				continue;
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
		}
		col_puts
		(
			file_name_col,
			src_data->file_name->str_text
		);
		if (src_data->move)
		{
			col_bol(file_name_col);
			col_puts(file_name_col, "Moved ");
			if (src_data->action == file_action_create)
				col_puts(file_name_col, "from ");
			else
				col_puts(file_name_col, "to ");
			col_puts(file_name_col, src_data->move->str_text);
		}
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	if (!cstate_data->src->length)
		change_verbose(cp, "no files");
	project_free(pp);
	change_free(cp);
	user_free(up);
	trace((/*{*/"}\n"));
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
	int		what_col;
	int		when_col;
	int		who_col;
	int		why_col;
	int		j;
	string_ty	*line1;

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

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 =
		str_format
		(
			"Project \"%S\"  Change %ld",
			project_name,
			change_number
		);
	col_title(line1->str_text, "History");
	str_free(line1);

	what_col = col_create(0, 15);
	when_col = col_create(16, 31);
	who_col = col_create(32, 40);
	why_col = col_create(41, 0);
	col_heading(what_col, "What\n------");
	col_heading(when_col, "When\n------");
	col_heading(who_col, "Who\n-----");
	col_heading(why_col, "Comment\n---------");

	/*
	 * list the history
	 */
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
				finish =
					cstate_data->history->list[j + 1]->when;
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

	/*
	 * clean up and go home
	 */
	col_close();
	if (!cstate_data->history->length)
		change_verbose(cp, "no history");
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


static int single_bit _((int));

static int
single_bit(n)
	int		n;
{
	int		result;

	/* see if no bits are set */
	if (!n)
		return -1;
	/*
	 * see if more than 1 bit is set
	 *	(only works on 2s compliment machines)
	 */
	if ((n & -n) != n)
		return -1;
	/*
	 * will need to extend this for 64bit machines,
	 * if ever have >32 states
	 */
	result = 0;
	if (n & 0xFFFF0000)
		result += 16;
	if (n & 0xFF00FF00)
		result += 8;
	if (n & 0xF0F0F0F0)
		result += 4;
	if (n & 0xCCCCCCCC)
		result += 2;
	if (n & 0xAAAAAAAA)
		result++;
	return result;
}


void
list_changes_in_state_mask(project_name, state_mask)
	string_ty	*project_name;
	int		state_mask;
{
	pstate		pstate_data;
	int		number_col = 0;
	int		state_col = 0;
	int		description_col = 0;
	int		j;
	project_ty	*pp;
	string_ty	*line1;
	string_ty	*line2;

	/*
	 * locate project data
	 */
	trace(("list_changes_in_state_mask(state_mask = 0x%X)\n{\n"/*}*/,
		state_mask));
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	pstate_data = project_pstate_get(pp);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name);
	j = single_bit(state_mask);
	if (j >= 0)
		line2 = str_format("List of Changes %s", cstate_state_ename(j));
	else
	{
		j = single_bit(~state_mask);
		if (j >= 0)
		{
			line2 =
				str_format
				(
					"List of Changes not %s",
					cstate_state_ename(j)
				);
		}
		else
			line2 = str_from_c("List of Changes");
	}
	col_title(line1->str_text, line2->str_text);
	str_free(line1);
	str_free(line2);

	number_col = col_create(0, 7);
	col_heading(number_col, "Change\n-------");
	if (!option_terse_get())
	{
		state_col = col_create(8, 23);
		description_col = col_create(24, 0);
		col_heading(state_col, "State\n-------");
		col_heading(description_col, "Description\n-------------");
	}

	/*
	 * list the project's changes
	 */
	for (j = 0; j < pstate_data->change->length; ++j)
	{
		cstate		cstate_data;
		long		change_number;
		change_ty	*cp;

		change_number = pstate_data->change->list[j];
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
		cstate_data = change_cstate_get(cp);
		if (state_mask & (1 << cstate_data->state))
		{
			col_printf(number_col, "%4ld", change_number);
			if (!option_terse_get())
			{
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
			}
			col_eoln();
		}
		change_free(cp);
	}

	/*
	 * clean up and go home
	 */
	col_close();
	if (!pstate_data->change->length)
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
	int		name_col = 0;
	int		desc_col = 0;
	table_ty	*tp;

	trace(("list_list_list()\n{\n"/*}*/));
	if (project_name)
		fatal("inappropriate -Project option");
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * create the columns
	 */
	col_open((char *)0);
	col_title("List of Lists", (char *)0);
	name_col = col_create(0, 15);
	col_heading(name_col, "Name\n------");
	if (!option_terse_get())
	{
		desc_col = col_create(16, 0);
		col_heading(desc_col, "Description\n-------------");
	}

	/*
	 * list the lists
	 */
	for (tp = table; tp < ENDOF(table); ++tp)
	{
		col_puts(name_col, tp->name);
		if (!option_terse_get())
			col_puts(desc_col, tp->description);
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
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
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;

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

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name);
	col_title(line1->str_text, "List of Developers");
	str_free(line1);

	login_col = col_create(0, 15);
	col_heading(login_col, "User\n------");
	if (!option_terse_get())
	{
		name_col = col_create(16, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list the project's developers
	 */
	for (j = 0; j < pstate_data->developer->length; ++j)
	{
		string_ty	*logname;

		logname = pstate_data->developer->list[j];
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	if (!pstate_data->developer->length)
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
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;

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

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name);
	col_title(line1->str_text, "List of Integrators");
	str_free(line1);

	login_col = col_create(0, 15);
	col_heading(login_col, "User\n------");
	if (!option_terse_get())
	{
		name_col = col_create(16, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list the project's integrators
	 */
	for (j = 0; j < pstate_data->integrator->length; ++j)
	{
		string_ty	*logname;

		logname = pstate_data->integrator->list[j];
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	if (!pstate_data->integrator->length)
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
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;

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

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name);
	col_title(line1->str_text, "List of Reviewers");
	str_free(line1);

	login_col = col_create(0, 15);
	col_heading(login_col, "User\n------");
	if (!option_terse_get())
	{
		name_col = col_create(16, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list the project's reviewers
	 */
	for (j = 0; j < pstate_data->reviewer->length; ++j)
	{
		string_ty	*logname;

		logname = pstate_data->reviewer->list[j];
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	if (!pstate_data->reviewer->length)
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
	int		name_col = 0;
	int		delta_col = 0;
	int		date_col = 0;
	int		change_col = 0;
	int		description_col = 0;
	size_t		j, k;
	project_ty	*pp;
	string_ty	*line1;

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

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name);
	col_title(line1->str_text, "History");
	str_free(line1);

	name_col = col_create(0, 0);
	delta_col = col_create(0, 7);
	col_heading(delta_col, "Delta\n-------");
	if (!option_terse_get())
	{
		date_col = col_create(8, 23);
		change_col = col_create(24, 31);
		description_col = col_create(32, 0);
		col_heading(date_col, "Date and Time\n---------------");
		col_heading(change_col, "Change\n-------");
		col_heading(description_col, "Description\n-------------");
	}

	/*
	 * list the project's sucessful i9ntegrations
	 */
	for (j = 0; j < pstate_data->history->length; ++j)
	{
		pstate_history	history_data;

		history_data = pstate_data->history->list[j];
		if
		(
			!option_terse_get()
		&&
			history_data->name
		&&
			history_data->name->length
		)
		{
			col_need(4);
			col_printf
			(
				name_col,
				"Name%s: ",
				(history_data->name->length==1?"":"s")
			);
			for (k = 0; k < history_data->name->length; ++k)
			{
				if (k)
					col_printf(name_col, ", ");
				col_printf
				(
					name_col,
					"\"%s\"",
					history_data->name->list[k]
						->str_text
				);
			}
		}
		col_printf(delta_col, "%4ld", history_data->delta_number);
		if (!option_terse_get())
		{
			cstate		cstate_data;
			time_t		t;
			change_ty	*cp;

			cp = change_alloc(pp, history_data->change_number);
			change_bind_existing(cp);
			cstate_data = change_cstate_get(cp);
			t =
				cstate_data->history->list
				[
					cstate_data->history->length - 1
				]->when;
			col_puts(date_col, ctime(&t));
			col_printf
			(
				change_col,
				"%4ld",
				history_data->change_number
			);
			assert(cstate_data->brief_description);
			col_puts
			(
				description_col,
				cstate_data->brief_description->str_text
			);
			change_free(cp);
		}
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	if (!pstate_data->history->length)
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
	if (option_terse_get())
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
		if (pstate_data->copyright_years)
		{
			pstate_copyright_years_list p;
			size_t		j;

			p = pstate_data->copyright_years;
			printf("copyright_years = [");
			for (j = 0; j < p->length; ++j)
			{
				if (j)
					printf(", ");
				printf("%ld", p->list[j]);
			}
			printf("];\n");
		}
	}
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


static void showtime _((int, time_t, int));

static void
showtime(colnum, when, exempt)
	int	colnum;
	time_t	when;
	int	exempt;
{
	if (when)
	{
		struct tm	*tm;
		char		buffer[100];

		tm = localtime(&when);
		strftime(buffer, sizeof(buffer), "%H:%M:%S %d-%b-%y", tm);
		col_puts(colnum, buffer);
	}
	else if (exempt)
		col_puts(colnum, "exempt");
	else
		col_puts(colnum, "required");
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
	line1 =
		str_format
		(
			"Project \"%S\", Change %ld",
			project_name,
			change_number
		);
	col_title(line1->str_text, "Change Details");
	str_free(line1);

	head_col = col_create(0, 0);
	body_col = col_create(8, 0);
	col_puts(head_col, "NAME");
	col_eoln();
	col_printf(body_col, "Project \"%s\"", project_name->str_text);
	if (cstate_data->delta_number)
		col_printf(body_col, ", Delta %ld", cstate_data->delta_number);
	if (cstate_data->state < cstate_state_completed || option_verbose_get())
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
		if (!cstate_data->regression_test_exempt)
		{
			col_puts
			(
				body_col,
			       "This change must pass a full regression test.  "
			);
		}
		if (cstate_data->test_exempt)
		{
			col_puts
			(
				body_col,
       "This change is exempt from testing against the development directory.  "
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
	 * architecture
	 */
	col_need(7);
	col_printf
	(
		head_col,
		"ARCHITECTURE%s",
		(cstate_data->architecture->length == 1 ? "" : "S")
	);
	col_eoln();
	col_puts(body_col, "This change must build and test in");
	if (cstate_data->architecture->length > 1)
		col_puts(body_col, " each of");
	col_puts(body_col, " the");
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		string_ty	*s;

		s = cstate_data->architecture->list[j];
		if (j)
		{
			if (j == cstate_data->architecture->length - 1)
				col_puts(body_col, " and");
			else
				col_puts(body_col, ",");
		}
		col_printf(body_col, " \"%s\"",  s->str_text);
	}
	col_printf
	(
		body_col,
		" architecture%s.",
		(cstate_data->architecture->length == 1 ? "" : "s")
	);
	col_eoln();

	if
	(
		cstate_data->state == cstate_state_being_developed
	||
		cstate_data->state == cstate_state_being_integrated
	)
	{
		int	arch_col;
		int	host_col;
		int	build_col;
		int	test_col;
		int	test_bl_col;
		int	test_reg_col;

		col_need(5);
		arch_col = col_create(8, 16);
		host_col = col_create(17, 25);
		build_col = col_create(26, 35);
		test_col = col_create(36, 45);
		test_bl_col = col_create(46, 55);
		test_reg_col = col_create(56, 65);
		col_heading(arch_col, "arch.\n--------");
		col_heading(host_col, "host\n--------");
		col_heading(build_col, "aeb\n---------");
		col_heading(test_col, "aet\n---------");
		col_heading(test_bl_col, "aet -bl\n---------");
		col_heading(test_reg_col, "aet -reg\n---------");

		for (j = 0; j < cstate_data->architecture->length; ++j)
		{
			cstate_architecture_times tp;
			string_ty	*s;

			s = cstate_data->architecture->list[j];
			tp = change_architecture_times_find(cp, s);
			
			col_puts(arch_col, tp->variant->str_text);
			if (tp->node)
				col_puts(host_col, tp->node->str_text);
			showtime(build_col, tp->build_time, 0);
			showtime
			(
				test_col,
				tp->test_time,
				cstate_data->test_exempt
			);
			showtime
			(
				test_bl_col,
				tp->test_baseline_time,
				cstate_data->test_baseline_exempt
			);
			showtime
			(
				test_reg_col,
				tp->regression_test_time,
				cstate_data->regression_test_exempt
			);
			col_eoln();
		}

		col_heading(arch_col, (char *)0);
		col_heading(host_col, (char *)0);
		col_heading(build_col, (char *)0);
		col_heading(test_col, (char *)0);
		col_heading(test_bl_col, (char *)0);
		col_heading(test_reg_col, (char *)0);

		if (cstate_data->architecture->length > 1)
		{
			col_puts(build_col, "---------\n");
			col_puts(test_col, "---------\n");
			col_puts(test_bl_col, "---------\n");
			col_puts(test_reg_col, "---------\n");

			showtime(build_col, cstate_data->build_time, 0);
			showtime
			(
				test_col,
				cstate_data->test_time,
				cstate_data->test_exempt
			);
			showtime
			(
				test_bl_col,
				cstate_data->test_baseline_time,
				cstate_data->test_baseline_exempt
			);
			showtime
			(
				test_reg_col,
				cstate_data->regression_test_time,
				cstate_data->regression_test_exempt
			);
			col_eoln();
		}
	}

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
		option_verbose_get()
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
			"This change is in the '%s' state.",
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
			col_puts(file_name_col, src_data->file_name->str_text);
			if (src_data->move)
			{
				col_bol(file_name_col);
				col_puts(file_name_col, "Moved ");
				if (src_data->action == file_action_create)
					col_puts(file_name_col, "from ");
				else
					col_puts(file_name_col, "to ");
				col_puts
				(
					file_name_col,
					src_data->move->str_text
				);
			}
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
	if (option_verbose_get())
	{
		int	what_col;
		int	when_col;
		int	who_col;
		int	why_col;

		what_col = col_create(8, 23);
		when_col = col_create(24, 39);
		who_col = col_create(40, 48);
		why_col = col_create(49, 0);
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
			if
			(
				history_data->what
			!=
				cstate_history_what_integrate_pass
			)
			{
				time_t	finish;

				if (j + 1 < cstate_data->history->length)
					finish =
						cstate_data->history->
							list[j + 1]->
								when;
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


static int list_locks_name_col;
static int list_locks_type_col;
static int list_locks_project_col;
static int list_locks_change_col;
static int list_locks_address_col;
static int list_locks_process_col;
static wlist list_locks_pnames;
static long list_locks_count;


static void list_locks_callback _((lock_walk_found *));

static void
list_locks_callback(found)
	lock_walk_found *found;
{
	char		*name_str;
	char		*type_str;
	char		*project_str;
	long		change_number;
	long		j;

	list_locks_count++;
	switch (found->name)
	{
	case lock_walk_name_master:
		name_str = "master";
		break;

	case lock_walk_name_gstate:
		name_str = "gonzo";
		break;

	case lock_walk_name_pstate:
		name_str = "project";
		break;

	case lock_walk_name_cstate:
		name_str = "change";
		break;

	case lock_walk_name_ustate:
		name_str = "user";
		break;

	case lock_walk_name_build:
		name_str = "build";
		break;

	default:
		name_str = "unknown";
		break;
	}

	switch (found->type)
	{
	case lock_walk_type_shared:
		type_str = "shared";
		break;

	case lock_walk_type_exclusive:
		type_str = "exclusive";
		break;

	default:
		type_str = "unknown";
		break;
	}

	project_str = 0;
	change_number = 0;
	switch (found->name)
	{
	case lock_walk_name_pstate:
	case lock_walk_name_build:
		for (j = 0; j < list_locks_pnames.wl_nwords; ++j)
		{
			string_ty *s;

			s = list_locks_pnames.wl_word[j];
			if ((s->str_hash & 0xFFFF) == found->subset)
			{
				project_str = s->str_text;
				break;
			}
		}
		if (!project_str)
			project_str = "unknown";
		break;

	case lock_walk_name_cstate:
		for (j = 0; j < list_locks_pnames.wl_nwords; ++j)
		{
			string_ty *s;
			long	cn;

			/*
			 * This is very messy, because the change
			 * number is added to the project name hash.
			 *
			 * Work out the change number,
			 * and the assume it can be more than,
			 * say, 5000.  We could do better by looking
			 * in each project, but what the hell.
			 *
			 * Use (mod 2**16) arithmetic, that's how its done.
			 * Should bring this out in lock.h if ever change.
			 *
			 * Loop and find the smallest change number.
			 * Use that as a probable "best" fit.
			 */
			s = list_locks_pnames.wl_word[j];
			cn = (found->subset - s->str_hash) & 0xFFFF;
			if
			(
				cn >= 1
			&&
				cn <= 5000
			&&
				(!change_number || cn < change_number)
			)
			{
				project_str = s->str_text;
				change_number = cn;
			}
		}
		if (!project_str)
			project_str = "unknown";
		break;

	default:
		break;
	}

	/*
	 * print it all out
	 */
	col_printf(list_locks_name_col, "%s", name_str);
	col_printf(list_locks_type_col, "%s", type_str);
	if (project_str)
		col_printf(list_locks_project_col, "%s", project_str);
	if (change_number)
		col_printf(list_locks_change_col, "%4ld", change_number);
	col_printf(list_locks_address_col, "%8.8lX", found->address);
	col_printf(list_locks_process_col, "%5d", found->pid);
	if (!found->pid_is_local)
		col_printf(list_locks_process_col, " remote");
	col_eoln();
}


void
list_locks(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	/*
	 * check for silly arguments
	 */
	trace(("list_locks()\n{\n"/*}*/));
	if (project_name)
		fatal("inappropriate -Project option");
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * get the list of projects
	 */
	gonzo_project_list(&list_locks_pnames);

	/*
	 * open the columns
	 */
	col_open((char *)0);
	list_locks_name_col = col_create(0, 8);
	list_locks_type_col = col_create(9, 19);
	list_locks_project_col = col_create(20, 32);
	list_locks_change_col = col_create(33, 40);
	list_locks_address_col = col_create(41, 50);
	list_locks_process_col = col_create(51, 0);
	list_locks_count = 0;

	/*
	 * set the column headings
	 */
	col_title("List of Locks", gonzo_lockpath_get()->str_text);
	col_heading(list_locks_name_col, "Type\n------");
	col_heading(list_locks_type_col, "Mode\n------");
	col_heading(list_locks_project_col, "Project\n---------");
	col_heading(list_locks_change_col, "Change\n------");
	col_heading(list_locks_address_col, "Address\n--------");
	col_heading(list_locks_process_col, "Process\n--------");

	/*
	 * list the locks found
	 */
	lock_walk(list_locks_callback);
	wl_free(&list_locks_pnames);
	if (list_locks_count == 0)
	{
		int info = col_create(4, 0);
		col_puts(info, "No locks found.");
		col_eoln();
	}
	col_close();
	trace((/*{*/"}\n"));
}


void
list_default_project(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	/*
	 * check for silly arguments
	 */
	trace(("list_default_project()\n{\n"/*}*/));
	if (project_name)
		fatal("inappropriate -Project option");
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * Find default project name;
	 * will generate fatal error if no default.
	 */
	project_name = user_default_project();

	/*
	 * print it out
	 */
	printf("%s\n", project_name->str_text);
	trace((/*{*/"}\n"));
}


void
list_default_change(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	user_ty		*up;

	/*
	 * check for silly arguments
	 */
	trace(("list_default_change()\n{\n"/*}*/));
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * resolve the project name
	 */
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
	 * Find default change number;
	 * will generate fatal error if no default.
	 */
	change_number = user_default_change(up);

	/*
	 * print it out
	 */
	printf("%ld\n", change_number);

	/*
	 * clean up and go home
	 */
	project_free(pp);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
list_outstanding_changes(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	trace(("list_outstanding_changes()\n{\n"/*}*/));
	if (change_number)
		fatal("inappropriate -Change option");
	list_changes_in_state_mask
	(
		project_name,
		~(1 << cstate_state_completed)
	);
	trace((/*{*/"}\n"));
}


void
list_outstanding_changes_all(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	pstate		pstate_data;
	int		project_col = 0;
	int		number_col = 0;
	int		state_col = 0;
	int		description_col = 0;
	int		j, k;
	wlist		name;

	trace(("list_outstanding_changes_all()\n{\n"/*}*/));
	if (project_name)
		fatal("inappropriate -Project option");
	if (change_number)
		fatal("inappropriate -Change option");

	/*
	 * list the projects
	 */
	gonzo_project_list(&name);
	qsort
	(
		name.wl_word,
		name.wl_nwords,
		sizeof(name.wl_word[0]),
		name_cmp
	);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	col_title("List of Outstanding Changes", "for all projects");
	project_col = col_create(0, 15);
	col_heading(project_col, "Project\n---------");
	number_col = col_create(16, 23);
	col_heading(number_col, "Change\n------");
	if (!option_terse_get())
	{
		state_col = col_create(24, 35);
		col_heading(state_col, "State\n-------");
		description_col = col_create(36, 0);
		col_heading(description_col, "Description\n-------------");
	}

	/*
	 * scan each project
	 */
	for (j = 0; j < name.wl_nwords; ++j)
	{
		project_ty	*pp;
		int		err;

		pp = project_alloc(name.wl_word[j]);
		project_bind_existing(pp);

		/*
		 * make sure we have permission
		 */
		os_become_orig();
		err = os_readable(project_pstate_path_get(pp));
		os_become_undo();
		if (err)
		{
			project_free(pp);
			continue;
		}

		/*
		 * list the project's changes
		 */
		pstate_data = project_pstate_get(pp);
		for (k = 0; k < pstate_data->change->length; ++k)
		{
			cstate		cstate_data;
			change_ty	*cp;

			/*
			 * make sure the change is not completed
			 */
			change_number = pstate_data->change->list[k];
			cp = change_alloc(pp, change_number);
			change_bind_existing(cp);
			cstate_data = change_cstate_get(cp);
			if (cstate_data->state == cstate_state_completed)
			{
				change_free(cp);
				continue;
			}

			/*
			 * print the details
			 */
			col_puts(project_col, project_name_get(pp)->str_text);
			col_printf(number_col, "%4ld", change_number);
			if (!option_terse_get())
			{
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
			}
			col_eoln();
			change_free(cp);
		}
		project_free(pp);
	}

	/*
	 * clean up and go home
	 */
	col_close();
	trace((/*{*/"}\n"));
}