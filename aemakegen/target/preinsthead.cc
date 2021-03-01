//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <aemakegen/target.h>


static inline bool
safe_isspace(char c)
{
    return isspace((unsigned char)c);
}


void
target::preprocess_include_headers(const nstring &fn)
{
    if (data.get_install_include().member(fn))
        return;
    nstring path = resolve(fn);
    FILE *fp = fopen(path.c_str(), "r");
    if (!fp)
        return;
    data.remember_install_include_source(fn);
    for (;;)
    {
        char line[2000];
        if (!fgets(line, sizeof(line), fp))
            break;
        // look for lines of the form
        // #include <libfubar/*.h>
        char *cp = line;
        while (safe_isspace(*cp))
            ++cp;
        if (*cp != '#')
            continue;
        ++cp;
        while (safe_isspace(*cp))
            ++cp;
        if (0 != memcmp(cp, "include", 7))
            continue;
        cp += 7;
        while (safe_isspace(*cp))
            ++cp;
        if (*cp != '<')
            continue;
        ++cp;
        char *name_begin = cp;
        while (*cp && *cp != '>')
            ++cp;
        if (*cp != '>')
            continue;
        char *name_end = cp;

        // build the name
        nstring name(name_begin, name_end - name_begin);
        if (name.empty())
            continue;

        // If not in library directory, ignore.
        if (!name.starts_with(data.get_library_directory() + "/"))
            continue;

        // process that file, too
        preprocess_include_headers(name);
    }
    fclose(fp);
}


// vim: set ts=8 sw=4 et :
