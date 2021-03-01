//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller
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
// MANIFEST: functions to manipulate valid_requess
//
//
// Valid-requests <request-list>
//
// Indicate what requests the server will accept.  The <request-list>
// is a space separated list of tokens.  If the server supports sending
// patches, it will include "update-patches" in this list.  The
// update-patches request does not actually do anything.
//

#include <common/error.h> // for assert
#include <libaegis/output.h>

#include <aecvsserver/response/valid_reques.h>


response_valid_requests::~response_valid_requests()
{
    assert(message);
    str_free(message);
    message = 0;
}


response_valid_requests::response_valid_requests(string_list_ty *arg) :
    message(arg->unsplit())
{
}


void
response_valid_requests::write(output_ty *np)
{
    assert(message);
    np->fprintf("Valid-requests %s\n", message->str_text);
}


response_code_ty
response_valid_requests::code_get()
    const
{
    return response_code_Valid_requests;
}
