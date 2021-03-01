//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <common/nstring/list.h>
#include <common/trace.h>
#include <common/wstring/list.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/common.fmtgen.h>
#include <libaegis/sub.h>
#include <libaegis/sub/change/files.h>


//
// NAME
//      sub_change_files - the change_files substitution
//
// SYNOPSIS
//      string_ty *sub_change_files(wstring_list_ty *arg);
//
// DESCRIPTION
//      The sub_change_files function implements the
//      change_files substitution.  The change_files
//      substitution is used to insert the absolute path of the
//      change_files.
//
// ARGUMENTS
//      arg     - list of arguments, including the function name as [0]
//
// RETURNS
//      a pointer to a string in dynamic memory;
//      or NULL on error, setting suberr appropriately.
//

wstring
sub_change_files(sub_context_ty *scp, const wstring_list &arg)
{
    trace(("sub_change_files()\n{\n"));
    wstring result;
    change::pointer cp = sub_context_change_get(scp);
    if (!cp)
    {
        scp->error_set(i18n("not valid in current context"));
        trace(("}\n"));
        return result;
    }

    //
    // See if we recognize any of these words.
    //
    bool not_var = false;
    bool quote = false;
    unsigned action_mask = 0;
    unsigned usage_mask = 0;
    for (size_t n = 1; n < arg.size(); ++n)
    {
        bool ok = false;
        nstring s = arg[n].to_nstring();
        for (unsigned k = 0; k < file_action_max; ++k)
        {
            if (0 == strcmp(s.c_str(), file_action_ename((file_action_ty)k)))
            {
                action_mask |= 1 << k;
                ok = true;
            }
        }
        for (size_t j = 0; j < file_usage_max; ++j)
        {
            if (0 == strcmp(s.c_str(), file_usage_ename((file_usage_ty)j)))
            {
                usage_mask |= 1 << j;
                ok = true;
            }
        }
        if (0 == strcmp(s.c_str(), "not") || 0 == strcmp(s.c_str(), "!"))
        {
            not_var = !not_var;
            ok = true;
        }
        if (0 == strcmp(s.c_str(), "quote"))
        {
            quote = true;
            ok = true;
        }
        if (!ok)
        {
            scp->error_set(i18n("file qualifier unknown"));
            trace(("}\n"));
            return result;
        }
    }

    //
    // Cope with some boundary conditions.
    //
    if (not_var)
    {
        action_mask = ~action_mask;
        usage_mask = ~usage_mask;
    }
    if (action_mask == 0)
        action_mask = ~0U;
    if (usage_mask == 0)
        usage_mask = ~0U;

    //
    // Look for files matching what they asked for.
    //
    nstring_list sl;
    for (size_t m = 0; ; ++m)
    {
        fstate_src_ty *src = change_file_nth(cp, m, view_path_first);
        if (!src)
            break;
        if
        (
            (action_mask & (1 << src->action))
        &&
            (usage_mask & (1 << src->usage))
        )
        {
            nstring fn(src->file_name);
            if (quote)
                fn = fn.quote_shell();
            sl.push_back(fn);
        }
    }

    //
    // Turn it into a space-separated string.
    //
    result = wstring(sl.unsplit());
    trace(("return %p;\n", result.get_ref()));
    trace(("}\n"));
    return result;
}


// vim: set ts=8 sw=4 et :
