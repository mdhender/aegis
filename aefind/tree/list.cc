//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002-2008 Peter Miller
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

#include <aefind/tree/list.h>
#include <aefind/tree/this.h>


tree_list::~tree_list()
{
    delete [] item;
    item = 0;
    length = 0;
    maximum = 0;
}


tree_list::tree_list() :
    item(0),
    length(0),
    maximum(0)
{
}


tree_list::tree_list(const tree_list &arg) :
    item(0),
    length(0),
    maximum(0)
{
    append(arg);
}


tree_list &
tree_list::operator=(const tree_list &arg)
{
    if (this != &arg)
    {
        clear();
        append(arg);
    }
    return *this;
}


void
tree_list::clear()
{
    while (length > 0)
    {
        --length;
	item[length].reset();
    }
}


void
tree_list::append(const tree_list &arg)
{
    for (size_t j = 0; j < arg.size(); ++j)
        append(arg[j]);
}


void
tree_list::append(const tree::pointer &tp)
{
    if (length >= maximum)
    {
	size_t new_maximum = maximum * 2 + 4;
	tree::pointer *new_item = new tree::pointer [new_maximum];
	for (size_t k = 0; k < length; ++k)
	    new_item[k] = item[k];
	delete [] item;
	item = new_item;
        maximum = new_maximum;
    }
    item[length++] = tp;
}


tree::pointer
tree_list::get(size_t n)
    const
{
    if (n >= length)
        return tree_this::create();
    return item[n];
}
