//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004 Peter Miller;
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
// MANIFEST: interface definition for aeimport/format/private.c
//

#ifndef AEIMPORT_FORMAT_PRIVATE_H
#define AEIMPORT_FORMAT_PRIVATE_H

#include <format.h>

struct format_vtable_ty
{
    size_t          size;
    void            (*destructor)(format_ty *);
    int             (*is_a_candidate)(format_ty *, string_ty *);
    string_ty       *(*sanitize)(format_ty *, string_ty *);
    struct format_version_ty *(*read_versions)(format_ty *, string_ty *,
	                string_ty *);
    string_ty       *(*history_put)(format_ty *);
    string_ty       *(*history_get)(format_ty *);
    string_ty       *(*history_query)(format_ty *);
    string_ty       *(*diff)(format_ty *);
    string_ty       *(*merge)(format_ty *);
    void            (*unlock)(format_ty *, string_ty *);
};

format_ty *format_new(format_vtable_ty *);

#endif // AEIMPORT_FORMAT_PRIVATE_H
