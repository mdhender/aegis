/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: interface definition for aegis/parse.c
 */

#ifndef PARSE_H
#define PARSE_H

#include <main.h>
#include <str.h>
#include <type.h>

typedef struct sem_ty sem_ty;
struct sem_ty
{
	type_ty	*type;
	sem_ty	*next;
	void	*addr;
};

void sem_push _((type_ty *, void *));
void sem_pop _((void));
void sem_integer _((long));
void sem_string _((string_ty *));
void sem_enum _((string_ty *));
void sem_list _((void));
void sem_field _((string_ty *));

void *parse _((char *, type_ty *));

#endif /* PARSE_H */
