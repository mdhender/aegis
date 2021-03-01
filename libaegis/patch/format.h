//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_PATCH_FORMAT_H
#define LIBAEGIS_PATCH_FORMAT_H

#include <libaegis/patch.h>

struct patch_context_ty; // forward

struct patch_format_ty
{
    const char *name;
    patch_ty *(*header)(struct patch_context_ty *);
    patch_hunk_ty *(*hunk)(struct patch_context_ty *);
};

#endif // LIBAEGIS_PATCH_FORMAT_H
