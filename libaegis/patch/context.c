/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate contexts
 */

#include <mem.h>
#include <patch/context.h>


patch_context_ty *
patch_context_new(ip)
	input_ty	*ip;
{
	patch_context_ty *this;

	this = mem_alloc(sizeof(patch_context_ty));
	this->input = ip;
	string_list_constructor(&this->buffer);
	return this;
}


void
patch_context_delete(this)
	patch_context_ty *this;
{
	string_list_destructor(&this->buffer);
	this->input = 0; /* don't delete it! */
}


string_ty *
patch_context_getline(this, n)
	patch_context_ty *this;
	int		n;
{
	string_ty	*s;

	if (n < 0)
		return 0;
	while (n >= this->buffer.nstrings)
	{
		s = input_one_line(this->input);
		if (!s)
			return 0;
		string_list_append(&this->buffer, s);
		str_free(s);
	}
	return this->buffer.string[n];
}


void
patch_context_discard(this, n)
	patch_context_ty *this;
	int		n;
{
	size_t		j;

	if (n <= 0)
		return;
	for (j = 0; j < n && j < this->buffer.nstrings; ++j)
		str_free(this->buffer.string[j]);
	if (n >= this->buffer.nstrings)
		this->buffer.nstrings = 0;
	else
	{
		for (j = n; j < this->buffer.nstrings; ++j)
			this->buffer.string[j - n] = this->buffer.string[j];
		this->buffer.nstrings -= n;
	}
}
