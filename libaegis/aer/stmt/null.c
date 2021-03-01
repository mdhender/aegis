/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996 Peter Miller;
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
 * MANIFEST: functions to manipulate null statements
 */

#include <aer/stmt/null.h>


static void run _((rpt_stmt_ty *, rpt_stmt_result_ty *));

static void
run(sp, rp)
	rpt_stmt_ty	*sp;
	rpt_stmt_result_ty *rp;
{
	rp->status = rpt_stmt_status_normal;
	rp->thrown = 0;
}


static rpt_stmt_method_ty method =
{
	sizeof(rpt_stmt_ty),
	"null",
	0,	/* construct */
	0,	/* destruct */
	run
};


rpt_stmt_ty *
rpt_stmt_null()
{
	return rpt_stmt_alloc(&method);
}
