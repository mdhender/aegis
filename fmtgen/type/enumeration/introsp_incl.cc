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

#include <fmtgen/type/enumeration/introsp_incl.h>


static int
nbits(size_t ntags)
{
    int n = 1;
    while (ntags > (size_t(1) << n))
        ++n;
    return n;
}


type_enumeration_introspector_include::~type_enumeration_introspector_include()
{
}


type_enumeration_introspector_include::type_enumeration_introspector_include(
        generator *a_gen, const nstring &a_name, bool a_global) :
    type_enumeration(a_gen, a_name, a_global)
{
}


type::pointer
type_enumeration_introspector_include::create(generator *a_gen,
    const nstring &a_name, bool a_global)
{
    return
        pointer
        (
            new type_enumeration_introspector_include(a_gen, a_name, a_global)
        );
}


void
type_enumeration_introspector_include::gen_body()
    const
{
    include_once("libaegis/introspector.h");
    include_once("libaegis/output.h");

    printf("\n");
    printf("#ifndef %s_DEF\n", def_name().upcase().c_str());
    printf("#define %s_DEF\n", def_name().upcase().c_str());
    printf("\n");
    if (!comment.empty())
        printf("/**\n%s\n*/\n", comment.c_str());
    printf("enum %s_ty\n", def_name().c_str());
    printf("{\n");
    for (size_t j = 0; j < elements.size(); ++j)
    {
        printf
        (
            "%s_%s",
            def_name().c_str(),
            elements[j].c_str()
        );
        if (j < elements.size() - 1)
            printf(",");
        printf("\n");
    }
    printf("};\n");
    printf
    (
        "#define %s_max %u\n",
        def_name().c_str(),
        (unsigned)elements.size()
    );

    include_once("libaegis/introspector/enumeration.h");
    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The introspector_enumeration_" + def_name() + " class is\n"
        "used to represent how the meta-data parser manipulates a\n"
        "" + def_name() + "_ty variable.\n"
    );
    printf("*/\n");
    printf("class introspector_enumeration_%s:\n", def_name().c_str());
    indent_more();
    printf("public introspector_enumeration\n");
    indent_less();
    printf("{\n");
    indent_less();
    printf("public:\n");
    indent_more();
    printf("/**\n");
    printf("* The destructor.\n");
    printf("*/\n");
    printf("virtual ~introspector_enumeration_%s();\n", def_name().c_str());

    printf("\n");
    indent_less();
    printf("private:\n");
    indent_more();
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The constructor.  It is private on purpose, use the\n"
        "#create class method instead.\n"
    );
    printf("*\n");
    printf("* @param how\n");
    wrap_and_print
    (
        "*     ",
        "This argument is used to describe how to actually manipulate\n"
        "a field of " + def_name() + "_ty enumerated type in a\n"
        "meta-data structure.\n"
    );
    printf("*/\n");
    wrap_and_print
    (
        "introspector_enumeration_" + def_name() +
        "(const introspector_enumeration::adapter::pointer &how);"
    );

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
    printf("* @param how\n");
    wrap_and_print
    (
        "*     ",
        "This argument is used to describe how to actually manipulate\n"
        "a field of " + def_name() + "_ty enumerated type in a\n"
        "meta-data structure.\n"
    );
    printf("*/\n");
    wrap_and_print
    (
        "static pointer create("
        "const introspector_enumeration::adapter::pointer &how);"
    );

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The ename class method is used to obtain the string\n"
        "corresponding to given value.\n"
    );
    printf("*\n");
    printf("* @param value\n");
    printf("*     The value to be translated.\n");
    printf("*/\n");
    wrap_and_print("static nstring ename(" + def_name() + "_ty value);");

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        // watch out for unbalanced '\'' characters.
        "The report_init class method is used to initialize the\n"
        "report generator with the names and values of this\n"
        "enumeration`s tags.\n"
    );
    printf("*/\n");
    printf("static void report_init(void);\n");

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The write class method is used to write a " + def_name() + "\n"
        "value to the given output.\n"
    );
    printf("*\n");
    printf("* @param fp\n");
    printf("*     The output stream to write on.\n");
    printf("* @param name\n");
    printf("*     The name of the field.\n");
    printf("* @param value\n");
    printf("*     The value to be printed.\n");
    printf("* @param show\n");
    wrap_and_print
    (
        "*     ",
        "true means show even if defult value, false means show only\n"
        "if not default value.\n"
    );
    printf("*/\n");
    wrap_and_print
    (
        "static void write(const output::pointer &fp,\n"
        "const char *name, " + def_name() + "_ty value, bool show);\n"
    );

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The write_xml class method is used to write a " + def_name() + "\n"
        "value as XML to the given output.\n"
    );
    printf("*\n");
    printf("* @param fp\n");
    printf("*     The output stream to write on.\n");
    printf("* @param name\n");
    printf("*     The name of the XML element.\n");
    printf("* @param value\n");
    printf("*     The value to be printed.\n");
    printf("* @param show\n");
    wrap_and_print
    (
        "*     ",
        "true means show even if defult value, false means show only\n"
        "if not default value.\n"
    );
    printf("*/\n");
    wrap_and_print
    (
        "static void write_xml(const output::pointer &fp,\n"
        "const char *name, " + def_name() + "_ty value, bool show);\n"
    );

    printf("\n");
    printf("/**\n");
    wrap_and_print
    (
        "* ",
        "The trace_print class method is used to write\n"
        "a " + def_name() + " value via trace_printf.\n"
        "Used by the #" + def_name() + "_trace macro.\n"
    );
    printf("*\n");
    printf("* @param name\n");
    printf("*      The name of the variable being printed.\n");
    printf("* @param value\n");
    printf("*     The value to be printed.\n");
    printf("*/\n");
    wrap_and_print
    (
        "static void trace_print(const char *name, " +
        def_name() + "_ty value);"
    );

    printf("\n");
    indent_less();
    printf("private:\n");
    indent_more();
    printf("/**\n");
    printf("* The default constructor.  Do not use.\n");
    printf("*/\n");
    printf("introspector_enumeration_%s();\n", def_name().c_str());

    printf("\n");
    printf("/**\n");
    printf("* The copy constructor.  Do not use.\n");
    printf("*/\n");
    wrap_and_print
    (
        "introspector_enumeration_" + def_name() +
        "(const introspector_enumeration_" + def_name() + " &);"
    );

    printf("\n");
    printf("/**\n");
    printf("* The assignment operator.  Do not use.\n");
    printf("*/\n");
    wrap_and_print
    (
        "introspector_enumeration_" + def_name() + " &operator=("
        "const introspector_enumeration_" + def_name() + " &);"
    );
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
        "introspector_enumeration_%s::trace_print(trace_stringize(x), "
        "(x)), 0)))\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("#else\n");
    printf("#define %s_trace(x)\n", def_name().c_str());
    printf("#endif\n");
    printf("#endif\n");

    printf("\n");
    printf("#endif // %s_DEF\n", def_name().upcase().c_str());
}


void
type_enumeration_introspector_include::gen_declarator(
    const nstring &variable_name, bool is_a_list, int,
    const nstring &cmnt) const
{
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
        "%s_ty %s%s",
        def_name().c_str(),
        (is_a_list ? "*" : ""),
        variable_name.c_str()
    );
    if (!is_a_list && use_bit_fields)
        printf(": %d", nbits(elements.size()));
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
        wrap_and_print
        (
            "bool " + variable_name + "_is_set_flag" +
            (use_bit_fields ? ": 1" : "") + ";"
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
        "The " + variable_name + "_get method is used to obtain the\n"
        "value of the " + variable_name + " instance variable, or the\n"
        "default if it has not been set.\n"
    );
    if (is_a_list)
    {
        printf("*\n");
        printf("* @param n\n");
        printf("*     The array index of interest.\n");
    }
    printf("*/\n");
    wrap_and_print
    (
        def_name() + "_ty " + variable_name + "_get(" +
        (is_a_list ? "int n" : "") + ") const;"
    );

    if (!is_a_list)
    {
        printf("\n");
        printf("/**\n");
        wrap_and_print
        (
            "* ",
            "The " + variable_name + "_is_set method is used to\n"
            "determine whether or not the " + variable_name + "\n"
            "instance variable has been set.\n"
        );
        printf("*/\n");
        wrap_and_print("bool " + variable_name + "_is_set() const;");

        printf("\n");
        printf("/**\n");
        wrap_and_print
        (
            "* ",
            "The " + variable_name + "_set method is used to set the\n"
            "value of the #" + variable_name + " instance variable,\n"
            "and record that it has been set.\n"
        );
        printf("*/\n");
        wrap_and_print
        (
            "void " + variable_name + "_set(" + def_name() + "_ty value);"
        );

        printf("\n");
        printf("/**\n");
        wrap_and_print
        (
            "* ",
            "The " + variable_name + "_clear method is used to reset\n"
            "the the #" + variable_name + " instance variable to its\n"
            "default value, and record that it is no longer set.\n"
        );
        printf("*/\n");
        wrap_and_print("void " + variable_name + "_clear();");

        printf("\n");
        printf("/**\n");
        wrap_and_print
        (
            "* ",
            "The " + variable_name + "_introspector_factory method\n"
            "is used to obtain an introspector for the\n"
            "#" + variable_name + " instance variable.\n"
        );
        printf("*/\n");
        wrap_and_print
        (
            "introspector::pointer " + variable_name +
            "_introspector_factory();"
        );
    }
}


// vim: set ts=8 sw=4 et :
