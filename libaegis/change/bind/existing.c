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
 * MANIFEST: functions to manipulate existings
 */

#include <change.h>
#include <error.h> /* for assert */
#include <project.h>
#include <sub.h>
#include <trace.h>


void
change_bind_existing(change_ty *cp)
{
	change_ty	*pcp;
	cstate		pcsp;
	cstate_branch_change_list lp;
	size_t		j;

	/*
	 * verify the change number given on the command line
	 */
	trace(("change_bind_existing(cp = %08lX)\n{\n"/*}*/, (long)cp));
	assert(cp->reference_count >= 1);
	if (cp->number == TRUNK_CHANGE_NUMBER)
	{
		trace((/*{*/"}\n"));
		return;
	}
	pcp = project_change_get(cp->pp);
	pcsp = change_cstate_get(pcp);
	if (!pcsp->branch)
	{
		assert(0);
		pcsp->branch = cstate_branch_type.alloc();
	}
	if (!pcsp->branch->change)
		pcsp->branch->change = cstate_branch_change_list_type.alloc();
	lp = pcsp->branch->change;
	for (j = 0; j < lp->length; ++j)
	{
		if (lp->list[j] == cp->number)
			break;
	}
	if (j >= lp->length)
		change_fatal(cp, 0, i18n("unknown change"));
	trace((/*{*/"}\n"));
}
