//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/error.h>
#include <common/str_list.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/cstate.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>

#include <aels/stack.h>


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
stack_from_change(change::pointer cp)
{
    stack = new string_list_ty();
    change_search_path_get(cp, stack, 1);
}
