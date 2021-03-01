//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/col.h>
#include <common/env.h>
#include <libaegis/help.h>
#include <common/language.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/output/bzip2.h>
#include <libaegis/output/file.h>
#include <libaegis/output/gzip.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/version.h>
#include <aexml/xml/change/cstate.h>
#include <aexml/xml/change/fstate.h>
#include <aexml/xml/change/pconf.h>
#include <aexml/xml/project/cstate.h>
#include <aexml/xml/project/fstate.h>
#include <aexml/xml/project/list.h>
#include <aexml/xml/project/state.h>
#include <aexml/xml/user/uconf.h>
#include <libaegis/zero.h>


struct table_ty
{
    const char      *name;
    const char      *description;
    void	    (*func)(string_ty *, long, output::pointer );
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
    output::pointer op;
    sub_context_ty  *scp;
    string_ty       *s1;
    string_ty       *s2;

    size_t nhit = 0;
    table_ty *hit[SIZEOF(table)];
    for (table_ty *tp = table; tp < ENDOF(table); ++tp)
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
	os_become_orig();
	if (outfile && ends_with(outfile, ".gz"))
	{
	    op = output_file::binary_open(outfile);
	    op = output_gzip::create(op);
	}
	else if
	(
	    outfile
	&&
	    (ends_with(outfile, ".bz") || ends_with(outfile, ".bz2"))
	)
	{
	    op = output_file::binary_open(outfile);
	    op = output_bzip2::create(op);
	}
	else
	    op = output_file::text_open(outfile);
	os_become_undo();
	hit[0]->func(project_name, change_number, op);
	op.reset();
	break;

    default:
	s1 = str_from_c(hit[0]->name);
	for (size_t j = 1; j < nhit; ++j)
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
}


static void
xml_usage(void)
{
    const char *progname = progname_get();
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
    trace(("xml_list()\n{\n"));
    arglex();
    while (arglex_token != arglex_token_eoln)
	generic_argument(xml_usage);


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

    //
    // Find the report and run it.
    //
    via_table(report_name, project_name, change_number, outfile);

    //
    // clean up and go home
    //
    if (project_name)
	str_free(project_name);
    if (outfile)
	str_free(outfile);
    trace(("}\n"));
}


int
main(int argc, char **argv)
{
    arglex2_init(argc, argv);
    resource_limits_init();
    os_become_init_mortal();
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
    quit(0);
    return 0;
}
