//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006 Peter Miller
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
// MANIFEST: functions to manipulate subsets
//

#include <common/str_list.h>


bool
string_list_ty::subset(const string_list_ty &arg)
    const
{
    //
    // test if "a is a subset of b"
    //
    if (nstrings > arg.nstrings)
	return false;
    for (size_t j = 0; j < nstrings; ++j)
    {
	size_t k;
	for (k = 0; k < arg.size(); ++k)
	    if (str_equal(string[j], arg[k]))
		break;
	if (k >= arg.nstrings)
	    return false;
    }
    return true;
}
