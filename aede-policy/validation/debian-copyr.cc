//
// aegis - project change supervisor
// Copyright (C) 2009, 2010, 2012 Peter Miller
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

#include <common/now.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/debian-copyr.h>


validation_debian_copyright::~validation_debian_copyright()
{
}


validation_debian_copyright::validation_debian_copyright()
{
}


validation::pointer
validation_debian_copyright::create(void)
{
    return pointer(new validation_debian_copyright());
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


bool
validation_debian_copyright::run(change::pointer cp)
{
    //
    // Don't check branches, only individual changes have control over
    // the debian/copyright source file.
    //
    if (cp->was_a_branch())
        return true;

    //
    // Don't perform this check for changes downloaded and applied by
    // aedist, because the original developer is no longer in control.
    //
    if (was_downloaded(cp))
        return true;

    //
    // Don't perform this check for change sets marked as owning a
    // foreign copyright.
    //
    if (cp->attributes_get_boolean("foreign-copyright"))
        return true;

    //
    // Check each file in the change set.
    //
    fstate_src_ty *src =
        cp->file_find("debian/copyright", view_path_extreme);
    if (!src)
    {
        sub_context_ty sc;
        sc.var_set_charstar("File_Name", "debian/copyright");
        change_error(cp, &sc, i18n("proj has no $filename file"));
        return false;
    }

    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;

    nstring year(calc_year());
    nstring who(cp->pconf_copyright_owner_get());
    nstring suggest("Copyright (C) " + year + " " + who);

    bool copyright_seen = false;
    bool public_domain_seen = false;
    os_become_orig();
    input ip = input_file_open(path);
    for (;;)
    {
        nstring line;
        if (!ip->one_line(line))
            break;
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
    ip.close();
    os_become_undo();
    if (!copyright_seen && !public_domain_seen)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", src->file_name);
        sc.var_set_string("Suggest", suggest);
        sc.var_optional("Suggest");
        change_error(cp, &sc, i18n("$filename: no current copyright notice"));
        return false;
    }
    return true;
}


// vim: set ts=8 sw=4 et :
