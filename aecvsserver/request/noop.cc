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
// noop
//
// This request is a null command in the sense that it doesn't do
// anything, but merely (as with any other requests expecting a response)
// sends back any responses pertaining to pending errors, pending Notified
// responses, etc.
//
// Response expected: yes.
// Root required: no.
//

#include <aecvsserver/request/noop.h>
#include <aecvsserver/server.h>


request_noop::~request_noop()
{
}


request_noop::request_noop()
{
}


void
request_noop::run_inner(server_ty *sp, string_ty *)
    const
{
    server_ok(sp);
}


const char *
request_noop::name()
    const
{
    return "noop";
}


bool
request_noop::reset()
    const
{
    return true;
}
