/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 * MANIFEST: interface definition for aegis/aer/value/struct.c
 */

#ifndef AEGIS_AER_VALUE_STRUCT_H
#define AEGIS_AER_VALUE_STRUCT_H

#include <aer/value.h>

struct symtab_ty;
struct string_ty;

rpt_value_ty *rpt_value_struct _((struct symtab_ty *));
struct symtab_ty *rpt_value_struct_query _((rpt_value_ty *));
rpt_value_ty *rpt_value_struct_lookup _((rpt_value_ty *, struct string_ty *));
void rpt_value_struct__set _((rpt_value_ty *, struct string_ty *,
	rpt_value_ty *));

#endif /* AEGIS_AER_VALUE_STRUCT_H */
