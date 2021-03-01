//
//	aegis - project change supervisor
//	Copyright (C) 1996, 2003-2006, 2008 Peter Miller
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
