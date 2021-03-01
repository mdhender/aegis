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
// MANIFEST: functions to manipulate unchangeds
//
//
// Unchanged <filename>
//
// Tell the server that <filename> has not been modified in the checked
// out directory.  The <filename> is a file within the most recent
// directory sent with Directory; it must not contain /.
//
// Response expected: no.
// Root required: yes.
//

#include <file_info.h>
#include <request/unchanged.h>
#include <os.h>
#include <server.h>


static void
run(server_ty *sp, string_ty *arg)
{
    directory_ty    *dp;
    string_ty       *server_side;
    file_info_ty    *fip;

    if (server_root_required(sp, "Unchanged"))
	return;
    if (server_directory_required(sp, "Unchanged"))
	return;

    //
    // Build the name of the file on the server side.
    //
    dp = sp->np->get_curdir();
    server_side = os_path_cat(dp->server_side, arg);

    //
    // Add the entry to the file info.
    //
    fip = server_file_info_find(sp, server_side, 1);
    fip->modified = 0;
}


const request_ty request_unchanged =
{
    "Unchanged",
    run,
    0, // no reset
};
