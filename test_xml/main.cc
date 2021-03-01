//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the main class
//

#include <ac/stdio.h>

#include <arglex.h>
#include <error.h>
#include <input/file.h>
#include <xmltextread/by_node.h>
#include <xml_node/dump.h>
#include <os.h>
#include <output/file.h>
#include <progname.h>
#include <quit.h>


static void
usage(void)
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s [ <infile> [ <outfile> ]]\n", progname);
    quit(1);
}


int
main(int argc, char **argv)
{
    arglex_init(argc, argv, 0);
    const char *ifn = 0;
    const char *ofn = 0;
    arglex();
    os_become_init_mortal();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    usage();

	case arglex_token_string:
	    if (!ifn)
		ifn = arglex_value.alv_string;
	    else if (!ofn)
		ofn = arglex_value.alv_string;
	    else
		fatal_raw("too many files");
	    break;

	case arglex_token_stdio:
	    if (!ifn)
		ifn = "";
	    else if (!ofn)
		ofn = "";
	    else
		fatal_raw("too many files");
	    break;
	}
	arglex();
    }
    os_become_orig();
    input_ty *ip = input_file_open(ifn && *ifn ? str_from_c(ifn) : 0);
    output_ty *op = output_file_text_open(ofn && *ofn ? str_from_c(ofn) : 0);
    xml_text_reader_by_node fubar(ip, false);
    xml_node_dump dumper(op);
    fubar.register_node_handler("revml", dumper);
    fubar.register_node_handler("revml/name", dumper);
    fubar.register_node_handler("revml/empty", dumper);
    fubar.parse();
    delete op;
    return 0;
}
