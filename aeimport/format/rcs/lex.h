/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aeimport/format/rcs/lex.c
 */

#ifndef AEIMPORT_FORMAT_RCS_LEX_H
#define AEIMPORT_FORMAT_RCS_LEX_H

#include <str.h>

void rcs_lex_open(string_ty *);
void rcs_lex_close(void);
void rcs_lex_keyword_expected(void);

int format_rcs_gram_lex(void);
void format_rcs_gram_error(const char *);

#endif /* AEIMPORT_FORMAT_RCS_LEX_H */
