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
 * MANIFEST: functions to manipulate becomes
 */

#include <change.h>
#include <error.h> /* for assert */
#include <project.h>
#include <trace.h>


void
change_become(cp)
	change_ty	*cp;
{
	trace(("change_become(cp = %8.8lX)\n{\n"/*}*/, cp));
	assert(cp->reference_count >= 1);
	project_become(cp->pp);
	trace((/*{*/"}\n"));
}


void
change_become_undo()
{
	trace(("change_become_undo()\n{\n"/*}*/));
	project_become_undo();
	trace((/*{*/"}\n"));
}
