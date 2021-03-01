//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate contexts
//

#include <mem.h>
#include <patch/context.h>


patch_context_ty *
patch_context_new(input_ty *ip)
{
	patch_context_ty *this_thing;

	this_thing = (patch_context_ty *)mem_alloc(sizeof(patch_context_ty));
	this_thing->input = ip;
	string_list_constructor(&this_thing->buffer);
	return this_thing;
}


void
patch_context_delete(patch_context_ty *this_thing)
{
	string_list_destructor(&this_thing->buffer);
	this_thing->input = 0; // don't delete it!
}


string_ty *
patch_context_getline(patch_context_ty *this_thing, int n)
{
	string_ty	*s;

	if (n < 0)
		return 0;
	while (n >= (int)this_thing->buffer.nstrings)
	{
		s = input_one_line(this_thing->input);
		if (!s)
			return 0;
		string_list_append(&this_thing->buffer, s);
		str_free(s);
	}
	return this_thing->buffer.string[n];
}


void
patch_context_discard(patch_context_ty *this_thing, int n)
{
	size_t		j;

	if (n <= 0)
		return;
	for (j = 0; j < (size_t)n && j < this_thing->buffer.nstrings; ++j)
		str_free(this_thing->buffer.string[j]);
	if (n >= (int)this_thing->buffer.nstrings)
		this_thing->buffer.nstrings = 0;
	else
	{
		for (j = n; j < this_thing->buffer.nstrings; ++j)
			this_thing->buffer.string[j - n] =
                            this_thing->buffer.string[j];
		this_thing->buffer.nstrings -= n;
	}
}
