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
// MANIFEST: functions to manipulate privates
//

#include <error.h> // for assert
#include <mem.h>
#include <net.h>
#include <server/private.h>


server_ty *
server_new(const server_method_ty *vptr, net_ty *np)
{
    server_ty       *result;

    assert(vptr);
    assert((size_t)vptr->size >= sizeof(server_ty));
    result = (server_ty *)mem_alloc(vptr->size);
    result->vptr = vptr;
    result->np = np;
    return result;
}
