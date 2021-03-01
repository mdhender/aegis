//
//	aegis - project change supervisor
//	Copyright (C) 1996, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate zeros
//

#include <common/error.h>
#include <libaegis/zero.h>


long
magic_zero_encode(long x)
{
	if (x == 0)
		x = MAGIC_ZERO;
	return x;
}


long
magic_zero_decode(long x)
{
	assert(MAGIC_ZERO != 0);
	if (x == MAGIC_ZERO)
		x = 0;
	return x;
}
