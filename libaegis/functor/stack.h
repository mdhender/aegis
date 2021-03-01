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

#ifndef LIBAEGIS_FUNCTOR_STACK_H
#define LIBAEGIS_FUNCTOR_STACK_H

#include <common/ac/stddef.h>
#include <libaegis/functor.h>

/**
  * The functor_stack class is used to represent an ordered set of
  * functors presented as a single functor.
  */
class functor_stack:
    public functor
{
public:
    /**
      * The destructor.
      */
    virtual ~functor_stack();

    /**
      * The default constructor.
      */
    functor_stack();

    // See base class for eodumentation.
    void operator()();

    void push_back(functor::pointer fp);
    void remove(functor::pointer fp);

private:
    pointer *content;
    size_t ncontents;
    size_t ncontents_maximum;

    /**
      * The copy constructor.  Do not use.
      */
    functor_stack(const functor_stack &);

    /**
      * The assignment operator.  Do not use.
      */
    functor_stack &operator=(const functor_stack &);
};

#endif // LIBAEGIS_FUNCTOR_STACK_H
