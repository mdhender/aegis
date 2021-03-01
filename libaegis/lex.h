/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1995, 1999, 2001 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/lex.c
 */

#ifndef LEX_H
#define LEX_H

#include <main.h>

struct sub_context_ty; /* existence */
struct input_ty; /* existence */
struct string_ty; /* existence */

void lex_open _((struct string_ty *));
void lex_open_env _((const char *name));
void lex_open_input _((struct input_ty *));
void lex_close _((void));
void lex_error _((struct sub_context_ty *, const char *));
void gram_error _((const char *));
int gram_lex _((void));

#endif /* LEX_H */
