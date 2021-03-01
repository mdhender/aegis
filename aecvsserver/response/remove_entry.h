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

#ifndef AECVSSERVER_RESPONSE_REMOVE_ENTRY_H
#define AECVSSERVER_RESPONSE_REMOVE_ENTRY_H

#include <aecvsserver/response.h>

/**
  * The response_remove_entry class is used to represent a remove entry
  * response to a CVS client.
  */
class response_remove_entry:
    public response
{
public:
    /**
      * The destructor.
      */
    virtual ~response_remove_entry();

    /**
      * The constructor.
      */
    response_remove_entry(string_ty *client, string_ty *server);

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
    response_remove_entry();

    /**
      * The copy constructor.  Do not use.
      */
    response_remove_entry(const response_remove_entry &);

    /**
      * The assignment operator.  Do not use.
      */
    response_remove_entry &operator=(const response_remove_entry &);
};

#endif // AECVSSERVER_RESPONSE_REMOVE_ENTRY_H
