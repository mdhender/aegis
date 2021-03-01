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
// The authentication is successful.  The client proceeds with the cvs
// protocol itself.
//

#include <libaegis/output.h>
#include <aecvsserver/response/love.h>


response_love::~response_love()
{
}


response_love::response_love()
{
}


void
response_love::write(output::pointer op)
{
    op->fputs("I LOVE YOU\n");
}


response_code_ty
response_love::code_get()
    const
{
    return response_code_love;
}


bool
response_love::flushable()
    const
{
    return true;
}
