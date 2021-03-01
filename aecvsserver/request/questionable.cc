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
// MANIFEST: functions to manipulate questionables
//
//
// Questionable <filename>
//
// Tell the server to check whether <filename> should be ignored, and
// if not, next time the server sends responses, send (in a M response)
// ? followed by the directory and filename.  The <filename> must not
// contain /; it needs to be a file in the directory named by the most
// recent @code{Directory} request.
//
// Response expected: no.
// Root required: yes.
// Directory required: yes.
//

#include <file_info.h>
#include <request/questionable.h>
#include <os.h>
#include <server.h>


static void
run(server_ty *sp, string_ty *arg)
{
    directory_ty    *dp;
    string_ty       *server_side;
    file_info_ty    *fip;

    if (server_root_required(sp, "Questionable"))
	return;
    if (server_directory_required(sp, "Questionable"))
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
    fip->questionable = 1;
}


const request_ty request_questionable =
{
    "Questionable",
    run,
    0, // no reset
};
