/*
 *	aegis - project change supervisor
 *	Copyright (C) 1992, 1993, 1995, 1997, 1999 Peter Miller;
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
 * MANIFEST: interface definition for aegis/output/pager.c
 */

#ifndef OUTPUT_PAGER_H
#define OUTPUT_PAGER_H

#include <output.h>

void option_pager_set _((int, void(*)(void)));

output_ty *output_pager_open _((void));

#endif /* OUTPUT_PAGER_H */