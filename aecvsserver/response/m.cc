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
// M <text>
//
// A one-line message for the user.  Note that the format of <text>
// is not designed for machine parsing.  Although sometimes scripts
// and clients will have little choice, the exact text which is output
// is subject to vary at the discretion of the server and the example
// output given in this document is just that, example output.
//


#include <common/ac/string.h>

#include <libaegis/output.h>
#include <aecvsserver/response/m.h>


response_m::~response_m()
{
    str_free(message);
    message = 0;
}


response_m::response_m(string_ty *arg) :
    message(arg)
{
}


void
response_m::write(output::pointer op)
{
    const char *cp = message->str_text;
    for (;;)
    {
	const char *ep = strchr(cp, '\n');
	if (!ep)
	    break;
	op->fprintf("M %.*s\n", (int)(ep - cp), cp);
	cp = ep + 1;
    }
    if (*cp)
	op->fprintf("M %s\n", cp);
}


response_code_ty
response_m::code_get()
    const
{
    return response_code_M;
}
