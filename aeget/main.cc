//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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
//	along with this program,  If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdlib.h>

#include <common/env.h>
#include <common/language.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/arglex2.h>
#include <libaegis/http.h>
#include <libaegis/os.h>

#include <aeget/cgi.h>


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
