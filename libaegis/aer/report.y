/*
 *      aegis - project change supervisor
 *      Copyright (C) 1994-1996, 1999, 2002, 2005-2008 Peter Miller
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program. If not, see
 *      <http://www.gnu.org/licenses/>.
 */

%{

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <libaegis/aer/expr/assign.h>
#include <libaegis/aer/expr/bit.h>
#include <libaegis/aer/expr/comma.h>
#include <libaegis/aer/expr/constant.h>
#include <libaegis/aer/expr/func.h>
#include <libaegis/aer/expr/in.h>
#include <libaegis/aer/expr/incr.h>
#include <libaegis/aer/expr/list.h>
#include <libaegis/aer/expr/logical.h>
#include <libaegis/aer/expr/lookup.h>
#include <libaegis/aer/expr/mul.h>
#include <libaegis/aer/expr/name.h>
#include <libaegis/aer/expr/neg.h>
#include <libaegis/aer/expr/plus.h>
#include <libaegis/aer/expr/power.h>
#include <libaegis/aer/expr/rel.h>
#include <libaegis/aer/expr/shift.h>
#include <libaegis/aer/expr/struct.h>
#include <libaegis/aer/expr/struct_asign.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/report.h>
#include <libaegis/aer/stmt/compound.h>
#include <libaegis/aer/stmt/expr.h>
#include <libaegis/aer/stmt/for.h>
#include <libaegis/aer/stmt/if.h>
#include <libaegis/aer/stmt/null.h>
#include <libaegis/aer/stmt/return.h>
#include <libaegis/aer/stmt/return.h>
#include <libaegis/aer/stmt/throw.h>
#include <libaegis/aer/stmt/throw.h>
#include <libaegis/aer/stmt/try.h>
#include <libaegis/aer/stmt/try.h>
#include <libaegis/aer/stmt/while.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/null.h>
#include <common/error.h>
#include <common/str.h>
#include <libaegis/sub.h>
#include <common/trace.h>

#ifdef DEBUG
#define YYDEBUG 1
#endif

static void
turn_on_debug(void)
{
#ifdef DEBUG
    extern int yydebug;

    yydebug = trace_pretest_;
#endif
}


static rpt_stmt::pointer result;


void
report_interpret(void)
{
    assert(!!result);
    sub_context_ty sc;
    rpt_stmt_result_ty ok;
    result->run(&ok);
    switch (ok.status)
    {
    case rpt_stmt_status_normal:
        break;

    case rpt_stmt_status_break:
        sc.fatal_intl(i18n("floating \"break\" statement"));
        break;

    case rpt_stmt_status_continue:
        sc.fatal_intl(i18n("floating \"continue\" statement"));
        break;

    case rpt_stmt_status_return:
        sc.fatal_intl(i18n("floating \"return\" statement"));
        break;

    case rpt_stmt_status_error:
        {
            rpt_value_error *ep =
                dynamic_cast<rpt_value_error *>(ok.thrown.get());
            assert(ep);
            if (ep)
                ep->print();
            sc.fatal_intl(i18n("report aborted"));
        }
        break;
    }
    result.reset();
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
    long lv_number;
    nstring *lv_string;
    rpt_value::pointer *lv_value;
    rpt_stmt::pointer *lv_stmt;
    rpt_expr::pointer *lv_expr;
}

%type <lv_expr> expr
%type <lv_value> CONSTANT
%type <lv_stmt> stmt stmt_list compound_stmt
%type <lv_string> NAME
%type <lv_expr> expr_list struct_list expr_list_opt struct_assign
%type <lv_stmt> declaration decl_name_list decl_name

/*
 * the precedences, lowest to highest
 */
%right  ELSE
%left   COMMA
%right  ASSIGN ASSIGN_PLUS ASSIGN_MINUS ASSIGN_MUL ASSIGN_DIV ASSIGN_MOD
        ASSIGN_POWER ASSIGN_SHIFT_LEFT ASSIGN_SHIFT_RIGHT ASSIGN_AND_BIT
        ASSIGN_OR_BIT ASSIGN_XOR_BIT ASSIGN_JOIN
%right  QUESTION COLON
%left   OR_LOGICAL
%left   AND_LOGICAL
%left   OR_BIT
%left   XOR_BIT
%left   AND_BIT
%left   EQ NE
%left   LT GT LE GE
%left   SHIFT_LEFT SHIFT_RIGHT
%left   PLUS MINUS JOIN
%left   MUL DIV MOD MATCH NMATCH IN
%right  POWER
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
        {
            result = *$1;
            delete $1;
        }
    ;

stmt_list
    : /* empty */
        {
            rpt_stmt::pointer sp = rpt_stmt_compound::create();
            $$ = new rpt_stmt::pointer(sp);
        }
    | stmt_list stmt
        {
            $$ = $1;
            (*$$)->append(*$2);
            delete $2;
        }
    ;

stmt
    : SEMICOLON
        {
            rpt_stmt::pointer sp = rpt_stmt_null::create();
            $$ = new rpt_stmt::pointer(sp);
        }
    | expr SEMICOLON
        {
            rpt_stmt::pointer sp = rpt_stmt_expr::create(*$1);
            $$ = new rpt_stmt::pointer(sp);
            delete $1;
        }
    | compound_stmt
        { $$ = $1; }
    | FOR LP expr SEMICOLON expr SEMICOLON expr RP stmt
        {
            rpt_stmt::pointer sp = rpt_stmt_for::create(*$3, *$5, *$7, *$9);
            $$ = new rpt_stmt::pointer(sp);
            delete $3;
            delete $5;
            delete $7;
            delete $9;
        }
    | FOR LP expr IN expr RP stmt
        {
            rpt_stmt::pointer sp = rpt_stmt_foreach::create(*$3, *$5, *$7);
            $$ = new rpt_stmt::pointer(sp);
            delete $3;
            delete $5;
            delete $7;
        }
    | declaration
        { $$ = $1; }
    | BREAK SEMICOLON
        {
            rpt_stmt::pointer sp = rpt_stmt_break::create();
            $$ = new rpt_stmt::pointer(sp);
        }
    | CONTINUE SEMICOLON
        {
            rpt_stmt::pointer sp = rpt_stmt_continue::create();
            $$ = new rpt_stmt::pointer(sp);
        }
    | IF LP expr RP stmt
        %prec ELSE
        {
            rpt_stmt::pointer sp = rpt_stmt_if::create(*$3, *$5);
            $$ = new rpt_stmt::pointer(sp);
            delete $3;
            delete $5;
        }
    | IF LP expr RP stmt ELSE stmt
        {
            rpt_stmt::pointer sp = rpt_stmt_if::create(*$3, *$5, *$7);
            $$ = new rpt_stmt::pointer(sp);
            delete $3;
            delete $5;
            delete $7;
        }
    | WHILE LP expr RP stmt
        {
            rpt_stmt::pointer sp = rpt_stmt_while::create(*$3, *$5);
            $$ = new rpt_stmt::pointer(sp);
            delete $3;
            delete $5;
        }
    | DO stmt WHILE LP expr RP SEMICOLON
        {
            rpt_stmt::pointer sp = rpt_stmt_do::create(*$2, *$5);
            $$ = new rpt_stmt::pointer(sp);
            delete $2;
            delete $5;
        }
    | RETURN expr SEMICOLON
        {
            rpt_stmt::pointer sp = rpt_stmt_return::create(*$2);
            $$ = new rpt_stmt::pointer(sp);
            delete $2;
        }
    | RETURN SEMICOLON
        {
            rpt_stmt::pointer sp = rpt_stmt_return::create();
            $$ = new rpt_stmt::pointer(sp);
        }
    | THROW expr SEMICOLON
        {
            rpt_stmt::pointer sp = rpt_stmt_throw::create(*$2);
            $$ = new rpt_stmt::pointer(sp);
            delete $2;
        }
    | TRY stmt CATCH LP expr RP stmt
        {
            rpt_stmt::pointer sp = rpt_stmt_try::create(*$2, *$5, *$7);
            $$ = new rpt_stmt::pointer(sp);
            delete $2;
            delete $5;
            delete $7;
        }
    | error
        {
            rpt_stmt::pointer sp = rpt_stmt_null::create();
            $$ = new rpt_stmt::pointer(sp);
        }
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
            rpt_stmt::pointer sp = rpt_stmt_compound::create();
            sp->append(*$1);
            delete $1;
            $$ = new rpt_stmt::pointer(sp);
        }
    | decl_name_list COMMA decl_name
        {
            $$ = $1;
            (*$$)->append(*$3);
            delete $3;
        }
    ;

decl_name
    : NAME
        {
            /*
             * declare the name
             * and initialize it to nul
             */
            rpt_expr_name__declare(*$1);
            rpt_expr::pointer e1 = rpt_expr_name(*$1);
            e1->pos_from_lex();
            delete $1;

            rpt_value::pointer vp = rpt_value_null::create();
            rpt_expr::pointer e2 = rpt_expr_constant::create(vp);
            e2->pos_from_lex();

            rpt_expr::pointer e3 = rpt_expr_assign::create(e1, e2);

            rpt_stmt::pointer sp = rpt_stmt_expr::create(e3);
            $$ = new rpt_stmt::pointer(sp);
        }
    ;

expr
    : CONSTANT
        {
            rpt_expr::pointer ep = rpt_expr_constant::create(*$1);
            delete $1;
            ep->pos_from_lex();
            $$ = new rpt_expr::pointer(ep);
        }
    | NAME
        {
            rpt_expr::pointer ep = rpt_expr_name(*$1);
            ep->pos_from_lex();
            delete $1;
            $$ = new rpt_expr::pointer(ep);
        }
    | LP expr RP
        { $$ = $2; }
    | expr LP expr_list_opt RP
        {
            rpt_expr::pointer ep = rpt_expr_func::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | LB struct_list RB
        {
            $$ = $2;
        }
    | LBB expr_list_opt RBB
        { $$ = $2; }
    | expr LBB expr RBB
        {
            rpt_expr::pointer ep = rpt_expr_lookup::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr DOT NAME
        {
            rpt_expr::pointer ep = rpt_expr_lookup::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | MINUS expr
        %prec unary.prec
        {
            rpt_expr::pointer ep = rpt_expr_neg::create(*$2);
            delete $2;
            $$ = new rpt_expr::pointer(ep);
        }
    | PLUS expr
        %prec unary.prec
        {
            rpt_expr::pointer ep = rpt_expr_pos::create(*$2);
            delete $2;
            $$ = new rpt_expr::pointer(ep);
        }
    | NOT_LOGICAL expr
        {
            rpt_expr::pointer ep = rpt_expr_not_logical::create(*$2);
            delete $2;
            $$ = new rpt_expr::pointer(ep);
        }
    | NOT_BIT expr
        {
            rpt_expr::pointer ep = rpt_expr_not_bit::create(*$2);
            delete $2;
            $$ = new rpt_expr::pointer(ep);
        }
    | INCR expr
        {
            rpt_expr::pointer ep = rpt_expr_inc_pre::create(*$2);
            delete $2;
            $$ = new rpt_expr::pointer(ep);
        }
    | DECR expr
        {
            rpt_expr::pointer ep = rpt_expr_dec_pre::create(*$2);
            delete $2;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr INCR
        {
            rpt_expr::pointer ep = rpt_expr_inc_post::create(*$1);
            delete $1;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr DECR
        {
            rpt_expr::pointer ep = rpt_expr_dec_post::create(*$1);
            delete $1;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr POWER expr
        {
            rpt_expr::pointer ep = rpt_expr_power::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr MUL expr
        {
            rpt_expr::pointer ep = rpt_expr_mul::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr DIV expr
        {
            rpt_expr::pointer ep = rpt_expr_div::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr MOD expr
        {
            rpt_expr::pointer ep = rpt_expr_mod::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr SHIFT_LEFT expr
        {
            rpt_expr::pointer ep = rpt_expr_shift_left::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr SHIFT_RIGHT expr
        {
            rpt_expr::pointer ep = rpt_expr_shift_right::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr PLUS expr
        {
            rpt_expr::pointer ep = rpt_expr_plus::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr MINUS expr
        {
            rpt_expr::pointer ep = rpt_expr_minus::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr JOIN expr
        {
            rpt_expr::pointer ep = rpt_expr_join::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr LT expr
        {
            rpt_expr::pointer ep = rpt_expr_lt::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr LE expr
        {
            rpt_expr::pointer ep = rpt_expr_le::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr GT expr
        {
            rpt_expr::pointer ep = rpt_expr_gt::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr GE expr
        {
            rpt_expr::pointer ep = rpt_expr_ge::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr EQ expr
        {
            rpt_expr::pointer ep = rpt_expr_eq::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr NE expr
        {
            rpt_expr::pointer ep = rpt_expr_ne::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr IN expr
        {
            rpt_expr::pointer ep = rpt_expr_in::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr MATCH expr
        {
            rpt_expr::pointer ep = rpt_expr_match::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr NMATCH expr
        {
            rpt_expr::pointer ep = rpt_expr_nmatch::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr AND_BIT expr
        {
            rpt_expr::pointer ep = rpt_expr_and_bit::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr AND_LOGICAL expr
        {
            rpt_expr::pointer ep = rpt_expr_and_logical::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr OR_BIT expr
        {
            rpt_expr::pointer ep = rpt_expr_or_bit::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr XOR_BIT expr
        {
            rpt_expr::pointer ep = rpt_expr_xor_bit::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr OR_LOGICAL expr
        {
            rpt_expr::pointer ep = rpt_expr_or_logical::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN expr
        {
            rpt_expr::pointer ep = rpt_expr_assign::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_PLUS expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_plus::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_MINUS expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_minus::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_MUL expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_mul::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_DIV expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_div::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_MOD expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_mod::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_POWER expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_power::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_JOIN expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_join::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_AND_BIT expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_and_bit::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_OR_BIT expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_or_bit::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_XOR_BIT expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_xor_bit::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_SHIFT_LEFT expr
        {
            rpt_expr::pointer ep = rpt_expr_assign_shift_left::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr ASSIGN_SHIFT_RIGHT expr
        {
            rpt_expr::pointer ep =
                rpt_expr_assign_shift_right::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr COMMA expr
        {
            rpt_expr::pointer ep = rpt_expr_comma::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr QUESTION expr COLON expr
        {
            rpt_expr::pointer ep = rpt_expr_if::create(*$1, *$3, *$5);
            delete $1;
            delete $3;
            delete $5;
            $$ = new rpt_expr::pointer(ep);
        }
    ;

expr_list_opt
    : /* empty */
        {
            rpt_expr::pointer ep = rpt_expr_list::create();
            ep->pos_from_lex();
            $$ = new rpt_expr::pointer(ep);
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
            rpt_expr::pointer ep = rpt_expr_list::create();
            ep->append(*$1);
            delete $1;
            $$ = new rpt_expr::pointer(ep);
        }
    | expr_list COMMA expr
        {
            $$ = $1;
            (*$$)->append(*$3);
            delete $3;
        }
    ;

struct_list
    : /* empty */
        {
            rpt_expr::pointer ep = rpt_expr_struct::create();
            ep->pos_from_lex();
            $$ = new rpt_expr::pointer(ep);
        }
    | struct_list struct_assign
        {
            $$ = $1;
            (*$$)->append(*$2);
            delete $2;
        }
    ;

struct_assign
    : NAME ASSIGN expr SEMICOLON
        {
            rpt_expr::pointer ep = rpt_expr_struct_assign::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new rpt_expr::pointer(ep);
        }
    ;
