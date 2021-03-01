//
// aegis - project change supervisor
// Copyright (C) 1997, 2002, 2003, 2005, 2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEFIND_LEX_H
#define AEFIND_LEX_H

#include <libaegis/arglex2.h>

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
    arglex_token_delete,
    arglex_token_div,
    arglex_token_eq,
    arglex_token_execute,
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
    arglex_token_resolve_not,
    arglex_token_right_paren,
    arglex_token_semicolon,
    arglex_token_shift_left,
    arglex_token_shift_right,
    arglex_token_size,
    arglex_token_stringize,
    arglex_token_this,
    arglex_token_this_resolved,
    arglex_token_this_unresolved,
    arglex_token_tilde,
    arglex_token_true,
    arglex_token_type,
    ARGLEX3_MAX
};

void cmdline_lex_open(int, char **);
void cmdline_lex_close(void);
int cmdline_lex(void);
void cmdline_error(const char *);
void usage(void);

struct sub_context_ty; // existence
void cmdline_lex_error(struct sub_context_ty *, const char *);

#endif // AEFIND_LEX_H
// vim: set ts=8 sw=4 et :
