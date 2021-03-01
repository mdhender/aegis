/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1999, 2001, 2002 Peter Miller;
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

#include <ac/stdio.h>

#include <ael.h>
#include <ael/change/changes.h>
#include <ael/change/default.h>
#include <ael/change/details.h>
#include <ael/change/files.h>
#include <ael/change/file_history.h>
#include <ael/change/history.h>
#include <ael/change/inappropriat.h>
#include <ael/change/outstanding.h>
#include <ael/change/outstand_all.h>
#include <ael/change/user.h>
#include <ael/change/version.h>
#include <ael/column_width.h>
#include <ael/formeditnum.h>
#include <ael/locks.h>
#include <output.h>
#include <ael/project/administrato.h>
#include <ael/project/aliases.h>
#include <ael/project/default.h>
#include <ael/project/developers.h>
#include <ael/project/files.h>
#include <ael/project/history.h>
#include <ael/project/inappropriat.h>
#include <ael/project/integrators.h>
#include <ael/project/projects.h>
#include <ael/project/reviewers.h>
#include <ael/state_file.h>
#include <arglex2.h>
#include <col.h>
#include <error.h>
#include <help.h>
#include <option.h>
#include <progname.h>
#include <sub.h>
#include <trace.h>
#include <zero.h>


static void list_list_list(string_ty *, long); /* forward */


typedef struct table_ty table_ty;
struct table_ty
{
    char	    *name;
    char	    *description;
    void	    (*func)(string_ty *, long);
};


static table_ty table[] =
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
	"Change_File_History",
	"List the history of all files in a change.",
	list_change_file_history,
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
	"Project_Aliases",
	"List all project aliases",
	list_project_aliases,
    },
    {
	"Reviewers",
	"List the reviewers of a project",
	list_reviewers,
    },
    {
	"State_File_Name",
	"List the full path name of the change's state file.",
	list_state_file_name,
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


static void
list_usage(void)
{
    char	    *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -List [ <option>... ] <listname>\n", progname);
    fprintf(stderr, "       %s -List -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -List -Help\n", progname);
    quit(1);
}


static void
list_help(void)
{
    help("ael", list_usage);
}


static void
list_list(void)
{
    trace(("list_list()\n{\n"));
    arglex();
    while (arglex_token != arglex_token_eoln)
	generic_argument(list_usage);
    list_list_list(0, 0);
    trace(("}\n"));
}


static void
list_main(void)
{
    sub_context_ty  *scp;
    char	    *listname;
    table_ty	    *tp;
    int		    j;
    table_ty	    *hit[SIZEOF(table)];
    int		    nhit;
    string_ty	    *s1;
    string_ty	    *s2;
    string_ty	    *project_name;
    long	    change_number;

    trace(("list_main()\n{\n"));
    arglex();
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
	sub_var_set_charstar(scp, "Name", listname);
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
	sub_var_set_charstar(scp, "Name", listname);
	sub_var_set_string(scp, "Name_List", s1);
	str_free(s1);
	sub_var_optional(scp, "Name_List");
	fatal_intl(scp, i18n("list $name ambiguous"));
	/* NOTREACHED */
	sub_context_delete(scp);
	break;
    }
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


void
list(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, list_help, },
	{arglex_token_list, list_list, },
    };

    trace(("list()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), list_main);
    trace(("}\n"));
}


static void
list_list_list(string_ty *project_name, long change_number)
{
    output_ty	    *name_col =	    0;
    output_ty	    *desc_col =	    0;
    table_ty	    *tp;
    col_ty	    *colp;

    trace(("list_list_list()\n{\n"));
    if (project_name)
	list_project_inappropriate();
    if (change_number)
	list_change_inappropriate();

    /*
     * create the columns
     */
    colp = col_open((string_ty *)0);
    col_title(colp, "List of Lists", (const char *)0);
    name_col = col_create(colp, 0, 15, "Name\n------");
    if (!option_terse_get())
    {
	desc_col = col_create(colp, 16, 0, "Description\n-------------");
    }

    /*
     * list the lists
     */
    for (tp = table; tp < ENDOF(table); ++tp)
    {
	output_fputs(name_col, tp->name);
	if (desc_col)
	    output_fputs(desc_col, tp->description);
	col_eoln(colp);
    }

    /*
     * clean up and go home
     */
    col_close(colp);
    trace(("}\n"));
}
