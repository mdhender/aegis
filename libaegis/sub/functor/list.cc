//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/arglex.h>
#include <common/trace.h>
#include <libaegis/sub/functor/list.h>


sub_functor_list::~sub_functor_list()
{
    clear();
    delete [] list;
    list = 0;
    length = 0;
    maximum = 0;
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
            trace(("hit \"%s\"\n", sfp->name_get().c_str()));
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
    const
{
    if (n >= length)
        return sub_functor::pointer();
    return list[n];
}


sub_functor::pointer
sub_functor_list::find(const nstring &name)
    const
{
    for (size_t j = 0; j < length; ++j)
    {
        sub_functor::pointer sfp = list[j];
        if (0 == strcasecmp(name.c_str(), sfp->name_get().c_str()))
            return sfp;
    }
    return sub_functor::pointer();
}
