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
 * MANIFEST: interface definition for libaegis/patch/format.c
 */

#ifndef LIBAEGIS_PATCH_FORMAT_H
#define LIBAEGIS_PATCH_FORMAT_H

#include <patch.h>

struct patch_context_ty; /* forward */

typedef struct patch_format_ty patch_format_ty;
struct patch_format_ty
{
	char *name;
	patch_ty *(*header)_((struct patch_context_ty *));
	patch_hunk_ty *(*hunk)_((struct patch_context_ty *));
};

#endif /* LIBAEGIS_PATCH_FORMAT_H */