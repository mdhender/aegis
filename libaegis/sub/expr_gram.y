/*
 *	aegis - project change supervisor
 *	Copyright (C) 1996, 1999, 2002, 2003, 2005-2008 Peter Miller
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
 *	along with this program. If not, see
 *	<http://www.gnu.org/licenses/>.
 */

%{

#include <common/ac/stdarg.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/str.h>
#include <libaegis/sub.h>
#include <libaegis/sub/expr_gram.h>
#include <libaegis/sub/expr_lex.h>
#include <common/trace.h>

#ifdef DEBUG
#define YYDEBUG 1
#endif

static void yyerror(const char *s);
static void yydebugger(void *, const char *, ...);

%}

%token DIV
%token EQ
%token GE
%token GT
%token JUNK
%token LE
%token LP
%token LT
%token MINUS
%token MOD
%token MUL
%token NE
%token NOT
%token NUMBER
%token PLUS
%token RP

%union
{
	long	lv_number;
}

%type <lv_number> NUMBER expr

%left NE EQ
%left LT LE GT GE
%left PLUS MINUS
%left MUL DIV MOD
%right UNARY

%{

static long result;
static sub_context_ty *scp;


nstring
sub_expr_gram(sub_context_ty *scp_arg, const nstring &s)
{
    extern int yyparse(void);
#ifdef DEBUG
    extern int      yydebug;
#endif

    trace(("sub_expr_gram()\n{\n"));
#ifdef DEBUG
    yydebug = trace_pretest_;
#endif
    scp = scp_arg;
    sub_expr_lex_open(s);
    int bad = yyparse();
    sub_expr_lex_close();
    trace(("bad = %d\n", bad));

    trace(("}\n"));
    if (bad)
	return "";
    return nstring::format("%ld", result);
}


static void
yyerror(const char *s)
{
    trace(("yyerror(\"%s\")\n{\n", s));
    scp->error_set(s);
    trace(("}\n"));
}

#ifdef DEBUG

/*
 * jiggery-pokery for yacc
 *
 *	Replace all calls to printf with a call to trace_printf.  The
 *	trace_where_ is needed to set the location, and is safe, because
 *	yacc only invokes the printf with an if (be careful, the printf
 *	is not in a compound statement).
 */
#define printf trace_where_, trace_printf

/*
 * jiggery-pokery for bison
 *
 *	Replace all calls to fprintf with a call to yydebugger.  Ignore
 *	the first argument, it will be "stderr".  The trace_where_ is
 *	needed to set the location, and is safe, because bison only
 *	invokes the printf with an if (be careful, the fprintf is not in
 *	a compound statement).
 */
#define fprintf trace_where_, yydebugger

static void
yydebugger(void *, const char *fmt, ...)
{
    va_list         ap;
    string_ty       *s;

    va_start(ap, fmt);
    s = str_vformat(fmt, ap);
    va_end(ap);
    trace_printf("%s", s->str_text);
    str_free(s);
}

#endif

%}

%%

grammar:
    expr
	{ result = $1; }
    ;

expr
    : LP expr RP
	{ $$ = $2; trace(("$$ = %ld;\n", $$)); }
    | NUMBER
	{ $$ = $1; trace(("$$ = %ld;\n", $$)); }
    | MINUS expr
	%prec UNARY
	{ $$ = -$2; trace(("$$ = %ld;\n", $$)); }
    | expr PLUS expr
	{ $$ = $1 + $3; trace(("$$ = %ld;\n", $$)); }
    | expr MINUS expr
	{ $$ = $1 - $3; trace(("$$ = %ld;\n", $$)); }
    | expr MUL expr
	{ $$ = $1 * $3; trace(("$$ = %ld;\n", $$)); }
    | expr DIV expr
	{ $$ = $3 ? $1 / $3 : 0; trace(("$$ = %ld;\n", $$)); }
    | expr MOD expr
	{ $$ = $3 ? $1 % $3 : 0; trace(("$$ = %ld;\n", $$)); }
    | expr EQ expr
	{ $$ = ($1 == $3); trace(("$$ = %ld;\n", $$)); }
    | expr NE expr
	{ $$ = ($1 != $3); trace(("$$ = %ld;\n", $$)); }
    | expr LT expr
	{ $$ = ($1 < $3); trace(("$$ = %ld;\n", $$)); }
    | expr LE expr
	{ $$ = ($1 <= $3); trace(("$$ = %ld;\n", $$)); }
    | expr GT expr
	{ $$ = ($1 > $3); trace(("$$ = %ld;\n", $$)); }
    | expr GE expr
	{ $$ = ($1 >= $3); trace(("$$ = %ld;\n", $$)); }
    | NOT expr
	%prec UNARY
	{ $$ = (!$2); trace(("$$ = %ld;\n", $$)); }
    ;
