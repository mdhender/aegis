//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#include <common/ac/stdio.h>

#include <common/nstring/list.h>
#include <libaegis/rfc822/functor/list_meta.h>


rfc822_functor_list_meta::~rfc822_functor_list_meta()
{
}


rfc822_functor_list_meta::rfc822_functor_list_meta() :
    first(true)
{
}


bool
rfc822_functor_list_meta::operator()(rfc822 &arg)
{
    if (first)
	first = false;
    else
	putchar('\n');
    nstring_list names;
    arg.keys(names);

    //
    // We sort the names so that automated tests get predictable
    // results, even if we choose to change the hashing algorithm one
    // day.  It also makes it more predictable for the dumb humans to
    // comprehend if they always come out in the same order.
    //
    names.sort();

    for (size_t j = 0; j < names.size(); ++j)
    {
	nstring key = names[j];
	printf("%s: %s\n", key.capitalize().c_str(), arg.get(key).c_str());
    }
    return true;
}
