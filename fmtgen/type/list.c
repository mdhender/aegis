/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate list types
 */

#include <indent.h>
#include <mem.h>
#include <type/list.h>


typedef struct type_list_ty type_list_ty;
struct type_list_ty
{
	/* inherited */
	TYPE_TY

	/* instance variables */
	type_ty		*subtype;
};


static void gen_include _((type_ty *));

static void
gen_include(type)
	type_ty		*type;
{
	string_ty	*s;
	type_list_ty	*this;

	this = (type_list_ty *)type;
	indent_putchar('\n');
	indent_printf("#ifndef %s_DEF\n", this->name->str_text);
	indent_printf("#define %s_DEF\n", this->name->str_text);
	indent_printf
	(
		"typedef struct %s *%s;\n",
		this->name->str_text,
		this->name->str_text
	);
	indent_printf("struct %s\n", this->name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1length;\n", "size_t");
	indent_printf("%s\1maximum;\n", "size_t");
	s = str_from_c("list");
	type_gen_include_declarator(this->subtype, s, 1);
	str_free(s);
	indent_printf(/*{*/"};\n");
	indent_printf("#endif /* %s_DEF */\n", this->name->str_text);

	indent_putchar('\n');
	indent_printf("extern type_ty %s_type;\n", this->name->str_text);

	indent_putchar('\n');
	indent_printf
	(
		"void %s_write _((char *, %s));\n",
		this->name->str_text,
		this->name->str_text
	);
}


static void gen_include_declarator _((type_ty *, string_ty *, int));

static void
gen_include_declarator(type, variable_name, is_a_list)
	type_ty		*type;
	string_ty	*variable_name;
	int		is_a_list;
{
	char		*deref;

	deref = (is_a_list ? "*" : "");
	indent_printf
	(
		"%s\1%s%s;\n",
		type->name->str_text,
		deref,
		variable_name->str_text
	);
}


static void gen_code _((type_ty *));

static void
gen_code(type)
	type_ty		*type;
{
	string_ty	*s;
	type_list_ty	*this;
	static int	lincl_done;

	if (!lincl_done)
	{
		indent_putchar('\n');
		indent_printf("#include <aer/value/list.h>\n");
		lincl_done = 1;
	}

	this = (type_list_ty *)type;
	indent_putchar('\n');
	indent_printf("void\n");
	indent_printf("%s_write(name, this)\n", this->name->str_text);
	indent_more();
	indent_printf("%s\1*name;\n", "char");
	indent_printf("%s\1this;\n", this->name->str_text);
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1j;\n", "size_t");
	indent_putchar('\n');
	indent_printf("if (!this)\n");
	indent_more();
	indent_printf("return;\n");
	indent_less();
	indent_printf
	(
		"trace((\"%s_write(name = \\\"%%s\\\", this = %%08lX)\\n\
{\\n\"/*}*/, name, (long)this));\n",
		this->name->str_text
	);
	indent_printf("if (name)\n");
	indent_more();
	indent_printf("indent_printf(\"%%s =\\n\", name);\n");
	indent_less();
	indent_printf("indent_printf(\"[\\n\"/*]*/);\n");
	indent_printf("for (j = 0; j < this->length; ++j)\n");
	indent_printf("{\n"/*}*/);
	s = str_from_c("list[j]");
	type_gen_code_declarator(this->subtype, s, 1);
	str_free(s);
	indent_printf("indent_printf(\",\\n\");\n");
	indent_printf(/*{*/"}\n");
	indent_printf("indent_printf(/*[*/\"]\");\n");
	indent_printf("if (name)\n");
	indent_more();
	indent_printf("indent_printf(\";\\n\");\n");
	indent_less();
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
	indent_printf("%s\1result;\n\n", this->name->str_text);
	indent_printf
	(
		"trace((\"%s_alloc()\\n{\\n\"/*}*/));\n",
		this->name->str_text
	);
	indent_printf
	(
		"result = mem_alloc(sizeof(struct %s));\n",
		this->name->str_text
	);
	indent_printf("result->list = 0;\n");
	indent_printf("result->length = 0;\n");
	indent_printf("result->maximum = 0;\n");
	indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf("return result;\n");
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
	indent_printf("%s\1this = that;\n", this->name->str_text);
	indent_printf("%s\1j;\n", "size_t");
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
	indent_printf("for (j = 0; j < this->length; ++j)\n");
	indent_more();
	s = str_from_c("list[j]");
	type_gen_free_declarator(this->subtype, s, 1);
	str_free(s);
	indent_less();
	indent_printf("if (this->list)\n");
	indent_more();
	indent_printf("mem_free(this->list);\n");
	indent_less();
	indent_printf("mem_free(this);\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"static void *%s_parse _((void *, type_ty **));\n",
		this->name->str_text
	);

	indent_putchar('\n');
	indent_printf("static void *\n");
	indent_printf("%s_parse(that, type_pp)\n", this->name->str_text);
	indent_more();
	indent_printf("%s\1*that;\n", "void");
	indent_printf("%s\1**type_pp;\n", "type_ty");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1this = that;\n", this->name->str_text);
	indent_printf("%s\1*addr;\n", "void");
	indent_putchar('\n');
	indent_printf
	(
		"trace((\"%s_parse(this = %%08lX, type_pp = %%08lX)\\n{\\n\"\
/*}*/, (long)this, (long)type_pp));\n",
		this->name->str_text
	);
	indent_printf("*type_pp = &%s_type;\n", this->subtype->name->str_text);
	indent_printf("trace_pointer(*type_pp);\n");
	indent_printf("if (this->length >= this->maximum)\n");
	indent_printf("{\n"/*}*/);
	indent_printf("size_t\1nbytes;\n\n");
	indent_printf("this->maximum = this->maximum * 2 + 16;\n");
	indent_printf("nbytes = this->maximum * sizeof(this->list[0]);\n");
	indent_printf("this->list = mem_change_size(this->list, nbytes);\n");
	indent_printf(/*{*/"}\n");
	indent_printf("addr = &this->list[this->length++];\n");
	indent_printf("trace((\"return %%08lX;\\n\", (long)addr));\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf("return addr;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"static rpt_value_ty *%s_convert _((void *));\n",
		this->name->str_text
	);

	indent_putchar('\n');
	indent_printf("static rpt_value_ty *\n");
	indent_printf("%s_convert(that)\n", this->name->str_text);
	indent_more();
	indent_printf("%s\1*that;\n", "void");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1this;\n", this->name->str_text);
	indent_printf("%s\1*result;\n", "rpt_value_ty");
	indent_printf("%s\1j;\n", "size_t");
	indent_printf("%s\1*vp;\n", "rpt_value_ty");
	indent_putchar('\n');
	indent_printf("this = *(%s *)that;\n", this->name->str_text);
	indent_printf("if (!this)\n");
	indent_more();
	indent_printf("return 0;\n");
	indent_less();
	indent_printf
	(
	    "trace((\"%s_convert(this = %%08lX)\\n{\\n\"/*}*/, (long)this));\n",
		this->name->str_text
	);
	indent_printf("result = rpt_value_list();\n");
	indent_printf("for (j = 0; j < this->length; ++j)\n");
	indent_printf("{\n"/*}*/);
	indent_printf
	(
		"vp = %s_type.convert(&this->list[j]);\n",
		this->subtype->name->str_text
	);
	indent_printf("assert(vp);\n");
	indent_printf("rpt_value_list_append(result, vp);\n");
	indent_printf("rpt_value_free(vp);\n");
	indent_printf(/*{*/"}\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
	indent_printf("return result;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("type_ty %s_type =\n", this->name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("\"%s\",\n", this->name->str_text);
	indent_printf("%s_alloc,\n", this->name->str_text);
	indent_printf("%s_free,\n", this->name->str_text);
	indent_printf("0, /* enum_parse */\n");
	indent_printf("%s_parse,\n", this->name->str_text);
	indent_printf("0, /* struct_parse */\n");
	indent_printf("0, /* fuzzy */\n");
	indent_printf("%s_convert,\n", this->name->str_text);
	indent_printf("generic_struct_is_set,\n");
	indent_printf(/*{*/"};\n");
}


static void gen_code_declarator _((type_ty *, string_ty *, int));

static void
gen_code_declarator(type, variable_name, is_a_list)
	type_ty		*type;
	string_ty	*variable_name;
	int		is_a_list;
{
	indent_printf("%s_write("/*)*/, type->name->str_text);
	if (is_a_list)
		indent_printf("\"\"");
	else
		indent_printf("\"%s\"", variable_name->str_text);
	indent_printf(/*(*/", this->%s);\n", variable_name->str_text);
}


static void gen_free_declarator _((type_ty *, string_ty *, int));

static void
gen_free_declarator(type, variable_name, is_a_list)
	type_ty		*type;
	string_ty	*variable_name;
	int		is_a_list;
{
	indent_printf
	(
		"%s_free(this->%s);\n",
		type->name->str_text,
		variable_name->str_text
	);
}


static void member_add _((type_ty *, string_ty *, type_ty *));

static void
member_add(type, member_name, member_type)
	type_ty		*type;
	string_ty	*member_name;
	type_ty		*member_type;
{
	type_list_ty	*this;

	this = (type_list_ty *)type;
	this->subtype = member_type;
}


type_method_ty type_list =
{
	sizeof(type_list_ty),
	"list",
	0, /* has a mask NOT */
	0, /* constructor */
	0, /* destructor */
	gen_include,
	gen_include_declarator,
	gen_code,
	gen_code_declarator,
	gen_free_declarator,
	member_add,
};
