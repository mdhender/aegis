/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1999, 2001-2003 Peter Miller;
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
 * MANIFEST: operating system entry point
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <aer/func/change.h>
#include <aer/func/project.h>
#include <aer/list.h>
#include <aer/parse.h>
#include <arglex2.h>
#include <env.h>
#include <error.h>
#include <help.h>
#include <language.h>
#include <os.h>
#include <progname.h>
#include <r250.h>
#include <sub.h>
#include <trace.h>
#include <str_list.h>
#include <zero.h>

/*
 * NAME
 *	report_usage
 *
 * SYNOPSIS
 *	void report_usage(void);
 *
 * DESCRIPTION
 *	The report_usage function is used to
 *	briefly describe how to used the 'aegis -RePorT' command.
 */

static void
report_usage(void)
{
    char	    *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ] <report-name>\n", progname);
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


/*
 * NAME
 *	report_help
 *
 * SYNOPSIS
 *	void report_help(void);
 *
 * DESCRIPTION
 *	The report_help function is used to
 *	describe in detail how to use the 'aegis -RePorT' command.
 */

static void
report_help(void)
{
	help("aer", report_usage);
}


/*
 * NAME
 *	report_main
 *
 * SYNOPSIS
 *	void report_main(void);
 *
 * DESCRIPTION
 *	The report_main function is used to
 *	report a change in the "being developed" or "being integrated" states.
 *	It extracts what to do from the command line.
 */

static void
report_main(void)
{
    string_ty	    *project_name;
    long	    change_number;
    string_list_ty  arg;
    string_ty	    *infile;
    string_ty	    *outfile;
    string_ty	    *s;

    trace(("report_main()\n{\n"));
    project_name = 0;
    change_number = 0;
    infile = 0;
    outfile = 0;
    string_list_constructor(&arg);
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(report_usage);
	    continue;

	case arglex_token_change:
	    if (change_number)
		duplicate_option(report_usage);
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, report_usage);
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		sub_context_ty *scp;

		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/*NOTREACHED*/
	    }
	    break;

	case arglex_token_project:
	    if (project_name)
		duplicate_option(report_usage);
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, report_usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_file:
	    if (infile)
		duplicate_option(report_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, report_usage);
		/*NOTREACHED*/

	    case arglex_token_string:
		trace(("accepting -File option\n"));
		infile = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		infile = str_from_c("");
		break;
	    }
	    break;

	case arglex_token_output:
	    if (outfile)
		duplicate_option(report_usage);
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_output, report_usage);
	    outfile = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_string:
	case arglex_token_number:
	    s = str_from_c(arglex_value.alv_string);
	    string_list_append(&arg, s);
	    str_free(s);
	    break;
	}
	arglex();
    }
    if (infile)
    {
	trace(("prepending report file name to args\n"));
	string_list_prepend(&arg, infile);
    }
    else if (arg.nstrings == 0)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	error_intl(scp, i18n("no report name"));
	report_usage();
	/*NOTREACHED*/
    }

    /*
     * pass the arguments to the report
     */
    if (infile)
    {
	trace(("setting input file name\n"));
	report_parse_filename_set(infile);
    }
    if (outfile)
    {
	trace(("setting output file name\n"));
	report_parse_output_set(outfile);
    }
    if (project_name)
	report_parse_project_set(project_name);
    if (change_number)
	report_parse_change_set(change_number);
    report_parse_argument_set(&arg);

    /*
     * run the report
     */
    report_run();

    /*
     * clean up and go home
     */
    if (project_name)
	str_free(project_name);
    if (infile)
	str_free(infile);
    if (outfile)
	str_free(outfile);
    string_list_destructor(&arg);
    trace(("}\n"));
}


/*
 * NAME
 *	report
 *
 * SYNOPSIS
 *	void report(void);
 *
 * DESCRIPTION
 *	The report function is used to
 *	dispatch the 'aegis -RePorT' command to the relevant functionality.
 *	Where it goes depends on the command line.
 */

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
	report_main();
	break;

    case arglex_token_help:
	report_help();
	break;

    case arglex_token_list:
	report_list(report_usage);
	break;
    }
    exit(0);
    return 0;
}
