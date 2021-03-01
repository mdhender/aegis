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

#include <common/trace.h>

#include <fmtgen/type/boolean/introsp_incl.h>


type_boolean_introspector_include::~type_boolean_introspector_include()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_boolean_introspector_include::type_boolean_introspector_include(
        generator *a_gen) :
    type_boolean(a_gen)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_boolean_introspector_include::create(generator *gen)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new type_boolean_introspector_include(gen));
}


void
type_boolean_introspector_include::gen_body()
    const
{
    include_once("libaegis/introspector.h");
}


void
type_boolean_introspector_include::gen_declarator(const nstring &variable_name,
    bool is_a_list, int, const nstring &cmnt) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    const char *deref = (is_a_list ? "*" : "");
    printf("\n");
    indent_less();
    printf("private:\n");
    indent_more();
    printf("/**\n");
    if (cmnt.empty())
    {
        wrap_and_print
        (
            "* ",
            default_instance_variable_comment(variable_name, is_a_list)
        );
    }
    else
        printf("%s\n", cmnt.c_str());
    printf("*/\n");
    printf("bool %s%s", deref, variable_name.c_str());
    if (!is_a_list && use_bit_fields)
        printf(": 1");
    printf(";\n");

    if (!is_a_list)
    {
        printf("\n");
        printf("/**\n");
        wrap_and_print
        (
            "* ",
            "The " + variable_name + "_is_set_flag instance\n"
            "variable is used to remember whether or not the\n"
            "#" + variable_name + " instance variable has been set.\n"
        );
        printf("*/\n");
        printf("bool %s_is_set_flag", variable_name.c_str());
        if (use_bit_fields)
            printf(": 1");
        printf(";\n");
    }

    printf("\n");
    indent_less();
    printf("public:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_set method is used to set the\n"
        "#" + variable_name + " instance variable, and remember that\n"
        "it has been set.\n"
    );
    printf("*\n");
    printf("* @param n\n");
    printf("*     The new value.\n");
    printf("*/\n");
    printf("void %s_set(bool n);\n", variable_name.c_str());

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_get method is used to obtain the\n"
        "value of the #" + variable_name + " instance variable, or\n"
        "false if it has not been set.\n"
    );
    printf("*/\n");
    printf("bool %s_get() const;\n", variable_name.c_str());

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_is_set method is used to determine\n"
        "whether or not the #" + variable_name + " instance variable\n"
        "is set.\n"
    );
    printf("*/\n");
    printf("bool %s_is_set() const;\n", variable_name.c_str());

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_clear method is used to clear the\n"
        "value of the #" + variable_name + " instance variable, and\n"
        "remember that it is no longer set.\n"
    );
    printf("*/\n");
    printf("void %s_clear();\n", variable_name.c_str());

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_introspector_factory method is\n"
        "used to obtain an introspector for the #" + variable_name + "\n"
        "instance variable.\n"
    );
    printf("*/\n");
    printf
    (
        "introspector::pointer %s_introspector_factory();\n",
        variable_name.c_str()
    );
}
