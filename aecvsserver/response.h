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
// MANIFEST: interface definition for ae-cvs-server/response.c
//

#ifndef AE_CVS_SERVER_RESPONSE_H
#define AE_CVS_SERVER_RESPONSE_H

#include <net.h> // for response_code_ty

struct output_ty; // forward

/**
  * The response_ty class is an abstract base class used to represent
  * a generic response to be sent to the client.
  */
struct response_ty
{
    const struct response_method_ty *vptr;
};

/**
  * The response_delete function is used to release the respources held
  * by a response once you are done with it.
  */
void response_delete(response_ty *rp);

/**
  * The response_ty::write method is used to write a response to the client.
  */
void response_write(response_ty *rp, struct output_ty *);

/**
  * The response_code_get is used to get the response code of the response.
  */
response_code_ty response_code_get(response_ty *);

/**
  * The response flushable function is used to determine if a response
  * should be flushed or not.
  */
int response_flushable(response_ty *);

/**
  * The output_mode_string function is used to write a file mode to an
  * ouput stream.
  */
void output_mode_string(struct output_ty *op, int mode);

#endif // AE_CVS_SERVER_RESPONSE_H
