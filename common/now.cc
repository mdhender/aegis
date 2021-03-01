//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate nows
//

#include <error.h>
#include <now.h>


static int do_not_clear;
static time_t when;


time_t
now(void)
{
    if (when == 0)
    {
	time_t          x;

	if (time(&x) == (time_t)(-1))
	    nfatal("time");
	now_set(x);
    }
    return when;
}


void
now_set(time_t x)
{
    when = (x == (time_t)(-1) || x == 0) ? 1 : x;
}


void
now_unclearable()
{
    do_not_clear = 1;
}


void
now_clear(void)
{
    if (!do_not_clear)
	when = 0;
}


static time_t
time_max_find(void)
{
    time_t          x;
    time_t          x2;

    //
    // The time_t type is a signed integer type, but we don't know
    // WHICH integer type, so it isn't a simple case of manipulating
    // the equivalent unsigned type to form the appropriate bit pattern.
    //
    x = 32767;
    for (;;)
    {
	//
	// Make the pattern one bit lobger, right justified.
	//
	x2 = (x << 1) | 1;

	//
	// If we have found the (time_t)(-1) value, the previous value
	// must have been the maximum positive one.
	//
	// Watch out for the arithmetic, this has to work for 2s
	// complement as well as 1s complement (i.e. all-bits-one could
	// be minus-zero).
	//
	if (x2 < x)
	    return x;

	//
	// Looks like it is actually an unsigned type (which isn't quite
	// standard conforming).
	//
	if (x2 == x)
	    return x;

	//
	// Remember this value, and go round again.
	//
	x = x2;
    }
}


time_t
time_max(void)
{
    static time_t   result;

    if (!result)
	result = time_max_find();
    return result;
}
