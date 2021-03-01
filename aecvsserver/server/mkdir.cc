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

#include <libaegis/os.h>
#include <aecvsserver/response/clear_sticky.h>
#include <aecvsserver/response/clearstatdir.h>
#include <aecvsserver/server.h>


void
server_mkdir(server_ty *sp, string_ty *client_side, string_ty *server_side)
{
    //
    // If the directory exists, do nothing.
    //
    if (sp->np->directory_find_client_side(client_side))
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
	new response_clear_sticky(client_side, server_side)
    );
    server_response_queue
    (
	sp,
	new response_clear_static_directory(client_side, server_side)
    );

    //
    // Remember that the directory now exists.
    //
    sp->np->directory_set(client_side, server_side);
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
