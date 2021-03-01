//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2006, 2008 Peter Miller
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

#include <libaegis/patch/context.h>


patch_context_ty::patch_context_ty(input &arg) :
    in(arg)
{
}


patch_context_ty::~patch_context_ty()
{
}


string_ty *
patch_context_ty::getline(int n)
{
    if (n < 0)
	return 0;
    while (n >= (int)buffer.nstrings)
    {
	nstring s;
	if (!in->one_line(s))
	    return 0;
	buffer.push_back(s.get_ref());
    }
    return buffer.string[n];
}


void
patch_context_ty::discard(int n)
{
    if (n <= 0)
	return;
    for (size_t j = 0; j < (size_t)n && j < buffer.nstrings; ++j)
	str_free(buffer.string[j]);
    if (n >= (int)buffer.nstrings)
	buffer.nstrings = 0;
    else
    {
	for (size_t k = n; k < buffer.nstrings; ++k)
    	    buffer.string[k - n] = buffer.string[k];
	// memory leak?  it didn't str_free() the strings
	buffer.nstrings -= n;
    }
}
