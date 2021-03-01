//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
//	All rights reserved.
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
// MANIFEST: implementation of the str_replace class
//

#include <common/ac/string.h>

#include <common/str.h>
#include <common/stracc.h>


string_ty *
str_replace(string_ty *str, string_ty *lhs, string_ty *rhs, int how_many_times)
{
    static stracc_t sa;

    //
    // Deal with some trivial cases.
    //
    if (lhs->str_length == 0 || how_many_times == 0)
	return str_copy(str);

    //
    // Default the number of times, if necessary.
    //
    if (how_many_times < 0)
	how_many_times = str->str_length + 1;

    //
    // Walk along the string replacing things.
    //
    const char *ip = str->str_text;
    const char *ip_end = str->str_text + str->str_length;
    sa.clear();
    while (ip < ip_end && (size_t)(ip_end - ip) >= lhs->str_length)
    {
	if (0 == memcmp(ip, lhs->str_text, lhs->str_length))
	{
	    sa.push_back(rhs->str_text, rhs->str_length);
	    ip += lhs->str_length;
	    if (--how_many_times <= 0)
		break;
	}
	else
	{
	    char c = *ip++;
	    sa.push_back(c);
	}
    }

    //
    // Collect the tail-end of the input.
    //
    sa.push_back(ip, ip_end - ip);

    //
    // Build ther answer.
    //
    return sa.mkstr();
}
