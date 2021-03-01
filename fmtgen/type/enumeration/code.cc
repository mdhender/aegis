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

#include <fmtgen/type/enumeration/code.h>


type_enumeration_code::~type_enumeration_code()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type_enumeration_code::type_enumeration_code(generator *a_gen,
        const nstring &a_name, bool a_global) :
    type_enumeration(a_gen, a_name, a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", def_name().quote_c().c_str()));
}


type::pointer
type_enumeration_code::create(generator *a_gen, const nstring &a_name,
    bool a_global)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new type_enumeration_code(a_gen, a_name, a_global));
}


void
type_enumeration_code::gen_body()
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
    printf
    (
        "static string_ty *%s_f[SIZEOF(%s_s)];\n",
        def_name().c_str(),
        def_name().c_str()
    );

    include_once("common/ac/stdio.h");
    printf("\n");
    printf("const char *\n");
    printf
    (
        "%s_ename(%s_ty this_thing)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("static char buffer[20];\n\n");
    printf("if ((int)this_thing >= 0 && (int)this_thing < %d)\n",
                  (int)elements.size());
    indent_more();
    printf("return %s_s[this_thing];\n", def_name().c_str());
    indent_less();
    printf
    (
        "snprintf(buffer, sizeof(buffer), \"%%d\", (int)this_thing);\n"
    );
    printf("return buffer;\n");
    printf("}\n");

    printf("\n");
    printf("void\n");
    printf
    (
        "%s_write(const output::pointer &fp, const char *name, "
            "%s_ty this_thing, bool show)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("if (this_thing == 0)\n");
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
    printf("fp->fputs(%s_s[this_thing]);\n",
                  def_name().c_str());
    printf("if (name)\n");
    indent_more();
    printf("fp->fputs(\";\\n\");\n");
    indent_less();
    printf("}\n");

    include_once("common/ac/assert.h");
    printf("\n");
    printf("void\n");
    printf
    (
        "%s_write_xml(const output::pointer &fp, const char *name, "
            "%s_ty this_thing, bool show)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("if (this_thing == 0)\n");
    printf("{\n");
    printf("if (!show || type_enum_option_query())\n");
    indent_more();
    printf("return;\n");
    indent_less();
    printf("}\n");
    printf("assert(name);\n");
    printf
    (
        "assert(this_thing < %s_max);\n",
        def_name().c_str()
    );
    printf("fp->fputc('<');\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputc('>');\n");
    printf("fp->fputs(%s_s[this_thing]);\n", def_name().c_str());
    printf("fp->fputs(\"</\");\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputs(\">\\n\");\n");
    printf("}\n");

    include_once("common/str.h"); // for slow_to_fast, etc
    printf("\n");
    printf("static bool\n");
    printf("%s_parse(string_ty *name, void *ptr)\n", def_name().c_str());
    printf("{\n");
    printf
    (
        "slow_to_fast(%s_s, %s_f, SIZEOF(%s_s));\n",
        def_name().c_str(),
        def_name().c_str(),
        def_name().c_str()
    );
    printf
    (
        "for (size_t j = 0; j < SIZEOF(%s_f); ++j)\n",
        def_name().c_str()
    );
    printf("{\n");
    printf("if (str_equal(name, %s_f[j]))\n",
        def_name().c_str());
    printf("{\n");
    printf("*(%s_ty *)ptr = (%s_ty)j;\n", def_name().c_str(),
        def_name().c_str());
    printf("return true;\n");
    printf("}\n");
    printf("}\n");
    printf("return false;\n");
    printf("}\n");

    include_once("libaegis/meta_type.h");
    printf("\n");
    printf("static string_ty *\n");
    printf("%s_fuzzy(string_ty *name)\n", def_name().c_str());
    printf("{\n");
    printf
    (
        "return generic_enum_fuzzy(name, %s_f, SIZEOF(%s_f));\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("}\n");

    printf("\n");
    printf("static rpt_value::pointer \n");
    printf("%s_convert(void *this_thing)\n", def_name().c_str());
    printf("{\n");
    printf("if (!%s_f[0])\n", def_name().c_str());
    indent_more();
    printf
    (
        "slow_to_fast(%s_s, %s_f, SIZEOF(%s_s));\n",
        def_name().c_str(),
        def_name().c_str(),
        def_name().c_str()
    );
    indent_less();
    printf("return\n");
    indent_more();
    printf("generic_enum_convert\n");
    printf("(\n");
    printf("(int)*(%s_ty *)this_thing,\n", def_name().c_str());
    printf("%s_f,\n", def_name().c_str());
    printf("SIZEOF(%s_f)\n", def_name().c_str());
    printf(");\n");
    indent_less();
    printf("}\n");

    printf("\n");
    printf("static bool\n");
    printf("%s_is_set(void *this_thing)\n", def_name().c_str());
    printf("{\n");
    printf("return (*(%s_ty *)this_thing != 0);\n", def_name().c_str());
    printf("}\n");

    printf("\n");
    printf("meta_type %s_type =\n", def_name().c_str());
    printf("{\n");
    printf("\"%s\",\n", def_name().c_str());
    printf("0, // alloc\n");
    printf("0, // free\n");
    printf("%s_parse,\n", def_name().c_str());
    printf("0, // list_parse\n");
    printf("0, // struct_parse\n");
    printf("%s_fuzzy,\n", def_name().c_str());
    printf("%s_convert,\n", def_name().c_str());
    printf("%s_is_set,\n", def_name().c_str());
    printf("};\n");

    printf("\n");
    printf("void\n");
    printf("%s__rpt_init()\n", def_name().c_str());
    printf("{\n");
    printf
    (
        "generic_enum__init(%s_s, SIZEOF(%s_s));\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("}\n");
}


void
type_enumeration_code::gen_report_initializations()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", def_name().quote_c().c_str()));
    printf("%s__rpt_init();\n", def_name().c_str());
}


void
type_enumeration_code::gen_declarator(const nstring &variable_name,
    bool is_a_list, int attributes, const nstring &) const
{
    int show = 1;
    if (attributes & (ATTRIBUTE_SHOW_IF_DEFAULT | ATTRIBUTE_HIDE_IF_DEFAULT))
        show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    printf("%s_write(fp, ", def_name().c_str());
    if (is_a_list)
    {
        printf("(const char *)0");
        show = 1;
    }
    else
    {
        printf("\"%s\"", variable_name.c_str());
    }
    printf(", this_thing->%s, %d);\n", variable_name.c_str(), show);
}


void
type_enumeration_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int attributes) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    trace(("name = %s\n", def_name().quote_c().c_str()));
    int show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    printf
    (
        "%s_write_xml(fp, \"%s\", this_thing->%s, %d);\n",
        def_name().c_str(),
        form_name.c_str(),
        member_name.c_str(),
        show
    );
}


void
type_enumeration_code::gen_free_declarator(const nstring &, bool is_a_list)
    const
{
    if (is_a_list)
        printf(";\n");
}


void
type_enumeration_code::gen_copy(const nstring &member_name)
    const
{
    printf
    (
        "result->%s = this_thing->%s;\n",
        member_name.c_str(),
        member_name.c_str()
    );
}


void
type_enumeration_code::gen_trace(const nstring &vname, const nstring &value)
    const
{
    printf
    (
        "trace_printf(\"%s = %%s;\\n\", %s_ename(%s));\n",
        vname.c_str(),
        def_name().c_str(),
        value.c_str()
    );
}


// vim: set ts=8 sw=4 et :
