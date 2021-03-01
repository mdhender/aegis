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

#include <fmtgen/type/integer/introsp_incl.h>


type_integer_introspector_include::~type_integer_introspector_include()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_integer_introspector_include::type_integer_introspector_include(
        generator *a_gen) :
    type_integer(a_gen)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_integer_introspector_include::create(generator *a_gen)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new type_integer_introspector_include(a_gen));
}

void
type_integer_introspector_include::gen_body()
    const
{
    include_once("libaegis/introspector.h");
}


void
type_integer_introspector_include::gen_declarator(const nstring &variable_name,
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
        wrap_and_print("* ", default_instance_variable_comment(variable_name));
    else
        printf("%s\n", cmnt.c_str());
    printf("*/\n");
    printf("%s %s%s;\n", "long", deref, variable_name.c_str());

    printf("\n");
    indent_less();
    printf("public:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_set method is used to set the\n"
        "#" + variable_name + " instance variable.\n"
    );
    printf("*\n");
    printf("* @param n\n");
    printf("*     The new value.\n");
    printf("*\n");
    printf("* @note\n");
    wrap_and_print
    (
        "*     ",
        "The use of magic_zero_encode is internal to this method.\n"
        "Clients of this API will not need to use magic_zero_encode.\n"
    );
    printf("*/\n");
    printf("void %s_set(long n);\n", variable_name.c_str());

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_get method is used to obtain the\n"
        "value of the #" + variable_name + " instance variable.\n"
    );
    printf("*\n");
    printf("* @note\n");
    wrap_and_print
    (
        "*     ",
        "The use of magic_zero_decode is internal to this method.\n"
        "Clients of this API will not need to use magic_zero_decode.\n"
    );
    printf("*/\n");
    printf("long %s_get() const;\n", variable_name.c_str());

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
        "value of the #" + variable_name + " instance variable.\n"
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
