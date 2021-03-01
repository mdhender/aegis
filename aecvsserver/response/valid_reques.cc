//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// //
// Valid-requests <request-list>
//
// Indicate what requests the server will accept.  The <request-list>
// is a space separated list of tokens.  If the server supports sending
// patches, it will include "update-patches" in this list.  The
// update-patches request does not actually do anything.
//

#include <common/ac/assert.h>

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
response_valid_requests::write(output::pointer np)
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


// vim: set ts=8 sw=4 et :
