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
 * MANIFEST: functions to manipulate data types
 */

#include <type.h>


void
type_gen_include_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	if (type->method->gen_include_declarator)
		type->method->gen_include_declarator(type, name, is_a_list);
}


void
type_gen_include(type, name)
	type_ty		*type;
	string_ty 	*name;
{
	if (type->method->gen_include)
		type->method->gen_include(type, name);
}


void
type_gen_code_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	if (type->method->gen_code_declarator)
		type->method->gen_code_declarator(type, name, is_a_list);
}


void
type_gen_code(type, name)
	type_ty		*type;
	string_ty	*name;
{
	if (type->method->gen_code)
		type->method->gen_code(type, name);
}


void
type_gen_free_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	if (type->method->gen_free_declarator)
		type->method->gen_free_declarator(type, name, is_a_list);
}
