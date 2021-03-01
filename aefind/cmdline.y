/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to parse the command line
 */

%{

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <aer/value/boolean.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <arglex2.h>
#include <change.h>
#include <change/file.h>
#include <cmdline.h>
#include <descend.h>
#include <error.h>
#include <function.h>
#include <function/execute.h>
#include <gonzo.h>
#include <help.h>
#include <lex.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <shorthand/name.h>
#include <shorthand/path.h>
#include <shorthand/print.h>
#include <shorthand/stat.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <tree.h>
#include <tree/arithmetic.h>
#include <tree/bitwise.h>
#include <tree/constant.h>
#include <tree/list.h>
#include <tree/logical.h>
#include <tree/match.h>
#include <tree/now.h>
#include <tree/relative.h>
#include <tree/this.h>
#include <user.h>

#ifdef DEBUG
#define YYDEBUG 1
#endif

%}

%token AMIN
%token ANDAND
%token ATIME
%token BASELINE
%token BASE_REL
%token BIT_AND
%token BIT_OR
%token BIT_XOR
%token BRANCH
%token CHANGE
%token CMIN
%token COLON
%token COMMA
%token CTIME
%token CUR_REL
%token DEBUG_keyword
%token DEVDIR
%token DIV
%token EQ
%token EXECUTE
%token FALSE_keyword
%token GE
%token GRANDPARENT
%token GT
%token HELP
%token JOIN
%token JUNK
%token LE
%token LIBRARY
%token LPAREN
%token LT
%token MINUS
%token MMIN
%token MOD
%token MTIME
%token MUL
%token NAME
%token NE
%token NEWER
%token NOT
%token NOW
%token NUMBER
%token OROR
%token PATH
%token PERM
%token PLUS
%token PRINT
%token PROJECT
%token QUESTION
%token REAL
%token RESOLVE
%token RESOLVE_NOT
%token RPAREN
%token SEMICOLON
%token SHIFT_LEFT
%token SHIFT_RIGHT
%token SSIZE
%token STRING
%token STRINGIZE
%token THIS
%token TILDE
%token TRACE
%token TRUE_keyword
%token TRUNK
%token TYPE

%union
{
    struct string_ty *lv_string;
    struct string_list_ty *lv_string_list;
    struct tree_ty  *lv_tree;
    struct tree_list_ty *lv_tree_list;
    long	    lv_number;
    double	    lv_real;
    struct tree_ty  *(*comparator)_((struct tree_ty *, struct tree_ty *));
}

%type <comparator>  comparator
%type <lv_number>   NUMBER
%type <lv_real>	    REAL
%type <lv_string>   STRING
%type <lv_string>   number_or_string
%type <lv_string_list> strings
%type <lv_tree>	    tree1 tree2 tree3 tree4 tree5 tree6 tree7
%type <lv_tree>	    tree8 tree9 tree10 tree11 tree12 tree13 tree14
%type <lv_tree_list> list
%type <lv_tree_list> list_opt
%type <lv_tree>     exec_list

%left COMMA
%left QUESTION COLON
%left OROR
%left ANDAND
%left BIT_OR
%left BIT_XOR
%left BIT_AND
%left LT LE GT GE
%left SHIFT_LEFT SHIFT_RIGHT
%left PLUS MINUS JOIN
%left MUL DIV MOD TILDE
%right NOT unary
%right LPAREN RPAREN

%{

static int	number_of_ops;
static string_list_ty *path;
static tree_ty	*tp;
static string_ty *project_name;
static long	change_number;
static int	grandparent;
static string_ty *branch;
static int	trunk;
static int	baseline;
static int	resolve;
static int	debug;


static void
report_error(rpt_value_ty *vp)
{
    sub_context_ty  *scp;

    if (vp->method->type != rpt_value_type_error)
       	return;
    scp = sub_context_new();
    sub_var_set_string(scp, "MeSsaGe", rpt_value_error_query(vp));
    fatal_intl(scp, i18n("$message"));
    /* NOTREACHED */
}


static void
walker(void *p, descend_message_ty msg, string_ty *pathname, struct stat *st)
{
    rpt_value_ty    *vp;

    switch (msg)
    {
    case descend_message_file:
    case descend_message_dir_before:
       	vp = tree_evaluate(tp, pathname, st);
       	if (vp->method->type == rpt_value_type_error)
	    report_error(vp);
       	rpt_value_free(vp);
	break;

    case descend_message_dir_after:
       	break;
    }
}


static string_list_ty *stack;
static project_ty *pp;
static change_ty *cp;


string_ty *
stack_relative(string_ty *fn)
{
    string_ty	    *s1;
    string_ty	    *s2;
    size_t	    k;

    assert(stack);
    os_become_orig();
    s1 = os_pathname(fn, 1);
    os_become_undo();

    s2 = 0;
    for (k = 0; k < stack->nstrings; ++k)
    {
       	s2 = os_below_dir(stack->string[k], s1);
       	if (s2)
	    break;
    }
    str_free(s1);

    if (!s2)
    {
       	sub_context_ty	*scp;

       	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", s1);
       	if (cp)
	    change_fatal(cp, scp, i18n("$filename unrelated"));
       	project_fatal(pp, scp, i18n("$filename unrelated"));
       	/* NOTREACHED */
    }

    if (s2->str_length == 0)
    {
       	str_free(s2);
       	s2 = str_from_c(".");
    }

    return s2;
}


string_ty *
stack_nth(int n)
{
    assert(n >= 0);
    assert(stack);
    assert(stack->nstrings);
    if (!stack)
       	return 0;
    if (n < 0 || n >= stack->nstrings)
       	return 0;
    return stack->string[n];
}


void
cmdline_grammar(int argc, char **argv)
{
    extern int yyparse _((void));
    size_t	    j;
    user_ty	    *up;
    cstate	    cstate_data;
    tree_ty	    *tp2;
    int		    based;

    /*
     * parse the command line
     */
    cmdline_lex_open(argc, argv);
    number_of_ops = 0;
    resolve = -1;
    yyparse();
    cmdline_lex_close();

    /*
     * reject illegal combinations of options
     */
    if (grandparent)
    {
	if (branch)
       	{
	    mutually_exclusive_options
	    (
	     	arglex_token_branch,
	     	arglex_token_grandparent,
	     	usage
	    );
       	}
       	if (trunk)
       	{
	    mutually_exclusive_options
	    (
	     	arglex_token_trunk,
	     	arglex_token_grandparent,
	     	usage
	    );
       	}
       	branch = str_from_c("..");
    }
    if (trunk)
    {
       	if (branch)
       	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
	     	arglex_token_trunk,
	     	usage
	    );
       	}
       	branch = str_from_c("");
    }

    /*
     * locate project data
     */
    if (!project_name)
       	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    stack = string_list_new();
    if (baseline)
    {
       	if (change_number)
       	{
	    mutually_exclusive_options
	    (
	     	arglex_token_branch,
	     	arglex_token_change,
	     	usage
	    );
       	}

       	/*
	 * Get the search path from the project.
	 */
	project_search_path_get(pp, stack, 1);

       	up = 0;
       	cp = 0;
       	cstate_data = 0;
    }
    else
    {
       	/*
	 * locate user data
	 */
       	up = user_executing(pp);

       	/*
	 * locate change data
	 */
       	if (!change_number)
	    change_number = user_default_change(up);
       	cp = change_alloc(pp, change_number);
       	change_bind_existing(cp);
       	cstate_data = change_cstate_get(cp);

       	if (cstate_data->state == cstate_state_completed)
       	{
	    /*
	     * Get the search path from the project.
	     */
	    project_search_path_get(pp, stack, 1);

	    up = 0;
	    cp = 0;
	    cstate_data = 0;
       	}
       	else
       	{
	    /*
	     * It is an error if the change is not in the
	     * being_developed state (if it does not have a
	     * directory).
	     */
	    if (cstate_data->state < cstate_state_being_developed)
	     	change_fatal(cp, 0, i18n("bad aefind state"));

	    /*
	     * Get the search path from the change.
	     */
	    change_search_path_get(cp, stack, 1);
       	}
    }

    /*
     * resolve the path of each path
     * 1. the absolute path of the file name is obtained
     * 2. if the file is inside the development directory, ok
     * 3. if the file is inside the baseline, ok
     * 4. if neither, error
     */
    based =
       	(
	    stack->nstrings >= 1
       	&&
	    (
	     	user_relative_filename_preference
	     	(
		    up,
		    uconf_relative_filename_preference_current
	     	)
	    ==
	     	uconf_relative_filename_preference_base
	    )
       	);
    for (j = 0; j < path->nstrings; ++j)
    {
       	string_ty	*s0;
       	string_ty	*s1;
       	string_ty	*s2;

       	s0 = path->string[j];
       	if (s0->str_text[0] == '/' || !based)
	    s1 = str_copy(s0);
       	else
	    s1 = str_format("%S/%S", stack->string[0], s0);
       	str_free(s0);
       	s2 = stack_relative(s1);
       	assert(s2);
       	str_free(s1);
       	path->string[j] = s2;
    }

    /*
     * Optimize the tree.
     *
     * We can't do this as we go, because we don't know the search
     * path until we finish parsing the command line.
     */
    if (debug > 1)
    {
       	tree_print(tp);
       	printf("\n");
       	fflush(stdout);
    }
    tp2 = tree_optimize(tp);
    tree_delete(tp);
    tp = tp2;

    /*
     * walk each of the directories in turn
     */
    if (debug)
    {
	tree_print(tp);
	printf("\n");
       	fflush(stdout);
    }
    for (j = 0; j < path->nstrings; ++j)
       	descend(path->string[j], resolve, walker, 0);

    /*
     * None of the stuff is deleted.
     * Assume program exits shortly.
     */
}


static tree_ty *
make_sure_has_side_effects(tree_ty *x)
{
    tree_ty	    *tp1;
    tree_ty	    *tp2;

    if (tree_useful(x))
       	return tree_copy(x);

#if 0
    /*
     * Most of the time, this is exactly what users want, a file list.
     * So we don't whine at them.
     */
    error_intl
    (
       	0,
      i18n("warning: expression has no side effects, assuming you meant -PRint")
    );
#endif

    tp1 = shorthand_print();
    tp2 = tree_and_new(x, tp1);
    tree_delete(tp1);
    return tp2;
}

%}

%%

find
    : HELP
       	{
	    help(0, usage);
	}
    | generic_options op generic_options
    ;

op
    : strings tree14
       	{
	    path = $1;
	    tp = make_sure_has_side_effects($2);
	    tree_delete($2);
       	}
    ;

strings
    : STRING
       	{
	    $$ = string_list_new();
	    string_list_append($$, $1);
	    str_free($1);
       	}
    | strings STRING
       	{
	    $$ = $1;
	    string_list_append($$, $2);
	    str_free($2);
       	}
    ;

/*
 * The fundamental building blocks of expressions.
 */
tree1
    : THIS
       	{
	    $$ = tree_this_new();
	}
    | NOW
       	{
	    $$ = tree_now_new();
	}
    | STRING
       	%prec LPAREN
       	{
	    rpt_value_ty	*vp;

	    vp = rpt_value_string($1);
	    str_free($1);
	    $$ = tree_constant_new(vp);
	    rpt_value_free(vp);
       	}
    | TRUE_keyword
       	{
	    rpt_value_ty	*vp;

	    vp = rpt_value_boolean(1);
	    $$ = tree_constant_new(vp);
	    rpt_value_free(vp);
       	}
    | FALSE_keyword
       	{
	    rpt_value_ty	*vp;

	    vp = rpt_value_boolean(0);
	    $$ = tree_constant_new(vp);
	    rpt_value_free(vp);
       	}
    | NUMBER
       	{
	    rpt_value_ty	*vp;

	    vp = rpt_value_integer($1);
	    $$ = tree_constant_new(vp);
	    rpt_value_free(vp);
       	}
    | REAL
       	{
	    rpt_value_ty	*vp;

	    vp = rpt_value_real($1);
	    $$ = tree_constant_new(vp);
	    rpt_value_free(vp);
       	}
    | LPAREN tree14 RPAREN
       	{
	    $$ = $2;
	}
    ;

/*
 * The simple tests and actions.
 */
tree1
    : PRINT
       	{
	    $$ = shorthand_print();
	}
    | EXECUTE exec_list SEMICOLON
       	{
	    tree_list_ty *tlp;

	    tlp = tree_list_new();
	    tree_list_append(tlp, $2);
	    tree_delete($2);
	    $$ = function_execute(tlp);
	    tree_list_delete(tlp);
       	}
    | NAME STRING
       	{
	    $$ = shorthand_name($2);
	    str_free($2);
       	}
    | PATH STRING
       	{
	    $$ = shorthand_path($2);
	    str_free($2);
       	}
    | TYPE STRING
       	{
	    $$ = shorthand_type($2);
	    str_free($2);
       	}
    ;

/*
 * These next few deal with comparing various inode aspects.  They are
 * all shorthand for various function invocations and a comparison.
 */
tree1
    : NEWER STRING
       	{
	    $$ = shorthand_newer($2);
	    str_free($2);
       	}
    | AMIN comparator NUMBER
       	{
	    $$ = shorthand_atime($2, $3, 60);
       	}
    | ATIME comparator NUMBER
       	{
	    $$ = shorthand_atime($2, $3, 24*60*60);
       	}
    | CMIN comparator NUMBER
       	{
	    $$ = shorthand_ctime($2, $3, 60);
       	}
    | CTIME comparator NUMBER
       	{
	    $$ = shorthand_ctime($2, $3, 24*60*60);
       	}
    | MMIN comparator NUMBER
       	{
	    $$ = shorthand_mtime($2, $3, 60);
       	}
    | MTIME comparator NUMBER
       	{
	    $$ = shorthand_mtime($2, $3, 24*60*60);
       	}
    | SSIZE comparator NUMBER
       	{
	    $$ = shorthand_size($2, $3);
       	}
    ;

comparator
    : /* empty */
       	{ $$ = tree_eq_new; }
    | EQ
       	{ $$ = tree_eq_new; }
    | NE
       	{ $$ = tree_ne_new; }
    | LT
       	{ $$ = tree_lt_new; }
    | LE
       	{ $$ = tree_le_new; }
    | GT
       	{ $$ = tree_gt_new; }
    | GE
       	{ $$ = tree_ge_new; }
    ;

tree1
    : STRING LPAREN list_opt RPAREN
       	{
	    $$ = function_indirection($1, $3);
	    str_free($1);
	    tree_list_delete($3);
       	}
    ;

list_opt
    : /* empty */
       	{
	    $$ = tree_list_new();
	}
    | list
       	{
	    $$ = $1;
	}
    ;

list
    : tree13
       	{
	    $$ = tree_list_new();
	    tree_list_append($$, $1);
	    tree_delete($1);
       	}
    | list COMMA tree13
       	{
	    $$ = $1;
	    tree_list_append($$, $3);
	    tree_delete($3);
       	}
    ;

exec_list
    : tree10
       	{
	    $$ = $1;
       	}
    | exec_list tree10
       	%prec ANDAND
       	{
	    static tree_ty  *t1;
	    tree_ty         *t2;

	    if (!t1)
	    {
		string_ty	*s;
		rpt_value_ty	*vp;

		s = str_from_c(" ");
		vp = rpt_value_string(s);
		str_free(s);
		t1 = tree_constant_new(vp);
		rpt_value_free(vp);
	    }

	    t2 = tree_join_new($1, t1);
	    tree_delete($1);
	    $$ = tree_join_new(t2, $2);
	    tree_delete(t2);
	    tree_delete($2);
       	}
    ;

tree2
    : tree1
	{
	    $$ = $1;
	}
    | NOT tree2
       	{
	    $$ = tree_not_new($2);
	    tree_delete($2);
       	}
    | PLUS tree2
       	%prec unary
       	{
	    $$ = tree_pos_new($2);
	    tree_delete($2);
       	}
    | MINUS tree2
       	%prec unary
       	{
	    $$ = tree_neg_new($2);
	    tree_delete($2);
       	}
    | TILDE tree2
       	%prec unary
       	{
	    $$ = tree_bitwise_not_new($2);
	    tree_delete($2);
       	}
    ;

tree3
    : tree2
       	{
	    $$ = $1;
	}
    | tree3 MUL tree2
       	{
	    $$ = tree_mul_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree3 DIV tree2
       	{
	    $$ = tree_divide_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree3 MOD tree2
       	{
	    $$ = tree_mod_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree3 TILDE tree2
       	{
	    $$ = tree_match_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree4
    : tree3
       	{
	    $$ = $1;
	}
    | tree4 PLUS tree3
       	{
	    $$ = tree_plus_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree4 MINUS tree3
       	{
	    $$ = tree_subtract_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree4 JOIN tree3
       	{
	    $$ = tree_join_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree5
    : tree4
       	{
	    $$ = $1;
	}
    | tree5 SHIFT_LEFT tree4
       	{
	    $$ = tree_shift_left_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree5 SHIFT_RIGHT tree4
       	{
	    $$ = tree_shift_right_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree6
    : tree5
       	{
	    $$ = $1;
	}
    | tree6 LT tree5
       	{
	    $$ = tree_lt_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree6 LE tree5
       	{
	    $$ = tree_le_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree6 GT tree5
       	{
	    $$ = tree_gt_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree6 GE tree5
       	{
	    $$ = tree_ge_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree7
    : tree6
       	{
	    $$ = $1;
       	}
    | tree7 EQ tree6
       	{
	    $$ = tree_eq_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree7 NE tree6
       	{
	    $$ = tree_ne_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree8
    : tree7
       	{
	    $$ = $1;
	}
    | tree8 BIT_AND tree7
       	{
	    $$ = tree_bitwise_and_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree9
    : tree8
       	{
	    $$ = $1;
       	}
    | tree9 BIT_XOR tree8
       	{
	    $$ = tree_bitwise_xor_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree10
    : tree9
       	{
	    $$ = $1;
	}
    | tree10 BIT_OR tree9
       	{
	    $$ = tree_bitwise_or_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree11
    : tree10
       	{
	    $$ = $1;
	}
    | tree11 ANDAND tree10
       	{
	    $$ = tree_and_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    | tree11 tree10
       	%prec ANDAND
       	{
	    $$ = tree_and_new($1, $2);
	    tree_delete($1);
	    tree_delete($2);
       	}
    ;

tree12
    : tree11
       	{
	    $$ = $1;
	}
    | tree12 OROR tree11
       	{
	    $$ = tree_or_new($1, $3);
	    tree_delete($1);
	    tree_delete($3);
       	}
    ;

tree13
    : tree12
       	{
	    $$ = $1;
	}
    | tree13 QUESTION tree13 COLON tree12
       	%prec QUESTION
       	{
	    $$ = tree_triadic_new($1, $3, $5);
	    tree_delete($1);
	    tree_delete($3);
	    tree_delete($5);
       	}
    ;

tree14
    : tree13
       	{
	    $$ = $1;
	}
    | tree14 COMMA tree13
       	{
	    tree_ty	    *tp2;

	    tp2 = make_sure_has_side_effects($1);
	    tree_delete($1);
	    $$ = tree_comma_new(tp2, $3);
	    tree_delete(tp2);
	    tree_delete($3);
       	}
    ;

generic_options
    : /* empty */
    | generic_options generic_option
    ;

generic_option
    : LIBRARY STRING
       	{
	    gonzo_library_append($2->str_text);
	    str_free($2);
       	}
    | RESOLVE
       	{
	    if (resolve > 0)
	    {
	     	duplicate_option_by_name(arglex_token_resolve, usage);
	    }
	    if (resolve >= 0)
	    {
		mutually_exclusive_options
		(
		    arglex_token_resolve,
		    arglex_token_resolve_not,
		    usage
		);
	    }
	    resolve = 1;
	}
    | RESOLVE_NOT
       	{
	    if (resolve == 0)
	    {
	     	duplicate_option_by_name(arglex_token_resolve_not, usage);
	    }
	    if (resolve >= 0)
	    {
		mutually_exclusive_options
		(
		    arglex_token_resolve,
		    arglex_token_resolve_not,
		    usage
		);
	    }
	    resolve = 0;
       	}
    | TRACE trace_strings
       	{
#ifndef DEBUG
	    error_intl(0, i18n("-TRace needs DEBUG"));
#endif
       	}
    ;

trace_strings
    : trace_string
    | trace_strings trace_string
    ;

trace_string
    : STRING
       	{
#ifdef DEBUG
	    trace_enable(arglex_value.alv_string);
	    yydebug = trace_pretest_;
#endif
       	}
    ;

generic_option
    : BASELINE
       	{
	    if (baseline)
	    {
	     	duplicate_option_by_name(arglex_token_baseline, usage);
	    }
    	    baseline = 1;
       	}
    | CHANGE NUMBER
       	{
    	    if (change_number)
    	    {
	       	duplicate_option_by_name(arglex_token_change, usage);
    	    }
    	    change_number = $2;
    	    if (change_number == 0)
	       	change_number = MAGIC_ZERO;
    	    else if (change_number < 1)
    	    {
	       	sub_context_ty	*scp;

	       	scp = sub_context_new();
	       	sub_var_set_long(scp, "Number", change_number);
	       	fatal_intl(scp, i18n("change $number out of range"));
	       	/* NOTREACHED */
	       	sub_context_delete(scp);
    	    }
       	}
    | PROJECT STRING
       	{
    	    if (project_name)
	       	duplicate_option_by_name(arglex_token_project, usage);
    	    project_name = str_from_c(arglex_value.alv_string);
       	}
    | BRANCH number_or_string
       	{
	    if (branch)
	     	duplicate_option(usage);
	    branch = $2;
       	}
    | TRUNK
       	{
	    if (trunk)
	     	duplicate_option(usage);
	    ++trunk;
       	}
    | GRANDPARENT
       	{
	    if (grandparent)
	     	duplicate_option(usage);
	    ++grandparent;
       	}
    | DEBUG_keyword
       	{
	    ++debug;
	}
    | BASE_REL
       	{
	    user_relative_filename_preference_argument(usage);
	}
    | CUR_REL
       	{
	    user_relative_filename_preference_argument(usage);
	}
    ;

number_or_string
    : NUMBER
       	{
	    $$ = str_format("%ld", $1);
	}
    | STRING
       	{
	    $$ = $1;
       	}
    ;
