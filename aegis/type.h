/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
 * MANIFEST: interface definition for aegis/type.c
 */

#ifndef TYPE_H
#define TYPE_H

#include <ac/stddef.h>

#include <main.h>
#include <str.h>

typedef struct type_table_ty type_table_ty;
struct type_table_ty
{
	char		*name;
	size_t		offset;
	struct type_ty	*type;
	unsigned long	mask;
	string_ty	*fast_name;
};

typedef struct type_ty type_ty;
struct type_ty
{
	char		*name;

	void *(*alloc)_((void));
	void (*free)_((void *this));
	int (*enum_parse)_((string_ty *name));
	void *(*list_parse)_((void *this, type_ty **type_pp));
	void *(*struct_parse)_((void *this, string_ty *name, type_ty **type_pp,
		unsigned long *maskp));
	string_ty *(*fuzzy)_((string_ty *name));
	struct rpt_value_ty *(*convert)_((void *this));
	int (*is_set)_((void *));
};

typedef struct generic_struct_ty generic_struct_ty;
struct generic_struct_ty
{
	unsigned long	mask;
};

extern type_ty integer_type;
extern type_ty time_type;
extern type_ty string_type;

void *generic_struct_parse _((void *, string_ty *, type_ty **, unsigned long *,
	type_table_ty *, size_t));
string_ty *generic_struct_fuzzy _((string_ty *, type_table_ty *, size_t));
int generic_struct_is_set _((void *));
string_ty *generic_enum_fuzzy _((string_ty *, string_ty **, size_t));
struct rpt_value_ty *generic_struct_convert _((void *, type_table_ty *,
	size_t));
struct rpt_value_ty *generic_enum_convert _((void *, string_ty **, size_t));
int generic_enum_is_set _((void *));
void generic_enum__init _((char **, size_t));

#endif /* TYPE_H */
