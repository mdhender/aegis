//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2001-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
//
// If you add another list to this file,
// don't forget to update man1/ael.1
//

#include <common/ac/stdio.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/branch/files.h>
#include <libaegis/ael/change/changes.h>
#include <libaegis/ael/change/default.h>
#include <libaegis/ael/change/details.h>
#include <libaegis/ael/change/file_history.h>
#include <libaegis/ael/change/file_invento.h>
#include <libaegis/ael/change/files.h>
#include <libaegis/ael/change/history.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/change/incomplete.h>
#include <libaegis/ael/change/inventory.h>
#include <libaegis/ael/change/outstand_all.h>
#include <libaegis/ael/change/outstanding.h>
#include <libaegis/ael/change/user.h>
#include <libaegis/ael/change/version.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/formeditnum.h>
#include <libaegis/ael/locks.h>
#include <libaegis/ael/project/administrato.h>
#include <libaegis/ael/project/aliases.h>
#include <libaegis/ael/project/default.h>
#include <libaegis/ael/project/details.h>
#include <libaegis/ael/project/developers.h>
#include <libaegis/ael/project/file_invento.h>
#include <libaegis/ael/project/filesbydelta.h>
#include <libaegis/ael/project/files.h>
#include <libaegis/ael/project/history.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/ael/project/integrators.h>
#include <libaegis/ael/project/projects.h>
#include <libaegis/ael/project/reviewers.h>
#include <libaegis/ael/state_file.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/help.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/sub.h>
#include <libaegis/zero.h>

#include <aegis/ael.h>


static void list_list_list(change_identifier &, string_list_ty *); // forward


struct table_ty
{
    const char *name;
    const char *description;
    void (*func)(change_identifier &, string_list_ty *);
    int number_allowed_args;
};


static table_ty table[] =
{
    {
        "Administrators",
        "List the administrators of a project",
        list_administrators,
        0,
    },
    {
        "Branch_Files",
        "List the files in a branch",
        list_branch_files,
        0,
    },
    {
        "Change_Details",
        "List all information about a change in large layout form.",
        list_change_details,
        0,
    },
    {
        "Change_Files",
        "List all files in a change",
        list_change_files,
        0,
    },
    {
        "Change_File_History",
        "List the history of all files in a change.",
        list_change_file_history,
        0,
    },
    {
        "Change_File_INventory",
        "List all files in a change with their UUID.",
        list_change_file_inventory,
        0,
    },
    {
        "Change_History",
        "List the history of a change",
        list_change_history,
        0,
    },
    {
        "Change_INventory",
        "List the changes of a project with their UUID.",
        list_change_inventory,
        0,
    },
    {
        "Changes",
        "List the changes of a project",
        list_changes,
        0,
    },
    {
        "Default_Change",
        "List the default change for the current user",
        list_default_change,
        1,
    },
    {
        "Default_Project",
        "List the default project for a (the current) user",
        list_default_project,
        1,
    },
    {
        "Developers",
        "List the developers of a project",
        list_developers,
        0,
    },
    {
        "INComplete",
        "List the incomplete changes (for a user)",
        list_incomplete_changes,
        0,
    },
    {
        "Integrators",
        "List the integrators of a project",
        list_integrators,
        0,
    },
    {
        "List_List",
        "List all lists available",
        list_list_list,
        0,
    },
    {
        "Locks",
        "List the active locks",
        list_locks,
        0,
    },
    {
        "Outstanding_Changes",
        "List the outstanding changes (for a user)",
        list_outstanding_changes,
        1,
    },
    {
        "All_Outstanding_Changes",
        "List the outstanding changes for all projects",
        list_outstanding_changes_all,
        0,
    },
    {
        "Branch_Details",
        "List the details of all changes in the branch (recursive)",
        list_project_details,
        0,
    },
    {
        "Project_Details",
        "List the details of all changes in the project (recursive)",
        list_project_details,
        0,
    },
    {
        "Project_Files",
        "List all files in the baseline of a project",
        list_project_files,
        0,
    },
    {
        "Project_Files_By_Delta",
        "List all files in the baseline of a project, as they appeared "
            "immediately after the integrate pass of the specified "
            "change or delta.",
        list_project_files_by_delta,
        0,
    },
    {
        "Project_File_INventory",
        "List all files in the baseline of a project with their UUID.",
        list_project_file_inventory,
        0,
    },
    {
        "Project_History",
        "List the integration history of a project",
        list_project_history,
        0,
    },
    {
        "Projects",
        "List all projects",
        list_projects,
        0,
    },
    {
        "Project_Aliases",
        "List all project aliases (the -Project option limits results)",
        list_project_aliases,
        0,
    },
    {
        "Reviewers",
        "List the reviewers of a project",
        list_reviewers,
        0,
    },
    {
        "State_File_Name",
        "List the full path name of the change's state file.",
        list_state_file_name,
        0,
    },
    {
        "Users_Changes",
        "List of changes owned by a (the current) user",
        list_user_changes,
        1,
    },
    {
        "Version",
        "List version of a project or change",
        list_version,
        0,
    },
};


static void
list_usage(void)
{
    const char      *progname;

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
    change_identifier cid;
    while (arglex_token != arglex_token_eoln)
        generic_argument(list_usage);
    list_list_list(cid, 0);
    trace(("}\n"));
}


static int
max_num_table_args(void)
{
    size_t          i;
    int             max_num_args = 0;
    trace(("max_num_table_args()\n{\n"));
    for (i = 0; i < SIZEOF(table); i++)
    {
        if (table[i].number_allowed_args > max_num_args)
        {
            max_num_args = table[i].number_allowed_args;
        }
    }
    trace(("\tmax_num_args = %d\n", max_num_args));
    trace(("}\n"));
    return max_num_args;
}


static void
list_main(void)
{
    sub_context_ty  *scp;
    table_ty        *tp;
    int             j;
    table_ty        *hit[SIZEOF(table)];
    int             nhit;
    string_ty       *s1;
    string_ty       *s2;

    trace(("list_main()\n{\n"));
    arglex();
    const char *listname = 0;
    change_identifier cid;
    string_list_ty args_list;
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
                string_ty       *s;

                s = str_from_c(arglex_value.alv_string);
                args_list.push_back(s);
                str_free(s);

                if (args_list.nstrings > (size_t)max_num_table_args())
                {
                    fatal_intl(0, i18n("too many lists"));
                }
            }
            else
            {
                listname = arglex_value.alv_string;
            }
            break;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_development_directory:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(list_usage);
            continue;
        }
        arglex();
    }
    cid.command_line_check(list_usage);
    if (!listname)
        fatal_intl(0, i18n("no list"));

    nhit = 0;
    for (tp = table; tp < ENDOF(table); ++tp)
    {
        if (arglex_compare(tp->name, listname, 0))
            hit[nhit++] = tp;
    }
    switch (nhit)
    {
    case 0:
        scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", listname);
        fatal_intl(scp, i18n("no $name list"));
        // NOTREACHED
        sub_context_delete(scp);

    case 1:
        hit[0]->func(cid, &args_list);
        break;

    default:
        s1 = str_from_c(hit[0]->name);
        for (j = 1; j < nhit; ++j)
        {
            s2 = str_format("%s, %s", s1->str_text, hit[j]->name);
            str_free(s1);
            s1 = s2;
        }
        scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", listname);
        sub_var_set_string(scp, "Name_List", s1);
        str_free(s1);
        sub_var_optional(scp, "Name_List");
        fatal_intl(scp, i18n("list $name ambiguous"));
        // NOTREACHED
        sub_context_delete(scp);
        break;
    }
    trace(("}\n"));
}


void
list(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, list_help, 0 },
        { arglex_token_list, list_list, 0 },
    };

    trace(("list()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), list_main);
    trace(("}\n"));
}


static void
list_list_list(change_identifier &cid, string_list_ty *)
{
    trace(("list_list_list()\n{\n"));
    if (cid.project_set())
        list_project_inappropriate();
    if (cid.set())
        list_change_inappropriate();

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    colp->title("List of Lists", (const char *)0);
    output::pointer name_col = colp->create(0, 15, "Name\n------");
    output::pointer desc_col;
    if (!option_terse_get())
    {
        desc_col = colp->create(16, 0, "Description\n-------------");
    }

    //
    // list the lists
    //
    for (table_ty *tp = table; tp < ENDOF(table); ++tp)
    {
        name_col->fputs(tp->name);
        if (desc_col)
            desc_col->fputs(tp->description);
        colp->eoln();
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
