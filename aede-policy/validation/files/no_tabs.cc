//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008 Peter Miller
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

#include <common/error.h> // for assert
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/cstate.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/no_tabs.h>


validation_files_no_tabs::~validation_files_no_tabs()
{
}


validation_files_no_tabs::validation_files_no_tabs()
{
}


bool
validation_files_no_tabs::check_branches()
    const
{
    return false;
}


bool
validation_files_no_tabs::check_downloaded()
    const
{
    return false;
}


bool
validation_files_no_tabs::check_foreign_copyright()
    const
{
    return false;
}


bool
validation_files_no_tabs::check_binaries()
    const
{
    return false;
}


bool
validation_files_no_tabs::check(change::pointer cp, fstate_src_ty *src)
{
    //
    // Don't perform this check for files marked as being allowed to
    // have tabs.
    //
    if
    (
        attributes_list_find_boolean
        (
            src->attribute,
            "aede-policy-tabs-allowed"
        )
    )
        return true;

    //
    // Some files must have tabs,
    // that's what the idiot format requires.
    //
    nstring t(src->file_name);
    if (t.basename().downcase().starts_with("makefile"))
        return true;

    string_ty *path = change_file_path(cp, src);
    assert(path);
    if (!path)
        return true;

    os_become_orig();
    bool ok = true;
    input ip = input_file_text_open(path);
    for (;;)
    {
        int c = ip->getch();
        if (c < 0)
            break;
        if (c == 0)
        {
            // binary files are exempt from the check.
            break;
        }
        if (c == '\t')
        {
            ok = false;
            break;
        }
    }
    ip.close();
    os_become_undo();
    if (!ok)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", src->file_name);
        change_error(cp, &sc, i18n("$filename: contains tab characters"));
    }
    return ok;
}
