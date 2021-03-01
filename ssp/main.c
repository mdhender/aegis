/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995 Peter Miller;
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
 * MANIFEST: operating system entry point
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <error.h>
#include <option.h>
#include <ssp.h>
#include <str.h>


static void usage _((void));

static void
usage()
{
	char		*progname;

	progname = option_progname_get();
	fprintf(stderr, "usage: %s [ infile [ outfile ]]\n", progname);
	fprintf(stderr, "       %s -help\n", progname);
	exit(1);
}


int
main(argc, argv)
	int		argc;
	char		**argv;
{
	char		*infile;
	char		*outfile;

	infile = 0;
	outfile = 0;
	arglex_init(argc, argv, (arglex_table_ty *)0);
	str_initialize();
	arglex();
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			error
			(
				"misaplaced \"%s\" command line argument",
				arglex_value.alv_string
			);
			usage();

		case arglex_token_string:
			if (!infile)
				infile = arglex_value.alv_string;
			else if (!outfile)
				outfile = arglex_value.alv_string;
			else
			{
				error("too many files named");
				usage();
			}
			break;

		case arglex_token_stdio:
			if (!infile)
				infile = "";
			else if (!outfile)
				outfile = "";
			else
			{
				error("too many files named");
				usage();
			}
			break;
		}
		arglex();
	}
	if (infile && !*infile)
		infile = 0;
	if (outfile && !*outfile)
		outfile = 0;
	
	ssp(infile, outfile);

	exit(0);
	return 0;
}
