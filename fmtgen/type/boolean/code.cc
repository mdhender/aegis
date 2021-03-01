//
// aegis - project change supervisor
// Copyright (C) 2004-2008 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <fmtgen/type/boolean/code.h>


type_boolean_code::~type_boolean_code()
{
}


type_boolean_code::type_boolean_code(generator *a_gen) :
    type_boolean(a_gen)
{
}


type::pointer
type_boolean_code::create(generator *gen)
{
    return pointer(new type_boolean_code(gen));
}


void
type_boolean_code::gen_body()
    const
{
    if (is_in_include_file())
        return;
    include_once("libaegis/io.h"); // for boolean_write
}


void
type_boolean_code::gen_declarator(const nstring &variable_name, bool is_a_list,
    int attributes, const nstring &) const
{
    int show = 1;
    if (attributes & (ATTRIBUTE_SHOW_IF_DEFAULT | ATTRIBUTE_HIDE_IF_DEFAULT))
        show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    printf("boolean_write(fp, ");
    if (is_a_list)
    {
        printf("(const char *)0");
        show = 1;
    }
    else
    {
        printf("\"%s\"", variable_name.c_str());
    }
    printf(", this_thing->%s, %d);\n", variable_name.c_str(), show);
}


void
type_boolean_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int attributes) const
{
    int show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    printf
    (
        "boolean_write_xml(fp, \"%s\", this_thing->%s, %d);\n",
        form_name.c_str(),
        member_name.c_str(),
        show
    );
}


void
type_boolean_code::gen_free_declarator(const nstring &, bool is_a_list)
    const
{
    if (is_a_list)
        printf(";\n");
}


void
type_boolean_code::gen_copy(const nstring &member_name)
    const
{
    printf
    (
        "result->%s = this_thing->%s;\n",
        member_name.c_str(),
        member_name.c_str()
    );
}


void
type_boolean_code::gen_trace(const nstring &vname, const nstring &value)
    const
{
    printf
    (
        "trace_bool_real(\"%s\", %s);\n",
        vname.c_str(),
        value.c_str()
    );
}
