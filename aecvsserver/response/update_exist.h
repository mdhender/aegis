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

#ifndef AECVSSERVER_RESPONSE_UPDATE_EXIST_H
#define AECVSSERVER_RESPONSE_UPDATE_EXIST_H

#include <libaegis/input.h>

#include <aecvsserver/response.h>

/**
  * The response_update_existing class is used to represent an update
  * existing response to a CVS client.
  */
class response_update_existing:
    public response
{
public:
    /**
      * The destructor.
      */
    virtual ~response_update_existing();

    /**
      * The constructor.
      *
      * @param client_side
      *     The name of the file being sent to the client, relative to the
      *     current directory on the client side.
      * @param server_side
      *     The name of the file being sent to the client, relative to the
      *     root on the server side.
      * @param source
      *     The input to be read to get the content of the file.  This will
      *     be closed once the request is sent to the client.
      * @param mode
      *     The file mode for the file.
      * @param
      *     The current version of the file.
      */
    response_update_existing(string_ty *client_side, string_ty *server_side,
	input &source, int mode, string_ty *version);

    // See base class for documentation.
    void write(output::pointer op);

    // See base class for documentation.
    response_code_ty code_get() const;

    // See base class for documentation.
    bool flushable() const;

private:
    string_ty *client_side;
    string_ty *server_side;
    input source;
    int mode;
    string_ty *version;

    /**
      * The default constructor.  Do not use.
      */
    response_update_existing();

    /**
      * The copy constructor.  Do not use.
      */
    response_update_existing(const response_update_existing &);

    /**
      * The assignment operator.  Do not use.
      */
    response_update_existing &operator=(const response_update_existing &);
};

#endif // AECVSSERVER_RESPONSE_UPDATE_EXIST_H
