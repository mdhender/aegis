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
// MANIFEST: interface definition for aecvsserver/request/add.c
//

#ifndef AECVSSERVER_REQUEST_ADD_H
#define AECVSSERVER_REQUEST_ADD_H

#include <aecvsserver/request.h>

/**
  * The request_add class is used to represent an add request from the
  * client.
  */
class request_add:
    public request
{
public:
    /**
      * The destructor.
      */
    virtual ~request_add();

    /**
      * The default constructor.
      */
    request_add();

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
    request_add(const request_add &);

    /**
      * The assignment operator.  Do not use.
      */
    request_add &operator=(const request_add &);
};

#endif // AECVSSERVER_REQUEST_ADD_H
