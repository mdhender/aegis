//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2003-2006 Peter Miller
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
// MANIFEST: functions to get GIF colormaps
//

#include <libaegis/gif.h>


void
gif_colormap_get(gif_ty *gp, int idx, int *r, int *g, int *b)
{
	if (idx < 0 || idx >= 256)
	{
		*r = 0;
		*g = 0;
		*b = 0;
		return;
	}
	*r = gp->colormap[idx][0];
	*g = gp->colormap[idx][1];
	*b = gp->colormap[idx][2];
}
