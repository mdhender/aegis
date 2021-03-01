/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/col/unformatted.c
 */

#ifndef LIBAEGIS_COL_UNFORMATTED_H
#define LIBAEGIS_COL_UNFORMATTED_H

#include <col.h>

struct wide_output_ty; /* existence */

col_ty *col_unformatted_open(struct wide_output_ty *, int);

#endif /* LIBAEGIS_COL_UNFORMATTED_H */
