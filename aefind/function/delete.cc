//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/errno.h>
#include <common/ac/unistd.h>

#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aefind/function/needs.h>
#include <aefind/function/delete.h>
#include <aefind/tree/list.h>
#include <aefind/tree/monadic.h>


tree_delete::~tree_delete()
{
}


tree_delete::tree_delete(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_delete::create(const tree::pointer &a_arg)
{
    return pointer(new tree_delete(a_arg));
}


tree::pointer
tree_delete::create_l(const tree_list &args)
{
    function_needs_one("delete", args);
    return create(args[0]);
}


rpt_value::pointer
tree_delete::evaluate(string_ty *path1, string_ty *path2, string_ty *path3,
    struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path1, path2, path3, st);
    rpt_value::pointer svp = rpt_value::stringize(vp);

    rpt_value_string *ss = dynamic_cast<rpt_value_string *>(svp.get());
    if (!ss)
    {
        // FIXME: shouldn't this be an error?
        return rpt_value_boolean::create(true);
    }


    {
        os_become_orig();
        bool ok = (unlink(ss->query().c_str()) >= 0);
        int err = errno;
        os_become_undo();
        if (!ok)
        {
            if (err != ENOENT)
            {
                sub_context_ty sc(__FILE__, __LINE__);
                sc.var_set_string("File_Name", ss->query());
                sc.error_intl("unlink $filename: $errno");
                sc.errno_setx(err);
                // FIXME: shouldn't this be an error?
            }
        }
    }

    return rpt_value_boolean::create(true);
}


tree::pointer
tree_delete::optimize(void)
    const
{
    return create(get_arg()->optimize());
}


bool
tree_delete::useful(void)
    const
{
    return true;
}


bool
tree_delete::constant(void)
    const
{
    return false;
}


const char *
tree_delete::name(void)
    const
{
    return "delete";
}


// vim: set ts=8 sw=4 et :
