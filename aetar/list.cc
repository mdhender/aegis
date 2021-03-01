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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <aetar/arglex3.h>
#include <libaegis/help.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <aetar/input/tar.h>
#include <aeannotate/list.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <common/progname.h>
#include <common/str.h>


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
    os_become_orig();
    input ifp = input_file_open(ifn);

    //
    // It may be compressed.
    //
    ifp = input_gunzip_open(ifp);
    ifp = input_bunzip2_open(ifp);

    //
    // Now treat it as a tar archive.
    //
    input_tar *itp = new input_tar(ifp);

    //
    // Write the file names to the output.
    //
    output::pointer ofp = output_file::text_open(ofn);
    for (;;)
    {
        bool executable = false;
	nstring archive_name;
	input ip = itp->child(archive_name, executable);
	if (!ip.is_open())
	    break;
	ofp->fputs(archive_name);
	ofp->fputc('\n');
#ifdef DEBUG
	ofp->flush();
#endif
    }
    ofp.reset();
    ifp.close();
    os_become_undo();
}
