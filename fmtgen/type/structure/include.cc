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

#include <fmtgen/type/structure/include.h>


type_structure_include::~type_structure_include()
{
}


type_structure_include::type_structure_include(generator *a_gen,
        const nstring &a_name, bool a_global) :
    type_structure(a_gen, a_name, a_global)
{
}


type::pointer
type_structure_include::create(generator *a_gen, const nstring &a_name,
    bool a_global)
{
    return pointer(new type_structure_include(a_gen, a_name, a_global));
}


void
type_structure_include::gen_body()
    const
{
    printf("\n");
    printf("#ifndef %s_DEF\n", def_name().c_str());
    printf("#define %s_DEF\n", def_name().c_str());
    printf("\n");
    int bit = 0;
    size_t j;
    for (j = 0; j < nelements; ++j)
    {
        printf
        (
            "#define\t%s_%s_mask\t",
            def_name().c_str(),
            element[j].name.c_str()
        );
        if (element[j].etype->has_a_mask())
            printf("((unsigned long)1 << %d)", bit++);
        else
            printf("((unsigned long)0)");
        printf("\n");
    }
    printf("\n");
    include_once("common/str.h");
    printf("struct %s_ty\n", def_name().c_str());
    printf("{\n");

    //
    // The next 3 fields must agree EXACTLY with generic_structure_ty
    // which is defined in libaegis/type.h
    //
    printf("long reference_count;\n");
    printf("unsigned long mask;\n");
    printf("string_ty *errpos;\n");

    for (j = 0; j < nelements; ++j)
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
    printf("};\n");
    printf("#endif // %s_DEF\n", def_name().c_str());

    printf("\n");
    include_once("libaegis/meta_type.h");
    printf("extern meta_type %s_type;\n", def_name().c_str());

    printf("\n");
    include_once("libaegis/output.h");
    if (toplevel_flag)
    {
        printf
        (
            "void %s_write(const output::pointer &fp, %s_ty *value);\n",
            def_name().c_str(),
            def_name().c_str()
        );
        printf
        (
            "void %s_write_xml(const output::pointer &fp, %s_ty *value);\n",
            def_name().c_str(),
            def_name().c_str()
        );
    }
    else
    {
        printf
        (
            "void %s_write(const output::pointer &fp, const char *name, "
                "%s_ty *value);\n",
            def_name().c_str(),
            def_name().c_str()
        );
        printf
        (
            "void %s_write_xml(const output::pointer &fp, const char *name, "
                "%s_ty *value);\n",
            def_name().c_str(),
            def_name().c_str()
        );
    }
    printf
    (
        "%s_ty *%s_copy(%s_ty *);\n",
        def_name().c_str(),
        def_name().c_str(),
        def_name().c_str()
    );
    printf
    (
        "%s_ty *%s_clone(%s_ty *);\n",
        def_name().c_str(),
        def_name().c_str(),
        def_name().c_str()
    );
    include_once("common/debug.h");
    printf("#ifndef %s_trace\n", def_name().c_str());
    printf("#ifdef DEBUG\n");
    printf
    (
        "void %s_trace_real(const char *name, const %s_ty *value);\n",
        def_name().c_str(),
        def_name().c_str()
    );
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
type_structure_include::gen_declarator(const nstring &variable_name,
    bool is_a_list, int, const nstring &) const
{
    const char *deref = (is_a_list ? "*" : "");
    printf
    (
        "%s_ty %s*%s;\n",
        def_name().c_str(),
        deref,
        variable_name.c_str()
    );
}


// vim: set ts=8 sw=4 et :
