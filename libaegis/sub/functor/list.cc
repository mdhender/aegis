//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//

#include <common/arglex.h>
#include <libaegis/sub/functor/list.h>


sub_functor_list::~sub_functor_list()
{
}


sub_functor_list::sub_functor_list() :
    list(0),
    length(0),
    maximum(0)
{
}


void
sub_functor_list::push_back(sub_functor::pointer sfp)
{
    if (length >= maximum)
    {
        size_t new_maximum = maximum * 2 + 8;
        sub_functor::pointer *new_list = new sub_functor::pointer [new_maximum];
        for (size_t j = 0; j < length; ++j)
            new_list[j] = list[j];
        delete [] list;
        list = new_list;
        maximum = new_maximum;
    }
    list[length++] = sfp;
}


void
sub_functor_list::match(const nstring &name, sub_functor_list &result)
{
    for (size_t j = 0; j < length; ++j)
    {
        sub_functor::pointer sfp = list[j];
        if (arglex_compare(sfp->name_get().c_str(), name.c_str(), 0))
        {
            if (sfp->override())
                result.clear();
            result.push_back(sfp);
        }
    }
}


void
sub_functor_list::clear()
{
    while (length > 0)
    {
        --length;
        list[length].reset();
    }
}


sub_functor::pointer
sub_functor_list::get(size_t n)
{
    if (n >= length)
        return sub_functor::pointer();
    return list[n];
}


sub_functor::pointer
sub_functor_list::find(const nstring &name)
{
    for (size_t j = 0; j < length; ++j)
    {
        sub_functor::pointer sfp = list[j];
        if (name == sfp->name_get())
            return sfp;
    }
    return sub_functor::pointer();
}
