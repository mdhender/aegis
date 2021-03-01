//
// aegis - project change supervisor
// Copyright (C) 1999, 2001-2006, 2008, 2010-2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <common/error.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/filter/gzip.h>
#include <common/progname.h>


enum
{
    arglex_token_input,
    arglex_token_output
};


static arglex_table_ty argtab[] =
{
    { "-Input", arglex_token_input, },
    { "-Output", arglex_token_output, },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s -o [ <infile> [ <outfile> ]]\n", progname);
    exit(1);
}


static void
test_input(string_ty *ifn, string_ty *ofn)
{
    os_become_orig();
    input ifp = input_file_open(ifn);
    ifp = input_gunzip_open(ifp);
    output::pointer ofp = output_file::binary_open(ofn);
    ofp << ifp;
}


static void
test_output(string_ty *ifn, string_ty *ofn)
{
    os_become_orig();
    input ifp = input_file_open(ifn);
    ifp = input_crlf::create(ifp);
    output::pointer ofp = output_file::text_open(ofn);
    ofp = output_filter_gzip::create(ofp);
    ofp << ifp;
}


int
main(int argc, char **argv)
{
    void (*func)(string_ty *, string_ty *);

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
            usage();

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
    func(ifn, ofn);
    exit(0);
    return 0;
}


// vim: set ts=8 sw=4 et :
