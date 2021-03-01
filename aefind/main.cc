//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2004, 2005 Peter Miller;
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

#include <common/ac/stdlib.h>

#include <common/env.h>
#include <aefind/cmdline.h>
#include <common/language.h>
#include <libaegis/os.h>
#include <common/quit.h>
#include <common/progname.h>
#include <libaegis/sub.h>


int
main(int argc, char **argv)
{
    //
    // initialize everything
    //
    progname_set(argv[0]);
    os_become_init_mortal();
    env_initialize();
    language_init();

    //
    // parse the command line
    //
    cmdline_grammar(argc, argv);

    //
    // If we get to gere, everything went OK.
    //
    quit(0);
    return 0;
}
