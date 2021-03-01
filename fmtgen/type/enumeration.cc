//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1997-1999, 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <fmtgen/indent.h>
#include <common/mem.h>
#include <fmtgen/parse.h>
#include <fmtgen/type/enumeration.h>


type_enumeration::~type_enumeration()
{
}


type_enumeration::type_enumeration(const nstring &a_name) :
    type(a_name)
{
}


void
type_enumeration::gen_include()
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
                  (unsigned)elements.size());
    indent_printf("#endif // %s_DEF\n", def_name().c_str());

    indent_putchar('\n');
    indent_printf("extern meta_type %s_type;\n", def_name().c_str());

    indent_putchar('\n');
    indent_printf
    (
	"void %s_write(const output::pointer &fp, const char *name, "
            "%s_ty value, bool show);\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"void %s_write_xml(const output::pointer &fp, const char *name, "
            "%s_ty value, bool show);\n",
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
type_enumeration::gen_include_declarator(const nstring &variable_name,
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
type_enumeration::gen_code()
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
                  (int)elements.size());
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
	"%s_write(const output::pointer &fp, const char *name, "
            "%s_ty this_thing, bool show)\n",
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
    indent_printf("fp->fputs(name);\n");
    indent_printf("fp->fputs(\" = \");\n");
    indent_printf("}\n");
    indent_printf("fp->fputs(%s_s[this_thing]);\n",
                  def_name().c_str());
    indent_printf("if (name)\n");
    indent_more();
    indent_printf("fp->fputs(\";\\n\");\n");
    indent_less();
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write_xml(const output::pointer &fp, const char *name, "
            "%s_ty this_thing, bool show)\n",
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
    indent_printf("fp->fputc('<');\n");
    indent_printf("fp->fputs(name);\n");
    indent_printf("fp->fputc('>');\n");
    indent_printf("fp->fputs(%s_s[this_thing]);\n", def_name().c_str());
    indent_printf("fp->fputs(\"</\");\n");
    indent_printf("fp->fputs(name);\n");
    indent_printf("fp->fputs(\">\\n\");\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static bool\n");
    indent_printf("%s_parse(string_ty *name, void *ptr)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf
    (
	"slow_to_fast(%s_s, %s_f, SIZEOF(%s_s));\n",
	def_name().c_str(),
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"for (size_t j = 0; j < SIZEOF(%s_f); ++j)\n",
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("if (str_equal(name, %s_f[j]))\n",
	def_name().c_str());
    indent_printf("{\n");
    indent_printf("*(%s_ty *)ptr = (%s_ty)j;\n", def_name().c_str(),
	def_name().c_str());
    indent_printf("return true;\n");
    indent_printf("}\n");
    indent_printf("}\n");
    indent_printf("return false;\n");
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
    indent_printf("static rpt_value::pointer \n");
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
    indent_printf("(int)*(%s_ty *)this_thing,\n", def_name().c_str());
    indent_printf("%s_f,\n", def_name().c_str());
    indent_printf("SIZEOF(%s_f)\n", def_name().c_str());
    indent_printf(");\n");
    indent_less();
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static bool\n");
    indent_printf("%s_is_set(void *this_thing)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("return (*(%s_ty *)this_thing != 0);\n", def_name().c_str());
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("meta_type %s_type =\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("\"%s\",\n", def_name().c_str());
    indent_printf("0, // alloc\n");
    indent_printf("0, // free\n");
    indent_printf("%s_parse,\n", def_name().c_str());
    indent_printf("0, // list_parse\n");
    indent_printf("0, // struct_parse\n");
    indent_printf("%s_fuzzy,\n", def_name().c_str());
    indent_printf("%s_convert,\n", def_name().c_str());
    indent_printf("%s_is_set,\n", def_name().c_str());
    indent_printf("};\n");
}


void
type_enumeration::gen_code_declarator(const nstring &variable_name,
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
type_enumeration::gen_code_call_xml(const nstring &form_name,
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
type_enumeration::gen_free_declarator(const nstring &, bool is_a_list)
    const
{
    if (is_a_list)
       	indent_printf(";\n");
}


void
type_enumeration::member_add(const nstring &member_name, type *, int)
{
    elements.push_back(member_name);
}


nstring
type_enumeration::c_name_inner()
    const
{
    return nstring::format("%s_ty", def_name().c_str());
}


bool
type_enumeration::has_a_mask()
    const
{
    return true;
}


void
type_enumeration::gen_code_copy(const nstring &member_name)
    const
{
    indent_printf
    (
	"result->%s = this_thing->%s;\n",
	member_name.c_str(),
	member_name.c_str()
    );
}


void
type_enumeration::gen_code_trace(const nstring &vname, const nstring &value)
    const
{
    indent_printf
    (
	"trace_printf(\"%s = %%s;\\n\", %s_ename(%s));\n",
	vname.c_str(),
	def_name().c_str(),
	value.c_str()
    );
}
