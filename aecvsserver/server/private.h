/*
 *	aegis - project change supervisor
 *	Copyright (C) 2004 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for ae-cvs-server/server/private.c
 */

#ifndef AE_CVS_SERVER_SERVER_PRIVATE_H
#define AE_CVS_SERVER_SERVER_PRIVATE_H

#include <net.h>
#include <server.h>

typedef struct server_method_ty server_method_ty;
struct server_method_ty
{
    int size;
    void (*destructor)(server_ty *);
    void (*run)(server_ty *);

    /*
     * Add new methods before this line.  That way, the compiler will
     * complain about initialiser types if you forget to add it to one
     * of the vtables.
     */
    const char *name;
};

/**
  * The server_new fucntion is used to allocate new un-constructed
  * instances of servers.  This should only be used by derived classes,
  * never directly.  The derived class is responsible for running the
  * constructor.
  */
server_ty *server_new(const server_method_ty *, net_ty *);

#endif /* AE_CVS_SERVER_SERVER_PRIVATE_H */
