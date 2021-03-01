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

#include <common/boolean.h>
#include <common/trace.h>

#include <fmtgen/type/enumeration/introsp_code.h>


type_enumeration_introspector_code::~type_enumeration_introspector_code()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_enumeration_introspector_code::type_enumeration_introspector_code(
        generator *a_gen, const nstring &a_name, bool a_global) :
    type_enumeration(a_gen, a_name, a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", def_name().quote_c().c_str()));
}


type::pointer
type_enumeration_introspector_code::create(generator *a_gen,
    const nstring &a_name, bool a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return
        pointer
        (
            new type_enumeration_introspector_code(a_gen, a_name, a_global)
        );
}


void
type_enumeration_introspector_code::gen_body()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", def_name().quote_c().c_str()));
    if (is_in_include_file())
        return;
    include_once("common/sizeof.h");

    printf("\n");
    printf("static const char *%s_s[] =\n", def_name().c_str());
    printf("{\n");
    for (size_t j = 0; j < elements.size(); ++j)
        printf("\"%s\",\n", elements[j].c_str());
    printf("};\n");

    printf("\n");
    printf("introspector_enumeration_%s::", def_name().c_str());
    printf("~introspector_enumeration_%s()\n", def_name().c_str());
    printf("{\n");
    printf("}\n");

    printf("\n");
    printf("introspector_enumeration_%s::", def_name().c_str());
    printf
    (
        "introspector_enumeration_%s(const adapter::pointer &arg) :\n",
        def_name().c_str()
    );
    indent_more();
    printf("introspector_enumeration(\"%s\", arg)\n", def_name().c_str());
    indent_less();
    printf("{\n");
    printf
    (
        "register_tags(%s_s, SIZEOF(%s_s));\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("}\n");

    printf("\n");
    printf("introspector::pointer\n");
    printf
    (
        "introspector_enumeration_%s::create(const adapter::pointer &arg)\n",
        def_name().c_str()
    );
    printf("{\n");
    printf
    (
        "return pointer(new introspector_enumeration_%s(arg));\n",
        def_name().c_str()
    );
    printf("}\n");

    printf("\n");
    printf("nstring\n");
    printf
    (
        "introspector_enumeration_%s::ename(%s_ty value)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf
    (
        //
        // The following code used to be
        //
        // if (int(value) >= 0 && size_t(value) < SIZEOF(%s_s))
        //
        // however, on 64 bit platform, it can be problematic, at
        // least according to gcc.
        //
        "if (int(value) >= 0 && value < %s_max)\n",
        def_name().c_str()
    );
    indent_more();
    printf("return %s_s[value];\n", def_name().c_str());
    indent_less();
    printf
    (
        "return nstring::format(\"%%d\", int(value));\n"
    );
    printf("}\n");

    include_once("common/ac/assert.h");
    include_once("libaegis/meta_type.h");
    printf("\n");
    printf("void\n");
    wrap_and_print
    (
        "",
        "introspector_enumeration_" + def_name() + "::write(const\n"
        "output::pointer &fp, const char *name, " + def_name() + "_ty\n"
        "value, bool show)\n"
    );
    printf("{\n");
    printf("if (value == 0)\n");
    printf("{\n");
    printf("if (!show || type_enum_option_query())\n");
    indent_more();
    printf("return;\n");
    indent_less();
    printf("}\n");
    printf("if (name)\n");
    printf("{\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputs(\" = \");\n");
    printf("}\n");
    printf
    (
        "assert(value < %s_max);\n",
        def_name().c_str()
    );
    printf("fp->fputs(%s_s[value]);\n", def_name().c_str());
    printf("if (name)\n");
    indent_more();
    printf("fp->fputs(\";\\n\");\n");
    indent_less();
    printf("}\n");

    include_once("common/ac/assert.h");
    printf("\n");
    printf("void\n");
    wrap_and_print
    (
        "",
        "introspector_enumeration_" + def_name() + "::write_xml(const\n"
        "output::pointer &fp, const char *name, " + def_name() + "_ty\n"
        "value, bool show)\n"
    );
    printf("{\n");
    printf("if (value == 0)\n");
    printf("{\n");
    printf("if (!show || type_enum_option_query())\n");
    indent_more();
    printf("return;\n");
    indent_less();
    printf("}\n");
    printf("assert(name);\n");
    printf
    (
        "assert(value < %s_max);\n",
        def_name().c_str()
    );
    printf("fp->fputc('<');\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputc('>');\n");
    printf("fp->fputs(%s_s[value]);\n", def_name().c_str());
    printf("fp->fputs(\"</\");\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputs(\">\\n\");\n");
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("introspector_enumeration_%s::report_init()\n", def_name().c_str());
    printf("{\n");
    nstring text =
        (
            "introspector_enumeration::report_init("
        +
            def_name()
        +
            "_s, SIZEOF("
        +
            def_name()
        +
            "_s));"
        );
    wrap_and_print("", text);
    printf("}\n");
}


void
type_enumeration_introspector_code::gen_methods(const nstring &class_name,
    const nstring &inst_var_name, int) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));

    printf("\n");
    printf("%s_ty\n", def_name().c_str());
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
    printf
    (
        "return (%s || %s_is_set_flag);\n",
        inst_var_name.c_str(),
        inst_var_name.c_str()
    );
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf
    (
        "%s::%s_set(%s_ty x)\n",
        class_name.c_str(),
        inst_var_name.c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("%s = x;\n", inst_var_name.c_str());
    printf("%s_is_set_flag = true;\n", inst_var_name.c_str());
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("%s::%s_clear()\n", class_name.c_str(), inst_var_name.c_str());
    printf("{\n");
    printf("%s = ", inst_var_name.c_str());
    if (elements.empty())
        printf("0");
    else
        printf("%s_%s", def_name().c_str(), elements[0].c_str());
    printf(";\n");
    printf("%s_is_set_flag = false;\n", inst_var_name.c_str());
    printf("}\n");

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
            "return introspector_enumeration_"
        +
            def_name()
        +
            "::create ( introspector_enumeration::adapter_by_method < "
        +
            class_name
        +
            ", "
        +
            def_name()
        +
            "_ty > ::create ( this, &"
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


void
type_enumeration_introspector_code::gen_report_initializations()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", def_name().quote_c().c_str()));
    printf("introspector_enumeration_%s::report_init();\n", def_name().c_str());
}


void
type_enumeration_introspector_code::gen_declarator(const nstring &variable_name,
    bool is_a_list, int attributes, const nstring &) const
{
    bool show = true;
    if (attributes & (ATTRIBUTE_SHOW_IF_DEFAULT | ATTRIBUTE_HIDE_IF_DEFAULT))
        show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    printf("introspector_enumeration_%s::write(fp, ", def_name().c_str());
    if (is_a_list)
    {
        printf("(const char *)0");
        show = true;
    }
    else
    {
        printf("\"%s\"", variable_name.c_str());
    }
    printf(", %s, %s);\n", variable_name.c_str(), bool_to_string(show).c_str());
}


void
type_enumeration_introspector_code::gen_call_xml(const nstring &,
    const nstring &member_name, int attributes) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", def_name().quote_c().c_str()));
    bool show_default = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    nstring text =
        (
            "introspector_enumeration_" + def_name() + "::write_xml(fp, "
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
type_enumeration_introspector_code::gen_free_declarator(const nstring &,
    bool is_a_list) const
{
    if (is_a_list)
        printf(";\n");
}


void
type_enumeration_introspector_code::gen_copy(const nstring &member_name)
    const
{
    printf("%s = rhs.%s;\n", member_name.c_str(), member_name.c_str());
}


void
type_enumeration_introspector_code::gen_trace(const nstring &vname,
    const nstring &value) const
{
    nstring text =
        (
            "trace_string_real("
        +
            vname.quote_c()
        +
            ", introspector_enumeration_"
        +
            def_name()
        +
            "::ename("
        +
            value
        +
            "));"
        );
    wrap_and_print("", text);
}


void
type_enumeration_introspector_code::gen_default_constructor(
    const nstring &inst_var_name)
{
    if (elements.empty())
    {
        printf("%s(0)", inst_var_name.c_str());
    }
    else
    {
        printf
        (
            "%s(%s_%s)",
            inst_var_name.c_str(),
            def_name().c_str(),
            elements[0].c_str()
        );
    }
    printf(",\n%s_is_set_flag(false)", inst_var_name.c_str());
}


void
type_enumeration_introspector_code::gen_copy_constructor(
    const nstring &inst_var_name)
{
    printf("%s(rhs.%s),\n", inst_var_name.c_str(), inst_var_name.c_str());
    printf
    (
        "%s_is_set_flag(rhs.%s_is_set_flag)",
        inst_var_name.c_str(),
        inst_var_name.c_str()
    );
}


void
type_enumeration_introspector_code::gen_assignment_operator(
    const nstring &inst_var_name)
{
    printf("%s = rhs.%s;\n", inst_var_name.c_str(), inst_var_name.c_str());
    printf
    (
        "%s_is_set_flag = rhs.%s_is_set_flag;\n",
        inst_var_name.c_str(),
        inst_var_name.c_str()
    );
}


void
type_enumeration_introspector_code::gen_write(const nstring&,
    const nstring &member_name, bool show_default) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    nstring text =
        (
            "introspector_enumeration_" + def_name() + "::write(fp, "
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


// vim: set ts=8 sw=4 et :
