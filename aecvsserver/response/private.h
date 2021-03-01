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
 * MANIFEST: interface definition for ae-cvs-server/response/private.c
 */

#ifndef AE_CVS_SERVER_RESPONSE_PRIVATE_H
#define AE_CVS_SERVER_RESPONSE_PRIVATE_H

#include <response.h>

struct output_ty; /* forward */

typedef struct response_method_ty response_method_ty;
struct response_method_ty
{
    int size;
    void (*destructor)(response_ty *);
    void (*write)(response_ty *, struct output_ty *);
    response_code_ty code;
    int flushable;
};

/**
  * The response_new function is used to allocate a new response instance.
  * Not to be used directly.  Shall only be called from derived classes.
  */
response_ty *response_new(const response_method_ty *);

#endif /* AE_CVS_SERVER_RESPONSE_PRIVATE_H */
