/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aeimport/format.c
 */

#ifndef AEIMPORT_FORMAT_H
#define AEIMPORT_FORMAT_H

#include <ac/time.h>

#include <str.h>

/*
 * This is the base class for deriving all format classes.
 */
typedef struct format_ty format_ty;
struct format_ty
{
    struct format_vtable_ty *vptr;
};

void format_delete(format_ty *);
struct format_search_list_ty *format_search(format_ty *, string_ty *);

/*
 * Fetch strings suitable for placing in the project "config" file.
 */
string_ty *format_history_get(format_ty *);
string_ty *format_history_put(format_ty *);
string_ty *format_history_query(format_ty *);
string_ty *format_diff(format_ty *);
string_ty *format_merge(format_ty *);
void format_unlock(format_ty *, string_ty *);

format_ty *format_find(const char *);

#endif /* AEIMPORT_FORMAT_H */
