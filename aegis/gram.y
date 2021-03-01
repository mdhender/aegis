/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: grammar and functions to parse aegis' data files
 */

%{

#include <stdlib.h>

#include <gram.h>
#include <lex.h>
#include <mem.h>
#include <parse.h>

%}

%token NAME
%token INTEGER
%token STRING

%union
{
	string_ty	*lv_string;
	long		lv_integer;
}

%type <lv_string> NAME STRING
%type <lv_integer> INTEGER integer

%%

file
	: field_list
	;

field_list
	: /* empty */
	| field_list field
	;

field
	: field_name '=' value ';'
		{
			sem_pop();
		}
	;

field_name
	: NAME
		{
			sem_field($1);
		}
	;

value
	: integer
		{
			sem_integer($1);
		}
	| STRING
		{
			sem_string($1);
		}
	| structure
	| list
	| enumeration
	;

integer
	: INTEGER
		{ $$ = $1; }
	| '-' INTEGER
		{ $$ = -$2; }
	;

structure
	: '{' field_list '}'
	;

list
	: list_begin optional_value_list list_end
	;

list_begin
	: '['
		{
			/* sem_list(); */
		}
	;

list_end
	: ']'
		{
			/* sem_pop(); */
		}
	;

optional_value_list
	: /* empty */
	| value_list optional_comma
	;

value_list
	: list_value
	| value_list ',' list_value
	;

list_value
	: { sem_list(); } value {sem_pop(); }
	;

optional_comma
	: /* empty */
	| ','
	;

enumeration
	: NAME
		{
			sem_enum($1);
		}
	;
