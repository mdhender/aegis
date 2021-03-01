//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2000, 2002-2006, 2008 Peter Miller
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

#include <common/ac/string.h>

#include <aeget/rect.h>
#include <libaegis/gif.h>


static	int	color[3]	= { -1, -1, -1 };
static	int	bevel		= -1;
static	int	mime		= -1;


void
rect_color(int r, int g, int b)
{
    if (r < 0)
	r = 0;
    if (r > 255)
	r = 255;
    if (g < 0)
	g = 0;
    if (g > 255)
	g = 255;
    if (b < 0)
	b = 0;
    if (b > 255)
	b = 255;
    color[0] = r;
    color[1] = g;
    color[2] = b;
}


static void
rect_color_default(int width, int height)
{
    int             n, n1, n2;
    int             r, g, b;

    r = g = b = 0;
    n = ((width + height) * 107) % (6 * 256);
    n1 = n / 256;
    n2 = n % 256;
    switch (n1)
    {
    case 0:
	r = 255;
	g = n2;
	b = 0;
	break;

    case 1:
	r = 255 - n2;
	g = 255;
	b = 0;
	break;

    case 2:
	r = 0;
	g = 255;
	b = n2;
	break;

    case 3:
	r = 0;
	g = 255 - n2;
	b = 255;
	break;

    case 4:
	r = n2;
	g = 0;
	b = 255;
	break;

    case 5:
	r = 255;
	g = 0;
	b = 255 - n2;
	break;
    }
    rect_color(r, g, b);
}


void
rect_bevel(int n)
{
    if (n < 0)
	n = 0;
    if (n > 10)
	n = 10;
    bevel = n;
}


static void
rect_bevel_default(int width, int height)
{
    int             n;

    n = (width < height ? width : height) / 10 + 1;
    if (n > 3)
	n = 3;
    rect_bevel(n);
}


void
rect_mime(int n)
{
    mime = !!n;
}


void
rect(const char *fn, int width, int height, const char *label)
{
    gif_ty          *fp;
    int             j;

    //
    // correct the size
    //
    if (width < 3)
	width = 3;
    if (height < 3)
	height = 3;

    //
    // generate a random color,
    // if the color was not set
    //
    if (color[0] < 0 && color[1] < 0 && color[2] < 0)
	rect_color_default(width, height);

    //
    // open the file
    //
    fp = gif_create(fn, width, height);
    if (mime)
	gif_mime(fp);

    //
    // there are two forms,
    // the 3D beveled form,
    // and the black outline form
    //
    if (bevel < 0)
	rect_bevel_default(width, height);
    if (bevel * 2 >= width)
	bevel = width / 3;
    if (bevel * 2 >= height)
	bevel = height / 3;

    if (bevel)
    {
	gif_colormap_set
	(
	    fp,
	    1,
	    (color[0] + 2 * 255) / 3,
	    (color[1] + 2 * 255) / 3,
	    (color[2] + 2 * 255) / 3
	);
	gif_colormap_set
	(
	    fp,
	    2,
	    color[0],
	    color[1],
	    color[2]
	);
	gif_colormap_set
	(
	    fp,
	    3,
	    color[0] * 2 / 3,
	    color[1] * 2 / 3,
	    color[2] * 2 / 3
	);

	for (j = 0; j < bevel; ++j)
	{
	    // top
	    gif_line(fp, j, j, width - 1 - j, j, 1);
	    // left
	    gif_line(fp, j, j, j, height - 1 - j, 1);
	    // bottom
	    gif_line
	    (
		fp,
		j + 1, height - 1 - j,
		width - 1 - j, height - 1 - j,
		3
	    );
	    // right
	    gif_line
	    (
		fp,
		width - 1 - j, j + 1,
		width - 1 - j, height - 1 - j,
		3
	    );
	}
	gif_rect(fp, bevel, bevel, width - bevel, height - bevel, 2);
    }
    else
    {
	gif_colormap_set
	(
	    fp,
	    1,
	    (color[0] < 128 ? 255 : 0),
	    (color[1] < 128 ? 255 : 0),
	    (color[1] < 128 ? 255 : 0)
	);
	gif_colormap_set
	(
	    fp,
	    2,
	    color[0],
	    color[1],
	    color[2]
	);

	gif_line(fp, 0, 0, width, 0, 1);
	gif_line(fp, width, 0, width, height, 1);
	gif_line(fp, width, height - 1, 0, height - 1, 1);
	gif_line(fp, 0, height - 1, 0, 0, 1);
	gif_rect(fp, 1, 1, width - 1, height - 1, 2);
    }

    if (label)
    {
	int gray = 255;
	int x, y;
	if (color[0] * 2 + color[1] * 5 + color[2] > 4*255)
    	    gray = 0;
	gif_colormap_set(fp, 4, gray, gray, gray);
	x = (width - 7 * (int)strlen(label)) / 2;
	y = (height + 8) / 2;
	gif_text(fp, x, y, label, 4);
    }

    //
    // finish up
    //
    gif_close(fp);
}
