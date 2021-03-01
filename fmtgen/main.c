/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
#include <stdlib.h>

#include <arglex.h>
#include <error.h>
#include <id.h>
#include <lex.h>
#include <option.h>
#include <parse.h>
#include <str.h>
#include <trace.h>


static void usage _((void));

static void
usage()
{
	char	*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s [ <option>... ] <file.def> <file.c> <file.h>\n", progname);
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
"	%s - file format read/write generator",
"",
"SYNOPSIS",
"	%s [ <option>... ] file.def file.c file.h",
"	%s -Help",
"",
"DESCRIPTION",
"	The %s program is used to process the .def files",
"	describing a file format into the .c and .h files which",
"	implement that format (with the help of gram.y and",
"	indent.c).",
"",
"OPTIONS",
"	The following options are understood:",
"",
"	-Help",
"		Give this help message.",
"",
"	-I<path>",
"		Specify and include search path.",
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
"	The %s program is Copyright (C) 1990, 1991, 1992, 1993 Peter Miller.",
"	All rights reserved.",
"",
"	The %s program comes with ABSOLUTELY NO WARRANTY; for",
"	details use the 'aegis -VERSion Warranty' command.  This",
"	is free software and you are welcome to redistribute it",
"	under certain conditions; for details use the 'aegis",
"	-VERSion Redistribution' command.",
"",
"AUTHOR",
"	Peter Miller   UUCP     uunet!munnari!bmr.gov.au!pmiller",
"	/\\/\\*          Internet pmiller@bmr.gov.au",
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
	{ "-\\I*",	(arglex_token_ty)arglex_token_include_short,	},
	{ "-Include",	(arglex_token_ty)arglex_token_include_long,	},
	{ 0, (arglex_token_ty)0, }, /* end marker */
};


int main _((int, char **));

int
main(argc, argv)
	int	argc;
	char	**argv;
{
	char	*filename[3];
	int	j;

	str_initialize();
	arglex_init(argc, argv, argtab);
	id_initialize();
	for (j = 0; j < SIZEOF(filename); ++j)
		filename[j] = 0;
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

		case arglex_token_include_long:
			if (arglex() != arglex_token_string)
				usage();
			/* fall through... */

		case arglex_token_include_short:
			lex_include_path(arglex_value.alv_string);
			break;

		case arglex_token_string:
			for (j = 0; j < SIZEOF(filename); ++j)
				if (!filename[j])
					break;
			if (j >= SIZEOF(filename))
				fatal("too many file names specified");
			filename[j] = arglex_value.alv_string;
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
	for (j = 0; j < SIZEOF(filename); ++j)
		if (!filename[j])
			fatal("too few file names specified");

	parse(filename[0], filename[1], filename[2]);
	exit(0);
	return 0;
}
