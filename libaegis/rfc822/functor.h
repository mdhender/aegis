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

#ifndef LIBAEGIS_RFC822_FUNCTOR_H
#define LIBAEGIS_RFC822_FUNCTOR_H

#include <libaegis/rfc822.h>

/**
  * The rfc822_functor class is used to represent an abstract object
  * which may be treated af if it was a simple pointer to a function.
  * Pointers to functions are common C solutions, and make for a good
  * idiom for "programmer portability".
  */
class rfc822_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~rfc822_functor();

    /**
      * The default constructor.
      */
    rfc822_functor();

    /**
      * The () operator is used to invoke the functionality implemented
      * in the functor.
      *
      * @param arg
      *     The rfc822 header in question.
      * @returns
      *     boolean; true to keep on walking down the version list, or
      *              false to stop at this point.
      */
    virtual bool operator()(rfc822 &arg) = 0;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rfc822_functor(const rfc822_functor &);

    /**
      * The assignment operator.  Do not use.
      */
    rfc822_functor &operator=(const rfc822_functor &);
};

#endif // LIBAEGIS_RFC822_FUNCTOR_H
