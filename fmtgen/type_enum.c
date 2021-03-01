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
 * MANIFEST: enumerated types functions gereation
 */

#include <type.h>
#include <indent.h>
#include <id.h>
#include <mem.h>


typedef struct type_enum_ty type_enum_ty;
struct type_enum_ty
{
	/* inherited */
	TYPE_T

	/* instance variables */
	parse_list_ty *list;
};


static void gen_include _((type_ty *, string_ty *));

static void
gen_include(type, name)
	type_ty		*type;
	string_ty	*name;
{
	type_enum_ty	*type2;
	parse_list_ty	*np;

	type2 = (type_enum_ty *)type;
	indent_putchar('\n');
	indent_printf("#ifndef %s_DEF\n", name->str_text);
	indent_printf("#define %s_DEF\n", name->str_text);
	indent_printf("enum %s\n", name->str_text);
	indent_printf("{\n"/*}*/);
	for (np = type2->list; np; np = np->next)
	{
		indent_printf("%s", np->name->str_text);
		if (np->next)
			indent_putchar(',');
		indent_putchar('\n');
	}
	indent_printf(/*{*/"};\n");
	indent_printf("typedef enum %s %s;\n", name->str_text, name->str_text);
	indent_printf("#endif /* %s_DEF */\n", name->str_text);

	indent_putchar('\n');
	indent_printf("extern type_ty %s_type;\n", name->str_text);

	indent_putchar('\n');
	indent_printf("void %s_write _((char *, %s));\n", name->str_text, name->str_text);
	indent_printf("char *%s_ename _((%s));\n", name->str_text, name->str_text);
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
	type_enum_ty	*type2;
	int		n;

	type2 = (type_enum_ty *)type;
	indent_putchar('\n');
	indent_printf("static char *%s_s[] =\n", name->str_text);
	indent_printf("{\n"/*}*/);
	for (np = type2->list, n = 0; np; np = np->next, ++n)
	{
		string_ty *s;

		id_search(np->name, ID_CLASS_ENUMEL, (long *)&s);
		indent_printf("\"%s\",\n", s->str_text);
	}
	indent_printf(/*{*/"};\n");

	indent_putchar('\n');
	indent_printf("char *\n");
	indent_printf("%s_ename(this)\n", name->str_text);
	indent_more();
	indent_printf("%s\1this;\n", name->str_text);
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("static char\1buffer[20];\n\n");
	indent_printf("if (this >= 0 && this < %d)\n", n);
	indent_more();
	indent_printf("return %s_s[this];\n", name->str_text);
	indent_less();
	indent_printf("sprintf(buffer, \"%%d\", this);\n");
	indent_printf("return buffer;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("void\n");
	indent_printf("%s_write(name, this)\n", name->str_text);
	indent_more();
	indent_printf("%s\1*name;\n", "char");
	indent_printf("%s\1this;\n", name->str_text);
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("if (name)\n");
	indent_more();
	indent_printf("indent_printf(\"%%s = \", name);\n");
	indent_less();
	indent_printf("indent_printf(\"%%s\", %s_s[this]);\n", name->str_text);
	indent_printf("if (name)\n");
	indent_more();
	indent_printf("indent_printf(\";\\n\");\n");
	indent_less();
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf
	(
		"static int %s_parse _((string_ty *, void *));\n",
		name->str_text
	);

	indent_putchar('\n');
	indent_printf("static int\n");
	indent_printf("%s_parse(name, addr)\n", name->str_text);
	indent_more();
	indent_printf("%s\1*name;\n", "string_ty");
	indent_printf("%s\1*addr;\n", "void");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1*%s_f[SIZEOF(%s_s)];\n", "static string_ty", name->str_text, name->str_text);
	indent_printf("%s\1j;\n", "int");
	indent_putchar('\n');
	indent_printf("slow_to_fast(%s_s, %s_f, SIZEOF(%s_s));\n", name->str_text, name->str_text, name->str_text);
	indent_printf("for (j = 0; j < SIZEOF(%s_f); ++j)\n", name->str_text);
	indent_printf("{\n");
	indent_printf("if (str_equal(name, %s_f[j]))\n", name->str_text);
	indent_printf("{\n");
	indent_printf("*(%s *)addr = j;\n", name->str_text);
	indent_printf("return 0;\n");
	indent_printf("}\n");
	indent_printf("}\n");
	indent_printf("return -1;\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("type_ty %s_type =\n", name->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("type_class_enum,\n");
	indent_printf("\"%s\",\n", name->str_text);
	indent_printf("0, /* alloc */\n");
	indent_printf("0, /* free */\n");
	indent_printf("%s_parse,\n", name->str_text);
	indent_printf("0, /* list_parse */\n");
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
	if (is_a_list)
		indent_printf(";\n");
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
type_create_enum(name, list)
	string_ty	*name;
	parse_list_ty	*list;
{
	type_enum_ty	*type;

	type = (type_enum_ty *)mem_alloc(sizeof(type_enum_ty));
	type->class = type_class_enum;
	type->method = &method;
	type->name = str_copy(name);
	type->list = list;
	id_assign(name, ID_CLASS_TYPE, (long)type);
	return (type_ty *)type;
}
