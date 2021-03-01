//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2010, 2012 Peter Miller
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

#include <common/nstring.h>
#include <libaegis/change/file.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/merge-fhist.h>


validation_files_merge_fhist::~validation_files_merge_fhist()
{
}


validation_files_merge_fhist::validation_files_merge_fhist()
{
}


validation::pointer
validation_files_merge_fhist::create(void)
{
    return pointer(new validation_files_merge_fhist());
}


bool
validation_files_merge_fhist::check_binaries()
    const
{
    return false;
}


bool
validation_files_merge_fhist::check(change::pointer cp, fstate_src_ty *src)
{
    nstring bad_juju = "/-/-/-/-/-/-/-/-/-/";

    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;

    os_become_orig();
    input ip = input_file_text_open(path);
    bool ok = true;
    int line_number = 0;
    for (;;)
    {
        nstring line;
        if (!ip->one_line(line))
            break;
        ++line_number;
        if (line.starts_with(bad_juju))
        {
            sub_context_ty sc;
            sc.var_set_format
            (
                "File_Name",
                "%s: %d",
                src->file_name->str_text,
                line_number
            );
            change_error(cp, &sc, i18n("$filename: merge conflicts"));
            ok = false;
            break;
        }
    }
    ip.close();
    os_become_undo();
    return ok;
}


// vim: set ts=8 sw=4 et :
