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
gif_colormap_set(gif_ty *gp, int idx, int r, int g, int b)
{
	if (idx >= 0 && idx < 256)
	{
		gp->colormap[idx][0] = r;
		gp->colormap[idx][1] = g;
		gp->colormap[idx][2] = b;
	}
}
