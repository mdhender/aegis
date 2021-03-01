//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1998, 1999, 2002-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate string type
//

#pragma implementation "type_string_ty"

#include <indent.h>
#include <mem.h>
#include <type/string.h>


type_string_ty::~type_string_ty()
{
}


type_string_ty::type_string_ty() :
    type_ty("string")
{
}


void
type_string_ty::gen_include_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    const char *deref = (is_a_list ? "*" : "");
    indent_printf("%s\1%s*%s;\n", "string_ty", deref, variable_name.c_str());
}


void
type_string_ty::gen_code_declarator(const nstring &variable_name,
    bool is_a_list, int attributes) const
{
    indent_printf("string_write(fp, ");
    if (is_a_list)
       	indent_printf("(const char *)0");
    else
	indent_printf("\"%s\"", variable_name.c_str());
    indent_printf(", this_thing->%s);\n", variable_name.c_str());
}


void
type_string_ty::gen_code_call_xml(const nstring &form_name,
    const nstring &member_name, int attributes) const
{
    indent_printf
    (
	"string_write_xml(fp, \"%s\", this_thing->%s);\n",
	form_name.c_str(),
	member_name.c_str()
    );
}


void
type_string_ty::gen_free_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    indent_printf("str_free(this_thing->%s);\n", variable_name.c_str());
}


nstring
type_string_ty::c_name_inner()
    const
{
    return "string_ty *";
}


bool
type_string_ty::has_a_mask()
    const
{
    return false;
}
