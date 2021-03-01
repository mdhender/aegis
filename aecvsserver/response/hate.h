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

#ifndef AECVSSERVER_RESPONSE_HATE_H
#define AECVSSERVER_RESPONSE_HATE_H

#include <aecvsserver/response.h>

/**
  * The response_hate class is used to represent a negative
  * authentication response to a CVS client.
  */
class response_hate:
    public response
{
public:
    /**
      * The destructor.
      */
    virtual ~response_hate();

    /**
      * The default constructor.
      */
    response_hate();

    // See base class for documentation.
    void write(output::pointer op);

    // See base class for documentation.
    response_code_ty code_get() const;

    // See base class for documentation.
    bool flushable() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    response_hate(const response_hate &);

    /**
      * The assignment operator.  Do not use.
      */
    response_hate &operator=(const response_hate &);
};

#endif // AECVSSERVER_RESPONSE_HATE_H
