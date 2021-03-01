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
// Set <variable>=<value>
//
// This request is used to set a user variable named <variable> to the
// given <value>.
//
// Response expected: no.
// Root required: no.
//

#include <aecvsserver/request/set.h>
#include <aecvsserver/server.h>


request_set::~request_set()
{
}


request_set::request_set()
{
}


void
request_set::run_inner(server_ty *, string_ty *)
    const
{
    //
    // It sure would help is cvsclient.texi specified us some variables
    // names and what they are used for.
    //
}


const char *
request_set::name()
    const
{
    return "Set";
}


bool
request_set::reset()
    const
{
    return false;
}
