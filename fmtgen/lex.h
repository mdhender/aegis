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
 * MANIFEST:  interface definition for fmtgen/lex.c
 */

#ifndef LEX_H
#define LEX_H

#include <main.h>
#include <str.h>

typedef struct parse_list_ty parse_list_ty;
struct parse_list_ty
{
	string_ty	*name;
	parse_list_ty	*next;
};

void lex_open _((char *));
void lex_close _((void));
void parse_error _((char *, ...));
int parse_lex _((void));
int lex_in_include_file _((void));
void lex_list_include_files _((void));
void lex_include_path _((char *));

#endif /* LEX_H */
