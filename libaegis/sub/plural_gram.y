/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate plural_gram.ys
 */

%{

#include <ac/stdarg.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <str.h>
#include <sub.h>
#include <sub/plural_gram.h>
#include <sub/plural_lex.h>
#include <trace.h>

#ifdef DEBUG
#define YYDEBUG 1
#endif

%}

%token ANDAND
%token BITAND
%token BITOR
%token BITXOR
%token COLON
%token DIV
%token EQ
%token EQU
%token GE
%token GT
%token INTEGER
%token JUNK
%token LE
%token LP
%token LT
%token MINUS
%token MOD
%token MUL
%token NE
%token NOT
%token NPLURALS
%token NUMBER
%token OROR
%token PLURAL
%token PLUS
%token QUEST
%token RP
%token SEMI

%union
{
    unsigned        lv_number;
}

%type <lv_number> INTEGER expr

%right EQU
%right QUEST COLON
%left OROR
%left ANDAND
%left BITOR
%left BITXOR
%left BITAND
%left NE EQ
%left LT LE GT GE
%left PLUS MINUS
%left MUL DIV MOD
%right UNARY NOT BITNOT

%{

static unsigned number;
static unsigned nplurals;
static unsigned plural;


unsigned
sub_plural_gram(string_ty *s, unsigned n)
{
    int		bad;
    extern int yyparse(void);
#ifdef DEBUG
    extern int  yydebug;
#endif

    trace(("sub_plural_gram()\n{\n"));
    number = n;
#ifdef DEBUG
    yydebug = trace_pretest_;
#endif
    sub_plural_lex_open(s);
    bad = yyparse();
    sub_plural_lex_close();
    trace(("bad = %d\n", bad));

    trace(("}\n"));
    if (bad)
	return 0;
    if (plural >= nplurals)
	return 0;
    return plural;
}


void
yyerror(const char *s)
{
    /* do nothing */
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
 *	the first argument, it will be ``stderr''.  The trace_where_ is
 *	needed to set the location, and is safe, because bison only
 *	invokes the printf with an if (be careful, the fprintf is not in
 *	a compound statement).
 */
#define fprintf trace_where_, yydebugger

void
yydebugger(void *junk, const char *fmt, ...)
{
	va_list		ap;
	string_ty	*s;

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
	nplurals SEMI plural
	;

nplurals
	: NPLURALS EQU INTEGER
	    {
		nplurals = $3;
	    }
	;

plural
	: PLURAL EQU expr
	    {
		plural = $3;
	    }
	;

expr
	: LP expr RP
	    {
		$$ = $2;
		trace(("$$ = %ld;\n", $$));
	    }
	| NUMBER
	    {
		$$ = number;
		trace(("$$ = %ld;\n", $$));
	    }
	| INTEGER
	    {
		$$ = $1;
		trace(("$$ = %ld;\n", $$));
	    }
	| MINUS expr
	    %prec UNARY
	    {
		$$ = -$2;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr PLUS expr
	    {
		$$ = $1 + $3;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr MINUS expr
	    {
		$$ = $1 - $3;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr MUL expr
	    {
		$$ = $1 * $3;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr DIV expr
	    {
		$$ = $3 ? $1 / $3 : 0;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr MOD expr
	    {
		$$ = $3 ? $1 % $3 : 0;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr EQ expr
	    {
		$$ = ($1 == $3);
		trace(("$$ = %ld;\n", $$));
	    }
	| expr NE expr
	    {
		$$ = ($1 != $3);
		trace(("$$ = %ld;\n", $$));
	    }
	| expr LT expr
	    {
		$$ = ($1 < $3);
		trace(("$$ = %ld;\n", $$));
	    }
	| expr LE expr
	    {
		$$ = ($1 <= $3);
		trace(("$$ = %ld;\n", $$));
	    }
	| expr GT expr
	    {
		$$ = ($1 > $3);
		trace(("$$ = %ld;\n", $$));
    	    }
	| expr GE expr
	    {
		$$ = ($1 >= $3);
		trace(("$$ = %ld;\n", $$));
	    }
	| NOT expr
	    {
		$$ = (!$2);
		trace(("$$ = %ld;\n", $$));
	    }
	| BITNOT expr
	    {
		$$ = ~$2;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr ANDAND expr
	    {
		$$ = $1 && $3;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr BITAND expr
	    {
		$$ = $1 & $3;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr OROR expr
	    {
		$$ = $1 || $3;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr BITOR expr
	    {
		$$ = $1 | $3;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr BITXOR expr
	    {
		$$ = $1 ^ $3;
		trace(("$$ = %ld;\n", $$));
	    }
	| expr QUEST expr COLON expr
	    {
		$$ = $1 ? $3 : $5;
		trace(("$$ = %ld;\n", $$));
	    }
	;