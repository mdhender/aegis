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
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate statements
 */

#include <aer/stmt.h>
#include <error.h>
#include <mem.h>


rpt_stmt_ty *
rpt_stmt_alloc(method)
	rpt_stmt_method_ty *method;
{
	rpt_stmt_ty	*this;

	this = mem_alloc(method->size);
	this->method = method;
	this->reference_count = 1;
	this->child = 0;
	this->nchild = 0;
	this->nchild_max = 0;
	if (method->construct)
		method->construct(this);
	return this;
}


rpt_stmt_ty *
rpt_stmt_copy(this)
	rpt_stmt_ty	*this;
{
	this->reference_count++;
	return this;
}


void
rpt_stmt_free(this)
	rpt_stmt_ty	*this;
{
	size_t		j;

	this->reference_count--;
	if (this->reference_count > 0)
		return;
	assert(this->reference_count == 0);
	if (this->method->destruct)
		this->method->destruct(this);
	for (j = 0; j < this->nchild; ++j)
		rpt_stmt_free(this->child[j]);
	if (this->child)
		mem_free(this->child);
	mem_free(this);
}


void
rpt_stmt_append(parent, child)
	rpt_stmt_ty	*parent;
	rpt_stmt_ty	*child;
{
	size_t		nbytes;

	if (parent->nchild >= parent->nchild_max)
	{
		parent->nchild_max = parent->nchild_max * 2 + 4;
		nbytes = parent->nchild_max * sizeof(parent->child[0]);
		parent->child = mem_change_size(parent->child, nbytes);
	}
	parent->child[parent->nchild++] = rpt_stmt_copy(child);
}


void
rpt_stmt_prepend(parent, child)
	rpt_stmt_ty	*parent;
	rpt_stmt_ty	*child;
{
	size_t		nbytes;
	size_t		j;

	if (parent->nchild >= parent->nchild_max)
	{
		parent->nchild_max = parent->nchild_max * 2 + 4;
		nbytes = parent->nchild_max * sizeof(parent->child[0]);
		parent->child = mem_change_size(parent->child, nbytes);
	}
	for (j = parent->nchild; j > 0; --j)
		parent->child[j] = parent->child[j - 1];
	parent->nchild++;
	parent->child[0] = rpt_stmt_copy(child);
}
