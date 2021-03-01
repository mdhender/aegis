//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate stacks
//

#include <change.h>
#include <change/file.h>
#include <cstate.h>
#include <error.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <stack.h>
#include <str_list.h>
#include <sub.h>


static	string_list_ty	*stack;


string_ty *
stack_relative(string_ty *fn)
{
    string_ty       *s1;
    string_ty       *s2;
    size_t	    k;

    assert(stack);
    os_become_orig();
    s1 = os_pathname(fn, 1);
    os_become_undo();

    s2 = 0;
    for (k = 0; k < stack->nstrings; ++k)
    {
	s2 = os_below_dir(stack->string[k], s1);
	if (s2)
    	    break;
    }
    str_free(s1);

    if (!s2)
	return 0;

    if (s2->str_length == 0)
    {
	str_free(s2);
	s2 = str_from_c(".");
    }

    return s2;
}


string_ty *
stack_nth(size_t n)
{
    assert(n >= 0);
    assert(stack);
    assert(stack->nstrings);
    if (!stack)
	return 0;
    if (n >= stack->nstrings)
	return 0;
    return stack->string[n];
}


void
stack_from_project(project_ty *pp)
{
    stack = new string_list_ty();
    project_search_path_get(pp, stack, 1);

    // error if project is a completed branch...
}


void
stack_from_change(change_ty *cp)
{
    stack = new string_list_ty();
    change_search_path_get(cp, stack, 1);
}
