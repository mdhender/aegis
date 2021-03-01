/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
 * MANIFEST: list interesting things about changes and projects
 *
 * If you add another list to this file,
 * don't forget to update man1/ael.1
 */

#include <stdio.h>
#include <ac/string.h>
#include <ac/stdlib.h>
#include <ac/time.h>
#include <ac/pwd.h>

#include <ael.h>
#include <aer/func/now.h>
#include <arglex2.h>
#include <col.h>
#include <change_bran.h>
#include <change_file.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <itab.h>
#include <lock.h>
#include <option.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project_file.h>
#include <project_hist.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <str_list.h>

#define ELAPSED_TIME_THRESHOLD (10L * 60L) /* ten minutes */

/*
 * widths of the various columns
 *
 * (Many are 8n-1; this does nice things with tabs.)
 */
#define USAGE_WIDTH	7	/* strlen("manual_") = 7 */
#define ACTION_WIDTH	8	/* strlen("insulate") = 8 */
#define EDIT_WIDTH	12	/* strlen("1.23 (4.56)") = 11,
				   strlen("1.23 -> 4.56") = 12 */
#define LOGIN_WIDTH	8	/* login names will be <= 8 */
#define WHAT_WIDTH	15	/* widest is 20, worst is 10 */
#define WHEN_WIDTH	15	/* ctime fits in 2 lines */
#define WHO_WIDTH	LOGIN_WIDTH
#define	CHANGE_WIDTH	7
#define	STATE_WIDTH	15	/* widest is 20, worst is 11 */
#define ARCH_WIDTH	8
#define HOST_WIDTH	8
#define TIME_WIDTH	9	/* strlen("99-Oct-99") = 9 */
#define INDENT_WIDTH	8	/* used for change_details */
#define PROJECT_WIDTH	15
#define DIRECTORY_WIDTH 23


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

	progname = progname_get();
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
	help("ael", list_usage);
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
	sub_context_ty	*scp;
	char		*listname;
	table_ty	*tp;
	int		j;
	table_ty	*hit[SIZEOF(table)];
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
			{
				fatal_intl(0, i18n("too many lists"));
			}
			listname = arglex_value.alv_string;
			break;

		case arglex_token_change:
			if (arglex() != arglex_token_number)
				option_needs_number(arglex_token_change, list_usage);
			/* fall through... */

		case arglex_token_number:
			if (change_number)
				duplicate_option_by_name(arglex_token_change, list_usage);
			change_number = arglex_value.alv_number;
			if (change_number == 0)
				change_number = MAGIC_ZERO;
			else if (change_number < 1)
			{
				scp = sub_context_new();
				sub_var_set(scp, "Number", "%ld", change_number);
				fatal_intl(scp, i18n("change $number out of range"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			break;

		case arglex_token_project:
			if (arglex() != arglex_token_string)
				option_needs_name(arglex_token_project, list_usage);
			if (project_name)
				duplicate_option_by_name(arglex_token_project, list_usage);
			project_name = str_from_c(arglex_value.alv_string);
			break;
		}
		arglex();
	}
	if (!listname)
		fatal_intl(0, i18n("no list"));

	nhit = 0;
	for (tp = table; tp < ENDOF(table); ++tp)
	{
		if (arglex_compare(tp->name, listname))
			hit[nhit++] = tp;
	}
	switch (nhit)
	{
	case 0:
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", listname);
		fatal_intl(scp, i18n("no $name list"));
		/* NOTREACHED */
		sub_context_delete(scp);

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
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", listname);
		sub_var_set(scp, "Name_List", "%S", s1);
		str_free(s1);
		sub_var_optional(scp, "Name_List");
		fatal_intl(scp, i18n("list $name ambiguous"));
		/* NOTREACHED */
		sub_context_delete(scp);
		break;
	}
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


void
list_projects(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	sub_context_ty	*scp;
	string_list_ty	name;
	int		name_col = 0;
	int		dir_col = 0;
	int		desc_col = 0;
	int		j;
	int		left;

	trace(("list_projects()\n{\n"/*}*/));
	if (project_name)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_project));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (change_number)
	{
		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * list the projects
	 */
	project_list_get(&name);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	col_title("List of Projects", (char *)0);

	left = 0;
	name_col = col_create(left, left + PROJECT_WIDTH);
	left += PROJECT_WIDTH + 1;
	col_heading(name_col, "Project\n---------");

	if (!option_terse_get())
	{
		dir_col = col_create(left, left + DIRECTORY_WIDTH);
		left += DIRECTORY_WIDTH + 1;
		col_heading(dir_col, "Directory\n-----------");

		desc_col = col_create(left, 0);
		col_heading(desc_col, "Description\n-------------");
	}

	/*
	 * list each project
	 */
	for (j = 0; j < name.nstrings; ++j)
	{
		project_ty	*pp;
		int		err;

		pp = project_alloc(name.string[j]);
		project_bind_existing(pp);

		err = project_is_readable(pp);

		col_puts(name_col, project_name_get(pp)->str_text);
		if (!option_terse_get())
		{
			if (err)
				col_puts(desc_col, strerror(err));
			else
			{
				string_ty	*top;

				/*
				 * The development directory of the
				 * project change is the one which
				 * contains the trunk or branch
				 * baseline.
				 */
				top = project_top_path_get(pp, 0);
				col_puts(dir_col, top->str_text);

				col_puts
				(
					desc_col,
					project_description_get(pp)->str_text
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
	trace((/*{*/"}\n"));
}


static void format_edit_number _((int, fstate_src));

static void
format_edit_number(edit_col, src_data)
	int		edit_col;
	fstate_src	src_data;
{
	if
	(
		src_data->edit_number_origin
	&&
		src_data->edit_number
	)
	{
		/*
		 * We have both the original version copied, and the
		 * current head revision.  Print them both, with a
		 * notation implying ``from the old one to the new one''
		 * if they differ.  Only print one if thay are the same.
		 */
		col_printf
		(
			edit_col,
			"%4s",
			src_data->edit_number_origin->str_text
		);
		if
		(
			str_equal
			(
				src_data->edit_number,
				src_data->edit_number_origin
			)
		)
			return;
		col_printf
		(
			edit_col,
			" -> %s",
			src_data->edit_number->str_text
		);
		return;
	}

	if (src_data->edit_number_origin)
	{
		/*
		 * The "original version" copied.
		 */
		col_printf
		(
			edit_col,
			"%4s",
			src_data->edit_number_origin->str_text
		);
	}
	if (src_data->edit_number)
	{
		/*
		 * For active branches, the current
		 * head revision.  For completed changes
		 * and branches, the revision at aeipass.
		 */
		col_printf
		(
			edit_col,
			"%4s",
			src_data->edit_number->str_text
		);
	}
}


void
list_project_files(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	change_ty	*cp;
	user_ty		*up;
	int		usage_col = 0;
	int		edit_col = 0;
	int		file_name_col = 0;
	int		j;
	string_ty	*line1;
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_project_files()\n{\n"/*}*/));
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
	 * locate change data
	 */
	if (change_number)
	{
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
	}
	else
		cp = 0;

	/*
	 * create the columns
	 */
	col_open((char *)0);
	if (change_number)
	{
		line1 =
			str_format
			(
				"Project \"%S\"  Change %ld",
				project_name_get(pp),
				magic_zero_decode(change_number)
			);
	}
	else
		line1 = str_format("Project \"%S\"", project_name_get(pp));
	col_title(line1->str_text, "List of Project's Files");
	str_free(line1);

	left = 0;
	if (!option_terse_get())
	{
		usage_col = col_create(left, left + USAGE_WIDTH);
		left += USAGE_WIDTH + 1;
		col_heading(usage_col, "Type\n-------");

		edit_col = col_create(left, left + EDIT_WIDTH);
		left += EDIT_WIDTH + 1;
		col_heading(edit_col, "Edit\n-------");
	}
	file_name_col = col_create(left, 0);
	col_heading(file_name_col, "File Name\n-----------");

	/*
	 * list the project's files
	 */
	for (j = 0; ; ++j)
	{
		fstate_src	src_data;

		src_data = project_file_nth(pp, j);
		if (!src_data)
			break;
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
		if (cp && change_file_find(cp, src_data->file_name))
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
			format_edit_number(edit_col, src_data);
			if
			(
				pp->parent
			&&
				change_is_a_branch(project_change_get(pp))
			&&
				!change_file_up_to_date(pp->parent, src_data)
			)
			{
				fstate_src	psrc_data;

				psrc_data =
					project_file_find
					(
						pp->parent,
						src_data->file_name
					);
				if (psrc_data && psrc_data->edit_number)
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
				magic_zero_decode(src_data->about_to_be_created_by)
			);
		}
		if (src_data->deleted_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"Deleted by change %ld.",
				magic_zero_decode(src_data->deleted_by)
			);
		}
		if (src_data->locked_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"Locked by change %ld.",
				magic_zero_decode(src_data->locked_by)
			);
		}
		if (src_data->about_to_be_copied_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"About to be copied by change %ld.",
				magic_zero_decode(src_data->about_to_be_copied_by)
			);
		}
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
	project_ty	*pp;
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;
	int		left;

	trace(("list_administrators()\n{\n"/*}*/));
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name_get(pp));
	col_title(line1->str_text, "List of Administrators");
	str_free(line1);

	left = 0;
	login_col = col_create(left, left + LOGIN_WIDTH);
	left += LOGIN_WIDTH + 2;
	col_heading(login_col, "User\n------");

	if (!option_terse_get())
	{
		name_col = col_create(left, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list project's administrators
	 */
	for (j = 0; ; ++j)
	{
		string_ty	*logname;

		logname = project_administrator_nth(pp, j);
		if (!logname)
			break;
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
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
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_change_files()\n{\n"/*}*/));
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
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	cstate_data = change_cstate_get(cp);
	assert(change_file_nth(cp, 0));

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 =
		str_format
		(
			"Project \"%S\"  Change %ld",
			project_name_get(pp),
			magic_zero_decode(change_number)
		);
	col_title(line1->str_text, "List of Change's Files");
	str_free(line1);

	left = 0;
	if (!option_terse_get())
	{
		usage_col = col_create(left, left + USAGE_WIDTH);
		left += USAGE_WIDTH + 1;
		col_heading(usage_col, "Type\n-------");

		action_col = col_create(left, left + ACTION_WIDTH);
		left += ACTION_WIDTH + 1;
		col_heading(action_col, "Action\n--------");

		edit_col = col_create(left, left + EDIT_WIDTH);
		left += EDIT_WIDTH + 1;
		col_heading(edit_col, "Edit\n-------");
	}
	file_name_col = col_create(left, 0);
	col_heading(file_name_col, "File Name\n-----------");

	/*
	 * list the change's files
	 */
	for (j = 0; ; ++j)
	{
		fstate_src	src_data;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
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
			format_edit_number(edit_col, src_data);
			if
			(
			      cstate_data->state == cstate_state_being_developed
			&&
				!change_file_up_to_date(pp, src_data)
			)
			{
				fstate_src	psrc_data;

				/*
				 * The current head revision of the
				 * branch may not equal the version
				 * ``originally'' copied.
				 */
				psrc_data =
					project_file_find
					(
						pp,
						src_data->file_name
					);
				if (psrc_data && psrc_data->edit_number)
				{
					col_printf
					(
						edit_col,
						" (%s)",
						psrc_data->edit_number->str_text
					);
				}
			}
			if (src_data->edit_number_origin_new)
			{
				/*
				 * The ``cross branch merge'' version.
				 */
				col_bol(edit_col);
				col_printf
				(
					edit_col,
					"{cross %4s}",
				      src_data->edit_number_origin_new->str_text
				);
			}
		}
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
				magic_zero_decode(src_data->about_to_be_created_by)
			);
		}
		if (src_data->deleted_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"Deleted by change %ld.",
				magic_zero_decode(src_data->deleted_by)
			);
		}
		if (src_data->locked_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"Locked by change %ld.",
				magic_zero_decode(src_data->locked_by)
			);
		}
		if (src_data->about_to_be_copied_by)
		{
			col_bol(file_name_col);
			col_printf
			(
				file_name_col,
				"About to be copied by change %ld.",
				magic_zero_decode(src_data->about_to_be_copied_by)
			);
		}
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
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_change_history()\n{\n"/*}*/));
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
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	cstate_data = change_cstate_get(cp);
	assert(change_file_nth(cp, (size_t)0));

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 =
		str_format
		(
			"Project \"%S\"  Change %ld",
			project_name_get(pp),
			magic_zero_decode(change_number)
		);
	col_title(line1->str_text, "History");
	str_free(line1);

	left = 0;
	what_col = col_create(left, left + WHAT_WIDTH);
	left += WHAT_WIDTH + 1;
	col_heading(what_col, "What\n------");

	when_col = col_create(left, left + WHEN_WIDTH);
	left += WHEN_WIDTH + 1;
	col_heading(when_col, "When\n------");

	who_col = col_create(left, left + WHO_WIDTH);
	left += WHO_WIDTH + 1;
	col_heading(who_col, "Who\n-----");

	why_col = col_create(left, 0);
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
	int		number_col = 0;
	int		state_col = 0;
	int		description_col = 0;
	int		j;
	project_ty	*pp;
	string_ty	*line1;
	string_ty	*line2;
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_changes_in_state_mask(state_mask = 0x%X)\n{\n"/*}*/,
		state_mask));
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name_get(pp));
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

	left = 0;
	number_col = col_create(left, left + CHANGE_WIDTH);
	left += CHANGE_WIDTH + 1;
	col_heading(number_col, "Change\n-------");

	if (!option_terse_get())
	{
		state_col = col_create(left, left + STATE_WIDTH);
		left += STATE_WIDTH + 1;
		col_heading(state_col, "State\n-------");

		description_col = col_create(left, 0);
		col_heading(description_col, "Description\n-------------");
	}

	/*
	 * list the project's changes
	 */
	for (j = 0; ; ++j)
	{
		cstate		cstate_data;
		long		change_number;
		change_ty	*cp;

		if (!project_change_nth(pp, j, &change_number))
			break;
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
		cstate_data = change_cstate_get(cp);
		if (state_mask & (1 << cstate_data->state))
		{
			col_printf
			(
				number_col,
				"%4ld",
				magic_zero_decode(change_number)
			);
			if (!option_terse_get())
			{
				col_puts
				(
					state_col,
					cstate_state_ename(cstate_data->state)
				);
				if
				(
					option_verbose_get()
				&&
			      cstate_data->state == cstate_state_being_developed
				)
				{
					col_bol(state_col);
					col_puts
					(
						state_col,
					     change_developer_name(cp)->str_text
					);
				}
				if
				(
					option_verbose_get()
				&&
			     cstate_data->state == cstate_state_being_integrated
				)
				{
					col_bol(state_col);
					col_puts
					(
						state_col,
					    change_integrator_name(cp)->str_text
					);
				}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_project));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

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
	project_ty	*pp;
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;
	int		left;

	trace(("list_developers()\n{\n"/*}*/));
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name_get(pp));
	col_title(line1->str_text, "List of Developers");
	str_free(line1);

	left = 0;
	login_col = col_create(left, left + LOGIN_WIDTH);
	left += LOGIN_WIDTH + 2;
	col_heading(login_col, "User\n------");

	if (!option_terse_get())
	{
		name_col = col_create(left, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list the project's developers
	 */
	for (j = 0; ; ++j)
	{
		string_ty	*logname;

		logname = project_developer_nth(pp, j);
		if (!logname)
			break;
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_integrators(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;
	int		left;

	trace(("list_integrators()\n{\n"/*}*/));
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name_get(pp));
	col_title(line1->str_text, "List of Integrators");
	str_free(line1);

	left = 0;
	login_col = col_create(left, left + LOGIN_WIDTH);
	left += LOGIN_WIDTH + 2;
	col_heading(login_col, "User\n------");

	if (!option_terse_get())
	{
		name_col = col_create(left, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list the project's integrators
	 */
	for (j = 0; ; ++j)
	{
		string_ty	*logname;

		logname = project_integrator_nth(pp, j);
		if (!logname)
			break;
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_reviewers(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	int		login_col = 0;
	int		name_col = 0;
	int		j;
	string_ty	*line1;
	int		left;

	trace(("list_reviewers()\n{\n"/*}*/));
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name_get(pp));
	col_title(line1->str_text, "List of Reviewers");
	str_free(line1);

	left = 0;
	login_col = col_create(left, left + LOGIN_WIDTH);
	left += LOGIN_WIDTH + 2;
	col_heading(login_col, "User\n------");

	if (!option_terse_get())
	{
		name_col = col_create(left, 0);
		col_heading(name_col, "Full Name\n-----------");
	}

	/*
	 * list the project's reviewers
	 */
	for (j = 0; ; ++j)
	{
		string_ty	*logname;

		logname = project_reviewer_nth(pp, j);
		if (!logname)
			break;
		col_puts(login_col, logname->str_text);
		if (!option_terse_get())
			col_puts(name_col, user_full_name(logname));
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_project_history(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	int		name_col = 0;
	int		delta_col = 0;
	int		date_col = 0;
	int		change_col = 0;
	int		description_col = 0;
	size_t		j, k;
	project_ty	*pp;
	string_ty	*line1;
	int		left;

	trace(("list_project_history()\n{\n"/*}*/));
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * locate project data
	 */
	if (!project_name)
		project_name = user_default_project();
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	line1 = str_format("Project \"%S\"", project_name_get(pp));
	col_title(line1->str_text, "History");
	str_free(line1);

	/* the delta name column is the whole page wide */
	name_col = col_create(0, 0);

	left = 0;
	delta_col = col_create(left, left + CHANGE_WIDTH);
	left += CHANGE_WIDTH + 1;
	col_heading(delta_col, "Delta\n-------");

	if (!option_terse_get())
	{
		date_col = col_create(left, left + WHEN_WIDTH);
		left += WHEN_WIDTH + 1;
		col_heading(date_col, "Date and Time\n---------------");

		change_col = col_create(left, left + CHANGE_WIDTH);
		left += CHANGE_WIDTH + 1;
		col_heading(change_col, "Change\n-------");

		description_col = col_create(left, 0);
		col_heading(description_col, "Description\n-------------");
	}

	/*
	 * list the project's successful integrations
	 */
	for (j = 0; ; ++j)
	{
		long		cn;
		long		dn;
		string_list_ty	name;

		if (!project_history_nth(pp, j, &cn, &dn, &name))
			break;
		if
		(
			!option_terse_get()
		&&
			name.nstrings
		)
		{
			col_need(4);
			col_printf
			(
				name_col,
				"Name%s: ",
				(name.nstrings == 1 ? "" : "s")
			);
			for (k = 0; k < name.nstrings; ++k)
			{
				if (k)
					col_printf(name_col, ", ");
				col_printf
				(
					name_col,
					"\"%s\"",
					name.string[k]->str_text
				);
			}
		}
		col_printf(delta_col, "%4ld", dn);
		if (!option_terse_get())
		{
			cstate		cstate_data;
			time_t		t;
			change_ty	*cp;

			cp = change_alloc(pp, cn);
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
				cn
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
		string_list_destructor(&name);
	}

	/*
	 * clean up and go home
	 */
	col_close();
	project_free(pp);
	trace((/*{*/"}\n"));
}


void
list_version(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	cstate		cstate_data;
	change_ty	*cp;
	user_ty		*up;
	string_ty	*vs;

	/*
	 * locate project data
	 */
	trace(("list_version()\n{\n"/*}*/));
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
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);

	cstate_data = change_cstate_get(cp);
	vs = project_version_short_get(pp);
	if (option_terse_get())
	{
		if (cstate_data->state == cstate_state_being_developed)
		{
			/* ...punctuation? */
			printf
			(
				"%s.C%3.3ld\n",
				vs->str_text,
				magic_zero_decode(change_number)
			);
		}
		else
		{
			/* ...punctuation? */
			printf("%s.D%3.3ld\n", vs->str_text, cstate_data->delta_number);
		}
	}
	else
	{
		/*
		 * a century should be enough
		 * for a while, at least :-)
		 */
		int		cy[100];
		int		ncy;
		string_ty	*s;

		printf("version = \"%s\";\n", vs->str_text);
		if (cstate_data->state == cstate_state_being_developed)
		{
			printf
			(
				"change_number = %ld;\n",
				magic_zero_decode(change_number)
			);
		}
		else
		{
			printf
			(
				"delta_number = %ld;\n",
				cstate_data->delta_number
			);
		}
		s = project_version_previous_get(pp);
		if (s)
			printf("version_previous = \"%s\";\n", s->str_text);
		change_copyright_years_get(cp, cy, SIZEOF(cy), &ncy);
		if (ncy)
		{
			int		j;

			printf("copyright_years = [");
			for (j = 0; j < ncy; ++j)
			{
				if (j)
					printf(", ");
				printf("%d", cy[j]);
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
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_change_details()\n{\n"/*}*/));
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
			project_name_get(pp),
			magic_zero_decode(change_number)
		);
	col_title(line1->str_text, "Change Details");
	str_free(line1);

	/* the heading columns is the whole page wide */
	head_col = col_create(0, 0);

	/* the body columns is indented */
	body_col = col_create(INDENT_WIDTH, 0);
	col_puts(head_col, "NAME");
	col_eoln();
	col_printf(body_col, "Project \"%s\"", project_name_get(pp)->str_text);
	if (cstate_data->delta_number)
		col_printf(body_col, ", Delta %ld", cstate_data->delta_number);
	if (cstate_data->state < cstate_state_completed || option_verbose_get())
	{
		col_printf
		(
			body_col,
			", Change %ld",
			magic_zero_decode(change_number)
		);
	}
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
	 * show the sub-changes of a branch
	 */
	if (cstate_data->branch && option_verbose_get())
	{
		project_ty	*sub_pp;
		int		number_col;
		int		state_col;
		int		description_col;

		col_need(5);
		col_puts(head_col, "BRANCH CONTENTS");
		col_eoln();

		/*
		 * create the columns
		 */
		left = INDENT_WIDTH;
		number_col = col_create(left, left + CHANGE_WIDTH);
		left += CHANGE_WIDTH + 1;
		col_heading(number_col, "Change\n-------");

		state_col = col_create(left, left + STATE_WIDTH);
		left += STATE_WIDTH + 1;
		col_heading(state_col, "State\n-------");

		description_col = col_create(left, 0);
		col_heading(description_col, "Description\n-------------");

		/*
		 * list the sub changes
		 */
		sub_pp = project_bind_branch(pp, cp);
		for (j = 0; ; ++j)
		{
			long		sub_cn;
			change_ty	*sub_cp;
			cstate		sub_cstate_data;

			if (!project_change_nth(sub_pp, j, &sub_cn))
				break;
			sub_cp = change_alloc(sub_pp, sub_cn);
			change_bind_existing(sub_cp);

			sub_cstate_data = change_cstate_get(sub_cp);
			col_printf
			(
				number_col,
				"%4ld",
				magic_zero_decode(sub_cn)
			);
			col_puts
			(
				state_col,
				cstate_state_ename(sub_cstate_data->state)
			);
			if (cstate_data->brief_description)
			{
				col_puts
				(
					description_col,
					sub_cstate_data->brief_description->str_text
				);
			}
			col_eoln();
			change_free(sub_cp);
		}
		col_heading(number_col, (char *)0);
		col_heading(state_col, (char *)0);
		col_heading(description_col, (char *)0);
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
		left = INDENT_WIDTH;
		arch_col = col_create(left, left + ARCH_WIDTH);
		left += ARCH_WIDTH + 1;
		col_heading(arch_col, "arch.\n--------");

		host_col = col_create(left, left + HOST_WIDTH);
		left += HOST_WIDTH + 1;
		col_heading(host_col, "host\n--------");

		build_col = col_create(left, left + TIME_WIDTH);
		left += TIME_WIDTH + 1;
		col_heading(build_col, "aeb\n---------");

		test_col = col_create(left, left + TIME_WIDTH);
		left += TIME_WIDTH + 1;
		col_heading(test_col, "aet\n---------");

		test_bl_col = col_create(left, left + TIME_WIDTH);
		left += TIME_WIDTH + 1;
		col_heading(test_bl_col, "aet -bl\n---------");

		test_reg_col = col_create(left, left + TIME_WIDTH);
		left += TIME_WIDTH + 1;
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
	if (cstate_data->state != cstate_state_completed)
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
	if (change_file_nth(cp, (size_t)0))
	{
		int	usage_col;
		int	action_col;
		int	edit_col;
		int	file_name_col;

		left = INDENT_WIDTH;
		usage_col = col_create(left, left + USAGE_WIDTH);
		left += USAGE_WIDTH + 1;
		col_heading(usage_col, "Type\n-------");

		action_col = col_create(left, left + ACTION_WIDTH);
		left += ACTION_WIDTH + 1;
		col_heading(action_col, "Action\n--------");

		edit_col = col_create(left, left + EDIT_WIDTH);
		left += EDIT_WIDTH + 1;
		col_heading(edit_col, "Edit\n-------");

		file_name_col = col_create(left, 0);
		col_heading(file_name_col, "File Name\n-----------");
		for (j = 0; ; ++j)
		{
			fstate_src	src_data;
	
			src_data = change_file_nth(cp, j);
			if (!src_data)
				break;
			assert(src_data->file_name);
			col_puts(usage_col, file_usage_ename(src_data->usage));
			col_puts
			(
				action_col,
				file_action_ename(src_data->action)
			);
			format_edit_number(edit_col, src_data);
			if
			(
			      cstate_data->state == cstate_state_being_developed
			&&
				!change_file_up_to_date(pp, src_data)
			)
			{
				fstate_src	psrc_data;

				/*
				 * The current head revision of the
				 * branch may not equal the version
				 * ``originally'' copied.
				 */
				psrc_data =
					project_file_find
					(
						pp,
						src_data->file_name
					);
				if (psrc_data && psrc_data->edit_number)
				{
					col_printf
					(
						edit_col,
						" (%s)",
						psrc_data->edit_number->str_text
					);
				}
			}
			if (src_data->edit_number_origin_new)
			{
				/*
				 * The ``cross branch merge'' version.
				 */
				col_bol(edit_col);
				col_printf
				(
					edit_col,
					"{cross %4s}",
				      src_data->edit_number_origin_new->str_text
				);
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

		left = INDENT_WIDTH;
		what_col = col_create(left, left + WHAT_WIDTH);
		left += WHAT_WIDTH + 1;
		col_heading(what_col, "What\n------");

		when_col = col_create(left, left + WHEN_WIDTH);
		left += WHEN_WIDTH + 1;
		col_heading(when_col, "When\n------");

		who_col = col_create(left, left + WHO_WIDTH);
		left += WHO_WIDTH + 1;
		col_heading(who_col, "Who\n-----");

		why_col = col_create(left, 0);
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
	string_list_ty	name;
	long		j;
	int		left;

	trace(("list_user_changes()\n{\n"/*}*/));
	if (project_name)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_project));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * get the list of projects
	 */
	project_list_get(&name);
	if (!name.nstrings)
		goto done;

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
	left = 0;
	project_col = col_create(left, left + PROJECT_WIDTH);
	left += PROJECT_WIDTH + 1;
	col_heading(project_col, "Project\n----------");

	change_col = col_create(left, left + CHANGE_WIDTH);
	left += CHANGE_WIDTH + 1;
	col_heading(change_col, "Change\n------");

	state_col = col_create(left, left + STATE_WIDTH);
	left += STATE_WIDTH + 1;
	col_heading(state_col, "State\n----------");

	description_col = col_create(left, 0);
	col_heading(description_col, "Description\n-------------");

	/*
	 * for each project, see if the current user
	 * is working on any of them.
	 */
	for (j = 0; j < name.nstrings; ++j)
	{
		project_ty	*pp;
		int		err;
		long		n;

		/*
		 * locate the project,
		 * and make sure we are allowed to look at it
		 */
		pp = project_alloc(name.string[j]);
		project_bind_existing(pp);
		err = project_is_readable(pp);
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
			col_printf
			(
				change_col,
				"%4ld",
				magic_zero_decode(change_number)
			);
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
static string_list_ty list_locks_pnames;
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
	static itab_ty	*user_name_by_uid;
	string_ty	*s;

	list_locks_count++;
	name_str = "unknown";
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

	case lock_walk_name_baseline:
		name_str = "baseline";
		break;

	case lock_walk_name_history:
		name_str = "history";
		break;

	case lock_walk_name_unknown:
		break;
	}

	type_str = "unknown";
	switch (found->type)
	{
	case lock_walk_type_shared:
		type_str = "shared";
		break;

	case lock_walk_type_exclusive:
		type_str = "exclusive";
		break;

	case lock_walk_type_unknown:
		break;
	}

	project_str = 0;
	change_number = 0;
	switch (found->name)
	{
	case lock_walk_name_pstate:
	case lock_walk_name_baseline:
	case lock_walk_name_history:
		for (j = 0; j < list_locks_pnames.nstrings; ++j)
		{
			s = list_locks_pnames.string[j];
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
		for (j = 0; j < list_locks_pnames.nstrings; ++j)
		{
			long	cn;

			/*
			 * This is very messy, because the change
			 * number is added to the project name hash.
			 *
			 * Work out the change number,
			 * and the assume it can't be more than,
			 * say, 5000.  We could do better by looking
			 * in each project, but what the hell.
			 *
			 * Use (mod 2**16) arithmetic, that's how its done.
			 * Should bring this out in lock.h if ever change.
			 *
			 * Loop and find the smallest change number.
			 * Use that as a probable "best" fit.
			 */
			s = list_locks_pnames.string[j];
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

	case lock_walk_name_ustate:
		/* This is the UID */
		change_number = found->subset;

		if (!user_name_by_uid)
		{
			user_name_by_uid = itab_alloc(100);
			setpwent();
			for (;;)
			{
				struct passwd	*pw;

				pw = getpwent();
				if (!pw)
					break;
				s = str_from_c(pw->pw_name);
				itab_assign(user_name_by_uid, pw->pw_uid, s);
			}
			endpwent();
		}
		s = itab_query(user_name_by_uid, change_number);
		project_str = s ? s->str_text : "unknown";
		break;

	case lock_walk_name_master:
	case lock_walk_name_gstate:
	case lock_walk_name_unknown:
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
	{
		col_printf
		(
			list_locks_change_col,
			"%4ld",
			magic_zero_decode(change_number)
		);
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_project));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * get the list of projects
	 */
	project_list_get(&list_locks_pnames);

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
	string_list_destructor(&list_locks_pnames);
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_project));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * resolve the project name
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
	 * Find default change number;
	 * will generate fatal error if no default.
	 */
	change_number = user_default_change(up);

	/*
	 * print it out
	 */
	printf("%ld\n", magic_zero_decode(change_number));

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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
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
	int		project_col = 0;
	int		number_col = 0;
	int		state_col = 0;
	int		description_col = 0;
	int		j, k;
	string_list_ty	name;
	int		left;

	trace(("list_outstanding_changes_all()\n{\n"/*}*/));
	if (project_name)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_project));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (change_number)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_change));
		fatal_intl(scp, i18n("inappropriate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * list the projects
	 */
	project_list_get(&name);

	/*
	 * create the columns
	 */
	col_open((char *)0);
	col_title("List of Outstanding Changes", "for all projects");

	left = 0;
	project_col = col_create(left, left + PROJECT_WIDTH);
	left += PROJECT_WIDTH + 1;
	col_heading(project_col, "Project\n---------");

	number_col = col_create(left, left + CHANGE_WIDTH);
	left += CHANGE_WIDTH + 1;
	col_heading(number_col, "Change\n------");

	if (!option_terse_get())
	{
		state_col = col_create(left, left + STATE_WIDTH);
		left += STATE_WIDTH + 1;
		col_heading(state_col, "State\n-------");

		description_col = col_create(left, 0);
		col_heading(description_col, "Description\n-------------");
	}

	/*
	 * scan each project
	 */
	for (j = 0; j < name.nstrings; ++j)
	{
		project_ty	*pp;
		int		err;

		pp = project_alloc(name.string[j]);
		project_bind_existing(pp);

		/*
		 * make sure we have permission
		 */
		err = project_is_readable(pp);
		if (err)
		{
			project_free(pp);
			continue;
		}

		/*
		 * list the project's changes
		 */
		for (k = 0; ; ++k)
		{
			cstate		cstate_data;
			change_ty	*cp;

			/*
			 * make sure the change is not completed
			 */
			if (!project_change_nth(pp, k, &change_number))
				break;
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
			col_printf
			(
				number_col,
				"%4ld",
				magic_zero_decode(change_number)
			);
			if (!option_terse_get())
			{
				col_puts
				(
					state_col,
					cstate_state_ename(cstate_data->state)
				);
				if
				(
					option_verbose_get()
				&&
			      cstate_data->state == cstate_state_being_developed
				)
				{
					col_bol(state_col);
					col_puts
					(
						state_col,
					     change_developer_name(cp)->str_text
					);
				}
				if
				(
					option_verbose_get()
				&&
			     cstate_data->state == cstate_state_being_integrated
				)
				{
					col_bol(state_col);
					col_puts
					(
						state_col,
					    change_integrator_name(cp)->str_text
					);
				}
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

			/*
			 * At some point, will need to recurse
			 * if it is a branch and not a leaf.
			 */
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
