/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1998, 1999, 2001-2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipluate structure types
 */

#include <error.h>
#include <indent.h>
#include <mem.h>
#include <type/structure.h>


typedef struct element_ty element_ty;
struct element_ty
{
    string_ty       *name;
    type_ty	    *type;
    int		    show;
};

typedef struct type_struct_ty type_struct_ty;
struct type_struct_ty
{
    /* inherited */
    TYPE_TY

    /* instance variables */
    size_t	    nelements;
    size_t	    nelements_max;
    element_ty	    *element;
    int		    toplevel;
};


static void
constructor(type_ty *type)
{
    type_struct_ty  *this_thing;

    this_thing = (type_struct_ty *)type;
    this_thing->nelements = 0;
    this_thing->nelements_max = 0;
    this_thing->element = 0;
    this_thing->toplevel = 0;
}


static void
destructor(type_ty *type)
{
    type_struct_ty  *this_thing;
    size_t	    j;

    this_thing = (type_struct_ty *)type;
    for (j = 0; j < this_thing->nelements; ++j)
	str_free(this_thing->element[j].name);
    if (this_thing->element)
	mem_free(this_thing->element);
}



static void
gen_include(type_ty *type)
{
    type_struct_ty  *this_thing;
    size_t	    j;
    int		    bit;

    this_thing = (type_struct_ty *)type;
    indent_putchar('\n');
    indent_printf("#ifndef %s_DEF\n", this_thing->name->str_text);
    indent_printf("#define %s_DEF\n", this_thing->name->str_text);
    indent_printf("\n");
    for (j = 0, bit = 0; j < this_thing->nelements; ++j)
    {
	indent_printf
	(
	    "#define\t%s_%s_mask\t",
	    this_thing->name->str_text,
	    this_thing->element[j].name->str_text
	);
	if (this_thing->element[j].type->method->has_a_mask)
	    indent_printf("((unsigned long)1 << %d)", bit++);
	else
	    indent_printf("((unsigned long)0)");
	indent_printf("\n");
    }
    indent_printf("\n");
    indent_printf
    (
	"typedef struct %s_ty %s_ty;\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("struct %s_ty\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("%s\1reference_count;\n", "long");
    indent_printf("%s\1mask;\n", "unsigned long");
    for (j = 0; j < this_thing->nelements; ++j)
    {
	element_ty	*ep;

	ep = &this_thing->element[j];
	type_gen_include_declarator(ep->type, ep->name, 0);
    }
    indent_printf("};\n");
    indent_printf("#endif /* %s_DEF */\n", this_thing->name->str_text);

    indent_putchar('\n');
    indent_printf("extern type_ty %s_type;\n", this_thing->name->str_text);

    indent_putchar('\n');
    if (this_thing->toplevel)
    {
	indent_printf
	(
    	    "void %s_write(struct output_ty *, %s_ty *);\n",
    	    this_thing->name->str_text,
    	    this_thing->name->str_text
	);
	indent_printf
	(
    	    "void %s_write_xml(struct output_ty *, %s_ty *);\n",
    	    this_thing->name->str_text,
    	    this_thing->name->str_text
	);
    }
    else
    {
	indent_printf
	(
    	    "void %s_write(struct output_ty *, const char *, %s_ty *);\n",
    	    this_thing->name->str_text,
    	    this_thing->name->str_text
	);
	indent_printf
	(
    	    "void %s_write_xml(struct output_ty *, const char *, %s_ty *);\n",
    	    this_thing->name->str_text,
    	    this_thing->name->str_text
	);
    }
    indent_printf
    (
	"%s_ty *%s_copy(%s_ty *);\n",
	this_thing->name->str_text,
	this_thing->name->str_text,
	this_thing->name->str_text
    );
}


static void
gen_include_declarator(type_ty *this_thing, string_ty *name, int is_a_list)
{
    const char      *deref;

    deref = (is_a_list ? "*" : "");
    indent_printf
    (
	"%s_ty\1%s*%s;\n",
	this_thing->name->str_text,
	deref,
	name->str_text
    );
}


static void
gen_code(type_ty *type)
{
    type_struct_ty  *this_thing;
    size_t	    j;

    this_thing = (type_struct_ty *)type;

    indent_putchar('\n');
    indent_printf("void\n");
    if (this_thing->toplevel)
    {
	indent_printf
	(
	    "%s_write(output_ty *fp, %s_ty *this_thing)\n",
	    this_thing->name->str_text,
	    this_thing->name->str_text
	);
    }
    else
    {
	indent_printf
	(
	    "%s_write(output_ty *fp, const char *name, %s_ty *this_thing)\n",
	    this_thing->name->str_text,
	    this_thing->name->str_text
	);
    }
    indent_printf("{\n");
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    if (this_thing->toplevel)
    {
	indent_printf
	(
            "trace((\"%s_write(this_thing = %%08lX)\\n{\\n\", "
            "(long)this_thing));\n",
	    this_thing->name->str_text
	);
    }
    else
    {
	indent_printf
	(
	    "trace((\"%s_write(name = \\\"%%s\\\", this_thing = %%08lX)\\n"
	    "{\\n\", name, (long)this_thing));\n",
	    this_thing->name->str_text
	);
    }
    indent_printf
    (
	"assert(((%s_ty *)this_thing)->reference_count > 0);\n",
	this_thing->name->str_text
    );
    indent_printf
    (
	"trace((\"rc = %%ld;\\n\", ((%s_ty *)this_thing)->reference_count));\n",
	this_thing->name->str_text
    );
    if (!this_thing->toplevel)
    {
	indent_printf("if (name)\n");
	indent_printf("{\n");
	indent_printf("output_fputs(fp, name);\n");
	indent_printf("output_fputs(fp, \" =\\n\");\n");
	indent_printf("}\n");
	indent_printf("output_fputs(fp, \"{\\n\");\n");
    }
    for (j = 0; j < this_thing->nelements; ++j)
    {
	element_ty	*ep;

	ep = &this_thing->element[j];
	type_gen_code_declarator(ep->type, ep->name, 0, ep->show);
    }
    if (!this_thing->toplevel)
    {
	indent_printf("output_fputs(fp, \"}\");\n");
	indent_printf("if (name)\n");
	indent_more();
	indent_printf("output_fputs(fp, \";\\n\");\n");
	indent_less();
    }
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    if (this_thing->toplevel)
    {
	indent_printf
	(
	    "%s_write_xml(output_ty *fp, %s_ty *this_thing)\n",
	    this_thing->name->str_text,
	    this_thing->name->str_text
	);
    }
    else
    {
	indent_printf
	(
	   "%s_write_xml(output_ty *fp, const char *name, %s_ty *this_thing)\n",
	    this_thing->name->str_text,
	    this_thing->name->str_text
	);
    }
    indent_printf("{\n");
    indent_printf("if (!this_thing)\n");
    indent_more();
    indent_printf("return;\n");
    indent_less();
    if (this_thing->toplevel)
    {
	indent_printf
	(
	    "trace((\"%s_write_xml(this_thing = %%08lX)\\n{\\n\", "
            "(long)this_thing));\n",
	    this_thing->name->str_text
	);
    }
    else
    {
	indent_printf
	(
	    "trace((\"%s_write_xml(name = \\\"%%s\\\", this_thing = %%08lX)\\n"
		"{\\n\", name, (long)this_thing));\n",
	    this_thing->name->str_text
	);
	indent_printf("assert(name);\n");
    }
    indent_printf
    (
	"assert(((%s_ty *)this_thing)->reference_count > 0);\n",
	this_thing->name->str_text
    );
    indent_printf
    (
	"trace((\"rc = %%ld;\\n\", ((%s_ty *)this_thing)->reference_count));\n",
	this_thing->name->str_text
    );
    if (!this_thing->toplevel)
    {
	indent_printf("assert(name);\n");
	indent_printf("output_fputc(fp, '<');\n");
	indent_printf("output_fputs(fp, name);\n");
	indent_printf("output_fputs(fp, \">\\n\");\n");
    }
    else
	indent_printf("output_fputs(fp, \"<%s>\\n\");\n",
                      this_thing->name->str_text);
    for (j = 0; j < this_thing->nelements; ++j)
    {
	element_ty	*ep;

	ep = &this_thing->element[j];
	type_gen_code_call_xml(ep->type, ep->name, ep->name, ep->show);
    }
    if (!this_thing->toplevel)
    {
	indent_printf("output_fputs(fp, \"</\");\n");
	indent_printf("output_fputs(fp, name);\n");
	indent_printf("output_fputs(fp, \">\\n\");\n");
    }
    else
    {
	indent_printf
	(
	    "output_fputs(fp, \"</%s>\\n\");\n",
	    this_thing->name->str_text
	);
    }
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void *\n");
    indent_printf("%s_alloc(void)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing;\n", this_thing->name->str_text);
    indent_putchar('\n');
    indent_printf
    (
	"trace((\"%s_alloc()\\n{\\n\"));\n",
	this_thing->name->str_text
    );
    indent_printf
    (
	"this_thing = mem_alloc(sizeof(%s_ty));\n",
	this_thing->name->str_text
    );
    indent_printf("this_thing->reference_count = 1;\n");
    indent_printf("this_thing->mask = 0;\n");
    for (j = 0; j < this_thing->nelements; ++j)
    {
	element_ty	*ep;

	ep = &this_thing->element[j];
	indent_printf("this_thing->%s = 0;\n", ep->name->str_text);
    }
    indent_printf("trace((\"return %%08lX;\\n\", (long)this_thing));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return this_thing;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("%s_ty *\n", this_thing->name->str_text);
    indent_printf
    (
	"%s_copy(%s_ty *this_thing)\n",
	this_thing->name->str_text,
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf
    (
	"trace((\"%s_copy()\\n{\\n\"));\n",
	this_thing->name->str_text
    );
    indent_printf("this_thing->reference_count++;\n");
    indent_printf("trace((\"return %%08lX;\\n\", (long)this_thing));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return this_thing;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static void\n");
    indent_printf("%s_free(void *that)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("%s_ty\1*this_thing = that;\n", this_thing->name->str_text);
    indent_putchar('\n');
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
	this_thing->name->str_text
    );
    for (j = 0; j < this_thing->nelements; ++j)
    {
	element_ty	*ep;

	ep = &this_thing->element[j];
	type_gen_free_declarator(ep->type, ep->name, 0);
    }
    indent_printf("mem_free(this_thing);\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf
    (
	"%s\1%s_table[] =\n", "static type_table_ty",
	this_thing->name->str_text
    );
    indent_printf("{\n");
    for (j = 0; j < this_thing->nelements; ++j)
    {
	element_ty	*ep;

	ep = &this_thing->element[j];
	indent_printf("{\n");
	indent_printf("\"%s\",\n", ep->name->str_text);
	indent_printf
	(
	    "offsetof(%s_ty, %s),\n",
	    this_thing->name->str_text,
	    ep->name->str_text
	);
	indent_printf("&%s_type,\n", ep->type->name->str_text);
	indent_printf
	(
	    "%s_%s_mask,\n",
	    this_thing->name->str_text,
	    ep->name->str_text
	);
	indent_printf("},\n");
    }
    if (!this_thing->nelements)
	    indent_printf("{ \"\", },\n");
    indent_printf("};\n");

    indent_putchar('\n');
    indent_printf("static void *\n");
    indent_printf
    (
	"%s_parse(void *this_thing, string_ty *name, type_ty **type_pp, "
	    "unsigned long *mask_p)\n",
	this_thing->name->str_text
    );
    indent_printf("{\n");
    indent_printf("void\1*addr;\n");
    indent_putchar('\n');
    indent_printf
    (
	"trace((\"%s_parse(this_thing = %%08lX, name = %%08lX, "
        "type_pp = %%08lX)\\n"
	    "{\\n\", (long)this_thing, (long)name, (long)type_pp));\n",
	this_thing->name->str_text
    );
    indent_printf
    (
	"assert(((%s_ty *)this_thing)->reference_count > 0);\n",
	this_thing->name->str_text
    );
    indent_printf
    (
	"assert(sizeof(%s_ty *) == sizeof(generic_struct_ty *));\n",
	this_thing->name->str_text
    );
    indent_printf("addr =\n");
    indent_more();
    indent_printf("generic_struct_parse\n(\n");
    indent_printf("this_thing,\n");
    indent_printf("name,\n");
    indent_printf("type_pp,\n");
    indent_printf("mask_p,\n");
    indent_printf("%s_table,\n", this_thing->name->str_text);
    indent_printf("SIZEOF(%s_table)\n", this_thing->name->str_text);
    indent_printf(");\n");
    indent_less();
    indent_printf("trace((\"return %%08lX;\\n}\\n\", (long)addr));\n");
    indent_printf("return addr;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static string_ty *\n");
    indent_printf("%s_fuzzy(string_ty *name)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("string_ty\1*result;\n");
    indent_putchar('\n');
    indent_printf
    (
	"trace((\"%s_fuzzy(name = %%08lX)\\n{\\n\", (long)name));\n",
	this_thing->name->str_text
    );
    indent_printf("result =\n");
    indent_more();
    indent_printf("generic_struct_fuzzy\n(\n");
    indent_printf("name,\n");
    indent_printf("%s_table,\n", this_thing->name->str_text);
    indent_printf("SIZEOF(%s_table)\n", this_thing->name->str_text);
    indent_printf(");\n");
    indent_less();
    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("static struct rpt_value_ty *\n");
    indent_printf("%s_convert(void *this_thing)\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("struct rpt_value_ty\1*result;\n");
    indent_putchar('\n');
    indent_printf
    (
	"trace((\"%s_convert(name = %%08lX)\\n{\\n\", (long)this_thing));\n",
	this_thing->name->str_text
    );
    indent_printf
    (
	"assert(((%s_ty *)this_thing)->reference_count > 0);\n",
	this_thing->name->str_text
    );
    indent_printf("result =\n");
    indent_more();
    indent_printf("generic_struct_convert\n(\n");
    indent_printf("this_thing,\n");
    indent_printf("%s_table,\n", this_thing->name->str_text);
    indent_printf("SIZEOF(%s_table)\n", this_thing->name->str_text);
    indent_printf(");\n");
    indent_less();
    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("type_ty %s_type =\n", this_thing->name->str_text);
    indent_printf("{\n");
    indent_printf("\"%s\",\n", this_thing->name->str_text);
    indent_printf("%s_alloc,\n", this_thing->name->str_text);
    indent_printf("%s_free,\n", this_thing->name->str_text);
    indent_printf("0, /* enum_parse */\n");
    indent_printf("0, /* list_parse */\n");
    indent_printf("%s_parse,\n", this_thing->name->str_text);
    indent_printf("%s_fuzzy,\n", this_thing->name->str_text);
    indent_printf("%s_convert,\n", this_thing->name->str_text);
    indent_printf("generic_struct_is_set,\n");
    indent_printf("};\n");
}


static void
gen_code_declarator(type_ty *type, string_ty *name, int is_a_list, int show)
{
    indent_printf("%s_write(fp, ", type->name->str_text);
    if (is_a_list)
	    indent_printf("(const char *)0");
    else
	    indent_printf("\"%s\"", name->str_text);
    indent_printf(", this_thing->%s);\n", name->str_text);
}


static void
gen_code_call_xml(type_ty *type, string_ty *form_name, string_ty *member_name,
    int show)
{
    indent_printf
    (
	"%s_write_xml(fp, \"%s\", this_thing->%s);\n",
	type->name->str_text,
	form_name->str_text,
	member_name->str_text
    );
}


static void
gen_free_declarator(type_ty *type, string_ty *name, int is_a_list)
{
    if (type->included_flag)
    {
	indent_printf
	(
    	    "%s_type.free(this_thing->%s);\n",
    	    type->name->str_text,
    	    name->str_text
	);
    }
    else
    {
	indent_printf
	(
    	    "%s_free(this_thing->%s);\n",
    	    type->name->str_text,
    	    name->str_text
	);
    }
}


static void
member_add(type_ty *type, string_ty *member_name, type_ty *member_type,
    int show)
{
    type_struct_ty  *this_thing;
    element_ty      *ep;

    this_thing = (type_struct_ty *)type;
    if (this_thing->nelements >= this_thing->nelements_max)
    {
	size_t          nbytes;

	this_thing->nelements_max = this_thing->nelements_max * 2 + 16;
	nbytes = this_thing->nelements_max * sizeof(this_thing->element[0]);
	this_thing->element =
            (element_ty *)mem_change_size(this_thing->element, nbytes);
    }
    ep = &this_thing->element[this_thing->nelements++];
    ep->type = member_type;
    ep->name = str_copy(member_name);
    ep->show = show;
}


static void
in_include_file(type_ty *type)
{
    type_struct_ty  *this_thing;
    element_ty      *ep;
    size_t          j;

    this_thing = (type_struct_ty *)type;
    for (j = 0; j < this_thing->nelements; ++j)
    {
	ep = &this_thing->element[j];
	type_in_include_file(ep->type);
    }
}


type_method_ty type_structure =
{
    sizeof(type_struct_ty),
    "structure",
    0, /* has a mask */
    constructor,
    destructor,
    gen_include,
    gen_include_declarator,
    gen_code,
    gen_code_declarator,
    gen_code_call_xml,
    gen_free_declarator,
    member_add,
    in_include_file,
};


void
type_structure_toplevel(type_ty *type)
{
    type_struct_ty  *this_thing;

    this_thing = (type_struct_ty *)type;
    this_thing->toplevel = 1;
}
