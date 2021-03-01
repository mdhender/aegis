//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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

#include <common/mem.h>
#include <common/str_list.h>


void
string_list_ty::push_front(string_ty *w)
{
    if (nstrings >= nstrings_max)
    {
	//
	// always 8 less than a power of 2, which is
	// most efficient for many memory allocators
	//
	size_t new_nstrings_max = nstrings_max * 2 + 8;
	string_ty **new_string = new string_ty *[new_nstrings_max];
	memcpy(new_string + 1, string, nstrings * sizeof(string[0]));
	delete [] string;
	string = new_string;
	nstrings_max = new_nstrings_max;
    }
    else
    {
	for (size_t j = nstrings; j > 0; --j)
	    string[j] = string[j - 1];
    }
    nstrings++;
    string[0] = str_copy(w);
}
