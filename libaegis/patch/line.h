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

#ifndef LIBAEGIS_PATCH_LINE_H
#define LIBAEGIS_PATCH_LINE_H

#include <common/str.h>

enum patch_line_type
{
	patch_line_type_unchanged,
	patch_line_type_changed,
	patch_line_type_inserted,
	patch_line_type_deleted
};

struct patch_line_ty
{
	patch_line_type	type;
	string_ty	*value;
};

void patch_line_constructor(patch_line_ty *, patch_line_type, string_ty *);
void patch_line_destructor(patch_line_ty *);

#endif // LIBAEGIS_PATCH_LINE_H
