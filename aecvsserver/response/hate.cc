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
// The authentication fails.  After sending this response, the server
// may close the connection.  It is up to the server to decide whether
// to give this response, which is generic, or a more specific response
// using 'E' and/or 'error'.
//

#include <libaegis/output.h>
#include <aecvsserver/response/hate.h>


response_hate::~response_hate()
{
}


response_hate::response_hate()
{
}


void
response_hate::write(output::pointer op)
{
    op->fputs("I HATE YOU\n");
}


response_code_ty
response_hate::code_get()
    const
{
    return response_code_hate;
}


bool
response_hate::flushable()
    const
{
    return true;
}
