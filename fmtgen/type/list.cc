//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1998, 1999, 2001-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate list types
//

#pragma implementation "type_list_ty"

#include <error.h> // for assert
#include <indent.h>
#include <mem.h>
#include <type/list.h>


type_list_ty::~type_list_ty()
{
}


type_list_ty::type_list_ty(const nstring &a_name, type_ty *a_type) :
    type_ty(a_name),
    subtype(a_type)
{
}


void
type_list_ty::gen_include()
    const
{
    assert(subtype);
    indent_putchar('\n');
    indent_printf("#ifndef %s_DEF\n", def_name().c_str());
    indent_printf("#define %s_DEF\n", def_name().c_str());
    indent_printf("struct %s_ty\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("size_t\1length;\n");
    indent_printf("size_t\1maximum;\n");
    subtype->gen_include_declarator("list", 1);
    indent_printf("};\n");
    indent_printf("#endif /* %s_DEF */\n", def_name().c_str());

    indent_putchar('\n');
    indent_printf("extern type_ty %s_type;\n", def_name().c_str());

    indent_putchar('\n');
    indent_printf
    (
	"%s_ty *%s_copy(%s_ty *);\n",
	def_name().c_str(),
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"void %s_write(struct output_ty *, const char *, %s_ty *);\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"void %s_write_xml(struct output_ty *, const char *, %s_ty *);\n",
	def_name().c_str(),
	def_name().c_str()
    );
}


void
type_list_ty::gen_include_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    assert(subtype);
    const char *deref = (is_a_list ? "*" : "");
    indent_printf
    (
	"%s_ty\1%s*%s;\n",
	def_name().c_str(),
	deref,
	variable_name.c_str()
    );
}


void
type_list_ty::gen_code()
    const
{
    assert(subtype);
    static bool lincl_done;
    if (!lincl_done)
    {
	indent_putchar('\n');
	indent_printf("#include <aer/value/list.h>\n");
	lincl_done = true;
    }

    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write(output_ty *fp, const char *name, %s_ty *this_thing)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("size_t\1j;\n");
    indent_putchar('\n');
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_write(name = \\\"%%s\\\", this_thing = %%08lX)\\n"
	    "{\\n\", name, (long)this_thing));\n",
	def_name().c_str()
    );
    indent_printf("if (name)\n");
    indent_printf("{\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \" =\\n\");\n");
    indent_printf("}\n");
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("output_fputs(fp, \"[\\n\");\n");
    indent_printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_printf("{\n");
    subtype->gen_code_declarator("list[j]", 1, ATTRIBUTE_SHOW_IF_DEFAULT);
    indent_printf("output_fputs(fp, \",\\n\");\n");
    indent_printf("}\n");
    indent_printf("output_fputs(fp, \"]\");\n");
    indent_printf("if (name)\n");
    indent_more();
    indent_printf("output_fputs(fp, \";\\n\");\n");
    indent_less();
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write_xml(output_ty *fp, const char *name, %s_ty *this_thing)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("size_t\1j;\n");
    indent_putchar('\n');
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_write_xml(name = \\\"%%s\\\", this_thing = %%08lX)\\n"
	    "{\\n\", name, (long)this_thing));\n",
	def_name().c_str()
    );
    indent_printf("assert(name);\n");
    indent_printf("output_fputc(fp, '<');\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \">\\n\");\n");
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_printf("{\n");
    subtype->gen_code_call_xml
    (
	subtype->def_name(),
	"list[j]",
	ATTRIBUTE_SHOW_IF_DEFAULT
    );
    indent_printf("}\n");
    indent_printf("output_fputs(fp, \"</\");\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \">\\n\");\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void *\n");
    indent_printf("%s_alloc(void)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("%s_ty\1*result;\n\n", def_name().c_str());
    indent_printf("trace((\"%s_alloc()\\n{\\n\"));\n",
                  def_name().c_str());
    indent_printf
    (
	"result = (%s_ty *)mem_alloc(sizeof(%s_ty));\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("result->list = 0;\n");
    indent_printf("result->length = 0;\n");
    indent_printf("result->maximum = 0;\n");
    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void\n");
    indent_printf("%s_free(void *that)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing;\n", def_name().c_str());
    indent_printf("size_t\1j;\n");
    indent_putchar('\n');
    indent_printf("this_thing = (%s_ty *)that;\n", def_name().c_str());
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_free(this_thing = %%08lX)\\n{\\n\", (long)this_thing));\n",
	def_name().c_str()
    );
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_more();
    subtype->gen_free_declarator("list[j]", 1);
    indent_less();
    indent_printf("if (this_thing->list)\n");
    indent_more();
    indent_printf("mem_free(this_thing->list);\n");
    indent_less();
    indent_printf("mem_free(this_thing);\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void *\n");
    indent_printf
    (
	"%s_parse(void *that, type_ty **type_pp)\n",
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing;\n", def_name().c_str());
    indent_printf("void\1*addr;\n");
    indent_putchar('\n');
    indent_printf("this_thing = (%s_ty *)that;\n", def_name().c_str());
    indent_printf
    (
	"trace((\"%s_parse(this_thing = %%08lX, type_pp = %%08lX)\\n"
	    "{\\n\", (long)this_thing, (long)type_pp));\n",
	def_name().c_str()
    );
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("*type_pp = &%s_type;\n", subtype->def_name().c_str());
    indent_printf("trace_pointer(*type_pp);\n");
    indent_printf("if (this_thing->length >= this_thing->maximum)\n");
    indent_printf("{\n");
    indent_printf("size_t\1nbytes;\n\n");
    indent_printf("this_thing->maximum = this_thing->maximum * 2 + 16;\n");
    indent_printf(
        "nbytes = this_thing->maximum * sizeof(this_thing->list[0]);\n");
    indent_printf
    (
	"this_thing->list = "
	    "(%s*)mem_change_size((void *)this_thing->list, nbytes);\n",
	subtype->c_name().c_str()
    );
    indent_printf("}\n");
    indent_printf("addr = &this_thing->list[this_thing->length++];\n");
    indent_printf("trace((\"return %%08lX;\\n\", (long)addr));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return addr;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static rpt_value_ty *\n");
    indent_printf("%s_convert(void *that)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing;\n", def_name().c_str());
    indent_printf("rpt_value_ty\1*result;\n");
    indent_printf("size_t\1j;\n");
    indent_printf("rpt_value_ty\1*vp;\n");
    indent_putchar('\n');
    indent_printf("this_thing = *(%s_ty **)that;\n",
	def_name().c_str());
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return 0;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_convert(this_thing = %%08lX)\\n{\\n\", "
        "(long)this_thing));\n",
	def_name().c_str()
    );
    indent_printf("assert(this_thing->length <= this_thing->maximum);\n");
    indent_printf("assert(!this_thing->list == !this_thing->maximum);\n");
    indent_printf("result = rpt_value_list();\n");
    indent_printf("for (j = 0; j < this_thing->length; ++j)\n");
    indent_printf("{\n");
    indent_printf
    (
	"vp = %s_type.convert(&this_thing->list[j]);\n",
	subtype->def_name().c_str()
    );
    indent_printf("assert(vp);\n");
    indent_printf("rpt_value_list_append(result, vp);\n");
    indent_printf("rpt_value_free(vp);\n");
    indent_printf("}\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("%s_ty *\n", def_name().c_str());
    indent_printf
    (
	"%s_copy(%s_ty *from)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("size_t\1j;\n");
    indent_printf("%s_ty\1*result;\n", def_name().c_str());
    indent_putchar('\n');
    indent_printf("if (!from)\n");
    indent_more();
    indent_printf("return 0;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_copy(from = %%08lX)\\n{\\n\", (long)from));\n",
	def_name().c_str()
    );
    indent_printf
    (
	"result = (%s_ty *)%s_alloc();\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("assert(from->length <= from->maximum);\n");
    indent_printf("assert(!from->list == !from->maximum);\n");
    indent_printf("for (j = 0; j < from->length; ++j)\n");
    indent_printf("{\n");
    indent_printf("%s\1mp;\n", subtype->c_name().c_str());
    indent_printf("%s\1*mpp;\n", subtype->c_name().c_str());
    indent_printf("type_ty\1*bogus;\n\n");
    indent_printf("mp = from->list[j];\n");
    indent_printf
    (
	"mpp = (%s*)%s_parse(result, &bogus);\n",
	subtype->c_name().c_str(),
	def_name().c_str()
    );
    indent_printf("*mpp = %s_copy(mp);\n", subtype->def_name().c_str());
    indent_printf("}\n");
    indent_printf("trace((\"return %%8.8lX\\n\", (long)result));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("type_ty %s_type =\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("\"%s\",\n", def_name().c_str());
    indent_printf("%s_alloc,\n", def_name().c_str());
    indent_printf("%s_free,\n", def_name().c_str());
    indent_printf("0, /* enum_parse */\n");
    indent_printf("%s_parse,\n", def_name().c_str());
    indent_printf("0, /* struct_parse */\n");
    indent_printf("0, /* fuzzy */\n");
    indent_printf("%s_convert,\n", def_name().c_str());
    indent_printf("generic_struct_is_set,\n");
    indent_printf("};\n");
}


void
type_list_ty::gen_code_declarator(const nstring &variable_name, bool is_a_list,
    int attributes) const
{
    assert(subtype);
    indent_printf("%s_write(fp, ", def_name().c_str());
    if (is_a_list)
       	indent_printf("\"\"");
    else
       	indent_printf("\"%s\"", variable_name.c_str());
    indent_printf(", this_thing->%s);\n", variable_name.c_str());
}


void
type_list_ty::gen_code_call_xml(const nstring &form_name,
    const nstring &member_name, int show) const
{
    assert(subtype);
    indent_printf
    (
	"%s_write_xml(fp, \"%s\", this_thing->%s);\n",
	def_name().c_str(),
	form_name.c_str(),
	member_name.c_str()
    );
}


void
type_list_ty::gen_free_declarator(const nstring &variable_name, bool is_a_list)
    const
{
    assert(subtype);
    if (is_in_include_file())
    {
	indent_printf
	(
    	    "%s_type.free(this_thing->%s);\n",
    	    def_name().c_str(),
    	    variable_name.c_str()
	);
    }
    else
    {
	indent_printf
	(
    	    "%s_free(this_thing->%s);\n",
    	    def_name().c_str(),
    	    variable_name.c_str()
	);
    }
}


void
type_list_ty::in_include_file()
{
    assert(subtype);
    type_ty::in_include_file();
    subtype->in_include_file();
}


nstring
type_list_ty::c_name_inner()
    const
{
    return str_format("%s_ty *", def_name().c_str());
}


bool
type_list_ty::has_a_mask()
    const
{
    return false;
}
