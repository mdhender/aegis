/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: interface definition for aefind/lex.c
 */

#ifndef AEFIND_LEX_H
#define AEFIND_LEX_H

#include <arglex2.h>

enum
{
	arglex_token_and = ARGLEX2_MAX,
	arglex_token_atime,
	arglex_token_bit_and,
	arglex_token_bit_or,
	arglex_token_bit_xor,
	arglex_token_comma,
	arglex_token_ctime,
	arglex_token_debug,
	arglex_token_div,
	arglex_token_eq,
	arglex_token_false,
	arglex_token_ge,
	arglex_token_gt,
	arglex_token_join,
	arglex_token_le,
	arglex_token_left_paren,
	arglex_token_lt,
	arglex_token_mod,
	arglex_token_mtime,
	arglex_token_mul,
	arglex_token_namekw,
	arglex_token_ne,
	arglex_token_newer,
	arglex_token_not,
	arglex_token_now,
	arglex_token_or,
	arglex_token_path,
	arglex_token_plus,
	arglex_token_print,
	arglex_token_resolve,
	arglex_token_right_paren,
	arglex_token_shift_left,
	arglex_token_shift_right,
	arglex_token_size,
	arglex_token_stringize,
	arglex_token_this,
	arglex_token_tilde,
	arglex_token_true,
	arglex_token_type,
	ARGLEX3_MAX
};

void cmdline_lex_open _((int, char **));
void cmdline_lex_close _((void));
int cmdline_lex _((void));
void cmdline_error _((char *));
void usage _((void));

struct sub_context_ty; /* existence */
void cmdline_lex_error _((struct sub_context_ty *, char *));

#endif /* AEFIND_LEX_H */
