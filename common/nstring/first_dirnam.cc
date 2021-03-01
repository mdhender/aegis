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

#include <common/nstring.h>


nstring
nstring::first_dirname()
    const
{
    const char *start = c_str();
    while (*start == '/' && start[1] == '/')
        ++start;
    const char *tmp = start;
    if (*tmp == '/')
	++tmp;
    const char *end = strchr(tmp, '/');
    if (!end)
    {
        if (tmp != start)
            return "/";
	return ".";
    }
    return nstring(start, end - start);
}
