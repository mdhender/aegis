//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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
#include <aemeasure/count.h>
#include <common/error.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>


enum
{
    arglex_token_language
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
    quit(1);
}


int
main(int argc, char **argv)
{
    const char	    *infile;
    const char	    *outfile;

    resource_limits_init();
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
    quit(0);
    return 0;
}
