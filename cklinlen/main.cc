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
//
//
// NAME
//	cklinlen
//
// SYNOPSIS
//	cklinlen [ -pw ][ <filename>... ]
//
// DESCRIPTION
//      The cklinlen program is used to check the named files for
//      various problems relating to our source code repositiory:
//
//      It checks to see if the file is in DOS format (using CRLF line
//      terminations).  Files in the repository are required to be in
//      UNIX format (using LF termination).
//
//      It checks to see if the lines are too long.  Lines are required
//      to be at most 80 characters wide.  (Tabs are interpretted to be
//      every 8 characters.)
//
//      It checks to see that there are no unprintable characters in the
//      file.  All source files are required to be plain ASCII text.
//
//      It checks to see that there is no trailing white space on the
//      ends of lines.  These are a waste of space, but more importantly
//      humans can't see them and they occasionally introduce subtle
//      defects.
//
//      It checks to see that there is no trailing blank lines on the
//      ends of files.  These are a waste of space, but more importantly
//      humans often can't see them and they occasionally introduce
//      subtle defects.
//
//      Binary files produce a fatal error.  The repository shouldn't
//      have binary files.
//
//      C++ comments in C files produce an error.  Many C compilers
//      are able to cope with them, but many cannot.  Using the right
//      comments gives the human reader a clue as to what language they
//      are reading.
//
//      C comments in C++ files produce an error.  Using the right
//      comments gives the human reader a clue as to what language they
//      are reading.
//
// EXIT STATUS
//      The cklinlen program will exit with an exit status of one for
//      all errors.  It will only exit with a status of 0 (success) if
//      there are no errors.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/arglex.h>
#include <cklinlen/check.h>
#include <common/error.h>
#include <common/progname.h>


enum
{
    arglex_token_from
};

static arglex_table_ty argtab[] =
{
    { "-From", arglex_token_from, },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s <filename>...\n", prog);
    fprintf(stderr, "       %s -From <filename>\n", prog);
    exit(1);
}


static void
process(const char *filename)
{
    FILE *fp = stdin;
    if (filename)
    {
	fp = fopen(filename, "r");
	if (!fp)
	{
	    perror(filename);
	    exit(1);
	}
    }
    for (;;)
    {
	char buffer[4000];
	if (!fgets(buffer, sizeof(buffer), fp))
	    break;
	size_t len = strlen(buffer);
	if (len > 0 && buffer[len - 1] == '\n')
	    buffer[len - 1] = 0;
	check(buffer);
    }
    if (fp != stdin)
	fclose(fp);
}


int
main(int argc, char **argv)
{
    arglex_init(argc, argv, argtab);
    for (;;)
    {
	switch (arglex())
	{
	default:
	    error_raw
	    (
		"misplaced \"%s\" command line argument",
		arglex_value.alv_string
	    );
	    usage();
	    // NOTREACHED

	case arglex_token_string:
	    check(arglex_value.alv_string);
	    break;

	case arglex_token_from:
	    switch (arglex())
	    {
	    case arglex_token_string:
		process(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		process(0);
		break;

	    default:
		usage();
		// NOTREACHED
	    }
	    break;

	case arglex_token_eoln:
	    //
	    // Report success.
	    //
	    exit(0);
	    return 0;
	}
    }
}
