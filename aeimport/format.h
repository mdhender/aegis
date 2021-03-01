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

#ifndef AEIMPORT_FORMAT_H
#define AEIMPORT_FORMAT_H

#include <common/ac/time.h>

#include <common/str.h>

//
// This is the base class for deriving all format classes.
//
struct format_ty
{
    struct format_vtable_ty *vptr;
};

void format_delete(format_ty *);
struct format_search_list_ty *format_search(format_ty *, string_ty *);

//
// Fetch strings suitable for placing in the project "config" file.
//
string_ty *format_history_get(format_ty *);
string_ty *format_history_put(format_ty *);
string_ty *format_history_query(format_ty *);
string_ty *format_diff(format_ty *);
string_ty *format_merge(format_ty *);
void format_unlock(format_ty *, string_ty *);
string_ty *format_sanitize(format_ty *, string_ty *, int);

format_ty *format_find(const char *);

#endif // AEIMPORT_FORMAT_H
