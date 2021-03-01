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

#include <common/ac/ctype.h>

#include <common/nstring.h>
#include <common/nstring/accumulator.h>


nstring
nstring::identifier()
    const
{
    static nstring_accumulator buf;
    buf.clear();
    const char *cp = c_str();
    for (;;)
    {
	unsigned char c = *cp++;
	if (!c)
		break;
	// C locale
	if (!isalnum((unsigned char)c))
    	    c = '_';
	buf.push_back(c);
    }
    return buf.mkstr();
}
