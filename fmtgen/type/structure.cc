//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1998, 1999, 2001-2008 Peter Miller
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

#include <common/error.h>
#include <fmtgen/indent.h>
#include <common/mem.h>
#include <fmtgen/type/structure.h>


type_structure::~type_structure()
{
    delete [] element;
    nelements = 0;
    nelements_max = 0;
    element = 0;
    toplevel_flag = false;
}


type_structure::type_structure(const nstring &a_name) :
    type(a_name),
    nelements(0),
    nelements_max(0),
    element(0),
    toplevel_flag(false)
{
}



void
type_structure::gen_include()
    const
{
    indent_putchar('\n');
    indent_printf("#ifndef %s_DEF\n", def_name().c_str());
    indent_printf("#define %s_DEF\n", def_name().c_str());
    indent_printf("\n");
    int bit = 0;
    size_t j;
    for (j = 0; j < nelements; ++j)
    {
	indent_printf
	(
	    "#define\t%s_%s_mask\t",
	    def_name().c_str(),
	    element[j].name.c_str()
	);
	if (element[j].etype->has_a_mask())
	    indent_printf("((unsigned long)1 << %d)", bit++);
	else
	    indent_printf("((unsigned long)0)");
	indent_printf("\n");
    }
    indent_printf("\n");
    indent_printf("struct %s_ty\n", def_name().c_str());
    indent_printf("{\n");

    //
    // The next 3 fields must agree EXACTLY with generic_structure_ty
    // which is defined in libaegis/type.h
    //
    indent_printf("long\1reference_count;\n");
    indent_printf("unsigned long\1mask;\n");
    indent_printf("string_ty\1*errpos;\n");

    for (j = 0; j < nelements; ++j)
    {
	element_ty *ep = &element[j];
	ep->etype->gen_include_declarator(ep->name, false);
    }
    indent_printf("};\n");
    indent_printf("#endif // %s_DEF\n", def_name().c_str());

    indent_putchar('\n');
    indent_printf("extern meta_type %s_type;\n", def_name().c_str());

    indent_putchar('\n');
    if (toplevel_flag)
    {
	indent_printf
	(
    	    "void %s_write(const output::pointer &fp, %s_ty *value);\n",
    	    def_name().c_str(),
    	    def_name().c_str()
	);
	indent_printf
	(
    	    "void %s_write_xml(const output::pointer &fp, %s_ty *value);\n",
    	    def_name().c_str(),
    	    def_name().c_str()
	);
    }
    else
    {
	indent_printf
	(
    	    "void %s_write(const output::pointer &fp, const char *name, "
                "%s_ty *value);\n",
    	    def_name().c_str(),
    	    def_name().c_str()
	);
	indent_printf
	(
    	    "void %s_write_xml(const output::pointer &fp, const char *name, "
                "%s_ty *value);\n",
    	    def_name().c_str(),
    	    def_name().c_str()
	);
    }
    indent_printf
    (
	"%s_ty *%s_copy(%s_ty *);\n",
	def_name().c_str(),
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"%s_ty *%s_clone(%s_ty *);\n",
	def_name().c_str(),
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("#ifdef DEBUG\n");
    indent_printf
    (
	"void %s_trace_real(const char *name, const %s_ty *value);\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf
    (
	"#define %s_trace(x) ((void)(trace_pretest_ && (trace_where_, "
	    "%s_trace_real(trace_stringize(x), x), 0)))\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("#else\n");
    indent_printf("#define %s_trace(x)\n", def_name().c_str());
    indent_printf("#endif\n");
}


void
type_structure::gen_include_declarator(const nstring &variable_name,
    bool is_a_list) const
{
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
type_structure::gen_code()
    const
{
    size_t j;
    indent_putchar('\n');
    indent_printf("void\n");
    if (toplevel_flag)
    {
	indent_printf
	(
	    "%s_write(const output::pointer &fp, %s_ty *this_thing)\n",
	    def_name().c_str(),
	    def_name().c_str()
	);
    }
    else
    {
	indent_printf
	(
	    "%s_write(const output::pointer &fp, const char *name, "
                "%s_ty *this_thing)\n",
	    def_name().c_str(),
	    def_name().c_str()
	);
    }
    indent_printf("{\n");
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    if (toplevel_flag)
    {
	indent_printf
	(
            "trace((\"%s_write(this_thing = %%08lX)\\n{\\n\", "
            "(long)this_thing));\n",
	    def_name().c_str()
	);
    }
    else
    {
	indent_printf
	(
	    "trace((\"%s_write(name = \\\"%%s\\\", this_thing = %%08lX)\\n"
	    "{\\n\", name, (long)this_thing));\n",
	    def_name().c_str()
	);
    }
    indent_printf
    (
	"assert(((%s_ty *)this_thing)->reference_count > 0);\n",
	def_name().c_str()
    );
    indent_printf
    (
	"trace((\"rc = %%ld;\\n\", ((%s_ty *)this_thing)->reference_count));\n",
	def_name().c_str()
    );
    if (!toplevel_flag)
    {
	indent_printf("if (name)\n");
	indent_printf("{\n");
	indent_printf("fp->fputs(name);\n");
	indent_printf("fp->fputs(\" =\\n\");\n");
	indent_printf("}\n");
	indent_printf("fp->fputs(\"{\\n\");\n");
    }
    for (j = 0; j < nelements; ++j)
    {
	element_ty *ep = &element[j];
	ep->etype->gen_code_declarator(ep->name, false, ep->attributes);
    }
    if (!toplevel_flag)
    {
	indent_printf("fp->fputs(\"}\");\n");
	indent_printf("if (name)\n");
	indent_more();
	indent_printf("fp->fputs(\";\\n\");\n");
	indent_less();
    }
    else if (!nelements)
    {
        // To silence "fp unused" warning
        indent_printf("(void)fp;\n");
    }
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    if (toplevel_flag)
    {
	indent_printf
	(
	    "%s_write_xml(const output::pointer &fp, %s_ty *this_thing)\n",
	    def_name().c_str(),
	    def_name().c_str()
	);
    }
    else
    {
	indent_printf
	(
	   "%s_write_xml(const output::pointer &fp, const char *name, "
                "%s_ty *this_thing)\n",
	    def_name().c_str(),
	    def_name().c_str()
	);
    }
    indent_printf("{\n");
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    if (toplevel_flag)
    {
	indent_printf
	(
	    "trace((\"%s_write_xml(this_thing = %%08lX)\\n{\\n\", "
            "(long)this_thing));\n",
	    def_name().c_str()
	);
    }
    else
    {
	indent_printf
	(
	    "trace((\"%s_write_xml(name = \\\"%%s\\\", this_thing = %%08lX)\\n"
		"{\\n\", name, (long)this_thing));\n",
	    def_name().c_str()
	);
	indent_printf("assert(name);\n");
    }
    indent_printf
    (
	"assert(((%s_ty *)this_thing)->reference_count > 0);\n",
	def_name().c_str()
    );
    indent_printf
    (
	"trace((\"rc = %%ld;\\n\", ((%s_ty *)this_thing)->reference_count));\n",
	def_name().c_str()
    );
    if (!toplevel_flag)
    {
	indent_printf("assert(name);\n");
	indent_printf("fp->fputc('<');\n");
	indent_printf("fp->fputs(name);\n");
	indent_printf("fp->fputs(\">\\n\");\n");
    }
    else
	indent_printf("fp->fputs(\"<%s>\\n\");\n",
                      def_name().c_str());
    for (j = 0; j < nelements; ++j)
    {
	element_ty *ep = &element[j];
	ep->etype->gen_code_call_xml(ep->name, ep->name, ep->attributes);
    }
    if (!toplevel_flag)
    {
	indent_printf("fp->fputs(\"</\");\n");
	indent_printf("fp->fputs(name);\n");
	indent_printf("fp->fputs(\">\\n\");\n");
    }
    else
    {
	indent_printf
	(
	    "fp->fputs(\"</%s>\\n\");\n",
	    def_name().c_str()
	);
    }
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void *\n");
    indent_printf("%s_alloc(void)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing;\n", def_name().c_str());
    indent_putchar('\n');
    indent_printf
    (
	"trace((\"%s_alloc()\\n{\\n\"));\n",
	def_name().c_str()
    );
    indent_printf
    (
	"this_thing = (%s_ty *)mem_alloc(sizeof(%s_ty));\n",
        def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("this_thing->reference_count = 1;\n");
    indent_printf("this_thing->mask = 0;\n");
    indent_printf("this_thing->errpos = str_copy(lex_position());\n");

    //
    // It is tempting to say
    //
    //     indent_printf("memset(this_this, 0, sizeof(*this_thing));\n");
    //
    // except that the ANCI C standard does NOT guarantee that the NULL
    // pointer is represented in memory as all-bits-zero (even though
    // you spell it "0").  This is also true of floating point zero.
    // As a result, each element is assigned zero individually.
    //
    for (j = 0; j < nelements; ++j)
    {
	element_ty *ep = &element[j];
        indent_printf
	(
	    "this_thing->%s = (%s)0;\n",
	    ep->name.c_str(),
	    ep->etype->c_name().c_str()
	);
    }
    indent_printf("trace((\"return %%08lX;\\n\", (long)this_thing));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return this_thing;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("%s_ty *\n", def_name().c_str());
    indent_printf
    (
	"%s_copy(%s_ty *this_thing)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf
    (
	"trace((\"%s_copy()\\n{\\n\"));\n",
	def_name().c_str()
    );
    indent_printf("this_thing->reference_count++;\n");
    indent_printf("trace((\"return %%08lX;\\n\", (long)this_thing));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return this_thing;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("%s_ty *\n", def_name().c_str());
    indent_printf
    (
	"%s_clone(%s_ty *this_thing)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return 0;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_clone()\\n{\\n\"));\n",
	def_name().c_str()
    );
    indent_printf
    (
	"%s_ty *result = (%s_ty *)%s_alloc();\n",
	def_name().c_str(),
	def_name().c_str(),
	def_name().c_str()
    );

    for (j = 0; j < nelements; ++j)
    {
	element_ty *ep = &element[j];
	ep->etype->gen_code_copy(ep->name);
    }

    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_printf("\n#ifdef DEBUG\n\n");
    indent_printf("void\n");
    indent_printf
    (
	"%s_trace_real(const char *name, const %s_ty *value)\n",
	def_name().c_str(),
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("if (name && *name)\n");
    indent_printf("{\n");
    indent_printf("trace_printf(\"%%s = \", name);\n");
    indent_printf("}\n");
    indent_printf("if (!value)\n");
    indent_printf("{\n");
    indent_printf("trace_printf(\"NULL\");\n");
    indent_printf("}\n");
    indent_printf("else\n");
    indent_printf("{\n");
    indent_printf("trace_printf(\"{\\n\");\n");
    for (j = 0; j < nelements; ++j)
    {
	element_ty *ep = &element[j];
	ep->etype->gen_code_trace(ep->name, "value->" + ep->name);
    }
    indent_printf("trace_printf(\"}\");\n");
    indent_printf("}\n");
    indent_printf("trace_printf((name && *name) ? \";\\n\" : \",\\n\");\n");
    indent_printf("}\n");
    indent_printf("\n#endif // DEBUG\n");

    indent_putchar('\n');
    indent_printf("static void\n");
    indent_printf("%s_free(void *that)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing;\n", def_name().c_str());
    indent_putchar('\n');
    indent_printf("this_thing = (%s_ty *)that;\n", def_name().c_str());
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf("this_thing->reference_count--;\n");
    indent_printf("assert(this_thing->reference_count >= 0);\n");
    indent_printf("if (this_thing->reference_count > 0)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    indent_printf
    (
	"trace((\"%s_free(this_thing = %%08lX)\\n{\\n\", (long)this_thing));\n",
	def_name().c_str()
    );
    indent_printf("if (this_thing->errpos)\n");
    indent_printf("{\n");
    indent_printf("str_free(this_thing->errpos);\n");
    indent_printf("this_thing->errpos = 0;\n");
    indent_printf("}\n");

    for (j = 0; j < nelements; ++j)
    {
	element_ty *ep = &element[j];
	ep->etype->gen_free_declarator(ep->name, false);
    }
    indent_printf("mem_free(this_thing);\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf
    (
	"%s\1%s_table[] =\n", "static type_table_ty",
	def_name().c_str()
    );
    indent_printf("{\n");
    for (j = 0; j < nelements; ++j)
    {
	element_ty *ep = &element[j];
	indent_printf("{\n");
	indent_printf("\"%s\",\n", ep->name.c_str());
	indent_printf
	(
	    "offsetof(%s_ty, %s),\n",
	    def_name().c_str(),
	    ep->name.c_str()
	);
	indent_printf("&%s_type,\n", ep->etype->def_name().c_str());
	indent_printf
	(
	    "%s_%s_mask,\n",
	    def_name().c_str(),
	    ep->name.c_str()
	);
	int redef = !!(ep->attributes & ATTRIBUTE_REDEFINITION_OK);
	indent_printf
	(
	    "%d, // redefinition %s ok\n",
	    redef,
	    (redef ? "is" : "not")
	);
	indent_printf("0, // fast_name\n");
	indent_printf("},\n");
    }
    if (!nelements)
	    indent_printf("{ \"\", 0, 0, 0, 0, 0 },\n");
    indent_printf("};\n");

    indent_putchar('\n');
    indent_printf("static void *\n");
    indent_printf
    (
	"%s_parse(void *this_thing, string_ty *name, meta_type **type_pp, "
	    "unsigned long *mask_p, int *redef_p)\n",
	def_name().c_str()
    );
    indent_printf("{\n");
    indent_printf("void\1*addr;\n");
    indent_putchar('\n');
    indent_printf
    (
	"trace((\"%s_parse(this_thing = %%08lX, name = %%08lX, "
        "type_pp = %%08lX)\\n"
	    "{\\n\", (long)this_thing, (long)name, (long)type_pp));\n",
	def_name().c_str()
    );
    indent_printf
    (
	"assert(((%s_ty *)this_thing)->reference_count > 0);\n",
	def_name().c_str()
    );
    indent_printf("addr =\n");
    indent_more();
    indent_printf("generic_struct_parse\n(\n");
    indent_printf("this_thing,\n");
    indent_printf("name,\n");
    indent_printf("type_pp,\n");
    indent_printf("mask_p,\n");
    indent_printf("redef_p,\n");
    indent_printf("%s_table,\n", def_name().c_str());
    indent_printf("SIZEOF(%s_table)\n", def_name().c_str());
    indent_printf(");\n");
    indent_less();
    indent_printf("trace((\"return %%08lX;\\n}\\n\", (long)addr));\n");
    indent_printf("return addr;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static string_ty *\n");
    indent_printf("%s_fuzzy(string_ty *name)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("string_ty\1*result;\n");
    indent_putchar('\n');
    indent_printf
    (
	"trace((\"%s_fuzzy(name = %%08lX)\\n{\\n\", (long)name));\n",
	def_name().c_str()
    );
    indent_printf("result =\n");
    indent_more();
    indent_printf("generic_struct_fuzzy\n(\n");
    indent_printf("name,\n");
    indent_printf("%s_table,\n", def_name().c_str());
    indent_printf("SIZEOF(%s_table)\n", def_name().c_str());
    indent_printf(");\n");
    indent_less();
    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static rpt_value::pointer\n");
    indent_printf("%s_convert(void *this_thing)\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf
    (
	"trace((\"%s_convert(name = %%08lX)\\n{\\n\", (long)this_thing));\n",
	def_name().c_str()
    );
    indent_printf
    (
	"assert(((%s_ty *)this_thing)->reference_count > 0);\n",
	def_name().c_str()
    );
    indent_printf("rpt_value::pointer result =\n");
    indent_more();
    indent_printf("generic_struct_convert\n(\n");
    indent_printf("this_thing,\n");
    indent_printf("%s_table,\n", def_name().c_str());
    indent_printf("SIZEOF(%s_table)\n", def_name().c_str());
    indent_printf(");\n");
    indent_less();
    indent_printf("trace((\"return %%08lX;\\n\", (long)result.get()));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("meta_type %s_type =\n", def_name().c_str());
    indent_printf("{\n");
    indent_printf("\"%s\",\n", def_name().c_str());
    indent_printf("%s_alloc,\n", def_name().c_str());
    indent_printf("%s_free,\n", def_name().c_str());
    indent_printf("0, // enum_parse\n");
    indent_printf("0, // list_parse\n");
    indent_printf("%s_parse,\n", def_name().c_str());
    indent_printf("%s_fuzzy,\n", def_name().c_str());
    indent_printf("%s_convert,\n", def_name().c_str());
    indent_printf("generic_struct_is_set,\n");
    indent_printf("};\n");
}


void
type_structure::gen_code_declarator(const nstring &variable_name,
    bool is_a_list, int) const
{
    indent_printf("%s_write(fp, ", def_name().c_str());
    if (is_a_list)
	    indent_printf("(const char *)0");
    else
	    indent_printf("\"%s\"", variable_name.c_str());
    indent_printf(", this_thing->%s);\n", variable_name.c_str());
}


void
type_structure::gen_code_call_xml(const nstring &form_name,
    const nstring &member_name, int) const
{
    indent_printf
    (
	"%s_write_xml(fp, \"%s\", this_thing->%s);\n",
	def_name().c_str(),
	form_name.c_str(),
	member_name.c_str()
    );
}


void
type_structure::gen_code_copy(const nstring &member_name)
    const
{
    indent_printf
    (
	"result->%s = %s_clone(this_thing->%s);\n",
	member_name.c_str(),
	def_name().c_str(),
	member_name.c_str()
    );
}


void
type_structure::gen_free_declarator(const nstring &variable_name, bool)
    const
{
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
type_structure::member_add(const nstring &member_name, type *member_type,
    int attributes)
{
    if (nelements >= nelements_max)
    {
	size_t new_nelements_max = nelements_max * 2 + 16;
	element_ty *new_element = new element_ty[new_nelements_max];
	for (size_t j = 0; j < nelements; ++j)
	    new_element[j] = element[j];
	delete [] element;
	nelements_max = new_nelements_max;
	element = new_element;
    }
    element_ty *ep = element + nelements++;
    ep->etype = member_type;
    ep->name = member_name;
    ep->attributes = attributes;
}


void
type_structure::in_include_file()
{
    type::in_include_file();
    for (size_t j = 0; j < nelements; ++j)
    {
	element[j].etype->in_include_file();
    }
}


nstring
type_structure::c_name_inner()
    const
{
    return (def_name() + "_ty *");
}


bool
type_structure::has_a_mask()
    const
{
    return false;
}


void
type_structure::toplevel()
{
    toplevel_flag = true;
}


void
type_structure::gen_code_trace(const nstring &vname, const nstring &value)
    const
{
    indent_printf
    (
	"%s_trace_real(\"%s\", %s);\n",
	def_name().c_str(),
	vname.c_str(),
	value.c_str()
    );
}
