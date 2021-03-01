//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2012 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/boolean.h>
#include <common/sizeof.h>


nstring
bool_to_string(bool value)
{
    return (value ? "true" : "false");
}


bool
string_to_bool(const nstring &value, bool dflt)
{
    return string_to_bool(value.c_str(), dflt);
}


bool
string_to_bool(const char *value, bool dflt)
{
    if (!value || !*value)
        return dflt;

    struct table_t
    {
        const char *name;
        bool value;
    };

    static const table_t table[] =
    {
        { "false", false },
        { "not", false },
        { "true", true },
        { "yes", true },
    };

    //
    // Look in the table for strings we understand.  We accept any
    // non-empty prefix (they are all distinct in the first character).
    //
    size_t len = strlen(value);
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (0 == strncasecmp(value, tp->name, len))
            return tp->value;
    }

    //
    // Try to turn it into a number, and if it evaluates to zero, it's
    // false (including words we don't know).
    //
    return !!atoi(value);
}


// vim: set ts=8 sw=4 et :
