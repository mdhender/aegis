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

#include <fmtgen/type/structure/code.h>


type_structure_code::~type_structure_code()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_structure_code::type_structure_code(generator *a_gen,
        const nstring &a_name, bool a_global) :
    type_structure(a_gen, a_name, a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_structure_code::create(generator *a_gen, const nstring &a_name,
    bool a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new type_structure_code(a_gen, a_name, a_global));
}


void
type_structure_code::gen_body()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (is_in_include_file())
        return;
    size_t j;
    include_once("common/ac/assert.h");
    include_once("common/sizeof.h");
    include_once("common/trace.h");
    include_once("libaegis/output.h");
    printf("\n");
    printf("void\n");
    if (toplevel_flag)
    {
        printf
        (
            "%s_write(const output::pointer &fp, %s_ty *this_thing)\n",
            def_name().c_str(),
            def_name().c_str()
        );
    }
    else
    {
        printf
        (
            "%s_write(const output::pointer &fp, const char *name, "
                "%s_ty *this_thing)\n",
            def_name().c_str(),
            def_name().c_str()
        );
    }
    printf("{\n");
    printf("if (!this_thing)\n");
    indent_more();
    printf("return;\n");
    indent_less();
    if (toplevel_flag)
    {
        printf
        (
            "trace((\"%s_write(this_thing = %%p)\\n{\\n\", "
            "this_thing));\n",
            def_name().c_str()
        );
    }
    else
    {
        printf
        (
            "trace((\"%s_write(name = \\\"%%s\\\", this_thing = %%p)\\n"
            "{\\n\", name, this_thing));\n",
            def_name().c_str()
        );
    }
    printf
    (
        "assert(((%s_ty *)this_thing)->reference_count > 0);\n",
        def_name().c_str()
    );
    printf
    (
        "trace((\"rc = %%ld;\\n\", ((%s_ty *)this_thing)->reference_count));\n",
        def_name().c_str()
    );
    if (!toplevel_flag)
    {
        printf("if (name)\n");
        printf("{\n");
        printf("fp->fputs(name);\n");
        printf("fp->fputs(\" =\\n\");\n");
        printf("}\n");
        printf("fp->fputs(\"{\\n\");\n");
    }
    trace(("nelements = %d\n", int(nelements)));
    for (j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        trace(("ep->name = %s;\n", ep->name.quote_c().c_str()));
        ep->etype->gen_declarator(ep->name, false, ep->attributes, ep->comment);
    }
    if (!toplevel_flag)
    {
        printf("fp->fputs(\"}\");\n");
        printf("if (name)\n");
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
    if (toplevel_flag)
    {
        printf
        (
            "%s_write_xml(const output::pointer &fp, %s_ty *this_thing)\n",
            def_name().c_str(),
            def_name().c_str()
        );
    }
    else
    {
        printf
        (
           "%s_write_xml(const output::pointer &fp, const char *name, "
                "%s_ty *this_thing)\n",
            def_name().c_str(),
            def_name().c_str()
        );
    }
    printf("{\n");
    printf("if (!this_thing)\n");
    indent_more();
    printf("return;\n");
    indent_less();
    if (toplevel_flag)
    {
        printf
        (
            "trace((\"%s_write_xml(this_thing = %%p)\\n{\\n\", "
            "this_thing));\n",
            def_name().c_str()
        );
    }
    else
    {
        printf
        (
            "trace((\"%s_write_xml(name = \\\"%%s\\\", this_thing = %%p)\\n"
                "{\\n\", name, this_thing));\n",
            def_name().c_str()
        );
        printf("assert(name);\n");
    }
    printf
    (
        "assert(((%s_ty *)this_thing)->reference_count > 0);\n",
        def_name().c_str()
    );
    printf
    (
        "trace((\"rc = %%ld;\\n\", ((%s_ty *)this_thing)->reference_count));\n",
        def_name().c_str()
    );
    if (!toplevel_flag)
    {
        printf("assert(name);\n");
        printf("fp->fputc('<');\n");
        printf("fp->fputs(name);\n");
        printf("fp->fputs(\">\\n\");\n");
    }
    else
        printf("fp->fputs(\"<%s>\\n\");\n",
                      def_name().c_str());
    for (j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_call_xml(ep->name, ep->name, ep->attributes);
    }
    if (!toplevel_flag)
    {
        printf("fp->fputs(\"</\");\n");
        printf("fp->fputs(name);\n");
        printf("fp->fputs(\">\\n\");\n");
    }
    else
    {
        printf
        (
            "fp->fputs(\"</%s>\\n\");\n",
            def_name().c_str()
        );
    }
    printf("}\n");

    include_once("common/mem.h");
    include_once("libaegis/meta_lex.h");
    printf("\n");
    printf("static void *\n");
    printf("%s_alloc(void)\n", def_name().c_str());
    printf("{\n");
    printf("%s_ty *this_thing;\n", def_name().c_str());
    printf("\n");
    printf
    (
        "trace((\"%s_alloc()\\n{\\n\"));\n",
        def_name().c_str()
    );
    printf
    (
        "this_thing = (%s_ty *)mem_alloc(sizeof(%s_ty));\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("this_thing->reference_count = 1;\n");
    printf("this_thing->mask = 0;\n");
    printf("this_thing->errpos = str_copy(lex_position());\n");

    //
    // It is tempting to say
    //
    //     printf("memset(this_this, 0, sizeof(*this_thing));\n");
    //
    // except that the ANCI C standard does NOT guarantee that the NULL
    // pointer is represented in memory as all-bits-zero (even though
    // you spell it "0").  This is also true of floating point zero.
    // As a result, each element is assigned zero individually.
    //
    for (j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        printf
        (
            "this_thing->%s = (%s)0;\n",
            ep->name.c_str(),
            ep->etype->c_name().c_str()
        );
    }
    printf("trace((\"return %%p;\\n\", this_thing));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return this_thing;\n");
    printf("}\n");

    printf("\n");
    printf("%s_ty *\n", def_name().c_str());
    printf
    (
        "%s_copy(%s_ty *this_thing)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf
    (
        "trace((\"%s_copy()\\n{\\n\"));\n",
        def_name().c_str()
    );
    printf("this_thing->reference_count++;\n");
    printf("trace((\"return %%p;\\n\", this_thing));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return this_thing;\n");
    printf("}\n");

    printf("\n");
    printf("%s_ty *\n", def_name().c_str());
    printf
    (
        "%s_clone(%s_ty *this_thing)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("if (!this_thing)\n");
    indent_more();
    printf("return 0;\n");
    indent_less();
    printf
    (
        "trace((\"%s_clone()\\n{\\n\"));\n",
        def_name().c_str()
    );
    printf
    (
        "%s_ty *result = (%s_ty *)%s_alloc();\n",
        def_name().c_str(),
        def_name().c_str(),
        def_name().c_str()
    );

    for (j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_copy(ep->name);
    }

    printf("trace((\"return %%p;\\n\", result));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return result;\n");
    printf("}\n");

    printf("\n#ifdef DEBUG\n\n");
    printf("void\n");
    printf
    (
        "%s_trace_real(const char *name, const %s_ty *value)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("if (name && *name)\n");
    printf("{\n");
    printf("trace_printf(\"%%s = \", name);\n");
    printf("}\n");
    printf("if (!value)\n");
    printf("{\n");
    printf("trace_printf(\"NULL\");\n");
    printf("}\n");
    printf("else\n");
    printf("{\n");
    printf("trace_printf(\"{\\n\");\n");
    for (j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_trace(ep->name, "value->" + ep->name);
    }
    printf("trace_printf(\"}\");\n");
    printf("}\n");
    printf("trace_printf((name && *name) ? \";\\n\" : \",\\n\");\n");
    printf("}\n");
    printf("\n#endif // DEBUG\n");

    printf("\n");
    printf("static void\n");
    printf("%s_free(void *that)\n", def_name().c_str());
    printf("{\n");
    printf("%s_ty *this_thing;\n", def_name().c_str());
    printf("\n");
    printf("this_thing = (%s_ty *)that;\n", def_name().c_str());
    printf("if (!this_thing)\n");
    indent_more();
    printf("return;\n");
    indent_less();
    printf("this_thing->reference_count--;\n");
    printf("assert(this_thing->reference_count >= 0);\n");
    printf("if (this_thing->reference_count > 0)\n");
    indent_more();
    printf("return;\n");
    indent_less();
    printf
    (
        "trace((\"%s_free(this_thing = %%p)\\n{\\n\", this_thing));\n",
        def_name().c_str()
    );
    printf("if (this_thing->errpos)\n");
    printf("{\n");
    printf("str_free(this_thing->errpos);\n");
    printf("this_thing->errpos = 0;\n");
    printf("}\n");

    for (j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        ep->etype->gen_free_declarator(ep->name, false);
    }
    printf("mem_free(this_thing);\n");
    printf("trace((\"}\\n\"));\n");
    printf("}\n");

    printf("\n");
    printf
    (
        "%s %s_table[] =\n", "static type_table_ty",
        def_name().c_str()
    );
    printf("{\n");
    for (j = 0; j < nelements; ++j)
    {
        element_ty *ep = &element[j];
        printf("{\n");
        printf("\"%s\",\n", ep->name.c_str());
        printf
        (
            "offsetof(%s_ty, %s),\n",
            def_name().c_str(),
            ep->name.c_str()
        );
        printf("&%s_type,\n", ep->etype->def_name().c_str());
        printf
        (
            "%s_%s_mask,\n",
            def_name().c_str(),
            ep->name.c_str()
        );
        int redef = !!(ep->attributes & ATTRIBUTE_REDEFINITION_OK);
        printf
        (
            "%d, // redefinition %s ok\n",
            redef,
            (redef ? "is" : "not")
        );
        printf("0, // fast_name\n");
        printf("},\n");
    }
    if (!nelements)
            printf("{ \"\", 0, 0, 0, 0, 0 },\n");
    printf("};\n");

    printf("\n");
    printf("static void *\n");
    printf
    (
        "%s_parse(void *this_thing, string_ty *name, meta_type **type_pp, "
            "unsigned long *mask_p, int *redef_p)\n",
        def_name().c_str()
    );
    printf("{\n");
    printf("void *addr;\n");
    printf("\n");
    printf
    (
        "trace((\"%s_parse(this_thing = %%p, name = %%p, "
        "type_pp = %%p)\\n"
            "{\\n\", this_thing, name, type_pp));\n",
        def_name().c_str()
    );
    printf
    (
        "assert(((%s_ty *)this_thing)->reference_count > 0);\n",
        def_name().c_str()
    );
    printf("addr =\n");
    indent_more();
    printf("generic_struct_parse\n(\n");
    printf("this_thing,\n");
    printf("name,\n");
    printf("type_pp,\n");
    printf("mask_p,\n");
    printf("redef_p,\n");
    printf("%s_table,\n", def_name().c_str());
    printf("SIZEOF(%s_table)\n", def_name().c_str());
    printf(");\n");
    indent_less();
    printf("trace((\"return %%p;\\n}\\n\", addr));\n");
    printf("return addr;\n");
    printf("}\n");

    printf("\n");
    printf("static string_ty *\n");
    printf("%s_fuzzy(string_ty *name)\n", def_name().c_str());
    printf("{\n");
    printf("string_ty *result;\n");
    printf("\n");
    printf
    (
        "trace((\"%s_fuzzy(name = %%p)\\n{\\n\", name));\n",
        def_name().c_str()
    );
    printf("result =\n");
    indent_more();
    printf("generic_struct_fuzzy\n(\n");
    printf("name,\n");
    printf("%s_table,\n", def_name().c_str());
    printf("SIZEOF(%s_table)\n", def_name().c_str());
    printf(");\n");
    indent_less();
    printf("trace((\"return %%p;\\n\", result));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return result;\n");
    printf("}\n");

    printf("\n");
    printf("static rpt_value::pointer\n");
    printf("%s_convert(void *this_thing)\n", def_name().c_str());
    printf("{\n");
    printf
    (
        "trace((\"%s_convert(name = %%p)\\n{\\n\", this_thing));\n",
        def_name().c_str()
    );
    printf
    (
        "assert(((%s_ty *)this_thing)->reference_count > 0);\n",
        def_name().c_str()
    );
    printf("rpt_value::pointer result =\n");
    indent_more();
    printf("generic_struct_convert\n(\n");
    printf("this_thing,\n");
    printf("%s_table,\n", def_name().c_str());
    printf("SIZEOF(%s_table)\n", def_name().c_str());
    printf(");\n");
    indent_less();
    printf("trace((\"return %%p;\\n\", result.get()));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return result;\n");
    printf("}\n");

    printf("\n");
    printf("meta_type %s_type =\n", def_name().c_str());
    printf("{\n");
    printf("\"%s\",\n", def_name().c_str());
    printf("%s_alloc,\n", def_name().c_str());
    printf("%s_free,\n", def_name().c_str());
    printf("0, // enum_parse\n");
    printf("0, // list_parse\n");
    printf("%s_parse,\n", def_name().c_str());
    printf("%s_fuzzy,\n", def_name().c_str());
    printf("%s_convert,\n", def_name().c_str());
    printf("generic_struct_is_set,\n");
    printf("};\n");
}


void
type_structure_code::gen_declarator(const nstring &variable_name,
    bool is_a_list, int, const nstring &) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf("%s_write(fp, ", def_name().c_str());
    if (is_a_list)
            printf("(const char *)0");
    else
            printf("\"%s\"", variable_name.c_str());
    printf(", this_thing->%s);\n", variable_name.c_str());
}


void
type_structure_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf
    (
        "%s_write_xml(fp, \"%s\", this_thing->%s);\n",
        def_name().c_str(),
        form_name.c_str(),
        member_name.c_str()
    );
}


void
type_structure_code::gen_copy(const nstring &member_name)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf
    (
        "result->%s = %s_clone(this_thing->%s);\n",
        member_name.c_str(),
        def_name().c_str(),
        member_name.c_str()
    );
}


void
type_structure_code::gen_free_declarator(const nstring &variable_name, bool)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (is_in_include_file())
    {
        printf
        (
            "%s_type.free(this_thing->%s);\n",
            def_name().c_str(),
            variable_name.c_str()
        );
    }
    else
    {
        printf
        (
            "%s_free(this_thing->%s);\n",
            def_name().c_str(),
            variable_name.c_str()
        );
    }
}


void
type_structure_code::gen_trace(const nstring &vname, const nstring &value)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    printf
    (
        "%s_trace_real(\"%s\", %s);\n",
        def_name().c_str(),
        vname.c_str(),
        value.c_str()
    );
}


// vim: set ts=8 sw=4 et :
