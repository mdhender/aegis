/*
 *      aegis - project change supervisor
 *      Copyright (C) 2001-2008 Peter Miller
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

#include <common/error.h> /* for assert */
#include <aeimport/format/sccs/gram.h>
#include <aeimport/format/sccs/lex.h>
#include <aeimport/format/version.h>
#include <aeimport/format/version_list.h>
#include <common/gettime.h>
#include <libaegis/help.h>
#include <common/itab.h>
#include <common/str_list.h>

#ifdef DEBUG
#undef YYDEBUG
#define YYDEBUG 1
#endif

%}

%token COMMENT
%token DELTA_BEGIN
%token DELTA_END
%token D_KEYWORD
%token E_KEYWORD
%token FLAGS
%token HEADER
%token I_KEYWORD
%token JUNK
%token MR
%token MR_EXCLUDE
%token MR_IGNORE
%token MR_INCLUDE
%token STRING
%token SUMMARY
%token TEXTLINE
%token TITLE_BEGIN
%token TITLE_END
%token USERS_BEGIN
%token USERS_END

%union {
    string_ty       *lv_string;
    long            lv_long;
}

%type <lv_long> date_and_time number
%type <lv_string> COMMENT STRING TEXTLINE

%{

static format_version_ty *current;
static itab_ty  *itp;
static string_ty *pfn;
static string_ty *lfn;


static format_version_ty *
find(int n)
{
    format_version_ty *fvp;

    assert(itp);
    fvp = (format_version_ty *)itab_query(itp, n);
    if (!fvp)
    {
        fvp = new format_version_ty();
        itab_assign(itp, n, fvp);
        fvp->filename_physical = str_copy(pfn);
        fvp->filename_logical = str_copy(lfn);
    }
    return fvp;
}


static int
count_the_dots(string_ty *s)
{
    const char      *cp;
    int             result;

    result = 0;
    for (cp = s->str_text; *cp; ++cp)
        if (*cp == '.')
            ++result;
    return result;
}


static void
walker(itab_ty *, itab_key_ty, void *data, void *)
{
    format_version_ty *fvp;
    int             ndots;
    size_t          j;

    fvp = (format_version_ty *)data;
    if (fvp->after_branch)
    {
        ndots = count_the_dots(fvp->edit);
        for (j = 0; j < fvp->after_branch->length; ++j)
        {
            format_version_ty *after;

            after = fvp->after_branch->item[j];
            if (after->edit && count_the_dots(after->edit) <= ndots)
            {
                fvp->after = after;
                if (j + 1 < fvp->after_branch->length)
                {
                    fvp->after_branch->item[j] =
                        fvp->after_branch->item[fvp->after_branch->length - 1];
                }
                fvp->after_branch->length--;
                if (fvp->after_branch->length == 0)
                {
                    format_version_list_delete(fvp->after_branch, 0);
                    fvp->after_branch = 0;
                }
                break;
            }
        }
    }
}


format_version_ty *
sccs_parse(string_ty *physical, string_ty *logical)
{
    extern int yyparse(void);
    format_version_ty *result;

    itp = itab_alloc();
    pfn = physical;
    lfn = logical;

    sccs_lex_open(physical);
#if YYDEBUG
    { extern int yydebug; yydebug = 1; }
#endif
    yyparse();
    sccs_lex_close();

    itab_walk(itp, walker, 0);

    result = find(1);
    itab_free(itp);
    itp = 0;
    return result;
}

%}

%%

sccsfile
    : header deltas users flags title body
    ;

header
    : HEADER ignore_arguments
    ;

deltas
    : delta
    | deltas delta
    ;

delta
    : summary delta_begin delta_lines delta_end
    ;

summary
    : SUMMARY ignore_arguments
    ;

delta_begin
    : DELTA_BEGIN STRING STRING date_and_time STRING number number
        {
            format_version_ty *before;

            /*
             * $2 some kind of state info, we ignore it
             * $3 sid
             * $4 time_t
             * $5 who
             * $6 number of this delta
             * $7 number of predecessor
             */
            before = $7 ? find($7) : 0;
            str_free($2);
            current = find($6);
            current->edit = $3;
            current->when = $4;
            current->who = $5 ;
            current->before = before;
            if (before)
            {
                if (!before->after_branch)
                {
                    before->after_branch = format_version_list_new();
                }
                format_version_list_append(before->after_branch, current);
            }
        }
    ;

date_and_time
    : STRING STRING
        {
            string_list_ty  sl;
            string_ty       *s;
            time_t          t;

            /*
             * Convert from the emminently sensable YY/MM/DD
             * into the hideous american MM/DD/YY
             */
            sl.split($1, "/");
            str_free($1);
            while (sl.nstrings < 3)
                sl.push_back(str_from_c(""));
            s =
                str_format
                (
                    "%s/%s/%s %s GMT",
                    sl.string[1]->str_text,
                    sl.string[2]->str_text,
                    sl.string[0]->str_text,
                    $2->str_text
                );
            str_free($2);
            t = date_scan(s->str_text);
            if (t == (time_t)-1)
                fatal_date_unknown(s->str_text);
            str_free(s);
            $$ = t;
        }
    ;

number
    : STRING
        {
            char *cp = 0;
            $$ = strtol($1->str_text, &cp, 10);
            if ($1->str_length && *cp != 0)
                yyerror("number expected");
            str_free($1);
        }
    ;

delta_lines
    : /* empty */
    | delta_lines delta_line
    ;

delta_line
    : comment
    | mr
    | included
    | excluded
    | ignored
    | error
    ;

comment
    : COMMENT
        {
            string_ty       *s;

            assert(current);
            if (current)
            {
                if (current->description)
                {
                    s =
			str_format
			(
			    "%s\n%s",
			    current->description->str_text,
			    $1->str_text
			);
                    str_free(current->description);
                    current->description = s;
                }
                else
                    current->description = str_copy($1);
            }
            str_free($1);
        }
    ;

mr
    : MR ignore_arguments
    ;

included
    : MR_INCLUDE ignore_arguments
    ;

excluded
    : MR_EXCLUDE ignore_arguments
    ;

ignored
    : MR_IGNORE ignore_arguments
    ;

delta_end
    : DELTA_END ignore_arguments
        {
            assert(current);
            if (!current->description)
            {
                current->description = str_from_c("no description");
            }
            current = 0;
        }
    ;

users
    : users_start textlines_opt users_end
    ;

users_start
    : USERS_BEGIN ignore_arguments
    ;

users_end
    : USERS_END ignore_arguments
    ;

flags
    : /* empty */
    | flags flag
    ;

flag
    : FLAGS ignore_arguments
    ;

ignore_arguments
    : /* empty */
    | ignore_arguments STRING
        { str_free($2); }
    ;

title
    : title_begin textlines_opt title_end
    ;

title_begin
    : TITLE_BEGIN ignore_arguments
    ;

title_end
    : TITLE_END ignore_arguments
    ;

body
    : /* empty */
    | body I_stuff
    | body D_stuff
    | body E_stuff
    | body error
    ;

/* ^AI <n> */
I_stuff
    : I_KEYWORD ignore_arguments textlines_opt
    ;

/* ^AD <n> */
D_stuff
    : D_KEYWORD ignore_arguments textlines_opt
    ;

/* ^AE <n> */
E_stuff
    : E_KEYWORD ignore_arguments textlines_opt
    ;

textlines_opt
    : /* empty */
    | textlines_opt TEXTLINE
        { str_free($2); }
    ;
