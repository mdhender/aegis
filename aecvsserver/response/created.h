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
 * MANIFEST: interface definition for aecvsserver/response/created.c
 */

#ifndef AECVSSERVER_RESPONSE_CREATED_H
#define AECVSSERVER_RESPONSE_CREATED_H

#include <response.h>

struct input_ty; /* forward */

/**
  * The response_created_new fucntion is used to allo a new instance of
  * a Created response.
  *
  * @param client_side
  *     The name of the file being sent to the client, relative to the
  *     client's current directory.
  * @param server_side
  *     The name of the file, relative to the server root (so it includes
  *     the module name) being sent to the client.
  * @param source
  *	The input to be read to get the content of the file.  This will
  *	be closed once the request is sent to the client.
  * @param mode
  *	The file mode for the file.
  * @param
  *     The current version of the file.
  */
response_ty *response_created_new(string_ty *client_side,
    string_ty *server_side, struct input_ty *source, int mode,
    string_ty *version);

#endif /* AECVSSERVER_RESPONSE_CREATED_H */
