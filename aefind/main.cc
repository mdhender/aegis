//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2004 Peter Miller;
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

#include <ac/stdlib.h>

#include <env.h>
#include <cmdline.h>
#include <language.h>
#include <os.h>
#include <progname.h>
#include <r250.h>
#include <str.h>
#include <sub.h>


int
main(int argc, char **argv)
{
    //
    // initialize everything
    //
    progname_set(argv[0]);
    r250_init();
    os_become_init_mortal();
    str_initialize();
    env_initialize();
    language_init();

    //
    // parse the command line
    //
    cmdline_grammar(argc, argv);

    //
    // If we get to gere, everything went OK.
    //
    exit(0);
    return 0;
}
