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

#include <common/gmatch.h>
#include <common/nstring/list.h>


int
nstring_list::gmatch_pattern(const nstring &pattern)
    const
{
    for (size_t j = 0; j < size(); ++j)
    {
	int result = gmatch(pattern, get(j));
	if (result)
	{
	    // could be 1 meaning a match, or
	    // could be -1 meaning an invalid pattern.
	    return result;
	}
    }
    return 0;
}
