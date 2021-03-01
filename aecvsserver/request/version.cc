//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate versions
//
//
// version
//
// Request that the server transmit its version message.
//
// Response expected: yes.
// Root required: no.
//

#include <output.h>
#include <request/version.h>
#include <server.h>
#include <version_stmp.h>


static void
run(server_ty *sp, string_ty *arg)
{
    //
    // Does this need some kind of prefix?
    // They usually do.
    //
    sp->np->printf("%s\n", version_stamp());
    server_ok(sp);
}


const request_ty request_version =
{
    "version",
    run,
    1, // reset
};
