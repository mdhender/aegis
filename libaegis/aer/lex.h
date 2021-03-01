//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 2002, 2003, 2005-2008 Peter Miller
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

#ifndef AEGIS_AER_LEX_H
#define AEGIS_AER_LEX_H

#include <libaegis/aer/pos.h>

class sub_context_ty; // forward

void rpt_lex_open(struct string_ty *);
void rpt_lex_close(void);

int aer_report_lex(void);

void aer_report_error(const char *);
void rpt_lex_error(const rpt_position::pointer &, const char *);
void aer_lex_error(sub_context_ty &, const rpt_position::pointer &,
    const char *);
void aer_lex_error(sub_context_ty &, const char *);

rpt_position::pointer rpt_lex_pos_get(void);

#endif // AEGIS_AER_LEX_H
