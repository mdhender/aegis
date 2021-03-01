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

#include <common/mem.h>

#include <aecvsserver/net.h>
#include <aecvsserver/server/private.h>


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


// vim: set ts=8 sw=4 et :
