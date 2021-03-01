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
// MANIFEST: method to unsplit string lists
//

#include <common/ac/string.h>

#include <common/stracc.h>
#include <common/str_list.h>
#include <common/trace.h>


string_ty *
string_list_ty::unsplit(const char *sep)
    const
{
    return unsplit(0, size(), sep);
}


string_ty *
string_list_ty::unsplit(size_t start, size_t stop, const char *sep)
    const
{
    if (!sep)
	sep = " ";
    trace(("string_list_ty::unsplit(start = %d, stop = %d, sep = \"%s\")\n",
	(int)start, (int)stop, sep));
    size_t seplen = strlen(sep);

    static stracc_t tmp;
    tmp.clear();

    for (size_t j = start; j <= stop && j < nstrings; j++)
    {
	string_ty *s = string[j];
	if (s->str_length)
	{
	    if (!tmp.empty())
		tmp.push_back(sep, seplen);
	    tmp.push_back(s->str_text, s->str_length);
	}
    }

    return tmp.mkstr();
}
