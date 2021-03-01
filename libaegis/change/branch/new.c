/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995-1999, 2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate change branches
 */

#include <change/branch.h>
#include <error.h>


void
change_branch_new(change_ty *cp)
{
	cstate		cstate_data;

	cstate_data = change_cstate_get(cp);
	assert(!cstate_data->branch);
	cstate_data->cause = change_cause_internal_enhancement;
	cstate_data->branch = cstate_branch_type.alloc();
	cstate_data->branch->change = cstate_branch_change_list_type.alloc();
}
