//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef AEIMPORT_FORMAT_RCS_LEX_H
#define AEIMPORT_FORMAT_RCS_LEX_H

#include <common/str.h>

void rcs_lex_open(string_ty *);
void rcs_lex_close(void);
void rcs_lex_keyword_expected(void);

int format_rcs_gram_lex(void);
void format_rcs_gram_error(const char *);

#endif // AEIMPORT_FORMAT_RCS_LEX_H
