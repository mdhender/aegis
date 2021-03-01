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

#include <aecvsserver/file_info.h>
#include <aecvsserver/request/is_modified.h>
#include <libaegis/os.h>
#include <aecvsserver/server.h>


request_is_modified::~request_is_modified()
{
}


request_is_modified::request_is_modified()
{
}


void
request_is_modified::run_inner(server_ty *sp, string_ty *arg)
    const
{
    string_ty       *server_side;
    directory_ty    *dp;
    file_info_ty    *fip;

    if (server_root_required(sp, "Is-modified"))
	return;
    if (server_directory_required(sp, "Is-modified"))
	return;

    dp = sp->np->get_curdir();
    server_side = os_path_cat(dp->server_side, arg);
    fip = server_file_info_find(sp, server_side, 1);
    fip->modified = 1;
    str_free(server_side);
}


const char *
request_is_modified::name()
    const
{
    return "Is-modified";
}


bool
request_is_modified::reset()
    const
{
    return false;
}
