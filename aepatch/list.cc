//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006, 2008 Peter Miller
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

#include <aepatch/arglex3.h>
#include <common/error.h>
#include <libaegis/help.h>
#include <aeannotate/list.h>
#include <libaegis/output/file.h>
#include <common/progname.h>
#include <aepatch/slurp.h>


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --list [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


void
list(void)
{
    string_ty *ifn = 0;
    string_ty *ofn = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_file:
	    if (ifn)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, usage);
		// NOTREACHED

	    case arglex_token_stdio:
		ifn = str_from_c("");
		break;

	    case arglex_token_string:
		ifn = str_from_c(arglex_value.alv_string);
		break;
	    }
	    break;

	case arglex_token_output:
	    if (ofn)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_output, usage);
		// NOTREACHED

	    case arglex_token_stdio:
		ofn = str_from_c("");
		break;

	    case arglex_token_string:
		ofn = str_from_c(arglex_value.alv_string);
		break;
	    }
	    break;
	}
	arglex();
    }

    //
    // read the input
    //
    patch_list_ty *plp = patch_slurp(ifn);
    assert(plp);

    //
    // Write the file names to the output.
    //
    output::pointer ofp = output_file::text_open(ofn);
    for (size_t j = 0; j < plp->length; ++j)
    {
	assert(plp->item[j]);
	assert(plp->item[j]->name.nstrings);
	assert(plp->item[j]->name.string[0]);
	ofp->fputs(plp->item[j]->name.string[0]);
	ofp->fputc('\n');
    }
}
