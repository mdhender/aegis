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

#ifndef FMTGEN_TYPE_VECTOR_H
#define FMTGEN_TYPE_VECTOR_H

#include <fmtgen/type.h>

/**
  * The type_vector class is used to represent an ordered list of type
  * pointers.
  */
class type_vector
{
public:
    /**
      * The destructor.
      */
    virtual ~type_vector();

    /**
      * The default constructor.
      */
    type_vector();

    /**
      * The copy constructor.
      */
    type_vector(const type_vector &);

    /**
      * The assignment operator.
      */
    type_vector &operator=(const type_vector &);

    void clear();

    void push_back(const type::pointer &arg);

    void push_back(const type_vector &arg);

    size_t size() const { return length; }

    bool empty() const { return (length == 0); }

    void gen_body() const;

    void gen_report_initializations() const;

    type::pointer
    get(size_t n)
        const
    {
        return (n < length ? item[n] : type::pointer());
    }

    type::pointer operator[](size_t n) const { return get(n); }

    /**
      * The member method is used to determine whether or not the given
      * type is already present.  Note that the comparison is by name, not
      * by the simple pointer value.
      */
    bool member(const type::pointer &p) const;

    /**
      * The reachable_closure method is used to walk the list of types,
      * and add to it any subtypes mentioned by them, and so on, until
      * the complete set of reachable types is obtained.
      */
    void reachable_closure();

private:
    size_t length;
    size_t maximum;
    type::pointer *item;

    void grow_slow(size_t n);

    void
    grow(size_t n)
    {
        if (length + n > maximum)
            grow_slow(n);
    }
};

#endif // FMTGEN_TYPE_VECTOR_H
