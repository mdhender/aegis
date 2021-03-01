/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1997 Peter Miller;
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
 * MANIFEST: interface definition for aegis/aenbr.c
 */

#ifndef AEGIS_AENBR_H
#define AEGIS_AENBR_H

#include <main.h>

struct project_ty;
struct string_ty;
struct user_ty;

struct project_ty *new_branch_internals _((struct user_ty *up,
	struct project_ty *pp, long cn, struct string_ty *dd));
void new_branch _((void));

#endif /* AEGIS_AENBR_H */
