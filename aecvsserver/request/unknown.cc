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
// MANIFEST: functions to manipulate unknowns
//

#include <ac/ctype.h>

#include <mem.h>
#include <request/unknown.h>
#include <server.h>


static void run(server_ty *sp, string_ty *arg);


static request_ty unknown =
{
    0, // name
    run,
    0, // reset
};


static void
run(server_ty *sp, string_ty *arg)
{
    //
    // Complain about unknown requests.
    //
    server_error(sp, "request \"%s\" unknown", unknown.name);

    //
    // The general rule is that requests with names which start
    // with an upper case letter accumulate input and parameters
    // for other requests.  The rest consume them.
    //
    if (!isupper((unsigned char)unknown.name[0]))
	server_accumulator_reset(sp);
}


const request_ty *
request_unknown(const char *name)
{
    if (unknown.name)
	mem_free((void *)unknown.name);
    unknown.name = mem_copy_string(name);
    return &unknown;
}
