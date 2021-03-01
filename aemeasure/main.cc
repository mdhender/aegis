//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: operating system start point, and command line argument parsing
//

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <count.h>
#include <error.h>
#include <progname.h>


enum
{
    arglex_token_language,
};

static arglex_table_ty argtab[] =
{
    { "-LANGuage", arglex_token_language, },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    fprintf
    (
	stderr,
	"Usage: %s [ <option>... ][ <infile> [ <outfile> ]]\n",
	progname_get()
    );
    exit(1);
}


int
main(int argc, char **argv)
{
    const char	    *infile;
    const char	    *outfile;

    arglex_init(argc, argv, argtab);
    arglex();

    infile = 0;
    outfile = 0;
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

	case arglex_token_language:
	    if (arglex() != arglex_token_string)
		fatal_raw("must give language name");
	    set_language(arglex_value.alv_string);
	    break;

	case arglex_token_string:
	    if (!infile)
		infile = arglex_value.alv_string;
	    else if (!outfile)
		outfile = arglex_value.alv_string;
	    else
	    {
		too_many_files:
		error_raw("too many file names");
	    }
	    break;

	case arglex_token_stdio:
	    if (!infile)
		infile = "";
	    else if (!outfile)
		outfile = "";
	    else
		goto too_many_files;
	    break;
	}
	arglex();
    }
    if (infile && !*infile)
	infile = 0;
    if (outfile && !*outfile)
	outfile = 0;

    //
    // Read the file and count the lines.
    //
    count(infile, outfile);

    //
    // Report success.
    //
    exit(0);
    return 0;
}
