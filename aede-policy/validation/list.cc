//
// aegis - project change supervisor
// Copyright (C) 2005-2008, 2010, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <aede-policy/validation/list.h>


validation_list::~validation_list()
{
    clear();
    delete [] list;
    list = 0;
    maximum = 0;
}


validation_list::validation_list() :
    length(0),
    maximum(0),
    list(0)
{
}


void
validation_list::clear()
{
    for (size_t j = 0; j < length; ++j)
        list[j].reset();
    length = 0;
}


void
validation_list::push_back(const validation::pointer &vp)
{
    if (length >= maximum)
    {
        size_t new_maximum = 2 * maximum + 16;
        validation::pointer *new_list = new validation::pointer [new_maximum];
        for (size_t j = 0; j < length; ++j)
            new_list[j] = list[j];
        delete [] list;
        list = new_list;
        maximum = new_maximum;
    }
    list[length++] = vp;
}


bool
validation_list::run(change::pointer cp)
    const
{
    bool result = true;
    for (size_t j = 0; j < length; ++j)
    {
        if (!list[j]->run(cp))
            result = false;
    }
    return result;
}


// vim: set ts=8 sw=4 et :
