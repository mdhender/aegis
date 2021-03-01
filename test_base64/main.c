/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate mains
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <error.h>
#include <input/base64.h>
#include <input/crlf.h>
#include <input/file.h>
#include <os.h>
#include <output/file.h>
#include <output/base64.h>
#include <progname.h>
#include <str.h>


enum
{
	arglex_token_input,
	arglex_token_output
};


static arglex_table_ty argtab[] =
{
	{ "-Input", arglex_token_input, },
	{ "-Output", arglex_token_output, },
	{ 0 } /* end marker */
};


static void usage _((void));

static void
usage()
{
	const char	*progname;

	progname = progname_get();
	fprintf(stderr, "Usage: %s [ -i | -o ][ <infile> [ <outfile> ]]\n", progname);
	exit(1);
}


static void test_input _((const char *, const char *));

static void
test_input(ifn, ofn)
	const char	*ifn;
	const char	*ofn;
{
	input_ty	*ifp;
	output_ty	*ofp;

	ifp = input_file_open(ifn);
	ofp = output_file_text_open(ofn);

	/*
	 * skip lines until we find a blank line
	 * as a crude way to pass over the rfc822  header
	 */
	for (;;)
	{
		string_ty	*s;

		s = input_one_line(ifp);
		if (!s)
			break;
		if (!s->str_length)
		{
			str_free(s);
			break;
		}
		str_free(s);
	}

	/*
	 * filter the rest
	 */
	ifp = input_base64(ifp);
	input_to_output(ifp, ofp);
	input_delete(ifp);
	output_delete(ofp);
}


static void test_output _((const char *, const char *));

static void
test_output(ifn, ofn)
	const char	*ifn;
	const char	*ofn;
{
	input_ty	*ifp;
	output_ty	*ofp;

	ifp = input_file_open(ifn);
	ifp = input_crlf(ifp, 1);
	ofp = output_file_text_open(ofn);
	output_fprintf(ofp, "Content-Type: application/x-aegis-test\n");
	output_fprintf(ofp, "Content-Transfer-Encoding: base64\n");
	output_fprintf(ofp, "\n");
	ofp = output_base64(ofp, 1);
	input_to_output(ifp, ofp);
	input_delete(ifp);
	output_delete(ofp);
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	const char	*ifn;
	const char	*ofn;
	void		(*func)_((const char *, const char *));

	arglex_init(argc, argv, argtab);
	str_initialize();
	arglex();
	os_become_init_mortal();

	ifn = 0;
	ofn = 0;
	func = 0;
	while (arglex_token != arglex_token_eoln)
	{
		switch (arglex_token)
		{
		default:
			usage();

		case arglex_token_stdio:
			if (!ifn)
				ifn = "";
			else if (!ofn)
				ofn = "";
			else
				usage();
			break;

		case arglex_token_string:
			if (!ifn)
				ifn = arglex_value.alv_string;
			else if (!ofn)
				ofn = arglex_value.alv_string;
			else
				usage();
			break;

		case arglex_token_input:
			if (func)
			{
				too_many:
				error_raw("too many test functions specified");
				usage();
			}
			func = test_input;
			break;

		case arglex_token_output:
			if (func)
				goto too_many;
			func = test_output;
			break;
		}
		arglex();
	}
	if (!func)
	{
		error_raw("no test function specified");
		usage();
	}

	os_become_orig();
	func(ifn, ofn);
	exit(0);
	return 0;
}
