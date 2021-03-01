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

#ifndef LIBAEGIS_RFC822_FUNCTOR_VERSION_PREV_H
#define LIBAEGIS_RFC822_FUNCTOR_VERSION_PREV_H

#include <libaegis/rfc822/functor.h>

/**
  * The rfc822_functor_version_previous class is used to represent the
  * parse state of a search for the first version number in the history
  * file.
  */
class rfc822_functor_version_previous:
    public rfc822_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~rfc822_functor_version_previous();

    /**
      * The default constructor.
      */
    rfc822_functor_version_previous();

    // See base class for documentation.
    bool operator()(rfc822 &arg);

    nstring get_result() const { return result; }

private:
    /**
      * The result instance variable is used to remember the version
      * string of the first revision in the file.
      */
    nstring result;

    /**
      * The copy constructor.
      */
    rfc822_functor_version_previous(const rfc822_functor_version_previous &);

    /**
      * The assignment operator.
      */
    rfc822_functor_version_previous &operator=(
	const rfc822_functor_version_previous &);
};

#endif // LIBAEGIS_RFC822_FUNCTOR_VERSION_PREV_H
