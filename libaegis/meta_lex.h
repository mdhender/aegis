//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1999, 2001, 2002, 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_META_LEX_H
#define LIBAEGIS_META_LEX_H

#include <common/main.h>

class sub_context_ty; // forward
class input; // forward
struct string_ty; // forward

void lex_open(struct string_ty *);
void lex_open_env(const char *name);
void lex_open_input(input &);
void lex_close(void);
void lex_error(struct sub_context_ty *, const char *);
void gram_error(const char *);
int gram_lex(void);

/**
  * The lex_position function is used to get the current lexer position.
  * If no file is open at present, it returns "end of input".
  */
string_ty *lex_position(void);

#endif // LIBAEGIS_META_LEX_H
