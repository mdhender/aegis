//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1997-1999, 2001-2004 Peter Miller;
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
// MANIFEST: enumerated types functions generation
//

#pragma implementation "type_enumeration_ty"

#include <indent.h>
#include <mem.h>
#include <parse.h>
#include <type/enumeration.h>


type_enumeration_ty::~type_enumeration_ty()
{
}


type_enumeration_ty::type_enumeration_ty(const nstring &a_name) :
    type_ty(a_name)
{
}


void
type_enumeration_ty::gen_include()
    const
{
    indent_putchar('\n');
    indent_printf("#ifndef %s_DEF\n", def_name().c_str());
    indent_printf("#define %s_DEF\n", def_name().c_str());
    indent_printf("enum %s_ty\n", def_name().c_str());
    indent_printf("{\n");
    for (size_t j = 0; j < elements.size(); ++j)
    {
	indent_printf
	(
	    "%s_%s",
	    def_name().c_str(),
	    elements[j].c_str()
	);
	if (j < elements.size() - 1)
	    indent_putchar(',');
	indent_putchar('\n');
    }
    indent_printf("};\n");
    indent_printf("#define %s_max %u\n",
                  def_name().c_str(),
                  elements.size());
    indent_printf("#endif // %s_DEF\n", def_name().c_str());

    indent_putchar('\n');
    indent_printf("extern type_ty %s_type;\n", def_name().c_str());

    indent_putchar('\n');
    indent_printf
    (
	"void %s_write(struct output_ty *, const char *, %s_ty, int);\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"void %s_write_xml(struct output_ty *, const char *, %s_ty, int);\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"const char *%s_ename(%s_ty);\n",
	def_name().c_str(),
	def_name().c_str()
    );
}


void
type_enumeration_ty::gen_include_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    indent_printf
    (
       	"%s_ty\1%s%s;\n",
       	def_name().c_str(),
       	(is_a_list ? "*" : ""),
       	variable_name.c_str()
    );
}


void
type_enumeration_ty::gen_code()
    const
{
    indent_putchar('\n');
    indent_printf("static const char *%s_s[] =\n", def_name().c_str());
    indent_printf("{\n");
    for (size_t j = 0; j < elements.size(); ++j)
	indent_printf("\"%s\",\n", elements[j].c_str());
    indent_printf("};\n");
    indent_printf
    (
	"static string_ty\1*%s_f[SIZEOF(%s_s)];\n",
	def_name().c_str(),
	def_name().c_str()
    );

    indent_putchar('\n');
    indent_printf("const char *\n");
    indent_printf
    (
	"%s_ename(%s_ty this_thing)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("static char\1buffer[20];\n\n");
    indent_printf("if ((int)this_thing >= 0 && (int)this_thing < %d)\n",
                  elements.size());
    indent_more();
    indent_printf("return %s_s[this_thing];\n", def_name().c_str());
    indent_less();
    indent_printf
    (
	"snprintf(buffer, sizeof(buffer), \"%%d\", (int)this_thing);\n"
    );
    indent_printf("return buffer;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write(output_ty *fp, const char *name, %s_ty this_thing, "
        "int show)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("if (this_thing == 0)\n");
    indent_printf("{\n");
    indent_printf("if (!show || type_enum_option_query())\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf("}\n");
    indent_printf("if (name)\n");
    indent_printf("{\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \" = \");\n");
    indent_printf("}\n");
    indent_printf("output_fputs(fp, %s_s[this_thing]);\n",
                  def_name().c_str());
    indent_printf("if (name)\n");
    indent_more();
    indent_printf("output_fputs(fp, \";\\n\");\n");
    indent_less();
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write_xml(output_ty *fp, const char *name, %s_ty this_thing, "
        "int show)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("if (this_thing == 0)\n");
    indent_printf("{\n");
    indent_printf("if (!show || type_enum_option_query())\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf("}\n");
    indent_printf("assert(name);\n");
    indent_printf
    (
        "assert((size_t)this_thing < SIZEOF(%s_s));\n",
	def_name().c_str()
    );
    indent_printf("output_fputc(fp, '<');\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputc(fp, '>');\n");
    indent_printf(
        "output_fputs(fp, %s_s[this_thing]);\n", def_name().c_str());
    indent_printf("output_fputs(fp, \"</\");\n");
    indent_printf("output_fputs(fp, name);\n");
    indent_printf("output_fputs(fp, \">\\n\");\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static int\n");
    indent_printf("%s_parse(string_ty *name)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("size_t\1j;\n");
    indent_putchar('\n');
    indent_printf
    (
	"slow_to_fast(%s_s, %s_f, SIZEOF(%s_s));\n",
	def_name().c_str(),
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"for (j = 0; j < SIZEOF(%s_f); ++j)\n",
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("if (str_equal(name, %s_f[j]))\n",
                  def_name().c_str());
    indent_more();
    indent_printf("return j;\n");
    indent_less();
    indent_printf("}\n");
    indent_printf("return -1;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static string_ty *\n");
    indent_printf("%s_fuzzy(string_ty *name)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf
    (
	"return generic_enum_fuzzy(name, %s_f, SIZEOF(%s_f));\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("}\n");

    generate_code__init
    (
	nstring::format
	(
	    "generic_enum__init(%s_s, SIZEOF(%s_s));",
	    def_name().c_str(),
	    def_name().c_str()
	)
    );

    indent_putchar('\n');
    indent_printf("static struct rpt_value_ty *\n");
    indent_printf("%s_convert(void *this_thing)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("if (!%s_f[0])\n", def_name().c_str());
    indent_more();
    indent_printf
    (
	"slow_to_fast(%s_s, %s_f, SIZEOF(%s_s));\n",
	def_name().c_str(),
	def_name().c_str(),
	def_name().c_str()
    );
    indent_less();
    indent_printf("return\n");
    indent_more();
    indent_printf("generic_enum_convert\n");
    indent_printf("(\n");
    indent_printf("this_thing,\n");
    indent_printf("%s_f,\n", def_name().c_str());
    indent_printf("SIZEOF(%s_f)\n", def_name().c_str());
    indent_printf(");\n");
    indent_less();
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("type_ty %s_type =\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("\"%s\",\n", def_name().c_str());
    indent_printf("0, // alloc\n");
    indent_printf("0, // free\n");
    indent_printf("%s_parse,\n", def_name().c_str());
    indent_printf("0, // list_parse\n");
    indent_printf("0, // struct_parse\n");
    indent_printf("%s_fuzzy,\n", def_name().c_str());
    indent_printf("%s_convert,\n", def_name().c_str());
    indent_printf("generic_enum_is_set,\n");
    indent_printf("};\n");
}


void
type_enumeration_ty::gen_code_declarator(const nstring &variable_name,
    bool is_a_list, int attributes) const
{
    int show = 1;
    if (attributes & (ATTRIBUTE_SHOW_IF_DEFAULT | ATTRIBUTE_HIDE_IF_DEFAULT))
	show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);

    indent_printf("%s_write(fp, ", def_name().c_str());
    if (is_a_list)
    {
	indent_printf("(const char *)0");
	show = 1;
    }
    else
    {
	indent_printf("\"%s\"", variable_name.c_str());
    }
    indent_printf(", this_thing->%s, %d);\n", variable_name.c_str(), show);
}


void
type_enumeration_ty::gen_code_call_xml(const nstring &form_name,
    const nstring &member_name, int attributes) const
{
    int show = !!(attributes & ATTRIBUTE_SHOW_IF_DEFAULT);
    indent_printf
    (
	"%s_write_xml(fp, \"%s\", this_thing->%s, %d);\n",
	def_name().c_str(),
	form_name.c_str(),
	member_name.c_str(),
	show
    );
}


void
type_enumeration_ty::gen_free_declarator(const nstring &variable_name,
    bool is_a_list) const
{
    if (is_a_list)
       	indent_printf(";\n");
}


void
type_enumeration_ty::member_add(const nstring &member_name,
    type_ty *member_type, int attributes)
{
    elements.push_back(member_name);
}


nstring
type_enumeration_ty::c_name_inner()
    const
{
    return nstring::format("%s_ty", def_name().c_str());
}


bool
type_enumeration_ty::has_a_mask()
    const
{
    return true;
}
