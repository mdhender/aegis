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
// MANIFEST: functions to manipulate repositorys
//
//
// Repository <server-side>
//
// A server need not implement Repository, but in order to interoperate
// with CVS 1.5 through 1.9 it must claim to implement it (in
// Valid-requests).  The client will not actually send the request.
//
// There was a Repository request which was like Directory except it
// only provided <server-side>, and the client side directory was assumed
// to be similarly named.
//
// Response expected: no.
// Root required: yes.
//

#include <request/repository.h>
#include <server.h>


static void
run(server_ty *sp, string_ty *server_side)
{
    if (server_root_required(sp, "Repository"))
	return;
    server_error(sp, "Repository: obsolete, not supported");
}


const request_ty request_repository =
{
    "Repository",
    run,
    0, // no reset
};
