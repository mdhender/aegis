//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
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

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <error.h>
#include <input/base64.h>
#include <input/crlf.h>
#include <input/file.h>
#include <input/quoted_print.h>
#include <input/uudecode.h>
#include <os.h>
#include <output/base64.h>
#include <output/file.h>
#include <output/quoted_print.h>
#include <output/uuencode.h>
#include <progname.h>
#include <quit.h>


enum
{
    arglex_token_input,
    arglex_token_output,
    arglex_token_quoted_printable,
    arglex_token_unix_to_unix,
    arglex_token_header_not
};


static arglex_table_ty argtab[] =
{
    { "-Quoted_Printable", arglex_token_quoted_printable, },
    { "-Input", arglex_token_input, },
    { "-Output", arglex_token_output, },
    { "-Unix_to_Unix_Encode", arglex_token_unix_to_unix, },
    { "-Unix_to_Unix_Decode", arglex_token_unix_to_unix, },
    { "-Unix_to_Unix", arglex_token_unix_to_unix, },
    { "-No_Header", arglex_token_header_not, },
    ARGLEX_END_MARKER
};


bool has_header = true;


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"Usage: %s [ -i | -o ][ <infile> [ <outfile> ]]\n",
	progname
    );
    quit(1);
}


static void
skip_header(input_ty *ifp)
{
    if (has_header)
    {
	//
	// skip lines until we find a blank line
	// as a crude way to pass over the rfc822  header
	//
	for (;;)
	{
	    nstring s;
	    if (!ifp->one_line(s))
		break;
	    if (s.empty())
		break;
	}
    }
}


static void
test_input_base64(string_ty *ifn, string_ty *ofn)
{
    input_ty *ifp = input_file_open(ifn);
    output_ty *ofp = output_file_text_open(ofn);

    skip_header(ifp);

    //
    // filter the rest
    //
    ifp = new input_base64(ifp, true);
    input_to_output(ifp, ofp);
    delete ifp;
    delete ofp;
}


static void
test_input_qp(string_ty *ifn, string_ty *ofn)
{
    input_ty *ifp = input_file_open(ifn);
    output_ty *ofp = output_file_text_open(ofn);

    skip_header(ifp);

    ifp = new input_quoted_printable(ifp, true);
    input_to_output(ifp, ofp);
    delete ifp;
    delete ofp;
}


static void
test_input_uu(string_ty *ifn, string_ty *ofn)
{
    input_ty *ifp = input_file_open(ifn);
    output_ty *ofp = output_file_text_open(ofn);

    skip_header(ifp);

    ifp = new input_uudecode(ifp, true);
    input_to_output(ifp, ofp);
    delete ifp;
    delete ofp;
}


static void
test_output_base64(string_ty *ifn, string_ty *ofn)
{
    input_ty *ifp = input_file_open(ifn);
    ifp = new input_crlf(ifp, true);
    output_ty *ofp = output_file_text_open(ofn);
    if (has_header)
    {
	ofp->fputs("Content-Type: application/x-aegis-test\n");
	ofp->fputs("Content-Transfer-Encoding: base64\n");
	ofp->fputs("\n");
    }
    ofp = new output_base64_ty(ofp, true);
    input_to_output(ifp, ofp);
    delete ifp;
    delete ofp;
}


static void
test_output_qp(string_ty *ifn, string_ty *ofn)
{
    input_ty *ifp = input_file_open(ifn);
    ifp = new input_crlf(ifp, true);
    output_ty *ofp = output_file_text_open(ofn);
    if (has_header)
    {
	ofp->fputs("Content-Type: application/x-aegis-test\n");
	ofp->fputs("Content-Transfer-Encoding: quoted-printable\n");
	ofp->fputs("\n");
    }
    ofp = new output_quoted_printable_ty(ofp, true, false);
    input_to_output(ifp, ofp);
    delete ifp;
    delete ofp;
}


static void
test_output_uu(string_ty *ifn, string_ty *ofn)
{
    input_ty *ifp = input_file_open(ifn);
    ifp = new input_crlf(ifp, true);
    output_ty *ofp = output_file_text_open(ofn);
    if (has_header)
    {
	ofp->fputs("Content-Type: application/x-aegis-test\n");
	ofp->fputs("Content-Transfer-Encoding: uuencode\n");
	ofp->fputs("\n");
    }
    ofp = new output_uuencode_ty(ofp, true);
    input_to_output(ifp, ofp);
    delete ifp;
    delete ofp;
}


int
main(int argc, char **argv)
{
    string_ty       *ifn;
    string_ty       *ofn;
    void            (*ifunc)(string_ty *, string_ty *);
    void            (*ofunc)(string_ty *, string_ty *);
    void            (*func)(string_ty *, string_ty *);

    ifunc = test_input_base64;
    ofunc = test_output_base64;
    arglex_init(argc, argv, argtab);
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

        case arglex_token_quoted_printable:
	    if (func)
		usage();
	    ifunc = test_input_qp;
	    ofunc = test_output_qp;
	    break;

        case arglex_token_unix_to_unix:
	    if (func)
		usage();
	    ifunc = test_input_uu;
	    ofunc = test_output_uu;
	    break;

        case arglex_token_stdio:
	    if (!ifn)
		ifn = str_from_c("");
	    else if (!ofn)
		ofn = str_from_c("");
	    else
		usage();
	    break;

        case arglex_token_string:
	    if (!ifn)
		ifn = str_from_c(arglex_value.alv_string);
	    else if (!ofn)
		ofn = str_from_c(arglex_value.alv_string);
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
	    func = ifunc;
	    break;

        case arglex_token_output:
	    if (func)
		goto too_many;
	    func = ofunc;
	    break;

	case arglex_token_header_not:
	    has_header = false;
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
    quit(0);
    return 0;
}
