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

#ifndef AECVSSERVER_RESPONSE_CLEAR_STICKY_H
#define AECVSSERVER_RESPONSE_CLEAR_STICKY_H

#include <aecvsserver/response.h>

/**
  * The response_clear_sticky class is used to represent
  */
class response_clear_sticky:
    public response
{
public:
    /**
      * The destructor.
      */
    virtual ~response_clear_sticky();

    /**
      * The default constructor.
      */
    response_clear_sticky(string_ty *client_side, string_ty *server_side);

    // See base class for documentation.
    void write(output::pointer op);

    // See base class for documentation.
    response_code_ty code_get() const;

private:
    string_ty *client_side;
    string_ty *server_side;

    /**
      * The default constructor.  Do not use.
      */
    response_clear_sticky();

    /**
      * The copy constructor.  Do not use.
      */
    response_clear_sticky(const response_clear_sticky &);

    /**
      * The assignment operator.  Do not use.
      */
    response_clear_sticky &operator=(const response_clear_sticky &);
};

#endif // AECVSSERVER_RESPONSE_CLEAR_STICKY_H
