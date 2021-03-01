//
// aegis - project change supervisor
// Copyright (C) 2005-2008, 2010, 2012 Peter Miller
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
#include <common/ac/string.h>

#include <common/now.h>
#include <common/nstring/accumulator.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/copyright.h>


validation_files_copyright::~validation_files_copyright()
{
}


static nstring
calc_year()
{
    time_t when = now();
    struct tm *tmp = localtime(&when);
    char buffer[10];
    strftime(buffer, sizeof(buffer), "%Y", tmp);
    return nstring(buffer);
}


validation_files_copyright::validation_files_copyright() :
    year(calc_year())
{
}


validation::pointer
validation_files_copyright::create(void)
{
    return pointer(new validation_files_copyright());
}


bool
validation_files_copyright::check_branches()
    const
{
    return false;
}


bool
validation_files_copyright::check_downloaded()
    const
{
    return false;
}


bool
validation_files_copyright::check_foreign_copyright()
    const
{
    return false;
}


bool
validation_files_copyright::check_binaries()
    const
{
    return false;
}


bool
validation_files_copyright::check(change::pointer cp, fstate_src_ty *src)
{
    //
    // We can only know the copyright owner once we know the change set,
    // because it can be changed by a change set.
    //
    if (who.empty())
    {
        who = nstring(cp->pconf_copyright_owner_get());
        suggest = "Copyright (C) " + year + " " + who;
    }

    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;

    os_become_orig();
    bool result = true;
    bool copyright_seen = false;
    bool public_domain_seen = false;
    input ip = input_file_open(path);
    nstring_accumulator sa;
    for (;;)
    {
        sa.clear();
        for (;;)
        {
            int c = ip->getch();
            if (c < 0)
            {
                if (sa.empty())
                    goto end_of_file;
                break;
            }
            if (c == 0)
            {
                //
                // Binary files are exempt from the copyright check.
                // Image formats usually have a place to put a copyright
                // notice, but there is no good way to check this.
                //
                ip.close();
                os_become_undo();
                return true;
            }
            if (c == '\n')
                break;
            sa.push_back((unsigned char)c);
        }
        nstring line = sa.mkstr();
        const char *s = strstr(line.c_str(), "Copyright (C)");
        if
        (
            s
        &&
            (
                (strstr(s, year.c_str()) && strstr(s, who.c_str()))
            ||
                strstr(s, "${date %Y}")
            )
        )
            copyright_seen = true;
        s = strstr(line.c_str(), "Public Domain");
        if (s && strstr(s, who.c_str()))
            public_domain_seen = true;
    }
end_of_file:
    ip.close();
    os_become_undo();
    if (!copyright_seen && !public_domain_seen)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", src->file_name);
        sc.var_set_string("Suggest", suggest);
        sc.var_optional("Suggest");
        change_error(cp, &sc, i18n("$filename: no current copyright notice"));
        result = false;
    }
    return result;
}


// vim: set ts=8 sw=4 et :
