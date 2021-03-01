/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1995 Peter Miller;
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
 * MANIFEST: interface definition for common/arglex.c
 */

#ifndef ARGLEX_H
#define ARGLEX_H

#include <main.h>

#define ARGLEX_END_MARKER { (char *)0, (arglex_token_ty)0, }

enum arglex_token_ty
{
	arglex_token_eoln = -20,
	arglex_token_help,
	arglex_token_number,
	arglex_token_option,
	arglex_token_stdio,
	arglex_token_string,
	arglex_token_trace,
	arglex_token_version
};
typedef enum arglex_token_ty arglex_token_ty;

typedef struct arglex_table_ty arglex_table_ty;
struct arglex_table_ty
{
	char		*t_name;
	arglex_token_ty	t_token;
};

typedef struct arglex_value_ty arglex_value_ty;
struct arglex_value_ty
{
	char	*alv_string;
	long	alv_number;
};

extern arglex_token_ty arglex_token;
extern arglex_value_ty arglex_value;

void arglex_init _((int, char **, arglex_table_ty *));
arglex_token_ty arglex _((void));
int arglex_compare _((char *formal, char *actual));
char *arglex_token_name _((arglex_token_ty));

arglex_table_ty *arglex_table_catenate _((arglex_table_ty *,
	arglex_table_ty *));

#endif /* ARGLEX_H */
