/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate lists
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex3.h>
#include <help.h>
#include <input/file.h>
#include <input/gunzip.h>
#include <input/tar.h>
#include <list.h>
#include <os.h>
#include <output/file.h>
#include <progname.h>
#include <str.h>


static void
usage(void)
{
	char	*progname;

	progname = progname_get();
	fprintf(stderr, "Usage: %s --list [ <option>... ]\n", progname);
	fprintf(stderr, "       %s --help\n", progname);
	exit(1);
}


void
list(void)
{
    string_ty	    *ifn;
    input_ty	    *ifp;
    string_ty	    *ofn;
    output_ty	    *ofp;

    ifn = 0;
    ofn = 0;
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
	       	/* NOTREACHED */

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
	       	/* NOTREACHED */

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

    /*
     * read the input
     */
    os_become_orig();
    ifp = input_file_open(ifn);

    /*
     * It may be compressed.
     */
    ifp = input_gunzip(ifp);

    /*
     * Now treat it as a tar archive.
     */
    ifp = input_tar(ifp);

    /*
     * Write the file names to the output.
     */
    ofp = output_file_text_open(ofn);
    for (;;)
    {
	input_ty	*ip;
	string_ty	*archive_name;

	ip = input_tar_child(ifp, &archive_name);
	if (!ip)
	    break;
	output_put_str(ofp, archive_name);
	output_fputc(ofp, '\n');
	input_delete(ip);
    }
    output_delete(ofp);
    input_delete(ifp);
    os_become_undo();
}
