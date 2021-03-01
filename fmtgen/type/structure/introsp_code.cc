//
// aegis - project change supervisor
// Copyright (C) 2008, 2011, 2012 Peter Miller
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

#include <fmtgen/type/structure/introsp_code.h>
#include <fmtgen/type/vector.h>


type_structure_introspector_code::~type_structure_introspector_code()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_structure_introspector_code::type_structure_introspector_code(
        generator *a_gen, const nstring &a_name, bool a_global) :
    type_structure(a_gen, a_name, a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_structure_introspector_code::create(generator *a_gen,
    const nstring &a_name, bool a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return
        pointer(new type_structure_introspector_code(a_gen, a_name, a_global));
}


void
type_structure_introspector_code::gen_body()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (is_in_include_file())
        return;
    include_once("common/ac/assert.h");
    include_once("common/trace.h");

    //
    // the destructor
    //
    printf("\n");
    printf("%s::~%s()\n", def_name().c_str(), def_name().c_str());
    printf("{\n");
    printf("trace((\"%%s\\n\", __PRETTY_FUNCTION__));\n");
    printf("}\n");

    //
    // the default constructor
    //
    printf("\n");
    printf("%s::%s()", def_name().c_str(), def_name().c_str());
    indent_more();
    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        printf("%s\n", (j ? "," : " :"));
        ep->etype->gen_default_constructor(ep->name);
    }
    indent_less();
    printf("\n{\n");
    printf("trace((\"%%s\\n\", __PRETTY_FUNCTION__));\n");
    printf("}\n");

    //
    // default create
    //
    printf("\n");
    printf("%s::pointer\n", def_name().c_str());
    printf("%s::create()\n", def_name().c_str());
    printf("{\n");
    printf("trace((\"%%s\\n\", __PRETTY_FUNCTION__));\n");
    printf("return pointer(new %s());\n", def_name().c_str());
    printf("}\n");

    //
    // the copy constructor
    //
    printf("\n");
    printf
    (
        "%s::%s(const %s &",
        def_name().c_str(),
        def_name().c_str(),
        def_name().c_str()
    );
    if (nelements > 0)
        printf("rhs");
    printf(")");
    indent_more();
    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        printf("%s\n", (j ? "," : " :"));
        ep->etype->gen_copy_constructor(ep->name);
    }
    indent_less();
    printf("\n{\n");
    printf("trace((\"%%s\\n\", __PRETTY_FUNCTION__));\n");
    printf("}\n");

    //
    // copy create
    //
    printf("\n");
    printf("%s::pointer\n", def_name().c_str());
    printf
    (
        "%s::create(const %s &rhs)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("trace((\"%%s\\n\", __PRETTY_FUNCTION__));\n");
    printf("return pointer(new %s(rhs));\n", def_name().c_str());
    printf("}\n");

    //
    // the assignment operator
    //
    printf("\n");
    printf("%s &\n", def_name().c_str());
    printf
    (
        "%s::operator=(const %s &rhs)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("trace((\"%%s\\n\", __PRETTY_FUNCTION__));\n");
    printf("if (this != &rhs)\n");
    printf("{\n");
    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_assignment_operator(ep->name);
    }
    printf("}\n");
    printf("return *this;\n");
    printf("}\n");

    //
    // Generate getters and setters for each of the instance variables.
    //
    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_methods(def_name(), ep->name, ep->attributes);
    }

    include_once("libaegis/introspector/structure.h");
    printf("\n");
    printf("introspector::pointer\n");
    printf("%s::introspector_factory()\n", def_name().c_str());
    printf("{\n");
    printf("trace((\"%%s\\n\", __PRETTY_FUNCTION__));\n");

    printf("introspector_structure::pointer ip =\n");
    indent_more();
    printf("introspector_structure::create(\"%s\");\n", def_name().c_str());
    indent_less();

    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        nstring text =
            "ip->register_member ( " + ep->name.quote_c() +
            ", introspector_structure::adapter_by_method < " + def_name() +
            " > ::create ( this, &" + def_name() + "::" + ep->name +
            "_introspector_factory, &" + def_name() + "::" + ep->name +
            "_is_set));"
            ;
        wrap_and_print("", text);
    }
    printf("return ip;\n");
    printf("};\n");

    printf("\n");
    printf("void\n");
    printf("%s::write(const output::pointer &fp", def_name().c_str());
    if (!toplevel_flag)
        printf(", const nstring &name_");
    printf(")\n");
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("trace((\"%%s\\n{\\n\", __PRETTY_FUNCTION__));\n");
    if (!toplevel_flag)
    {
        printf("if (name_)\n");
        printf("{\n");
        printf("fp->fputs(name_);\n");
        printf("fp->fputs(\" =\\n\");\n");
        printf("}\n");
        printf("fp->fputs(\"{\\n\");\n");
    }
    trace(("nelements = %d\n", int(nelements)));
    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        trace(("ep->name = %s;\n", ep->name.quote_c().c_str()));
        bool show_default = false;
        ep->etype->gen_write(ep->name, ep->name, show_default);
    }
    if (!toplevel_flag)
    {
        printf("fp->fputs(\"}\");\n");
        printf("if (name_)\n");
        indent_more();
        printf("fp->fputs(\";\\n\");\n");
        indent_less();
    }
    else if (!nelements)
    {
        // To silence "fp unused" warning
        printf("(void)fp;\n");
    }
    printf("trace((\"}\\n\"));\n");
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("%s::write_xml(const output::pointer &fp", def_name().c_str());
    if (!toplevel_flag)
    {
        printf(", const nstring &name_");
    }
    printf(")\n");
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("trace((\"%%s\\n{\\n\", __PRETTY_FUNCTION__));\n");
    if (toplevel_flag)
    {
        printf("fp->fputs(\"<%s>\\n\");\n", def_name().c_str());
    }
    else
    {
        printf("assert(!name_.empty());\n");
        printf("fp->fputc('<');\n");
        printf("fp->fputs(name_);\n");
        printf("fp->fputs(\">\\n\");\n");
    }
    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_call_xml(ep->name, ep->name, ep->attributes);
    }
    if (toplevel_flag)
    {
        printf("fp->fputs(\"</%s>\\n\");\n", def_name().c_str());
    }
    else
    {
        printf("fp->fputs(\"</\");\n");
        printf("fp->fputs(name_);\n");
        printf("fp->fputs(\">\\n\");\n");
    }
    printf("trace((\"}\\n\"));\n");
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf("%s::trace_print(const char *name_)\n", def_name().c_str());
    indent_more();
    printf("const\n");
    indent_less();
    printf("{\n");
    printf("if (name_ && *name_)\n");
    printf("{\n");
    printf("trace_printf(\"%%s = \", name_);\n");
    printf("}\n");
    printf("trace_printf(\"{\\n\");\n");
    for (size_t j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_trace(ep->name, ep->name);
    }
    printf("trace_printf(\"}\");\n");
    printf("trace_printf((name_ && *name_) ? \";\\n\" : \",\\n\");\n");
    printf("}\n");

    if (toplevel_flag)
    {
        include_once("libaegis/meta_context/introspector.h");
        include_once("libaegis/os.h");
        printf("\n");
        printf("%s::pointer\n", def_name().c_str());
        printf
        (
            "%s::create_from_file(const nstring &filename)\n",
            def_name().c_str()
        );
        printf("{\n");
        printf("trace((\"%%s\\n{\\n\", __PRETTY_FUNCTION__));\n");
        printf("pointer p = create();\n");
        printf("introspector::pointer ip = p->introspector_factory();\n");
        printf("meta_context_introspector meta;\n");
        printf("os_become_must_be_active();\n");
        printf("meta.parse_file(filename, ip);\n");
        printf("trace((\"}\\n\"));\n");
        printf("return p;\n");
        printf("}\n");

        include_once("libaegis/io.h");
        include_once("libaegis/output/file.h");
        include_once("libaegis/output/filter/gzip.h");
        include_once("libaegis/output/filter/indent.h");
        printf("\n");
        printf("void\n");
        printf
        (
            "%s::write_file(const nstring &filename, bool needs_compression)\n",
            def_name().c_str()
        );
        indent_more();
        printf("const\n");
        indent_less();
        printf("{\n");
        printf("trace((\"%%s\\n{\\n\", __PRETTY_FUNCTION__));\n");
        printf("if (filename)\n");
        indent_more();
        printf("os_become_must_be_active();\n");
        indent_less();
        nstring text =
            "output::pointer fp = (needs_compression ?\n"
            "output_filter_gzip::create(output_file::binary_open(filename)) :\n"
            "output_file::text_open(filename));\n"
            ;
        wrap_and_print("", text);
        printf("fp = output_filter_indent::create(fp);\n");
        printf("io_comment_emit(fp);\n");
        printf("write(fp);\n");
        printf("type_enum_option_clear();\n");
        printf("trace((\"}\\n\"));\n");
        printf("}\n");

        printf("\n");
        printf("void\n");
        printf("%s::report_init(void)\n", def_name().c_str());
        printf("{\n");
        printf("trace((\"%%s\\n{\\n\", __PRETTY_FUNCTION__));\n");

        //
        // find the list of types we care about.
        // generate report initialization for them.
        //
        type_vector unique_types;
        get_reachable(unique_types);
        unique_types.reachable_closure();
        unique_types.gen_report_initializations();

        printf("trace((\"}\\n\"));\n");
        printf("}\n");
    }
}


void
type_structure_introspector_code::gen_declarator(const nstring &variable_name,
    bool, int, const nstring &cmnt) const
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
    printf("%s::pointer %s;\n", def_name().c_str(), variable_name.c_str());

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
        "used to create a new instrospector for manipulating the\n"
        "contents of the " + variable_name + " instance variable.\n"
        "The structure will be create()ed if necessary.\n"
    );
    printf("*/\n");
    printf
    (
        "introspector::pointer %s_introspector_factory();\n",
        variable_name.c_str()
    );
}


void
type_structure_introspector_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf
    (
        "if (%s)\n{\n%s->write_xml(fp, \"%s\");\n}\n",
        member_name.c_str(),
        member_name.c_str(),
        form_name.c_str()
    );
}


void
type_structure_introspector_code::gen_default_constructor(
    const nstring &member_name)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("%s()", member_name.c_str());
}


void
type_structure_introspector_code::gen_copy_constructor(
    const nstring &member_name)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("%s(rhs.%s)", member_name.c_str(), member_name.c_str());
}


void
type_structure_introspector_code::gen_assignment_operator(
    const nstring &member_name)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("%s = rhs.%s;\n", member_name.c_str(), member_name.c_str());
}


void
type_structure_introspector_code::gen_free_declarator(const nstring &, bool)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
type_structure_introspector_code::gen_trace(const nstring &vname,
    const nstring &value) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("if (%s)\n", value.c_str());
    printf("{\n");
    printf("%s->trace_print(\"*%s\");\n", value.c_str(), vname.c_str());
    printf("}\n");
    printf("else\n");
    printf("{\n");
    printf("trace_printf(\"%s = NULL\");\n", vname.c_str());
    printf("}\n");
}


void
type_structure_introspector_code::gen_copy(nstring const &inst_var_name)
    const
{
    printf("%s(rhs.%s)", inst_var_name.c_str(), inst_var_name.c_str());
}


void
type_structure_introspector_code::gen_write(const nstring &form_name,
    const nstring &member_name, bool) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("if (%s)\n", member_name.c_str());
    printf("{\n");
    nstring text = member_name + "->write(fp, " + form_name.quote_c() + ");";
    wrap_and_print("", text);
    printf("}\n");
}


// vim: set ts=8 sw=4 et :
