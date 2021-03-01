//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006, 2008 Peter Miller
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
#include <libaegis/help.h>
#include <libaegis/input/base64.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/file.h>
#include <libaegis/input/quoted_print.h>
#include <libaegis/input/uudecode.h>
#include <libaegis/os.h>
#include <libaegis/output/base64.h>
#include <libaegis/output/bzip2.h>
#include <libaegis/output/file.h>
#include <libaegis/output/quoted_print.h>
#include <libaegis/output/uuencode.h>


enum
{
    arglex_token_bzip,
    arglex_token_input,
    arglex_token_output,
    arglex_token_quoted_printable,
    arglex_token_unix_to_unix,
    arglex_token_header_not
};


static arglex_table_ty argtab[] =
{
    { "-BZip", arglex_token_bzip, },
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
skip_header(input &ifp)
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
test_input_bunzip(string_ty *ifn, string_ty *ofn)
{
    input ifp = input_file_open(ifn);
    output::pointer ofp = output_file::text_open(ofn);

    skip_header(ifp);

    //
    // filter the rest
    //
    ifp = new input_bunzip2(ifp);
    ofp << ifp;
}


static void
test_input_base64(string_ty *ifn, string_ty *ofn)
{
    input ifp = input_file_open(ifn);
    output::pointer ofp = output_file::text_open(ofn);

    skip_header(ifp);

    //
    // filter the rest
    //
    ifp = new input_base64(ifp);
    ofp << ifp;
}


static void
test_input_qp(string_ty *ifn, string_ty *ofn)
{
    input ifp = input_file_open(ifn);
    output::pointer ofp = output_file::text_open(ofn);

    skip_header(ifp);

    ifp = new input_quoted_printable(ifp);
    ofp << ifp;
}


static void
test_input_uu(string_ty *ifn, string_ty *ofn)
{
    input ifp = input_file_open(ifn);
    output::pointer ofp = output_file::text_open(ofn);

    skip_header(ifp);

    ifp = new input_uudecode(ifp);
    ofp << ifp;
}


static void
test_output_base64(string_ty *ifn, string_ty *ofn)
{
    input ifp = input_file_open(ifn);
    ifp = new input_crlf(ifp);
    output::pointer ofp = output_file::text_open(ofn);
    if (has_header)
    {
	ofp->fputs("Content-Type: application/x-aegis-test\n");
	ofp->fputs("Content-Transfer-Encoding: base64\n");
	ofp->fputs("\n");
    }
    ofp = output_base64::create(ofp);
    ofp << ifp;
}


static void
test_output_bzip(string_ty *ifn, string_ty *ofn)
{
    input ifp = input_file_open(ifn);
    ifp = new input_crlf(ifp);
    output::pointer ofp = output_file::text_open(ofn);
    if (has_header)
    {
	ofp->fputs("Content-Type: application/x-aegis-test\n");
	ofp->fputs("Content-Transfer-Encoding: 8bit\n");
	ofp->fputs("\n");
    }
    ofp = output_bzip2::create(ofp);
    ofp << ifp;
}


static void
test_output_qp(string_ty *ifn, string_ty *ofn)
{
    input ifp = input_file_open(ifn);
    ifp = new input_crlf(ifp);
    output::pointer ofp = output_file::text_open(ofn);
    if (has_header)
    {
	ofp->fputs("Content-Type: application/x-aegis-test\n");
	ofp->fputs("Content-Transfer-Encoding: quoted-printable\n");
	ofp->fputs("\n");
    }
    ofp = output_quoted_printable::create(ofp, false);
    ofp << ifp;
}


static void
test_output_uu(string_ty *ifn, string_ty *ofn)
{
    input ifp = input_file_open(ifn);
    ifp = new input_crlf(ifp);
    output::pointer ofp = output_file::text_open(ofn);
    if (has_header)
    {
	ofp->fputs("Content-Type: application/x-aegis-test\n");
	ofp->fputs("Content-Transfer-Encoding: uuencode\n");
	ofp->fputs("\n");
    }
    ofp = output_uuencode::create(ofp);
    ofp << ifp;
}


int
main(int argc, char **argv)
{
    void (*ifunc)(string_ty *, string_ty *);
    void (*ofunc)(string_ty *, string_ty *);
    void (*func)(string_ty *, string_ty *);

    ifunc = test_input_base64;
    ofunc = test_output_base64;
    arglex_init(argc, argv, argtab);
    arglex();
    os_become_init_mortal();

    string_ty *ifn = 0;
    string_ty *ofn = 0;
    func = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

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

        case arglex_token_bzip:
	    if (func)
		usage();
	    ifunc = test_input_bunzip;
	    ofunc = test_output_bzip;
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
