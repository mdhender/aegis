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
// MANIFEST: functions to manipulate arguments
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

#include <request/argument.h>
#include <server.h>


static void
run(server_ty *sp, string_ty *arg)
{
    if (server_root_required(sp, "Argument"))
	return;
    server_argument(sp, arg);
}


const request_ty request_argument =
{
    "Argument",
    run,
    0, // no reset
};