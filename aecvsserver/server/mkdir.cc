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
// MANIFEST: functions to manipulate mkdirs
//

#include <os.h>
#include <response/clear_sticky.h>
#include <response/clearstatdir.h>
#include <server.h>


void
server_mkdir(server_ty *sp, string_ty *client_side, string_ty *server_side)
{
    //
    // If the directory exists, do nothing.
    //
    if (net_directory_find_client_side(sp->np, client_side))
	return;

    //
    // Make sure the parent directory exists.
    //
    server_mkdir_above(sp, client_side, server_side);

    //
    // Have the client create the directory.
    //
    server_response_queue
    (
	sp,
	response_clear_sticky_new(client_side, server_side)
    );
    server_response_queue
    (
	sp,
	response_clear_static_directory(client_side, server_side)
    );

    //
    // Remember that the directory now exists.
    //
    net_directory_set(sp->np, client_side, server_side);
}


void
server_mkdir_above(server_ty *sp, string_ty *client_side,
    string_ty *server_side)
{
    string_ty       *cs2;
    string_ty       *ss2;

    //
    // Make sure the parent directory exists.
    //
    cs2 = os_dirname_relative(client_side);
    ss2 = os_dirname_relative(server_side);
    server_mkdir(sp, cs2, ss2);
    str_free(cs2);
    str_free(ss2);
}
