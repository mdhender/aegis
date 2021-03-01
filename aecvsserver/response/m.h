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
// MANIFEST: interface definition for aecvsserver/response/m.c
//

#ifndef AECVSSERVER_RESPONSE_M_H
#define AECVSSERVER_RESPONSE_M_H

#include <common/ac/stdarg.h>
#include <aecvsserver/response.h>

/**
  * The response_m class is used to represent an "M" response to a CVS
  * client.
  */
class response_m:
    public response
{
public:
    /**
      * The destructor.
      */
    virtual ~response_m();

    /**
      * The constructor.
      */
    response_m(string_ty *);

    // See base class for documentation.
    void write(output_ty *op);

    // See base class for documentation.
    response_code_ty code_get() const;

private:
    string_ty *message;

    /**
      * The default constructor.  Do not use.
      */
    response_m();

    /**
      * The copy constructor.  Do not use.
      */
    response_m(const response_m &);

    /**
      * The assignment operator.  Do not use.
      */
    response_m &operator=(const response_m &);
};

#endif // AECVSSERVER_RESPONSE_M_H
