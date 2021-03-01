//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_RFC822_FUNCTOR_VERS_SEARCH_H
#define LIBAEGIS_RFC822_FUNCTOR_VERS_SEARCH_H

#include <libaegis/rfc822/functor.h>

/**
  * The rfc822_functor_version_search class is used to represent the
  * state of walking a history file looking for a specific version
  * number.
  */
class rfc822_functor_version_search:
    public rfc822_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~rfc822_functor_version_search();

    /**
      * The constructor.
      */
    rfc822_functor_version_search(const nstring &arg);

    // See base class for documentation.
    bool operator()(rfc822 &arg);

    /**
      * The get_result method is used after walking the history file, to
      * determine if the specified version was present.
      */
    bool get_result() const { return result; }

private:
    /**
      * The version instance variable is used to remember the version to
      * search for.
      */
    nstring version;

    /**
      * The result instance variable is used to remember whether or not
      * the version was found.
      */
    bool result;

    /**
      * The default constructor.  Do not use.
      */
    rfc822_functor_version_search();

    /**
      * The copy constructor.  Do not use.
      */
    rfc822_functor_version_search(const rfc822_functor_version_search &);

    /**
      * The assignment operator.  Do not use.
      */
    rfc822_functor_version_search &operator=(
	const rfc822_functor_version_search &);
};

#endif // LIBAEGIS_RFC822_FUNCTOR_VERS_SEARCH_H
