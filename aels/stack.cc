//
// aegis - project change supervisor
// Copyright (C) 2001, 2002, 2004-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/nstring/list.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>

#include <aels/stack.h>


static nstring_list stack;


nstring
stack_relative(const nstring &fn)
{
    assert(!stack.empty());
    os_become_orig();
    nstring s1 = os_pathname(fn, true);
    os_become_undo();

    for (size_t k = 0; k < stack.size(); ++k)
    {
        nstring s2 = os_below_dir(stack[k], s1);
        if (!s2.empty())
            return s2;
    }
    return "";
}


nstring
stack_nth(size_t n)
{
    assert(!stack.empty());
    if (n >= stack.size())
        return "";
    return stack[n];
}


void
stack_from_project(project *pp)
{
    pp->search_path_get(stack, true);

    // error if project is a completed branch...
}


void
stack_from_change(const change::pointer &cp)
{
    cp->search_path_get(stack, true);
}


// vim: set ts=8 sw=4 et :
