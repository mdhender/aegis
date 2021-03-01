//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini;
//	Copyright (C) 2006, 2007 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/stdio.h>

#include <common/arglex.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/output/file.h>
#include <libaegis/input.h>
#include <libaegis/input/file.h>
#include <common/progname.h>
#include <common/uuidentifier.h>

enum
{
    arglex_token_generate,
    arglex_token_check
};

static arglex_table_ty argtab[] =
{
    { "-GENerate", arglex_token_generate, },
    { "-Check", arglex_token_check, },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    const char	    *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s [ -GENerate | -CHEck ]\n", progname);
    exit(EXIT_FAILURE);
}


static int
generate_uuid(string_ty *, string_ty *ofn)
{
    output_ty	    *ofp;
    string_ty	    *uuid;

    uuid = universal_unique_identifier();
    assert(uuid);
    os_become_orig();
    ofp = output_file_text_open(ofn);
    if (!ofp)
	fatal_raw("Unable to open %s from writing", ofn);
    ofp->fputs(uuid);
    ofp->end_of_line();
    delete ofp;
    ofp = 0;
    os_become_undo();
    return 1;
}


static int
check_uuid(string_ty *ifn, string_ty *)
{
    os_become_orig();
    input ifp = input_file_open(ifn);
    nstring uuid;
    if (!ifp->one_line(uuid))
	fatal_raw("Unable to read uuid from %s", ifn);
    ifp.close();
    os_become_undo();
    return universal_unique_identifier_valid(uuid);
}


int
main(int argc, char **argv)
{
    int		    ret;
    string_ty	    *ifn;
    string_ty	    *ofn;
    int		    (*func)(string_ty *, string_ty *);

    arglex_init(argc, argv, argtab);
    arglex();
    os_become_init_mortal();
    ifn = NULL;
    ofn = NULL;
    func = NULL;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	case arglex_token_stdio:
	    if (!ifn)
		ifn = str_from_c("");
	    else if (!ofn)
		ofn = str_from_c("");
	    break;

	case arglex_token_string:
	    if (!ifn)
		ifn = str_from_c(arglex_value.alv_string);
	    else if (!ofn)
		ofn = str_from_c(arglex_value.alv_string);
	    else
		usage();
	    break;

	case arglex_token_generate:
	    func = generate_uuid;
	    break;

	case arglex_token_check:
	    func = check_uuid;
	    break;

	default:
	    usage();
	}
	arglex();
    }
    if (!func)
	usage();

    ret = func(ifn, ofn);
    exit(ret ? EXIT_SUCCESS : EXIT_FAILURE);
}
