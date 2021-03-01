/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for libaegis/gif .c files
 */

#ifndef LIBAEGIS_GIF_H
#define LIBAEGIS_GIF_H

#include <main.h>

enum gif_mode_ty
{
	gif_mode_rdonly,
	gif_mode_rdwr
};
typedef enum gif_mode_ty gif_mode_ty;

typedef struct gif_ty gif_ty;
struct gif_ty
{
	char		*fn;
	int		width, height;
	unsigned char	colormap[256][3];
	unsigned char	*image_flat;
	unsigned char	**image;
	gif_mode_ty	mode;
	int		mime;
};

gif_ty *gif_open _((char *path, int mode));
void gif_close _((gif_ty *));
gif_ty *gif_create _((char *path, int size_x, int size_y));
void gif_rename _((gif_ty *, char *));
int gif_pixel_get _((gif_ty *gp, int x, int y));
void gif_pixel_set _((gif_ty *gp, int x, int y, int color));
void gif_colormap_get _((gif_ty *, int, int *, int *, int *));
void gif_colormap_set _((gif_ty *, int, int, int, int));
void gif_mime _((gif_ty *));

void gif_line _((gif_ty *gp, int x1, int y1, int x2, int y2, int color));
void gif_rect _((gif_ty *gp, int x1, int y1, int x2, int y2, int color));
void gif_text _((gif_ty *gp, int x, int y, char *text, int color));

#endif /* LIBAEGIS_GIF_H */
