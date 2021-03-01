//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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
// MANIFEST: functions to manipulate clearstatdirs
//
//
// Clear-static-directory <pathname>
//
// This instructs the client to clear the Entries.Static flag.
// The <pathname> must end in a slash; its purpose is to specify a
// directory, not a file within a directory.
//
// Reverse Engineering Notes:
//	The directory is created on the client side if it does not exist.
//

#include <common/ac/string.h>

#include <libaegis/output.h>
#include <aecvsserver/response/clearstatdir.h>
#include <aecvsserver/server.h>


response_clear_static_directory::~response_clear_static_directory()
{
    str_free(client_side);
    client_side = 0;
    str_free(server_side);
    server_side = 0;
}


response_clear_static_directory::response_clear_static_directory(
	string_ty *arg1, string_ty *arg2) :
    client_side(str_copy(arg1)),
    server_side(str_copy(arg2))
{
}


void
response_clear_static_directory::write(output_ty *op)
{
    const char *dir = client_side->str_text;
    op->fprintf("Clear-static-directory %s/\n", dir);
    op->fprintf(ROOT_PATH "/%s/\n", dir);
}


response_code_ty
response_clear_static_directory::code_get()
    const
{
    return response_code_Clear_static_directory;
}
