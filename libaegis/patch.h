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
 * MANIFEST: interface definition for libaegis/patch/
 */

#ifndef LIBAEGIS_PATCH_H
#define LIBAEGIS_PATCH_H

#include <patch/hunk_list.h>
#include <str_list.h>

typedef struct patch_ty patch_ty;
struct patch_ty
{
	string_list_ty	name;
	int		action;
	int		usage;
	patch_hunk_list_ty actions;
};

typedef struct patch_list_ty patch_list_ty;
struct patch_list_ty
{
	string_ty	*project_name;
	long		change_number;
	string_ty	*brief_description;
	string_ty	*description;

	size_t		length;
	size_t		maximum;
	patch_ty	**item;
};

struct input_ty; /* forward */
patch_list_ty *patch_read _((struct input_ty *));
void patch_apply _((patch_ty *, string_ty *, string_ty *));

#endif /* LIBAEGIS_PATCH_H */
