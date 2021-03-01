/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 2001, 2002 Peter Miller;
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

	void *(*alloc)(void);
	void (*free)(void *this);
	int (*enum_parse)(string_ty *name);
	void *(*list_parse)(void *this, type_ty **type_pp);
	void *(*struct_parse)(void *this, string_ty *name, type_ty **type_pp,
		unsigned long *maskp);
	string_ty *(*fuzzy)(string_ty *name);
	struct rpt_value_ty *(*convert)(void *this);
	int (*is_set)(void *);
};

typedef struct generic_struct_ty generic_struct_ty;
struct generic_struct_ty
{
	long		reference_count;
	unsigned long	mask;
};

extern type_ty integer_type;
extern type_ty real_type;
extern type_ty string_type;
extern type_ty time_type;

void *generic_struct_parse(void *, string_ty *, type_ty **, unsigned long *,
	type_table_ty *, size_t);
string_ty *generic_struct_fuzzy(string_ty *, type_table_ty *, size_t);
int generic_struct_is_set(void *);
string_ty *generic_enum_fuzzy(string_ty *, string_ty **, size_t);
struct rpt_value_ty *generic_struct_convert(void *, type_table_ty *,
	size_t);
struct rpt_value_ty *generic_enum_convert(void *, string_ty **, size_t);
int generic_enum_is_set(void *);
void generic_enum__init(char **, size_t);

void type_enum_option_set(void);
void type_enum_option_clear(void);
int type_enum_option_query(void);

#endif /* TYPE_H */
