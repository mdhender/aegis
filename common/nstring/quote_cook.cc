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

#include <common/ac/ctype.h>

#include <common/nstring.h>
#include <common/nstring/accumulator.h>


nstring
nstring::quote_cook(void)
    const
{
    // do we need to quote anything?
    char mode = '\0';
    const char *s = c_str();
    for (;;)
    {
        unsigned char c = *s++;
        if (!c)
            break;
        switch (c)
        {
        case '\'':
        case '"':
            if (mode != '\'' && mode != '"')
                mode = ('\'' + '"') - c;
            break;

        case '/':
            if (*s == '*')
            {
                if (mode != '\'' && mode != '"')
                    mode = -1;
            }
            break;

        case '+':
            if (*s == '=')
            {
                if (mode != '\'' && mode != '"')
                    mode = -1;
            }
            break;

        case ' ':
        case ':':
        case ';':
        case '=':
        case '[':
        case '\\':
        case ']':
        case '{':
        case '}':
            if (mode != '\'' && mode != '"')
                mode = -1;
            break;

        default:
            if (!isprint(c))
            {
                if (mode != '\'' && mode != '"')
                    mode = -1;
            }
            break;
        }
    }

    if (!mode)
    {
        // no quoting required
        return nstring(c_str(), size());
    }
    if (mode < 0)
        mode = '"';

    nstring_accumulator ac;
    ac.push_back(mode);
    s = c_str();
    for (;;)
    {
        unsigned char c = *s++;
        switch (c)
        {
        case '\0':
            ac.push_back(mode);
            return ac.mkstr();

        case '\\':
            ac.push_back("\\\\");
            break;

        case '"':
        case '\'':
            if (c == mode)
                ac.push_back('\\');
            ac.push_back(c);
            break;

        case '\a':
            ac.push_back("\\a");
            break;

        case '\b':
            ac.push_back("\\b");
            break;

        case '\f':
            ac.push_back("\\f");
            break;

        case '\n':
            ac.push_back("\\n");
            break;

        case '\r':
            ac.push_back("\\r");
            break;

        case '\v':
            ac.push_back("\\v");
            break;

        case ' ':
            ac.push_back(c);
            break;

        default:
            if (isprint(c))
                ac.push_back(c);
            else
                ac.push_back(nstring::format("\\%03o", c));
            break;
        }
    }
}


// vim: set ts=8 sw=4 et :
