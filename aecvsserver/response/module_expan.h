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

#ifndef AECVSSERVER_RESPONSE_MODULE_EXPAN_H
#define AECVSSERVER_RESPONSE_MODULE_EXPAN_H

#include <aecvsserver/response.h>

/**
  * The response_module_expansion class is used to represent a module
  * expansion response to a CVS client.
  */
class response_module_expansion:
    public response
{
public:
    /**
      * The destructor.
      */
    virtual ~response_module_expansion();

    /**
      * The constructor.
      */
    response_module_expansion(string_ty *answer);

    // See base class for documentation.
    void write(output::pointer op);

    // See base class for documentation.
    response_code_ty code_get() const;

private:
    string_ty *answer;

    /**
      * The default constructor.
      */
    response_module_expansion();

    /**
      * The copy constructor.  Do not use.
      */
    response_module_expansion(const response_module_expansion &);

    /**
      * The assignment operator.  Do not use.
      */
    response_module_expansion &operator=(const response_module_expansion &);
};

#endif // AECVSSERVER_RESPONSE_MODULE_EXPAN_H
