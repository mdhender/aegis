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

#include <common/trace.h>

#include <fmtgen/type/structure/introsp_incl.h>


type_structure_introspector_include::~type_structure_introspector_include()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_structure_introspector_include::type_structure_introspector_include(
        generator *a_gen, const nstring &a_name, bool a_global) :
    type_structure(a_gen, a_name, a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_structure_introspector_include::create(generator *a_gen,
    const nstring &a_name, bool a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return
        pointer
        (
            new type_structure_introspector_include(a_gen, a_name, a_global)
        );
}


void
type_structure_introspector_include::gen_body()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    include_once("libaegis/introspector.h");
    include_once("libaegis/output.h");

    printf("\n");
    printf("#ifndef %s_DEF\n", def_name().c_str());
    printf("#define %s_DEF\n", def_name().c_str());

    printf("\n");
    if (comment.empty())
    {
        if (toplevel_flag)
        {
            printf("/**\n");
            wrap_and_print
            (
                "* ",
                "The " + def_name() + " class is used to represent "
                "''" + def_name() + "'' formatted Aegis meta-data.  "
                "See ae" + def_name() + "(5) for more information."
            );
            printf("*/\n");
        };
    }
    else
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
    wrap_and_print(def_name() + "(const " + def_name() + " &rhs);");

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
    wrap_and_print("static pointer create(const " + def_name() + " &rhs);");

    printf("\n");
    printf("/**\n");
    printf("* The assignment operator.\n");
    printf("*\n");
    printf("* @param rhs\n");
    printf("*     The right hand side of the assignment.\n");
    printf("*/\n");
    wrap_and_print(def_name() + " &operator=(const " + def_name() + " &rhs);");

    //
    // Generate declarations and getters and setters for each of the
    // instance variables.
    //
    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_declarator
        (
            ep->name,
            false,
            ATTRIBUTE_SHOW_IF_DEFAULT,
            ep->comment
        );
    }

    printf("\n");
    indent_less();
    printf("public:\n");
    indent_more();
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
    if (!toplevel_flag)
    {
        printf("* @param name\n");
        printf("*     The name of this structure member.\n");
    }
    printf("*/\n");
    wrap_and_print
    (
        nstring("void write(const output::pointer &fp") +
        (toplevel_flag ? "" : ", const nstring &name") +
        ") const;"
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
    if (!toplevel_flag)
    {
        printf("* @param name\n");
        printf("*     The name of this structure member.\n");
    }
    printf("*/\n");
    wrap_and_print
    (
        nstring("void write_xml(const output::pointer &fp") +
        (toplevel_flag ? "" : ", const nstring &name") +
        ") const;"
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

    if (toplevel_flag)
    {
        printf("\n");
        printf("/**\n");
        wrap_and_print
        (
            "* ",
            "The create_from_file class method is used to read\n"
            "the given file to create a new dynamically allocated\n"
            "instance of this class.\n"
        );
        printf("*\n");
        printf("* @param filename\n");
        printf("*     The name of the file to be read.\n");
        printf("* @note\n");
        wrap_and_print
        (
            "*     ",
            "If any errors are encountered, this method will not\n"
            "return.  All errors will print a fatal error message,\n"
            "and exit with an exit status of 1.\n"
        );
        printf("*/\n");
        printf("static pointer create_from_file(const nstring &filename);\n");

        printf("\n");
        printf("/**\n");
        wrap_and_print
        (
            "* ",
            "The write_file method is used to write the data to the\n"
            "named file, via the #write method.\n"
        );
        printf("*\n");
        printf("* @param filename\n");
        printf("*     The name of the file to be written.\n");
        printf("* @param compress\n");
        printf("*     true if data should be compressed, false if not.\n");
        printf("* @note\n");
        wrap_and_print
        (
            "*     ",
            "If any errors are encountered, this method will not\n"
            "return.  All errors will print a fatal error message,\n"
            "and exit with an exit status of 1.\n"
        );
        printf("*/\n");
        printf("void write_file(const nstring &filename,");
        printf(" bool compress) const;\n");

        printf("\n");
        printf("/**\n");
        wrap_and_print
        (
            "* ",
            "The report_init class method is used to initialize\n"
            "the report generator with the names and values of\n"
            "enumerations used by this meta-data.\n"
        );
        printf("*/\n");
        printf("static void report_init();\n");
    }
    printf("};\n");

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + def_name() + "_trace macro is used to conditionally\n"
        "print a trace of a " + def_name() + " value, if the DEBUG\n"
        "macro is defined.\n"
    );
    printf("*/\n");
    include_once("common/debug.h");
    printf("#ifndef %s_trace\n", def_name().c_str());
    printf("#ifdef DEBUG\n");
    printf
    (
        "#define %s_trace(x) ((void)(trace_pretest_ && (trace_where_, "
            "(x).trace_print(trace_stringize(x)), 0)))\n",
        def_name().c_str()
    );
    printf("#else\n");
    printf("#define %s_trace(x)\n", def_name().c_str());
    printf("#endif\n");
    printf("#endif\n");

    printf("\n");
    printf("#endif // %s_DEF\n", def_name().c_str());
}


void
type_structure_introspector_include::gen_declarator(
    const nstring &variable_name, bool is_a_list, int,
    const nstring &cmnt) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));

    printf("\n");
    indent_less();
    printf("private:\n");
    indent_more();
    if (!cmnt.empty())
    {
        printf("/**\n%s\n */\n", cmnt.c_str());
    }
    printf
    (
        "%s::pointer %s%s;\n",
        def_name().c_str(),
        (is_a_list ? "*" : ""),
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
        "The " + variable_name + "_get method is used to obtain\n"
        "a pointer to the contents of the " + variable_name + "\n"
        "instance variable.  The structure will be create()ed if\n"
        "necessary.\n"
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
        "The " + variable_name + "_is_set method may be used to\n"
        "determine whether or not the " + variable_name + " instance\n"
        "variable presently has any contents.\n"
    );
    printf("*/\n");
    printf("bool %s_is_set() const;\n", variable_name.c_str());

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The " + variable_name + "_clear method is used to dascard\n"
        "contents of the " + variable_name + " instance variable.\n"
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
    wrap_and_print
    (
        "introspector::pointer " + variable_name + "_introspector_factory();\n"
    );
}


// vim: set ts=8 sw=4 et :
