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
// MANIFEST: interface definition for aecvsserver/request/directory.c
//

#ifndef AECVSSERVER_REQUEST_DIRECTORY_H
#define AECVSSERVER_REQUEST_DIRECTORY_H

#include <aecvsserver/request.h>

/**
  * The request_directory class is used to represent a directory request
  * from a CVS client.
  */
class request_directory:
    public request
{
public:
    /**
      * The destructor.
      */
    virtual ~request_directory();

    /**
      * The default constructor.
      */
    request_directory();

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
    request_directory(const request_directory &);

    /**
      * The assignment operator.  Do not use.
      */
    request_directory &operator=(const request_directory &);
};

#endif // AECVSSERVER_REQUEST_DIRECTORY_H
