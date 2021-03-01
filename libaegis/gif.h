//
// aegis - project change supervisor
// Copyright (C) 1997, 2002-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_GIF_H
#define LIBAEGIS_GIF_H

enum gif_mode_ty
{
    gif_mode_rdonly,
    gif_mode_rdwr
};

struct gif_ty
{
    char            *fn;
    int             width;
    int             height;
    unsigned char   colormap[256][3];
    unsigned char   *image_flat;
    unsigned char   **image;
    gif_mode_ty     mode;
    int             mime;
};

gif_ty *gif_open(const char *path, int mode);
void gif_close(gif_ty *);
gif_ty *gif_create(const char *path, int size_x, int size_y);
void gif_rename(gif_ty *, const char *);
int gif_pixel_get(gif_ty *gp, int x, int y);
void gif_pixel_set(gif_ty *gp, int x, int y, int color);
void gif_colormap_get(gif_ty *, int, int *, int *, int *);
void gif_colormap_set(gif_ty *, int, int, int, int);
void gif_mime(gif_ty *);

void gif_line(gif_ty *gp, int x1, int y1, int x2, int y2, int color);
void gif_rect(gif_ty *gp, int x1, int y1, int x2, int y2, int color);
void gif_text(gif_ty *gp, int x, int y, const char *text, int color);

#endif // LIBAEGIS_GIF_H
// vim: set ts=8 sw=4 et :
