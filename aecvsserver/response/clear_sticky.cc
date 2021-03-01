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
// MANIFEST: functions to manipulate clear_stickys
//
//
// Clear-sticky <pathname>
//
// Clear any sticky tag or date set by Set-sticky.
//
//
// Its is not clear whether this only applies to directories (they appear
// to need to end in /) or whether it can apply to files as well.
// The descriptions of the Set-sticky response and the Sticky request
// seems to imply that they are for directories only.
//
// Reverse Engineering Notes:
//	The directory is created on the client side if it does not exist.
//

#include <common/ac/string.h>

#include <libaegis/output.h>
#include <aecvsserver/response/clear_sticky.h>
#include <aecvsserver/server.h>


response_clear_sticky::~response_clear_sticky()
{
    str_free(client_side);
    client_side = 0;
    str_free(server_side);
    server_side = 0;
}


response_clear_sticky::response_clear_sticky(string_ty *arg1, string_ty *arg2) :
    client_side(str_copy(arg1)),
    server_side(str_copy(arg2))
{
}


void
response_clear_sticky::write(output_ty *op)
{
    const char *dir = client_side->str_text;
    op->fprintf("Clear-sticky %s/\n", dir);
    op->fprintf(ROOT_PATH "/%s/\n", dir);
}


response_code_ty
response_clear_sticky::code_get()
    const
{
    return response_code_Clear_sticky;
}
