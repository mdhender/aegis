//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_PATCH_LIST_H
#define LIBAEGIS_PATCH_LIST_H

#include <libaegis/patch.h>

struct patch_list_ty
{
    string_ty       *project_name;
    long            change_number;
    string_ty       *brief_description;
    string_ty       *description;
    string_ty       *comment;

    size_t          length;
    size_t          maximum;
    patch_ty        **item;
};

patch_list_ty *patch_list_new(void);
void patch_list_delete(patch_list_ty *);
void patch_list_append(patch_list_ty *, patch_ty *);
patch_list_ty *patch_read(input &, int);

#endif // LIBAEGIS_PATCH_LIST_H
