//
//	aegis - project change supervisor
//	Copyright (C) 1994-1997, 1999, 2001-2008 Peter Miller
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

#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/aer/func/change.h>
#include <libaegis/aer/func/project.h>
#include <libaegis/aer/list.h>
#include <libaegis/aer/parse.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/sub.h>
#include <libaegis/zero.h>

#include <aegis/aer.h>


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
    fprintf
    (
	stderr,
	"usage: %s -RePorT [ <option>... ] <report-name>\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -RePorT [ <option>... ] -File <filename>\n",
	progname
    );
    fprintf(stderr, "       %s -RePorT -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -RePorT -Help\n", progname);
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


static void
report_list2(void)
{
    report_list(report_usage);
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
    arglex();
    change_identifier cid;
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

	case arglex_token_baseline:
	case arglex_token_change:
	case arglex_token_delta:
	case arglex_token_delta_date:
        case arglex_token_delta_from_change:
	case arglex_token_grandparent:
	case arglex_token_project:
	case arglex_token_trunk:
	    cid.command_line_parse(report_usage);
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
    cid.command_line_check(report_usage);
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
    report_parse_project_set(cid);
    report_parse_change_set(cid);
    report_parse_argument_set(&arg);

    //
    // run the report
    //
    report_run();

    //
    // clean up and go home
    //
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

void
report(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, report_help, 0 },
	{ arglex_token_list, report_list2, 0 },
    };

    trace(("report()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), report_main);
    trace(("}\n"));
}
