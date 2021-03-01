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

#include <fmtgen/type/enumeration/include.h>


type_enumeration_include::~type_enumeration_include()
{
}


type_enumeration_include::type_enumeration_include(generator *a_gen,
        const nstring &a_name, bool a_global) :
    type_enumeration(a_gen, a_name, a_global)
{
}


type::pointer
type_enumeration_include::create(generator *a_gen, const nstring &a_name,
    bool a_global)
{
    return pointer(new type_enumeration_include(a_gen, a_name, a_global));
}


void
type_enumeration_include::gen_body()
    const
{
    include_once("libaegis/meta_type.h");
    include_once("libaegis/output.h");

    printf("\n");
    printf("#ifndef %s_DEF\n", def_name().c_str());
    printf("#define %s_DEF\n", def_name().c_str());
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
    printf("#define %s_max %u\n",
                  def_name().c_str(),
                  (unsigned)elements.size());
    printf("#endif // %s_DEF\n", def_name().c_str());

    printf("\n");
    printf("extern meta_type %s_type;\n", def_name().c_str());

    printf("\n");
    printf
    (
        "void %s_write(const output::pointer &fp, const char *name, "
            "%s_ty value, bool show);\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf
    (
        "void %s_write_xml(const output::pointer &fp, const char *name, "
            "%s_ty value, bool show);\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf
    (
        "const char *%s_ename(%s_ty);\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("void %s__rpt_init(void);\n", def_name().c_str());
}


void
type_enumeration_include::gen_declarator(const nstring &variable_name,
    bool is_a_list, int, const nstring &) const
{
    printf
    (
        "%s_ty %s%s;\n",
        def_name().c_str(),
        (is_a_list ? "*" : ""),
        variable_name.c_str()
    );
}
