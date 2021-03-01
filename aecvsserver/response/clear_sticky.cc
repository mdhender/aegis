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

#include <ac/string.h>

#include <output.h>
#include <response/clear_sticky.h>
#include <response/private.h>
#include <server.h>


typedef struct response_clear_sticky_ty response_clear_sticky_ty;
struct response_clear_sticky_ty
{
    response_ty     inherited;
    string_ty       *client_side;
    string_ty       *server_side;
};


static void
destructor(response_ty *rp)
{
    response_clear_sticky_ty *rcsp;

    rcsp = (response_clear_sticky_ty *)rp;
    str_free(rcsp->client_side);
    rcsp->client_side = 0;
    str_free(rcsp->server_side);
    rcsp->server_side = 0;
}


static void
write(response_ty *rp, output_ty *op)
{
    response_clear_sticky_ty *rcsp;
    const char      *dir;

    rcsp = (response_clear_sticky_ty *)rp;
    dir = rcsp->client_side->str_text;
    output_fprintf(op, "Clear-sticky %s/\n", dir);
    output_fprintf(op, ROOT_PATH "/%s/\n", dir);
}


static const response_method_ty vtbl =
{
    sizeof(response_clear_sticky_ty),
    destructor,
    write,
    response_code_Clear_sticky,
    0, // not flushable
};


response_ty *
response_clear_sticky_new(string_ty *client_side, string_ty *server_side)
{
    response_ty     *rp;
    response_clear_sticky_ty *rcsp;

    rp = response_new(&vtbl);
    rcsp = (response_clear_sticky_ty *)rp;
    rcsp->client_side = str_copy(client_side);
    rcsp->server_side = str_copy(server_side);
    return rp;
}
