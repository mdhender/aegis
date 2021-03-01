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

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/gmatch.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/cstate.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/line_length.h>


validation_files_line_length::~validation_files_line_length()
{
}


validation_files_line_length::validation_files_line_length(int linlen) :
    line_length(linlen > 0 ? linlen : 80)
{
}


bool
validation_files_line_length::check_branches()
    const
{
    return false;
}


bool
validation_files_line_length::check_downloaded()
    const
{
    return false;
}


bool
validation_files_line_length::check_foreign_copyright()
    const
{
    return false;
}


bool
validation_files_line_length::check_binaries()
    const
{
    return false;
}


static int
calc_line_length(const nstring &text)
{
    const char *cp = text.c_str();
    const char *ep = cp + text.size();
    int column = 0;
    while (cp < ep)
    {
        unsigned char c = *cp++;
        if (c == '\t')
            column = (column + 8) & ~7;
        else
            ++column;
    }
    return column;
}


bool
validation_files_line_length::check(change::pointer cp, fstate_src_ty *src)
{
    //
    // Tests have notoriously long lines.
    //
    if (gmatch("test/*/*.sh", src->file_name->str_text))
        return true;

    //
    // Don't perform this check for files marked as being allowed to
    // have really long lines.
    //
    long max_lin_len =
        attributes_list_find_integer
        (
            src->attribute,
            "aede-policy-line-length",
            line_length
        );
    if (max_lin_len <= 0)
        return true;

    nstring path(change_file_path(cp, src));
    assert(!path.empty());
    if (path.empty())
        return true;
    os_become_orig();
    bool ok = true;
    input ip = input_file_text_open(path);
    int line_number = 0;
    for (;;)
    {
        nstring text;
        if (!ip->one_line(text))
            break;
        ++line_number;
        int ll = calc_line_length(text);
        if (ll > max_lin_len)
        {
            sub_context_ty sc;
            sc.var_set_format
            (
                "File_Name",
                "%s: %d",
                src->file_name->str_text,
                line_number
            );
            sc.var_set_long("Number", ll - max_lin_len);
            sc.var_optional("Number");
            change_error(cp, &sc, "$filename: line too long, by $number");
            ok = false;
        }
    }
    ip.close();
    os_become_undo();
    return ok;
}
