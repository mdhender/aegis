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

#include <fmtgen/type/list/code.h>


type_list_code::~type_list_code()
{
}


type_list_code::type_list_code(generator *a_gen, const nstring &a_name,
        bool a_global, const type::pointer &a_subtype) :
    type_list(a_gen, a_name, a_global, a_subtype)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


type::pointer
type_list_code::create(generator *a_gen, const nstring &a_name, bool a_global,
    const type::pointer &a_type)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new type_list_code(a_gen, a_name, a_global, a_type));
}


void
type_list_code::gen_body()
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
    printf
    (
        "%s_write(const output::pointer &fp, const char *name, "
            "%s_ty *this_thing)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("size_t j;\n");
    printf("\n");
    printf("if (!this_thing)\n");
    indent_more();
    printf("return;\n");
    indent_less();
    printf
    (
        "trace((\"%s_write(name = \\\"%%s\\\", this_thing = %%p)\\n"
            "{\\n\", name, this_thing));\n",
        def_name().c_str()
    );
    printf("if (name)\n");
    printf("{\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputs(\" =\\n\");\n");
    printf("}\n");
    printf("assert(this_thing->length <= this_thing->maximum);\n");
    printf("assert(!this_thing->list == !this_thing->maximum);\n");
    printf("fp->fputs(\"[\\n\");\n");
    printf("for (j = 0; j < this_thing->length; ++j)\n");
    printf("{\n");
    subtype->gen_declarator("list[j]", 1, ATTRIBUTE_SHOW_IF_DEFAULT, nstring());
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
    printf
    (
        "%s_write_xml(const output::pointer &fp, const char *name, "
            "%s_ty *this_thing)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("size_t j;\n");
    printf("\n");
    printf("if (!this_thing)\n");
    indent_more();
    printf("return;\n");
    indent_less();
    printf
    (
        "trace((\"%s_write_xml(name = \\\"%%s\\\", this_thing = %%p)\\n"
            "{\\n\", name, this_thing));\n",
        def_name().c_str()
    );
    printf("assert(name);\n");
    printf("fp->fputc('<');\n");
    printf("fp->fputs(name);\n");
    printf("fp->fputs(\">\\n\");\n");
    printf("assert(this_thing->length <= this_thing->maximum);\n");
    printf("assert(!this_thing->list == !this_thing->maximum);\n");
    printf("for (j = 0; j < this_thing->length; ++j)\n");
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

    include_once("common/mem.h");
    printf("\n");
    printf("static void *\n");
    printf("%s_alloc(void)\n", def_name().c_str());
    printf("{\n");
    printf("%s_ty *result;\n\n", def_name().c_str());
    printf("trace((\"%s_alloc()\\n{\\n\"));\n",
                  def_name().c_str());
    printf
    (
        "result = (%s_ty *)mem_alloc(sizeof(%s_ty));\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("result->list = 0;\n");
    printf("result->length = 0;\n");
    printf("result->maximum = 0;\n");
    printf("trace((\"return %%p;\\n\", result));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return result;\n");
    printf("}\n");

    printf("\n");
    printf("static void\n");
    printf("%s_free(void *that)\n", def_name().c_str());
    printf("{\n");
    printf("%s_ty *this_thing;\n", def_name().c_str());
    printf("size_t j;\n");
    printf("\n");
    printf("this_thing = (%s_ty *)that;\n", def_name().c_str());
    printf("if (!this_thing)\n");
    indent_more();
    printf("return;\n");
    indent_less();
    printf
    (
        "trace((\"%s_free(this_thing = %%p)\\n{\\n\", this_thing));\n",
        def_name().c_str()
    );
    printf("assert(this_thing->length <= this_thing->maximum);\n");
    printf("assert(!this_thing->list == !this_thing->maximum);\n");
    printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_more();
    subtype->gen_free_declarator("list[j]", 1);
    indent_less();
    printf("delete [] this_thing->list;\n");
    printf("mem_free(this_thing);\n");
    printf("trace((\"}\\n\"));\n");
    printf("}\n");

    printf("\n");
    printf("static void *\n");
    printf
    (
        "%s_parse(void *that, meta_type **type_pp)\n",
        def_name().c_str()
    );
    printf("{\n");
    printf("%s_ty *this_thing;\n", def_name().c_str());
    printf("void *addr;\n");
    printf("\n");
    printf("this_thing = (%s_ty *)that;\n", def_name().c_str());
    printf
    (
        "trace((\"%s_parse(this_thing = %%p, type_pp = %%p)\\n"
            "{\\n\", this_thing, type_pp));\n",
        def_name().c_str()
    );
    printf("assert(this_thing->length <= this_thing->maximum);\n");
    printf("assert(!this_thing->list == !this_thing->maximum);\n");
    printf("*type_pp = &%s_type;\n", subtype->def_name().c_str());
    printf("trace_pointer(*type_pp);\n");
    printf("if (this_thing->length >= this_thing->maximum)\n");
    printf("{\n");
    printf("this_thing->maximum = this_thing->maximum * 2 + 16;\n");
    printf
    (
        "%s *new_list = new %s [this_thing->maximum];\n",
        subtype->c_name().c_str(),
        subtype->c_name().c_str()
    );
    printf("for (size_t j = 0; j < this_thing->length; ++j)\n");
    indent_more();
    printf("new_list[j] = this_thing->list[j];\n");
    indent_less();
    printf("delete [] this_thing->list;\n");
    printf("this_thing->list = new_list;\n");
    printf("}\n");
    printf("addr = &this_thing->list[this_thing->length++];\n");
    printf("trace((\"return %%p;\\n\", addr));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return addr;\n");
    printf("}\n");

    printf("\n");
    printf("static rpt_value::pointer\n");
    printf("%s_convert(void *that)\n", def_name().c_str());
    printf("{\n");
    printf("%s_ty *this_thing;\n", def_name().c_str());
    printf("size_t j;\n");
    printf("rpt_value::pointer vp;\n");
    printf("\n");
    printf("this_thing = *(%s_ty **)that;\n",
        def_name().c_str());
    printf("if (!this_thing)\n");
    indent_more();
    printf("return rpt_value::pointer();\n");
    indent_less();
    printf
    (
        "trace((\"%s_convert(this_thing = %%p)\\n{\\n\", "
        "this_thing));\n",
        def_name().c_str()
    );
    printf("assert(this_thing->length <= this_thing->maximum);\n");
    printf("assert(!this_thing->list == !this_thing->maximum);\n");
    printf("rpt_value_list *p = new rpt_value_list();\n");
    printf("rpt_value::pointer result(p);\n");
    printf("for (j = 0; j < this_thing->length; ++j)\n");
    printf("{\n");
    printf
    (
        "vp = %s_type.convert(&this_thing->list[j]);\n",
        subtype->def_name().c_str()
    );
    printf("assert(vp);\n");
    printf("p->append(vp);\n");
    printf("}\n");
    printf("trace((\"}\\n\"));\n");
    printf("trace((\"return %%p;\\n\", result.get()));\n");
    printf("return result;\n");
    printf("}\n");

    printf("\n");
    printf("%s_ty *\n", def_name().c_str());
    printf
    (
        "%s_copy(%s_ty *from)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("size_t j;\n");
    printf("%s_ty *result;\n", def_name().c_str());
    printf("\n");
    printf("if (!from)\n");
    indent_more();
    printf("return 0;\n");
    indent_less();
    printf
    (
        "trace((\"%s_copy(from = %%p)\\n{\\n\", from));\n",
        def_name().c_str()
    );
    printf
    (
        "result = (%s_ty *)%s_alloc();\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("assert(from->length <= from->maximum);\n");
    printf("assert(!from->list == !from->maximum);\n");
    printf("for (j = 0; j < from->length; ++j)\n");
    printf("{\n");
    printf("%s mp;\n", subtype->c_name().c_str());
    printf("%s *mpp;\n", subtype->c_name().c_str());
    printf("meta_type *bogus;\n\n");
    printf("mp = from->list[j];\n");
    printf
    (
        "mpp = (%s*)%s_parse(result, &bogus);\n",
        subtype->c_name().c_str(),
        def_name().c_str()
    );
    printf("*mpp = %s_copy(mp);\n", subtype->def_name().c_str());
    printf("}\n");
    printf("trace((\"return %%8.8lX\\n\", (long)result));\n");
    printf("trace((\"}\\n\"));\n");
    printf("return result;\n");
    printf("}\n");

    printf("\n");
    printf("%s_ty *\n", def_name().c_str());
    printf
    (
        "%s_clone(%s_ty *from)\n",
        def_name().c_str(),
        def_name().c_str()
    );
    printf("{\n");
    printf("return %s_copy(from);\n", def_name().c_str());
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
    printf("trace_printf(\"[\\n\");\n");
    printf
    (
        "trace_printf(\"// length = %%ld\\n\", (long)value->length);\n"
    );
    printf
    (
        "trace_printf(\"// maximum = %%ld\\n\", (long)value->maximum);\n"
    );
    printf("assert(value->length <= value->maximum);\n");
    printf("assert(!value->list == !value->maximum);\n");
    printf("for (size_t j = 0; j < value->length; ++j)\n");
    printf("{\n");
    printf("%s mp = value->list[j];\n", subtype->c_name().c_str());
    subtype->gen_trace("", "mp");
    printf("}\n");
    printf("trace_printf(\"]\");\n");
    printf("}\n");
    printf("trace_printf((name && *name) ? \";\\n\" : \",\\n\");\n");
    printf("}\n");
    printf("\n#endif // DEBUG\n");

    printf("\n");
    printf("meta_type %s_type =\n", def_name().c_str());
    printf("{\n");
    printf("\"%s\",\n", def_name().c_str());
    printf("%s_alloc,\n", def_name().c_str());
    printf("%s_free,\n", def_name().c_str());
    printf("0, // enum_parse\n");
    printf("%s_parse,\n", def_name().c_str());
    printf("0, // struct_parse\n");
    printf("0, // fuzzy\n");
    printf("%s_convert,\n", def_name().c_str());
    printf("generic_struct_is_set,\n");
    printf("};\n");
}


void
type_list_code::gen_declarator(const nstring &variable_name, bool is_a_list,
    int, const nstring &) const
{
    assert(subtype);
    printf("%s_write(fp, ", def_name().c_str());
    if (is_a_list)
        printf("\"\"");
    else
        printf("\"%s\"", variable_name.c_str());
    printf(", this_thing->%s);\n", variable_name.c_str());
}


void
type_list_code::gen_call_xml(const nstring &form_name,
    const nstring &member_name, int) const
{
    assert(subtype);
    printf
    (
        "%s_write_xml(fp, \"%s\", this_thing->%s);\n",
        def_name().c_str(),
        form_name.c_str(),
        member_name.c_str()
    );
}


void
type_list_code::gen_copy(const nstring &member_name)
    const
{
    assert(subtype);
    printf
    (
        "result->%s = %s_clone(this_thing->%s);\n",
        member_name.c_str(),
        def_name().c_str(),
        member_name.c_str()
    );
}


void
type_list_code::gen_free_declarator(const nstring &variable_name, bool)
    const
{
    assert(subtype);
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
type_list_code::gen_trace(const nstring &vname, const nstring &value)
    const
{
    printf
    (
        "%s_trace_real(\"%s\", %s);\n",
        def_name().c_str(),
        vname.c_str(),
        value.c_str()
    );
}


// vim: set ts=8 sw=4 et :
