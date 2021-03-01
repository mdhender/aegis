//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1998, 1999, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <fmtgen/indent.h>
#include <common/mem.h>
#include <fmtgen/type/string.h>


type_string::~type_string()
{
}


type_string::type_string() :
    type("string")
{
}


void
type_string::gen_include_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    const char *deref = (is_a_list ? "*" : "");
    indent_printf("%s\1%s*%s;\n", "string_ty", deref, variable_name.c_str());
}


void
type_string::gen_code_declarator(const nstring &variable_name,
    bool is_a_list, int) const
{
    indent_printf("string_write(fp, ");
    if (is_a_list)
       	indent_printf("(const char *)0");
    else
	indent_printf("\"%s\"", variable_name.c_str());
    indent_printf(", this_thing->%s);\n", variable_name.c_str());
}


void
type_string::gen_code_call_xml(const nstring &form_name,
    const nstring &member_name, int) const
{
    indent_printf
    (
	"string_write_xml(fp, \"%s\", this_thing->%s);\n",
	form_name.c_str(),
	member_name.c_str()
    );
}


void
type_string::gen_code_copy(const nstring &member_name)
    const
{
    indent_printf
    (
	"result->%s = str_copy(this_thing->%s);\n",
	member_name.c_str(),
	member_name.c_str()
    );
}


void
type_string::gen_free_declarator(const nstring &variable_name, bool)
    const
{
    indent_printf("str_free(this_thing->%s);\n", variable_name.c_str());
}


nstring
type_string::c_name_inner()
    const
{
    return "string_ty *";
}


bool
type_string::has_a_mask()
    const
{
    return false;
}


void
type_string::gen_code_trace(const nstring &vname, const nstring &value)
    const
{
    indent_printf
    (
	"trace_string_real(\"%s\", %s);\n",
	vname.c_str(),
	value.c_str()
    );
}
