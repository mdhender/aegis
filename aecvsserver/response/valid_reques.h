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

#ifndef AECVSSERVER_RESPONSE_VALID_REQUES_H
#define AECVSSERVER_RESPONSE_VALID_REQUES_H

#include <aecvsserver/response.h>

struct string_ty; // forward
struct string_list_ty; // forward

/**
  * The response_valid_requests class is used to represent the response
  * from the valid requests command.
  */
class response_valid_requests:
    public response
{
public:
    /**
      * The destructor.
      */
    virtual ~response_valid_requests();

    /**
      * The constructor.
      */
    response_valid_requests(string_list_ty *arg);

    // See base class for documentation.
    void write(output::pointer op);

    // See base class for documentation.
    response_code_ty code_get() const;

private:
    string_ty *message;

    /**
      * The default constructor.  Do not use.
      */
    response_valid_requests();

    /**
      * The copy constructor.  Do not use.
      */
    response_valid_requests(const response_valid_requests &);

    /**
      * The assignment operator.  Do not use.
      */
    response_valid_requests &operator=(const response_valid_requests &);
};

#endif // AECVSSERVER_RESPONSE_VALID_REQUES_H
