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
// MANIFEST: functions to manipulate mains
//
//
//
// NAME
//	file_check
//
// SYNOPSIS
//	coding_standar [ -pw ][ <filename>... ]
//	coding_standar -v
//
// DESCRIPTION
//	The coding_standar program is used to check the named files for
//	various problems relating to our source code repositiory:
//
//	It checks to see if the file is in DOS format (using CRLF line
//	terminations).	Files in the repository are required to be in
//	UNIX format (using LF termination).
//
//	It checks to see if the lines are too long.  Lines are required
//	to be at most 80 chyaracatres wide.  (Tabs are interpretted to
//	be every 8 characters.)
//
//	It checks to see that there are no unprintable characters in
//	the file.  All source files are required to be plain ASCII text.
//
//	It checks to see that there is no trailing white space on the ends
//	of lines.  These are a waste of space, but more importantly humans
//	can't see them and they occasionally introduce subtle defects.
//
//	It checks to see that there is no trailing blank lines on the
//	ends of files.	These are a waste of space, but more importantly
//	humans often can't see them and they occasionally introduce
//	subtle defects.
//
//	Binary files produce a warning.  The repository shouldn't have
//	binary files, but some were inherited from CVS.  This may change
//	to being a fatal error at some time in the future.
//
//	C++ commands in C files produce a warning.  Many C compilers
//	are able to cope with them, but many cannot.
//
// OPTIONS
//	The -p option turns on warnings when C comments appear in C++ files.
//	The default is to silently ignore such comments.
//
//	The -w option may be used to generate warnings instead of fatal
//	errors.  The default is to produce fatal errors (exit status 1).
//
//	The -v option may be used to print the version of the coding_standar
//	program and exit.
//
// EXIT STATUS
//	The coding_standar program will exit with an exit status of one
//	for all errors.  It will only exit with a status of 0 (success)
//	if there are no errors.
//

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <check.h>
#include <error.h>
#include <progname.h>
#include <str.h>


enum
{
    arglex_token_warnings
};

static arglex_table_ty argtab[] =
{
    { "-Warnings", arglex_token_warnings, },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    fprintf(stderr, "Usage: %s [ -w ] <filename>...\n", progname_get());
    exit(1);
}


int
main(int argc, char **argv)
{
    arglex_init(argc, argv, argtab);
    str_initialize();
    while (arglex() != arglex_token_eoln)
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

	case arglex_token_warnings:
	    ++warning;
	    break;

	case arglex_token_string:
	    check(arglex_value.alv_string);
	    break;
	}
    }

    //
    // Report success.
    //
    exit(0);
    return 0;
}
