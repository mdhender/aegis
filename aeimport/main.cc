//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#include <aeimport/arglex3.h>
#include <common/env.h>
#include <aeimport/import.h>
#include <common/language.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/undo.h>


//
// NAME
//	import
//
// SYNOPSIS
//	void import(void);
//
// DESCRIPTION
//	The import function is used to
//	dispatch the `aeimport' command to the relevant functionality.
//	Where it goes depends on the command line.
//

int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init();
    arglex3_init(argc, argv);
    env_initialize();
    language_init();
    quit_register(log_quitter);
    quit_register(undo_quitter);
    os_interrupt_register();
    import();
    quit(0);
    return 0;
}
