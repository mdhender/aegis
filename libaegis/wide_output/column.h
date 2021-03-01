//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2004 Peter Miller;
//	All rights reserved.
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
// MANIFEST: interface definition for libaegis/wide_output/column.c
//

#ifndef LIBAEGIS_WIDE_OUTPUT_COLUMN_H
#define LIBAEGIS_WIDE_OUTPUT_COLUMN_H

#include <wide_output.h>

struct column_row_ty
{
	size_t		length_max;
	size_t		length;
	wchar_t		*text;
	int		printing_width;
};

struct wide_output_ty *wide_output_column_open(int, int);
column_row_ty *wide_output_column_get(struct wide_output_ty *, int);
void wide_output_column_reset(struct wide_output_ty *);

#endif // LIBAEGIS_WIDE_OUTPUT_COLUMN_H
