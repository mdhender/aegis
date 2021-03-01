/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1999 Peter Miller;
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
 * MANIFEST: functions to parse report grammar
 */

%{

#include <ac/stdio.h>

#include <aer/expr/assign.h>
#include <aer/expr/bit.h>
#include <aer/expr/constant.h>
#include <aer/expr/func.h>
#include <aer/expr/in.h>
#include <aer/expr/incr.h>
#include <aer/expr/list.h>
#include <aer/expr/logical.h>
#include <aer/expr/lookup.h>
#include <aer/expr/mul.h>
#include <aer/expr/name.h>
#include <aer/expr/neg.h>
#include <aer/expr/plus.h>
#include <aer/expr/power.h>
#include <aer/expr/rel.h>
#include <aer/expr/shift.h>
#include <aer/expr/struct.h>
#include <aer/expr/struct_asign.h>
#include <aer/lex.h>
#include <aer/report.h>
#include <aer/stmt/compound.h>
#include <aer/stmt/expr.h>
#include <aer/stmt/for.h>
#include <aer/stmt/if.h>
#include <aer/stmt/null.h>
#include <aer/stmt/return.h>
#include <aer/stmt/return.h>
#include <aer/stmt/throw.h>
#include <aer/stmt/throw.h>
#include <aer/stmt/try.h>
#include <aer/stmt/try.h>
#include <aer/stmt/while.h>
#include <aer/value/error.h>
#include <aer/value/null.h>
#include <error.h>
#include <str.h>
#include <sub.h>
#include <trace.h>

#ifdef DEBUG
#define YYDEBUG 1
#endif

static void turn_on_debug _((void));

static void
turn_on_debug()
{
#ifdef DEBUG
	extern int yydebug;

	yydebug = trace_pretest_;
#endif
}


static rpt_stmt_ty *result;


void
report_interpret()
{
	sub_context_ty	*scp;
	rpt_stmt_result_ty ok;

	rpt_stmt_run(result, &ok);
	switch (ok.status)
	{
	case rpt_stmt_status_normal:
		break;

	case rpt_stmt_status_break:
		scp = sub_context_new();
		fatal_intl(scp, i18n("floating ``break'' statement"));
		sub_context_delete(scp);
		break;

	case rpt_stmt_status_continue:
		scp = sub_context_new();
		fatal_intl(scp, i18n("floating ``continue'' statement"));
		sub_context_delete(scp);
		break;

	case rpt_stmt_status_return:
		scp = sub_context_new();
		fatal_intl(scp, i18n("floating ``return'' statement"));
		sub_context_delete(scp);
		rpt_value_free(ok.thrown);
		break;

	case rpt_stmt_status_error:
		assert(ok.thrown->method->type == rpt_value_type_error);
		rpt_value_error_print(ok.thrown);
		scp = sub_context_new();
		fatal_intl(scp, i18n("report aborted"));
		sub_context_delete(scp);
		rpt_value_free(ok.thrown);
		break;
	}
	rpt_stmt_free(result);
}

%}

%token AND_BIT
%token AND_LOGICAL
%token ASSIGN
%token ASSIGN_AND_BIT
%token ASSIGN_DIV
%token ASSIGN_JOIN
%token ASSIGN_MINUS
%token ASSIGN_MOD
%token ASSIGN_MUL
%token ASSIGN_OR_BIT
%token ASSIGN_PLUS
%token ASSIGN_POWER
%token ASSIGN_SHIFT_LEFT
%token ASSIGN_SHIFT_RIGHT
%token ASSIGN_XOR_BIT
%token AUTO
%token BREAK
%token CASE
%token CATCH
%token COLON
%token COMMA
%token CONSTANT
%token CONTINUE
%token DECR
%token DEFAULT
%token DIV
%token DO
%token DOT
%token ELSE
%token EQ
%token FOR
%token FUNCTION
%token GE
%token GLOBAL
%token GOTO
%token GT
%token IF
%token IN
%token INCR
%token JOIN
%token JUNK
%token LB
%token LBB
%token LE
%token LP
%token LT
%token MATCH
%token MINUS
%token MOD
%token MUL
%token NAME
%token NE
%token NMATCH
%token NOT_BIT
%token NOT_LOGICAL
%token OR_BIT
%token OR_LOGICAL
%token PLUS
%token POWER
%token RB
%token RBB
%token RETURN
%token RP
%token SEMICOLON
%token SHIFT_LEFT
%token SHIFT_RIGHT
%token SWITCH
%token THROW
%token TRY
%token WHILE
%token XOR_BIT

%union
{
	long		lv_number;
	struct string_ty *lv_string;
	struct rpt_value_ty *lv_value;
	struct rpt_stmt_ty *lv_stmt;
	struct rpt_expr_ty *lv_expr;
}

%type <lv_expr>	expr
%type <lv_value> CONSTANT
%type <lv_stmt> stmt stmt_list compound_stmt
%type <lv_string> NAME
%type <lv_expr> expr_list struct_list expr_list_opt struct_assign
%type <lv_stmt> declaration decl_name_list decl_name

/*
 * the precedences, lowest to highest
 */
%right	ELSE
%left   COMMA
%right  ASSIGN ASSIGN_PLUS ASSIGN_MINUS ASSIGN_MUL ASSIGN_DIV ASSIGN_MOD ASSIGN_POWER ASSIGN_SHIFT_LEFT ASSIGN_SHIFT_RIGHT ASSIGN_AND_BIT ASSIGN_OR_BIT ASSIGN_XOR_BIT ASSIGN_JOIN
%right  QUESTION COLON
%left   OR_LOGICAL
%left   AND_LOGICAL
%left   OR_BIT
%left   XOR_BIT
%left	AND_BIT
%left   EQ NE
%left   LT GT LE GE
%left   SHIFT_LEFT SHIFT_RIGHT
%left   PLUS MINUS JOIN
%left   MUL DIV MOD MATCH NMATCH IN
%right	POWER
%right  INCR DECR NOT_LOGICAL NOT_BIT unary.prec
%left   LP RP LBB RBB LB RB DOT

%%

report_debugged
	: turn_on_debug report
	;

turn_on_debug
	: /* empty */
		{ turn_on_debug(); }
	;

report
	: stmt_list
		{ result = $1; }
	;

stmt_list
	: /* empty */
		{ $$ = rpt_stmt_compound(); }
	| stmt_list stmt
		{
			$$ = $1;
			rpt_stmt_append($$, $2);
			rpt_stmt_free($2);
		}
	;

stmt
	: SEMICOLON
		{ $$ = rpt_stmt_null(); }
	| expr SEMICOLON
		{
			$$ = rpt_stmt_expr($1);
			rpt_expr_free($1);
		}
	| compound_stmt
		{ $$ = $1; }
	| FOR LP expr SEMICOLON expr SEMICOLON expr RP stmt
		{
			$$ = rpt_stmt_for($3, $5, $7, $9);
			rpt_expr_free($3);
			rpt_expr_free($5);
			rpt_expr_free($7);
			rpt_stmt_free($9);
		}
	| FOR LP expr IN expr RP stmt
		{
			$$ = rpt_stmt_foreach($3, $5, $7);
			rpt_expr_free($3);
			rpt_expr_free($5);
			rpt_stmt_free($7);
		}
	| declaration
		{ $$ = $1; }
	| BREAK SEMICOLON
		{ $$ = rpt_stmt_break(); }
	| CONTINUE SEMICOLON
		{ $$ = rpt_stmt_continue(); }
	| IF LP expr RP stmt
		%prec ELSE
		{
			$$ = rpt_stmt_if($3);
			rpt_expr_free($3);
			rpt_stmt_append($$, $5);
			rpt_stmt_free($5);
		}
	| IF LP expr RP stmt ELSE stmt
		{
			$$ = rpt_stmt_if($3);
			rpt_expr_free($3);
			rpt_stmt_append($$, $5);
			rpt_stmt_free($5);
			rpt_stmt_append($$, $7);
			rpt_stmt_free($7);
		}
	| WHILE LP expr RP stmt
		{
			$$ = rpt_stmt_while($3, $5);
			rpt_expr_free($3);
			rpt_stmt_free($5);
		}
	| DO stmt WHILE LP expr RP SEMICOLON
		{
			$$ = rpt_stmt_do($5, $2);
			rpt_stmt_free($2);
			rpt_expr_free($5);
		}
	| RETURN expr SEMICOLON
		{ $$ = rpt_stmt_return($2); }
	| RETURN SEMICOLON
		{ $$ = rpt_stmt_return((rpt_expr_ty *)0); }
	| THROW expr SEMICOLON
		{ $$ = rpt_stmt_throw($2); }
	| TRY stmt CATCH LP expr RP stmt
		{ $$ = rpt_stmt_try($2, $5, $7); }
	| error
		{ $$ = rpt_stmt_null(); }
	;

compound_stmt
	: LB stmt_list RB
		{ $$ = $2; }
	;

declaration
	: AUTO decl_name_list SEMICOLON
		{ $$ = $2; }
	;

decl_name_list
	: decl_name
		{
			$$ = rpt_stmt_compound();
			rpt_stmt_append($$, $1);
		}
	| decl_name_list COMMA decl_name
		{
			$$ = $1;
			rpt_stmt_append($$, $3);
		}
	;

decl_name
	: NAME
		{
			rpt_expr_ty	*e1;
			rpt_expr_ty	*e2;
			rpt_expr_ty	*e3;
			rpt_value_ty	*vp;

			/*
			 * declare the name
			 * and initialize it to nul
			 */
			rpt_expr_name__declare($1);
			e1 = rpt_expr_name($1);
			e1->pos = rpt_lex_pos_get();
			str_free($1);

			vp = rpt_value_nul();
			e2 = rpt_expr_constant(vp);
			e2->pos = rpt_lex_pos_get();
			rpt_value_free(vp);

			e3 = rpt_expr_assign(e1, e2);
			rpt_expr_free(e1);
			rpt_expr_free(e2);

			$$ = rpt_stmt_expr(e3);
			rpt_expr_free(e3);
		}
	;

expr
	: CONSTANT
		{
			$$ = rpt_expr_constant($1);
			$$->pos = rpt_lex_pos_get();
			rpt_value_free($1);
		}
	| NAME
		{
			$$ = rpt_expr_name($1);
			$$->pos = rpt_lex_pos_get();
			str_free($1);
		}
	| LP expr RP
		{ $$ = $2; }
	| expr LP expr_list_opt RP
		{
			$$ = rpt_expr_func($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| LB struct_list RB
		{
			$$ = $2;
		}
	| LBB expr_list_opt RBB
		{ $$ = $2; }
	| expr LBB expr RBB
		{
			$$ = rpt_expr_lookup($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr DOT NAME
		{
			$$ = rpt_expr_dot($1, $3);
			rpt_expr_free($1);
			str_free($3);
		}
	| MINUS expr
		%prec unary.prec
		{
			$$ = rpt_expr_neg($2);
			rpt_expr_free($2);
		}
	| PLUS expr
		%prec unary.prec
		{
			$$ = rpt_expr_pos($2);
			rpt_expr_free($2);
		}
	| NOT_LOGICAL expr
		{
			$$ = rpt_expr_not_logical($2);
			rpt_expr_free($2);
		}
	| NOT_BIT expr
		{
			$$ = rpt_expr_not_bit($2);
			rpt_expr_free($2);
		}
	| INCR expr
		{
			$$ = rpt_expr_inc_pre($2);
			rpt_expr_free($2);
		}
	| DECR expr
		{
			$$ = rpt_expr_dec_pre($2);
			rpt_expr_free($2);
		}
	| expr INCR
		{
			$$ = rpt_expr_inc_post($1);
			rpt_expr_free($1);
		}
	| expr DECR
		{
			$$ = rpt_expr_dec_post($1);
			rpt_expr_free($1);
		}
	| expr POWER expr
		{
			$$ = rpt_expr_power($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr MUL expr
		{
			$$ = rpt_expr_mul($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr DIV expr
		{
			$$ = rpt_expr_div($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr MOD expr
		{
			$$ = rpt_expr_mod($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr SHIFT_LEFT expr
		{
			$$ = rpt_expr_shift_left($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr SHIFT_RIGHT expr
		{
			$$ = rpt_expr_shift_right($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr PLUS expr
		{
			$$ = rpt_expr_plus($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr MINUS expr
		{
			$$ = rpt_expr_minus($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr JOIN expr
		{
			$$ = rpt_expr_join($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr LT expr
		{
			$$ = rpt_expr_lt($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr LE expr
		{
			$$ = rpt_expr_le($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr GT expr
		{
			$$ = rpt_expr_gt($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr GE expr
		{
			$$ = rpt_expr_ge($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr EQ expr
		{
			$$ = rpt_expr_eq($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr NE expr
		{
			$$ = rpt_expr_ne($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr IN expr
		{
			$$ = rpt_expr_in($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr MATCH expr
		{
			$$ = rpt_expr_match($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr NMATCH expr
		{
			$$ = rpt_expr_nmatch($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr AND_BIT expr
		{
			$$ = rpt_expr_and_bit($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr AND_LOGICAL expr
		{
			$$ = rpt_expr_and_logical($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr OR_BIT expr
		{
			$$ = rpt_expr_or_bit($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr XOR_BIT expr
		{
			$$ = rpt_expr_xor_bit($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr OR_LOGICAL expr
		{
			$$ = rpt_expr_or_logical($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN expr
		{
			$$ = rpt_expr_assign($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_PLUS expr
		{
			$$ = rpt_expr_assign_plus($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_MINUS expr
		{
			$$ = rpt_expr_assign_minus($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_MUL expr
		{
			$$ = rpt_expr_assign_mul($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_DIV expr
		{
			$$ = rpt_expr_assign_div($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_MOD expr
		{
			$$ = rpt_expr_assign_mod($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_POWER expr
		{
			$$ = rpt_expr_assign_power($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_JOIN expr
		{
			$$ = rpt_expr_assign_join($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_AND_BIT expr
		{
			$$ = rpt_expr_assign_and_bit($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_OR_BIT expr
		{
			$$ = rpt_expr_assign_or_bit($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_XOR_BIT expr
		{
			$$ = rpt_expr_assign_xor_bit($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_SHIFT_LEFT expr
		{
			$$ = rpt_expr_assign_shift_left($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr ASSIGN_SHIFT_RIGHT expr
		{
			$$ = rpt_expr_assign_shift_right($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr COMMA expr
		{
			$$ = rpt_expr_comma($1, $3);
			rpt_expr_free($1);
			rpt_expr_free($3);
		}
	| expr QUESTION expr COLON expr
		{
			$$ = rpt_expr_if($1, $3, $5);
			rpt_expr_free($1);
			rpt_expr_free($3);
			rpt_expr_free($5);
		}
	;

expr_list_opt
	: /* empty */
		{
			$$ = rpt_expr_list();
			$$->pos = rpt_lex_pos_get();
		}
	| expr_list
		{
			$$ = $1;
		}
	;

expr_list
	: expr
		%prec COMMA
		{
			$$ = rpt_expr_list();
			rpt_expr_append($$, $1);
		}
	| expr_list COMMA expr
		{
			$$ = $1;
			rpt_expr_append($$, $3);
		}
	;

struct_list
	: /* empty */
		{
			$$ = rpt_expr_struct();
			$$->pos = rpt_lex_pos_get();
		}
	| struct_list struct_assign
		{
			$$ = $1;
			rpt_expr_append($$, $2);
			rpt_expr_free($2);
		}
	;

struct_assign
	: NAME ASSIGN expr SEMICOLON
		{
			$$ = rpt_expr_struct_assign($1, $3);
			$$->pos = rpt_pos_copy($3->pos);
			str_free($1);
			rpt_expr_free($3);
		}
	;
