//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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
// MANIFEST: interface definition for aecvsserver/response/clearstatdir.c
//

#ifndef AECVSSERVER_RESPONSE_CLEARSTATDIR_H
#define AECVSSERVER_RESPONSE_CLEARSTATDIR_H

#include <aecvsserver/response.h>

/**
  * The response_clear_static_directory class is used to represent
  */
class response_clear_static_directory:
    public response
{
public:
    /**
      * The destructor.
      */
    virtual ~response_clear_static_directory();

    /**
      * The constructor.
      */
    response_clear_static_directory(string_ty *client_side,
	string_ty *server_side);

    // See base class for documentation.
    void write(output_ty *op);

    // See base class for documentation.
    response_code_ty code_get() const;

private:
    string_ty *client_side;
    string_ty *server_side;

    /**
      * The default constructor.  Do not use.
      */
    response_clear_static_directory();

    /**
      * The copy constructor.  Do not use.
      */
    response_clear_static_directory(const response_clear_static_directory &);

    /**
      * The assignment operator.  Do not use.
      */
    response_clear_static_directory &operator=(
	const response_clear_static_directory &);
};

#endif // AECVSSERVER_RESPONSE_CLEARSTATDIR_H
