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

#include <common/ac/stdlib.h>

#include <libaegis/gif.h>


#define XLO (1 << 0)
#define XHI (1 << 1)
#define YLO (1 << 2)
#define YHI (1 << 3)

#define clip_x_min 0
#define clip_y_min 0
#define clip_x_max (gp->width)
#define clip_y_max (gp->height)

#define encode(x, y) \
	(					\
		(x < clip_x_min ? XLO : 0)	\
	|					\
		(x > clip_x_max ? XHI : 0)	\
	|					\
		(y < clip_y_min ? YLO : 0)	\
	|					\
		(y > clip_y_max ? YHI : 0)	\
	)


void
gif_line(gif_ty *gp, int x1, int y1, int x2, int y2, int clr)
{
	int		c1;
	int		c2;
	double		dx1, dy1;
	double		dx2, dy2;
	int		dx;
	int		dy;
	int		e;
	int		dirn;

	dx1 = x1;
	dx2 = x2;
	dy1 = y1;
	dy2 = y2;

	c1 = encode(x1, y1);
	c2 = encode(x2, y2);
	while (c1 || c2)
	{
		if (c1 & c2)
			return; // outside viewport
		if (c1 & XLO)
		{
			//
			// No divide by zero possibility, because (x1 != x2)
			// otherwise (c1 & c2) would have been true.
			//
			y1 = y1 + (y2 - y1) * (clip_x_min - x1)
				/ (x2 - x1);			//lint !e414
			x1 = clip_x_min;
			c1 = encode(x1, y1);
			continue;
		}
		if (c1 & XHI)
		{
			//
			// No divide by zero possibility, because (x1 != x2)
			// otherwise (c1 & c2) would have been true.
			//
			y1 = y1 + (y2 - y1) * (clip_x_max - x1)
				/ (x2 - x1);			//lint !e414
			x1 = clip_x_max;
			c1 = encode(x1, y1);
			continue;
		}
		if (c1 & YLO)
		{
			//
			// No divide by zero possibility, because (y1 != y2)
			// otherwise (c1 & c2) would have been true.
			//
			x1 = x1 + (x2 - x1) * (clip_y_min - y1)
				/ (y2 - y1);			//lint !e414
			y1 = clip_y_min;
			c1 = encode(x1, y1);
			continue;
		}
		if (c1 & YHI)
		{
			//
			// No divide by zero possibility, because (y1 != y2)
			// otherwise (c1 & c2) would have been true.
			//
			x1 = x1 + (x2 - x1) * (clip_y_max - y1)
				/ (y2 - y1);			//lint !e414
			y1 = clip_y_max;
			c1 = encode(x1, y1);
			continue;
		}
		if (c2 & XLO)
		{
			//
			// No divide by zero possibility, because (x1 != x2)
			// otherwise (c1 & c2) would have been true.
			//
			y2 = y2 + (y1 - y2) * (clip_x_min - x2)
				/ (x1 - x2);			//lint !e414
			x2 = clip_x_min;
			c2 = encode(x2, y2);
			continue;
		}
		if (c2 & XHI)
		{
			//
			// No divide by zero possibility, because (x1 != x2)
			// otherwise (c1 & c2) would have been true.
			//
			y2 = y2 + (y1 - y2) * (clip_x_max - x2)
				/ (x1 - x2);			//lint !e414
			x2 = clip_x_max;
			c2 = encode(x2, y2);
			continue;
		}
		if (c2 & YLO)
		{
			//
			// No divide by zero possibility, because (y1 != y2)
			// otherwise (c1 & c2) would have been true.
			//
			x2 = x2 + (x1 - x2) * (clip_y_min - y2)
				/ (y1 - y2);			//lint !e414
			y2 = clip_y_min;
			c2 = encode(x2, y2);
			continue;
		}
		if (c2 & YHI)
		{
			//
			// No divide by zero possibility, because (y1 != y2)
			// otherwise (c1 & c2) would have been true.
			//
			x2 = x2 + (x1 - x2) * (clip_y_max - y2)
				/ (y1 - y2);			//lint !e414
			y2 = clip_y_max;
			c2 = encode(x2, y2);
			continue;
		}
	}

	//
	// round the clipped values
	//
	x1 = (int)(0.5 + dx1);
	y1 = (int)(0.5 + dy1);
	x2 = (int)(0.5 + dx2);
	y2 = (int)(0.5 + dy2);

	//
	// draw the line
	//
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	if (dx > dy)
	{
		if (x1 > x2)
		{
			int	hold;

			hold = x1;
			x1 = x2;
			x2 = hold;
			hold = y1;
			y1 = y2;
			y2 = hold;
		}
		dirn = (y1 < y2) ? 1 : -1;
		e = dx/2;
		do
		{
			gif_pixel_set(gp, x1, y1, clr);
			e -= dy;
			if (e < 0)
			{
				e += dx;
				y1 += dirn;
			}
			x1++;
		}
		while
			(x1 <= x2);
	}
	else
	{
		if (y1 > y2)
		{
			int	hold;

			hold = y1;
			y1 = y2;
			y2 = hold;
			hold = x1;
			x1 = x2;
			x2 = hold;
		}
		dirn = (x1 < x2) ? 1 : -1;
		e = dy/2;
		do
		{
			gif_pixel_set(gp, x1, y1, clr);
			e -= dx;
			if (e < 0)
			{
				e += dy;
				x1 += dirn;
			}
			y1++;
		}
		while
			(y1 <= y2);
	}
}
