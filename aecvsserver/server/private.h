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

#ifndef AE_CVS_SERVER_SERVER_PRIVATE_H
#define AE_CVS_SERVER_SERVER_PRIVATE_H

#include <aecvsserver/net.h>
#include <aecvsserver/server.h>

struct server_method_ty
{
    int size;
    void (*destructor)(server_ty *);
    void (*run)(server_ty *);

    //
    // Add new methods before this line.  That way, the compiler will
    // complain about initialiser types if you forget to add it to one
    // of the vtables.
    //
    const char *name;
};

/**
  * The server_new fucntion is used to allocate new un-constructed
  * instances of servers.  This should only be used by derived classes,
  * never directly.  The derived class is responsible for running the
  * constructor.
  */
server_ty *server_new(const server_method_ty *, net_ty *);

#endif // AE_CVS_SERVER_SERVER_PRIVATE_H
