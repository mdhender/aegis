/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to parse RCS files
 */

%{

#include <ac/stdlib.h>

#include <error.h>
#include <format/rcs/gram.h>
#include <format/rcs/lex.h>
#include <format/version_list.h>
#include <gettime.h>
#include <help.h>
#include <str_list.h>
#include <symtab.h>

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
	string_ty	*lv_string;
	string_list_ty	*lv_string_list;
}

%type <lv_string> NUMBER STRING IDENTIFIER date author next log state
%type <lv_string_list> strings_opt branch branch_opt identifiers_opt numbers_opt

%{

static symtab_ty	*stp;
static format_version_ty *head;
static string_ty	*pfn;
static string_ty	*lfn;


static format_version_ty *ancestor _((format_version_ty *));

static format_version_ty *
ancestor(fvp)
	format_version_ty *fvp;
{
	while (fvp->before)
		fvp = fvp->before;
	return fvp;
}


format_version_ty *
rcs_parse(physical, logical)
	string_ty	*physical;
	string_ty	*logical;
{
	extern int yyparse _((void));

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


static time_t str2date _((string_ty *));

static time_t
str2date(s)
	string_ty	*s;
{
	string_list_ty	sl;
	string_ty	*s2;
	time_t		t;

	string_list_constructor(&sl);
	str2wl(&sl, s, ".", 0);
	if (sl.nstrings != 6)
	{
		string_list_destructor(&sl);

		failed:
		fatal_date_unknown(s->str_text);
	}

	/*
	 * Turn it into a format that scan_date understands.
	 * (Probably not the fastest way to get is converted into a date.)
	 */
	s2 =
		str_format
		(
			"%S/%S/%S %S:%S:%S GMT",
			sl.string[1],
			sl.string[2],
			sl.string[0],
			sl.string[3],
			sl.string[4],
			sl.string[5]
		);
	string_list_destructor(&sl);
	t = date_scan(s2->str_text);
	str_free(s2);
	if (t == (time_t)-1)
		goto failed;
	return t;
}


static format_version_ty *find _((string_ty *));

static format_version_ty *
find(edit)
	string_ty	*edit;
{
	format_version_ty *rp;

	assert(stp);
	rp = symtab_query(stp, edit);
	if (!rp)
	{
		rp = format_version_new();
		symtab_assign(stp, edit, rp);
		rp->edit = str_copy(edit);
		rp->filename_physical = str_copy(pfn);
		rp->filename_logical = str_copy(lfn);
	}
	return rp;
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
			string_list_delete($2);
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
		{ $$ = string_list_new(); }
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
		{ string_list_delete($2); }
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
			string_list_append(&rp->tag, $1);

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
		{ string_list_delete($2); }
	;

expand_opt
	: /* empty */
	| expand
	;

identifiers_opt
	: /* empty */
		{
			$$ = string_list_new();
		}
	| identifiers_opt IDENTIFIER
		{
			$$ = $1;
			string_list_append($$, $2);
			str_free($2);
		}
	;

strings_opt
	: /* empty */
		{
			$$ = string_list_new();
		}
	| strings_opt STRING
		{
			$$ = $1;
			string_list_append($$, $2);
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
			size_t		j;
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
			string_list_delete($5);

			if ($6)
			{
				rp->before = find($6);
				rp->before->after = rp;
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
			$$ = string_list_new();
		}
	| numbers_opt NUMBER
		{
			$$ = $1;
			string_list_append($$, $2);
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
