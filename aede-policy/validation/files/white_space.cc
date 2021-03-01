//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <common/error.h> // for assert
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/white_space.h>


validation_files_white_space::~validation_files_white_space()
{
}


validation_files_white_space::validation_files_white_space()
{
}


bool
validation_files_white_space::check_branches()
    const
{
    return false;
}


bool
validation_files_white_space::check_downloaded()
    const
{
    return false;
}


bool
validation_files_white_space::check_foreign_copyright()
    const
{
    return false;
}


bool
validation_files_white_space::check_binaries()
    const
{
    return false;
}


bool
validation_files_white_space::check(change::pointer cp, fstate_src_ty *src)
{
    nstring path(change_file_path(cp, src));
    assert(!path.empty());
    if (path.empty())
        return true;

    os_become_orig();
    int num_blank_lines = 0;
    bool ok = true;
    input ip = input_file_text_open(path);
    for (;;)
    {
        nstring line;
        if (!ip->one_line(line))
            break;

        //
        // check for white space on the end of the line
        //
        const char *dp = line.c_str();
        const char *ep = dp + line.size();
        if (dp < ep && isspace((unsigned char)ep[-1]))
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", ip->name());
            change_error
            (
                cp,
                &sc,
                i18n("$filename: white space at end of line")
            );
            ok = false;
        }

        //
        // check for a blank line
        //
        while (dp < ep)
        {
            if (!isspace((unsigned char)*dp))
                break;
            ++dp;
        }
        if (dp == ep)
            ++num_blank_lines;
        else
            num_blank_lines = 0;
    }
    ip.close();
    os_become_undo();
    if (num_blank_lines)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", src->file_name);
        sc.var_set_long("Number", num_blank_lines);
        sc.var_optional("Number");
        change_error(cp, &sc, i18n("$filename: blank lines at end of file"));
        ok = false;
    }
    return ok;
}
