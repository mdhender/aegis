//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>

#include <common/nstring.h>
#include <common/nstring/accumulator.h>


nstring
nstring::trim_lines()
    const
{
    nstring_accumulator ac;
    const char *sp = c_str();
    while (*sp && *sp != '\n' && isspace((unsigned char)*sp))
	++sp;
    for (;;)
    {
	unsigned char c = *sp++;
	if (!c)
	    break;
	if (c == '\n')
	{
	    ac.push_back(c);
	    while (*sp && *sp != '\n' && isspace((unsigned char)*sp))
	       	++sp;
	}
	else if (isspace(c))
	{
	    for (;;)
	    {
		if (!*sp || *sp == '\n')
		    break;
		if (!isspace((unsigned char)*sp))
		{
		    ac.push_back(' ');
		    break;
		}
		++sp;
	    }
	}
	else
	{
	    ac.push_back(c);
	}
    }
    return ac.mkstr();
}
