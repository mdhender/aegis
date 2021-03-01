/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998, 2000, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for aerect/rect.c
 */

#ifndef AERECT_RECT_H
#define AERECT_RECT_H

#include <main.h>

void rect(const char *, int, int, const char *);
void rect_color(int, int, int);
void rect_bevel(int);
void rect_mime(int);

#endif /* AERECT_RECT_H */
