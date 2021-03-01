//
//	aegis - project change supervisor
//	Copyright (C) 1993, 1994, 1998, 1999, 2002-2008 Peter Miller
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
#include <fmtgen/type/time.h>


type_time::~type_time()
{
}


type_time::type_time() :
    type("time")
{
}


void
type_time::gen_include_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    const char *deref = (is_a_list ? "*" : "");
    indent_printf("%s\1%s%s;\n", "time_t", deref, variable_name.c_str());
}


void
type_time::gen_code_declarator(const nstring &variable_name, bool is_a_list,
    int attributes) const
{
    int show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    indent_printf("time_write(fp, ");
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
type_time::gen_code_call_xml(const nstring &form_name,
    const nstring &member_name, int attributes) const
{
    int show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    indent_printf
    (
	"time_write_xml(fp, \"%s\", this_thing->%s, %d);\n",
	form_name.c_str(),
	member_name.c_str(),
	show
    );
}


void
type_time::gen_free_declarator(const nstring &, bool is_a_list)
    const
{
    if (is_a_list)
       	indent_printf(";\n");
}


nstring
type_time::c_name_inner()
    const
{
    return "time_t";
}


bool
type_time::has_a_mask()
    const
{
    return true;
}


void
type_time::gen_code_copy(const nstring &member_name)
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
type_time::gen_code_trace(const nstring &vname, const nstring &value)
    const
{
    indent_printf
    (
	"trace_time_real(\"%s\", %s);\n",
	vname.c_str(),
	value.c_str()
    );
}
