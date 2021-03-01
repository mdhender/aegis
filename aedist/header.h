/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: interface definition for aedist/header.c
 */

#ifndef AEDIST_HEADER_H
#define AEDIST_HEADER_H

#include <main.h>

struct input_ty; /* existence */
struct string_ty; /* existence */
struct symtab_ty; /* existence */

typedef struct header_ty header_ty;
struct header_ty
{
	struct symtab_ty *stp;
};

header_ty *header_read _((struct input_ty *));
struct string_ty *header_query _((header_ty *, const char *));
void header_delete _((header_ty *));

#endif /* AEDIST_HEADER_H */
