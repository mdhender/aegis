/*
 *	aegis - project change supervisor
 *	Copyright (C) 2000-2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/change/attributes.c
 */

#ifndef LIBAEGIS_CHANGE_ATTRIBUTES_H
#define LIBAEGIS_CHANGE_ATTRIBUTES_H

#include <main.h>

struct cattr;
struct cstate;
struct pconf;
struct project_ty;

void change_attributes_default(struct cattr *, struct project_ty *,
	struct pconf *);
void change_attributes_copy(struct cattr *, struct cstate *);
void change_attributes_verify(string_ty *, struct cattr *);
void change_attributes_edit(struct cattr **, int);

#endif /* LIBAEGIS_CHANGE_ATTRIBUTES_H */
