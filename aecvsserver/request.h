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

#ifndef AECVSSERVER_REQUEST_H
#define AECVSSERVER_REQUEST_H

#include <common/str.h>

#include <aecvsserver/server.h>

/**
  * The request abstract base class is used to represent a request
  * from a client.
  */
class request
{
public:
    /**
      * The destructor.
      */
    virtual ~request();

    /**
      * The find class method is used to locate the function to call for
      * the named request.  Returns NULL for unknown requests.
      *
      * @param name
      *     The name of the request.
      */
    static const request *find(string_ty *name);

    /**
      * The run method is used to run the server request handler.  It
      * will return when the client closes the connection.
      */
    void run(struct server_ty *sp, string_ty *arg) const;

    /**
      * The name method is used to obtain the name of the request.
      */
    virtual const char *name() const = 0;

    /**
      * The reset method returns true of the argument accumulator needs
      * to be reset after the request has run.
      */
    virtual bool reset() const = 0;

protected:
    /**
      * The default constructor.
      */
    request();

    /**
      * The run_inner method is used to run the server request handler.
      * It will return when the client closes the connection.
      */
    virtual void run_inner(struct server_ty *sp, string_ty *arg) const = 0;

    /**
      * The get_list class method is used to obtain the list of the
      * names of known requests.
      *
      * @param wl
      *     The list of the names of known requests is returned here.
      */
    static void get_list(string_list_ty &wl);

private:
    /**
      * The copy constructor.  Do not use.
      */
    request(const request &);

    /**
      * The assignment operator.  Do not use.
      */
    request &operator=(const request &);
};

#endif // AECVSSERVER_REQUEST_H
