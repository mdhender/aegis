//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <fmtgen/type/string/code.h>


type_string_code::~type_string_code()
{
}


type_string_code::type_string_code(generator *a_gen) :
    type_string(a_gen)
{
}


type::pointer
type_string_code::create(generator *a_gen)
{
    return pointer(new type_string_code(a_gen));
}


void
type_string_code::gen_body()
    const
{
    include_once("libaegis/io.h"); // for string_write
}


void
type_string_code::gen_declarator(const nstring &variable_name, bool is_a_list,
    int, const nstring &) const
{
    printf("string_write(fp, ");
    if (is_a_list)
        printf("(const char *)0");
    else
        printf("\"%s\"", variable_name.c_str());
    printf(", this_thing->%s);\n", variable_name.c_str());
}


void
type_string_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int) const
{
    printf
    (
        "string_write_xml(fp, \"%s\", this_thing->%s);\n",
        form_name.c_str(),
        member_name.c_str()
    );
}


void
type_string_code::gen_copy(const nstring &member_name)
    const
{
    printf
    (
        "result->%s = str_copy(this_thing->%s);\n",
        member_name.c_str(),
        member_name.c_str()
    );
}


void
type_string_code::gen_free_declarator(const nstring &variable_name, bool)
    const
{
    printf("str_free(this_thing->%s);\n", variable_name.c_str());
}


void
type_string_code::gen_trace(const nstring &vname, const nstring &value)
    const
{
    printf
    (
        "trace_string_real(\"%s\", %s);\n",
        vname.c_str(),
        value.c_str()
    );
}
