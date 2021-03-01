/*
 *      aegis - project change supervisor
 *      Copyright (C) 2001-2006, 2008 Peter Miller
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

#include <common/ac/stdlib.h>

#include <common/error.h>
#include <aeimport/format/rcs/gram.h>
#include <aeimport/format/rcs/lex.h>
#include <aeimport/format/version_list.h>
#include <common/gettime.h>
#include <libaegis/help.h>
#include <common/str_list.h>
#include <common/symtab.h>

%}

%token ACCESS
%token AUTHOR
%token BRANCH
%token COLON
%token COMMENT
%token DATE
%token DESC
%token EXPAND
%token HEAD
%token IDENTIFIER
%token JUNK
%token LOCKS
%token LOG
%token NEXT
%token NUMBER
%token SEMI
%token STATE
%token STRICT
%token STRING
%token SUFFIX
%token SYMBOLS
%token TEXT

%union
{
    string_ty       *lv_string;
    string_list_ty  *lv_string_list;
}

%type <lv_string> NUMBER STRING IDENTIFIER date author next log state
%type <lv_string_list> strings_opt branch branch_opt identifiers_opt numbers_opt

%{

static symtab_ty *stp;
static format_version_ty *head;
static string_ty *pfn;
static string_ty *lfn;


static format_version_ty *
ancestor(format_version_ty *fvp)
{
    while (fvp->before)
        fvp = fvp->before;
    return fvp;
}


format_version_ty *
rcs_parse(string_ty *physical, string_ty *logical)
{
    extern int yyparse(void);

    head = 0;

    stp = symtab_alloc(5);
    pfn = physical;
    lfn = logical;

    rcs_lex_open(physical);
    yyparse();
    rcs_lex_close();

    symtab_free(stp);
    stp = 0;
    return ancestor(head);
}


static time_t
str2date(string_ty *s)
{
    string_list_ty sl;
    sl.split(s, ".");
    if (sl.nstrings != 6)
    {
        fatal_date_unknown(s->str_text);
    }

    /*
     * Turn it into a format that scan_date understands.
     * (Probably not the fastest way to get is converted into a date.)
     */
    string_ty *s2 =
        str_format
        (
            "%s/%s/%s %s:%s:%s GMT",
            sl.string[1]->str_text,
            sl.string[2]->str_text,
            sl.string[0]->str_text,
            sl.string[3]->str_text,
            sl.string[4]->str_text,
            sl.string[5]->str_text
        );
    time_t t = date_scan(s2->str_text);
    str_free(s2);
    if (t == (time_t)-1)
    {
        fatal_date_unknown(s->str_text);
    }
    return t;
}


static format_version_ty *
find(string_ty *edit)
{
    format_version_ty *rp;

    assert(stp);
    rp = (format_version_ty *)symtab_query(stp, edit);
    if (!rp)
    {
        rp = new format_version_ty();
        symtab_assign(stp, edit, rp);
        rp->edit = str_copy(edit);
        rp->filename_physical = str_copy(pfn);
        rp->filename_logical = str_copy(lfn);
    }
    return rp;
}


static int
is_a_branch_version_number(string_ty *s)
{
    const char *cp;
    int count_the_dots;

    count_the_dots = 0;
    for (cp = s->str_text; *cp; ++cp)
    {
        if (*cp == '.')
        {
            ++count_the_dots;
            if (count_the_dots >= 2)
                return 1;
        }
    }
    return 0;
}


%}

%%

file
    : admin tree desc edits
    ;

admin
    : head branch_opt suffix_opt access symbols locks strict_opt
      comment_opt expand_opt
        {
            delete $2;
        }
    ;

head
    : HEAD NUMBER SEMI
        {
            head = find($2);
            str_free($2);
        }
    ;

branch
    : BRANCH numbers_opt SEMI
        { $$ = $2; }
    ;

branch_opt
    : /* empty */
        { $$ = new string_list_ty(); }
    | branch
        { $$ = $1; }
    ;

suffix
    : SUFFIX IDENTIFIER SEMI
        { str_free($2); }
    | SUFFIX STRING SEMI
        { str_free($2); }
    ;

suffix_opt
    : /* empty */
    | suffix
    ;

access
    : ACCESS identifiers_opt SEMI
        { delete $2; }
    ;

symbols
    : SYMBOLS symbol_list SEMI
    ;

symbol_list
    : /* empty */
    | symbol_list symbol
    ;

symbol
    : IDENTIFIER COLON NUMBER
        {
            format_version_ty *rp;

            rp = find($3);
            rp->tag.push_back($1);

            str_free($1);
            str_free($3);
        }
    ;

locks
    : LOCKS lock_list SEMI
    ;

lock_list
    : /* empty */
    | lock_list lock
    ;

lock
    : IDENTIFIER COLON NUMBER
        {
            str_free($1);
            str_free($3);
        }
    ;

strict
    : STRICT SEMI
    ;

strict_opt
    : /* empty */
    | strict
    ;

comment
    : COMMENT STRING SEMI
        { str_free($2); }
    ;

comment_opt
    : /* empty */
    | comment
    ;

expand
    : EXPAND strings_opt SEMI
        { delete $2; }
    ;

expand_opt
    : /* empty */
    | expand
    ;

identifiers_opt
    : /* empty */
        {
            $$ = new string_list_ty();
        }
    | identifiers_opt IDENTIFIER
        {
            $$ = $1;
            $$->push_back($2);
            str_free($2);
        }
    ;

strings_opt
    : /* empty */
        {
            $$ = new string_list_ty();
        }
    | strings_opt STRING
        {
            $$ = $1;
            $$->push_back($2);
            str_free($2);
        }
    ;

tree
    : /* empty */
    | tree delta
    ;

delta
    : NUMBER date author state branch next
        {
            size_t      j;
            format_version_ty *rp;
            static string_ty *dead;

            rp = find($1);

            rp->when = str2date($2);
            str_free($2);
            rp->who = $3;

            /*
             * The RCS manual says that
             * "Exp" means experimental
             * "Stab" means stable
             * "Rel" means released
             *
             * CVS uses "dead" to indicate a file which has
             * been deleted, and should not be checked out.
             */
            if (!dead)
                dead = str_from_c("dead");
            rp->dead = str_equal($4, dead);
            str_free($4);

            if ($5->nstrings && !rp->after_branch)
                rp->after_branch = format_version_list_new();
            for (j = 0; j < $5->nstrings; ++j)
            {
                format_version_ty *other;

                other = find($5->string[j]);
                other->before = rp;
                format_version_list_append
                (
                    rp->after_branch,
                    other
                );
            }
            delete $5;

            if ($6)
            {
                /*
                 * The trunk is a roll-back lists, but
                 * the branches are roll-forward lists.
                 * (Sheesh!)
                 */
                if (is_a_branch_version_number($1))
                {
                    rp->after = find($6);
                    rp->after->before = rp;
                }
                else
                {
                    rp->before = find($6);
                    rp->before->after = rp;
                }
                str_free($6);
            }
        }
    ;

date
    : DATE NUMBER SEMI
        { $$ = $2; }
    ;

author
    : AUTHOR IDENTIFIER SEMI
        { $$ = $2; }
    ;

state
    : STATE IDENTIFIER SEMI
        { $$ = $2; }
    ;

numbers_opt
    : /* empty */
        {
            $$ = new string_list_ty();
        }
    | numbers_opt NUMBER
        {
            $$ = $1;
            $$->push_back($2);
            str_free($2);
        }
    ;

next
    : NEXT NUMBER SEMI
        { $$ = $2; }
    | NEXT SEMI
        { $$ = 0; }
    ;

desc
    : DESC STRING nosemi
        { str_free($2); }
    ;

edits
    : edit
    | edits edit
    ;

edit
    : NUMBER log text
        {
            format_version_ty *rp;

            rp = find($1);
            rp->description = $2;
        }
    ;

log
    : LOG STRING nosemi
        { $$ = $2; }
    ;

text
    : TEXT STRING nosemi
        { str_free($2); }
    ;

nosemi
    : /* empty */
        { rcs_lex_keyword_expected(); }
    ;
