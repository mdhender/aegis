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
response_clear_sticky::write(output::pointer op)
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
