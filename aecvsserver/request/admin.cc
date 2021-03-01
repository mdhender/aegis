//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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
// admin
//
// Actually do a cvs admin command.  This uses any previous Argument,
// Directory, Entry, or Modified requests, if they have been sent.
// The last Directory sent specifies the working directory at the time
// of the operation.
//
// The admin command can take Is-modified instead of
// Modified with no known change in behavior.
//
// Root required: yes.
// Response expected: yes.
//
//
// From cvs(1):
// admin [rcs-options] files...
//     This is the cvs interface to assorted administrative facilities,
//     similar to rcs(1).  Execute control functions on the source
//     repository.  (Changes repository directly; uses working directory
//     without changing it.)
//
//
// Reverse Engineering Notes:
//
// We can infer from the cvs(1) man page that the Argument requests
// preceeding the admin request will be the rcs-options and then the
// file names.
//
// You can expect Entry, Unchanged and Is-modified requests to preceed
// this request for each of the files named in the Argument requests
// (with the necessary Directory requests for context).
//

#include <aecvsserver/request/admin.h>
#include <aecvsserver/server.h>


request_admin::~request_admin()
{
}


request_admin::request_admin()
{
}


void
request_admin::run_inner(server_ty *sp, string_ty *)
    const
{
    if (server_root_required(sp, "admin"))
	return;
    server_m
    (
	sp,
	"This command has been ignored.\n"
	    "You must use Aegis commands for Aegis administration."
    );
    server_ok(sp);
}


const char *
request_admin::name()
    const
{
    return "admin";
}


bool
request_admin::reset()
    const
{
    return true;
}
