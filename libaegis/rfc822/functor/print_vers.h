//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: interface of the rfc822_functor_print_vers class
//

#ifndef LIBAEGIS_RFC822_FUNCTOR_PRINT_VERS_H
#define LIBAEGIS_RFC822_FUNCTOR_PRINT_VERS_H

#pragma interface "rfc822_functor_print_version"

#include <rfc822/functor.h>

/**
  * The rfc822_functor_print_version class is used to represent a simple
  * functior which extratcts and prints a version number.
  */
class rfc822_functor_print_version:
    public rfc822_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~rfc822_functor_print_version();

    /**
      * The default constructor.
      */
    rfc822_functor_print_version();

    // See base class for documentation.
    bool operator()(rfc822 &arg);

private:
    /**
      * The copy constructor.  Do not use.
      */
    rfc822_functor_print_version(const rfc822_functor_print_version &);

    /**
      * The assignment operator.  Do not use.
      */
    rfc822_functor_print_version &operator=(
	const rfc822_functor_print_version &);
};

#endif // LIBAEGIS_RFC822_FUNCTOR_PRINT_VERS_H
