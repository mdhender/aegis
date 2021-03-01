//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the nstring::replace method
//

#include <ac/string.h>

#include <nstring.h>
#include <nstring/accumulator.h>


nstring
nstring::replace(const nstring &lhs, const nstring &rhs, int maximum)
    const
{
    static nstring_accumulator sa;
    sa.clear();

    //
    // Deal with some trivial cases.
    //
    if (lhs.size() == 0 || maximum == 0)
	return *this;

    //
    // Default the number of times, if necessary.
    //
    if (maximum < 0)
	maximum = size() + 1;

    //
    // Walk along the string replacing things.
    //
    const char *ip = c_str();
    const char *ip_end = ip + size();
    while (ip < ip_end && (size_t)(ip_end - ip) >= lhs.size())
    {
	if (0 == memcmp(ip, lhs.c_str(), lhs.size()))
	{
	    sa.push_back(rhs);
	    ip += lhs.size();
	    if (--maximum <= 0)
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
    if (ip < ip_end)
	sa.push_back(ip, (size_t)(ip_end - ip));

    //
    // Build ther answer.
    //
    return sa.mkstr();
}
