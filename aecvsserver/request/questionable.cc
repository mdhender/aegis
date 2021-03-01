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

#include <aecvsserver/file_info.h>
#include <aecvsserver/request/questionable.h>
#include <libaegis/os.h>
#include <aecvsserver/server.h>


request_questionable::~request_questionable()
{
}


request_questionable::request_questionable()
{
}


void
request_questionable::run_inner(server_ty *sp, string_ty *arg)
    const
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


const char *
request_questionable::name()
    const
{
    return "Questionable";
}


bool
request_questionable::reset()
    const
{
    return false;
}
