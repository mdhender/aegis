/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 * MANIFEST: functions to implement the 'aegis -RePorT' command
 */

#include <stdio.h>

#include <aer.h>
#include <aer/list.h>
#include <aer/parse.h>
#include <aer/func/change.h>
#include <aer/func/project.h>
#include <arglex2.h>
#include <error.h>
#include <help.h>
#include <option.h>
#include <trace.h>


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

static void report_usage _((void));

static void
report_usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s -RePorT [ <option>... ] <report-name>\n", progname);
	fprintf(stderr, "       %s -RePorT [ <option>... ] -File <filename>\n", progname);
	fprintf(stderr, "       %s -RePorT -List [ <option>... ]\n", progname);
	fprintf(stderr, "       %s -RePorT -Help\n", progname);
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

static void report_help _((void));

static void
report_help()
{
	static char *text[] =
	{
#include <../man1/aer.h>
	};

	help(text, SIZEOF(text), report_usage);
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

static void report_main _((void));

static void
report_main()
{
	string_ty	*project_name;
	long		change_number;
	wlist		arg;
	string_ty	*infile;
	string_ty	*outfile;
	string_ty	*s;

	trace(("report_main()\n{\n"/*}*/));
	project_name = 0;
	change_number = 0;
	infile = 0;
	outfile = 0;
	wl_zero(&arg);
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			generic_argument(report_usage);
			continue;

		case arglex_token_change:
			if (change_number)
				goto duplicate;
			if (arglex() != arglex_token_number)
				report_usage();
			change_number = arglex_value.alv_number;
			if (change_number < 1)
				fatal("change %ld out of range", change_number);
			break;

		case arglex_token_project:
			if (project_name)
				goto duplicate;
			if (arglex() != arglex_token_string)
				report_usage();
			project_name = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_file:
			if (infile)
			{
				duplicate:
				error("duplicate %s option", arglex_value.alv_string);
				report_usage();
			}
			if (arglex() != arglex_token_string)
			{
				fatal("the -File option requires a string argument");
			}
			trace(("accepting -File option\n"));
			infile = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_output:
			if (outfile)
				goto duplicate;
			if (arglex() != arglex_token_string)
			{
				fatal("the -Output option requires a string argument");
			}
			outfile = str_from_c(arglex_value.alv_string);
			break;

		case arglex_token_string:
		case arglex_token_number:
			s = str_from_c(arglex_value.alv_string);
			wl_append(&arg, s);
			str_free(s);
			break;
		}
		arglex();
	}
	if (infile)
	{
		trace(("prepending report file name to args\n"));
		wl_prepend(&arg, infile);
	}
	else if (arg.wl_nwords == 0)
	{
		error("no report name specified");
		report_usage();
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
	wl_free(&arg);
	trace((/*{*/"}\n"));
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

void
report()
{
	trace(("report()\n{\n"/*}*/));
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
	trace((/*{*/"}\n"));
}
