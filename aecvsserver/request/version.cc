//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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
//
// version
//
// Request that the server transmit its version message.
//
// Response expected: yes.
// Root required: no.
//

#include <libaegis/output.h>
#include <aecvsserver/request/version.h>
#include <aecvsserver/server.h>
#include <common/version_stmp.h>


request_version::~request_version()
{
}


request_version::request_version()
{
}


void
request_version::run_inner(server_ty *sp, string_ty *)
    const
{
    //
    // Does this need some kind of prefix?
    // They usually do.
    //
    sp->np->printf("%s\n", version_stamp());
    server_ok(sp);
}


const char *
request_version::name()
    const
{
    return "version";
}


bool
request_version::reset()
    const
{
    return true;
}
