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

#ifndef LIBAEGIS_RFC822_FUNCTOR_LIST_META_H
#define LIBAEGIS_RFC822_FUNCTOR_LIST_META_H

#include <libaegis/rfc822/functor.h>

/**
  * The rfc822_functor_list_meta class is used to represent header
  * processing which prints some of the meta-data.
  */
class rfc822_functor_list_meta:
    public rfc822_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~rfc822_functor_list_meta();

    /**
      * The default constructor.
      */
    rfc822_functor_list_meta();

    // See base class for documentation.
    bool operator()(rfc822 &arg);

private:
    /**
      * The first instance variable is used to remember whether or not
      * we are at the first item or not.
      */
    bool first;

    /**
      * The copy constructor.  Do not use.
      */
    rfc822_functor_list_meta(const rfc822_functor_list_meta &);

    /**
      * The assignment operator.  Do not use.
      */
    rfc822_functor_list_meta &operator=(const rfc822_functor_list_meta &);
};

#endif // LIBAEGIS_RFC822_FUNCTOR_LIST_META_H
