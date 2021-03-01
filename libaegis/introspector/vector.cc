//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/error.h>
#include <libaegis/introspector/vector.h>


introspector_vector::~introspector_vector()
{
    delete [] item;
    length = 0;
    maximum = 0;
    item = 0;
}


introspector_vector::introspector_vector() :
    length(0),
    maximum(0),
    item(0)
{
}


introspector_vector::introspector_vector(const introspector_vector &arg) :
    length(0),
    maximum(0),
    item(0)
{
    push_back(arg);
}


introspector_vector &
introspector_vector::operator=(const introspector_vector &arg)
{
    if (this != &arg)
    {
        clear();
        push_back(arg);
    }
    return *this;
}


void
introspector_vector::clear()
{
    while (length > 0)
    {
        --length;
        item[length].reset();
    }
}


void
introspector_vector::push_back(const introspector_vector &arg)
{
    grow(arg.length);
    for (size_t j = 0; j < arg.length; ++j)
        item[length++] = arg.item[j];
}


void
introspector_vector::grow_slow(size_t nitems)
{
    assert(length + nitems > maximum);
    size_t new_maximum = maximum;
    for (;;)
    {
        new_maximum = new_maximum * 2 + 8;
        if (length + nitems <= new_maximum)
            break;
    }
    introspector::pointer *new_item =
        new introspector::pointer [new_maximum];
    for (size_t j = 0; j < length; ++j)
        new_item[j] = item[j];
    delete [] item;
    item = new_item;
    maximum = new_maximum;
}


// vim: set ts=8 sw=4 et :
