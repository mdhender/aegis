/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: interface definition for common/str.c
 */

#ifndef STR_H
#define STR_H

#include <ac/stddef.h>
#include <ac/stdarg.h>
#include <main.h>

typedef unsigned long str_hash_ty;

typedef struct string_ty string_ty;
struct string_ty
{
	str_hash_ty	str_hash;
	string_ty	*str_next;
	long		str_references;
	size_t		str_length;
	char		str_text[1];
};

void str_initialize _((void));
string_ty *str_from_c _((char *));
string_ty *str_n_from_c _((char *, size_t));
string_ty *str_copy _((string_ty *));
void str_free _((string_ty *));
string_ty *str_catenate _((string_ty *, string_ty *));
string_ty *str_cat_three _((string_ty *, string_ty *, string_ty *));
int str_bool _((string_ty *));
string_ty *str_upcase _((string_ty *));
string_ty *str_downcase _((string_ty *));
void str_dump _((void));
string_ty *str_field _((string_ty *str, int sep, int fldnum));
void slow_to_fast _((char **, string_ty **, size_t));
string_ty *str_format _((char *, ...));
string_ty *str_vformat _((char *, va_list));

#define str_equal(s1, s2) ((s1) == (s2))

#endif /* STR_H */
