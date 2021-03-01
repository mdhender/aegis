//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 2002, 2003, 2005, 2006, 2008 Peter Miller.
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

#ifndef LEX_H
#define LEX_H

#include <common/main.h>
#include <common/str.h>

void lex_open(const char *);
void lex_close(void);
void parse_error(const char *, ...)			ATTR_PRINTF(1, 2);
int parse_lex(void);
int lex_in_include_file(void);
void lex_list_include_files(void);
void lex_include_path(const char *);

void lex_debug_printf(const char *, ...)		ATTR_PRINTF(1, 2);
void lex_debug_fprintf(void *, const char *, ...)	ATTR_PRINTF(2, 3);

// fix for stupid Sun yacc
#define parse_error parse_error
#define parse_lex parse_lex

#endif // LEX_H
