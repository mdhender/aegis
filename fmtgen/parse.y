/*
 * aegis - project change supervisor
 * Copyright (C) 1991-1994, 1997-1999, 2001-2008, 2012 Peter Miller
 * Copyright (C) 2007 Walter Franzini
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

%{

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/progname.h>
#include <common/sizeof.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <fmtgen/generator.h>
#include <fmtgen/indent.h>
#include <fmtgen/lex.h>
#include <fmtgen/parse.h>

#ifdef DEBUG
#define YYDEBUG 1
extern int yydebug;
#define fprintf lex_debug_fprintf
#endif

%}

%token  BOOLEAN
%token  BOOLEAN_CONSTANT
%token  HIDE_IF_DEFAULT
%token  INCLUDE
%token  INTEGER
%token  INTEGER_CONSTANT
%token  NAME
%token  REAL
%token  REDEFINITION_OK
%token  SHOW_IF_DEFAULT
%token  STRING
%token  STRING_CONSTANT
%token  TIME
%token  TYPE

%union
{
    nstring *lv_string;
    long lv_integer;
    type::pointer *lv_type;
}

%type <lv_string> NAME STRING_CONSTANT
%type <lv_integer> INTEGER_CONSTANT attributes
%type <lv_type> type structure list enumeration enum_list_begin

%{

struct name_ty
{
    name_ty() : parent(0), global_flag(false) { }
    name_ty *parent;
    nstring name_short;
    nstring name_long;
    type::pointer name_type;
    bool global_flag;
    nstring comment;
    nstring position;

    bool
    global()
        const
    {
        trace(("name_short = %s, global() -> %d\n",
            name_short.quote_c().c_str(), global_flag));
        return global_flag;
    }
};

static name_ty *current;
static symtab<type::pointer> typedef_symtab;
static generator::pointer gen;


static bool
check_name_ne(const nstring &name, const nstring &verboten)
{
    if (name == verboten)
    {
    }
    return false;
}


static void
check_name(const nstring &name)
{
    static const char *suffixes[] =
    {
        "_",
        "_get",
        "_is_set",
        "_set",
        "_is_set_flag",
        "_introspector_factory",
    };
    for (const char **spp = suffixes; spp < ENDOF(suffixes); ++spp)
    {
        nstring suffix(*spp);
        if (name.ends_with(suffix))
        {
            yyerror("names may not end with %s", suffix.quote_c().c_str());
            return;
        }
    }

    static const char *avoids[] =
    {
        "clone",
        "create",
        "create_from_file",
        "introspector_factory",
        "list",
        "pointer",
        "report_init",
        "trace_print",
        "write",
        "write_file",
        "write_xml",
    };
    for (const char **app = avoids; app < ENDOF(avoids); ++app)
    {
        nstring avoid(*app);
        if (name == avoid)
        {
            yyerror("may not use %s as a name", name.quote_c().c_str());
            return;
        }
    }
}


static void
push_name(const nstring &s)
{
    trace(("push_name(s = %s)\n{\n", s.quote_c().c_str()));
    name_ty *np = new name_ty;
    np->name_short = s;
    np->name_long = current->name_long + "_" + s;
    np->parent = current;
    np->global_flag = current && current->global_flag;
    np->comment = lex_comment_get();
    np->position = lex_position_get();
    current = np;
    trace(("}\n"));
}


static void
push_name_abs(const nstring &s)
{
    trace(("push_name_abs(s = %s)\n{\n", s.quote_c().c_str()));
    name_ty *np = new name_ty;
    np->name_short = s;
    np->name_long = s;
    np->parent = current;
    np->global_flag = true;
    np->comment = lex_comment_get();
    np->position = lex_position_get();
    current = np;
    trace(("}\n"));
}


static void
pop_name(void)
{
    trace(("pop_name()\n{\n"));
    name_ty *np = current;
    current = np->parent;
    delete np;
    trace(("}\n"));
}


static void
define_type(const type::pointer &defined_type)
{
    trace(("define_type(defined_type = %p)\n{\n", defined_type.get()));
    defined_type->gen_body();
    trace(("}\n"));
}


void
parse(const generator::pointer &g, const nstring &definition_file)
{
    extern int yyparse(void);

    /*
     * initial name is the basename of the definition file
     */
    trace(("parse(def = %s)\n{\n", definition_file.quote_c().c_str()));
#ifdef DEBUG
    yydebug = trace_pretest_;
#endif
    gen = g;
    lex_open(definition_file);
    nstring s = generator::base_name(definition_file);
    check_name(s);
    push_name_abs(s);
    current->global_flag = false;

    /*
     * parse the definition file
     */
    trace(("yyparse()\n{\n"));
    yyparse();
    trace(("}\n"));
    lex_close();

    /*
     * emit a structure containing its fields
     */
    current->name_type->toplevel();
    define_type(current->name_type);

    /*
     * Emit code to read and write files of the top-level type.
     */
    define_type(gen->top_level_factory(current->name_type));

    /*
     * generate the output
     */
    gen->generate_file();

    /*
     * Release all the resources.
     */
    pop_name();
    gen.reset();
    (typedef_symtab).clear();
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
            type::pointer tp = *$4;
            delete $4;

            tp->typedef_set();
            typedef_symtab.assign(current->name_long, tp);
            if (lex_in_include_file())
                tp->in_include_file();
            pop_name();
        }
    | '#' INCLUDE STRING_CONSTANT
        {
            nstring filename = *$3;
            delete $3;
            lex_open(filename);
        }
    | error
    ;

type_name
    : NAME
        {
            nstring name = *$1;
            delete $1;

            check_name(name);
            push_name_abs(name);
        }
    ;

field
    : field_name '=' type attributes ';'
        {
            type::pointer tp = *$3;
            delete $3;
            int attributes = $4;
            current->parent->name_type->member_add
            (
                nstring(current->name_short),
                tp,
                attributes,
                current->comment
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
            nstring name = *$1;
            delete $1;
            check_name(name);
            push_name(name);
        }
    ;

type
    : STRING
        {
            type::pointer tp = gen->type_string_factory();
            define_type(tp);
            $$ = new type::pointer(tp);
        }
    | BOOLEAN
        {
            type::pointer tp = gen->type_boolean_factory();
            define_type(tp);
            $$ = new type::pointer(tp);
        }
    | INTEGER
        {
            type::pointer tp = gen->type_integer_factory();
            define_type(tp);
            $$ = new type::pointer(tp);
        }
    | REAL
        {
            type::pointer tp = gen->type_real_factory();
            define_type(tp);
            $$ = new type::pointer(tp);
        }
    | TIME
        {
            type::pointer tp = gen->type_time_factory();
            define_type(tp);
            $$ = new type::pointer(tp);
        }
    | NAME
        {
            nstring name = *$1;
            delete $1;

            type::pointer tp = typedef_symtab.get(name);
            if (!tp)
            {
                yyerror("type \"%s\" undefined", name.c_str());
                tp = gen->type_integer_factory();
            }
            $$ = new type::pointer(tp);
        }
    | structure
        {
            $$ = $1;
            define_type(*$$);
        }
    | list
        {
            $$ = $1;
            define_type(*$$);
        }
    | enumeration
        {
            $$ = $1;
            define_type(*$$);
        }
    ;

structure
    : '{' field_list '}'
        {
            $$ = new type::pointer(current->name_type);
        }
    ;

field_list
    : /* empty */
        {
            current->name_type =
                gen->type_structure_factory
                (
                    current->name_long,
                    current->global()
                );
            current->name_type->comment_set(current->comment);
        }
    | field_list field
    ;

list
    : '[' type ']'
        {
            type::pointer subtype = *$2;
            delete $2;
            push_name("list");
            type::pointer tp =
                gen->type_list_factory
                (
                    current->name_long,
                    current->global(),
                    subtype
                );
            pop_name();
            $$ = new type::pointer(tp);
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
            type::pointer tp =
                gen->type_enum_factory(current->name_long, current->global());
            current->name_type = tp;
            $$ = new type::pointer(tp);
        }
    ;

enum_list
    : enum_name
    | enum_list ',' enum_name
    ;

enum_name
    : NAME
        {
            nstring name = *$1;
            delete $1;

            push_name(name);
            current->parent->name_type->member_add
            (
                nstring(current->name_short),
                type::pointer(),
                1,
                current->comment
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


/* vim: set ts=8 sw=4 et : */
