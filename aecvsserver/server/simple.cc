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
//

#include <common/ac/assert.h>
#include <common/ac/ctype.h>

#include <aecvsserver/request.h>
#include <aecvsserver/response/error.h>
#include <aecvsserver/server/simple.h>
#include <aecvsserver/server/private.h>


static void
run(server_ty *sp)
{
    for (;;)
    {
        const char      *cp;
        string_ty       *request_name;
        const request *rp;

        //
        // Get the next request.
        //
        nstring s;
        if (!server_getline(sp, s))
            break;

        //
        // Extract the request name (the first word).
        //
        cp = s.c_str();
        while (*cp && !isspace((unsigned char)*cp))
            ++cp;
        request_name = str_n_from_c(s.c_str(), cp - s.c_str());

        //
        // Locate the request function.
        //
        rp = request::find(request_name);
        assert(rp);
        if (rp)
        {
            string_ty       *argument;

            //
            // Skip the request name to form the argument string.
            //
            // Note: only skip one white space character, not as many
            // white space characters as you can find.
            //
            cp = s.c_str();
            while (*cp && !isspace((unsigned char)*cp))
                ++cp;
            if (*cp && isspace((unsigned char)*cp))
                ++cp;
            argument = str_n_from_c(cp, s.c_str() + s.size() - cp);

            //
            // Perform the request.
            //
            rp->run(sp, argument);
            str_free(argument);
        }
        str_free(request_name);
    }
}


static const server_method_ty vtbl =
{
    sizeof(server_ty),
    0, // destructor
    run,
    "simple",
};


server_ty *
server_simple_new(net_ty *np)
{
    return server_new(&vtbl, np);
}


// vim: set ts=8 sw=4 et :
