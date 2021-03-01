/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
 * MANIFEST: interface definition for aegis/lex.c
 */

#ifndef LEX_H
#define LEX_H

#include <main.h>

typedef struct lex_input_ty lex_input_ty;

typedef struct lex_input_method_ty lex_input_method_ty;
struct lex_input_method_ty
{
	void (*destruct)_((lex_input_ty *));
	int (*get)_((lex_input_ty *));
	void (*unget)_((lex_input_ty *, int));
	char *(*name)_((lex_input_ty *));
};

struct lex_input_ty
{
	lex_input_method_ty method;
	/* ...instance variables here... */
};

void lex_open _((char *));
void lex_open_env _((char *name));
void lex_close _((void));
void gram_error _((char *, ...));
int gram_lex _((void));

#endif /* LEX_H */
