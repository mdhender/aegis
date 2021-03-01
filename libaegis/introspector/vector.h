//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#ifndef LIBAEGIS_INTROSPECTOR_VECTOR_H
#define LIBAEGIS_INTROSPECTOR_VECTOR_H

#include <libaegis/introspector.h>

/**
  * The introspector_vector class is used to represent an ordered list of
  * introspector::pointer values.  It has O(1) append times.
  */
class introspector_vector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_vector();

    /**
      * The default constructor.
      */
    introspector_vector();

    /**
      * The copy constructor.
      */
    introspector_vector(const introspector_vector &);

    /**
      * The assignment operator.
      */
    introspector_vector &operator=(const introspector_vector &);

    /**
      * The clear method is used to discard all items in the vector.
      */
    void clear();

    /**
      * The empty method is used to determine whether or not the vector is
      * empty.
      */
    bool empty() const { return (length == 0); }

    /**
      * The size method is used to obtain the number of items in the vector.
      */
    size_t size() const { return length; }

    /**
      * The back method is used to obtain the item on the back of the vector.
      */
    introspector::pointer
    back()
        const
    {
        return (length == 0 ? introspector::pointer() : item[length - 1]);
    }

    /**
      * The push_back method is used to append another item to the end of
      * the vector.
      *
      * @param ip
      *     The item to be appended.
      */
    void
    push_back(const introspector::pointer &ip)
    {
        grow(1);
        item[length++] = ip;
    }

    /**
      * The push_back method is used to append the contents of another
      * vector to the end of this vector.
      *
      * @param il
      *     The introspector vector to be appended.
      */
    void push_back(const introspector_vector &il);

    /**
      * The pop_back method is used to discard the last item from this
      * vector.
      */
    void pop_back();

private:
    /**
      * The length instance variable is used to remember the number of
      * used entries in the allocated item array.
      */
    size_t length;

    /**
      * The maximum instance variable is used to remember the size of
      * the allocated item array.
      */
    size_t maximum;

    /**
      * The item instance variable is used to remember the address of
      * the base of an allocated array of introspector pointers.
      */
    introspector::pointer *item;

    /**
      * The grow_slow method is used to allocate more space, in the case
      * where the current allocation has grown too small.
      *
      * @param nitems
      *     The number of items to grow by.
      */
    void grow_slow(size_t nitems);

    /**
      * The grow method is used to ensure the is sufficient allocated
      * space.
      *
      * @param nitems
      *     The number of items to grow by.
      */
    void
    grow(size_t nitems)
    {
        if (length + nitems > maximum)
            grow_slow(nitems);
    }
};

#endif // LIBAEGIS_INTROSPECTOR_VECTOR_H
