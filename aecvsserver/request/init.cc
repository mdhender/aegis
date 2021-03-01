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
// init <root-name>
//
// If it doesn't already exist, create a cvs repository <root-name>.
// Note that <root-name> is a local directory and *not* a fully qualified
// CVSROOT variable.
//
// Response expected: yes.
// Root required: no.
//

#include <aecvsserver/request/init.h>
#include <aecvsserver/server.h>


request_init::~request_init()
{
}


request_init::request_init()
{
}


void
request_init::run_inner(server_ty *sp, string_ty *)
    const
{
    server_error
    (
	sp,
	"init: not supported; use Aegis project managment commands instead"
    );
}


const char *
request_init::name()
    const
{
    return "init";
}


bool
request_init::reset()
    const
{
    return true;
}
