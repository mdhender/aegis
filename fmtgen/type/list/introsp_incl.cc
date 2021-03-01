//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/error.h>
#include <common/trace.h>

#include <fmtgen/type/list/introsp_incl.h>


type_list_introspector_include::~type_list_introspector_include()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_list_introspector_include::type_list_introspector_include(generator *a_gen,
        const nstring &a_name, bool a_global, const type::pointer &a_type) :
    type_list(a_gen, a_name, a_global, a_type)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_list_introspector_include::create(generator *a_gen, const nstring &a_name,
    bool a_global, const type::pointer &a_type)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return
        pointer
        (
            new type_list_introspector_include(a_gen, a_name, a_global, a_type)
        );
}


void
type_list_introspector_include::gen_body()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    assert(subtype);
    include_once("libaegis/introspector.h");
    include_once("libaegis/output.h");
    printf("\n");
    printf("#ifndef %s_DEF\n", def_name().upcase().c_str());
    printf("#define %s_DEF\n", def_name().upcase().c_str());
    printf("\n");
    if (!comment.empty())
    {
        printf("/**\n%s\n*/\n", comment.c_str());
    }
    printf("class %s\n", def_name().c_str());
    printf("{\n");
    indent_less();
    printf("public:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The pointer typedef is used to have a pointer type which\n"
        "may be used by clients of this API, allowing the pointer\n"
        "implementation to be changed without extensive code changes\n"
        "throughout the rest of the code base.\n"
    );
    printf("*/\n");
    printf("typedef aegis_shared_ptr<%s> pointer;\n", def_name().c_str());

    printf("\n");
    printf("/**\n");
    printf("* The destructor.\n");
    printf("*/\n");
    printf("virtual ~%s();\n", def_name().c_str());

    printf("\n");
    indent_less();
    printf("private:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The default constructor.  It is private on purpose, use the\n"
        "#create class method instead.\n"
    );
    printf("*/\n");
    printf("%s();\n", def_name().c_str());

    printf("\n");
    indent_less();
    printf("public:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The create class method is used to create new dynamically\n"
        "allocated instances of this class.\n"
    );
    printf("*/\n");
    printf("static pointer create();\n");

    printf("\n");
    indent_less();
    printf("private:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The copy constructor.  It is private on purpose, use the\n"
        "#create class method instead.\n"
    );
    printf("*\n");
    printf("* @param rhs\n");
    printf("*     The right hand side of the initialization.\n");
    printf("*/\n");
    printf("%s(const %s &rhs);\n", def_name().c_str(), def_name().c_str());

    printf("\n");
    indent_less();
    printf("public:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The create class method is used to create new dynamically\n"
        "allocated instances of this class.\n"
    );
    printf("*\n");
    printf("* @param rhs\n");
    printf("*     The right hand side of the initialization.\n");
    printf("*/\n");
    printf("static pointer create(const %s &rhs);\n", def_name().c_str());

    printf("\n");
    printf("/**\n");
    printf("* The assignment operator.\n");
    printf("*\n");
    printf("* @param rhs\n");
    printf("*     The right hand side of the assignment.\n");
    printf("*/\n");
    printf
    (
        "%s &operator=(const %s &rhs);\n",
        def_name().c_str(),
        def_name().c_str()
    );

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The size method may be used to obtain the numer of items in\n"
        "this list.\n"
    );
    printf("*/\n");
    printf("size_t size() const { return length; }\n");

    printf("\n");
    printf("/**\n");
    printf("* The get method is used to get the n'th element of this list.\n");
    printf("*\n");
    printf("* @param n\n");
    printf("*    The list element of interest, zero based.\n");
    printf("*    The results are undefined if n >= size().\n");
    printf("*/\n");
    printf("bool &get(size_t n);\n");

    printf("\n");
    printf("/**\n");
    printf("* The clear method may be used to discard the list contents.\n");
    printf("*/\n");
    printf("void clear();\n");

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The push_back method may be used to append a value to the\n"
        "end of this list.\n"
    );
    printf("*\n");
    printf("* @param value\n");
    printf("*     The value to be appended to the list.\n");
    printf("*/\n");
    printf("void push_back(bool value);\n");

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The introspector_factory method is used to create an\n"
        "introspector for this class and instance, usually used\n"
        "with the meta-data parser when reading the contents of a\n"
        "meta-data file.\n"
    );
    printf("*/\n");
    printf("introspector::pointer introspector_factory();\n");

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The write method is used to write this structure's contents\n"
        "to the given output.\n"
    );
    printf("*\n");
    printf("* @param fp\n");
    printf("*     Where to write the output.\n");
    printf("* @param name\n");
    printf("*     The name of this structure member.\n");
    printf("*/\n");
    printf
    (
        "void write(const output::pointer &fp, const nstring &name) const;\n"
    );

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The write_xml method is used to write this structure's\n"
        "contents to the given output as XML.\n"
    );
    printf("*\n");
    printf("* @param fp\n");
    printf("*     Where to write the output.\n");
    printf("* @param name\n");
    printf("*     The name of this structure member.\n");
    printf("*/\n");
    printf
    (
        "void write_xml(const output::pointer &fp, "
        "const nstring &name) const;\n"
    );

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The trace_print method is used by the trace_" + def_name() + " macro\n"
        "to print an object of this type.\n"
    );
    printf("*\n");
    printf("* @param name\n");
    printf("*     The name of the variable being printed.\n");
    printf("*/\n");
    printf("void trace_print(const char *name) const;\n");

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The push_back_new_and_introspector_factory method is used\n"
        "to append a value to the end of the list, and return an\n"
        "introspector for managing that new value.\n"
    );
    printf("*/\n");
    printf("introspector::pointer push_back_new_and_introspector_factory();\n");

    //
    // emit the declarator for the list.
    // We don't need comments, here, it will emits its own.
    subtype->gen_declarator("list", true, 0, nstring());

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The length instance variable is used to remember the number\n"
        "items used in the #list array.\n"
    );
    printf("*/\n");
    printf("size_t length;\n");

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The maximum instance variable is used to remember the\n"
        "number items allocated to the #list array.\n"
    );
    printf("* @note\n");
    printf("*     assert(!list == !maximum);\n");
    printf("*     assert(length <= maximum);\n");
    printf("*/\n");
    printf("size_t maximum;\n");
    printf("};\n");
    printf("#endif // %s_DEF\n", def_name().upcase().c_str());

    include_once("common/debug.h");
    printf("#ifndef %s_trace\n", def_name().c_str());
    printf("#ifdef DEBUG\n");
    printf
    (
        "#define %s_trace(x) ((void)(trace_pretest_ && (trace_where_, "
            "%s_trace_real(trace_stringize(x), x), 0)))\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("#else\n");
    printf("#define %s_trace(x)\n", def_name().c_str());
    printf("#endif\n");
    printf("#endif\n");
}


void
type_list_introspector_include::gen_declarator(const nstring &variable_name,
    bool is_a_list, int, const nstring &cmnt) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    assert(subtype);
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
    printf
    (
        "%s::pointer %s%s;\n",
        def_name().c_str(),
        deref,
        variable_name.c_str()
    );

    printf("\n");
    indent_less();
    printf("public:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_get method is used to obtain the\n"
        "value of the #" + variable_name + " instance variable.  It\n"
        "will be created if it does not yet exist.\n"
    );
    printf("*/\n");
    printf
    (
        "%s::pointer %s_get();\n",
        def_name().c_str(),
        variable_name.c_str()
    );

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


// vim: set ts=8 sw=4 et :
