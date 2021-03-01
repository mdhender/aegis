//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2002, 2005, 2006 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface definition for aegis/aer/value/cstate.c
//

#ifndef AEGIS_AER_VALUE_CSTATE_H
#define AEGIS_AER_VALUE_CSTATE_H

#include <libaegis/aer/value.h>

struct project_ty;

rpt_value_ty *rpt_value_cstate(struct project_ty *, long, long *);

#endif // AEGIS_AER_VALUE_CSTATE_H
