/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993, 1994, 1995 Peter Miller;
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
 * MANIFEST: operating system start-up point
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <error.h>
#include <option.h>
#include <str.h>
#include <trace.h>
#include <txt2c.h>


static void usage _((void));

static void
usage()
{
	char	*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s [ <option>... ][ <infile> [ <outfile> ]]\n", progname);
	fprintf(stderr, "       %s -Help\n", progname);
	exit(1);
}


static void help _((void));

static void
help()
{
	static char *text[] =
	{
"NAME",
"	%s - tuen text files into lists of C strings",
"",
"SYNOPSIS",
"	%s [ <option>... ][ <infile> [ <outfile> ]]",
"	%s -Help",
"",
"DESCRIPTION",
"	The %s program is used to process text files and produce",
"	output contaings lists of C strings.",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Help",
"		Give this help message.",
"",
"	All options may be abbreviated; the abbreviation is",
"	documented as the upper case letters, all lower case",
"	letters and underscores (_) are optional.  You must use",
"	consecutive sequences of optional letters.",
"",
"	All options are case insensitive, you may type them in",
"	upper case or lower case or a combination of both, case",
"	is not important.",
"",
"	For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"	are all interpreted to mean the -Project option.  The",
"	argument \"-prj\" will not be understood, because",
"	consecutive optional characters were not supplied.",
"",
"	Options and other command line arguments may be mixed",
"	arbitrarily on the command line, after the function",
"	selectors.",
"",
"	The GNU long option names are understood.  Since all",
"	option names for aegis are long, this means ignoring the",
"	extra leading '-'.  The \"--option=value\" convention is",
"	also understood.",
"",
"EXIT STATUS",
"	The %s program will exit with a status of 1 on any",
"	error.	The %s program will only exit with a status",
"	of 0 if there are no errors.",
"",
"COPYRIGHT",
"	The %s program is Copyright (C) 1990, 1991, 1992, 1993,",
"	1994 Peter Miller.  All rights reserved.",
"",
"	The %s program comes with ABSOLUTELY NO WARRANTY; for",
"	details use the 'aegis -VERSion Warranty' command.  This",
"	is free software and you are welcome to redistribute it",
"	under certain conditions; for details use the 'aegis",
"	-VERSion Redistribution' command.",
"",
"AUTHOR",
"	Peter Miller   UUCP     uunet!munnari!agso.gov.au!pmiller",
"	/\\/\\*          Internet pmiller@agso.gov.au",
		0
	};

	char	**cpp;
	char	*progname;

	progname = option_progname_get();
	for (cpp = text; *cpp; ++cpp)
	{
		printf(*cpp, progname);
		printf("\n");
	}
}


enum
{
	arglex_token_include_short,
	arglex_token_include_long
};

static arglex_table_ty argtab[] =
{
	{ 0, (arglex_token_ty)0, }, /* end marker */
};


int main _((int, char **));

int
main(argc, argv)
	int	argc;
	char	**argv;
{
	char	*infile;
	char	*outfile;

	str_initialize();
	arglex_init(argc, argv, argtab);
	infile = 0;
	outfile = 0;
	if (arglex() == arglex_token_help)
	{
		if (arglex() != arglex_token_eoln)
			usage();
		help();
		exit(0);
	}

	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			error
			(
				"misplaced \"%s\" command line argument",
				arglex_value.alv_string
			);
			usage();

		case arglex_token_string:
			if (!infile)
				infile = arglex_value.alv_string;
			else if (!outfile)
				outfile = arglex_value.alv_string;
			else
				fatal("too many file names specified");
			break;

		case arglex_token_stdio:
			if (!infile)
				infile = "";
			else if (!outfile)
				outfile = "";
			else
				fatal("too many file names specified");
			break;

#ifdef DEBUG
		case arglex_token_trace:
			while (arglex() == arglex_token_string)
				trace_enable(arglex_value.alv_string);
			continue;
#endif
		}
		arglex();
	}
	if (infile && !*infile)
		infile = 0;
	if (outfile && !*outfile)
		outfile = 0;

	txt2c(infile, outfile);
	exit(0);
	return 0;
}
