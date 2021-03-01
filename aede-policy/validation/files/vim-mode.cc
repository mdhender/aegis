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

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <libaegis/attribute.h>
#include <libaegis/change/file.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/vim-mode.h>


validation_files_vim_mode::~validation_files_vim_mode()
{
}


validation_files_vim_mode::validation_files_vim_mode()
{
}


validation::pointer
validation_files_vim_mode::create(void)
{
    return pointer(new validation_files_vim_mode());
}


bool
validation_files_vim_mode::check(change::pointer cp, fstate_src_ty *src)
{
    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;

    //
    // Don't perform this check for files marked as having a vim-mode
    // exception.
    //
    bool required =
        attributes_list_find_boolean
        (
            src->attribute,
            "aede-policy-vim-mode-required",
            true
        );
    if (!required)
        return true;

    os_become_orig();
    bool vim_mode_seen = false;
    input ip = input_file_open(path);
    for (;;)
    {
        nstring line;
        if (!ip->one_line(line))
            break;
        // don't match self
        const char *v = strstr(line.c_str(), "vi" "m: set ");
        if (v)
        {
            v += 8;
            if (strstr(v, " :"))
            {
                vim_mode_seen = true;
                break;
            }
        }
    }
    ip.close();
    os_become_undo();
    if (!vim_mode_seen)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", src->file_name);
        change_error
        (
            cp,
            &sc,
            i18n("$filename: does not contain a vim mode line")
        );
    }
    return vim_mode_seen;
}


bool
validation_files_vim_mode::check_binaries(void)
    const
{
    return false;
}


bool
validation_files_vim_mode::check_branches(void)
    const
{
    return false;
}


bool
validation_files_vim_mode::check_downloaded(void)
    const
{
    return false;
}


bool
validation_files_vim_mode::check_foreign_copyright(void)
    const
{
    return false;
}


// vim: set ts=8 sw=4 et :
