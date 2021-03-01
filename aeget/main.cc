//
//	aegis - project change supervisor
//	Copyright (C) 2003-2005 Peter Miller;
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

#include <libaegis/arglex2.h>
#include <common/env.h>
#include <aeget/cgi.h>
#include <aeget/http.h>
#include <common/language.h>
#include <libaegis/os.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>


int
main(int argc, char **argv)
{
    //
    // Initialize everything thyat needs it.
    //
    os_become_init_mortal();
    arglex2_init(argc, argv);
    env_initialize();
    language_init();
    resource_limits_init();

    //
    // Process the request
    //
    cgi();

    //
    // Report success.
    //
    quit(0);
    return 0;
}
