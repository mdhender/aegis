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

#include <common/ac/ctype.h>

#include <aecvsserver/request/unknown.h>
#include <aecvsserver/server.h>


request_unknown::~request_unknown()
{
}


request_unknown::request_unknown(string_ty *arg) :
    vname(str_copy(arg))
{
}


void
request_unknown::run_inner(server_ty *sp, string_ty *)
    const
{
    //
    // Complain about unknown requests.
    //
    server_error(sp, "request \"%s\" unknown", vname->str_text);
}


const char *
request_unknown::name()
    const
{
    return vname->str_text;
}


bool
request_unknown::reset()
    const
{
    //
    // The general rule is that requests with names which start
    // with an upper case letter accumulate input and parameters
    // for other requests.  The rest consume them.
    //
    return !isupper((unsigned char)vname->str_text[0]);
}
