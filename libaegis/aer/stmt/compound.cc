//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate compound statements
//

#include <libaegis/aer/stmt/compound.h>
#include <common/trace.h>


static void
run(rpt_stmt_ty *this_thing, rpt_stmt_result_ty *rp)
{
	size_t		j;

	trace(("stmt_compound::run(this_thing = %08lX)\n{\n",
               (long)this_thing));
	for (j = 0; j < this_thing->nchild; ++j)
	{
		trace(("child %ld\n", (long)j));
		rpt_stmt_run(this_thing->child[j], rp);
		if (rp->status != rpt_stmt_status_normal)
		{
			trace(("return %d;\n", rp->status));
			trace(("}\n"));
			return;
		}
	}
	rp->status = rpt_stmt_status_normal;
	rp->thrown = 0;
	trace(("}\n"));
}


static rpt_stmt_method_ty method =
{
	sizeof(rpt_stmt_ty),
	"compound",
	0, // construct
	0, // destruct
	run
};


rpt_stmt_ty *
rpt_stmt_compound()
{
	return rpt_stmt_alloc(&method);
}
