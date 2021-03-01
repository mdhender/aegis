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

#include <error.h> // for assert
#include <output.h>
#include <response/private.h>
#include <response/valid_reques.h>


typedef struct response_valid_requests_ty response_valid_requests_ty;
struct response_valid_requests_ty
{
    response_ty     inherited;
    string_ty       *message;
};


static void
destructor(response_ty *rp)
{
    response_valid_requests_ty *rvrp;

    assert(rp);
    rvrp = (response_valid_requests_ty *)rp;
    assert(rvrp->message);
    str_free(rvrp->message);
    rvrp->message = 0;
}


static void
write(response_ty *rp, output_ty *np)
{
    response_valid_requests_ty *rvrp;

    assert(rp);
    rvrp = (response_valid_requests_ty *)rp;
    assert(rvrp->message);
    output_fprintf(np, "Valid-requests %s\n", rvrp->message->str_text);
}


static const response_method_ty vtbl =
{
    sizeof(response_valid_requests_ty),
    destructor,
    write,
    response_code_Valid_requests,
    0, // not flushable
};


response_ty *
response_valid_requests_new(string_list_ty *slp)
{
    response_ty     *rp;
    response_valid_requests_ty *rvrp;

    rp = response_new(&vtbl);
    rvrp = (response_valid_requests_ty *)rp;
    rvrp->message = wl2str(slp, 0, slp->nstrings, " ");
    return rp;
}
