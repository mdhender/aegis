//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1997, 1999, 2002-2006, 2008 Peter Miller
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

#include <common/arglex.h>
#include <common/error.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <fmtgen/lex.h>
#include <fmtgen/parse.h>


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s [ <option>... ] <file.def> <file.c> <libaegis/file.h>\n",
	progname
    );
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
help(void)
{
    static const char *const text[] =
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
"	The %s program is Copyright (C) 1990, 1991, 1992, 1993,",
"",
"	The %s program comes with ABSOLUTELY NO WARRANTY; for",
"	details use the 'aegis -VERSion Warranty' command.  This",
"	is free software and you are welcome to redistribute it",
"	under certain conditions; for details use the 'aegis",
"	-VERSion Redistribution' command.",
"",
"AUTHOR",
"	Peter Miller   E-Mail: millerp@canb.auug.org.au",
"	/\\/\\*             WWW: http://www.canb.auug.org.au/~millerp/",
	    0
    };

    const char      *const *cpp;
    const char      *progname;

    trace(("help()\n{\n"));
    progname = progname_get();
    for (cpp = text; *cpp; ++cpp)
    {
	printf(*cpp, progname);
	printf("\n");
    }
    trace(("}\n"));
}


enum
{
    arglex_token_include_short,
    arglex_token_include_long
};

static arglex_table_ty argtab[] =
{
    { "-\\I*", arglex_token_include_short, },
    { "-Include", arglex_token_include_long, },
    ARGLEX_END_MARKER
};


int
main(int argc, char **argv)
{
    const char      *filename[3];
    size_t          j;

    arglex_init(argc, argv, argtab);
    for (j = 0; j < SIZEOF(filename); ++j)
	filename[j] = 0;
    if (arglex() == arglex_token_help)
    {
	if (arglex() != arglex_token_eoln)
    	    usage();
	help();
	quit(0);
    }

    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    error_raw
	    (
		"misplaced \"%s\" command line argument",
		arglex_value.alv_string
	    );
	    usage();

	case arglex_token_include_long:
	    if (arglex() != arglex_token_string)
		usage();
	    // fall through...

	case arglex_token_include_short:
	    lex_include_path(arglex_value.alv_string);
	    break;

	case arglex_token_string:
	    for (j = 0; j < SIZEOF(filename); ++j)
		if (!filename[j])
	    	    break;
	    if (j >= SIZEOF(filename))
		fatal_raw("too many file names specified");
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
    	    fatal_raw("too few file names specified");

    parse(filename[0], filename[1], filename[2]);
    quit(0);
    return 0;
}
