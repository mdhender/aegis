/*
 * aegis - project change supervisor
 * Copyright (C) 1997-1999, 2001-2008, 2011, 2012 Peter Miller
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

%{

#include <common/ac/assert.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/version.h>

#include <aefind/cmdline.h>
#include <aefind/descend.h>
#include <aefind/function.h>
#include <aefind/function/execute.h>
#include <aefind/lex.h>
#include <aefind/shorthand/delete.h>
#include <aefind/shorthand/name.h>
#include <aefind/shorthand/path.h>
#include <aefind/shorthand/print.h>
#include <aefind/shorthand/stat.h>
#include <aefind/tree.h>
#include <aefind/tree/arithmetic.h>
#include <aefind/tree/bitwise.h>
#include <aefind/tree/constant.h>
#include <aefind/tree/list.h>
#include <aefind/tree/logical.h>
#include <aefind/tree/match.h>
#include <aefind/tree/now.h>
#include <aefind/tree/relative.h>
#include <aefind/tree/this.h>

#ifdef DEBUG
#define YYDEBUG 1
#define YYERROR_VERBOSE 1
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
%token DELETE
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
%token VERSION

%union
{
    struct string_ty *lv_string;
    struct string_list_ty *lv_string_list;
    tree::pointer *lv_tree;
    class tree_list *lv_tree_list;
    long lv_number;
    double lv_real;
    diadic_t comparator;
}

%type <comparator>  comparator
%type <lv_number>   NUMBER THIS
%type <lv_real>     REAL
%type <lv_string>   STRING
%type <lv_string>   number_or_string
%type <lv_string_list> strings strings_or_dot
%type <lv_tree>     tree1 tree2 tree3 tree4 tree5 tree6 tree7
%type <lv_tree>     tree8 tree9 tree10 tree11 tree12 tree13 tree14
%type <lv_tree_list> list
%type <lv_tree_list> list_opt
%type <lv_tree>     exec_list

%left COMMA
%right QUESTION COLON
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

static int      number_of_ops;
static string_list_ty *path;
static tree::pointer filter;
static string_ty *project_name;
static long     change_number;
static int      grandparent;
static string_ty *branch;
static int      trunk;
static int      baseline;
static int      resolve;
static int      debug;


static void
report_error(const rpt_value::pointer &vp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    const rpt_value_error *rve =
        dynamic_cast<const rpt_value_error *>(vp.get());
    if (!rve)
        return;
    sub_context_ty sc;
    sc.var_set_string("MeSsaGe", rve->query());
    sc.fatal_intl(i18n("$message"));
    /* NOTREACHED */
}


static void
walker(void *, descend_message_ty msg, string_ty *path_unres,
    string_ty *path_maybe, string_ty *path_res, struct stat *st)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    switch (msg)
    {
    case descend_message_file:
    case descend_message_dir_before:
        {
            rpt_value::pointer vp =
                filter->evaluate(path_unres, path_maybe, path_res, st);
            if (vp->is_an_error())
                report_error(vp);
        }
        break;

    case descend_message_dir_after:
        break;
    }
}


static string_list_ty *stack;
static project *pp;
static change::pointer cp;


string_ty *
stack_relative(string_ty *fn)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    assert(stack);
    os_become_orig();
    string_ty *s1 = os_pathname(fn, 1);
    os_become_undo();

    string_ty *s2 = 0;
    for (size_t k = 0; k < stack->nstrings; ++k)
    {
        s2 = os_below_dir(stack->string[k], s1);
        if (s2)
            break;
    }
    str_free(s1);

    if (!s2)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", s1);
        if (cp)
            change_fatal(cp, &sc, i18n("$filename unrelated"));
        project_fatal(pp, &sc, i18n("$filename unrelated"));
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
    trace(("%s\n", __PRETTY_FUNCTION__));
    assert(n >= 0);
    assert(stack);
    assert(stack->nstrings);
    if (!stack)
        return 0;
    if (n < 0 || n >= (int)stack->nstrings)
        return 0;
    return stack->string[n];
}


int
stack_eliminate(string_ty *filename)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    fstate_src_ty *src = pp->file_find(filename, view_path_simple);
    if (!src)
        return 0;
    switch (src->action)
    {
    case file_action_create:
    case file_action_modify:
        break;

    case file_action_remove:
        return 1;

    case file_action_insulate:
    case file_action_transparent:
        break;
    }
    return 0;
}


void
cmdline_grammar(int argc, char **argv)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    extern int yyparse(void);
    size_t          j;
    cstate_ty       *cstate_data;
    int             based;

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
    {
        nstring n = user_ty::create()->default_project();
        project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    stack = new string_list_ty();
    user_ty::pointer up = user_ty::create();
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
        pp->search_path_get(stack, true);

        cp = 0;
        cstate_data = 0;
    }
    else
    {
        /*
         * locate change data
         */
        if (!change_number)
            change_number = up->default_change(pp);
        cp = change_alloc(pp, change_number);
        change_bind_existing(cp);
        cstate_data = cp->cstate_get();

        if (cstate_data->state == cstate_state_completed)
        {
            /*
             * Get the search path from the project.
             */
            pp->search_path_get(stack, true);

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
            cp->search_path_get(stack, true);
        }
    }

    /*
     * resolve the path of each path
     * 1. the absolute path of the file name is obtained
     * 2. if the file is inside the development directory, ok
     * 3. if the file is inside the baseline, ok
     * 4. if neither, error
     */
    assert(up);
    based =
        (
            stack->nstrings >= 1
        &&
            (
                up->relative_filename_preference
                (
                    uconf_relative_filename_preference_current
                )
            ==
                uconf_relative_filename_preference_base
            )
        );
    for (j = 0; j < path->nstrings; ++j)
    {
        string_ty       *s0;
        string_ty       *s1;
        string_ty       *s2;

        s0 = path->string[j];
        if (s0->str_text[0] == '/' || !based)
            s1 = str_copy(s0);
        else
            s1 = os_path_join(stack->string[0], s0);
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
        filter->print();
        printf("\n");
        fflush(stdout);
    }
    filter = filter->optimize();

    /*
     * walk each of the directories in turn
     */
    if (debug)
    {
        filter->print();
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


static tree::pointer
make_sure_has_side_effects(const tree::pointer &x)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (x->useful())
        return x;

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

    return tree_and::create(x, shorthand_print());
}

%}

%%

find
    : HELP
        {
            help(0, usage);
            quit(0);
        }
    | generic_options op generic_options
    ;

op
    : strings_or_dot tree14
        {
            path = $1;
            filter = make_sure_has_side_effects(*$2);
            delete $2;
        }
    ;

strings_or_dot
    : strings
    | /* empty */
        {
            string_ty       *dot;

            /*
             * Default the path list to "." (the current directory).
             */
            $$ = new string_list_ty();
            dot = str_from_c(".");
            $$->push_back(dot);
            str_free(dot);
        }
    ;

strings
    : STRING
        {
            $$ = new string_list_ty();
            $$->push_back($1);
            str_free($1);
        }
    | strings STRING
        {
            $$ = $1;
            $$->push_back($2);
            str_free($2);
        }
    ;

/*
 * The fundamental building blocks of expressions.
 */
tree1
    : THIS
        {
            tree::pointer tp = tree_this::create($1);
            $$ = new tree::pointer(tp);
        }
    | NOW
        {
            tree::pointer tp = tree_now_new();
            $$ = new tree::pointer(tp);
        }
    | STRING
        %prec LPAREN
        {
            rpt_value::pointer vp = rpt_value_string::create(nstring($1));
            str_free($1);
            tree::pointer tp = tree_constant::create(vp);
            $$ = new tree::pointer(tp);
        }
    | TRUE_keyword
        {
            rpt_value::pointer vp = rpt_value_boolean::create(true);
            tree::pointer tp = tree_constant::create(vp);
            $$ = new tree::pointer(tp);
        }
    | FALSE_keyword
        {
            rpt_value::pointer vp = rpt_value_boolean::create(false);
            tree::pointer tp = tree_constant::create(vp);
            $$ = new tree::pointer(tp);
        }
    | NUMBER
        {
            rpt_value::pointer vp = rpt_value_integer::create($1);
            tree::pointer tp = tree_constant::create(vp);
            $$ = new tree::pointer(tp);
        }
    | REAL
        {
            rpt_value::pointer vp = rpt_value_real::create($1);
            tree::pointer tp = tree_constant::create(vp);
            $$ = new tree::pointer(tp);
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
            tree::pointer tp = shorthand_print();
            $$ = new tree::pointer(tp);
        }
    | DELETE
        {
            tree::pointer tp = shorthand_delete();
            $$ = new tree::pointer(tp);
        }
    | EXECUTE exec_list SEMICOLON
        {
            tree::pointer tp = tree_execute::create(*$2);
            delete $2;
            $$ = new tree::pointer(tp);
        }
    | NAME STRING
        {
            tree::pointer tp = shorthand_name(nstring($2));
            str_free($2);
            $$ = new tree::pointer(tp);
        }
    | PATH STRING
        {
            tree::pointer tp = shorthand_path(nstring($2));
            str_free($2);
            $$ = new tree::pointer(tp);
        }
    | TYPE STRING
        {
            tree::pointer tp = shorthand_type(nstring($2));
            str_free($2);
            $$ = new tree::pointer(tp);
        }
    ;

/*
 * These next few deal with comparing various inode aspects.  They are
 * all shorthand for various function invocations and a comparison.
 */
tree1
    : NEWER STRING
        {
            tree::pointer tp = shorthand_newer(nstring($2));
            str_free($2);
            $$ = new tree::pointer(tp);
        }
    | AMIN comparator NUMBER
        {
            tree::pointer tp = shorthand_atime($2, $3, 60);
            $$ = new tree::pointer(tp);
        }
    | ATIME comparator NUMBER
        {
            tree::pointer tp = shorthand_atime($2, $3, 24*60*60);
            $$ = new tree::pointer(tp);
        }
    | CMIN comparator NUMBER
        {
            tree::pointer tp = shorthand_ctime($2, $3, 60);
            $$ = new tree::pointer(tp);
        }
    | CTIME comparator NUMBER
        {
            tree::pointer tp = shorthand_ctime($2, $3, 24*60*60);
            $$ = new tree::pointer(tp);
        }
    | MMIN comparator NUMBER
        {
            tree::pointer tp = shorthand_mtime($2, $3, 60);
            $$ = new tree::pointer(tp);
        }
    | MTIME comparator NUMBER
        {
            tree::pointer tp = shorthand_mtime($2, $3, 24*60*60);
            $$ = new tree::pointer(tp);
        }
    | SSIZE comparator NUMBER
        {
            tree::pointer tp = shorthand_size($2, $3);
            $$ = new tree::pointer(tp);
        }
    ;

comparator
    : /* empty */
        { $$ = &tree_eq::create; }
    | EQ
        { $$ = &tree_eq::create; }
    | NE
        { $$ = &tree_ne::create; }
    | LT
        { $$ = &tree_lt::create; }
    | LE
        { $$ = &tree_le::create; }
    | GT
        { $$ = &tree_gt::create; }
    | GE
        { $$ = &tree_ge::create; }
    ;

tree1
    : STRING LPAREN list_opt RPAREN
        {
            tree::pointer tp = function_indirection($1, *$3);
            str_free($1);
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

list_opt
    : /* empty */
        {
            $$ = new tree_list();
        }
    | list
        {
            $$ = $1;
        }
    ;

list
    : tree13
        {
            $$ = new tree_list();
            $$->append(*$1);
            delete $1;
        }
    | list COMMA tree13
        {
            $$ = $1;
            $$->append(*$3);
            delete $3;
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
            static tree::pointer t1;
            if (!t1)
            {
                rpt_value::pointer vp = rpt_value_string::create(" ");
                t1 = tree_constant::create(vp);
            }

            tree::pointer t2 = tree_join::create(*$1, t1);
            delete $1;
            tree::pointer tp = tree_join::create(t2, *$2);
            delete $2;
            $$ = new tree::pointer(tp);
        }
    ;

tree2
    : tree1
        {
            $$ = $1;
        }
    | NOT tree2
        {
            tree::pointer tp = tree_not::create(*$2);
            delete $2;
            $$ = new tree::pointer(tp);
        }
    | PLUS tree2
        %prec unary
        {
            tree::pointer tp = tree_pos::create(*$2);
            delete $2;
            $$ = new tree::pointer(tp);
        }
    | MINUS tree2
        %prec unary
        {
            tree::pointer tp = tree_neg::create(*$2);
            delete $2;
            $$ = new tree::pointer(tp);
        }
    | TILDE tree2
        %prec unary
        {
            tree::pointer tp = tree_bitwise_not::create(*$2);
            delete $2;
            $$ = new tree::pointer(tp);
        }
    ;

tree3
    : tree2
        {
            $$ = $1;
        }
    | tree3 MUL tree2
        {
            tree::pointer tp = tree_mul::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree3 DIV tree2
        {
            tree::pointer tp = tree_divide::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree3 MOD tree2
        {
            tree::pointer tp = tree_mod::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree3 TILDE tree2
        {
            tree::pointer tp = tree_match::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

tree4
    : tree3
        {
            $$ = $1;
        }
    | tree4 PLUS tree3
        {
            tree::pointer tp = tree_plus::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree4 MINUS tree3
        {
            tree::pointer tp = tree_subtract::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree4 JOIN tree3
        {
            tree::pointer tp = tree_join::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

tree5
    : tree4
        {
            $$ = $1;
        }
    | tree5 SHIFT_LEFT tree4
        {
            tree::pointer tp = tree_shift_left::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree5 SHIFT_RIGHT tree4
        {
            tree::pointer tp = tree_shift_right::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

tree6
    : tree5
        {
            $$ = $1;
        }
    | tree6 LT tree5
        {
            tree::pointer tp = tree_lt::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree6 LE tree5
        {
            tree::pointer tp = tree_le::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree6 GT tree5
        {
            tree::pointer tp = tree_gt::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree6 GE tree5
        {
            tree::pointer tp = tree_ge::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

tree7
    : tree6
        {
            $$ = $1;
        }
    | tree7 EQ tree6
        {
            tree::pointer tp = tree_eq::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree7 NE tree6
        {
            tree::pointer tp = tree_ne::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

tree8
    : tree7
        {
            $$ = $1;
        }
    | tree8 BIT_AND tree7
        {
            tree::pointer tp = tree_bitwise_and::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

tree9
    : tree8
        {
            $$ = $1;
        }
    | tree9 BIT_XOR tree8
        {
            tree::pointer tp = tree_bitwise_xor::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

tree10
    : tree9
        {
            $$ = $1;
        }
    | tree10 BIT_OR tree9
        {
            tree::pointer tp = tree_bitwise_or::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    ;

tree11
    : tree10
        {
            $$ = $1;
        }
    | tree11 ANDAND tree10
        {
            tree::pointer tp = tree_and::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
        }
    | tree11 tree10
        %prec ANDAND
        {
            tree::pointer tp = tree_and::create(*$1, *$2);
            delete $1;
            delete $2;
            $$ = new tree::pointer(tp);
        }
    ;

tree12
    : tree11
        {
            $$ = $1;
        }
    | tree12 OROR tree11
        {
            tree::pointer tp = tree_or::create(*$1, *$3);
            delete $1;
            delete $3;
            $$ = new tree::pointer(tp);
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
            tree::pointer tp = tree_triadic::create(*$1, *$3, *$5);
            delete $1;
            delete $3;
            delete $5;
            $$ = new tree::pointer(tp);
        }
    ;

tree14
    : tree13
        {
            $$ = $1;
        }
    | tree14 COMMA tree13
        {
            tree::pointer tp2 = make_sure_has_side_effects(*$1);
            delete $1;
            tree::pointer tp = tree_comma::create(tp2, *$3);
            delete $3;
            $$ = new tree::pointer(tp);
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
    | VERSION
        {
            version();
            quit(0);
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
                sub_context_ty  *scp;

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
            user_ty::relative_filename_preference_argument(usage);
        }
    | CUR_REL
        {
            user_ty::relative_filename_preference_argument(usage);
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


// vim: set ts=8 sw=4 et :
