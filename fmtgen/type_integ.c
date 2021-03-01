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
 * MANIFEST: functions to manipulate integer type
 */

#include <type.h>
#include <indent.h>
#include <mem.h>


static void gen_include_declarator _((type_ty *, string_ty *, int));

static void
gen_include_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	char		*deref;

	deref = (is_a_list ? "*" : "");
	indent_printf("%s\1%s%s;\n", "long", deref, name->str_text);
}


static void gen_code_declarator _((type_ty *, string_ty *, int));

static void
gen_code_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	indent_printf("integer_write("/*)*/);
	if (is_a_list)
		indent_printf("(char *)0");
	else
		indent_printf("\"%s\"", name->str_text);
	indent_printf(/*(*/", this->%s);\n", name->str_text);
}


static void gen_free_declarator _((type_ty *, string_ty *, int));

static void
gen_free_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	if (is_a_list)
		indent_printf(";\n");
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
type_create_integer()
{
	type_ty		*type;

	type = (type_ty *)mem_alloc(sizeof(type_ty));
	type->class = type_class_integer;
	type->method = &method;
	type->name = str_from_c("integer");
	return type;
}
