//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the sub_diversion_stack class
//

#include <common/error.h> // for assert
#include <libaegis/sub/diversion/stack.h>


sub_diversion_stack::~sub_diversion_stack()
{
    delete [] stack;
}


sub_diversion_stack::sub_diversion_stack() :
    top(0),
    max(0),
    stack(0)
{
}


void
sub_diversion_stack::push_back(wstring_ty *s, bool resub)
{
    if (top >= max)
    {
	size_t new_max = max * 2 + 4;
	sub_diversion *new_stack = new sub_diversion[new_max];
	for (size_t j = 0; j < top; ++j)
	    new_stack[j] = stack[j];
	delete [] stack;
	max = new_max;
	stack = new_stack;
    }
    stack[top++] = sub_diversion(s, resub);
}


void
sub_diversion_stack::pop_back()
{
    if (top > 0)
	--top;
}


bool
sub_diversion_stack::resub_both()
    const
{
    return (top > 0 && stack[top - 1].resub_both());
}


wchar_t
sub_diversion_stack::getch()
{
    if (top == 0)
	return 0;
    return stack[top - 1].getch();
}


void
sub_diversion_stack::ungetch(wchar_t c)
{
    assert(top);
    if (top)
	stack[top - 1].ungetch(c);
}
