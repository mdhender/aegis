/*
 * aegis - project change supervisor
 * Copyright (C) 1991-1994, 1999, 2002, 2004-2006, 2008, 2012 Peter Miller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

%{

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/error.h>

#include <common/nstring.h>
#include <common/trace.h>
#include <libaegis/gram.h>
#include <libaegis/meta_lex.h>
#include <libaegis/sem.h>

#ifdef DEBUG
#define YYDEBUG 1
#endif

%}

%token INTEGER
%token NAME
%token REAL
%token STRING

%union
{
    nstring *lv_string;
    long lv_integer;
    double lv_real;
}

%type <lv_string> NAME STRING string
%type <lv_integer> INTEGER integer
%type <lv_real> REAL real

%%

file
    : turn_debug_on field_list
    ;

turn_debug_on
    : /* empty */
        {
#ifdef DEBUG
            yydebug = trace_pretest_;
#endif
        }
    ;

field_list
    : /* empty */
    | field_list field
    ;

field
    : field_name '=' value ';'
        {
            sem_field_end();
        }
    ;

field_name
    : NAME
        {
            sem_field(*$1);
            delete $1;
        }
    ;

value
    : integer
        {
            sem_integer($1);
        }
    | real
        {
            sem_real($1);
        }
    | string
        {
            sem_string(*$1);
            delete $1;
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

real
    : REAL
        { $$ = $1; }
    | '-' REAL
        { $$ = -$2; }
    ;

string
    : STRING
        { $$ = $1; }
    | string STRING
        {
            nstring result = *$1 + *$2;
            delete $1;
            delete $2;
            $$ = new nstring(result);
        }
    ;

structure
    : '{' field_list '}'
    ;

list
    : '[' optional_value_list ']'
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
    : { sem_list(); } value { sem_list_end(); }
    ;

optional_comma
    : /* empty */
    | ','
    ;

enumeration
    : NAME
        {
            sem_enum(*$1);
            delete $1;
        }
    ;


/* vim: set ts=8 sw=4 et : */
