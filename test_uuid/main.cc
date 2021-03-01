//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2007 Walter Franzini;
//	Copyright (C) 2006-2008 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/stdio.h>

#include <common/arglex.h>
#include <common/error.h> // for assert
#include <common/nstring.h>
#include <common/progname.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <common/uuidentifier/translate.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/output/file.h>
#include <libaegis/input.h>
#include <libaegis/input/file.h>

enum
{
    arglex_token_generate,
    arglex_token_check,
    arglex_token_uuid_translate
};

static arglex_table_ty argtab[] =
{
    { "-Check", arglex_token_check, },
    { "-GENerate", arglex_token_generate, },
    { "-Translate", arglex_token_uuid_translate, },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    const char	    *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "Usage: %s [ -GENerate | -CHEck | -Translate] [input [output]]\n",
        progname
    );
    exit(EXIT_FAILURE);
}


static int
generate_uuid(const nstring &, const nstring &ofn)
{
    trace_nstring(ofn);

    nstring uuid(universal_unique_identifier());
    assert(!uuid.empty());
    os_become_orig();
    output::pointer ofp = output_file::open(ofn);
    if (!ofp)
        fatal_raw("Unable to open %s from writing", ofn.c_str());
    ofp->fputs(uuid);
    ofp->end_of_line();
    os_become_undo();
    return 1;
}


static int
check_uuid(const nstring &ifn, const nstring &)
{
    os_become_orig();
    input ifp = input_file_open(ifn);
    nstring uuid;
    if (!ifp->one_line(uuid))
	fatal_raw("Unable to read uuid from %s", ifn.c_str());
    os_become_undo();
    return universal_unique_identifier_valid(uuid);
}

static int
translate_uuid(const nstring &ifn, const nstring &ofn)
{
    os_become_orig();
    input ifp = input_file_open(ifn);
    output::pointer ofp = output_file::open(ofn);
    nstring uuid;
    if (!ifp->one_line(uuid))
        fatal_raw("Unable to read uuid from %s", ifn.c_str());
    ofp->fputs(uuid_translate(uuid));
    ofp->end_of_line();
    ofp.reset();
    os_become_undo();
    return 1;
}


int
main(int argc, char **argv)
{
    int	(*func)(const nstring &, const nstring &);

    arglex_init(argc, argv, argtab);

    nstring ifn;
    nstring ofn;
    func = NULL;
    arglex();
    os_become_init_mortal();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	case arglex_token_stdio:
	    if (ifn.empty())
		ifn = nstring("");
	    else if (ofn.empty())
		ofn = nstring("");
            else
                usage();
	    break;

	case arglex_token_string:
	    if (ifn.empty())
		ifn = nstring(arglex_value.alv_string);
	    else if (ofn.empty())
		ofn = nstring(arglex_value.alv_string);
	    else
		usage();
	    break;

	case arglex_token_generate:
	    func = generate_uuid;
	    break;

	case arglex_token_check:
	    func = check_uuid;
	    break;

        case arglex_token_uuid_translate:
            func = translate_uuid;
            break;

	default:
            generic_argument(usage);
            break;
	}
	arglex();
    }
    if (!func)
	usage();

    int ret = func(ifn, ofn);
    exit(ret ? EXIT_SUCCESS : EXIT_FAILURE);
}
