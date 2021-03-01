//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_PATCH_LINE_LIST_H
#define LIBAEGIS_PATCH_LINE_LIST_H

#include <libaegis/patch/line.h>

struct patch_line_list_ty
{
	int		start_line_number;

	size_t		length;
	size_t		maximum;
	patch_line_ty	*item;
};

void patch_line_list_constructor(patch_line_list_ty *);
void patch_line_list_destructor(patch_line_list_ty *);
void patch_line_list_append(patch_line_list_ty *, patch_line_type,
	string_ty *);

#endif // LIBAEGIS_PATCH_LINE_LIST_H
