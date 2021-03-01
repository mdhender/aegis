//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate simples
//

#include <ac/ctype.h>

#include <error.h> // for assert
#include <request.h>
#include <response/error.h>
#include <server/simple.h>
#include <server/private.h>


static void
run(server_ty *sp)
{
    for (;;)
    {
	string_ty       *s;
	const char      *cp;
	string_ty       *request_name;
	const request_ty *rp;

	//
	// Get the next request.
	//
	s = server_getline(sp);
	if (!s)
	    break;

	//
	// Extract the request name (the first word).
	//
	cp = s->str_text;
	while (*cp && !isspace((unsigned char)*cp))
	    ++cp;
	request_name = str_n_from_c(s->str_text, cp - s->str_text);

	//
	// Locate the request function.
	//
	rp = request_find(request_name);
	assert(rp);
	if (rp)
	{
	    string_ty       *argument;

	    //
	    // Skip the request name to form the argument string.
	    //
	    // Note: only skip one white space character, not as many
	    // white space characters as you can find.
	    //
	    cp = s->str_text;
	    while (*cp && !isspace((unsigned char)*cp))
		++cp;
	    if (*cp && isspace((unsigned char)*cp))
		++cp;
	    argument = str_n_from_c(cp, s->str_text + s->str_length - cp);

	    //
	    // Perform the request.
	    //
	    request_run(rp, sp, argument);
	    str_free(argument);
	}
	str_free(s);
	str_free(request_name);
    }
}


static const server_method_ty vtbl =
{
    sizeof(server_ty),
    0, // destructor
    run,
    "simple",
};


server_ty *
server_simple_new(net_ty *np)
{
    return server_new(&vtbl, np);
}
