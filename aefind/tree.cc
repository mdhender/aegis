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

#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/mem.h>
#include <common/str.h>
#include <libaegis/aer/value.h>

#include <aefind/tree/constant.h>


tree::~tree()
{
}


tree::tree()
{
}


rpt_value::pointer
tree::evaluate_constant()
    const
{
    string_ty *pathname = str_from_c("\377");
    struct stat st;
    memset(&st, 0, sizeof(st));
    rpt_value::pointer vp = evaluate(pathname, pathname, pathname, &st);
    str_free(pathname);
    return vp;
}


bool
tree::constant()
    const
{
    return false;
}


tree::pointer
tree::optimize_constant()
    const
{
    rpt_value::pointer vp = evaluate_constant();
    tree::pointer result = tree_constant::create(vp);
    return result;
}


// vim: set ts=8 sw=4 et :
