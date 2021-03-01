/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1997-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: grammar and functions to parse aegis file contents definitions
 */

%{

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <error.h>
#include <indent.h>
#include <lex.h>
#include <mem.h>
#include <parse.h>
#include <str.h>
#include <symtab.h>
#include <trace.h>
#include <type/enumeration.h>
#include <type/integer.h>
#include <type/list.h>
#include <type/real.h>
#include <type/string.h>
#include <type/structure.h>
#include <type/time.h>
#include <str_list.h>

#ifdef DEBUG
#define YYDEBUG 1
extern int yydebug;
#define printf trace_where, trace_printf
#endif

%}

%token	SHOW_IF_DEFAULT
%token	HIDE_IF_DEFAULT
%token	INCLUDE
%token	INTEGER
%token	INTEGER_CONSTANT
%token	NAME
%token	REAL
%token	STRING
%token	STRING_CONSTANT
%token	TIME
%token	TYPE

%union
{
	string_ty	*lv_string;
	long		lv_integer;
	type_ty		*lv_type;
}

%type <lv_string> NAME STRING_CONSTANT
%type <lv_integer> INTEGER_CONSTANT if_default_clause
%type <lv_type> type structure list enumeration enum_list_begin

%{

typedef struct name_ty name_ty;
struct name_ty
{
	name_ty		*parent;
	string_ty	*name_short;
	string_ty	*name_long;
	type_ty		*type;
};

static name_ty	*current;
static size_t	emit_length;
static size_t	emit_length_max;
static type_ty	**emit_list;
static int	time_used;
static symtab_ty *typedef_symtab;
static string_list_ty	initialize;


static void push_name _((string_ty *));

static void
push_name(s)
	string_ty	*s;
{
	name_ty		*np;

	trace(("push_name(s = \"%s\")\n{\n"/*}*/, s->str_text));
	np = mem_alloc(sizeof(name_ty));
	np->name_short = str_copy(s);
	np->name_long = str_format("%S_%S", current->name_long, s);
	np->parent = current;
	np->type = 0;
	current = np;
	trace((/*{*/"}\n"));
}


static void push_name_abs _((string_ty *));

static void
push_name_abs(s)
	string_ty	*s;
{
	name_ty		*np;

	trace(("push_name_abs(s = \"%s\")\n{\n"/*}*/, s->str_text));
	np = mem_alloc(sizeof(name_ty));
	np->name_short = str_copy(s);
	np->name_long = str_copy(s);
	np->parent = current;
	np->type = 0;
	current = np;
	trace((/*{*/"}\n"));
}


static void pop_name _((void));

static void
pop_name()
{
	name_ty		*np;

	trace(("pop_name()\n{\n"/*}*/));
	np = current;
	current = np->parent;
	str_free(np->name_short);
	str_free(np->name_long);
	mem_free(np);
	trace((/*{*/"}\n"));
}


static void define_type _((type_ty *));

static void
define_type(type)
	type_ty		*type;
{
	size_t		nbytes;

	trace(("define_type(type = %08lX)\n{\n"/*}*/, (long)type));
	if (emit_length >= emit_length_max)
	{
		emit_length_max += 10;
		nbytes = emit_length_max * sizeof(emit_list[0]);
		emit_list = mem_change_size(emit_list, nbytes);
	}
	emit_list[emit_length++] = type;
	trace((/*{*/"}\n"));
}


static char *base_name _((char *));

static char *
base_name(s)
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


static void generate_include_file _((char *));

static void
generate_include_file(include_file)
	char		*include_file;
{
	char		*cp1;
	size_t		j;
	string_ty	*s;

	trace(("generate_include_file(h = \"%s\")\n{\n"/*}*/, include_file));
	s = current->name_long;
	indent_open(include_file);
	cp1 = base_name(include_file);
	indent_putchar('\n');
	indent_printf("#ifndef %s_H\n", cp1);
	indent_printf("#define %s_H\n", cp1);
	indent_putchar('\n');
	if (time_used)
	{
		indent_printf("#include <ac/time.h>\n");
		indent_putchar('\n');
	}
	indent_printf("#include <type.h>\n");
	indent_printf("#include <str.h>\n");
	indent_printf("#include <parse.h>\n");
	indent_putchar('\n');
	indent_printf("struct output_ty; /* existence */\n");
	for (j = 0; j < emit_length; ++j)
		type_gen_include(emit_list[j]);
	indent_putchar('\n');
	indent_printf
	(
	   "void %s_write_file _((string_ty *filename, %s value, int comp));\n",
		s->str_text,
		s->str_text
	);
	indent_printf
	(
		"%s %s_read_file _((string_ty *filename));\n",
		s->str_text,
		s->str_text
	);
	indent_printf("void %s__rpt_init _((void));\n", s->str_text);
	indent_putchar('\n');
	indent_printf("#endif /* %s_H */\n", cp1);
	indent_close();
	trace((/*{*/"}\n"));
}


static void generate_code_file _((char *, char *));

static void
generate_code_file(code_file, include_file)
	char		*code_file;
	char		*include_file;
{
	char		*cp1;
	size_t		j;
	string_ty	*s;

	trace(("generate_code_file(c = \"%s\", h = \"%s\")\n{\n"/*}*/,
		code_file, include_file));
	cp1 = strrchr(include_file, '/');
	if (cp1)
		cp1++;
	else
		cp1 = include_file;
	s = current->name_short;
	indent_open(code_file);
	indent_putchar('\n');
	indent_printf("#include <ac/stddef.h>\n");
	indent_printf("#include <ac/stdio.h>\n");
	indent_putchar('\n');
	indent_printf("#include <%s>\n", cp1);
	indent_printf("#include <error.h>\n");
	indent_printf("#include <output/indent.h>\n");
	indent_printf("#include <output/file.h>\n");
	indent_printf("#include <output/gzip.h>\n");
	indent_printf("#include <io.h>\n");
	indent_printf("#include <mem.h>\n");
	indent_printf("#include <os.h>\n");
	indent_printf("#include <trace.h>\n");
	indent_printf("#include <type.h>\n");
	cp1 = base_name(code_file);
	for (j = 0; j < emit_length; ++j)
	{
		type_ty		*tp;

		tp = emit_list[j];
		if (tp->included_flag)
			continue;
		type_gen_code(tp);
	}
	indent_putchar('\n');
	indent_printf("%s\n", cp1);
	indent_printf("%s_read_file(filename)\n", s->str_text);
	indent_more();
	indent_printf("%s\1*filename;\n", "string_ty");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("%s\1result;\n\n", cp1);
	indent_printf
	(
"trace((\"%s_read_file(filename = \\\"%%s\\\")\\n{\\n\"/*}*/, \
(filename ? filename->str_text : \"\")));\n",
		cp1
	);
	indent_printf("os_become_must_be_active();\n");
	indent_printf
	(
		"result = parse(filename, &%s_type);\n",
		s->str_text
	);
	indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf("return result;\n");
	indent_printf(/*{*/"}\n");
	indent_putchar('\n');
	indent_printf("void\n");
	indent_printf
	(
		"%s_write_file(filename, value, compress)\n",
		s->str_text
	);
	indent_more();
	indent_printf("%s\1*filename;\n", "string_ty");
	indent_printf("%s\1value;\n", s->str_text);
	indent_printf("%s\1compress;\n", "int");
	indent_less();
	indent_printf("{\n"/*}*/);
	indent_printf("output_ty *fp;\n\n");
	indent_printf
	(
		"trace((\"%s_write_file(filename = \\\"%%s\\\", value = \
%%08lX)\\n{\\n\"/*}*/, (filename ? filename->str_text : \"\"), \
(long)value));\n",
		cp1
	);
	indent_printf("if (filename)\n");
	indent_more();
	indent_printf("os_become_must_be_active();\n");
	indent_less();
	indent_printf("if (compress)\n{\n");
	indent_printf("fp = output_file_binary_open(filename);\n");
	indent_printf("fp = output_gzip(fp);\n");
	indent_printf("}\nelse\n{\n");
	indent_printf("fp = output_file_text_open(filename);\n");
	indent_printf("}\n");
	indent_printf("fp = output_indent(fp);\n");
	indent_printf("io_comment_emit(fp);\n");
	indent_printf("%s_write(fp, value);\n", s->str_text);
	indent_printf("type_enum_option_clear();\n");
	indent_printf("output_delete(fp);\n");
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");

	indent_putchar('\n');
	indent_printf("void\n");
	indent_printf("%s__rpt_init()\n", s->str_text);
	indent_printf("{\n"/*}*/);
	indent_printf("trace((\"%s__rpt_init()\\n{\\n\"/*}*/));\n", cp1);
	for (j = 0; j < initialize.nstrings; ++j)
		indent_printf("%s\n", initialize.string[j]->str_text);
	indent_printf("trace((/*{*/\"}\\n\"));\n");
	indent_printf(/*{*/"}\n");
	indent_close();
	trace((/*{*/"}\n"));
}


void
generate_code__init(s)
	string_ty	*s;
{
	string_list_append(&initialize, s);
}


void
parse(definition_file, code_file, include_file)
	char		*definition_file;
	char		*code_file;
	char		*include_file;
{
	string_ty	*s;
	extern int yyparse _((void));

	/*
	 * initial name is the basename of the definition file
	 */
	trace(("parse(def = \"%s\", c = \"%s\", h = \"%s\")\n{\n"/*}*/,
		definition_file, code_file, include_file));
#ifdef DEBUG
	yydebug = trace_pretest_;
#endif
	s = str_from_c(base_name(definition_file));
	push_name_abs(s);
	str_free(s);
	typedef_symtab = symtab_alloc(10);

	/*
	 * parse the definition file
	 */
	lex_open(definition_file);
	trace(("yyparse()\n{\n"/*}*/));
	yyparse();
	trace((/*{*/"}\n"));
	lex_close();

	/*
	 * remember to emit a structure containing its fields
	 */
	type_structure_toplevel(current->type);
	define_type(current->type);

	/*
	 * generate the files
	 */
	generate_include_file(include_file);
	generate_code_file(code_file, include_file);
	pop_name();
	trace((/*{*/"}\n"));
}

%}

%%

description
	: typedef_list field_list
	;

typedef_list
	: /* empty */
	| typedef_list typedef
	;

typedef
	: TYPE type_name '=' type ';'
		{
			$4->is_a_typedef = 1;
			symtab_assign(typedef_symtab, current->name_long, $4);
			pop_name();
			if (lex_in_include_file())
				type_in_include_file($4);
		}
	| '#' INCLUDE STRING_CONSTANT
		{
			lex_open($3->str_text);
			str_free($3);
		}
	| error
	;

type_name
	: NAME
		{
			push_name_abs($1);
		}
	;

field
	: field_name '=' type if_default_clause ';'
		{
			type_member_add
			(
				current->parent->type,
				current->name_short,
				$3,
				$4
			);
			pop_name();
		}
	| field_name error
		{
			pop_name();
		}
	;

field_name
	: NAME
		{
			push_name($1);
			str_free($1);
		}
	;

type
	: STRING
		{
			$$ = type_new(&type_string, (string_ty *)0);
		}
	| INTEGER
		{
			$$ = type_new(&type_integer, (string_ty *)0);
		}
	| REAL
		{
			$$ = type_new(&type_real, (string_ty *)0);
		}
	| TIME
		{
			time_used = 1;
			$$ = type_new(&type_time, (string_ty *)0);
		}
	| NAME
		{
			type_ty		*data;

			data = symtab_query(typedef_symtab, $1);
			if (data)
				$$ = data;
			else
			{
				yyerror("type \"%s\" undefined", $1->str_text);
				$$ = type_new(&type_integer, (string_ty *)0);
			}
			str_free($1);
		}
	| structure
		{
			$$ = $1;
			define_type($$);
		}
	| list
		{
			$$ = $1;
			define_type($$);
		}
	| enumeration
		{
			$$ = $1;
			define_type($$);
		}
	;

structure
	: '{' field_list '}'
		{
			$$ = current->type;
		}
	;

field_list
	: /* empty */
		{
			current->type =
				type_new(&type_structure, current->name_long);
		}
	| field_list field
	;

list
	: '[' type ']'
		{
			static string_ty	*list;

			if (!list)
				list = str_from_c("list");
			push_name(list);
			$$ = type_new(&type_list, current->name_long);
			type_member_add($$, (string_ty *)0, $2, 1);
			pop_name();
		}
	;

enumeration
	: '(' enum_list_begin enum_list optional_comma ')'
		{
			$$ = $2;
		}
	;

enum_list_begin
	: /* empty */
		{
			$$ = type_new(&type_enumeration, current->name_long);
			current->type = $$;
		}
	;

enum_list
	: enum_name
	| enum_list ',' enum_name
	;

enum_name
	: NAME
		{
			push_name($1);
			str_free($1);
			type_member_add
			(
				current->parent->type,
				current->name_short,
				(type_ty *)0,
				1
			);
			pop_name();
		}
	;

optional_comma
	: /* empty */
	| ','
	;

if_default_clause
	: /* empty */
		{ $$ = -1; }
	| SHOW_IF_DEFAULT
		{ $$ = 1; }
	| HIDE_IF_DEFAULT
		{ $$ = 0; }
	;
