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
// MANIFEST: interface definition for ae-cvs-server/response.c
//

#ifndef AECVSSERVER_RESPONSE_H
#define AECVSSERVER_RESPONSE_H

#include <aecvsserver/net.h> // for response_code_ty

struct output_ty; // forward

/**
  * The response class is an abstract base class used to represent
  * a generic response to be sent to the client.
  */
class response
{
public:
    /**
      * The destructor.
      */
    virtual ~response();

    /**
      * The write method is used to write a response to the client.
      */
    virtual void write(output_ty *op) = 0;

    /**
      * The response_code_get is used to get the response code of the response.
      */
    virtual response_code_ty code_get() const = 0;

    /**
      * The flushable method is used to determine if a response should
      * be flushed or not.
      */
    virtual bool flushable() const;

protected:
    /**
      * The default constructor.
      */
    response();

private:

    /**
      * The copy constructor.  Do not use.
      */
    response(const response &);

    /**
      * The assignment operator  Do not use.
      */
    response &operator=(const response &);
};


/**
  * The output_mode_string function is used to write a file mode to an
  * ouput stream.
  *
  * @param op
  *     where to write the string
  * @param mode
  *     the mode bits to turn into a string.
  */
void output_mode_string(struct output_ty *op, int mode);

#endif // AECVSSERVER_RESPONSE_H
