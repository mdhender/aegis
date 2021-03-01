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
 * MANIFEST: list types function generation
 */

#include <type.h>
#include <indent.h>
#include <mem.h>
#include <id.h>


typedef struct type_list_ty type_list_ty;
struct type_list_ty
{
	/* inherited */
	TYPE_T

	/* instance variables */
	type_ty		*subtype;
};


static void gen_include _((type_ty *, string_ty *));

static void
gen_include(type, name)
	type_ty		*type;
	string_ty	*name;
{
	string_ty	*s;
	type_list_ty	*type2;

	type2 = (type_list_ty *)type;
	indent_putchar('\n');
	indent_printf("#ifndef %s_DEF\n", name->str_text);
	indent_printf("#define %s_DEF\n", name->str_text);
	indent_printf("typedef struct %s *%s;\n", name->str_text, name->str_text);
	indent_printf("struct %s\n", name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1length;\n", "size_t", name->str_text);
	s = str_from_c("list");
	type_gen_include_declarator(type2->subtype, s, 1);
	str_free(s);
	indent_printf(/*{*/"};\n");
	indent_printf("#endif /* %s_DEF */\n", name->str_text);

	indent_putchar('\n');
	indent_printf("extern type_ty %s_type;\n", name->str_text);

	indent_putchar('\n');
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
	string_ty	*s;
	type_list_ty	*type2;

	type2 = (type_list_ty *)type;
	indent_putchar('\n');
	indent_printf("void\n");
	indent_printf("%s_write(name, this)\n", name->str_text);
	indent_more();
	indent_printf("%s\1*name;\n", "char");
	indent_printf("%s\1this;\n", name->str_text);
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1j;\n", "size_t");
	indent_putchar('\n');
	indent_printf("if (!this)\n");
	indent_more();
	indent_printf("return;\n");
	indent_less();
	indent_printf("trace((\"%s_write(name = \\\"%%s\\\", this = %%08lX)\\n{\\n\"/*}*/, name, this));\n", name->str_text);
	indent_printf("if (name)\n");
	indent_more();
	indent_printf("indent_printf(\"%%s =\\n\", name);\n");
	indent_less();
	indent_printf("indent_printf(\"[\\n\"/*]*/);\n");
	indent_printf("for (j = 0; j < this->length; ++j)\n");
	indent_printf("{\n"/*}*/);
	s = str_from_c("list[j]");
	type_gen_code_declarator(type2->subtype, s, 1);
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
	indent_printf("static void *%s_alloc _((void));\n", name->str_text);

	indent_putchar('\n');
	indent_printf("static void *\n");
	indent_printf("%s_alloc()\n", name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1*result;\n\n", "void");
	indent_printf("trace((\"%s_alloc()\\n{\\n\"/*}*/));\n", name->str_text);
	indent_printf
	(
		"result = (void *)mem_alloc_clear(sizeof(struct %s));\n",
		name->str_text
	);
	indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
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
	indent_printf("%s\1j;\n", "size_t");
	indent_putchar('\n');
	indent_printf("if (!this)\n");
	indent_more();
	indent_printf("return;\n");
	indent_less();
	indent_printf("trace((\"%s_free(this = %%08lX)\\n{\\n\"/*}*/, this));\n", name->str_text);
	indent_printf("for (j = 0; j < this->length; ++j)\n");
	indent_more();
	s = str_from_c("list[j]");
	type_gen_free_declarator(type2->subtype, s, 1);
	str_free(s);
	indent_less();
	indent_printf("if (this->list)\n");
	indent_more();
	indent_printf("mem_free((char *)this->list);\n");
	indent_less();
	indent_printf("mem_free((char *)this);\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"static void %s_parse _((void *, type_ty **, void **));\n",
		name->str_text
	);

	indent_putchar('\n');
	indent_printf("static void\n");
	indent_printf("%s_parse(that, type_pp, addr_p)\n", name->str_text);
	indent_more();
	indent_printf("%s\1*that;\n", "void");
	indent_printf("%s\1**type_pp;\n", "type_ty");
	indent_printf("%s\1**addr_p;\n", "void");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1this = (%s)that;\n", name->str_text, name->str_text);
	indent_putchar('\n');
	indent_printf("trace((\"%s_parse(this = %%08lX, type_pp = %%08lX, addr_p = %%08lX)\\n{\\n\"/*}*/, this, type_pp, addr_p));\n", name->str_text);
	indent_printf("*type_pp = &%s_type;\n", type2->subtype->name->str_text);
	indent_printf("trace_pointer(*type_pp);\n");
	indent_printf("*addr_p = enlarge(&this->length, (char **)&this->list, sizeof(%s));\n", type->name->str_text);
	indent_printf("trace_pointer(*addr_p);\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("type_ty %s_type =\n", name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("type_class_list,\n");
	indent_printf("\"%s\",\n", name->str_text);
	indent_printf("%s_alloc,\n", name->str_text);
	indent_printf("%s_free,\n", name->str_text);
	indent_printf("0, /* enum_parse */\n");
	indent_printf("%s_parse,\n", name->str_text);
	indent_printf("0, /* struct_parse */\n");
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
		indent_printf("\"\"");
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
type_create_list(name, subtype)
	string_ty	*name;
	type_ty		*subtype;
{
	type_list_ty	*type;

	type = (type_list_ty *)mem_alloc(sizeof(type_list_ty));
	type->class = type_class_list;
	type->method = &method;
	type->name = str_copy(name);
	type->subtype = subtype;
	id_assign(name, ID_CLASS_TYPE, (long)type);
	return (type_ty *)type;
}
