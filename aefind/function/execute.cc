//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/trace.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/os.h>

#include <aefind/function/execute.h>
#include <aefind/function/needs.h>
#include <aefind/tree/list.h>
#include <aefind/tree/monadic.h>


tree_execute::~tree_execute()
{
}


tree_execute::tree_execute(const pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_execute::create(const pointer &a_arg)
{
    return pointer(new tree_execute(a_arg));
}


tree::pointer
tree_execute::create_l(const tree_list &args)
{
    trace(("tree_execute::create\n"));
    function_needs_one("execute", args);
    return create(args[0]);
}


rpt_value::pointer
tree_execute::evaluate(string_ty *path1, string_ty *path2, string_ty *path3,
    struct stat *st) const
{
    trace(("tree::execute::evaluate\n"));
    rpt_value::pointer vp = get_arg()->evaluate(path1, path2, path3, st);
    rpt_value::pointer svp = rpt_value::stringize(vp);

    rpt_value_string *ss = dynamic_cast<rpt_value_string *>(svp.get());
    if (!ss)
    {
        // FIXME: shouldn't thi be an error?
        return rpt_value_boolean::create(true);
    }
    nstring cmd(ss->query());

    os_become_orig();
    int exit_status = os_execute_retcode(cmd, OS_EXEC_FLAG_SILENT, 0);
    os_become_undo();
    return rpt_value_boolean::create(exit_status == 0);
}


tree::pointer
tree_execute::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


bool
tree_execute::useful()
    const
{
    trace(("tree::execute::useful\n"));
    return true;
}


const char *
tree_execute::name()
    const
{
    return "execute";
}
