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

#ifndef AECVSSERVER_REQUEST_VERSION_H
#define AECVSSERVER_REQUEST_VERSION_H

#include <aecvsserver/request.h>

/**
  * The request_version class is used to represent a version request
  * from a CVS client.
  */
class request_version:
    public request
{
public:
    /**
      * The destructor.
      */
    virtual ~request_version();

    /**
      * The default constructor.
      */
    request_version();

    // See base class for documentation.
    void run_inner(server_ty *sp, string_ty *fn) const;

    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool reset() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    request_version(const request_version &);

    /**
      * The assignment operator.  Do not use.
      */
    request_version &operator=(const request_version &);
};

#endif // AECVSSERVER_REQUEST_VERSION_H
