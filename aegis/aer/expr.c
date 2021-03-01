/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995 Peter Miller;
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
 * MANIFEST: functions to manipulate expressions
 */

#include <ac/stdarg.h>

#include <aer/expr.h>
#include <aer/lex.h>
#include <error.h>
#include <mem.h>
#include <str.h>
#include <trace.h>


rpt_expr_ty *
rpt_expr_alloc(method)
	rpt_expr_method_ty *method;
{
	rpt_expr_ty	*this;

	trace(("rpt_expr_alloc(%08lX)\n{\n"/*}*/, (long)method));
	trace(("method is \"%s\"\n", method->name));
	this = mem_alloc(method->size);
	this->method = method;
	this->reference_count = 1;
	this->pos = 0;
	this->child = 0;
	this->nchild = 0;
	this->nchild_max = 0;
	if (method->construct)
		method->construct(this);
	trace(("return %08lX;\n", (long)this));
	trace((/*{*/"}\n"));
	return this;
}


rpt_expr_ty *
rpt_expr_copy(this)
	rpt_expr_ty	*this;
{
	this->reference_count++;
	return this;
}


void
rpt_expr_free(this)
	rpt_expr_ty	*this;
{
	size_t		j;

	this->reference_count--;
	if (this->reference_count > 0)
		return;
	trace(("rpt_expr_free(%08lX)\n{\n"/*}*/, (long)this));
	trace(("method is \"%s\"\n", this->method->name));
	assert(this->reference_count == 0);
	if (this->method->destruct)
		this->method->destruct(this);
	for (j = 0; j < this->nchild; ++j)
		rpt_expr_free(this->child[j]);
	if (this->child)
		mem_free(this->child);
	if (this->pos)
		rpt_lex_pos_free(this->pos);
	mem_free(this);
	trace((/*{*/"}\n"));
}


void
rpt_expr_append(parent, child)
	rpt_expr_ty	*parent;
	rpt_expr_ty	*child;
{
	size_t		nbytes;

	trace(("rpt_expr_append(parent = %08lX, child = %08lX)\n{\n"/*}*/, (long)parent, (long)child));
	trace(("parent method is \"%s\"\n", parent->method->name));
	trace(("child method is \"%s\"\n", child->method->name));
	if (parent->nchild >= parent->nchild_max)
	{
		parent->nchild_max = parent->nchild_max * 2 + 4;
		nbytes = parent->nchild_max * sizeof(parent->child[0]);
		parent->child = mem_change_size(parent->child, nbytes);
	}
	parent->child[parent->nchild++] = rpt_expr_copy(child);

	/*
	 * update the parent's position
	 */
	assert(child->pos);
	trace(("child->pos = \"%s\" %ld;\n", child->pos->file_name->str_text, child->pos->line_number));
	if (!parent->pos)
		parent->pos = rpt_lex_pos_copy(child->pos);
	else
	{
		rpt_lex_pos_ty	*p;

		p = parent->pos;
		parent->pos = rpt_lex_pos_union(p, child->pos);
		rpt_lex_pos_free(p);
	}
	trace((/*{*/"}\n"));
}


void
rpt_expr_prepend(parent, child)
	rpt_expr_ty	*parent;
	rpt_expr_ty	*child;
{
	size_t		nbytes;
	size_t		j;

	trace(("rpt_expr_prepend(parent = %08lX, child = %08lX)\n{\n"/*}*/, (long)parent, (long)child));
	trace(("parent method is \"%s\"\n", parent->method->name));
	trace(("child method is \"%s\"\n", child->method->name));
	if (parent->nchild >= parent->nchild_max)
	{
		parent->nchild_max = parent->nchild_max * 2 + 4;
		nbytes = parent->nchild_max * sizeof(parent->child[0]);
		parent->child = mem_change_size(parent->child, nbytes);
	}
	for (j = parent->nchild; j > 0; --j)
		parent->child[j] = parent->child[j - 1];
	parent->nchild++;
	parent->child[0] = rpt_expr_copy(child);

	/*
	 * update the parent's position
	 */
	assert(child->pos);
	if (!parent->pos)
		parent->pos = rpt_lex_pos_copy(child->pos);
	else
	{
		rpt_lex_pos_ty	*p;

		p = parent->pos;
		parent->pos = rpt_lex_pos_union(p, child->pos);
		rpt_lex_pos_free(p);
	}
	trace((/*{*/"}\n"));
}


void
rpt_expr_error(ep, fmt sva_last)
	rpt_expr_ty	*ep;
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	string_ty	*s;

	sva_init(ap, fmt);
	s = str_vformat(fmt, ap);
	va_end(ap);

	assert(ep->pos);
	rpt_lex_error(ep->pos, "%S", s);
	str_free(s);
}


int
rpt_expr_lvalue(ep)
	rpt_expr_ty	*ep;
{
	if (!ep->method->lvalue)
		return 0;
	return ep->method->lvalue(ep);
}
