//
//	aegis - project change supervisor
//	Copyright (C) 1998, 2002-2006, 2008 Peter Miller
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

#include <libaegis/gif.h>
#include <libaegis/gif/private.h>
#include <common/mem.h>


gif_ty *
gif_create(const char *path, int width, int height)
{
    gif_ty          *result;
    int             j;
    size_t          nbytes;

    //
    // initialize things
    //
    result = (gif_ty *)mem_alloc(sizeof(gif_ty));
    for (j = 0; j < 256; ++j)
    {
	result->colormap[j][0] = j;
	result->colormap[j][1] = j;
	result->colormap[j][2] = j;
    }

    result->fn = path ? mem_copy_string(path) : (char *)0;
    if (width < 1)
	width = 1;
    else if (width > SIZE_MAX)
	width = SIZE_MAX;
    result->width = width;
    if (height < 1)
	height = 1;
    else if (height > SIZE_MAX)
	height = SIZE_MAX;
    result->height = height;
    result->mode = gif_mode_rdwr;
    result->mime = 0;

    //
    // allocate the image
    //
    nbytes = (size_t)result->width * (size_t)result->height;
    result->image_flat = (unsigned char *)mem_alloc(nbytes);
    memset(result->image_flat, 0, nbytes);
    result->image = (unsigned char **)mem_alloc(
        (size_t)result->height * sizeof(unsigned char *));
    for (j = 0; j < result->height; ++j)
	result->image[j] = result->image_flat + j * result->width;

    //
    // all done
    //
    return result;
}
