//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2001-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: operating system entry point
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <libaegis/aer/func/change.h>
#include <libaegis/aer/func/project.h>
#include <libaegis/aer/list.h>
#include <libaegis/aer/parse.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <common/env.h>
#include <libaegis/help.h>
#include <common/language.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <common/str_list.h>
#include <libaegis/version.h>
#include <libaegis/zero.h>


//
// NAME
//	report_usage
//
// SYNOPSIS
//	void report_usage(void);
//
// DESCRIPTION
//	The report_usage function is used to
//	briefly describe how to used the 'aegis -RePorT' command.
//

static void
report_usage(void)
{
    const char      *progname;

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


//
// NAME
//	report_help
//
// SYNOPSIS
//	void report_help(void);
//
// DESCRIPTION
//	The report_help function is used to
//	describe in detail how to use the 'aegis -RePorT' command.
//

static void
report_help(void)
{
	help("aer", report_usage);
}


//
// NAME
//	report_main
//
// SYNOPSIS
//	void report_main(void);
//
// DESCRIPTION
//	The report_main function is used to
//	report a change in the "being developed" or "being integrated" states.
//	It extracts what to do from the command line.
//

static void
report_main(void)
{
    trace(("report_main()\n{\n"));
    string_ty *project_name = 0;
    long change_number = 0;
    string_ty *infile = 0;
    string_ty *outfile = 0;
    string_list_ty arg;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(report_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    arglex_parse_change(&project_name, &change_number, report_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, report_usage);
	    continue;

	case arglex_token_file:
	    if (infile)
		duplicate_option(report_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, report_usage);
		// NOTREACHED

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
	    {
		string_ty *s = str_from_c(arglex_value.alv_string);
		arg.push_back(s);
		str_free(s);
	    }
	    break;
	}
	arglex();
    }
    if (infile)
    {
	trace(("prepending report file name to args\n"));
	arg.push_front(infile);
    }
    else if (arg.nstrings == 0)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	error_intl(scp, i18n("no report name"));
	report_usage();
	// NOTREACHED
    }

    //
    // pass the arguments to the report
    //
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

    //
    // run the report
    //
    report_run();

    //
    // clean up and go home
    //
    if (project_name)
	str_free(project_name);
    if (infile)
	str_free(infile);
    if (outfile)
	str_free(outfile);
    trace(("}\n"));
}


//
// NAME
//	report
//
// SYNOPSIS
//	void report(void);
//
// DESCRIPTION
//	The report function is used to
//	dispatch the 'aegis -RePorT' command to the relevant functionality.
//	Where it goes depends on the command line.
//

int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init_mortal();
    arglex2_init(argc, argv);
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

    case arglex_token_version:
	version();
	break;
    }
    quit(0);
    return 0;
}
