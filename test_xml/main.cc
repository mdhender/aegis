//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#include <common/arglex.h>
#include <common/error.h>
#include <libaegis/input/file.h>
#include <libaegis/xmltextread/by_node.h>
#include <test_xml/xml_node/dump.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <common/progname.h>
#include <common/quit.h>


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
    input ip = input_file_open(ifn && *ifn ? str_from_c(ifn) : 0);
    output::pointer op =
        output_file::text_open(ofn && *ofn ? str_from_c(ofn) : 0);
    xml_text_reader_by_node fubar(ip, false);
    xml_node_dump dumper(op);
    fubar.register_node_handler("revml", dumper);
    fubar.register_node_handler("revml/name", dumper);
    fubar.register_node_handler("revml/empty", dumper);
    fubar.parse();
    op.reset();
    return 0;
}
