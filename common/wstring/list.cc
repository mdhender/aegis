//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <common/wstring/list.h>


wstring_list &
wstring_list::operator=(const wstring_list &arg)
{
    if (&arg != this)
    {
	clear();
	push_back(arg);
    }
    return *this;
}


void
wstring_list::clear()
{
    while (!content.empty())
    {
	content.pop_back();
    }
}


void
wstring_list::push_back(const wstring_list &arg)
{
    for (size_t j = 0; j < arg.size(); ++j)
	push_back(arg[j]);
}


wstring
wstring_list::get(int n)
    const
{
    if (n < 0 || (size_t)n >= content.size())
	return wstring();
    return wstring(content[n]);
}


wstring
wstring_list::unsplit(const char *separator)
    const
{
    return wstring(content.unsplit(separator));
}


wstring
wstring_list::unsplit(size_t first, size_t last, const char *separator)
    const
{
    return wstring(content.unsplit(first, last, separator));
}
