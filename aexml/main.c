/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
#include <ac/string.h>

#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <col.h>
#include <env.h>
#include <error.h>
#include <help.h>
#include <language.h>
#include <option.h>
#include <os.h>
#include <output/file.h>
#include <output/gzip.h>
#include <progname.h>
#include <r250.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <version.h>
#include <xml/change/cstate.h>
#include <xml/change/fstate.h>
#include <xml/change/pconf.h>
#include <xml/project/cstate.h>
#include <xml/project/fstate.h>
#include <xml/project/list.h>
#include <xml/project/state.h>
#include <xml/user/uconf.h>
#include <zero.h>


typedef struct table_ty table_ty;
struct table_ty
{
    const char      *name;
    const char      *description;
    void	    (*func)(string_ty *, long, output_ty *);
};


static table_ty table[] =
{
    {
	"Change_Files",
	"Internal change file state.  See aefstate(5) for structure.",
	xml_change_fstate,
    },
    {
	"Change_State",
	"Internal change state.  See aecstate(5) for structure.",
	xml_change_cstate,
    },
    {
	"Project",
	"List of projects.  See aegstate(5) for structure.",
	xml_project_list,
    },
    {
	"Project_Change_State",
	"Internal project change state.  See aecstate(5) for structure.",
	xml_project_cstate,
    },
    {
	"Project_Config_File",
	"The project config file.  See aepconf(5) for structure.",
	xml_change_pconf,
    },
    {
	"Project_Files",
	"Internal project file state.  See aefstate(5) for structure.",
	xml_project_fstate,
    },
    {
	"Project_State",
	"Internal project state.  See aepstate(5) for structure.",
	xml_project_pstate,
    },
    {
	"User_Config_File",
	"The user config file.  See aeuconf(5) for structure.",
	xml_user_uconf,
    },
};


static int
ends_with(string_ty *haystack, const char *needle)
{
    size_t          needle_length;

    needle_length = strlen(needle);
    return
	(
	    haystack->str_length > needle_length
	&&
	    (
		0
	    ==
		memcmp
		(
		    haystack->str_text + haystack->str_length - needle_length,
		    needle,
		    needle_length
		)
	    )
	);
}


static void
via_table(const char *listname, string_ty *project_name, long change_number,
    string_ty *outfile)
{
    size_t          nhit;
    table_ty        *tp;
    output_ty       *op;
    table_ty        *hit[SIZEOF(table)];
    sub_context_ty  *scp;
    string_ty       *s1;
    string_ty       *s2;
    size_t          j;

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
	os_become_orig();
	if (outfile && ends_with(outfile, ".gz"))
	{
	    op = output_file_binary_open(outfile);
	    op = output_gzip(op);
	}
	else
	    op = output_file_text_open(outfile);
	os_become_undo();
	hit[0]->func(project_name, change_number, op);
	output_delete(op);
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
}


static void
xml_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ] <xml-name>\n", progname);
    fprintf
    (
	stderr,
	"       %s [ <option>... ] -File <filename>\n",
	progname
    );
    fprintf(stderr, "       %s -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
xml_help(void)
{
    help((char *)0, xml_usage);
}


static void
xml_list(void)
{
    col_ty          *colp;
    output_ty       *name_col = 0;
    output_ty       *desc_col = 0;
    table_ty        *tp;

    trace(("xml_list()\n{\n"));
    arglex();
    while (arglex_token != arglex_token_eoln)
	generic_argument(xml_usage);


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


static void
xml_main(void)
{
    string_ty	    *project_name;
    long	    change_number;
    string_ty	    *outfile;
    const char      *report_name;

    trace(("xml_main()\n{\n"));
    project_name = 0;
    change_number = 0;
    report_name = 0;
    outfile = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(xml_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    arglex_parse_change(&project_name, &change_number, xml_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, xml_usage);
	    continue;

	case arglex_token_output:
	    if (outfile)
		duplicate_option(xml_usage);
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_output, xml_usage);
	    outfile = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_string:
	    if (report_name)
		fatal_intl(0, i18n("too many lists"));
	    report_name = arglex_value.alv_string;
	    break;
	}
	arglex();
    }
    if (!report_name)
	fatal_intl(0, i18n("no list"));

    /*
     * Find the report and run it.
     */
    via_table(report_name, project_name, change_number, outfile);

    /*
     * clean up and go home
     */
    if (project_name)
	str_free(project_name);
    if (outfile)
	str_free(outfile);
    trace(("}\n"));
}


int
main(int argc, char **argv)
{
    r250_init();
    os_become_init_mortal();
    arglex2_init(argc, argv);
    str_initialize();
    env_initialize();
    language_init();
    switch (arglex())
    {
    default:
	xml_main();
	break;

    case arglex_token_help:
	xml_help();
	break;

    case arglex_token_list:
	xml_list();
	break;

    case arglex_token_version:
	version();
	break;
    }
    exit(0);
    return 0;
}
