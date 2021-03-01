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

#include <fmtgen/type/time/introsp_code.h>


type_time_introspector_code::~type_time_introspector_code()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_time_introspector_code::type_time_introspector_code(
        generator *a_gen) :
    type_time(a_gen)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_time_introspector_code::create(generator *a_gen)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new type_time_introspector_code(a_gen));
}


void
type_time_introspector_code::gen_declarator(const nstring &variable_name,
    bool is_a_list, int attributes, const nstring &) const
{
    bool show = true;
    if (attributes & (ATTRIBUTE_SHOW_IF_DEFAULT | ATTRIBUTE_HIDE_IF_DEFAULT))
        show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    nstring text = "introspector_time::write(fp, ";
    if (is_a_list)
    {
        text += "(const char *)0";
        show = true;
    }
    else
    {
        text += variable_name.quote_c();
    }
    text += ", " + variable_name + ", " + bool_to_string(show) + ");";
    wrap_and_print("", text);
}


void
type_time_introspector_code::gen_call_xml(const nstring &,
    const nstring &member_name, int attributes) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", def_name().quote_c().c_str()));
    bool show_default = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    nstring text =
        (
            "introspector_time::write_xml(fp, "
        +
            member_name.quote_c()
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


void
type_time_introspector_code::gen_free_declarator(const nstring &,
    bool is_a_list) const
{
    if (is_a_list)
        printf(";\n");
}


void
type_time_introspector_code::gen_copy(const nstring &member_name)
    const
{
    printf("%s = rhs.%s;\n", member_name.c_str(), member_name.c_str());
}


void
type_time_introspector_code::gen_trace(const nstring &vname,
    const nstring &value) const
{
    printf
    (
        "trace_time_real(%s, %s_get());\n",
        vname.quote_c().c_str(),
        value.c_str()
    );
}


void
type_time_introspector_code::gen_default_constructor(
    const nstring &inst_var_name)
{
    printf("%s(0)", inst_var_name.c_str());
}


void
type_time_introspector_code::gen_copy_constructor(
    const nstring &inst_var_name)
{
    printf("%s(rhs.%s)", inst_var_name.c_str(), inst_var_name.c_str());
}


void
type_time_introspector_code::gen_assignment_operator(
    const nstring &inst_var_name)
{
    printf("%s = rhs.%s;\n", inst_var_name.c_str(), inst_var_name.c_str());
}


void
type_time_introspector_code::gen_write(const nstring&,
    const nstring &member_name, bool show_default) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring text =
        (
            "introspector_time::write(fp, "
        +
            member_name.quote_c()
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


void
type_time_introspector_code::gen_methods(const nstring &class_name,
    const nstring &inst_var_name, int) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));

    printf("\n");
    printf("time_t\n");
    printf("%s::%s_get()\n", class_name.c_str(), inst_var_name.c_str());
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("return %s;\n", inst_var_name.c_str());
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
    printf("%s::%s_set(time_t x)\n", class_name.c_str(), inst_var_name.c_str());
    printf("{\n");
    printf("%s = x;\n", inst_var_name.c_str());
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("%s::%s_clear()\n", class_name.c_str(), inst_var_name.c_str());
    printf("{\n");
    printf("%s = 0;\n", inst_var_name.c_str());
    printf("}\n");

    include_once("libaegis/introspector/time.h");
    printf("\n");
    printf("introspector::pointer\n");
    printf
    (
        "%s::%s_introspector_factory()\n",
        class_name.c_str(),
        inst_var_name.c_str()
    );
    printf("{\n");
    nstring text =
        (
            "return introspector_time::create ( "
            "introspector_time::adapter_by_method < "
        +
            class_name
        +
            " > ::create ( this, &"
        +
            class_name
        +
            "::"
        +
            inst_var_name
        +
            "_set, &"
        +
            class_name
        +
            "::"
        +
            inst_var_name
        +
            "_is_set));"
        );
    wrap_and_print("", text);
    printf("}\n");
}
