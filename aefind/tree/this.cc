//
// aegis - project change supervisor
// Copyright (C) 1997, 2002, 2004-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
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
#include <common/ac/stdio.h>

#include <common/str.h>
#include <common/trace.h>
#include <libaegis/aer/value/string.h>

#include <aefind/tree/this.h>


tree_this::~tree_this()
{
}


tree_this::tree_this::tree_this(int arg) :
    resolved(arg)
{
}


tree::pointer
tree_this::create(int arg)
{
    return pointer(new tree_this(arg));
}


void
tree_this::print()
    const
{
    printf("%s", name());
}


rpt_value::pointer
tree_this::evaluate(string_ty *unresolved_path, string_ty *path,
    string_ty *resolved_path, struct stat *) const
{
    trace(("tree_this::evaluate\n"));
    assert(path);
    if (resolved == 0)
        return rpt_value_string::create(nstring(unresolved_path));
    if (resolved > 0)
        return rpt_value_string::create(nstring(resolved_path));
    return rpt_value_string::create(nstring(path));
}


bool
tree_this::useful()
    const
{
    trace(("tree::this::useful\n"));
    return false;
}


bool
tree_this::constant()
    const
{
    trace(("tree::this::constant\n"));
    return false;
}


const char *
tree_this::name()
    const
{
    if (resolved == 0)
        return "{-}";
    if (resolved > 0)
        return "{+}";
    return "{}";
}


tree::pointer
tree_this::optimize()
    const
{
    return pointer(new tree_this(resolved));
}


// vim: set ts=8 sw=4 et :
