//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2004-2006, 2008 Peter Miller
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

#include <common/skip_unlucky.h>


long
skip_unlucky(long n)
{
    long	    mult;

    //
    // Celtic tradition, I think.  Anyone know its origin?
    //
    if (n == 13)
	++n;

    //
    // Christian tradition: the number of the devil.
    //
    if (n == 666 || n == 69)
	++n;

    //
    // Eastern tradition: the number "4" can sound like "death"
    // if slightly mis-pronounced.
    //
    for (mult = 1; mult < n; mult *= 10)
    {
	long            fudge;

	fudge = n / mult;
	if ((fudge % 10) == 4)
    	    n = (fudge + 1) * mult;
    }
    return n;
}
