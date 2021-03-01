//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2003, 2004 Peter Miller.
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
// MANIFEST: functions to manipulate statements
//

#include <aer/stmt.h>
#include <error.h>
#include <mem.h>


rpt_stmt_ty *
rpt_stmt_alloc(rpt_stmt_method_ty *method)
{
	rpt_stmt_ty	*this_thing;

	this_thing = (rpt_stmt_ty *)mem_alloc(method->size);
	this_thing->method = method;
	this_thing->reference_count = 1;
	this_thing->child = 0;
	this_thing->nchild = 0;
	this_thing->nchild_max = 0;
	if (method->construct)
		method->construct(this_thing);
	return this_thing;
}


rpt_stmt_ty *
rpt_stmt_copy(rpt_stmt_ty *this_thing)
{
	this_thing->reference_count++;
	return this_thing;
}


void
rpt_stmt_free(rpt_stmt_ty *this_thing)
{
	size_t		j;

	this_thing->reference_count--;
	if (this_thing->reference_count > 0)
		return;
	assert(this_thing->reference_count == 0);
	if (this_thing->method->destruct)
		this_thing->method->destruct(this_thing);
	for (j = 0; j < this_thing->nchild; ++j)
		rpt_stmt_free(this_thing->child[j]);
	if (this_thing->child)
		mem_free(this_thing->child);
	mem_free(this_thing);
}


void
rpt_stmt_append(rpt_stmt_ty *parent, rpt_stmt_ty *child)
{
	size_t		nbytes;

	if (parent->nchild >= parent->nchild_max)
	{
		parent->nchild_max = parent->nchild_max * 2 + 4;
		nbytes = parent->nchild_max * sizeof(parent->child[0]);
		parent->child =
                    (rpt_stmt_ty **)mem_change_size(parent->child, nbytes);
	}
	parent->child[parent->nchild++] = rpt_stmt_copy(child);
}


void
rpt_stmt_prepend(rpt_stmt_ty *parent, rpt_stmt_ty *child)
{
	size_t		nbytes;
	size_t		j;

	if (parent->nchild >= parent->nchild_max)
	{
		parent->nchild_max = parent->nchild_max * 2 + 4;
		nbytes = parent->nchild_max * sizeof(parent->child[0]);
		parent->child =
                    (rpt_stmt_ty **)mem_change_size(parent->child, nbytes);
	}
	for (j = parent->nchild; j > 0; --j)
		parent->child[j] = parent->child[j - 1];
	parent->nchild++;
	parent->child[0] = rpt_stmt_copy(child);
}
