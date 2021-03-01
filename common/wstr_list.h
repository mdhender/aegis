//
//	aegis - project change supervisor
//	Copyright (C) 1995, 2002-2004 Peter Miller;
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
// MANIFEST: interface definition for common/wstr_list.c
//

#ifndef COMMON_WSTR_LIST_H
#define COMMON_WSTR_LIST_H

#include <wstr.h>

/** \addtogroup WStringList
  * \brief Lists of wide char strings
  * \ingroup WString
  * @{
  */

struct wstring_list_ty
{
	size_t		nitems;
	size_t		nitems_max;
	wstring_ty	**item;
};

int wstring_list_member(wstring_list_ty *, wstring_ty *);
wstring_ty *wstring_list_to_wstring(wstring_list_ty *, size_t, size_t,
    const char *);
void wstring_to_wstring_list(wstring_list_ty *, wstring_ty *, const char *,
    int);
void wstring_list_prepend(wstring_list_ty *, wstring_ty *);
void wstring_list_append(wstring_list_ty *, wstring_ty *);
void wstring_list_append_unique(wstring_list_ty *, wstring_ty *);
void wstring_list_copy(wstring_list_ty *, wstring_list_ty *);
void wstring_list_delete(wstring_list_ty *, wstring_ty *);
void wstring_list_destructor(wstring_list_ty *);
void wstring_list_constructor(wstring_list_ty *);

int wstring_list_equal(wstring_list_ty *, wstring_list_ty *);
int wstring_list_subset(wstring_list_ty *, wstring_list_ty *);

/** @} */
#endif // COMMON_WSTR_LIST_H
