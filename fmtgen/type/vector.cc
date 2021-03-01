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

#include <fmtgen/type/vector.h>


type_vector::~type_vector()
{
    delete [] item;
    length = 0;
    maximum = 0;
    item = 0;
}


type_vector::type_vector() :
    length(0),
    maximum(0),
    item(0)
{
}


type_vector::type_vector(const type_vector &arg) :
    length(0),
    maximum(0),
    item(0)
{
    push_back(arg);
}


type_vector &
type_vector::operator=(const type_vector &arg)
{
    if (this != &arg)
    {
        clear();
        push_back(arg);
    }
    return *this;
}


void
type_vector::clear()
{
    while (length > 0)
    {
        --length;
        item[length].reset();
    }
}


void
type_vector::grow_slow(size_t n)
{
    if (length + n > maximum)
    {
        size_t new_maximum = maximum * 2 + 16;
        while (length + n > new_maximum)
            new_maximum = new_maximum * 2 + 16;
        type::pointer *new_item = new type::pointer [new_maximum];
        for (size_t j = 0; j < length; ++j)
            new_item[j] = item[j];
        delete [] item;
        item = new_item;
        maximum = new_maximum;
    }
}


void
type_vector::push_back(const type::pointer &arg)
{
    grow(1);
    item[length++] = arg;
}


void
type_vector::push_back(const type_vector &arg)
{
    grow(arg.size());
    for (size_t j = 0; j < arg.size(); ++j)
        item[length + j] = arg[j];
    length += arg.size();
}


void
type_vector::gen_body()
    const
{
    for (size_t j = 0; j < length; ++j)
        item[j]->gen_body();
}


void
type_vector::gen_report_initializations()
    const
{
    for (size_t j = 0; j < length; ++j)
    {
        type::pointer tp = item[j];
        tp->gen_report_initializations();
    }
}


bool
type_vector::member(const type::pointer &p)
    const
{
    for (size_t j = 0; j < length; ++j)
        if (item[j]->c_name() == p->c_name())
            return true;
    return false;
}


void
type_vector::reachable_closure()
{
    size_t top = 0;
    while (top < size())
    {
        type::pointer p = get(top);
        ++top;

        //
        // get the list of sub-types
        //
        type_vector tv;
        p->get_reachable(tv);

        //
        // Only append unique sub-types, otherwise we could
        // infinite loop.
        //
        for (size_t j = 0; j < tv.size(); ++j)
        {
            type::pointer p2 = tv[j];
            if (!member(p2))
                push_back(p2);
        }
    }
}
