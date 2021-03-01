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
// MANIFEST: interface definition for ae-cvs-server/server/password.c
//

#ifndef AE_CVS_SERVER_SERVER_PASSWORD_H
#define AE_CVS_SERVER_SERVER_PASSWORD_H

#include <aecvsserver/server.h>

/**
  * The server_passowrd_new function is used to craete a new instance
  * of a pserver.
  */
server_ty *server_password_new(struct net_ty *);

#endif // AE_CVS_SERVER_SERVER_PASSWORD_H
