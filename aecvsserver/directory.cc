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

#include <aecvsserver/directory.h>
#include <common/mem.h>


void
directory_constructor(directory_ty *dp, string_ty *client, string_ty *server)
{
    dp->client_side = str_copy(client);
    dp->server_side = str_copy(server);
}


directory_ty *
directory_new(string_ty *client, string_ty *server)
{
    directory_ty    *dp;

    dp = (directory_ty *)mem_alloc(sizeof(directory_ty));
    directory_constructor(dp, client, server);
    return dp;
}


void
directory_copy_constructor(directory_ty *dp, const directory_ty *from)
{
    dp->client_side = str_copy(from->client_side);
    dp->server_side = str_copy(from->server_side);
}


void
directory_destructor(directory_ty *dp)
{
    str_free(dp->client_side);
    str_free(dp->server_side);
    dp->client_side = 0;
    dp->server_side = 0;
}


void
directory_delete(directory_ty *dp)
{
    directory_destructor(dp);
    mem_free(dp);
}
