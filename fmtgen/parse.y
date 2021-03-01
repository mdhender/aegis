/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1997-1999, 2001-2008 Peter Miller
 *	Copyright (C) 2007 Walter Franzini
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see
 *	<http://www.gnu.org/licenses/>.
 *
 */

%{

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <fmtgen/indent.h>
#include <fmtgen/lex.h>
#include <fmtgen/parse.h>
#include <fmtgen/type/boolean.h>
#include <fmtgen/type/enumeration.h>
#include <fmtgen/type/integer.h>
#include <fmtgen/type/list.h>
#include <fmtgen/type/real.h>
#include <fmtgen/type/string.h>
#include <fmtgen/type/structure.h>
#include <fmtgen/type/time.h>

#ifdef DEBUG
#define YYDEBUG 1
extern int yydebug;
#define printf lex_debug_printf
#define fprintf lex_debug_fprintf
#endif

%}

%token	BOOLEAN
%token	BOOLEAN_CONSTANT
%token	HIDE_IF_DEFAULT
%token	INCLUDE
%token	INTEGER
%token	INTEGER_CONSTANT
%token	NAME
%token	REAL
%token	REDEFINITION_OK
%token	SHOW_IF_DEFAULT
%token	STRING
%token	STRING_CONSTANT
%token	TIME
%token	TYPE

%union
{
    string_ty       *lv_string;
    long            lv_integer;
    type *lv_type;
}

%type <lv_string> NAME STRING_CONSTANT
%type <lv_integer> INTEGER_CONSTANT attributes
%type <lv_type> type structure list enumeration enum_list_begin

%{

struct name_ty
{
    name_ty         *parent;
    string_ty       *name_short;
    string_ty       *name_long;
    type *name_type;
};

static name_ty	*current;
static size_t	emit_length;
static size_t	emit_length_max;
static type **emit_list;
static int	time_used;
static symtab_ty *typedef_symtab;
static string_list_ty	initialize;


static void
push_name(string_ty *s)
{
    name_ty         *np;

    trace(("push_name(s = \"%s\")\n{\n", s->str_text));
    np = (name_ty *)mem_alloc(sizeof(name_ty));
    np->name_short = str_copy(s);
    np->name_long =
	str_format("%s_%s", current->name_long->str_text, s->str_text);
    np->parent = current;
    np->name_type = 0;
    current = np;
    trace(("}\n"));
}


static void
push_name_abs(string_ty *s)
{
    name_ty         *np;

    trace(("push_name_abs(s = \"%s\")\n{\n", s->str_text));
    np = (name_ty *)mem_alloc(sizeof(name_ty));
    np->name_short = str_copy(s);
    np->name_long = str_copy(s);
    np->parent = current;
    np->name_type = 0;
    current = np;
    trace(("}\n"));
}


static void
pop_name(void)
{
    name_ty         *np;

    trace(("pop_name()\n{\n"));
    np = current;
    current = np->parent;
    str_free(np->name_short);
    str_free(np->name_long);
    mem_free(np);
    trace(("}\n"));
}


static void
define_type(type *defined_type)
{
    trace(("define_type(defined_type = %08lX)\n{\n", (long)defined_type));
    if (emit_length >= emit_length_max)
    {
	size_t new_emit_length_max = emit_length_max * 2 + 8;
	type **new_emit_list = new type * [new_emit_length_max];
	for (size_t j = 0; j < emit_length; ++j)
	    new_emit_list[j] = emit_list[j];
	delete [] emit_list;
	emit_list = new_emit_list;
	emit_length_max = new_emit_length_max;
    }
    emit_list[emit_length++] = defined_type;
    trace(("}\n"));
}


static const char *
base_name(const char *s)
{
    static char     buffer[256];
    const char      *cp;
    char            *bp;

    cp = strrchr(s, '/');
    if (cp)
	++cp;
    else
	cp = s;
    strendcpy(buffer, cp, buffer + sizeof(buffer));
    bp = strrchr(buffer, '.');
    if (bp)
	*bp = 0;
    for (bp = buffer; *bp; ++bp)
    {
	if (!isalnum((unsigned char)*bp))
    	    *bp = '_';
    }
    return buffer;
}


static const char *
calculate_include_define_name(const char *s)
{
    char            *bp;
    static char     buffer[256];

    bp = buffer;
    while (*s && bp < ENDOF(buffer) - 1)
    {
	unsigned char   c;

	c = *s++;
	if (!c)
	    break;
	if (islower(c))
	    c = toupper(c);
	else if (!isalnum(c))
	    c = '_';
	*bp++ = c;
    }
    *bp = 0;
    return buffer;
}


static void
this_file_is_generated(const char *definition_file)
{
    /*
     * DO NOT insert a timestamp in the generated files, it needlessly
     * changes the fingerprint.  This, in turn, causes cook to cook
     * too much, compiling several hundred files which do not need to
     * be compiled.
     */
    indent_printf
    (
	"//\n"
	"// This file is generated by %s from \"%s\".\n"
	"// If you want to change the contents of this file\n"
	"// you need to edit %s\n"
	"// or you need to enhance %s.\n"
	"//\n",
	progname_get(),
	definition_file,
	definition_file,
	progname_get()
    );
}


static void
generate_include_file(const char *include_file, const char *definition_file)
{
    const char      *cp1;
    size_t	    j;
    string_ty	    *s;

    trace(("generate_include_file(h = \"%s\")\n{\n", include_file));
    s = current->name_long;
    indent_open(include_file);
    this_file_is_generated(definition_file);
    cp1 = calculate_include_define_name(include_file);
    indent_putchar('\n');
    indent_printf("#ifndef %s\n", cp1);
    indent_printf("#define %s\n", cp1);
    indent_putchar('\n');
    if (time_used)
    {
	indent_printf("#include <common/ac/time.h>\n");
	indent_putchar('\n');
    }
    indent_printf("#include <common/str.h>\n");
    indent_printf("#include <libaegis/output.h>\n");
    indent_printf("#include <libaegis/meta_lex.h>\n");
    indent_printf("#include <libaegis/meta_parse.h>\n");
    indent_printf("#include <libaegis/meta_type.h>\n");
    indent_putchar('\n');
    indent_printf("class nstring; // forward\n");
    for (j = 0; j < emit_length; ++j)
	emit_list[j]->gen_include();
    indent_putchar('\n');
    indent_printf("/**\n");
    indent_printf("  * The %s_write_file function is used to\n", s->str_text);
    indent_printf("  * write %s meta data to the named file.\n", s->str_text);
    indent_printf("  *\n");
    indent_printf("  * @param filename\n");
    indent_printf("  *     The name of the file to be written.\n");
    indent_printf("  * @param value\n");
    indent_printf("  *     The value of the meta-data to be written.\n");
    indent_printf("  * @param comp\n");
    indent_printf("  *     true (non-zero) if data should be compressed.\n");
    indent_printf("  * @note\n");
    indent_printf("  *      If any errors are encountered, this\n");
    indent_printf("  *      function will not return.  All errors\n");
    indent_printf("  *      will print a fatal error message, and\n");
    indent_printf("  *      exit with an exit status of 1.\n");
    indent_printf("  */\n");
    indent_printf
    (
	"void %s_write_file(string_ty *filename, %s_ty *value, int comp);\n",
	s->str_text,
	s->str_text
    );

    indent_putchar('\n');
    indent_printf("/**\n");
    indent_printf("  * The %s_write_file function is used to\n", s->str_text);
    indent_printf("  * write %s meta data to the named file.\n", s->str_text);
    indent_printf("  *\n");
    indent_printf("  * @param filnam\n");
    indent_printf("  *     The name of the file to be written.\n");
    indent_printf("  * @param value\n");
    indent_printf("  *     The value of the meta-data to be written.\n");
    indent_printf("  * @param comp\n");
    indent_printf("  *     true if data should be compressed.\n");
    indent_printf("  * @note\n");
    indent_printf("  *      If any errors are encountered, this\n");
    indent_printf("  *      function will not return.  All errors\n");
    indent_printf("  *      will print a fatal error message, and\n");
    indent_printf("  *      exit with an exit status of 1.\n");
    indent_printf("  */\n");
    indent_printf
    (
	"void %s_write_file(const nstring &filnam, %s_ty *value, bool comp);\n",
	s->str_text,
	s->str_text
    );

    indent_putchar('\n');
    indent_printf("/**\n");
    indent_printf("  * The %s_read_file function is used to\n", s->str_text);
    indent_printf("  * read %s meta data from the named file.\n", s->str_text);
    indent_printf("  *\n");
    indent_printf("  * @param filename\n");
    indent_printf("  *     The name of the file to be read.\n");
    indent_printf("  * @returns\n");
    indent_printf("  *     a pointer to a dynamically allocated\n");
    indent_printf("  *     value read from the file.\n");
    indent_printf("  * @note\n");
    indent_printf("  *      If any errors are encountered, this\n");
    indent_printf("  *      function will not return.  All errors\n");
    indent_printf("  *      (including syntax errors) will print a\n");
    indent_printf("  *      fatal error message, and exit with an\n");
    indent_printf("  *      exit status of 1.\n");
    indent_printf("  */\n");
    indent_printf
    (
	"%s_ty *%s_read_file(string_ty *filename);\n",
	s->str_text,
	s->str_text
    );

    indent_putchar('\n');
    indent_printf("/**\n");
    indent_printf("  * The %s_read_file function is used to\n", s->str_text);
    indent_printf("  * read %s meta data from the named file.\n", s->str_text);
    indent_printf("  *\n");
    indent_printf("  * @param filename\n");
    indent_printf("  *     The name of the file to be read.\n");
    indent_printf("  * @returns\n");
    indent_printf("  *     a pointer to a dynamically allocated\n");
    indent_printf("  *     value read from the file.\n");
    indent_printf("  * @note\n");
    indent_printf("  *      If any errors are encountered, this\n");
    indent_printf("  *      function will not return.  All errors\n");
    indent_printf("  *      (including syntax errors) will print a\n");
    indent_printf("  *      fatal error message, and exit with an\n");
    indent_printf("  *      exit status of 1.\n");
    indent_printf("  */\n");
    indent_printf
    (
	"%s_ty *%s_read_file(const nstring &filename);\n",
	s->str_text,
	s->str_text
    );

    indent_printf("void %s__rpt_init(void);\n", s->str_text);
    indent_putchar('\n');
    indent_printf("#endif // %s\n", cp1);
    indent_close();
    trace(("}\n"));
}


static void
generate_code_file(const char *code_file, const char *include_file,
    const char *definition_file)
{
    size_t	    j;
    string_ty	    *s;

    trace(("generate_code_file(c = \"%s\", h = \"%s\")\n{\n", code_file,
	include_file));
    s = current->name_short;
    indent_open(code_file);
    this_file_is_generated(definition_file);
    indent_putchar('\n');
    indent_printf("#include <common/ac/stddef.h>\n");
    indent_printf("#include <common/ac/stdio.h>\n");
    indent_putchar('\n');
    indent_printf("#include <common/error.h>\n");
    indent_printf("#include <common/mem.h>\n");
    indent_printf("#include <common/trace.h>\n");
    indent_printf("#include <libaegis/io.h>\n");
    indent_printf("#include <libaegis/meta_type.h>\n");
    indent_printf("#include <libaegis/os.h>\n");
    indent_printf("#include <libaegis/output/file.h>\n");
    indent_printf("#include <libaegis/output/gzip.h>\n");
    indent_printf("#include <libaegis/output/indent.h>\n");
    indent_printf("#include <%s>\n", include_file);
    const char *cp1 = base_name(code_file);
    for (j = 0; j < emit_length; ++j)
    {
	type *tp = emit_list[j];
	if (!tp->is_in_include_file())
	    tp->gen_code();
    }
    indent_putchar('\n');
    indent_printf("%s_ty *\n", cp1);
    indent_printf("%s_read_file(const nstring &filename)\n", s->str_text);
    indent_printf("{\n");
    indent_printf("return %s_read_file(filename.get_ref());\n", s->str_text);
    indent_printf("}\n");
    indent_putchar('\n');
    indent_printf("%s_ty *\n", cp1);
    indent_printf("%s_read_file(string_ty *filename)\n", s->str_text);
    indent_printf("{\n");
    indent_printf("%s_ty\1*result;\n\n", cp1);
    indent_printf
    (
	"trace((\"%s_read_file(filename = \\\"%%s\\\")\\n{\\n\", "
	    "(filename ? filename->str_text : \"\")));\n",
	cp1
    );
    indent_printf("os_become_must_be_active();\n");
    indent_printf
    (
	"result = (%s_ty *)parse(filename, &%s_type);\n",
	cp1,
	s->str_text
    );
    indent_printf("trace((\"return %%08lX;\\n\", (long)result));\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("return result;\n");
    indent_printf("}\n");
    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write_file(const nstring &filename, %s_ty *value, "
	    "bool comp)\n",
	s->str_text,
	s->str_text
    );
    indent_printf("{\n");
    indent_printf
    (
	"%s_write_file(filename.get_ref(), value, comp);\n",
	s->str_text
    );
    indent_printf("}\n");
    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf
    (
	"%s_write_file(string_ty *filename, %s_ty *value, "
	    "int needs_compression)\n",
	s->str_text,
	s->str_text
    );
    indent_printf("{\n");
    indent_printf
    (
	"trace((\"%s_write_file(filename = \\\"%%s\\\", value = %%08lX)\\n"
	    "{\\n\", (filename ? filename->str_text : \"\"), (long)value));\n",
	cp1
    );
    indent_printf("if (filename)\n");
    indent_more();
    indent_printf("os_become_must_be_active();\n");
    indent_less();
    indent_printf("output::pointer fp;\n");
    indent_printf("if (needs_compression)\n{\n");
    indent_printf("fp = output_file::binary_open(filename);\n");
    indent_printf("fp = output_gzip::create(fp);\n");
    indent_printf("}\nelse\n{\n");
    indent_printf("fp = output_file::text_open(filename);\n");
    indent_printf("}\n");
    indent_printf("fp = output_indent::create(fp);\n");
    indent_printf("io_comment_emit(fp);\n");
    indent_printf("%s_write(fp, value);\n", s->str_text);
    indent_printf("type_enum_option_clear();\n");
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");

    indent_putchar('\n');
    indent_printf("void\n");
    indent_printf("%s__rpt_init(void)\n", s->str_text);
    indent_printf("{\n");
    indent_printf("trace((\"%s__rpt_init()\\n{\\n\"));\n", cp1);
    for (j = 0; j < initialize.nstrings; ++j)
	indent_printf("%s\n", initialize.string[j]->str_text);
    indent_printf("trace((\"}\\n\"));\n");
    indent_printf("}\n");
    indent_close();
    trace(("}\n"));
}


void
generate_code__init(const nstring &s)
{
    initialize.push_back(s.get_ref());
}


void
parse(const char *definition_file, const char *code_file,
    const char *include_file)
{
    string_ty       *s;
    extern int      yyparse(void);

    /*
     * initial name is the basename of the definition file
     */
    trace(("parse(def = \"%s\", c = \"%s\", h = \"%s\")\n{\n",
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
    trace(("yyparse()\n{\n"));
    yyparse();
    trace(("}\n"));
    lex_close();

    /*
     * remember to emit a structure containing its fields
     */
    current->name_type->toplevel();
    define_type(current->name_type);

    /*
     * generate the files
     */
    generate_include_file(include_file, definition_file);
    generate_code_file(code_file, include_file, definition_file);
    pop_name();
    trace(("}\n"));
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
	    $4->typedef_set();
	    symtab_assign(typedef_symtab, current->name_long, $4);
	    pop_name();
	    if (lex_in_include_file())
		$4->in_include_file();
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
    : field_name '=' type attributes ';'
	{
	    current->parent->name_type->member_add
	    (
		nstring(current->name_short),
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
	    $$ = new type_string();
	}
    | BOOLEAN
	{
	    $$ = new type_boolean();
	}
    | INTEGER
	{
	    $$ = new type_integer();
	}
    | REAL
	{
	    $$ = new type_real();
	}
    | TIME
	{
	    time_used = 1;
	    $$ = new type_time();
	}
    | NAME
	{
	    type *data = (type *)symtab_query(typedef_symtab, $1);
	    if (data)
		$$ = data;
	    else
	    {
		yyerror("type \"%s\" undefined", $1->str_text);
		$$ = new type_integer();
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
	    $$ = current->name_type;
	}
    ;

field_list
    : /* empty */
	{
	    current->name_type =
                new type_structure(nstring(current->name_long));
	}
    | field_list field
    ;

list
    : '[' type ']'
	{
	    static string_ty    *list;

	    if (!list)
		list = str_from_c("list");
	    push_name(list);
	    $$ = new type_list(nstring(current->name_long), $2);
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
	    $$ = new type_enumeration(nstring(current->name_long));
	    current->name_type = $$;
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
	    current->parent->name_type->member_add
	    (
		nstring(current->name_short),
		(type *)0,
		1
	    );
	    pop_name();
	}
    ;

optional_comma
    : /* empty */
    | ','
    ;

attributes
    : /* empty */
	{ $$ = 0; }
    | attributes REDEFINITION_OK
	{ $$ = $1 | ATTRIBUTE_REDEFINITION_OK; }
    | attributes SHOW_IF_DEFAULT
	{ $$ = $1 | ATTRIBUTE_SHOW_IF_DEFAULT; }
    | attributes HIDE_IF_DEFAULT
	{ $$ = $1 | ATTRIBUTE_HIDE_IF_DEFAULT; }
    ;
