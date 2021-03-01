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

#include <aecvsserver/response/error.h>
#include <aecvsserver/server/kerberos.h>
#include <aecvsserver/server/private.h>


static void
run(server_ty *sp)
{
    server_error(sp, "kerberos server mode not supported");
}


static const server_method_ty vtbl =
{
    sizeof(server_ty),
    0, // destructor
    run,
    "kerberos",
};


server_ty *
server_kerberos_new(net_ty *np)
{
    return server_new(&vtbl, np);
}
