//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#include <aecvsserver/request/valid_reques.h>
#include <aecvsserver/response/valid_reques.h>


request_valid_requests::~request_valid_requests()
{
}


request_valid_requests::request_valid_requests()
{
}


//
// Ask the server to send back a "Valid-requests" response,
// listing all the requests this server understands.
//
// Response expected: yes.
// Root required: no.
//
//
void
request_valid_requests::run_inner(server_ty *sp, string_ty *)
    const
{
    string_list_ty wl;
    get_list(wl);
    server_response_queue(sp, new response_valid_requests(&wl));
    server_ok(sp);
}


const char *
request_valid_requests::name()
    const
{
    return "valid-requests";
}


bool
request_valid_requests::reset()
    const
{
    return true;
}
