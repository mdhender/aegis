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
 * MANIFEST: functions to manipulate compound statements
 */

#include <aer/stmt/compound.h>
#include <trace.h>


static rpt_stmt_status_ty run _((rpt_stmt_ty *));

static rpt_stmt_status_ty
run(this)
	rpt_stmt_ty	*this;
{
	size_t		j;
	rpt_stmt_status_ty result;

	trace(("stmt_compound::run(this = %08lX)\n{\n"/*}*/, (long)this));
	for (j = 0; j < this->nchild; ++j)
	{
		trace(("child %ld\n", (long)j));
		result = rpt_stmt_run(this->child[j]);
		if (result != rpt_stmt_status_normal)
		{
			trace(("return %d;\n", result));
			trace((/*{*/"}\n"));
			return result;
		}
	}
	trace(("return normal;\n"));
	trace((/*{*/"}\n"));
	return rpt_stmt_status_normal;
}


static rpt_stmt_method_ty method =
{
	sizeof(rpt_stmt_ty),
	"compound",
	0, /* construct */
	0, /* destruct */
	run
};


rpt_stmt_ty *
rpt_stmt_compound()
{
	return rpt_stmt_alloc(&method);
}
