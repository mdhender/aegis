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
// MANIFEST: functions to manipulate kerbeross
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
