/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: struct types function generation
 */

#include <type.h>
#include <indent.h>
#include <mem.h>
#include <id.h>
#include <error.h>


typedef struct type_struct_ty type_struct_ty;
struct type_struct_ty
{
	/* inherited */
	TYPE_T

	/* instance variables */
	parse_list_ty	*list;
	int		toplevel;
};


static void gen_include _((type_ty *, string_ty *));

static void
gen_include(type, name)
	type_ty		*type;
	string_ty	*name;
{
	parse_list_ty	*np;
	type_struct_ty	*type2;
	int		bitno;

	type2 = (type_struct_ty *)type;
	indent_putchar('\n');
	indent_printf("#ifndef %s_DEF\n", name->str_text);
	indent_printf("#define %s_DEF\n", name->str_text);
	indent_printf("\n");
	for (bitno = 0, np = type2->list; np; np = np->next, ++bitno)
	{
		indent_printf
		(
			"#define\t%s_%s_mask\t((unsigned long)1 << %d)\n",
			name->str_text,
			np->name->str_text,
			bitno
		);
	}
	indent_printf("\n");
	indent_printf("typedef struct %s *%s;\n", name->str_text, name->str_text);
	indent_printf("struct %s\n", name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1mask;\n", "unsigned long");
	for (np = type2->list; np; np = np->next)
	{
		type_ty		*tp;

		if (!id_search(np->name, ID_CLASS_FIELD, (long *)&tp))
			fatal("field \"%s\" vanished!", np->name->str_text);
		type_gen_include_declarator(tp, np->name, 0);
	}
	indent_printf(/*{*/"};\n");
	indent_printf("#endif /* %s_DEF */\n", name->str_text);

	indent_putchar('\n');
	indent_printf("extern type_ty %s_type;\n", name->str_text);

	indent_putchar('\n');
	if (type2->toplevel)
		indent_printf("void %s_write _((%s));\n", name->str_text, name->str_text);
	else
		indent_printf("void %s_write _((char *, %s));\n", name->str_text, name->str_text);
}


static void gen_include_declarator _((type_ty *, string_ty *, int));

static void
gen_include_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	char		*deref;

	deref = (is_a_list ? "*" : "");
	indent_printf("%s\1%s%s;\n", type->name->str_text, deref, name->str_text);
}


static void gen_code _((type_ty *, string_ty *));

static void
gen_code(type, name)
	type_ty		*type;
	string_ty	*name;
{
	parse_list_ty	*np;
	type_struct_ty	*type2;
	static int	done_table;
	static int	done_offsetof;

	if (!done_offsetof)
	{
		done_offsetof = 1;
		indent_putchar('\n');
		indent_printf("#ifndef offsetof\n");
		indent_printf
		(
	"#define offsetof(a, b)\t((size_t)((char *)&((a *)0)->b - (char *)0))\n"
		);
		indent_printf("#endif\n");
	}

	type2 = (type_struct_ty *)type;
	indent_putchar('\n');
	indent_printf("void\n");
	if (type2->toplevel)
		indent_printf("%s_write(this)\n", name->str_text);
	else
		indent_printf("%s_write(name, this)\n", name->str_text);
	indent_more();
	if (!type2->toplevel)
		indent_printf("%s\1*name;\n", "char");
	indent_printf("%s\1this;\n", name->str_text);
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("if (!this)\n");
	indent_more();
	indent_printf("return;\n");
	indent_less();
	if (type2->toplevel)
		indent_printf("trace((\"%s_write(this = %%08lX)\\n{\\n\"/*}*/, this));\n", name->str_text);
	else
		indent_printf("trace((\"%s_write(name = \\\"%%s\\\", this = %%08lX)\\n{\\n\"/*}*/, name, this));\n", name->str_text);
	if (!type2->toplevel)
	{
		indent_printf("if (name)\n");
		indent_more();
		indent_printf("indent_printf(\"%%s =\\n\", name);\n");
		indent_less();
		indent_printf("indent_printf(\"{\\n\"/*}*/);\n");
	}
	for (np = type2->list; np; np = np->next)
	{
		type_ty *tp;

		id_search(np->name, ID_CLASS_FIELD, (long *)&tp);
		type_gen_code_declarator(tp, np->name, 0);
	}
	if (!type2->toplevel)
	{
		indent_printf("indent_printf(/*{*/\"}\");\n");
		indent_printf("if (name)\n");
		indent_more();
		indent_printf("indent_printf(\";\\n\");\n");
		indent_less();
	}
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("static void *%s_alloc _((void));\n", name->str_text);

	indent_putchar('\n');
	indent_printf("static void *\n");
	indent_printf("%s_alloc()\n", name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1*result;\n", "void");
	indent_putchar('\n');
	indent_printf("trace((\"%s_alloc()\\n{\\n\"/*}*/));\n", name->str_text);
	indent_printf
	(
		"result = (void *)mem_alloc_clear(sizeof(struct %s));\n",
		name->str_text
	);
	indent_printf("trace((/*{*/\"return %%08lX;\\n}\\n\", (long)result));\n");
	indent_printf("return result;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("static void %s_free _((void *));\n", name->str_text);

	indent_putchar('\n');
	indent_printf("static void\n");
	indent_printf("%s_free(that)\n", name->str_text);
	indent_more();
	indent_printf("%s\1*that;\n", "void");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1this = (%s)that;\n", name->str_text, name->str_text);
	indent_putchar('\n');
	indent_printf("if (!this)\n");
	indent_more();
	indent_printf("return;\n");
	indent_less();
	indent_printf("trace((\"%s_free(this = %%08lX)\\n{\\n\"/*}*/, this));\n", name->str_text);
	for (np = type2->list; np; np = np->next)
	{
		type_ty		*tp;

		id_search(np->name, ID_CLASS_FIELD, (long *)&tp);
		type_gen_free_declarator(tp, np->name, 0);
	}
	indent_printf("mem_free((char *)this);\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");

	if (!done_table)
	{
		indent_printf("\n");
		indent_printf("typedef struct table_ty table_ty;\n");
		indent_printf("struct table_ty\n");
		indent_printf("{\n");
		indent_printf("%s\1*name;\n", "char");
		indent_printf("%s\1offset;\n", "size_t");
		indent_printf("%s\1*type;\n", "type_ty");
		indent_printf("%s\1mask;\n", "unsigned long");
		indent_printf("%s\1*fast_name;\n", "string_ty");
		indent_printf("};\n");
		done_table = 1;
	}

	indent_putchar('\n');
	indent_printf
	(
		"static int %s_parse _((void *, string_ty *, type_ty **,\n\
void **, unsigned long *));\n",
		name->str_text
	);

	indent_putchar('\n');
	indent_printf("static int\n");
	indent_printf("%s_parse(that, name, type_pp, addr_p, mask_p)\n", name->str_text);
	indent_more();
	indent_printf("%s\1*that;\n", "void");
	indent_printf("%s\1*name;\n", "string_ty");
	indent_printf("%s\1**type_pp;\n", "type_ty");
	indent_printf("%s\1**addr_p;\n", "void");
	indent_printf("%s\1*mask_p;\n", "unsigned long");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1table[] =\n", "static table_ty");
	indent_printf("{\n"/*}*/);
	for (np = type2->list; np; np = np->next)
	{
		type_ty		*tp;

		id_search(np->name, ID_CLASS_FIELD, (long *)&tp);
		indent_printf("{\n"/*}*/);
		indent_printf("\"%s\",\n", np->name->str_text);
		indent_printf
		(
			"offsetof(struct %s, %s),\n",
			name->str_text,
			np->name->str_text
		);
		indent_printf("&%s_type,\n", tp->name->str_text);
		indent_printf
		(
			"%s_%s_mask,\n",
			name->str_text,
			np->name->str_text
		);
		indent_printf(/*{*/"},\n");
	}
	if (!type2->list)
		indent_printf("{ \"\", },\n");
	indent_printf(/*{*/"};\n");
	indent_printf("%s\1this = (%s)that;\n", name->str_text, name->str_text);
	indent_printf("%s\1*tp;\n", "table_ty");
	indent_printf("%s\1result;\n", "int");
	indent_putchar('\n');
	indent_printf("trace((\"%s_parse(this = %%08lX, name = %%08lX, type_pp = %%08lX, addr_p = %%08lX)\\n{\\n\"/*}*/, this, name, type_pp, addr_p));\n", name->str_text);
	indent_printf("result = -1;\n");
	indent_printf("for (tp = table; tp < ENDOF(table); ++tp)\n");
	indent_printf("{\n");
	indent_printf("if (!tp->fast_name)\n");
	indent_more();
	indent_printf("tp->fast_name = str_from_c(tp->name);\n");
	indent_less();
	indent_printf("if (str_equal(name, tp->fast_name))\n");
	indent_printf("{\n"/*}*/);
	indent_printf("*type_pp = tp->type;\n");
	indent_printf("trace_pointer(*type_pp);\n");
	indent_printf("*addr_p = (void *)((char *)this + tp->offset);\n");
	indent_printf("trace_pointer(*addr_p);\n");
	indent_printf("*mask_p = tp->mask;\n");
	indent_printf("result = 0;\n");
	indent_printf("break;\n");
	indent_printf(/*{*/"}\n");
	indent_printf(/*{*/"}\n");
	indent_printf("trace((/*{*/\"return %%d;\\n}\\n\", result));\n");
	indent_printf("return result;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("type_ty %s_type =\n", name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("type_class_struct,\n");
	indent_printf("\"%s\",\n", name->str_text);
	indent_printf("%s_alloc,\n", name->str_text);
	indent_printf("%s_free,\n", name->str_text);
	indent_printf("0, /* enum_parse */\n");
	indent_printf("0, /* list_parse */\n");
	indent_printf("%s_parse,\n", name->str_text);
	indent_printf(/*{*/"};\n");
}


static void gen_code_declarator _((type_ty *, string_ty *, int));

static void
gen_code_declarator(type, name, is_a_list)
	type_ty		*type;
	string_ty	*name;
	int		is_a_list;
{
	indent_printf("%s_write("/*)*/, type->name->str_text);
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
	indent_printf("%s_free(this->%s);\n", type->name->str_text, name->str_text);
}


static type_method_ty method =
{
	gen_include,
	gen_include_declarator,
	gen_code,
	gen_code_declarator,
	gen_free_declarator,
};


type_ty *
type_create_struct(name, list, toplevel)
	string_ty	*name;
	parse_list_ty	*list;
	int		toplevel;
{
	type_struct_ty	*type;

	type = (type_struct_ty *)mem_alloc(sizeof(type_struct_ty));
	type->class = type_class_struct;
	type->method = &method;
	type->name = str_copy(name);
	type->list = list;
	type->toplevel = toplevel;
	id_assign(name, ID_CLASS_TYPE, (long)type);
	return (type_ty *)type;
}
