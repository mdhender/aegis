//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2003-2006, 2008 Peter Miller
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

#include <libaegis/gif.h>


void
gif_rect(gif_ty *gp, int x1, int y1, int x2, int y2, int clr)
{
	int		tmp;
	int		x, y;

	if (x1 > x2)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y1 > y2)
	{
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	for (y = y1; y < y2; ++y)
		for (x = x1; x < x2; ++x)
			gif_pixel_set(gp, x, y, clr);
}
