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
// Unchanged <filename>
//
// Tell the server that <filename> has not been modified in the checked
// out directory.  The <filename> is a file within the most recent
// directory sent with Directory; it must not contain /.
//
// Response expected: no.
// Root required: yes.
//

#include <aecvsserver/file_info.h>
#include <aecvsserver/request/unchanged.h>
#include <libaegis/os.h>
#include <aecvsserver/server.h>


request_unchanged::~request_unchanged()
{
}


request_unchanged::request_unchanged()
{
}


void
request_unchanged::run_inner(server_ty *sp, string_ty *arg)
    const
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


const char *
request_unchanged::name()
    const
{
    return "Unchanged";
}


bool
request_unchanged::reset()
    const
{
    return false;
}
