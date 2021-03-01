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
 * MANIFEST: interface definition for aeimport/format/search_list.c
 */

#ifndef AEIMPORT_FORMAT_SEARCH_LIST_H
#define AEIMPORT_FORMAT_SEARCH_LIST_H

#include <format/search.h>


typedef struct format_search_list_ty format_search_list_ty;
struct format_search_list_ty
{
	format_search_ty **item;
	size_t		length;
	size_t		maximum;
};

format_search_list_ty *format_search_list_new _((void));
void format_search_list_constructor _((format_search_list_ty *));
void format_search_list_destructor _((format_search_list_ty *));
void format_search_list_delete _((format_search_list_ty *));
void format_search_list_append _((format_search_list_ty *,
	format_search_ty *));

void format_search_list_staff _((format_search_list_ty *,
	struct string_list_ty *));
void format_search_list_validate _((format_search_list_ty *));

#endif /* AEIMPORT_FORMAT_SEARCH_LIST_H */
