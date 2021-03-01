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

#include <common/error.h> // for assert
#include <common/nstring.h>


nstring
nstring::substring(long start, long nbytes)
    const
{
    //
    // If start is negative, measure from the end instead of the beginning.
    //
    if (start < 0)
    {
	start += size();
    }

    //
    // If nbytes is negative, measure to the left instead of to the right.
    //
    if (nbytes < 0)
    {
	start += nbytes;
	nbytes = -nbytes;
    }

    //
    // If the start of the string is negative, pretend the negative part
    // is empty.
    //
    if (start < 0)
    {
	nbytes += start;
	start = 0;
    }

    //
    // If, after all that, the substring is still to the left of the
    // content, return the empty string.
    //
    assert(start >= 0);
    if (start + nbytes < 0)
	return "";
    assert(nbytes >= 0);

    //
    // If the bumber of bytes would project beynd the end of the string,
    // truncate them.
    //
    assert(start + nbytes >= 0);
    if ((size_t)(start + nbytes) > size())
	nbytes = size() - start;

    //
    // Build the substring.
    //
    return nstring(c_str() + start, nbytes);
}
