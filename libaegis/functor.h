//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_FUNCTOR_H
#define LIBAEGIS_FUNCTOR_H

#include <common/ac/shared_ptr.h>

/**
  * The functor class is used to represent the most generic of all
  * callback functions, one that takes no arguments.
  */
class functor
{
public:
    typedef aegis_shared_ptr<functor> pointer;

    /**
      * The destructor.
      */
    virtual ~functor();

protected:
    /**
      * The default constructor.
      */
    functor();

public:
    /**
      * The function call operator, with no arguments.
      * This is why it is called a functor.
      */
    virtual void operator()() = 0;

    /// helper function to make some code less ugly
    void
    call()
    {
        operator()();
    }

    /**
      * The copy constructor.  Do not use.
      */
    functor(const functor &);

    /**
      * The assignment operator.  Do not use.
      */
    functor &operator=(const functor &);
};

#endif // LIBAEGIS_FUNCTOR_H
