/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: interface definition for fmtgen/type.c
 */

#ifndef TYPE_H
#define TYPE_H

#include <main.h>
#include <lex.h>
#include <str.h>

enum type_class_ty
{
	type_class_enum,
	type_class_integer,
	type_class_list,
	type_class_string,
	type_class_struct,
	type_class_ref
};
typedef enum type_class_ty type_class_ty;

#define TYPE_T \
	type_class_ty	class; \
	struct type_method_ty *method; \
	string_ty	*name; \
	int		included_flag;

typedef struct type_ty type_ty;
struct type_ty
{
	TYPE_T
};

typedef struct type_method_ty type_method_ty;
struct type_method_ty
{
	void (*gen_include)_((type_ty *, string_ty *));
	void (*gen_include_declarator)_((type_ty *, string_ty *, int));
	void (*gen_code)_((type_ty *, string_ty *));
	void (*gen_code_declarator)_((type_ty *, string_ty *, int));
	void (*gen_free_declarator)_((type_ty *, string_ty *, int));
};

void type_gen_include _((type_ty *, string_ty *));
void type_gen_include_declarator _((type_ty *, string_ty *, int));
void type_gen_code _((type_ty *, string_ty *));
void type_gen_code_declarator _((type_ty *, string_ty *, int));
void type_gen_free_declarator _((type_ty *, string_ty *, int));
type_ty *type_create_string _((void));
type_ty *type_create_integer _((void));
type_ty *type_create_ref _((string_ty *name, string_ty *ref));
type_ty *type_create_list _((string_ty *name, type_ty *subtype));
type_ty *type_create_struct _((string_ty *name, parse_list_ty *contents, int n));
type_ty *type_create_enum _((string_ty *name, parse_list_ty *plp));

#endif /* TYPE_H */
