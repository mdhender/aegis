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
// E <text>
//
// A one-line message for the user, on stderr.  Note that the format
// of <text> is not designed for machine parsing.  Although sometimes
// scripts and clients will have little choice, the exact text which
// is output is subject to vary at the discretion of the server.
//

#include <common/ac/string.h>

#include <libaegis/output.h>
#include <aecvsserver/response/e.h>


response_e::~response_e()
{
    str_free(message);
    message = 0;
}


response_e::response_e(string_ty *arg) :
    message(arg)
{
}


void
response_e::write(output::pointer np)
{
    const char *cp = message->str_text;
    for (;;)
    {
	const char *ep = strchr(cp, '\n');
	if (!ep)
	    break;
	np->fprintf("E %.*s\n", (int)(ep - cp), cp);
	cp = ep + 1;
    }
    if (*cp)
	np->fprintf("E %s\n", cp);
}


response_code_ty
response_e::code_get()
    const
{
    return response_code_E;
}
