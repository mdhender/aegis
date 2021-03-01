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
#include <fmtgen/type/string/introsp_code.h>


type_string_introspector_code::~type_string_introspector_code()
{
}


type_string_introspector_code::type_string_introspector_code(generator *a_gen) :
    type_string(a_gen)
{
}


type::pointer
type_string_introspector_code::create(generator *a_gen)
{
    return pointer(new type_string_introspector_code(a_gen));
}


void
type_string_introspector_code::gen_body()
    const
{
    include_once("libaegis/introspector/nstring.h");
}


void
type_string_introspector_code::gen_declarator(const nstring &variable_name,
    bool is_a_list, int, const nstring &) const
{
    printf("introspector_nstring::write(fp, ");
    if (is_a_list)
        printf("(const char *)0");
    else
        printf("\"%s\"", variable_name.c_str());
    printf(", %s);\n", variable_name.c_str());
}


void
type_string_introspector_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int) const
{
    printf
    (
        "introspector_nstring::write_xml(fp, \"%s\", %s);\n",
        form_name.c_str(),
        member_name.c_str()
    );
}


void
type_string_introspector_code::gen_copy(const nstring &member_name)
    const
{
    printf
    (
        "%s = rhs.%s;\n",
        member_name.c_str(),
        member_name.c_str()
    );
}


void
type_string_introspector_code::gen_free_declarator(const nstring &variable_name,
    bool) const
{
    printf("// %s: %d: instance variable %s\n", __FILE__, __LINE__,
        variable_name.quote_c().c_str());
}


void
type_string_introspector_code::gen_trace(const nstring &vname,
    const nstring &value) const
{
    nstring text = "trace_string_real(" + vname.quote_c() + ", " + value + ");";
    wrap_and_print("", text);
}


void
type_string_introspector_code::gen_default_constructor(
    const nstring &inst_var_name)
{
    printf("%s()", inst_var_name.c_str());
}


void
type_string_introspector_code::gen_methods(const nstring &class_name,
    const nstring &inst_var_name, int) const
{
    printf("\n");
    printf("nstring\n");
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
    printf("return !%s.empty();\n", inst_var_name.c_str());
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf
    (
        "%s::%s_set(const nstring &value_)\n",
        class_name.c_str(),
        inst_var_name.c_str()
    );
    printf("{\n");
    printf("%s = value_;\n", inst_var_name.c_str());
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("%s::%s_clear()\n", class_name.c_str(), inst_var_name.c_str());
    printf("{\n");
    printf("%s.clear();\n", inst_var_name.c_str());
    printf("}\n");

    include_once("libaegis/introspector/nstring.h");
    printf("\n");
    printf("introspector::pointer\n");
    printf
    (
        "%s::%s_introspector_factory()\n",
        class_name.c_str(),
        inst_var_name.c_str()
    );
    printf("{\n");
    printf("return introspector_nstring::create(%s);\n", inst_var_name.c_str());
    printf("}\n");
}


void
type_string_introspector_code::gen_write(const nstring &form_name,
    const nstring &member_name, bool show_default) const
{
    nstring text =
        (
            "introspector_nstring::write(fp, "
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
