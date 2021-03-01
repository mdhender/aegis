//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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
// Argument <text>
//
// Save argument for use in a subsequent command.  Arguments accumulate
// until an argument-using command is given, at which point they are
// forgotten.
//
// Response expected: no.
// Root required: yes.
//

#include <aecvsserver/request/argument.h>
#include <aecvsserver/server.h>


request_argument::~request_argument()
{
}


request_argument::request_argument()
{
}


void
request_argument::run_inner(server_ty *sp, string_ty *arg)
    const
{
    if (server_root_required(sp, "Argument"))
	return;
    server_argument(sp, arg);
}


const char *
request_argument::name()
    const
{
    return "Argument";
}


bool
request_argument::reset()
    const
{
    return false;
}
