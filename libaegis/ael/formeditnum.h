/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/ael/formeditnum.c
 */

#ifndef LIBAEGIS_AEL_FORMEDITNUM_H
#define LIBAEGIS_AEL_FORMEDITNUM_H

#include <fstate.h>

struct output_ty; /* existence */

void list_format_edit_number(struct output_ty *, fstate_src_ty *);

#endif /* LIBAEGIS_AEL_FORMEDITNUM_H */
