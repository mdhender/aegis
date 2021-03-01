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
 * MANIFEST: functions to manipulate data types
 */

#include <error.h>
#include <mem.h>
#include <trace.h>
#include <type.h>


void
type_gen_include_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	trace(("type_gen_include_declarator(type = %08lX, name = \"%s\", \
is_a_list = %d)\n{\n"/*}*/, (long)type, name->str_text, is_a_list));
	if (type->method->gen_include_declarator)
		type->method->gen_include_declarator(type, name, is_a_list);
	trace((/*{*/"}\n"));
}


void
type_gen_include(type)
	type_ty		*type;
{
	trace(("type_gen_include(type = %08lX)\n{\n"/*}*/, (long)type));
	if (type->method->gen_include)
		type->method->gen_include(type);
	trace((/*{*/"}\n"));
}


void
type_gen_code_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	trace(("type_gen_code_declarator(type = %08lX, name = \"%s\", \
is_a_list = %d)\n{\n"/*}*/, (long)type, name->str_text, is_a_list));
	if (type->method->gen_code_declarator)
		type->method->gen_code_declarator(type, name, is_a_list);
	trace((/*{*/"}\n"));
}


void
type_gen_code(type)
	type_ty		*type;
{
	trace(("type_gen_code(type = %08lX)\n{\n"/*}*/, (long)type));
	if (type->method->gen_code)
		type->method->gen_code(type);
	trace((/*{*/"}\n"));
}


void
type_gen_free_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	trace(("type_gen_free_declarator(type = %08lX, name = \"%s\", \
is_a_list = %d)\n{\n"/*}*/, (long)type, name->str_text, is_a_list));
	if (type->method->gen_free_declarator)
		type->method->gen_free_declarator(type, name, is_a_list);
	trace((/*{*/"}\n"));
}


void
type_member_add(type, member_name, member_type)
	type_ty		*type;
	string_ty	*member_name;
	type_ty		*member_type;
{
	trace(("type_member_add(type = %08lX, member_name = %08lX, member_type \
= %08lX)\n{\n"/*}*/, (long)type, (long)member_name, (long)member_type));
	assert(type->method->member_add);
	if (type->method->member_add)
		type->method->member_add(type, member_name, member_type);
	trace((/*{*/"}\n"));
}


type_ty *
type_new(method, name)
	type_method_ty	*method;
	string_ty	*name;
{
	type_ty		*type;

	trace(("type_new(method = %08lX, bane = %08lX)\n{\n"/*}*/, (long)method,
		(long)name));
	type = mem_alloc(method->size);
	type->method = method;
	type->name = name ? str_copy(name) : str_from_c(method->name);
	trace_string(type->name->str_text);
	type->is_a_typedef = 0;
	type->included_flag = 0;
	if (method->constructor)
		method->constructor(type);
	trace(("return %08lX;\n", (long)type));
	trace((/*{*/"}\n"));
	return type;
}


void
type_delete(type)
	type_ty	*type;
{
	trace(("type_delete(type = %08lX)\n{\n"/*}*/, (long)type));
	if (type->method->destructor)
		type->method->destructor(type);
	str_free(type->name);
	mem_free(type);
	trace((/*{*/"}\n"));
}
