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

#include <common/boolean.h>
#include <common/trace.h>

#include <fmtgen/type/integer/introsp_code.h>


type_integer_introspector_code::~type_integer_introspector_code()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_integer_introspector_code::type_integer_introspector_code(
        generator *a_gen) :
    type_integer(a_gen)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_integer_introspector_code::create(generator *a_gen)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new type_integer_introspector_code(a_gen));
}


void
type_integer_introspector_code::gen_body()
    const
{
    include_once("common/trace.h"); // for trace_long_real
    include_once("libaegis/introspector/integer.h");
}


void
type_integer_introspector_code::gen_declarator(const nstring &variable_name,
    bool is_a_list, int attributes, const nstring &) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    bool show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    printf("introspector_integer::write(fp, ");
    if (is_a_list)
    {
        printf("nstring()");
        show = true;
    }
    else
    {
        printf("%s", variable_name.quote_c().c_str());
    }
    printf(", %s, %s);\n", variable_name.c_str(), bool_to_string(show).c_str());
}


void
type_integer_introspector_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int attributes) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    bool show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    nstring text =
        (
            "introspector_integer::write_xml(fp, "
        +
            form_name.quote_c()
        +
            ", "
        +
            member_name
        +
            ", "
        +
            bool_to_string(show)
        +
            ");"
        );
    wrap_and_print("", text);
}


void
type_integer_introspector_code::gen_free_declarator(const nstring &,
    bool is_a_list) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (is_a_list)
        printf(";\n");
}


void
type_integer_introspector_code::gen_copy(const nstring &member_name)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf
    (
        "result->%s = this_thing->%s;\n",
        member_name.c_str(),
        member_name.c_str()
    );
}


void
type_integer_introspector_code::gen_trace(const nstring &vname,
    const nstring &value) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf
    (
        "trace_long_real(\"%s\", &%s);\n",
        vname.c_str(),
        value.c_str()
    );
}


void
type_integer_introspector_code::gen_default_constructor(const nstring &instvar)
{
    printf("%s(0)", instvar.c_str());
}


void
type_integer_introspector_code::gen_methods(const nstring &class_name,
    const nstring &inst_var_name, int) const
{
    include_once("libaegis/zero.h");

    printf("\n");
    printf("long\n");
    printf("%s::%s_get()\n", class_name.c_str(), inst_var_name.c_str());
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("return magic_zero_decode(%s);\n", inst_var_name.c_str());
    printf("}\n");

    printf("\n");
    printf("bool\n");
    printf("%s::%s_is_set()\n", class_name.c_str(), inst_var_name.c_str());
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("return (%s != 0);\n", inst_var_name.c_str());
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("%s::%s_set(long n)\n", class_name.c_str(), inst_var_name.c_str());
    printf("{\n");
    printf("%s = magic_zero_encode(n);\n", inst_var_name.c_str());
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("%s::%s_clear()\n", class_name.c_str(), inst_var_name.c_str());
    printf("{\n");
    printf("%s = 0;\n", inst_var_name.c_str());
    printf("}\n");

    include_once("libaegis/introspector/integer.h");
    printf("\n");
    printf("introspector::pointer\n");
    printf
    (
        "%s::%s_introspector_factory()\n",
        class_name.c_str(),
        inst_var_name.c_str()
    );
    printf("{\n");
    printf("return introspector_integer::create(%s);\n", inst_var_name.c_str());
    printf("}\n");
}


void
type_integer_introspector_code::gen_write(const nstring &form_name,
    const nstring &member_name, bool show_default) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring text =
        (
            "introspector_integer::write(fp, "
        +
            form_name.quote_c()
        +
            ", "
        +
            member_name
        +
            ", "
        +
            bool_to_string(show_default)
        +
            ");"
        );
    wrap_and_print("", text);
}
