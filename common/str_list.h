/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1996, 1997, 2003 Peter Miller;
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
 * MANIFEST: interface definition for common/str_list.c
 */

#ifndef COMMON_STR_LIST_H
#define COMMON_STR_LIST_H

#include <str.h>

typedef struct string_list_ty string_list_ty;
struct string_list_ty
{
	size_t		nstrings;
	size_t		nstrings_max;
	string_ty	**string;
};

int string_list_member(string_list_ty *, string_ty *);
string_ty *wl2str(string_list_ty *, int, int, const char *);
void str2wl(string_list_ty *, string_ty *, const char *, int);
void string_list_prepend(string_list_ty *, string_ty *);
void string_list_prepend_list(string_list_ty *, string_list_ty *);
void string_list_append(string_list_ty *, string_ty *);
void string_list_append_list(string_list_ty *, string_list_ty *);
void string_list_append_unique(string_list_ty *, string_ty *);
void string_list_append_list_unique(string_list_ty *, string_list_ty *);
void string_list_copy(string_list_ty *, string_list_ty *);
void string_list_remove(string_list_ty *, string_ty *);
void string_list_remove_list(string_list_ty *, string_list_ty *);
void string_list_destructor(string_list_ty *);
void string_list_delete(string_list_ty *);
void string_list_constructor(string_list_ty *);
string_list_ty *string_list_new(void);

int string_list_equal(string_list_ty *, string_list_ty *);
int string_list_subset(string_list_ty *, string_list_ty *);
void string_list_sort(string_list_ty *);

void string_list_quote_shell(string_list_ty *, string_list_ty *);

int string_list_validate(string_list_ty *);

#endif /* COMMON_STR_LIST_H */
