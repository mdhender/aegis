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

#include <common/trace.h>

#include <fmtgen/type/list/introsp_code.h>


type_list_introspector_code::~type_list_introspector_code()
{
}


type_list_introspector_code::type_list_introspector_code(generator *a_gen,
        const nstring &a_name, bool a_global, const type::pointer &a_subtype) :
    type_list(a_gen, a_name, a_global, a_subtype)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_list_introspector_code::create(generator *a_gen, const nstring &a_name,
    bool a_global, const type::pointer &a_type)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return
        pointer
        (
            new type_list_introspector_code(a_gen, a_name, a_global, a_type)
        );
}


void
type_list_introspector_code::gen_body()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (is_in_include_file())
    {
        trace(("no code for %s because in include file\n",
            def_name().quote_c().c_str()));
        return;
    }
    assert(subtype);
    include_once("common/ac/assert.h");
    include_once("libaegis/aer/value/list.h");

    include_once("common/trace.h");
    printf("\n");
    printf("void\n");
    nstring text =
        (
            def_name()
        +
            "::write(const output::pointer &fp, const nstring &name)"
        );
    wrap_and_print("", text);
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("trace((\"%%s {\\n\", __PRETTY_FUNCTION__));\n");
    printf("if (name)\n");
    printf("{\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputs(\" =\\n\");\n");
    printf("}\n");
    printf("assert(length <= maximum);\n");
    printf("assert(!list == !maximum);\n");
    printf("fp->fputs(\"[\\n\");\n");
    printf("for (size_t j = 0; j < length; ++j)\n");
    printf("{\n");
    bool show_if_default = true;
    subtype->gen_write(nstring(), "list[j]", show_if_default);
    printf("fp->fputs(\",\\n\");\n");
    printf("}\n");
    printf("fp->fputs(\"]\");\n");
    printf("if (name)\n");
    indent_more();
    printf("fp->fputs(\";\\n\");\n");
    indent_less();
    printf("trace((\"}\\n\"));\n");
    printf("}\n");

    printf("\n");
    printf("void\n");
    text =
        (
            def_name()
        +
            "::write_xml(const output::pointer &fp, const nstring &name)"
        );
    wrap_and_print("", text);
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("trace((\"%%s {\\n\", __PRETTY_FUNCTION__));\n");
    printf("assert(name);\n");
    printf("fp->fputc('<');\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputs(\">\\n\");\n");
    printf("assert(length <= maximum);\n");
    printf("assert(!list == !maximum);\n");
    printf("for (size_t j = 0; j < length; ++j)\n");
    printf("{\n");
    subtype->gen_call_xml
    (
        subtype->def_name(),
        "list[j]",
        ATTRIBUTE_SHOW_IF_DEFAULT
    );
    printf("}\n");
    printf("fp->fputs(\"</\");\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputs(\">\\n\");\n");
    printf("trace((\"}\\n\"));\n");
    printf("}\n");

    printf("\n");
    printf("%s::%s() :\n", def_name().c_str(), def_name().c_str());
    indent_more();
    printf("list(0),\n");
    printf("length(0),\n");
    printf("maximum(0)\n");
    indent_less();
    printf("{\n");
    printf("trace((\"%%s\\n\", __PRETTY_FUNCTION__));\n");
    printf("}\n");

    printf("\n");
    printf("%s::~%s()\n", def_name().c_str(), def_name().c_str());
    printf("{\n");
    printf("trace((\"%%s {\\n\", __PRETTY_FUNCTION__));\n");
    printf("delete [] list;\n");
    printf("list = 0;\n");
    printf("length = 0;\n");
    printf("maximum = 0;\n");
    printf("trace((\"}\\n\"));\n");
    printf("}\n");
}


void
type_list_introspector_code::gen_declarator(const nstring &variable_name,
    bool is_a_list, int, const nstring &) const
{
    assert(subtype);
    printf("%s->write(fp, ", variable_name.c_str());
    if (is_a_list)
        printf("\"\"");
    else
        printf("%s", variable_name.quote_c().c_str());
    printf(");\n");
}


void
type_list_introspector_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int) const
{
    assert(subtype);
    printf("if (%s)\n", member_name.c_str());
    indent_more();
    printf
    (
        "%s->write_xml(fp, %s);\n",
        member_name.c_str(),
        form_name.quote_c().c_str()
    );
    indent_less();
}


void
type_list_introspector_code::gen_copy(const nstring &member_name)
    const
{
    assert(subtype);
    printf("if (%s)\n", member_name.c_str());
    indent_more();
    printf
    (
        "%s = %s::create(*rhs.%s);\n",
        member_name.c_str(),
        def_name().c_str(),
        member_name.c_str()
    );
    indent_less();
}


void
type_list_introspector_code::gen_trace(const nstring &vname,
    const nstring &value) const
{
    printf("if (%s)\n", vname.c_str());
    printf("{\n");
    nstring text = value + "->trace_print( " + vname.quote_c() + ");";
    wrap_and_print("", text);
    printf("}\n");
}


void
type_list_introspector_code::gen_default_constructor(const nstring &var_name)
{
    printf("%s()", var_name.c_str());
}


void
type_list_introspector_code::gen_write(const nstring &,
    const nstring &member_name, bool) const
{
    printf("if (%s)\n", member_name.c_str());
    printf("{\n");
    nstring text =
        (
            member_name
        +
            "->write ( fp, "
        +
            member_name.quote_c()
        +
            ");"
        );
    wrap_and_print("", text);
    printf("}\n");
}


void
type_list_introspector_code::gen_methods(const nstring &class_name,
    const nstring &inst_var_name, int) const
{
    printf("\n");
    printf("%s::pointer\n", def_name().c_str());
    printf
    (
        "%s::%s_get()\n",
        class_name.c_str(),
        inst_var_name.c_str()
    );
    printf("{\n");
    printf("if (!%s)\n", inst_var_name.c_str());
    printf("{\n");
    printf("%s = %s::create();\n", inst_var_name.c_str(), def_name().c_str());
    printf("}\n");
    printf("return %s;\n", inst_var_name.c_str());
    printf("}\n");

    printf("\n");
    printf("bool\n");
    printf
    (
        "%s::%s_is_set()\n",
        class_name.c_str(),
        inst_var_name.c_str()
    );
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("return !!%s;\n", inst_var_name.c_str());
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
    printf("return %s_get()->introspector_factory();\n", inst_var_name.c_str());
    printf("}\n");
}


// vim: set ts=8 sw=4 et :
