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
 * MANIFEST: functions to manipulate outstandings
 */

#include <ael/change/by_state.h>
#include <ael/change/inappropriat.h>
#include <ael/change/outstanding.h>
#include <cstate.h>
#include <str.h>
#include <trace.h>


void
list_outstanding_changes(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	trace(("list_outstanding_changes()\n{\n"));
	if (change_number)
		list_change_inappropriate();
	list_changes_in_state_mask
	(
		project_name,
		~(1 << cstate_state_completed)
	);
	trace(("}\n"));
}
