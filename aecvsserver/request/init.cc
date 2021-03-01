//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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
// MANIFEST: functions to manipulate inits
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
request_init::run_inner(server_ty *sp, string_ty *arg)
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
