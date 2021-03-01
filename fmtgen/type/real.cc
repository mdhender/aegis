//
//	aegis - project change supervisor
//	Copyright (C) 1998, 1999, 2002-2007 Peter Miller
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
// MANIFEST: functions to manipulate real type
//

#include <fmtgen/indent.h>
#include <common/mem.h>
#include <fmtgen/type/real.h>


type_real::~type_real()
{
}


type_real::type_real() :
    type("real")
{
}


void
type_real::gen_include_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    const char *deref = (is_a_list ? "*" : "");
    indent_printf("%s\1%s%s;\n", "double", deref, variable_name.c_str());
}


void
type_real::gen_code_declarator(const nstring &variable_name, bool is_a_list,
    int attributes) const
{
    int show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    indent_printf("real_write(fp, ");
    if (is_a_list)
    {
	indent_printf("(const char *)0");
	show = 1;
    }
    else
    {
	indent_printf("\"%s\"", variable_name.c_str());
    }
    indent_printf(", this_thing->%s, %d);\n", variable_name.c_str(), show);
}


void
type_real::gen_code_call_xml(const nstring &form_name,
    const nstring &member_name, int attributes) const
{
    int show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    indent_printf
    (
	"real_write_xml(fp, \"%s\", this_thing->%s, %d);\n",
	form_name.c_str(),
	member_name.c_str(),
	show
    );
}


void
type_real::gen_free_declarator(const nstring &, bool is_a_list)
    const
{
    if (is_a_list)
       	indent_printf(";\n");
}


nstring
type_real::c_name_inner()
    const
{
    return "double";
}


bool
type_real::has_a_mask()
    const
{
    return true;
}


void
type_real::gen_code_copy(const nstring &member_name)
    const
{
    indent_printf
    (
	"result->%s = this_thing->%s;\n",
	member_name.c_str(),
	member_name.c_str()
    );
}


void
type_real::gen_code_trace(const nstring &vname, const nstring &value)
    const
{
    indent_printf
    (
	"trace_double_real(\"%s\", %s);\n",
	vname.c_str(),
	value.c_str()
    );
}
