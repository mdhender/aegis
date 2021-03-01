/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1998, 1999 Peter Miller;
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
	string_ty	*name;
	type_ty		*type;
};

typedef struct type_struct_ty type_struct_ty;
struct type_struct_ty
{
	/* inherited */
	TYPE_TY

	/* instance variables */
	size_t		nelements;
	size_t		nelements_max;
	element_ty	*element;
	int		toplevel;
};


static void constructor _((type_ty *));

static void
constructor(type)
	type_ty		*type;
{
	type_struct_ty	*this;

	this = (type_struct_ty *)type;
	this->nelements = 0;
	this->nelements_max = 0;
	this->element = 0;
	this->toplevel = 0;
}


static void destructor _((type_ty *));

static void
destructor(type)
	type_ty		*type;
{
	type_struct_ty	*this;
	size_t		j;

	this = (type_struct_ty *)type;
	for (j = 0; j < this->nelements; ++j)
		str_free(this->element[j].name);
	if (this->element)
		mem_free(this->element);
}



static void gen_include _((type_ty *));

static void
gen_include(type)
	type_ty		*type;
{
	type_struct_ty	*this;
	long		j;
	int		bit;

	this = (type_struct_ty *)type;
	indent_putchar('\n');
	indent_printf("#ifndef %s_DEF\n", this->name->str_text);
	indent_printf("#define %s_DEF\n", this->name->str_text);
	indent_printf("\n");
	for (j = 0, bit = 0; j < this->nelements; ++j)
	{
		indent_printf
		(
			"#define\t%s_%s_mask\t",
			this->name->str_text,
			this->element[j].name->str_text
		);
		if (this->element[j].type->method->has_a_mask)
			indent_printf("((unsigned long)1 << %d)", bit++);
		else
			indent_printf("((unsigned long)0)");
		indent_printf("\n");
	}
	indent_printf("\n");
	indent_printf
	(
		"typedef struct %s *%s;\n",
		this->name->str_text,
		this->name->str_text
	);
	indent_printf("struct %s\n", this->name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1mask;\n", "unsigned long");
	for (j = 0; j < this->nelements; ++j)
	{
		element_ty	*ep;

		ep = &this->element[j];
		type_gen_include_declarator(ep->type, ep->name, 0);
	}
	indent_printf(/*{*/"};\n");
	indent_printf("#endif /* %s_DEF */\n", this->name->str_text);

	indent_putchar('\n');
	indent_printf("extern type_ty %s_type;\n", this->name->str_text);

	indent_putchar('\n');
	if (this->toplevel)
	{
		indent_printf
		(
			"void %s_write _((struct output_ty *, %s));\n",
			this->name->str_text,
			this->name->str_text
		);
	}
	else
	{
		indent_printf
		(
			"void %s_write _((struct output_ty *, char *, %s));\n",
			this->name->str_text,
			this->name->str_text
		);
	}
}


static void gen_include_declarator _((type_ty *, string_ty *, int));

static void
gen_include_declarator(this, name, is_a_list)
	type_ty		*this;
	string_ty	*name;
	int		is_a_list;
{
	char		*deref;

	deref = (is_a_list ? "*" : "");
	indent_printf
	(
		"%s\1%s%s;\n",
		this->name->str_text,
		deref,
		name->str_text
	);
}


static void gen_code _((type_ty *));

static void
gen_code(type)
	type_ty		*type;
{
	type_struct_ty	*this;
	size_t		j;

	this = (type_struct_ty *)type;
	indent_putchar('\n');
	indent_printf("void\n");
	if (this->toplevel)
		indent_printf("%s_write(fp, this)\n", this->name->str_text);
	else
		indent_printf("%s_write(fp, name, this)\n", this->name->str_text);
	indent_more();
	indent_printf("%s\1*fp;\n", "output_ty");
	if (!this->toplevel)
		indent_printf("%s\1*name;\n", "char");
	indent_printf("%s\1this;\n", this->name->str_text);
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("if (!this)\n");
	indent_more();
	indent_printf("return;\n");
	indent_less();
	if (this->toplevel)
	{
		indent_printf
		(
		    "trace((\"%s_write(this = %%08lX)\\n{\\n\"/*}*/, this));\n",
			this->name->str_text
		);
	}
	else
	{
		indent_printf
		(
			"trace((\"%s_write(name = \\\"%%s\\\", this = %%08lX)\
\\n{\\n\"/*}*/, name, (long)this));\n",
			this->name->str_text
		);
	}
	if (!this->toplevel)
	{
		indent_printf("if (name)\n");
		indent_more();
		indent_printf("output_fprintf(fp, \"%%s =\\n\", name);\n");
		indent_less();
		indent_printf("output_fprintf(fp, \"{\\n\"/*}*/);\n");
	}
	for (j = 0; j < this->nelements; ++j)
	{
		element_ty	*ep;

		ep = &this->element[j];
		type_gen_code_declarator(ep->type, ep->name, 0);
	}
	if (!this->toplevel)
	{
		indent_printf("output_fprintf(fp, /*{*/\"}\");\n");
		indent_printf("if (name)\n");
		indent_more();
		indent_printf("output_fprintf(fp, \";\\n\");\n");
		indent_less();
	}
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"static void *%s_alloc _((void));\n",
		this->name->str_text
	);

	indent_putchar('\n');
	indent_printf("static void *\n");
	indent_printf("%s_alloc()\n", this->name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1this;\n", this->name->str_text);
	indent_putchar('\n');
	indent_printf
	(
		"trace((\"%s_alloc()\\n{\\n\"/*}*/));\n",
		this->name->str_text
	);
	indent_printf
	(
		"this = mem_alloc(sizeof(struct %s));\n",
		this->name->str_text
	);
	indent_printf("this->mask = 0;\n");
	for (j = 0; j < this->nelements; ++j)
	{
		element_ty	*ep;

		ep = &this->element[j];
		indent_printf("this->%s = 0;\n", ep->name->str_text);
	}
	indent_printf("trace((\"return %%08lX;\\n\", (long)this));\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf("return this;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"static void %s_free _((void *));\n",
		this->name->str_text
	);

	indent_putchar('\n');
	indent_printf("static void\n");
	indent_printf("%s_free(that)\n", this->name->str_text);
	indent_more();
	indent_printf("%s\1*that;\n", "void");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf
	(
		"%s\1this = that;\n",
		this->name->str_text
	);
	indent_putchar('\n');
	indent_printf("if (!this)\n");
	indent_more();
	indent_printf("return;\n");
	indent_less();
	indent_printf
	(
	       "trace((\"%s_free(this = %%08lX)\\n{\\n\"/*}*/, (long)this));\n",
		this->name->str_text
	);
	for (j = 0; j < this->nelements; ++j)
	{
		element_ty	*ep;

		ep = &this->element[j];
		type_gen_free_declarator(ep->type, ep->name, 0);
	}
	indent_printf("mem_free(this);\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"%s\1%s_table[] =\n", "static type_table_ty",
		this->name->str_text
	);
	indent_printf("{\n"/*}*/);
	for (j = 0; j < this->nelements; ++j)
	{
		element_ty	*ep;

		ep = &this->element[j];
		indent_printf("{\n"/*}*/);
		indent_printf("\"%s\",\n", ep->name->str_text);
		indent_printf
		(
			"offsetof(struct %s, %s),\n",
			this->name->str_text,
			ep->name->str_text
		);
		indent_printf("&%s_type,\n", ep->type->name->str_text);
		indent_printf
		(
			"%s_%s_mask,\n",
			this->name->str_text,
			ep->name->str_text
		);
		indent_printf(/*{*/"},\n");
	}
	if (!this->nelements)
		indent_printf("{ \"\", },\n");
	indent_printf(/*{*/"};\n");

	indent_putchar('\n');
	indent_printf
	(
		"static void *%s_parse _((void *, string_ty *, type_ty **, \
unsigned long *));\n",
		this->name->str_text
	);

	indent_putchar('\n');
	indent_printf("static void *\n");
	indent_printf
	(
		"%s_parse(this, name, type_pp, mask_p)\n",
		this->name->str_text
	);
	indent_more();
	indent_printf("%s\1*this;\n", "void");
	indent_printf("%s\1*name;\n", "string_ty");
	indent_printf("%s\1**type_pp;\n", "type_ty");
	indent_printf("%s\1*mask_p;\n", "unsigned long");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1*addr;\n", "void");
	indent_putchar('\n');
	indent_printf
	(
		"trace((\"%s_parse(this = %%08lX, name = %%08lX, type_pp = \
%%08lX)\\n{\\n\"/*}*/, (long)this, (long)name, (long)type_pp));\n",
		this->name->str_text
	);
	indent_printf
	(
		"assert(sizeof(%s) == sizeof(generic_struct_ty *));\n",
		this->name->str_text
	);
	indent_printf("addr =\n");
	indent_more();
	indent_printf("generic_struct_parse\n(\n"/*)*/);
	indent_printf("this,\n");
	indent_printf("name,\n");
	indent_printf("type_pp,\n");
	indent_printf("mask_p,\n");
	indent_printf("%s_table,\n", this->name->str_text);
	indent_printf("SIZEOF(%s_table)\n", this->name->str_text);
	indent_printf(/*(*/");\n");
	indent_less();
	indent_printf("trace((/*{*/\"return %%08lX;\\n}\\n\", (long)addr));\n");
	indent_printf("return addr;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"static string_ty *%s_fuzzy _((string_ty *));\n",
		this->name->str_text
	);

	indent_putchar('\n');
	indent_printf("static string_ty *\n");
	indent_printf("%s_fuzzy(name)\n", this->name->str_text);
	indent_more();
	indent_printf("%s\1*name;\n", "string_ty");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1*result;\n", "string_ty");
	indent_putchar('\n');
	indent_printf
	(
	      "trace((\"%s_fuzzy(name = %%08lX)\\n{\\n\"/*}*/, (long)name));\n",
		this->name->str_text
	);
	indent_printf("result =\n");
	indent_more();
	indent_printf("generic_struct_fuzzy\n(\n"/*)*/);
	indent_printf("name,\n");
	indent_printf("%s_table,\n", this->name->str_text);
	indent_printf("SIZEOF(%s_table)\n", this->name->str_text);
	indent_printf(/*(*/");\n");
	indent_less();
	indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf("return result;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"static struct rpt_value_ty *%s_convert _((void *));\n",
		this->name->str_text
	);

	indent_putchar('\n');
	indent_printf("static struct rpt_value_ty *\n");
	indent_printf("%s_convert(this)\n", this->name->str_text);
	indent_more();
	indent_printf("%s\1*this;\n", "void");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1*result;\n", "struct rpt_value_ty");
	indent_putchar('\n');
	indent_printf
	(
	    "trace((\"%s_convert(name = %%08lX)\\n{\\n\"/*}*/, (long)this));\n",
		this->name->str_text
	);
	indent_printf("result =\n");
	indent_more();
	indent_printf("generic_struct_convert\n(\n"/*)*/);
	indent_printf("this,\n");
	indent_printf("%s_table,\n", this->name->str_text);
	indent_printf("SIZEOF(%s_table)\n", this->name->str_text);
	indent_printf(/*(*/");\n");
	indent_less();
	indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf("return result;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("type_ty %s_type =\n", this->name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("\"%s\",\n", this->name->str_text);
	indent_printf("%s_alloc,\n", this->name->str_text);
	indent_printf("%s_free,\n", this->name->str_text);
	indent_printf("0, /* enum_parse */\n");
	indent_printf("0, /* list_parse */\n");
	indent_printf("%s_parse,\n", this->name->str_text);
	indent_printf("%s_fuzzy,\n", this->name->str_text);
	indent_printf("%s_convert,\n", this->name->str_text);
	indent_printf("generic_struct_is_set,\n");
	indent_printf(/*{*/"};\n");
}


static void gen_code_declarator _((type_ty *, string_ty *, int));

static void
gen_code_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	indent_printf("%s_write(fp, "/*)*/, type->name->str_text);
	if (is_a_list)
		indent_printf("(char *)0");
	else
		indent_printf("\"%s\"", name->str_text);
	indent_printf(/*(*/", this->%s);\n", name->str_text);
}


static void gen_free_declarator _((type_ty *, string_ty *, int));

static void
gen_free_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	if (type->included_flag)
	{
		indent_printf
		(
			"%s_type.free(this->%s);\n",
			type->name->str_text,
			name->str_text
		);
	}
	else
	{
		indent_printf
		(
			"%s_free(this->%s);\n",
			type->name->str_text,
			name->str_text
		);
	}
}


static void member_add _((type_ty *, string_ty *, type_ty *));

static void
member_add(type, member_name, member_type)
	type_ty		*type;
	string_ty	*member_name;
	type_ty		*member_type;
{
	type_struct_ty	*this;
	element_ty	*ep;

	this = (type_struct_ty *)type;
	if (this->nelements >= this->nelements_max)
	{
		size_t		nbytes;

		this->nelements_max = this->nelements_max * 2 + 16;
		nbytes = this->nelements_max * sizeof(this->element[0]);
		this->element = mem_change_size(this->element, nbytes);
	}
	ep = &this->element[this->nelements++];
	ep->type = member_type;
	ep->name = str_copy(member_name);
}


static void in_include_file _((type_ty *));

static void
in_include_file(type)
	type_ty		*type;
{
	type_struct_ty	*this;
	element_ty	*ep;
	size_t		j;

	this = (type_struct_ty *)type;
	for (j = 0; j < this->nelements; ++j)
	{
		ep = &this->element[j];
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
	gen_free_declarator,
	member_add,
	in_include_file,
};


void
type_structure_toplevel(type)
	type_ty		*type;
{
	type_struct_ty	*this;

	this = (type_struct_ty *)type;
	this->toplevel = 1;
}
