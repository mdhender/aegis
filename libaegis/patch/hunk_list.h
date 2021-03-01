/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/patch/hunk_list.c
 */

#ifndef LIBAEGIS_PATCH_HUNK_LIST_H
#define LIBAEGIS_PATCH_HUNK_LIST_H

#include <patch/hunk.h>

typedef struct patch_hunk_list_ty patch_hunk_list_ty;
struct patch_hunk_list_ty
{
	size_t		length;
	size_t		maximum;
	patch_hunk_ty	**item;
};

void patch_hunk_list_constructor _((patch_hunk_list_ty *));
void patch_hunk_list_destructor _((patch_hunk_list_ty *));
void patch_hunk_list_append _((patch_hunk_list_ty *, patch_hunk_ty *));

#endif /* LIBAEGIS_PATCH_HUNK_LIST_H */