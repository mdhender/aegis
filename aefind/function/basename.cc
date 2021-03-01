//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/str.h>
#include <libaegis/aer/value/string.h>

#include <aefind/function/basename.h>
#include <aefind/function/needs.h>
#include <aefind/tree/list.h>
#include <aefind/tree/monadic.h>


tree_basename::~tree_basename()
{
}


tree_basename::tree_basename(const pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_basename::create(const pointer &a_arg)
{
    return pointer(new tree_basename(a_arg));
}


tree::pointer
tree_basename::create_l(const tree_list &args)
{
    function_needs_one("execute", args);
    return create(args[0]);
}


rpt_value::pointer
tree_basename::evaluate(string_ty *path1, string_ty *path2, string_ty *path3,
    struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path1, path2, path3, st);
    rpt_value::pointer svp = rpt_value::stringize(vp);

    rpt_value_string *ss = dynamic_cast<rpt_value_string *>(svp.get());
    if (!ss)
    {
        // FIXME:: shouldn't this be an error?
        return rpt_value_string::create("");
    }

    nstring s(ss->query());
    rpt_value::pointer result = rpt_value_string::create(s.basename());
    return result;
}


tree::pointer
tree_basename::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_basename::name()
    const
{
    return "basename";
}
