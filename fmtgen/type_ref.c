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
 * MANIFEST: functions to manipulate reference types
 */

#include <type.h>
#include <indent.h>
#include <mem.h>
#include <id.h>
#include <error.h>


typedef struct type_ref_ty type_ref_ty;
struct type_ref_ty
{
	/* inherited */
	TYPE_T

	/* instance variables */
	string_ty	*ref;
};


static void gen_include_declarator _((type_ty *, string_ty *, int));

static void
gen_include_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	type_ref_ty	*type2;
	type_ty		*tp;

	type2 = (type_ref_ty *)type;
	if (!id_search(type2->ref, ID_CLASS_TYPE, (long *)&tp))
		fatal("type \"%s\" vanished", type2->ref->str_text);
	type_gen_include_declarator(tp, name, is_a_list);
}


static void gen_code_declarator _((type_ty *, string_ty *, int));

static void
gen_code_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	type_ref_ty	*type2;
	type_ty		*tp;

	type2 = (type_ref_ty *)type;
	if (!id_search(type2->ref, ID_CLASS_TYPE, (long *)&tp))
		fatal("type \"%s\" vanished", type2->ref->str_text);
	type_gen_code_declarator(tp, name, is_a_list);
}


static void gen_free_declarator _((type_ty *, string_ty *, int));

static void
gen_free_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	type_ref_ty	*type2;
	type_ty		*tp;

	type2 = (type_ref_ty *)type;
	if (!id_search(type2->ref, ID_CLASS_TYPE, (long *)&tp))
		fatal("type \"%s\" vanished", type2->ref->str_text);
	type_gen_free_declarator(tp, name, is_a_list);
}


static type_method_ty method =
{
	0, /* gen_include */
	gen_include_declarator,
	0, /* gen_code */
	gen_code_declarator,
	gen_free_declarator,
};


type_ty *
type_create_ref(name, ref)
	string_ty	*name;
	string_ty	*ref;
{
	type_ref_ty	*type;
	type_ty		*tp;

	if (id_search(ref, ID_CLASS_TYPE, (long *)&tp))
		id_assign(name, ID_CLASS_TYPE, (long)tp);
	type = (type_ref_ty *)mem_alloc(sizeof(type_ref_ty));
	type->class = type_class_ref;
	type->method = &method;
	type->name = str_copy(name);
	type->ref = str_copy(ref);
	return (type_ty *)type;
}
