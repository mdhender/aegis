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
// Root <pathname>
//
// Tell the server which CVSROOT to use.  Note that <pathname> is a local
// directory and *not* a fully qualified CVSROOT variable.  <pathname>
// must already exist; if creating a new root, use the "init" request,
// not Root.  <pathname> does not include the hostname of the server,
// how to access the server, etc.; by the time the CVS protocol is in use,
// connection, authentication, etc., are already taken care of.
//
// The Root request must be sent only once, and it must be sent before
// any requests other than Valid-responses, valid-requests, UseUnchanged,
// Set, Global_option, init, noop, or version.
//
// Response expected: no.
// Root required: no.
//

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <aecvsserver/response/error.h>
#include <aecvsserver/request/root.h>
#include <aecvsserver/server.h>


request_root::~request_root()
{
}


request_root::request_root()
{
}


void
request_root::run_inner(server_ty *sp, string_ty *arg)
    const
{
    assert(sp);
    assert(sp->np);
    if (sp->np->get_is_rooted())
    {
        //
        // The Root request must be sent only once, and it must be sent before
        //
        server_error(sp, "too many Root requests");
        return;
    }
    if (0 != strcmp(arg->str_text, ROOT_PATH))
    {
        //
        // cvsclient.texi:
        // "The client must send the identical string for cvs root both
        // [at authentication time] and later in the Root request of
        // the cvs protocol itself.  Servers are encouraged to enforce
        // this restriction."
        //
        // We only allow one Root specification, exactly ROOT_PATH,
        // and we check it in both places.
        //
        server_error(sp, "%s: no such repository", arg->str_text);
        return;
    }

    //
    // Remember that we have seen the Root request.
    //
    sp->np->set_is_rooted();
}


const char *
request_root::name()
    const
{
    return "Root";
}


bool
request_root::reset()
    const
{
    return false;
}


// vim: set ts=8 sw=4 et :
