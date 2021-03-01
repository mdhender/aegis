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
 * MANIFEST: grammar and functions to parse aegis file contents definitions
 */

%{

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <error.h>
#include <id.h>
#include <indent.h>
#include <lex.h>
#include <mem.h>
#include <parse.h>
#include <str.h>
#include <trace.h>
#include <type.h>

#ifdef DEBUG
#define YYDEBUG 1
extern int yydebug;
#define printf trace_where, trace_printf
#endif

%}

%token	TYPE
%token	NAME
%token	STRING
%token	STRING_CONSTANT
%token	INTEGER
%token	INTEGER_CONSTANT
%token	INCLUDE

%union
{
	string_ty	*lv_string;
	long		lv_integer;
	parse_list_ty	*lv_parse_list;
	type_ty		*lv_type;
}

%type <lv_string> NAME
%type <lv_integer> INTEGER_CONSTANT
%type <lv_parse_list> field_list enum_list
%type <lv_type> type structure list enumeration
%type <lv_string> type_name field field_name enum_name STRING_CONSTANT

%{

typedef struct name_ty name_ty;
struct name_ty
{
	string_ty	*name;
	name_ty		*prev;
};

static	parse_list_ty	*type_root;
static	name_ty		*name_root;


static void pl_append _((parse_list_ty **, string_ty *));

static void
pl_append(listp, s)
	parse_list_ty	**listp;
	string_ty	*s;
{
	parse_list_ty	*plp;

	trace(("pl_append(listp = %08lX, s = \"%s\")\n{\n"/*}*/, listp, s->str_text));
	while (*listp)
		listp = &(*listp)->next;
	plp = (parse_list_ty *)mem_alloc(sizeof(parse_list_ty));
	plp->name = str_copy(s);
	plp->next = 0;
	*listp = plp;
	trace((/*{*/"}\n"));
}


static void push_name _((string_ty *));

static void
push_name(s)
	string_ty *s;
{
	name_ty	*np;

	trace(("push_name(s = \"%s\")\n{\n"/*}*/, s->str_text));
	assert(name_root);
	np = (name_ty *)mem_alloc(sizeof(name_ty));
	np->name = str_format("%S_%S", name_root->name, s);
	np->prev = name_root;
	name_root = np;
	trace((/*{*/"}\n"));
}


static void push_name_abs _((string_ty *));

static void
push_name_abs(s)
	string_ty *s;
{
	name_ty	*np;

	trace(("push_name_abs(s = \"%s\")\n{\n"/*}*/, s->str_text));
	np = (name_ty *)mem_alloc(sizeof(name_ty));
	np->name = str_copy(s);
	np->prev = name_root;
	name_root = np;
	trace((/*{*/"}\n"));
}


static void pop_name _((void));

static void
pop_name()
{
	name_ty	*np;

	trace(("pop_name()\n{\n"/*}*/));
	np = name_root;
	name_root = np->prev;
	str_free(np->name);
	free(np);
	trace((/*{*/"}\n"));
}


static string_ty *get_name _((void));

static string_ty *
get_name()
{
	return str_copy(name_root->name);
}


static void define_type _((string_ty *, type_ty *));

static void
define_type(name, type)
	string_ty *name;
	type_ty	*type;
{
	trace(("define_type(name = \"%s\")\n{\n"/*}*/, name->str_text));
	type->included_flag = lex_in_include_file();
	pl_append(&type_root, name);
	trace((/*{*/"}\n"));
}


static char *basename _((char *));

static char *
basename(s)
	char		*s;
{
	static char	buffer[256];
	char		*cp;

	cp = strrchr(s, '/');
	if (cp)
		++cp;
	else
		cp = s;
	strcpy(buffer, cp);
	cp = strrchr(buffer, '.');
	if (cp)
		*cp = 0;
	for (cp = buffer; *cp; ++cp)
	{
		if (!isalnum(*cp))
			*cp = '_';
	}
	return buffer;
}


void
parse(definition_file, code_file, include_file)
	char		*definition_file;
	char		*code_file;
	char		*include_file;
{
	string_ty	*s;
	char		*cp1;
	parse_list_ty	*tnp;

	trace(("parse(def = \"%s\", c = \"%s\", h = \"%s\")\n{\n"/*}*/,
		definition_file, code_file, include_file));
#ifdef DEBUG
	yydebug = trace_pretest_;
#endif

	s = str_from_c(basename(definition_file));
	push_name_abs(s);

	lex_open(definition_file);
	trace(("yyparse()\n{\n"/*}*/));
	yyparse();
	trace((/*{*/"}\n"));
	lex_close();

	pop_name();

	indent_open(include_file);
	cp1 = basename(include_file);
	indent_putchar('\n');
	indent_printf("#ifndef %s_H\n", cp1);
	indent_printf("#define %s_H\n", cp1);
	indent_putchar('\n');
	indent_printf("#include <main.h>\n");
	indent_printf("#include <type.h>\n");
	indent_printf("#include <str.h>\n");
	indent_printf("#include <parse.h>\n");
	for (tnp = type_root; tnp; tnp = tnp->next)
	{
		type_ty	*tp;

		if (!id_search(tnp->name, ID_CLASS_TYPE, (long *)&tp))
			fatal("type \"%s\" lost!", tnp->name->str_text);
		type_gen_include(tp, tnp->name);
	}
	indent_putchar('\n');
	indent_printf
	(
		"void %s_write_file _((char *filename, %s value));\n",
		s->str_text,
		s->str_text
	);
	indent_printf
	(
		"%s %s_read_file _((char *filename));\n",
		s->str_text,
		s->str_text
	);
	indent_putchar('\n');
	indent_printf("#endif /* %s_H */\n", cp1);
	indent_close();

	cp1 = strrchr(include_file, '/');
	if (cp1)
		cp1++;
	else
		cp1 = include_file;

	indent_open(code_file);
	indent_putchar('\n');
	indent_printf("#include <stddef.h>\n");
	indent_printf("#include <stdio.h>\n");
	indent_putchar('\n');
	indent_printf("#include <%s>\n", cp1);
	indent_printf("#include <error.h>\n");
	indent_printf("#include <indent.h>\n");
	indent_printf("#include <io.h>\n");
	indent_printf("#include <mem.h>\n");
	indent_printf("#include <os.h>\n");
	indent_printf("#include <trace.h>\n");
	indent_printf("#include <type.h>\n");
	cp1 = basename(code_file);
	for (tnp = type_root; tnp; tnp = tnp->next)
	{
		type_ty	*tp;

		if (!id_search(tnp->name, ID_CLASS_TYPE, (long *)&tp))
			fatal("type \"%s\" lost!", tnp->name->str_text);
		if (tp->included_flag)
			continue;
		type_gen_code(tp, tnp->name);
	}
	indent_putchar('\n');
	indent_printf("%s\n", cp1);
	indent_printf("%s_read_file(filename)\n", s->str_text);
	indent_more();
	indent_printf("%s\1*filename;\n", "char");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1result;\n\n", cp1);
	indent_printf("trace((\"%s_read_file(filename = \\\"%%s\\\")\\n{\\n\"/*}*/, filename));\n", cp1);
	indent_printf("os_become_must_be_active();\n");
	indent_printf
	(
		"result = (%s)parse(filename, &%s_type);\n",
		s->str_text,
		s->str_text
	);
	indent_printf("trace((\"return %%08lX;\\n\", result));\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf("return result;\n");
	indent_printf(/*{*/"}\n");
	indent_putchar('\n');
	indent_printf("void\n");
	indent_printf("%s_write_file(filename, value)\n", s->str_text);
	indent_more();
	indent_printf("%s\1*filename;\n", "char");
	indent_printf("%s\1value;\n", s->str_text);
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("trace((\"%s_write_file(filename = \\\"%%s\\\", value = %%08lX)\\n{\\n\"/*}*/, filename, value));\n", cp1);
	indent_printf("if (filename)\n");
	indent_more();
	indent_printf("os_become_must_be_active();\n");
	indent_less();
	indent_printf("indent_open(filename);\n");
	indent_printf("%s_write(value);\n", s->str_text);
	indent_printf("indent_close();\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");
	indent_close();

	str_free(s);
	trace((/*{*/"}\n"));
}

%}

%%

description
	: typedef_list field_list
		{
			string_ty	*s;
			type_ty		*type;

			s = get_name();
			type = type_create_struct(s, $2, 1);
			define_type(s, type);
		}
	;

typedef_list
	: /* empty */
	| typedef_list typedef
	;

typedef
	: TYPE type_name '=' type ';'
		{
			str_free($2);
			pop_name();
		}
	| '#' INCLUDE STRING_CONSTANT
		{
			lex_open($3->str_text);
			str_free($3);
		}
	;

type_name
	: NAME
		{
			$$ = $1;
			push_name_abs($1);
		}
	;

field
	: field_name '=' type ';'
		{
			id_assign($1, ID_CLASS_FIELD, (long)$3);
			$$ = $1;
			pop_name();
		}
	;

field_name
	: NAME
		{
			push_name($1);
			$$ = $1;
		}
	;

type
	: STRING
		{
			$$ = type_create_string();
		}
	| INTEGER
		{
			$$ = type_create_integer();
		}
	| NAME
		{
			type_ty		*tp;

			if (id_search($1, ID_CLASS_TYPE, (long *)&tp))
			{
				/* $$ = type_create_ref(get_name(), $1); */
				$$ = tp;
			}
			else
			{
				yyerror("type \"%s\" undefined", $1->str_text);
				$$ = type_create_integer();
			}
			str_free($1);
		}
	| structure
		{ $$ = $1; }
	| list
		{ $$ = $1; }
	| enumeration
		{ $$ = $1; }
	;

structure
	: '{' field_list '}'
		{
			string_ty	*s;

			s = get_name();
			$$ = type_create_struct(s, $2, 0);
			define_type(s, $$);
		}
	;

field_list
	: /* empty */
		{
			$$ = 0;
		}
	| field_list field
		{
			$$ = $1;
			pl_append(&$$, $2);
		}
	;

list
	: '[' type ']'
		{
			string_ty	*s;
			static string_ty	*list;

			if (!list)
				list = str_from_c("list");
			push_name(list);
			s = get_name();
			$$ = type_create_list(s, $2);
			define_type(s, $$);
			pop_name();
		}
	;

enumeration
	: '(' enum_list optional_comma ')'
		{
			string_ty *s;

			s = get_name();
			$$ = type_create_enum(s, $2);
			define_type(s, $$);
		}
	;

optional_comma
	: /* empty */
	| ','
	;

enum_list
	: enum_name
		{
			$$ = 0;
			pl_append(&$$, $1);
		}
	| enum_list ',' enum_name
		{
			$$ = $1;
			pl_append(&$$, $3);
		}
	;

enum_name
	: NAME
		{
			string_ty *s;

			push_name($1);
			s = get_name();
			pop_name();
			id_assign(s, ID_CLASS_ENUMEL, (long)$1);
			$$ = s;
		}
	;
