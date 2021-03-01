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
// MANIFEST: interface definition for ae-cvs-server/request.c
//

#ifndef AE_CVS_SERVER_REQUEST_H
#define AE_CVS_SERVER_REQUEST_H

#include <str.h>

struct server_ty;

struct request_ty
{
    const char      *name;
    void            (*run)(struct server_ty *, string_ty *);

    //
    // The reset member is true of the argument accumulator needs to be
    // reset after the command has run.
    //
    int             reset;
};

/**
  * The request_find function is used to locate the function to call
  * for the named request.  Returns NULL for unknown requests.
  */
const request_ty *request_find(string_ty *name);

/**
  * The request_run function is used to run the server request handler.
  * It will return when the client closes the connection.
  */
void request_run(const request_ty *rp, struct server_ty *, string_ty *);

#endif // AE_CVS_SERVER_REQUEST_H
